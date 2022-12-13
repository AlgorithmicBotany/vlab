/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifdef WIN32
#endif

#ifndef WIN32
#error This file to be compiled in MS Windows version only
#endif

#include "warningset.h"

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>

#include "msglogwnd.h"
#include "resource.h"
#include "canvas.h"
#include "msgrow.h"
#include "drawparam.h"
#include "wininterface.h"
#include "mainwindow.h"

extern MainWindow mainWindow;

extern "C" void AddMsg(const char *);

MsgLogWnd *pLog = NULL;

void AddMsg(const char *msg) {
  if (NULL != pLog)
    pLog->AddMessage(msg);
}

void MsgLogWnd::Register(HINSTANCE hInst) {
  WNDCLASS wc;
  {
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MsgLogWnd::WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LOG));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = LOGClassName;
  }
  RegisterClass(&wc);
}

LRESULT CALLBACK MsgLogWnd::WndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam) {
  MsgLogWnd *pSelf =
      reinterpret_cast<MsgLogWnd *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  switch (msg) {
    HANDLE_MSG(hwnd, WM_CREATE, MsgLogWnd::OnCreate);
    HANDLE_MSG(hwnd, WM_DESTROY, MsgLogWnd::OnDestroy);
    HANDLE_MSG(hwnd, WM_PAINT, pSelf->Paint);
    HANDLE_MSG(hwnd, WM_SIZE, pSelf->Size);
    HANDLE_MSG(hwnd, WM_CLOSE, pSelf->Close);
    HANDLE_MSG(hwnd, WM_VSCROLL, pSelf->VScroll);
    HANDLE_MSG(hwnd, WM_CONTEXTMENU, pSelf->ContextMenu);
    HANDLE_MSG(hwnd, WM_COMMAND, pSelf->Command);
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

BOOL MsgLogWnd::OnCreate(HWND hwnd, CREATESTRUCT *) {
  MsgLogWnd *pNew = new MsgLogWnd(hwnd);
  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pNew));
  return true;
}

void MsgLogWnd::OnDestroy(HWND hwnd) {
  MsgLogWnd *pSelf =
      reinterpret_cast<MsgLogWnd *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  delete pSelf;
  SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
}

MsgLogWnd::MsgLogWnd(HWND hwnd) : _hWnd(hwnd) {
  assert(NULL == pLog);
  pLog = this;
  _arr = new MsgRow[MaxRows];
  _lines = 0;
  _topLine = 0;
  _hFont = (HFONT)GetStockObject(ANSI_FIXED_FONT);
  {
    UpdateCanvas cnv(_hWnd);
    SelectGDIObject sf(cnv, _hFont);
    TEXTMETRIC tm;
    GetTextMetrics(cnv, &tm);
    _lineheight = tm.tmHeight + tm.tmExternalLeading;
  }

  {
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = _lines * _lineheight;
    si.nPos = 0;
    SetScrollInfo(_hWnd, SB_VERT, &si, true);
  }
}

MsgLogWnd::~MsgLogWnd() {
  delete[] _arr;
  pLog = NULL;
}

void MsgLogWnd::Size(HWND, UINT, int, int y) {
  _rowsPerPage = y / _lineheight;
  SCROLLINFO si;
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
  si.nPage = _rowsPerPage;
  si.nMin = 0;
  si.nMax = _lines;
  SetScrollInfo(_hWnd, SB_VERT, &si, true);
  int tline = _lines - _rowsPerPage;
  if (tline < _topLine)
    _topLine = tline;
  if (_topLine < 0)
    _topLine = 0;
}

void MsgLogWnd::Paint(HWND) {
  PaintCanvas pc(_hWnd);
  SelectGDIObject sf(pc, _hFont);
  int y = 0;
  for (int i = _topLine; i < _lines; i++) {
    SetBkColor(pc, _arr[i].BgColor());
    SetTextColor(pc, _arr[i].TextColor());
    pc.TextOut(0, y, _arr[i].Str(), _arr[i].Len());
    y += _lineheight;
  }
}

void MsgLogWnd::AddMessage(const char *msg) {
  if (_lines < MaxRows) {
    const char *eol = strchr(msg, '\n');
    while (NULL != eol) {
      _arr[_lines].Append(msg, eol - msg);
      _lines++;
      if (MaxRows == _lines)
        _ShiftRowsUp();
      msg = eol + 1;
      eol = strchr(msg, '\n');
    }
    if (0 != *msg)
      _arr[_lines].Append(msg);

    if (_lines - _rowsPerPage > _topLine)
      _topLine = _lines - _rowsPerPage;

    // Update scrollbar info
    {
      SCROLLINFO si;
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE | SIF_DISABLENOSCROLL;
      si.nMin = 0;
      si.nMax = _lines;
      si.nPage = _rowsPerPage;
      si.nPos = _topLine;
      SetScrollInfo(_hWnd, SB_VERT, &si, true);
    }
    InvalidateRect(_hWnd, NULL, true);
  }
}

void MsgLogWnd::_ShiftRowsUp() {
  for (int i = 0; i < MaxRows - 1; i++)
    _arr[i] = _arr[i + 1];
  _arr[MaxRows - 1].Clear();
  _lines--;
}

void MsgLogWnd::Close(HWND hwn) {
  SendMessage(mainWindow.GetHwnd(), WM_CLOSE, 0, 0);
}

void MsgLogWnd::AdjustSize(HWND hMain) {
  WINDOWPLACEMENT wp;
  wp.length = sizeof(WINDOWPLACEMENT);
  GetWindowPlacement(hMain, &wp);
  if ((SW_RESTORE == wp.showCmd) || (SW_SHOWNORMAL == wp.showCmd)) {
    RECT r;
    r.left = wp.rcNormalPosition.left;
    r.right = wp.rcNormalPosition.right - r.left;
    r.top = wp.rcNormalPosition.bottom;
    r.bottom = 120;
    MoveWindow(r);
  }
}

void MsgLogWnd::VScroll(HWND, HWND, UINT code, int pos) {
  SCROLLINFO si;
  si.cbSize = sizeof(SCROLLINFO);
  si.fMask = SIF_POS;
  switch (code) {
  case SB_LINEUP:
    if (_topLine > 0) {
      _topLine--;
    } else
      return;
    break;
  case SB_LINEDOWN:
    if (_topLine < _lines - _rowsPerPage) {
      _topLine++;
    } else
      return;
    break;
  case SB_PAGEDOWN:
    if (_topLine < _lines - _rowsPerPage) {
      _topLine += _rowsPerPage;
      if (_topLine >= _lines - _rowsPerPage)
        _topLine = _lines - _rowsPerPage - 1;
    } else
      return;
    break;
  case SB_PAGEUP:
    if (_topLine > 0) {
      _topLine -= _rowsPerPage;
      if (_topLine < 0)
        _topLine = 0;
    } else
      return;
    break;
  case SB_THUMBTRACK:
  case SB_THUMBPOSITION:
    _topLine = pos;
    break;
  default:
    return;
  }
  si.nPos = _topLine;
  SetScrollInfo(_hWnd, SB_VERT, &si, true);
  InvalidateRect(_hWnd, NULL, true);
}

void MsgLogWnd::MoveWindow(const RECT &r) {
  ::MoveWindow(_hWnd, r.left, r.top, r.right, r.bottom, true);
}

void MsgLogWnd::ContextMenu(HWND, HWND, UINT x, UINT y) {
  HMENU hMenu = GetSubMenu(hCntxtMenus, 0);
  TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, x, y, 0, _hWnd, NULL);
}

void MsgLogWnd::Command(HWND, int id, HWND, UINT) {
  switch (id) {
  case ID_MSGLOGWND_COPY:
    _Copy();
    break;
  case ID_MSGLOGWND_CUT:
    _Cut();
    break;
  }
}

void MsgLogWnd::_Copy() {
  int strsz = 0;
  for (int i = 0; i < _lines; i++) {
    strsz += _arr[i].Len();
    if (i < _lines - 1)
      strsz += 2; // cr/lf at end of each but last line
  }
  BOOL res;
  char *Str = NULL;
  HGLOBAL hMem = NULL;
  int strix = 0;

  hMem = GlobalAlloc(GHND, strsz + 1);
  if (NULL == hMem)
    goto fail;

  Str = (char *)GlobalLock(hMem);

  for (int i = 0; i < _lines; i++) {
    strcpy(Str + strix, _arr[i].Str());
    strix += _arr[i].Len();
    if (i < _lines - 1) {
      strcpy(Str + strix, "\r\n");
      strix += 2;
    }
  }
  GlobalUnlock(hMem);
  Str = NULL;

  res = OpenClipboard(_hWnd);

  if (!(res))
    goto fail;

  EmptyClipboard();

  SetClipboardData(CF_TEXT, hMem);
  hMem = NULL;
  CloseClipboard();
fail:
  if (NULL != Str)
    GlobalUnlock(Str);
  if (NULL != hMem)
    GlobalFree(hMem);
}

void MsgLogWnd::_Cut() {
  _Copy();
  _Clear();
}

void MsgLogWnd::_Clear() {
  for (int i = 0; i < _lines; i++)
    _arr[i].Clear();
  _lines = 0;
  _topLine = 0;
  {
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_POS | SIF_RANGE | SIF_DISABLENOSCROLL;
    si.nMin = 0;
    si.nMax = _lines * _lineheight;
    si.nPos = 0;
    SetScrollInfo(_hWnd, SB_VERT, &si, true);
  }
  InvalidateRect(_hWnd, NULL, true);
}
