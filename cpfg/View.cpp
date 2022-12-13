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
#include <string.h>
#include <windows.h>

#include <GL/gl.h>

#include "canvas.h"
#include "drawparam.h"
#include "view.h"
#include "myexception.h"
#include "comlineparam.h"

class BeginGL {
public:
  BeginGL(GLenum mode) {
    assert(0 == _counter);
    glBegin(mode);

#ifndef NDEBUG
    _counter++;
#endif
  }
  ~BeginGL() {
#ifndef NDEBUG
    _counter--;
#endif
    glEnd();
  }

private:
#ifndef NDEBUG
  static int _counter;
#endif
};

#ifndef NDEBUG
int BeginGL::_counter = 0;
#endif

View::View(HWND hwnd) : _hwnd(hwnd), _cnv(_hwnd) {
  PIXELFORMATDESCRIPTOR pfd;
  {
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags =
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL; //| PFD_GENERIC_ACCELERATED;
    pfd.dwFlags |= clp.doublebuffer ? PFD_DOUBLEBUFFER : 0;
    pfd.iPixelType = (BYTE)((clp.iscolormapfile || clp.ismaterialfile)
                                ? PFD_TYPE_RGBA
                                : PFD_TYPE_COLORINDEX);
    pfd.cColorBits = 24;
    pfd.cRedBits = 0;
    pfd.cRedShift = 0;
    pfd.cGreenBits = 0;
    pfd.cGreenShift = 0;
    pfd.cBlueBits = 0;
    pfd.cBlueShift = 0;
    pfd.cAlphaBits = 16;
    pfd.cAlphaShift = 0;
    pfd.cAccumBits = 0;
    pfd.cAccumRedBits = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits = 0;
    pfd.cAccumAlphaBits = 0;
    pfd.cDepthBits = 16;
    pfd.cStencilBits = 0;
    pfd.cAuxBuffers = 0;
    pfd.iLayerType = PFD_MAIN_PLANE;
    pfd.bReserved = 0;
    pfd.dwLayerMask = 0;
    pfd.dwVisibleMask = 0;
    pfd.dwDamageMask = 0;
  }

  int nMyPixelFormatID;
  nMyPixelFormatID = ChoosePixelFormat(_cnv, &pfd);

  if (0 == nMyPixelFormatID)
    throw MyException("Cannot choose pixel format");

  SetPixelFormat(_cnv, nMyPixelFormatID, &pfd);

  _hRC = wglCreateContext(_cnv);
  if (NULL == _hRC)
    throw MyException("Cannot create RC");

  wglMakeCurrent(_cnv, _hRC);
}

View::~View() {
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(_hRC);
}

extern "C" void Reshape(int, int);
void View::Size(int x, int y) { Reshape(x, y); }

extern "C" void draw_scene_callback();
void View::Paint() {
  ValidateRect(_hwnd, NULL);
  draw_scene_callback();
}

extern "C" int oflist;

unsigned int View::MakeRasterFont(const DRAWPARAM *pDP) {
  unsigned int toRet = glGenLists(256);
  if (0 == toRet)
    return 0;

  try {
    Font newfont(pDP->winfont.Name, pDP->winfont.size, pDP->winfont.italic != 0,
                 pDP->winfont.bold != 0);
    SelectGDIObject selectfont(_cnv, newfont);
    wglUseFontBitmaps(_cnv, 0, 256, toRet);
  } catch (MyException) {
    wglUseFontBitmaps(_cnv, 0, 256, toRet);
  }

  {
    oflist = glGenLists(128);
    Font arial("Arial", 12, true, false);
    SelectGDIObject selectfont(_cnv, arial);
    wglUseFontOutlines(_cnv, 0, 128, oflist, 0.0f, 0.1f, WGL_FONT_LINES, 0);
  }

  return toRet;
}
