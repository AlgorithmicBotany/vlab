/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __CONTEXTSCANNER_H__
#define __CONTEXTSCANNER_H__

class ContextScanner {
public:
  ContextScanner(const LEngine &lengine, LstringIterator &iterator, bool bRing,
                 int iConsiderGroup);

  virtual bool Confirm() = 0;
  virtual bool Accept(__lc_ModuleIdType moduleId) = 0;
  virtual bool Advance() = 0;
  virtual bool Reset() = 0;
  virtual __lc_ModuleIdType CurrentModuleId() const = 0;
  virtual const char *Address() const = 0;

protected:
  const bool _bRing;
  const LEngine &_lengine;
  LstringIterator &_iterator;
  LIteratorReset _reset;

  int ConsiderGroup() const { return _iConsiderGroup; }

private:
  const int _iConsiderGroup;
};

class RightContextScanner : public ContextScanner {
public:
  RightContextScanner(const LEngine &lengine, LstringIterator &iterator,
                      bool bRing, int iConsiderGroup);

  bool Confirm() {
    _reset.SetPosition();
    return true;
  }

  bool Accept(__lc_ModuleIdType moduleId);
  bool Reset() {
    _reset.Reset();
    return true;
  }

  __lc_ModuleIdType CurrentModuleId() const { return _iterator.GetModuleId(); }

  const char *Address() const { return _iterator.Ptr(); }

  bool Advance() {
    ASSERT(!_iterator.AtEnd());
    ++_iterator;
    return true;
  }

private:
};

class LeftContextScanner : public ContextScanner {
public:
  LeftContextScanner(const LEngine &lengine, LstringIterator &iterator,
                     bool bRing, int iConsiderGroup);

  bool Confirm() {
    _reset.SetPosition();
    return true;
  }

  bool Accept(__lc_ModuleIdType moduleId);
  bool Reset() {
    _reset.Reset();
    return true;
  }

  __lc_ModuleIdType CurrentModuleId() const { return _iterator.GetModuleId(); }

  const char *Address() const { return _iterator.Ptr(); }

  bool Advance() { return true; }
};

extern RightContextScanner *pgRContextScanner;
extern LeftContextScanner *pgLContextScanner;
extern RightContextScanner *pgRNContextScanner;
extern LeftContextScanner *pgLNContextScanner;

#endif
