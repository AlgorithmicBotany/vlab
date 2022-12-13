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



#include "lstring.h"
#include "lstriter.h"
#include "succstor.h"
#include <stdio.h>

const int BfSize = 256;
static char bf[BfSize];

Lstring::Lstring(size_t initSize) : _pPool(0) {
  // allocate memory
  _mem = (char *)malloc(initSize);
  if (0 == _mem) {
    sprintf(bf, "Out of memory, cannot allocate: %zu", initSize);
    throw bf;
  }
  _size = initSize;
  _lastByte = 0;
  // by default grow forward
  _direction = eForward;
}

Lstring::Lstring(MemoryPool *pPool) : _pPool(pPool) {
  // memory provided by the MemoryPool
  _mem = _pPool->GetMem();

  if (0 != _mem) {
    _size = _pPool->GetMemSize();
  }
  // unless nothing available
  // then allocate memory yourself
  else {
    _pPool = 0;
    _mem = (char *)malloc(eInitSize);
    if (0 == _mem) {
      sprintf(bf, "Out of memory, cannot allocate: %d", eInitSize);
      throw bf;
    }
    _size = eInitSize;
  }

  _lastByte = 0;
  _direction = eForward;
}

Lstring::~Lstring() {
  // free memory
  if (_pPool != 0)
    _pPool->Release(_mem);
  else
    free(_mem);
}

void Lstring::Clear(DerivationDirection dir) {
  // assume new growing direction
  _direction = dir;
  // reset the content
  if (eForward == _direction)
    _lastByte = 0;
  else
    _lastByte = _size;
}

void Lstring::Add(SuccessorStorage &storage) {
  // depending on the growth direction
  // add the contents of the storage
  // in front or at the end
  if (eForward == _direction)
    _Append(storage);
  else
    _Prepend(storage);
  // and free the storage
  storage.Clear();
}

void Lstring::Append(const LstringIterator &iter) {
  // append one module
  // pointed to by the iterator
  // available in forward growth only
  ASSERT(eForward == _direction);
  const size_t size = iter.GetModuleSize();
  // make sure there is enough room
  while (BytesFree() < size)
    _Grow();
  // copy the module
  memcpy(_mem + _lastByte, iter.Ptr(), size);
  // move the _lastByte pointer
  _lastByte += size;
  ASSERT(_lastByte <= _size);
}

void Lstring::Append(const Lstring &src) {
  ASSERT(eForward == _direction);
  while (BytesFree() < src.BytesUsed())
    _Grow();
  memcpy(_mem + _lastByte, src._mem, src.BytesUsed());
  _lastByte += src.BytesUsed();
  ASSERT(_lastByte <= _size);
}

void Lstring::_Append(const SuccessorStorage &storage) {
  // make sure there is enough room
  while (BytesFree() < storage.Size())
    _Grow();
  // copy the contents
  memcpy(_mem + _lastByte, storage.Bf(), storage.Size());
  // move the _lastByte pointer
  _lastByte += storage.Size();
  ASSERT(_lastByte <= _size);
}

void Lstring::_Prepend(const SuccessorStorage &storage) {
  // make sure there is enough room
  while (BytesFree() < storage.Size())
    _Grow();
  // move the _lastByte pointer
  _lastByte -= storage.Size();
  // copy the contents
  memcpy(_mem + _lastByte, storage.Bf(), storage.Size());
}

void Lstring::_Grow() {
  // double the size of the buffer
  size_t newsize = _size * 2;
  // if using pool
  if (0 != _pPool) {
    // then alloc as new
    char *aNew = (char *)malloc(newsize);
    if (0 == aNew) {
      sprintf(bf, "Out of memory, cannot allocate: %zu", newsize);
      throw bf;
    }
    // release the pool
    _pPool->Release(_mem);
    _pPool = 0;
    _mem = aNew;
  }
  // otherwise realloc
  else {
    char *aNew = (char *)realloc(_mem, newsize);
    if (0 == aNew) {
      sprintf(bf, "Out of memory, cannot allocate: %zu", newsize);
      throw bf;
    }
    _mem = aNew;
  }
  // if growing backward
  // move the contents to the end
  // of the buffer
  if (eBackward == _direction) {
    size_t SizeToMove = _size - _lastByte;
    size_t TargetOffset = newsize - SizeToMove;
    size_t SourceOffset = _lastByte;
    memmove(_mem + TargetOffset, _mem + SourceOffset, SizeToMove);
    _lastByte += TargetOffset - SourceOffset;
  }
  _size = newsize;
}

size_t Lstring::BytesUsed() const {
  if (eForward == _direction)
    return _lastByte;
  else
    return _size - _lastByte;
}

void Lstring::Swap(Lstring &swp) {
  // swap the contents
  // of two Lstrings
  {
    char *tmp = _mem;
    _mem = swp._mem;
    swp._mem = tmp;
  }

  {
    size_t tmp = _size;
    _size = swp._size;
    swp._size = tmp;
  }

  {
    size_t tmp = _lastByte;
    _lastByte = swp._lastByte;
    swp._lastByte = tmp;
  }

  {
    DerivationDirection tmp = _direction;
    _direction = swp._direction;
    swp._direction = tmp;
  }

  {
    MemoryPool *pTmp = _pPool;
    _pPool = swp._pPool;
    swp._pPool = pTmp;
  }
}

char *Lstring::GetParams(const LstringIterator &iter) const {
  ASSERT(iter.GetModuleSize() > sizeof(__lc_ModuleIdType));
  return const_cast<char *>(iter.Ptr() + sizeof(__lc_ModuleIdType));
}

void Lstring::Insert(void *pModule, size_t size, size_t offset) {
  if (BytesFree() < size) {
    _Grow();
  }

  if (eForward == _direction) {
    memmove(_mem + offset + size, _mem + offset, _lastByte - offset);
    memcpy(_mem + offset, pModule, size);
    _lastByte += size;
  } else {
    memmove(_mem + _lastByte - size, _mem + _lastByte, offset - _lastByte);
    memcpy(_mem + offset - size, pModule, size);
    _lastByte -= size;
  }
}

void Lstring::Replace(const Lstring &src, const Lstring::Range &srcpos,
                      const Lstring::Range &trgpos) {
  ASSERT(&src != this);

  if (srcpos.Size() != trgpos.Size()) {
    if (srcpos.Size() > trgpos.Size()) {
      size_t incrmnt = srcpos.Size() - trgpos.Size();
      while (incrmnt > BytesFree())
        _Grow();
    }

    if (eForward == _direction) {
      size_t offset = trgpos.End();
      size_t distance = srcpos.Size() - trgpos.Size();
      size_t dsize = _lastByte - trgpos.End();
      memmove(_mem + offset + distance, _mem + offset, dsize);
      _lastByte += distance;
    } else {
      ASSERT(!"Not supported");
      memmove(_mem + _lastByte - srcpos.Size() + trgpos.Size(),
              _mem + _lastByte, trgpos.Begin() - _lastByte);
      _lastByte -= srcpos.Size() - trgpos.Size();
    }
  }

  memcpy(_mem + trgpos.Begin(), src._mem + srcpos.Begin(), srcpos.Size());
}
