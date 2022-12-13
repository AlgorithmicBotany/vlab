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



#include "psout.h"
#include <algorithm>
#include <sstream>

PsOutputStore::PsOutputStore() {}
void PsOutputStore::PrintToStream(std::ostream &trg) {
  for (size_t i = 0; i < data.size(); i++) {
    trg << data[i]->command;
  }
}

PsOutputStore &PsOutputStore::operator<<(const std::string input) {
  data.back()->command.append(input);
  return *this;
}
PsOutputStore &PsOutputStore::operator<<(const char *input) {
  data.back()->command.append(input);
  return *this;
}
PsOutputStore &PsOutputStore::operator<<(const char input) {
  data.back()->command += input;
  return *this;
}
PsOutputStore &PsOutputStore::operator<<(const float input) {
  std::ostringstream ostr;
  ostr << input;
  data.back()->command.append(ostr.str());
  return *this;
}
PsOutputStore &PsOutputStore::operator<<(const int input) {
  std::ostringstream ostr;
  ostr << input;
  data.back()->command.append(ostr.str());
  return *this;
}

void PsOutputStore::SetDepth(int d) { data.back()->depth = d; }
void PsOutputStore::AddNew() { data.push_back(new Data()); }
void PsOutputStore::Reset() { data.clear(); }
void PsOutputStore::Sort() {
  std::sort(data.begin(), data.end(), DataComparator());
}
