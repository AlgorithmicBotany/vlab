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



#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <iostream>
#include <regex>
#include <iterator>


#include "file.h"

#if defined(__APPLE__) || defined(linux)
#include <sys/file.h>
#endif

std::string do_replace( std::string const & in, std::string const & from, std::string const & to )
{
  return std::regex_replace( in, std::regex(from), to );
}



File::File(const char *fname, const char *mode) {
  name = std::string(fname);
  do_replace(name," ", "\ ");
  const char *filename = name.c_str();

  _fp = fopen(filename, mode);
#if defined(__APPLE__) || defined(linux)
  if (_fp)
    flock(fileno(_fp), LOCK_EX);
#endif
}

File::~File() {
#if defined(__APPLE__) || defined(linux)
  if (_fp)
    flock(fileno(_fp), LOCK_UN);
#endif
  if (_fp)
    fclose(_fp);
}

void WriteTextFile::PrintF(const char *format, ...) {
  assert(Valid());
  va_list args;
  va_start(args, format);
  vfprintf(_fp, format, args);
  va_end(args);
}
