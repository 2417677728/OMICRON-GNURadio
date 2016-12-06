/*
 * Copyright (C) 2015 Bastian Bloessl <bloessl@ccs-labs.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "lms.h"
#include <cstring>
#include <iostream>

using namespace gr::frequencyAdaptiveOFDM::equalizer;

void lms::equalize(gr_complex *in, int n, gr_complex *symbols, uint8_t *bits, boost::shared_ptr<gr::digital::constellation> mod[4]) {

	if(n == 0) {
		std::memcpy(d_H, in, 64 * sizeof(gr_complex));

	} else if(n == 1) {
		double signal = 0;
		double noise = 0;
		for(int i = 0; i < 64; i++) {
			noise += std::pow(std::abs(d_H[i] - in[i]), 2);
			signal += std::pow(std::abs(d_H[i] + in[i]), 2);
		}

		d_snr = 10 * std::log10(signal / noise / 2);

		for(int i = 0; i < 64; i++) {
			d_H[i] += in[i];
			d_H[i] /= LONG[i] * gr_complex(2, 0);
		}

	} else {
		int c = 0;
		for(int i = 0; i < 64; i++) {
			if( (i == 11) || (i == 25) || (i == 32) || (i == 39) || (i == 53) || (i < 6) || ( i > 58)) {
				continue;
			} else {
				symbols[c] = in[i] / d_H[i];
				gr_complex point;

				if (i < (12+6)) {
					bits[c] = mod[0]->decision_maker(&symbols[c]);
					mod[0]->map_to_points(bits[c], &point);
				} else if (i < (24+6)) {
					bits[c] = mod[1]->decision_maker(&symbols[c]);
					mod[1]->map_to_points(bits[c], &point);
				} else if (i < (36+6)) {
					bits[c] = mod[2]->decision_maker(&symbols[c]);
					mod[2]->map_to_points(bits[c], &point);
				} else if (i < (48+6)) {
					bits[c] = mod[3]->decision_maker(&symbols[c]);
					mod[3]->map_to_points(bits[c], &point);
				}

				d_H[i] = gr_complex(1-alpha,0) * d_H[i] + gr_complex(alpha,0) * in[i] / point;
				c++;
			}
		}
	}
}

double
lms::get_snr() {
	return d_snr;
}
