/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "lstring.h"
#include "lstriter.h"
#include "lengine.h"

#include "ContextScanner.h"

RightContextScanner *pgRContextScanner = NULL;
LeftContextScanner *pgLContextScanner = NULL;
RightContextScanner *pgRNContextScanner = NULL;
LeftContextScanner *pgLNContextScanner = NULL;

ContextScanner::ContextScanner(const LEngine &lengine,
                               LstringIterator &iterator, bool bRing,
                               int iConsiderGroup)
    : _bRing(bRing), _lengine(lengine), _iterator(iterator), _reset(_iterator),
      _iConsiderGroup(iConsiderGroup) {}

RightContextScanner::RightContextScanner(const LEngine &lengine,
                                         LstringIterator &iterator, bool bRing,
                                         int iConsiderGroup)
    : ContextScanner(lengine, iterator, bRing, iConsiderGroup) {}

bool RightContextScanner::Accept(__lc_ModuleIdType moduleId) {
  if (_iterator.AtEnd()) {
    if (_bRing) {
      _iterator.FindBOS();
    } else {
      return false;
    }
  }

  if (EB_id == moduleId) {
    if (!_iterator.FindEOB(_bRing))
      return false;
  }

  size_t lastPosition = _iterator.Position();
  while (_iterator.GetModuleId() != moduleId) {
    if (_iterator.AtEnd()) {
      return false;
    } else if (!_lengine.Considered(_iterator.GetModuleId(), ConsiderGroup())) {
      _iterator.Advance(_bRing);
    } else if (SB_id == _iterator.GetModuleId()) {
      // Skip the branch
      _iterator.Advance(_bRing);
      if (!_iterator.FindEOB(_bRing))
        return false;
      else
        _iterator.Advance(_bRing);
    } else {
      break;
    }

    if (_iterator.Position() == lastPosition)
      break;
  }

  return (_iterator.GetModuleId() == moduleId);
}

LeftContextScanner::LeftContextScanner(const LEngine &lengine,
                                       LstringIterator &iterator, bool bRing,
                                       int iConsiderGroup)
    : ContextScanner(lengine, iterator, bRing, iConsiderGroup) {}

bool LeftContextScanner::Accept(__lc_ModuleIdType moduleId) {
  // if beginning of the string -- break
  if (_iterator.AtBeginning()) {
    if (!_bRing)
      return false;
    else
      _iterator.FindEOS();
  }

  _iterator.Back(_bRing);

  size_t lastPosition = _iterator.Position();

  while (_iterator.GetModuleId() != moduleId) {
    // skip ignored/not considered modules
    if (!_lengine.Considered(_iterator.GetModuleId(), ConsiderGroup())) {
      if (!_iterator.Back(_bRing))
        return false;
    }
    // if SB found
    else if (SB_id == _iterator.GetModuleId()) {
      // and not at the beginning of the string
      // skip SB
      if (!_iterator.Back(_bRing))
        return false;
    }
    // if EB found
    else if (EB_id == _iterator.GetModuleId()) {
      // and not at the beginning of the string
      if (!_iterator.Back(_bRing)) {
        return false;
      }
      if (!_iterator.FindBOB(_bRing))
        return false;
      _iterator.Back(_bRing);
    } else {
      break;
    }

    if (_iterator.Position() == lastPosition)
      break;
  }

  return (moduleId == _iterator.GetModuleId());
}
