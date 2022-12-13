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



#ifndef __MEM_H__
#define __MEM_H__

#include <string>

class Mem {

public:
  Mem(long initial_size = 1);
  ~Mem();

  void append(const void *ptr, long size);
   // appends the contents of the string
  void append_string(const std::string &str);
  // appends the contents of the string + the '\0' at the end
  void append_string0(const std::string &str);
  // appends a long
  void append_long(long num);
  void append_ulong(unsigned long num);
  void append_char(char c);
  void append_byte(unsigned char c);
  // easy access to the memory
  unsigned char &operator[](long ind);
  // expands memory at the index
  void expand(long index, long len);
  // shrinks memory at the index
  void shrink(long index, long len);
  // displays the contents of the memory
  void show_ascii(void);
  // this should really be private
  void realloc(long min_size);

  // this is where the data is stored
  unsigned char *data;
  long size;

private:
  long allocated;
};

#endif
