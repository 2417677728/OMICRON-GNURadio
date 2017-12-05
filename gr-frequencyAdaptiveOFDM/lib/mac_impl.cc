/* -*- c++ -*- */
/*
 * Copyright 2017 Samuel Rey <samuel.rey.escudero@gmail.com>
 *                  Bastian Bloessl <bloessl@ccs-labs.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "mac_impl.h"
#include <gnuradio/block_detail.h>

#if defined(__APPLE__)
#include <architecture/byte_order.h>
#define htole16(x) OSSwapHostToLittleInt16(x)
#else
#include <endian.h>
#endif

#include <boost/crc.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace gr {
  namespace frequencyAdaptiveOFDM {

    mac::sptr
    mac::make(void* m_and_p, std::vector<uint8_t> src_mac, std::vector<uint8_t> dst_mac,
              std::vector<uint8_t> bss_mac, bool debug, char* tx_packets_f) {
      return gnuradio::get_initial_sptr(new mac_impl((mac_and_parse*)m_and_p, src_mac, dst_mac, bss_mac, debug, tx_packets_f));
    }

    mac_impl::mac_impl(mac_and_parse*  m_and_p, std::vector<uint8_t> src_mac, std::vector<uint8_t> dst_mac,
                        std::vector<uint8_t> bss_mac, bool debug, char* tx_packets_f) :
        block("mac",
          gr::io_signature::make(0, 0, 0),
          gr::io_signature::make(0, 0, 0)),
        d_seq_nr(0),
        d_debug(debug) {

      message_port_register_in(pmt::mp("app in"));
      message_port_register_out(pmt::mp("phy out"));
      set_msg_handler(pmt::mp("app in"), boost::bind(&mac_impl::app_in, this, _1));

      if(!d_mac_and_parse->check_mac(src_mac)) throw std::invalid_argument("wrong mac address size");
      if(!d_mac_and_parse->check_mac(dst_mac)) throw std::invalid_argument("wrong mac address size");
      if(!d_mac_and_parse->check_mac(bss_mac)) throw std::invalid_argument("wrong mac address size");

      for(int i = 0; i < 6; i++) {
        d_src_mac[i] = src_mac[i];
        d_dst_mac[i] = dst_mac[i];
        d_bss_mac[i] = bss_mac[i];
      }

      d_mac_and_parse = m_and_p;
      n_tx_packets = 0;
      tx_packets_fn = tx_packets_f;

      if (d_debug) {
        ofdm_param ofdm(d_mac_and_parse->getEncoding(), d_mac_and_parse->getPuncturing());
        ofdm.print();
      }
      pthread_mutex_init(&d_mutex, NULL);
    }

    mac_impl::~mac_impl() {
      pthread_mutex_destroy(&d_mutex);
    }

    void
    mac_impl::app_in (pmt::pmt_t msg) {
      size_t       msg_len;
      const char   *msdu;
      std::string  str;

      dout << "MAC_&_PARSE: new message in app_in\n";
      if(pmt::is_symbol(msg)) {

        str = pmt::symbol_to_string(msg);
        msg_len = str.length();
        msdu = str.data();

      } else if(pmt::is_pair(msg)) {

        msg_len = pmt::blob_length(pmt::cdr(msg));
        msdu = reinterpret_cast<const char *>(pmt::blob_data(pmt::cdr(msg)));

      } else {
        throw std::runtime_error("MAC expects PDUs or strings");
        return;
      }

      if(msg_len > MAX_PAYLOAD_SIZE) {
        throw std::runtime_error("Frame too large (> 1500)");
      }

      // make MAC frame
      /*if (d_mac_and_parse->getAckReceived()) {
        if (d_mac_and_parse->d_debug_ack){
          timeval time_now;
          gettimeofday(&time_now, NULL);
          std::cout << "\t\tWARNING: MAC: ACK received after timeout: " <<
          time_now.tv_sec << " sec " << time_now.tv_usec << " usec\n";
        }
        d_mac_and_parse->setAckReceived(false);
      }*/
      sendDataMsg(msdu, msg_len);
      usleep(TIMEOUT);
      /*if (!d_mac_and_parse->getAckReceived()) {
        if (d_mac_and_parse->d_debug_ack){
          timeval time_now;
          gettimeofday(&time_now, NULL);
          std::cout << "\t\t\t\tWARNING: MAC: ACK NOT received after timeout: " <<
          time_now.tv_sec << " sec " << time_now.tv_usec << " usec\n";
        }

      }else{
        d_mac_and_parse->setAckReceived(false);
      }*/

      if(tx_packets_fn != ""){
        n_tx_packets++;
        std::fstream tx_packets_fs(tx_packets_fn, std::ofstream::out);
        tx_packets_fs << n_tx_packets << std::endl;
        tx_packets_fs.close();
      }
    }

    void
    mac_impl::sendDataMsg(const char* msdu, size_t msg_len) {
      int psdu_length;
      timeval tv;
      unsigned long current_time;

      pthread_mutex_lock(&d_mutex);
      generate_mac_data_frame(msdu, msg_len, &psdu_length);
      gettimeofday(&tv, NULL);
      current_time = 1000000 * tv.tv_sec + tv.tv_usec;

      unsigned long expiration = d_mac_and_parse->getTimestamp() + 3*TIMEOUT;
      if (current_time > expiration) {
        if (d_mac_and_parse->d_debug || d_mac_and_parse->d_debug_ack){
          std::cout << "MAC: old timestamp. current time: " << current_time/1000000 << " s "
                    << current_time%1000000 << " us. Expiration: " << expiration/1000000 << "s "
                    << expiration%1000000 << " us.\n";
        }
        d_mac_and_parse->decrease_encoding();
      }
      ofdm_param ofdm(d_mac_and_parse->getEncoding(), d_mac_and_parse->getPuncturing());
      send_message(psdu_length, ofdm);
      pthread_mutex_unlock(&d_mutex);

      if (d_mac_and_parse->d_debug_ack){
        timeval time_now;
        gettimeofday(&time_now, NULL);
        std::cout << "MAC: Message sent: " <<
        time_now.tv_sec << " sec " << time_now.tv_usec << " usec\n";
      }
    }

    void
    mac_impl::sendAck(uint8_t ra[], int *psdu_size) {
      pthread_mutex_lock(&d_mutex);
      generate_mac_ack_frame(ra, psdu_size);
      send_message(*psdu_size, ofdm_param(std::vector<int>(N_RB, BPSK), P_1_2));
      pthread_mutex_unlock(&d_mutex);
    }

    void
    mac_impl::send_message(int psdu_length, ofdm_param ofdm) {
      // dict
      pmt::pmt_t dict = pmt::make_dict();
      dict = pmt::dict_add(dict, pmt::mp("crc_included"), pmt::PMT_T);
      pmt::pmt_t encoding = pmt::init_s32vector(4, ofdm.resource_blocks_e);
      dict = pmt::dict_add(dict, pmt::mp("encoding"), encoding);
      dict = pmt::dict_add(dict, pmt::mp("puncturing"), pmt::from_long(ofdm.punct));

      // blob
      pmt::pmt_t mac = pmt::make_blob(d_psdu, psdu_length);
      message_port_pub(pmt::mp("phy out"), pmt::cons(dict, mac));
    }

    void
    mac_impl::generate_mac_data_frame(const char *msdu, int msdu_size, int *psdu_size) {
      // mac header
      mac_header header;
      header.frame_control = 0x0008;
      header.duration = 0x0000;

      for(int i = 0; i < 6; i++) {
        header.addr1[i] = d_dst_mac[i];
        header.addr2[i] = d_src_mac[i];
        header.addr3[i] = d_bss_mac[i];
      }

      header.seq_nr = 0;
      for (int i = 0; i < 12; i++) {
        if(d_seq_nr & (1 << i)) {
          header.seq_nr |=  (1 << (i + 4));
        }
      }
      header.seq_nr = htole16(header.seq_nr);
      d_seq_nr++;

      //header size is 24, plus 4 for FCS means 28 bytes
      *psdu_size = 28 + msdu_size;

      //copy mac header into psdu
      std::memcpy(d_psdu, &header, 24);
      //copy msdu into psdu
      memcpy(d_psdu + 24, msdu, msdu_size);
      //compute and store fcs
      boost::crc_32_type result;
      result.process_bytes(d_psdu, msdu_size + 24);

      uint32_t fcs = result.checksum();
      memcpy(d_psdu + msdu_size + 24, &fcs, sizeof(uint32_t));
    }

    void
    mac_impl::generate_mac_ack_frame(uint8_t ra[], int *psdu_size){
      mac_ack_header header;
      header.frame_control = 0x00D4;
      header.duration = 0x0000;

      for(int i = 0; i < 6; i++) {
        header.ra[i] = ra[i];
      }
      *psdu_size = 10;
      std::memcpy(d_psdu, &header, *psdu_size);
      boost::crc_32_type result;
      result.process_bytes(d_psdu, *psdu_size);

      uint32_t fcs = result.checksum();
      memcpy(d_psdu + *psdu_size, &fcs, sizeof(uint32_t));

      // Plus 4bytes of FCS
      *psdu_size += 4;
    }
  } /* namespace frequencyAdaptiveOFDM */
} /* namespace gr */
