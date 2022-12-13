/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */




#ifndef __LIB_RA_SEARCH_H_VLAB__
#define __LIB_RA_SEARCH_H_VLAB__

#include <string>

void searchBegin(
    const std::string & oofs,
    const std::string & start_path,
    const std::string & pattern,
    bool caseSensitive = false,
    bool exactMatch = false
    );
std::string searchContinue(
    bool blocking = true
    );
void searchEnd();

#endif
