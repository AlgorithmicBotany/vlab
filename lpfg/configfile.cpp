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



#include <cstring>

#include "configfile.h"
#include "utils.h"
#include "file.h"

int ConfigFile::_Label(const char *cmnd, int &cntn, const char *lbls[],
                       int count) const {
  if (0 == cmnd[0])
    return -2;
  for (int i = 0; i < count; ++i) {
    size_t l = strlen(lbls[i]);
    if (0 == strncmp(cmnd, lbls[i], l)) {
      cntn = l;
      return i;
    }
  }
  return -1;
}

bool ConfigFile::_ReadOnOff(const char *cmnd, unsigned int f) {
  cmnd = Utils::SkipBlanks(cmnd);
  if (cmnd[0] == 0)
    return false;
  if (!strncmp(cmnd, "on", 2)) {
    cmnd += 2;
    cmnd = Utils::SkipBlanks(cmnd);
    if (0 != cmnd[0])
      return false;
    _FlagSet(f);
    return true;
  }
  if (!strncmp(cmnd, "off", 3)) {
    cmnd += 3;
    cmnd = Utils::SkipBlanks(cmnd);
    if (0 != cmnd[0])
      return false;
    _FlagClear(f);
    return true;
  }
  return false;
}

void ConfigFile::_Error(const char *cmnd, const ReadTextFile &src) const {
  Utils::Message("Error in command: %s\nin file %s at line %d\n", cmnd,
                 src.Filename(), src.Line());
}
