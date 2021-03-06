/* -*- c++ -*- */
/*
 * Copyright 2016   Samuel Rey <samuel.rey.escudero@gmail.com>
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


#ifndef INCLUDED_ADAPTIVEOFDM_MAPPER_IMPL_H
#define INCLUDED_ADAPTIVEOFDM_MAPPER_IMPL_H

#include <adaptiveOFDM/mapper.h>
#include "utils.h"
#include <fstream>

namespace gr {
  namespace adaptiveOFDM {

    class mapper_impl : public mapper
    {
    private:
      bool d_debug_enc;
      bool d_debug;
      bool d_log;
      char* d_symbols;
      int d_symbols_offset;
      int d_symbols_len;
      ofdm_param d_ofdm;

      std::ofstream tx_enc_fstream;

    public:
      mapper_impl(bool debug_enc, Encoding e, bool debug, bool log, char* tx_enc_f);
      ~mapper_impl();

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

      void print_message(const char *msg, size_t len);
    };


  } // namespace adaptiveOFDM
} // namespace gr

#endif /* INCLUDED_ADAPTIVEOFDM_MAPPER_IMPL_H */
