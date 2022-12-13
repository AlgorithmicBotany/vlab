/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "warningset.h"

#include <assert.h>
#include <windows.h>

#include "drawparam.h"
#include "wininterface.h"
#include "mainwindow.h"
#include "app.h"

extern App app;
MainWindow mainWindow;

HMENU hCntxtMenus = NULL;

extern "C" void init_window();
void WInitializeGraphics() {
  InitializeMenus();
  init_window();
  mainWindow.InitGraphics(app.HInst());
  mainWindow.Run();
}

void SetWindowsIdleFunc(IdleFun f) { mainWindow.SetIdleFunc(f); }

void WinSwapBuffers() { mainWindow.SwapBuffers(); }

void WinFreeGraphics() {
  // Everything will be freed when handling WM_DESTROY message
}

void WinGetMousePosition(int *pXPos, int *pYPos) {
  mainWindow.GetMousePos(pXPos, pYPos);
}

void WinMakeRasterFont(unsigned int *flb, const DRAWPARAM *pDrp) {
  *flb = mainWindow.MakeRasterFont(pDrp);
}
