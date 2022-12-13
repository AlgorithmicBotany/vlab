/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __MYEXCEPTION_H__
#define __MYEXCEPTION_H__

class MyException {
public:
  MyException(const char *msg);

  void Display(HWND hwnd = NULL);

private:
  const char *_msg;
  DWORD _errCode;
};

class FatalError : public MyException {
public:
  FatalError(const char *msg) : MyException(msg) {}
};

#else
#error File already included
#endif
