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



#ifndef __LSTRING_H__
#define __LSTRING_H__

#include "mempool.h"
#include "lpfgparams.h"
#include "include/lparams.h"
#include "include/lintrfc.h"

#include "succstor.h"

class LstringIterator;

class Lstring {
  friend class LstringIterator;

public:
  Lstring(size_t initSize = eInitSize);
  Lstring(MemoryPool *);
  ~Lstring();
  void Clear(DerivationDirection dir = eForward);
  void Add(SuccessorStorage &);         // after adding clears the storage
  void Append(const LstringIterator &); // adds the current module
  void Append(const Lstring &);
  size_t AllocatedSize() const { return _size; }
  size_t BytesUsed() const;
  size_t BytesFree() const {
    if (eForward == _direction)
      return _size - _lastByte;
    else
      return _lastByte;
  }
  void Swap(Lstring &);
  DerivationDirection Direction() const { return _direction; }
  char *GetParams(const LstringIterator &) const;

  void Insert(void *pModule, size_t size, size_t offset);

  class Range {
  public:
    size_t Begin() const { return _begin; }
    void Begin(size_t b) { _begin = b; }
    size_t End() const { return _end; }
    void End(size_t e) { _end = e; }
    size_t Size() const { return End() - Begin(); }

  private:
    size_t _begin;
    size_t _end;
  };

  void operator=(const Lstring &src);

  void Replace(const Lstring &src, const Range &srcpos, const Range &trgpos);

private:
  void _Append(const SuccessorStorage &);
  void _Prepend(const SuccessorStorage &);
  char *_mem;
  enum { eInitSize = 1024 };
  size_t _size;
  size_t _lastByte;
  DerivationDirection _direction;
  MemoryPool *_pPool;
  void _Grow();
};
 
#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
