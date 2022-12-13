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



#include <string>

#include <cstdarg>
#include <cstdio>
#include <cstring>

#include "utils.h"

#include "comlineparam.h"
#include "control.h"

#define NOMESSAGE 0

static FILE *fpLog = 0;

void Utils::Warning(const char *format, ...) {
  if (clp.warnings) {
    static char bf[10240];
    va_list args;
    va_start(args, format);
    vsprintf(bf, format, args);
    va_end(args);
  }
}

void Utils::WarningParsing(const char *format, ...) {
  static char bf[10240];
  va_list args;
  va_start(args, format);
  vsprintf(bf, format, args);
  va_end(args);
}

