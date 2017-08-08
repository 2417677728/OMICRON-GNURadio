#!/usr/bin/env python
# -*- coding: utf-8 -*-
# 
# Copyright 2017 Samuel Rey Escudero <samuel.rey.escudero@gmail.com>
# 
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
# 
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
# 

import os
from time import time
import pmt
from gnuradio import gr

class write_frame_data(gr.sync_block):
    """
    Receive the SNR and Encoding used in each WiFi frame and stores it in the files given
    as arguments. It also stored the delay between Wifi Frames in milliseconds

        - snr_file: path for the file for the snr data
        - enc_file: path for the file for the coding scheme used
        - delay_file: path for the file for the wifi frame delay data
        - debug: if true, the information writen in the files will be displayed 
    """
    def __init__(self, snr_file, enc_file, delay_file, debug):
        gr.sync_block.__init__(self,
            "sink",
            None,
            None)

        self.snr_file = snr_file
        self.enc_file = enc_file
        self.delay_file = delay_file
        self.debug = debug

        # Time in millis
        self.last_time = time() * 1000

        # Check if files have been provided and reset them
        if self.snr_file != "":
            open(self.snr_file, 'w').close()
        else:
            print("No file for SNR information provided.")
        if self.enc_file != "":
            open(self.enc_file, 'w').close()
        else:
            print("No file for Encoding information provided.")
        if self.delay_file != "":
            open(self.delay_file, 'w').close()
        else:
            print("No file for Frame Delay information provided.")

        self.message_port_register_in(pmt.intern("frame data"))
        self.set_msg_handler(pmt.intern("frame data"), self.write_data)


    def write_data(self, msg):
        snr = pmt.f64vector_elements(pmt.dict_ref(msg, pmt.intern("snr"), 
                                    pmt.make_vector(0, pmt.from_long(0))))
        encoding = pmt.s32vector_elements(pmt.dict_ref(msg, pmt.intern("encoding"),
                                    pmt.make_vector(0, pmt.from_long(0))))
        puncturing = pmt.to_long(pmt.dict_ref(msg, pmt.intern("puncturing"),
                                    pmt.from_long(-1)))

        time_now = time() * 1000
        delay = str(time_now - self.last_time)
        self.last_time = time_now

        snr_str = ""
        enc_str = ""
        for i in range(len(snr)):
            snr_str += str(snr[i])
            enc_str += str(encoding[i]) + ", "
            if i != len(snr) - 1:
                snr_str += ", "
                
        enc_str += str(puncturing)

        if self.snr_file != "":
            f_snr = open(self.snr_file, 'a')
            f_snr.write(snr_str + '\n')
            f_snr.close()

        if self.enc_file != "":
            f_enc = open(self.enc_file, 'a')    
            f_enc.write(enc_str + '\n')
            f_enc.close()

        if self.delay_file != "":
            f_delay = open(self.delay_file, 'a')
            f_delay.write(delay + '\n')
            f_delay.close()

        if self.debug:
            print("SNR: " + snr_str)
            print("Encoding: " + enc_str)
            print("Delay in millis: " + delay)