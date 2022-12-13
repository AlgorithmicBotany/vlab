/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#include <cmath>
#include <cassert>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <qgl.h>

#include "geometry.h"
#include "gridview.h"
#include "glutils.h"
#include "colors.h"

GridView::GridView(QOpenGLWidget *w)
    : _oglwidget(w), _IdleTask(this), _TranslateTask(this), _ZoomTask(this) {
  _GridDrawWhat = eDrawGrid | eDrawLabels | eDrawAxis;
  _pTask = &_IdleTask;
  _scale = 1.0;
  SetScale(1.2);
  SetCenter(0.0, 0.0, 0.0);
  ReadColors();
}

void GridView::GInit() {
  QFontMetrics fm(_oglwidget->font());
  _charHeight = fm.height();
  _minusWidth = fm.width("-");
  // Initialize a clear colour
  const GLdouble *clcl = GetColor(Background);
  glClearColor(clcl[0], clcl[1], clcl[2], clcl[3]);
  glLineWidth(GetSize(AxisWidth));
}

void GridView::Resize() { _SetView(); }

void GridView::_SetView() {
  // Note: window size is set in the inherited widget Ctrl,
  //  which handles the Qt API.
  const double wc = double(_WindowSize.x) / double(_WindowSize.y);

  // [PASCAL] to change the clip plans replace 1 by 100
  if (wc > 1.0) {
    _MinPoint.Set(_center.X() - _scale * wc, _center.Y() - _scale, -100.0);
    _MaxPoint.Set(_center.X() + _scale * wc, _center.Y() + _scale, 100.0);
    _upp = (2.0 * _scale) / _WindowSize.y;
  } else {
    _MinPoint.Set(_center.X() - _scale, _center.Y() - _scale / wc, -100.0);
    _MaxPoint.Set(_center.X() + _scale, _center.Y() + _scale / wc, 100.0);
    _upp = (2.0 * _scale) / _WindowSize.x;
  }

  // Y
  
  int numoflabels = _WindowSize.y / (3 * _charHeight);
  double yrange = _MaxPoint.Y() - _MinPoint.Y();
  assert(yrange > 0.0);
  double step = yrange / numoflabels;
  double relstep = step;
  while (relstep > 10.0)
    relstep /= 10.0;
  while (relstep < 1.0)
    relstep *= 10.0;
  double row = step / relstep;
  int Row = (int)floor(0.5 + log10(row));
  if (relstep < 2.0)
    relstep = 1.0;
  else if (relstep < 5.0)
    relstep = 2.0;
  else
    relstep = 5.0;
  _GridStep = relstep * row;

  strcpy(_Labelformat, "%.");

  switch (Row) {
  case -5:
    strcat(_Labelformat, "5f");
    break;
  case -4:
    strcat(_Labelformat, "4f");
    break;
  case -3:
    strcat(_Labelformat, "3f");
    break;
  case -2:
    strcat(_Labelformat, "2f");
    break;
  case -1:
    strcat(_Labelformat, "1f");
    break;
  default:
    strcat(_Labelformat, "0f");
    break;
  }
  

  
  int steps = int(_MinPoint.X() / _GridStep) - 1;
  if (1 & steps)
    steps--;
  _GridMin.X(_GridStep * steps);
  steps = int(_MaxPoint.X() / _GridStep) + 1;
  _GridMax.X(_GridStep * steps);
  

  
  steps = int(_MinPoint.Y() / _GridStep) - 1;
  _GridMin.Y(_GridStep * steps);
  steps = int(_MaxPoint.Y() / _GridStep) + 1;
  _GridMax.Y(_GridStep * steps);
  

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(_MinPoint.X(), _MaxPoint.X(), _MinPoint.Y(), _MaxPoint.Y(),
          _MinPoint.Z(), _MaxPoint.Z());

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, _WindowSize.x, _WindowSize.y);
}

void GridView::MapScreenToWorld(int x, int y, WorldPoint &res) const {
  res.X(_MinPoint.X() + _upp * x);
  res.Y(_MaxPoint.Y() - _upp * y);
  res.Z(0.0);
}


void GridView::MapWorldToScreen(WorldPoint res, int &x, int &y) const {
  x = (int)(((float)res.X() - (float)_MinPoint.X())/(float)_upp);
  y = (int)(((float)res.Y() - (float)_MinPoint.Y())/(float)_upp);
}


void GridView::MoveView(ScreenPoint sp) {
  WorldPoint wp(-_upp * sp.x(), _upp * sp.y(), 0.0);
  _center += wp;
  _SetView();
}

void GridView::Zoom(int zoom) {
  if (zoom == 0)
    return;
  if (zoom > 0) {
    if (_scale > 100.0)
      return;
  } else {
    if (_scale < 0.001)
      return;
  }

  _scale *= pow(1.003, zoom);

  _SetView();
}

void GridView::SetScale(double scale) {
  if (scale <= 0.0)
    scale = 0.001;
  if (scale > 100.0)
    scale = 100.0;
  else if (scale < 0.001)
    scale = 0.001;
  _scale = scale;
}

double GridView::GetScale(void) const
{
  return _scale;
}


void GridView::SetCenter(double x, double y, double z) {
  if (x > 100.0)
    x = 100.0;
  else if (x < -100.0)
    x = -100.0;

  if (y > 100.0)
    y = 100.0;
  else if (y < -100.0)
    y = -100.0;

  _center.Set(x, y, z);
}

void GridView::SetCenter(WorldPoint c) { SetCenter(c.X(), c.Y(), c.Z()); }

void GridView::_PaintGrid(int retinaScale) {
  glLineWidth(GetSize(AxisWidth));
  float GridStep = _GridStep * retinaScale;

  if (_GridDrawWhat & eDrawGrid) {
    glColor3dv(GetColor(Grid));
    GLlines gll;
    for (double x = _GridMin.X(); x < _GridMax.X(); x += GridStep) {
      gll.Vertex(x, _GridMin.Y());
      gll.Vertex(x, _GridMax.Y());
    }
    for (double y = _GridMin.Y(); y < _GridMax.Y(); y += GridStep) {
      gll.Vertex(_GridMin.X(), y);
      gll.Vertex(_GridMax.X(), y);
    }
  }

  if (_GridDrawWhat & eDrawAxis) {
    GLlines gll;
    glColor3dv(GetColor(XAxis));
    glLineWidth(GetSize(AxisWidth));
    gll.Vertex(_GridMin.X(), 0.0);
    gll.Vertex(_GridMax.X(), 0.0);
    glColor3dv(GetColor(YAxis));
    gll.Vertex(0.0, _GridMin.Y());
    gll.Vertex(0.0, _GridMax.Y());
  }

  if (_GridDrawWhat & eDrawLabels) {
    glColor3dv(GetColor(Labels));
    static char bf[32];
    const double yline = _MinPoint.Y() + _upp * 4;
    if ((_GridMin.X() > 0.0) || (_GridMax.X() < 0.0)) {
      for (double x = _GridMin.X(); x < _GridMax.X(); x += 2 * GridStep) {
        sprintf(bf, _Labelformat, x);
        rtext(x, yline, bf);
      }
    } else {
      double x;
      for (x = _GridMin.X(); x < -GridStep / 2.0; x += 2 * GridStep) {
        sprintf(bf, _Labelformat, x);
        rtext(x, yline, bf);
      }
      sprintf(bf, _Labelformat, 0.0);
      rtext(0.0, yline, bf);

      for (x = 2 * GridStep; x < _GridMax.X(); x += 2 * GridStep) {
        sprintf(bf, _Labelformat, x);
        rtext(x, yline, bf);

      }
    }
    const double xline = _MinPoint.X() + _upp * 4;
    const double minushift = _minusWidth * _upp;
    if ((_GridMin.Y() > 0.0) || (_GridMax.X() < 0.0)) {
      for (double y = _GridMin.Y() + 2 * GridStep; y < _GridMax.Y();
           y += GridStep) {
        sprintf(bf, _Labelformat, y);
        if ('-' != bf[0]) {
          rtext(xline + minushift, y, bf);

        } else {
          rtext(xline, y, bf);

        }
      }
    } else {
      double y;
      for (y = _GridMin.Y() + 2 * GridStep; y < -GridStep * 0.5;
           y += GridStep) {
        sprintf(bf, _Labelformat, y);

        if ('-' != bf[0]) {
          rtext(xline + minushift, y, bf);
        } else {
          rtext(xline, y, bf);
        }
      }
      if ((_GridMin.Y() < 0.0) && (_GridMax.Y() > 0.0)) {
        sprintf(bf, _Labelformat, 0.0);
	rtext(xline + minushift, 0.0, bf);
        
      }
      for (y = GridStep; y < _GridMax.Y(); y += GridStep) {
        sprintf(bf, _Labelformat, y);
        if ('-' != bf[0]) {
          rtext(xline + minushift, y, bf);
        } else {
          rtext(xline, y, bf);
        }
      }
    }
  }
}
