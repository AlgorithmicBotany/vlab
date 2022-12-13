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



#include "lpfgparams.h"

namespace LPFGParams {
#ifdef _WINDOWS
const char *PreprocScript = "preproc.bat";
#else
const char *PreprocScript = "preproc.sh";
#endif

#ifdef _WINDOWS
const char *CompileScript = "cmpl.bat";
#else
const char *CompileScript = "cmpl.sh";
#endif

#ifdef _WINDOWS
const char *CompiledLsys = ".\\lsys.dll";
#else
const char *CompiledLsys = "./lsys.so";
#endif

#ifdef _WINDOWS
const char *CompiledOLsys = ".\\lsys.o";
#else
const char *CompiledOLsys = "./lsys.o";
#endif

#ifdef _WINDOWS
const char *TranslatedFileName = "lsys.i";
#else
const char *TranslatedFileName = "lsys.ii";
#endif

} // namespace LPFGParams
