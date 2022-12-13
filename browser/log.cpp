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
#include <unistd.h>

#include "log.h"

static const int DEBUG = 0;

void LOG(const char *fmt_str, ...) {
  if (!DEBUG)
    return;

  long pid = long(getpid());
  fprintf(stderr, "vlabd[%ld]:", pid);

  va_list ap;
  va_start(ap, fmt_str);
  vfprintf(stderr, fmt_str, ap);
  va_end(ap);

  fprintf(stderr, "\n");
}
