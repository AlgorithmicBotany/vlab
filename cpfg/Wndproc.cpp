/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "warningset.h"

#include <windows.h>
#include <windowsx.h>

#include "wndproc.h"
#include "drawparam.h"
#include "wininterface.h"
#include "mainwindow.h"

extern MainWindow mainWindow;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam,
                             LPARAM lParam) {
  switch (msg) {
    HANDLE_MSG(hwnd, WM_CREATE, mainWindow.Create);
    HANDLE_MSG(hwnd, WM_SIZE, mainWindow.Size);
    HANDLE_MSG(hwnd, WM_PAINT, mainWindow.Paint);
    HANDLE_MSG(hwnd, WM_MOUSEMOVE, mainWindow.MouseMove);
    HANDLE_MSG(hwnd, WM_LBUTTONDOWN, mainWindow.LButtonDown);
    HANDLE_MSG(hwnd, WM_MBUTTONDOWN, mainWindow.MButtonDown);
    HANDLE_MSG(hwnd, WM_RBUTTONDOWN, mainWindow.RButtonDown);
    HANDLE_MSG(hwnd, WM_LBUTTONUP, mainWindow.ButtonUp);
    HANDLE_MSG(hwnd, WM_MBUTTONUP, mainWindow.ButtonUp);
    HANDLE_MSG(hwnd, WM_COMMAND, mainWindow.Command);
    HANDLE_MSG(hwnd, WM_MOVE, mainWindow.Move);
    HANDLE_MSG(hwnd, WM_SHOWWINDOW, mainWindow.Show);

    HANDLE_MSG(hwnd, WM_KEYDOWN, mainWindow.OnKey);
  case WM_DESTROY:
    mainWindow.Destroy();
    PostQuitMessage(0);
    return 0;
  }

  return DefWindowProc(hwnd, msg, wParam, lParam);
}
