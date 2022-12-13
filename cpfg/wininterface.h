/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __WININTERFACE_H__
#define __WININTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef char (*IdleFun)(void *);

void WInitializeGraphics(void);
void ErrorMsg(const char *, int);
void SetWindowsIdleFunc(IdleFun);
void WinSwapBuffers(void);
void WinFreeGraphics(void);
void WinGetMousePosition(int *, int *);
void WinMakeRasterFont(unsigned int *, const DRAWPARAM *);
void InitializeMenus(void);

extern HMENU hCntxtMenus;

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
