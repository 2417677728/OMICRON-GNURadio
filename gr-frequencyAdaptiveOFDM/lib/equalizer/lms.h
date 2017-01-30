/*
 * Copyright (C) 2015 	Samuel Rey <samuel.rey.escudero@gmail.com>
 *						Bastian Bloessl <bloessl@ccs-labs.org>
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

#ifndef INCLUDED_FREQUENCYADAPTIVEOFDM_EQUALIZER_LMS_H
#define INCLUDED_FREQUENCYADAPTIVEOFDM_EQUALIZER_LMS_H

#include "base.h"
#include <vector>

namespace gr {
namespace frequencyAdaptiveOFDM {
namespace equalizer {

class lms: public base {
public:
	virtual void equalize(gr_complex *in, int n, gr_complex *symbols, uint8_t *bits, boost::shared_ptr<gr::digital::constellation> mod[4]);
private:
	double get_snr();
	std::vector<double> resource_blocks_snr();

	gr_complex d_H[64];
	double d_snr;
	std::vector<double> d_resource_block_snr;
	const double alpha = 0.5;
};

} /* namespace channel_estimation */
} /* namespace frequencyAdaptiveOFDM */
} /* namespace gr */

#endif /* INCLUDED_FREQUENCYADAPTIVEOFDM_EQUALIZER_LMS_H */
