/* -*- c++ -*- */
/*
 * Copyright 2016 Samuel Rey Escudero <samuel.rey.escudero@gmail.com>
 *                 Bastian Bloessl <bloessl@ccs-labs.org>
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

#ifndef INCLUDED_ADAPTIVEOFDM_MAC_IMPL_H
#define INCLUDED_ADAPTIVEOFDM_MAC_IMPL_H

#include <adaptiveOFDM/mac_and_parse.h>
#include <adaptiveOFDM/mac.h>
#include <fstream>

namespace gr {
  namespace adaptiveOFDM {

    class mac_impl : public mac
    {
     public:
      mac_impl(mac_and_parse*  m_and_p,
                std::vector<uint8_t> src_mac,
                std::vector<uint8_t> dst_mac,
                std::vector<uint8_t> bss_mac,
                bool debug,
                char* tx_packets_f,
                char* tx_enc_f);
      ~mac_impl();

      void sendAck(uint8_t ra[], int *psdu_size);

    private:
      mac_and_parse* d_mac_and_parse;
      pthread_mutex_t d_mutex;

      uint8_t d_src_mac[6];
      uint8_t d_dst_mac[6];
      uint8_t d_bss_mac[6];
      uint8_t d_psdu[1528];
      uint16_t d_seq_nr;

      // For meassuring QoS
      char* tx_packets_fn;
      long n_tx_packets;
      bool d_debug;
      std::ofstream tx_enc_fstream;

      void generate_mac_data_frame(const char *msdu, int msdu_size, int *psdu_size);
      void generate_mac_ack_frame(uint8_t ra[], int *psdu_size);
      void send_message(int psdu_length, int enc);
      void app_in (pmt::pmt_t msg);
    };

  } // namespace adaptiveOFDM
} // namespace gr

#endif /* INCLUDED_ADAPTIVEOFDM_MAC_IMPL_H */
