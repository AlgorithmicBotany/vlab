/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "warningset.h"

#pragma warning(disable : 4189)
#pragma warning(disable : 4511)
#pragma warning(disable : 4512)

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <fstream>

#include <windows.h>
#include <windowsx.h>

#include "drawparam.h"
#include "wininterface.h"
#include "app.h"
#include "classnames.h"
#include "wndproc.h"
#include "dynarray.h"
#include "resource.h"
#include "msglogwnd.h"
#include "log.h"

#include "../Lstudio/cmndefs.h"

#include <scramble.h>

extern "C" int main(int, char **);

void App::Init(HINSTANCE hInst) {
  _hInst = hInst;
  assert(NULL != _hInst);
  WNDCLASS wc;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = _hInst;
  wc.hIcon = LoadIcon(_hInst, MAKEINTRESOURCE(IDI_ICONL));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = CPFGClassName;

  if (!RegisterClass(&wc))
    throw("Error registering");

  MsgLogWnd::Register(hInst);
  CreateWindow(LOGClassName, "Message Log", WS_OVERLAPPEDWINDOW | WS_VSCROLL,
               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL,
               NULL, _hInst, NULL);
}

char OrigDir[_MAX_PATH + 1];
char LabTable[_MAX_PATH + 1];

extern "C" void CleanLabTable() {
  WIN32_FIND_DATA fd;
  HANDLE hF = FindFirstFile("*.*", &fd);
  while (hF != INVALID_HANDLE_VALUE) {
    if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      DeleteFile(fd.cFileName);
    if (!FindNextFile(hF, &fd)) {
      FindClose(hF);
      hF = INVALID_HANDLE_VALUE;
    }
  }
  SetCurrentDirectory(OrigDir);
  RemoveDirectory(LabTable);
}

void _BuildTmpDir() {
  GetCurrentDirectory(_MAX_PATH, OrigDir);
  char tmpth[_MAX_PATH + 1];
  GetTempPath(_MAX_PATH + 1, tmpth);
  int i;
  for (i = 0; i < 1000; ++i) {
    sprintf(LabTable, "%sgrdk%x.tmp", tmpth, i);
    if (CreateDirectory(LabTable, NULL))
      break;
  }
  if (1000 == i)
    throw "Error reading ldm file";

  SetCurrentDirectory(LabTable);
}

char *_FirstSpace(char *p) {
  while (!isspace(*p) && (*p != 0))
    ++p;
  if (0 == *p)
    return NULL;
  else
    return p;
}

char *_FirstNonSpace(char *p) {
  while (isspace(*p))
    ++p;
  return p;
}

int App::Run(char *cmd) {
  if (PrjVar::IsEvalVer())
    Message("Demo version\n");

  if (PrjVar::CpfgViewer) {
    if (__argc != 2) {
      MessageBox(NULL, "Incorrect number of command line parameters", "Error",
                 MB_ICONSTOP);
      return 0;
    }

    {
      FileDescrambler fd(__argv[1]);
      _BuildTmpDir();

      fd.Extract();
    }

    static char cmndln[4096];

    {
      FILE *fp = fopen("cmndln.txt", "rt");
      int ix = 0;
      for (;;) {
        int c = fgetc(fp);
        if (EOF == c) {
          cmndln[ix] = 0;
          break;
        } else {
          cmndln[ix] = c;
          ++ix;
        }
      }

      fclose(fp);
    }

    char *argv[256];
    int argc = 0;
    {
      char *pX = cmndln;
      while (*pX != 0) {
        argv[argc] = pX;
        ++argc;
        pX = _FirstSpace(pX);
        if (NULL == pX)
          break;
        *pX = 0;
        ++pX;
        pX = _FirstNonSpace(pX);
      }
    }

    return main(argc, argv);
  } else
    return main(__argc, __argv);
}

BOOL CALLBACK App::FilesDlgProc(HWND hwnd, UINT msg, WPARAM wParam,
                                LPARAM lParam) {
  static App *pSelf;
  switch (msg) {
  case WM_INITDIALOG:
    pSelf = (App *)lParam;
    pSelf->InitFileDlg(hwnd);
    return true;
  case WM_COMMAND:
    HANDLE_WM_COMMAND(hwnd, wParam, lParam, pSelf->FileDlgCommand);
    return true;
  }
  return false;
}

void App::InitFileDlg(HWND hwnd) {
  _hFileDlg = hwnd;
  _hLSystemTxt = GetDlgItem(_hFileDlg, IDC_LSYSTEM);
  _hViewTxt = GetDlgItem(_hFileDlg, IDC_VIEW);
  _hMapMatTxt = GetDlgItem(_hFileDlg, IDC_MATMAP);
}

void App::FileDlgCommand(HWND, int id, HWND, UINT) {
  switch (id) {
  case IDCANCEL:
  case IDOK:
    EndDialog(_hFileDlg, id);
    break;
  }
}
