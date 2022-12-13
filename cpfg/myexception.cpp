/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "warningset.h"

#include <windows.h>
#include <stdio.h>

#include "myexception.h"

MyException::MyException(const char *msg) {
  _msg = msg;
  _errCode = GetLastError();
}

void MyException::Display(HWND hwnd) {
  char bf[128];
  sprintf(bf, "%s\nError code is %d", _msg, _errCode);
  MessageBox(hwnd, bf, "Error !", MB_ICONSTOP);
}
