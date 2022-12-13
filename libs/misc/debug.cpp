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



#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"

int debug_printf(const char *fmt_str, ...) {
  if (DEBUG) {
    fprintf(stderr, "Debug: ");
    va_list ap;

    va_start(ap, fmt_str);

    int result = vfprintf(stderr, fmt_str, ap);

    va_end(ap);

    return result;
  } else
    return 0;
}

int debug_printf_nh(const char *fmt_str, ...) {
  if (DEBUG) {
    va_list ap;

    va_start(ap, fmt_str);

    int result = vfprintf(stderr, fmt_str, ap);

    va_end(ap);

    return result;
  } else
    return 0;
}
