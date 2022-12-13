/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "warningset.h"

#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <shlobj.h>

#include "drawparam.h"
#include "wininterface.h"
#include "mainwindow.h"
#include "canvas.h"
#include "view.h"
#include "classnames.h"
#include "resource.h"
#include "msglogwnd.h"
#include "myexception.h"
#include "control.h"
#include "comlineparam.h"
#include "../Lstudio/cmndefs.h"

extern "C" HWND hLProject;
extern "C" int popupmenuon;
extern "C" HWND hMain = NULL;
extern "C" HANDLE hLSemaphore;

static bool _LoadWindowPlacement(WINDOWPLACEMENT *);

MainWindow::MainWindow() {
  _hWnd = NULL;
  _pIdleFunc = NULL;
  popupmenuon = 0;
  _logVisible = false;
}

bool MainWindow::Create(HWND hwnd, CREATESTRUCT *pCS) {
  try {
    _hWnd = hwnd;
    _pView = new View(_hWnd);
    UINT mmenu =
#ifdef VIEWER
        IDR_VIEWERMAIN
#else
        IDR_MAIN
#endif
        ;
    _hMainMenu = LoadMenu(pCS->hInstance, MAKEINTRESOURCE(mmenu));
    _hFM = LoadMenu(pCS->hInstance, MAKEINTRESOURCE(IDR_MAINORIG));
    _hFullMenu = GetSubMenu(_hFM, 0);
    _hPopupMain = GetSubMenu(_hMainMenu, 0);
    UINT amenu =
#ifdef VIEWER
        IDR_VIEWERANIMATE
#else
        IDR_ANIMATE
#endif
        ;
    _hAnimateMenu = LoadMenu(pCS->hInstance, MAKEINTRESOURCE(amenu));
    _hFA = LoadMenu(pCS->hInstance, MAKEINTRESOURCE(IDR_ANIMATEORIG));
    _hFullAnimate = GetSubMenu(_hFA, 0);
    _hPopupAnim = GetSubMenu(_hAnimateMenu, 0);
    hCntxtMenus = LoadMenu(pCS->hInstance, MAKEINTRESOURCE(IDR_CONTEXTMENUS));
    _hHidden = LoadMenu(pCS->hInstance, MAKEINTRESOURCE(IDR_HIDDEN));
    hMain = _hWnd;
    return true;
  } catch (MyException e) {
    e.Display(_hWnd);
    return false;
  }
}

void MainWindow::Destroy() {
  DestroyMenu(_hHidden);
  DestroyMenu(_hFA);
  DestroyMenu(_hAnimateMenu);
  DestroyMenu(_hFM);
  DestroyMenu(_hMainMenu);
  DestroyMenu(hCntxtMenus);
  delete _pView;
  {
    HKEY hKey;
    DWORD disp;
    WINDOWPLACEMENT wp;
    LONG res =
        RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\RadekSoftware\\cpfg", 0,
                       NULL, 0, KEY_SET_VALUE, NULL, &hKey, &disp);

    if (ERROR_SUCCESS != res)
      return;
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hMain, &wp);
    res = RegSetValueEx(hKey, "WindowPlacement", 0, REG_BINARY,
                        (const BYTE *)&wp, sizeof(WINDOWPLACEMENT));
    RegCloseKey(hKey);
  }
  hMain = NULL;
}

void MainWindow::InitGraphics(HINSTANCE hInst) {
  WINDOWPLACEMENT wp;
  DWORD wstyle = WS_OVERLAPPEDWINDOW;
  if (clp.noborder)
    wstyle = WS_BORDER | WS_POPUP;
  RECT isize;
  if (_LoadWindowPlacement(&wp)) {
    isize.left = wp.rcNormalPosition.left;
    isize.top = wp.rcNormalPosition.top;
    isize.right /* actually width */ = wp.rcNormalPosition.right - isize.left;
    isize.bottom /* actually height */ = wp.rcNormalPosition.bottom - isize.top;
  } else {
    isize.left = CW_USEDEFAULT;
    isize.right = CW_USEDEFAULT;
    isize.top = CW_USEDEFAULT;
    isize.bottom = CW_USEDEFAULT;
  }

  if (clp.xsize != -1 || clp.xpos != -1) {
    RECT r;
    r.left = 50;
    r.top = 50;
    if (clp.xpos != -1) {
      isize.left = r.left = clp.xpos;
      isize.top = r.top = clp.ypos;
    }
    r.right = r.left + clp.xsize;
    r.bottom = r.top + clp.ysize;
    if (clp.compensateforframe)
      AdjustWindowRect(&r, wstyle, false);
    isize.right = r.right - r.left;
    isize.bottom = r.bottom - r.top;
  }

  HWND _hWnd =
      CreateWindow(CPFGClassName, clp.titlename, wstyle, isize.left, isize.top,
                   isize.right, isize.bottom, NULL, NULL, hInst, NULL);

  {
    HANDLE hFile = CreateFile(".\\xwnds", GENERIC_READ | GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                              OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE != hFile) {
      DWORD aux;
      char name[16];
      ReadFile(hFile, &hLProject, sizeof(HWND), &aux, NULL);
      WriteFile(hFile, &_hWnd, sizeof(HWND), &aux, NULL);
      CloseHandle(hFile);
      HANDLE hSemaphore =
          OpenSemaphore(SEMAPHORE_MODIFY_STATE, false, "CpfgStarted");
      assert(NULL != hSemaphore);
      assert(sizeof(unsigned int) == sizeof(HWND));
      sprintf(name, "Mtx%08x", (unsigned int)hLProject);
      hLSemaphore = OpenSemaphore(SEMAPHORE_MODIFY_STATE, FALSE, name);
      ReleaseSemaphore(hSemaphore, 1, NULL);
      CloseHandle(hSemaphore);
    } else {
      hLProject = NULL;
      hLSemaphore = NULL;
    }
  }
  ShowWindow(_hWnd, SW_SHOWNORMAL);
  UpdateWindow(_hWnd);

  _hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ACCELERATOR1));
}

int MainWindow::Run() {
  MSG msg;
  for (;;) {
    if (NULL != _pIdleFunc) {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (WM_QUIT == msg.message)
          return msg.wParam;
#ifndef VIEWER
        if (!TranslateAccelerator(_hWnd, _hAccel, &msg))
#endif
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      } else {
        if (_pIdleFunc(NULL))
          _pIdleFunc = NULL;
      }
    } else {
      if (GetMessage(&msg, NULL, 0, 0)) {
#ifndef VIEWER
        if (!TranslateAccelerator(_hWnd, _hAccel, &msg))
#endif
        {
          TranslateMessage(&msg);
          DispatchMessage(&msg);
        }
      } else
        return msg.wParam;
    }
  }
}

void MainWindow::Move(HWND, int, int) { _UpdateMsgLog(0); }

void MainWindow::Size(HWND, UINT state, int w, int h) {
  if ((0 != w) || (0 != h))
    _pView->Size(w, h);
  _UpdateMsgLog(state);
}

void MainWindow::Paint(HWND) { _pView->Paint(); }

void MainWindow::SetIdleFunc(IdleFun f) { _pIdleFunc = f; }

void MainWindow::SwapBuffers() { _pView->SwapBuffers(); }

void MainWindow::MouseMove(HWND, int x, int y, UINT) {
  _mousePos.x = x;
  _mousePos.y = y;
}

void MainWindow::GetMousePos(int *x, int *y) {
  *x = _mousePos.x;
  *y = _mousePos.y;
}

extern "C" void OnSKeyDown(void);
void MainWindow::OnKey(HWND, UINT vk, BOOL fDown, int, UINT) {
  if (!fDown)
    return;
  switch (vk) {
  case 'S':
    OnSKeyDown();
    break;
  }
}

extern "C" void OnLButtonDown(int, int);
extern "C" void OnMButtonDown(int, int);
extern "C" void StartPan(int, int);
extern "C" void RereadFunctionsRerunNoSync();

extern "C" void InsertX(int, int);

void MainWindow::LButtonDown(HWND, BOOL, int x, int y, UINT flags) {
  SetCapture(_hWnd);
  if ((MK_CONTROL | MK_SHIFT) == ((MK_CONTROL | MK_SHIFT) & flags))
    InsertX(x, y);
  else if (MK_CONTROL & flags)
    OnMButtonDown(x, y);
  else if (MK_SHIFT & flags)
    StartPan(x, y);
  else
    OnLButtonDown(x, y);
}

void MainWindow::MButtonDown(HWND, BOOL, int x, int y, UINT) {
  SetCapture(_hWnd);
  OnMButtonDown(x, y);
}

extern "C" void OnRButtonDown(int, int);
extern "C" int animateMenu;

void MainWindow::RButtonDown(HWND, BOOL, int x, int y, UINT) {
  POINT p;
  p.x = x;
  p.y = y;
  ClientToScreen(_hWnd, &p);
  popupmenuon = 1;
  HMENU hM = 0;
  if (0 == hLProject && animateMenu)
    hM = _hFullAnimate;
  else if (0 == hLProject) // && !animateMenu
    hM = _hFullMenu;
  else if (animateMenu) // && 0!=hLProject
    hM = _hPopupAnim;
  else // !animateMenu && 0!=hLProject
    hM = _hPopupMain;

  if (!animateMenu) {
    UINT enbl = (clp.afilename == NULL) ? MF_GRAYED : MF_ENABLED;
    EnableMenuItem(hM, ID_MAINMENU_ANIMATEMODE, enbl);
  }
  TrackPopupMenu(hM, TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, _hWnd, NULL);
  popupmenuon = 0;
}

extern "C" void OnButtonUp(void);
void MainWindow::ButtonUp(HWND, int, int, UINT) {
  ReleaseCapture();
  OnButtonUp();
}

extern "C" void do_it(int);
extern "C" void save_file(int);
extern "C" void SaveAsBmp(const char *);
extern "C" void SaveViewVolume();
extern "C" void RereadColors();
extern "C" void RereadContours();
extern "C" void RereadSurfaces();
extern "C" void Rerun();
extern "C" void BeginRecording();
extern "C" void RereadFunctionsRerun();
extern "C" void RereadCurveXYZRerun();
extern "C" void NewProjection();
extern "C" void NewRender();
extern "C" void RestoreView();

void MainWindow::Command(HWND, int id, HWND, UINT) {
  switch (id) {
  case PrjVar::ccNewModel:
  case ID_MAINMENU_NEWMODEL:
    pLog->SetForeground();
    SetForegroundWindow(_hWnd);
    do_it(11);
    break;
  case PrjVar::ccNewLsystem:
  case ID_MAINMENU_NEWLSYSTEM:
    do_it(12);
    break;
  case PrjVar::ccNewHomomorphism:
  case ID_MAINMENU_NEWHOMOMORPHISM:
    do_it(16);
    break;
  case PrjVar::ccNewView:
  case ID_MAINMENU_NEWVIEW:
    do_it(13);
    break;
  case ID_MAINMENU_ANIMATEMODE:
    do_it(20);
    break;
  case ID_MAINMENU_EXIT:
    do_it(99);
    break;
  case ID_ANIMATE_STEP:
    do_it(21);
    break;
  case ID_ANIMATE_RUN:
    do_it(22);
    break;
  case ID_ANIMATE_FOREVER:
    do_it(23);
    break;
  case ID_ANIMATE_STOP:
    do_it(24);
    break;
  case ID_ANIMATE_REWIND:
    do_it(25);
    break;
  case ID_ANIMATE_CLEAR:
    do_it(26);
    break;
  case ID_ANIMATE_DONTANIMATE:
    do_it(27);
    break;
  case PrjVar::ccNewAnimate:
  case ID_MENUITEM40044:
    do_it(14);
    break;
  case ID_MAINMENU_OUTPUT_IMAGE_RGB_SAVEASRGB:
    save_file(1);
    break;
  case ID_MAINMENU_OUTPUT_RAYSHADE_SAVEASRAS:
    save_file(2);
    break;
  case ID_MAINMENU_OUTPUT_IMAGE_TGA_SAVEASTGA:
    save_file(3);
    break;
  case ID_MAINMENU_OUTPUT_IMAGE_RLE_SAVEASRLE:
    save_file(4);
    break;
  case ID_MAINMENU_OUTPUT_IMAGE_RAYSHADE_SAVEASRAY:
    save_file(5);
    break;
  case ID_MAINMENU_OUTPUT_POSTSCRIPT_SAVEASPS:
    save_file(6);
    break;
  case ID_MAINMENU_OUTPUT_STRING_TEXT_SAVEASSTR:
    save_file(7);
    break;
  case ID_MAINMENU_OUTPUT_IMAGE_BMP_SAVEASBMP:
    Redraw(rfClear);
    SaveAsBmp("output.bmp");
    break;
  case ID_MAINMENU_OUTPUT_VIEWVOLUME_SAVEASVOL:
    SaveViewVolume();
    break;
  case ID_OBJ_:
    OutputObj();
    break;
  case ID_BEGINRECORDING:
    BeginRecording();
    break;
  case ID_STOPRECORDING:
    SelectInMenu(98);
    break;
  case PrjVar::ccRereadColors:
    RereadColors();
    break;
  case PrjVar::ccRereadContours:
    RereadContours();
    break;
  case PrjVar::ccRereadSurfaces:
    RereadSurfaces();
    break;
  case PrjVar::ccRerun:
    Rerun();
    break;
  case PrjVar::ccRereadFunctionsRerun:
    RereadFunctionsRerun();
    break;
  case PrjVar::ccRereadCurvesRerun:
    RereadCurveXYZRerun();
    break;
  case PrjVar::ccNewProjection:
    NewProjection();
    break;
  case PrjVar::ccNewRender:
    NewRender();
    break;
  case ID_ANIMATE_RESTOREVIEW:
    RestoreView();
    break;
  case ID_MAINMENU_OUTPUT_SELECTOUTPUTDIRECTORY:
    SelectOutputDir();
    break;
  case ID_IMAGE_RGB:
    SaveRGB();
    break;
  case ID_IMAGE_TGA:
    SaveTGA();
    break;
  case ID_IMAGE_BMP:
    SaveBMP();
    break;
  case ID_OUTPUT_RAYSHADE:
    SaveRayshade();
    break;
  case ID_OUTPUT_POSTSCRIPT:
    SavePostscript();
    break;
  case ID_OUTPUT_STRING:
    SaveString();
    break;
  case ID_OUTPUT_GLSFORMAT:
    SaveGLS();
    break;
  case ID_OUTPUT_VIEWVOLUME:
    SaveVV();
    break;
  case ID_OUTPUT_OBJ:
    SaveOBJ();
    break;
  case ID_MAINMENU_INPUT_STRINGTEXT:
    InputString(clp.savefilename[SAVE_STRING], 't');
    break;
  }
}

unsigned int MainWindow::MakeRasterFont(const DRAWPARAM *pDP) {
  return _pView->MakeRasterFont(pDP);
}

void MainWindow::_UpdateMsgLog(UINT state) {
  if (NULL != pLog && !clp.nomsglog) {
    if (SIZE_MINIMIZED == state)
      ShowWindow(pLog->Handle(), SW_MINIMIZE);
    else {
      pLog->AdjustSize(_hWnd);
      if (!_logVisible) {
        ShowWindow(pLog->Handle(), SW_SHOW);
        _logVisible = true;
      }
    }
  }
}

void MainWindow::Show(HWND, BOOL show, UINT) {}

bool _LoadWindowPlacement(WINDOWPLACEMENT *pWP) {
  HKEY hK;
  LONG res = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\RadekSoftware\\cpfg", 0,
                          KEY_QUERY_VALUE, &hK);

  if (ERROR_SUCCESS != res)
    return false;

  char nm[] = "WindowPlacement";

  DWORD type = REG_BINARY;
  DWORD cbDt = sizeof(WINDOWPLACEMENT);
  res = RegQueryValueEx(hK, nm, NULL, &type, (BYTE *)pWP, &cbDt);

  RegCloseKey(hK);

  if (ERROR_SUCCESS != res)
    return false;
  return true;
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lp, LPARAM pData) {
  TCHAR szDir[MAX_PATH];

  switch (uMsg) {
  case BFFM_INITIALIZED:
    if (GetCurrentDirectory(sizeof(szDir) / sizeof(TCHAR), szDir)) {
      // WParam is TRUE since you are passing a path.
      // It would be FALSE if you were passing a pidl.
      SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)szDir);
    }
    break;
    /*
       case BFFM_SELCHANGED:
          // Set the status window to the currently selected path.
          if (SHGetPathFromIDList((LPITEMIDLIST) lp ,szDir))
          {
             SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)szDir);
          }
          break;
    */
  }
  return 0;
}

void MainWindow::SelectOutputDir() {
  BROWSEINFO bi;
  TCHAR szDir[MAX_PATH];
  LPITEMIDLIST pidl;
  LPMALLOC pMalloc;

  if (SUCCEEDED(SHGetMalloc(&pMalloc))) {
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = _hWnd;
    bi.pszDisplayName = 0;
    bi.pidlRoot = 0;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;
    bi.lpfn = BrowseCallbackProc;

    pidl = SHBrowseForFolder(&bi);
    if (pidl) {
      if (SHGetPathFromIDList(pidl, szDir)) {
        strcpy(clp.savedir, szDir);
        int l = strlen(clp.savedir);
        if ('\\' != clp.savedir[l - 1]) {
          clp.savedir[l] = '\\';
          clp.savedir[l + 1] = 0;
        }
      }

      pMalloc->Free(pidl);
      pMalloc->Release();
    }
  }
}
