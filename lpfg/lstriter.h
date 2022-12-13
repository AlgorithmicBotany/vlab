/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __LSTRINGITERATOR_H__
#define __LSTRINGITERATOR_H__

#include <cstdlib>
#include <cstring>

#include "include/lparams.h"
#include "include/lintrfc.h"

#include "asrt.h"

size_t GetSizeOfParams(int);
const char *GetNameOf(int);

class Lstring;
class SuccessorStorage;

class LstringIterator {
  friend class LIteratorReset;

public:
  LstringIterator(const Lstring &);
  LstringIterator(const Lstring &, size_t);
  LstringIterator(const LstringIterator &citer)
      : _lstring(citer._lstring), _mem(citer._mem),
        ModuleName(citer.ModuleName), _currentPos(citer._currentPos),
        _CurrentModuleId(citer._CurrentModuleId),
        _moduleSize(citer._moduleSize) {}
  void Info() const;
  void Dump();

  void FindBOS();
  void FindEOS();
  bool AtEnd() const;
  bool AtBeginning() const;
  bool ValidPosition(size_t pos, int MaxModuleId) const;
  void AppendCurrent(SuccessorStorage &) const;
  void MoveTo(size_t pos, int
#if !defined(NDEBUG) && !defined(NOASSERT)
                              MaxModuleId
#endif
  ) {
    ASSERT(ValidPosition(pos, MaxModuleId));
    _currentPos = pos;
    _RetrieveModuleId();
  }

  __lc_ModuleIdType GetModuleId() const {
    ASSERT(!AtEnd());
    return _CurrentModuleId;
  }
  bool TryAdvance(int);
  void operator=(const LstringIterator &rhs) {
    ASSERT(_mem == rhs._mem);
    _currentPos = rhs._currentPos;
    _CurrentModuleId = rhs._CurrentModuleId;
    _moduleSize = rhs._moduleSize;
    ModuleName = rhs.ModuleName;
  }
  bool operator++(); // true if not end of string
  void operator--();
  bool operator+=(int);
  bool operator-=(int);
  bool FindEOB(bool bRing) { return bRing ? FindEOBRing() : FindEOB(); }
  bool FindEOB(); // if current module is EB will return immediately
  bool FindEOBRing();
  bool FindBOB(bool bRing) { return bRing ? FindBOBRing() : FindBOB(); }
  bool FindBOB(); // if current module is SB will return immediately
  bool FindBOBRing();
  const char *Ptr() const { return _mem + _currentPos; }
  const char *GetModuleName() const { return GetNameOf(GetModuleId()); }
  size_t GetModuleSize() const {
    ASSERT(!AtEnd());
    ASSERT(_moduleSize > 0);
    return _moduleSize;
  }

  void Advance(bool bRing) {
    this->operator++();
    if (AtEnd() && bRing)
      FindBOS();
  }

  bool Back(bool bRing) {
    if (AtBeginning()) {
      if (bRing)
        FindEOS();
      else
        return false;
    }
    this->operator--();
    return true;
  }

  size_t Position() const { return _currentPos; }

private:
  const Lstring &_lstring;
  const char *_mem;
  const char *ModuleName;

protected:
  size_t _currentPos;

private:
  __lc_ModuleIdType _CurrentModuleId;
  size_t _moduleSize;
  void _RetrieveModuleId() {
    ASSERT(!AtEnd());
#ifdef NO_MEMCOPY
    _CurrentModuleId =
        (reinterpret_cast<const __lc_BasicParameterStruct *>(Ptr()))->moduleId;
#else
    const __lc_BasicParameterStruct *pB =
        reinterpret_cast<const __lc_BasicParameterStruct *>(Ptr());
    memcpy(&_CurrentModuleId, &(pB->moduleId), sizeof(__lc_ModuleIdType));
#endif
    ModuleName = GetModuleName();
    _moduleSize = GetSizeOfParams(_CurrentModuleId);
  }
};

class LIteratorReset {
public:
  LIteratorReset(LstringIterator &iter) : _iter(iter) {
    _resetPos = _iter._currentPos;
  }
  void Reset();
  void SetPosition() { _resetPos = _iter._currentPos; }

private:
  LstringIterator &_iter;
  size_t _resetPos;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
