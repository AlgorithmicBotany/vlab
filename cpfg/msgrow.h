/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __MSGROW_H__
#define __MSGROW_H__

enum LineMode { lmNormal = 1, lmError };

class MsgRow {
public:
  MsgRow() {
    _str[0] = 0;
    _len = 0;
    _LineMode = lmNormal;
  }
  ~MsgRow() {}
  const char *Str() const { return _str; }
  int Len() const { return _len; }
  void SetMsg(const char *str);
  void Append(const char *, int);
  void Append(const char *);
  void Clear() {
    _str[0] = 0;
    _len = 0;
  }
  MsgRow &operator=(const MsgRow &src) {
    _len = src._len;
    strncpy(_str, src._str, _len + 1);
    return *this;
  }
  COLORREF TextColor() const;
  COLORREF BgColor() const;

private:
  enum { MaxStrLen = 100 };
  char _str[MaxStrLen];
  int _len;
  LineMode _LineMode;
};

#else
#error File already included
#endif
