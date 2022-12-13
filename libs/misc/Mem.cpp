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



#include "Mem.h"
#include "xmemory.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Mem::Mem(long initial_size) {
  allocated = initial_size;
  data = (unsigned char *)xmalloc(sizeof(unsigned char) * allocated);
  size = 0;
}

Mem::~Mem() {
  if (data != NULL)
    xfree(data);
}

void Mem::append(const void *dta, long sz) {
  // do we need to reallocate memory?
  if (size + sz > allocated)
    this->realloc(size + sz);

  // append the new data to the rest
  memmove(data + size, dta, sz);
  size += sz;
}

// appends the contents of the string
void Mem::append_string(const std::string &str) {
  append(str.c_str(), str.length());
}
// appends the contents of the string + the '\0' at the end
void Mem::append_string0(const std::string &str) {
  append_string(str);
  append_byte(0);
}

// appends a long
void Mem::append_long(long num) { append(&num, sizeof(long)); }

void Mem::append_ulong(unsigned long num) {
  append(&num, sizeof(unsigned long));
}

void Mem::append_char(char c) { append(&c, 1); }
void Mem::append_byte(unsigned char c) { append(&c, 1); }

unsigned char &Mem::operator[](long ind) {
  assert(ind >= 0);
  assert(ind < size);

  return data[ind];
}

void Mem::realloc(long min_size) {
  if (min_size <= allocated)
    return;

  // try to add 50 %
  long to_add = allocated / 2 + 1;
  // if that is not enough, add exactly what is needed
  if (allocated + to_add < min_size)
    to_add = min_size - allocated;
  allocated += to_add;
  data = (unsigned char *)xrealloc(data, allocated);
}

void Mem::expand(long ind, long len) {
  assert(ind < size);
  assert(ind >= 0);
  assert(len > 0);

  // fprintf( stderr, "Mem::expand by %ld\n", len);

  // make sure we have enough memory
  this->realloc(size + len);

  // move everything from index 'len' places
  memmove(data + ind + len, data + ind, size - ind);

  // done
  size += len;
}

void Mem::shrink(long ind, long len) {
  assert(ind < size);
  assert(ind >= 0);
  assert(len > 0);
  assert(len < size - ind);

  // fprintf( stderr, "Mem::shrink by %ld\n", len);

  // move everything after ind+len to ind
  memmove(data + ind, data + ind + len, size - ind - len);
  size -= len;

  // done
}

void Mem::show_ascii(void) {
  printf("\n"
         "\nMemory dump:\n");
  for (long i = 0; i < size; i++)
    putchar(data[i]);
}
