/*
 * testbed: Simulation environment for PFPSim Framework models
 *
 * Copyright (C) 2016 Concordia Univ., Montreal
 *     Samar Abdi
 *     Umair Aftab
 *     Gordon Bailey
 *     Faras Dewal
 *     Shafigh Parsazad
 *     Eric Tremblay
 *
 * Copyright (C) 2016 Ericsson
 *     Bochra Boughzala
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "./Testbed.h"
#include <string>

Testbed::Testbed(sc_module_name nm, pfp::core::PFPObject* parent,std::string configfile ):TestbedSIM(nm,parent,configfile) {  // NOLINT
    /*sc_spawn threads*/
}

void Testbed::init() {
    init_SIM(); /* Calls the init of sub PE's and CE's */
}
void Testbed::Testbed_PortServiceThread() {
  // Thread function to service input ports.
}
void Testbed::TestbedThread(std::size_t thread_id) {
  // Thread function for module functionalty
}
