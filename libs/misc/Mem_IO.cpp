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
#include <string.h>

#include "Mem_IO.h"
#include "xmemory.h"
#include "xstring.h"

/******************************************************************************
 *
 * constructor
 *
 */

Mem_IO::Mem_IO() {
  data = NULL;
  size = 0;
  alloc_size = 0;
}

/******************************************************************************
 *
 * destructor
 *
 */

Mem_IO::~Mem_IO() {
  xfree(data);
  data = NULL;
  size = 0;
  alloc_size = 0;
}

/******************************************************************************
 *
 * printf - just like printf, but add the result into memory
 *
 */

void Mem_IO::printf(const char *fmt_str, ...) {
  // print evrything into a buffer first
  char buf[4096];
  {
    va_list ap;
    va_start(ap, fmt_str);
    vsprintf(buf, fmt_str, ap);
    va_end(ap);
  }

  // do we need to add more memory?
  int n = xstrlen(buf);
  if (n > alloc_size - size) {
    // not enough memory, so add some
    alloc_size += n * 2;
    data = (Byte *)xrealloc(data, alloc_size);
  }

  // append the buffer to the data in memory
  memcpy(data + size, buf, n);

  // update the size
  size += n;
}
/******************************************************************************
 *
 * put - just like putc(3) but adds the result into memory
 *
 */

void Mem_IO::put(Byte c) {
  // do we need to add more memory?
  if (1 > alloc_size - size) {
    // not enough memory, so add some
    alloc_size += 10;
    data = (Byte *)xrealloc(data, alloc_size);
  }

  // append the buffer to the data in memory
  data[size] = c;

  // update the size
  size += 1;
}
