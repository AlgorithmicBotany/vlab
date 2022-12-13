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



#ifndef __SUCCESSORSTORAGE_H__
#define __SUCCESSORSTORAGE_H__

#include <cstdlib>

class SuccessorStorage {
public:
  SuccessorStorage();
  ~SuccessorStorage();
  void Clear() { _appendPos = 0; }
  size_t Size() const { return _appendPos; }
  char *GetNextChunk(int iSize);

  const char *Bf() const { return _mem; }

  void AddOneModule(const char *buffer, int iSize);

private:
  void _Grow(size_t);
  char *_mem;
  size_t _size;
  size_t _appendPos;
  enum { eInitSize = 4096 };
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
