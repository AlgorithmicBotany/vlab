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



#ifndef __LPFGPARAMS_H__
#define __LPFGPARAMS_H__

/*
        LPFGparams:
        these are the params used only by lpfg
*/

enum DerivationDirection { eForward, eBackward };

namespace LPFGParams {
enum {
  NumOfColors = 256,
  AnimTimer = 101,
  DefaultContourDivisions = 8,
  MinContourDivisions = 3,
  MaxContourDivisions = 10000,
  ObjectNameLength = 32,
  MaxNumOfLights = 8
};

extern const char *PreprocScript;
extern const char *CompileScript;
extern const char *CompiledLsys;
extern const char *CompiledOLsys;
extern const char *TranslatedFileName;
} // namespace LPFGParams

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
