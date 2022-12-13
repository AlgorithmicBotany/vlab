/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __VIEW_H__
#define __VIEW_H__

class View {
public:
  View(HWND);
  ~View();
  void Size(int, int);
  void Paint();
  void SwapBuffers() { ::SwapBuffers(_cnv); }
  unsigned int MakeRasterFont(const DRAWPARAM *);

private:
  HWND _hwnd;
  UpdateCanvas _cnv;
  HGLRC _hRC;
};

#else
#error File already included
#endif
