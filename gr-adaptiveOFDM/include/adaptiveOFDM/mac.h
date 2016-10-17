/*
 * Copyright (C) 2013, 2016 Bastian Bloessl <bloessl@ccs-labs.org>
 *                          Samuel Rey Escudero <samuel.rey.escudero@gmail.com>
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
#ifndef INCLUDED_ADAPTIVEOFD_MAC_H
#define INCLUDED_ADAPTIVEOFD_MAC_H

#include <adaptiveOFDM/api.h>
#include <gnuradio/block.h>

namespace gr {
namespace adaptiveOFDM {

class ADAPTIVEOFDM_API mac : virtual public block
{
public:

  typedef boost::shared_ptr<mac> sptr;
  static sptr make(std::vector<uint8_t> src_mac,
      std::vector<uint8_t> dst_mac,
      std::vector<uint8_t> bss_mac);
};

}  // namespace adaptiveOFDM
}  // namespace gr

#endif /* INCLUDED_ADAPTIVEOFD_MAC_H */
