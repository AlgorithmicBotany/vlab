/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "warningset.h"

#include <windows.h>
#include <string.h>

#include "msgrow.h"

void MsgRow::SetMsg(const char *str) {
  if ('\x1B' == str[0]) {
    switch (str[1]) {
    case 1:
      _LineMode = lmNormal;
      break;
    case 2:
      _LineMode = lmError;
      break;
    default:
      _LineMode = lmNormal;
    }
    str += 2;
  }
  strncpy(_str, str, MaxStrLen);
  _len = strlen(_str);
}

void MsgRow::Append(const char *str, int n) {
  if ('\x1B' == str[0]) {
    switch (str[1]) {
    case 1:
      _LineMode = lmNormal;
      break;
    case 2:
      _LineMode = lmError;
      break;
    default:
      _LineMode = lmNormal;
    }
    str += 2;
    n -= 2;
  }
  int roomleft = MaxStrLen - _len;
  if (n > roomleft)
    n = roomleft;
  strncat(_str, str, n);
  _len += n;
  _str[_len] = 0;
}

void MsgRow::Append(const char *str) {
  if ('\x1B' == str[0]) {
    switch (str[1]) {
    case 1:
      _LineMode = lmNormal;
      break;
    case 2:
      _LineMode = lmError;
      break;
    default:
      _LineMode = lmNormal;
    }
    str += 2;
  }
  int roomleft = MaxStrLen - _len;
  strncat(_str, str, roomleft);
  _str[MaxStrLen - 1] = 0;
  _len = strlen(_str);
}

COLORREF MsgRow::TextColor() const {
  switch (_LineMode) {
  case lmNormal:
    return RGB(255, 255, 255);
  case lmError:
    return RGB(255, 255, 255);
  }
  return RGB(255, 255, 255);
}

COLORREF MsgRow::BgColor() const {
  switch (_LineMode) {
  case lmNormal:
    return RGB(0, 0, 0);
  case lmError:
    return RGB(255, 0, 0);
  }
  return RGB(0, 0, 0);
}
