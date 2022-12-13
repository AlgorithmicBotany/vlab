/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include <cstring>
#include <cstdio>

#include "asrt.h"
#include "succstor.h"

const int BfSize = 256;
static char bf[BfSize];

SuccessorStorage::SuccessorStorage() {
  _mem = (char *)malloc(eInitSize);
  _size = eInitSize;
  _appendPos = 0;
}

SuccessorStorage::~SuccessorStorage() { free(_mem); }

void SuccessorStorage::_Grow(size_t requestedSize) {
  size_t newsize = 2 * _size;
  while (newsize < requestedSize)
    newsize *= 2;
  char *pNew = (char *)realloc(_mem, newsize);
  if (0 == pNew) {
    sprintf(bf, "Out of memory, cannot allocate: %zu", newsize);
    throw bf;
  }
  _size = newsize;
  _mem = pNew;
}

char *SuccessorStorage::GetNextChunk(int iSize) {
  ASSERT(_chunkSize > 0);
  while (_appendPos + iSize > _size)
    _Grow(_size * 2);

  char *pResult = _mem + _appendPos;
  _appendPos += iSize;
  return pResult;
}

void SuccessorStorage::AddOneModule(const char *buffer, int iSize) {
  ASSERT(_chunkSize > 0);
  while (_appendPos + iSize > _size)
    _Grow(_size * 2);

  memcpy(_mem + _appendPos, buffer, iSize);
  _appendPos += iSize;
}
