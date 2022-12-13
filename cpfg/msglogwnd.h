/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __MSGLOGWND_H__
#define __MSGLOGWND_H__

extern "C" void AddMsg(const char *);

class MsgRow;

class MsgLogWnd {
public:
  static void Register(HINSTANCE);
  static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
  static BOOL OnCreate(HWND, CREATESTRUCT *);
  static void OnDestroy(HWND);
  void Size(HWND, UINT, int, int);
  void Paint(HWND);
  void Close(HWND);
  void VScroll(HWND, HWND, UINT, int);
  void ContextMenu(HWND, HWND, UINT, UINT);
  void Command(HWND, int, HWND, UINT);

  void AddMessage(const char *);

  void AdjustSize(HWND);
  void MoveWindow(const RECT &);
  void SetForeground() { SetForegroundWindow(_hWnd); }
  HWND Handle() const { return _hWnd; }

private:
  enum { MaxRows = 256 };
  MsgLogWnd(HWND);
  ~MsgLogWnd();

  const HWND _hWnd;

  MsgRow *_arr;
  int _lines;
  int _lineheight;
  int _topLine;
  int _rowsPerPage;

  void _ShiftRowsUp();

  void _Copy();
  void _Cut();
  void _Clear();

  MsgLogWnd &operator=(MsgLogWnd &);
  HFONT _hFont;
};

extern MsgLogWnd *pLog;

#define LOGClassName "CPFGLogWnd"

#else
#error File already included
#endif
