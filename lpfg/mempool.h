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



#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <cstdlib>

#include "asrt.h"

class MemoryPool {
public:
  MemoryPool();
  ~MemoryPool();
  char *GetMem() {
    if (_stack.Empty())
      return 0;
    else
      return _bf + eChunkSize * _stack.Pop();
  }
  size_t GetMemSize() const { return eChunkSize; }
  void Release(const char *pX) {
    ASSERT(!_stack.Full());
    size_t ix = (pX - _bf) / eChunkSize;
    _stack.Push(ix);
  }
  enum { eChunkSize = 1024, eChunkCount = 64 };

private:
  class Stack {
  public:
    Stack() : _top(0) {}
    void Push(size_t i) {
      ASSERT(!Full());
      _arr[_top] = i;
      ++_top;
    }
    size_t Pop() {
      ASSERT(!Empty());
      --_top;
      return _arr[_top];
    }
    bool Empty() const { return 0 == _top; }
    bool Full() const { return eSize == _top; }

  private:
    enum { eSize = eChunkCount };
    int _top;
    size_t _arr[eSize];
  };
  Stack _stack;
  char *_bf;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
