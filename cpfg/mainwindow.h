/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

class View;

class MainWindow {
public:
  MainWindow();
  bool Create(HWND, CREATESTRUCT *);
  void Destroy();
  void InitGraphics(HINSTANCE);
  int Run();
  void Size(HWND, UINT, int, int);
  void Paint(HWND);
  void MouseMove(HWND, int, int, UINT);
  void LButtonDown(HWND, BOOL, int, int, UINT);
  void MButtonDown(HWND, BOOL, int, int, UINT);
  void RButtonDown(HWND, BOOL, int, int, UINT);
  void ButtonUp(HWND, int, int, UINT);
  void OnKey(HWND, UINT, BOOL, int, UINT);
  void Move(HWND, int, int);
  void Command(HWND, int, HWND, UINT);
  void Show(HWND, BOOL, UINT);
  void GetMousePos(int *, int *);
  void SetIdleFunc(IdleFun);
  void SwapBuffers();
  unsigned int MakeRasterFont(const DRAWPARAM *);
  HWND GetHwnd() const { return _hWnd; }
  void SelectOutputDir();

private:
  void _UpdateMsgLog(UINT);

  HACCEL _hAccel;
  HWND _hWnd;
  View *_pView;
  IdleFun _pIdleFunc;
  HMENU _hMainMenu;
  HMENU _hFullMenu;
  HMENU _hAnimateMenu;
  HMENU _hFullAnimate;
  HMENU _hPopupMain;
  HMENU _hPopupAnim;
  HMENU _hHidden;
  HMENU _hFM, _hFA;
  POINT _mousePos;
  bool _logVisible;
};

#else
#error File already included
#endif
