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
#include "parse_mac_impl.h"
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
  namespace adaptiveOFDM {

    parse_mac::sptr
    parse_mac::make(void*  m_and_p, std::vector<uint8_t> src_mac, bool debug, char* rx_packets_f) {
      return gnuradio::get_initial_sptr(new parse_mac_impl((mac_and_parse*)m_and_p, src_mac, debug, rx_packets_f));
    }


    parse_mac_impl::parse_mac_impl(mac_and_parse*  m_and_p, std::vector<uint8_t> src_mac, bool debug, char* rx_packets_f) :
        block("parse_mac",
          gr::io_signature::make(0, 0, 0),
          gr::io_signature::make(0, 0, 0)),
        d_last_seq_no(-1),
        d_debug(debug) {

      message_port_register_in(pmt::mp("phy in"));
      message_port_register_out(pmt::mp("per"));
      message_port_register_out(pmt::mp("frame data"));
      message_port_register_out(pmt::mp("app out"));
      set_msg_handler(pmt::mp("phy in"), boost::bind(&parse_mac_impl::phy_in, this, _1));

      d_mac_and_parse = m_and_p;
      n_rx_packets = 0;
      rx_packets_fn = rx_packets_f;

      for(int i = 0; i < 6; i++) {
        d_src_mac[i] = src_mac[i];
      }
      if(!d_mac_and_parse->check_mac(src_mac)) throw std::invalid_argument("wrong mac address size");
    }

    parse_mac_impl::~parse_mac_impl() {}


    void
    parse_mac_impl::phy_in (pmt::pmt_t msg) {
      // this must be a pair
      if (!pmt::is_blob(pmt::cdr(msg))) {
        throw std::runtime_error("PMT must be blob");
      }
      if(pmt::is_eof_object(msg)) {
        detail().get()->set_done(true);
        return;
      } else if(pmt::is_symbol(msg)) {
        return;
      }

      pmt::pmt_t dict = pmt::car(msg);
      d_snr = pmt::to_double(pmt::dict_ref(dict, pmt::mp("snr"), pmt::from_double(0)));
      d_snr_var = pmt::to_double(pmt::dict_ref(dict, pmt::mp("snr_var"), pmt::from_double(0)));
      int enc = pmt::to_uint64(pmt::dict_ref(dict, pmt::mp("encoding"),
                                      pmt::from_uint64(-1)));
      msg = pmt::cdr(msg);

      int data_len = pmt::blob_length(msg);
      mac_header *h = (mac_header*)pmt::blob_data(msg);

      if (!equal_mac(d_src_mac, h->addr1)){
        dout << std::endl << std::endl << "Message not for me. Ignoring it." << std::endl;
        return;
      }
      //mylog(boost::format("length: %1%") % data_len );

      dout << std::endl << "new mac frame  (length " << data_len << ")" << std::endl;
      dout << "=========================================" << std::endl;

      #define HEX(a) std::hex << std::setfill('0') << std::setw(2) << int(a) << std::dec
      dout << "duration: " << HEX(h->duration >> 8) << " " << HEX(h->duration  & 0xff) << std::endl;
      dout << "frame control: " << HEX(h->frame_control >> 8) << " " << HEX(h->frame_control & 0xff);

      switch((h->frame_control >> 2) & 3) {
        case 0:
          dout << " (MANAGEMENT)" << std::endl;
          parse_management((char*)h, data_len);
          break;
        case 1:
          dout << " (CONTROL)" << std::endl;
          parse_control((char*)h, data_len);
          break;

        case 2:
          dout << " (DATA)" << std::endl;
          parse_data((char*)h, data_len);
          parse_body((char*)pmt::blob_data(msg), h, data_len);
          int psdu_length;
          d_mac_and_parse->sendAck(h->addr2, &psdu_length);

          // Measuring delay
          /*timeval time_now;
          gettimeofday(&time_now, NULL);
          std::cerr << "\t\tMAC_&_PARSE: data message received and processed: " << time_now.tv_sec << " seg " << time_now.tv_usec << " us\n";*/

          if(rx_packets_fn != ""){
            n_rx_packets++;
            std::fstream rx_packets_fs(rx_packets_fn, std::ofstream::out);
            rx_packets_fs << n_rx_packets << std::endl;
            rx_packets_fs.close();
          }
          send_frame_data(enc);
          break;
        default:
          dout << " (unknown)" << std::endl;
          break;
      }
      decide_encoding();
    }

    void
    parse_mac_impl::send_frame_data(int enc) {
      pmt::pmt_t dict = pmt::make_dict();
      dict = pmt::dict_add(dict, pmt::mp("snr"), pmt::from_double(d_snr));
      dict = pmt::dict_add(dict, pmt::mp("encoding"), pmt::from_long(enc));
      dict = pmt::dict_add(dict, pmt::mp("snr_var"),pmt::from_double(d_snr_var));
      message_port_pub(pmt::mp("frame data"), dict);
    }

    void
    parse_mac_impl::parse_management(char *buf, int length) {
      mac_header* h = (mac_header*)buf;

      if(length < 24) {
        dout << "too short for a management frame" << std::endl;
        return;
      }

      dout << "Subtype: ";
      switch(((h->frame_control) >> 4) & 0xf) {
        case 0:
          dout << "Association Request";
          break;
        case 1:
          dout << "Association Response";
          break;
        case 2:
          dout << "Reassociation Request";
          break;
        case 3:
          dout << "Reassociation Response";
          break;
        case 4:
          dout << "Probe Request";
          break;
        case 5:
          dout << "Probe Response";
          break;
        case 6:
          dout << "Timing Advertisement";
          break;
        case 7:
          dout << "Reserved";
          break;
        case 8:
          dout << "Beacon" << std::endl;
          if(length < 38) {
            return;
          }
          {
          uint8_t* len = (uint8_t*) (buf + 24 + 13);
          if(length < 38 + *len) {
            return;
          }
          std::string s(buf + 24 + 14, *len);
          dout << "SSID: " << s;
          }
          break;
        case 9:
          dout << "ATIM";
          break;
        case 10:
          dout << "Disassociation";
          break;
        case 11:
          dout << "Authentication";
          break;
        case 12:
          dout << "Deauthentication";
          break;
        case 13:
          dout << "Action";
          break;
        case 14:
          dout << "Action No ACK";
          break;
        case 15:
          dout << "Reserved";
          break;
        default:
          break;
      }
      dout << std::endl;

      dout << "seq nr: " << int(h->seq_nr >> 4) << std::endl;
      dout << "My mac: ";
      print_mac_address(d_src_mac, true);
      dout << "mac 1: ";
      print_mac_address(h->addr1, true);
      dout << "mac 2: ";
      print_mac_address(h->addr2, true);
      dout << "mac 3: ";
      print_mac_address(h->addr3, true);
    }


    void
    parse_mac_impl::parse_data(char *buf, int length) {
      mac_header* h = (mac_header*)buf;
      if(length < 24) {
        dout << "too short for a data frame" << std::endl;
        return;
      }

      dout << "Subtype: ";
      switch(((h->frame_control) >> 4) & 0xf) {
        case 0:
          dout << "Data";
          break;
        case 1:
          dout << "Data + CF-ACK";
          break;
        case 2:
          dout << "Data + CR-Poll";
          break;
        case 3:
          dout << "Data + CF-ACK + CF-Poll";
          break;
        case 4:
          dout << "Null";
          break;
        case 5:
          dout << "CF-ACK";
          break;
        case 6:
          dout << "CF-Poll";
          break;
        case 7:
          dout << "CF-ACK + CF-Poll";
          break;
        case 8:
          dout << "QoS Data";
          break;
        case 9:
          dout << "QoS Data + CF-ACK";
          break;
        case 10:
          dout << "QoS Data + CF-Poll";
          break;
        case 11:
          dout << "QoS Data + CF-ACK + CF-Poll";
          break;
        case 12:
          dout << "QoS Null";
          break;
        case 13:
          dout << "Reserved";
          break;
        case 14:
          dout << "QoS CF-Poll";
          break;
        case 15:
          dout << "QoS CF-ACK + CF-Poll";
          break;
        default:
          break;
      }
      dout << std::endl;

      int seq_no = int(h->seq_nr >> 4);
      dout << "seq nr: " << seq_no << std::endl;
      dout << "My mac: ";
      print_mac_address(d_src_mac, true);
      dout << "mac 1: ";
      print_mac_address(h->addr1, true);
      dout << "mac 2: ";
      print_mac_address(h->addr2, true);
      dout << "mac 3: ";
      print_mac_address(h->addr3, true);

      d_100_rx_packets += seq_no - d_last_seq_no;
      d_lost_packets += seq_no - d_last_seq_no - 1;
      float per = d_lost_packets / float(d_100_rx_packets);
      dout << "instantaneous PER: " << per << std::endl;

      // keep track of values
      d_last_seq_no = seq_no;
      pmt::pmt_t pdu = pmt::make_f32vector(1, per * 100);
      message_port_pub(pmt::mp("per"), pmt::cons( pmt::PMT_NIL, pdu ));

      if (d_100_rx_packets >= 100 || d_100_rx_packets < 0) {
        d_lost_packets = 0;
        d_100_rx_packets = 0;
      }
    }

    void
    parse_mac_impl::process_ack() {
      //dout << ": ACK received";
      //d_mac_and_parse->setAckReceived(true);
    }

    void
    parse_mac_impl::parse_control(char *buf, int length) {
      mac_header* h = (mac_header*)buf;

      dout << "Subtype: ";
      switch(((h->frame_control) >> 4) & 0xf) {
        case 7:
          dout << "Control Wrapper";
          break;
        case 8:
          dout << "Block ACK Requrest";
          break;
        case 9:
          dout << "Block ACK";
          break;
        case 10:
          dout << "PS Poll";
          break;
        case 11:
          dout << "RTS";
          break;
        case 12:
          dout << "CTS";
          break;
        case 13:
          dout << "ACK";
          process_ack();
          break;
        case 14:
          dout << "CF-End";
          break;
        case 15:
          dout << "CF-End + CF-ACK";
          break;
        default:
          dout << "Reserved";
          break;
      }
      dout << std::endl;

      dout << "RA: ";
      print_mac_address(h->addr1, true);
    }

    void
    parse_mac_impl::parse_body(char* frame, mac_header *h, int data_len){
      // DATA
      if((((h->frame_control) >> 2) & 63) == 2) {
        print_ascii(frame + 24, data_len - 24);
        send_data(frame + 24, data_len - 24);
      // QoS Data
      } else if((((h->frame_control) >> 2) & 63) == 34) {
        print_ascii(frame + 26, data_len - 26);
        send_data(frame + 26, data_len - 26);
      }
    }

    void
    parse_mac_impl::print_mac_address(uint8_t *addr, bool new_line) {
      if(!d_debug) {
        return;
      }

      dout << std::setfill('0') << std::hex << std::setw(2);
      for(int i = 0; i < 6; i++) {
        dout << (int)addr[i];
        if(i != 5) {
          dout << ":";
        }
      }
      dout << std::dec;
      if(new_line) {
        dout << std::endl;
      }
    }

    bool
    parse_mac_impl::equal_mac(uint8_t *addr1, uint8_t *addr2) {
      for(int i = 0; i < 6; i++){
        if(addr1[i] != addr2[i]){
          return false;
        }
      }
      return true;
    }

    void
    parse_mac_impl::print_ascii(char* buf, int length) {
      for(int i = 0; i < length; i++) {
        if((buf[i] > 31) && (buf[i] < 127)) {
          dout << buf[i];
        } else {
          dout << ".";
        }
      }
      dout << std::endl;
    }

    void
    parse_mac_impl::send_data(char* buf, int length){
      uint8_t* data = (uint8_t*) buf;
      pmt::pmt_t pdu = pmt::init_u8vector(length, data);
      message_port_pub(pmt::mp("app out"), pmt::cons( pmt::PMT_NIL, pdu ));
    }

    void
    parse_mac_impl::decide_encoding(){
      dout << std::endl << "SNR: " << d_snr << std::endl;

      if (d_snr >= MIN_SNR_64QAM_3_4) {
        d_mac_and_parse->setEncoding(QAM64_3_4);
      } else if (d_snr >= MIN_SNR_64QAM_2_3) {
        d_mac_and_parse->setEncoding(QAM64_2_3);
      } else if (d_snr >= MIN_SNR_16QAM_3_4) {
        d_mac_and_parse->setEncoding(QAM16_3_4);
      } else if (d_snr >= MIN_SNR_16QAM_1_2) {
        d_mac_and_parse->setEncoding(QAM16_1_2);
      } else if (d_snr >= MIN_SNR_QPSK_3_4) {
        d_mac_and_parse->setEncoding(QPSK_3_4);
      } else if (d_snr >= MIN_SNR_QPSK_1_2) {
        d_mac_and_parse->setEncoding(QPSK_1_2);
      } else if (d_snr >= MIN_SNR_BPSK_3_4) {
        d_mac_and_parse->setEncoding(BPSK_3_4);
      } else {
        d_mac_and_parse->setEncoding(BPSK_1_2);
      }
    }
  } /* namespace adaptiveOFDM */
} /* namespace gr */
