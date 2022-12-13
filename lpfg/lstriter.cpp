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



#include "lstriter.h"
#include "lstring.h"
#include "succstor.h"

#include "StdModulesStruct.h"

LstringIterator::LstringIterator(const Lstring &lstring)
    : _lstring(lstring), _mem(_lstring._mem) {
  FindBOS();
}

LstringIterator::LstringIterator(const Lstring &lstring, size_t pos)
    : _lstring(lstring), _mem(_lstring._mem), _currentPos(pos) {
  if (!AtEnd())
    _RetrieveModuleId();
}

void LstringIterator::FindBOS() {
  if (eForward == _lstring._direction)
    _currentPos = 0;
  else
    _currentPos = _lstring._lastByte;

  if (!AtEnd())
    _RetrieveModuleId();
}

void LstringIterator::FindEOS() {
  if (eForward == _lstring._direction)
    _currentPos = _lstring._lastByte;
  else
    _currentPos = _lstring._size;
}

void LstringIterator::operator--() {
  ASSERT(!AtBeginning());
  ASSERT(_currentPos >= GetModuleSize());
  _currentPos -= sizeof(__lc_ModuleIdType);
  _RetrieveModuleId();
  _currentPos -= GetModuleSize() - sizeof(__lc_ModuleIdType);
}

bool LstringIterator::operator++() {
  ASSERT(!AtEnd());
  const size_t Size = GetModuleSize();

  _currentPos += Size;

  ASSERT((eForward == _lstring._direction) &&
             (_currentPos <= _lstring._lastByte) ||
         (eBackward == _lstring._direction) && (_currentPos <= _lstring._size));

  if (!AtEnd()) {
    _RetrieveModuleId();
    return true;
  } else
    return false;
}

bool LstringIterator::TryAdvance(int n) {
  ASSERT(!AtEnd());
  ASSERT(n > 0);
  while (n > 0) {
    if (!operator++())
      return false;
    --n;
  }
  return true;
}

bool LstringIterator::operator+=(int n) {
  ASSERT(n > 0);
  for (int i = 0; i < n - 1; ++i)
    operator++();
  return operator++();
}

bool LstringIterator::operator-=(int n) {
  ASSERT(n > 0);
  for (int i = 0; i < n; ++i) {
    if (AtBeginning())
      return false;
    operator--();
  }
  return true;
}

bool LstringIterator::AtEnd() const {
  if (eForward == _lstring._direction) {
    ASSERT(_currentPos <= _lstring._lastByte);
    return _lstring._lastByte == _currentPos;
  } else {
    ASSERT(_currentPos <= _lstring._size);
    return _currentPos == _lstring._size;
  }
}

bool LstringIterator::FindEOB() {
  int nested = 1;
  if (!AtEnd())
    do {
      const int mid = GetModuleId();
      if (SB_id == mid)
        ++nested;
      else if (EB_id == mid) {
        --nested;
        if (0 == nested)
          return true;
      }
    } while (operator++());
  return false;
}

bool LstringIterator::FindEOBRing() {
  int nested = 1;
  const size_t initPos = _currentPos;
  for (;;) {
    if (AtEnd())
      FindBOS();
    const int mid = GetModuleId();
    if (SB_id == mid)
      ++nested;
    else if (EB_id == mid) {
      --nested;
      if (0 == nested)
        return true;
    }
    operator++();
    if (initPos == _currentPos)
      return false;
  }
}

bool LstringIterator::FindBOB() {
  int nested = 1;
  while (!AtBeginning()) {
    const int mid = GetModuleId();
    if (SB_id == mid) {
      --nested;
      if (0 == nested)
        return true;
    } else if (EB_id == mid)
      ++nested;
    operator--();
  }
  return false;
}

bool LstringIterator::FindBOBRing() {
  int nested = 1;
  const size_t initPos = _currentPos;
  for (;;) {
    const int mid = GetModuleId();
    if (SB_id == mid) {
      --nested;
      if (0 == nested)
        return true;
    } else if (EB_id == mid)
      ++nested;
    if (AtBeginning())
      FindEOS();
    operator--();
    if (initPos == _currentPos)
      return false;
  }
}

bool LstringIterator::AtBeginning() const {
  if (eForward == _lstring._direction) {
    return 0 == _currentPos;
  } else {
    ASSERT(_currentPos >= _lstring._lastByte);
    return _currentPos == _lstring._lastByte;
  }
}

void LstringIterator::AppendCurrent(SuccessorStorage &storage) const {
  ASSERT(!AtEnd());
  storage.AddOneModule(_mem + _currentPos, GetModuleSize());
}

bool LstringIterator::ValidPosition(size_t pos, int MaxModuleId) const {
  if (eForward == _lstring._direction) {
    if (pos > _lstring._lastByte)
      return false;
  } else // eBackward
  {
    if (pos < _lstring._lastByte || _currentPos > _lstring._size)
      return false;
  }

  const __lc_BasicParameterStruct *pModule =
      reinterpret_cast<const __lc_BasicParameterStruct *>(_mem + pos);
  const __lc_ModuleIdType *pMit = &pModule->moduleId;
  if (*pMit < 0)
    return false;
  if (*pMit > MaxModuleId)
    return false;

  pMit += GetSizeOfParams(*pMit) / sizeof(__lc_ModuleIdType) - 1;
  return pModule->moduleId == *pMit;
}

void LIteratorReset::Reset() {
  _iter._currentPos = _resetPos;
  if (!_iter.AtEnd())
    _iter._RetrieveModuleId();
}
