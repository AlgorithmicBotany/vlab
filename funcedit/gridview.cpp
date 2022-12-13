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



#include <cassert>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include <qgl.h>

#include "colors.h"
#include "geometry.h"
#include "glutils.h"
#include "gridview.h"

GridView::GridView() : _IdleTask(this), _TranslateTask(this), _ZoomTask(this) {
  _GridDrawWhat = eDrawGrid | eDrawLabels | eDrawAxis;
  _pTask = &_IdleTask;
  _scale = 1.0;
  SetScale(1.2);
  SetCenter(0.0, 0.0, 0.0);
  _WindowSize.x = 1;
  _WindowSize.y = 1;
  ReadColors();
}

void GridView::GInit() {
  // Initialize a font from Qt
  {
    QFont f;
    QFontMetrics fm(f);

    _charHeight = fm.ascent();
    _minusWidth = fm.width('-');
  }
  {
    // Initialize a clear colour
    const GLdouble *clcl = GetColor(Background);
    glClearColor(clcl[0], clcl[1], clcl[2], clcl[3]);
  }
  glLineWidth(GetSize(AxisWidth));
}

void GridView::Resize() { _SetView(); }

void GridView::_SetView() {
  // Note: window size is set in the inherited widget Ctrl,
  //  which handles the Qt API.
  const double wc = double(_WindowSize.x) / double(_WindowSize.y);

  if (wc > 1.0) {
    _MinPoint.Set(_center.X() - _scale * wc, _center.Y() - _scale, -1.0);
    _MaxPoint.Set(_center.X() + _scale * wc, _center.Y() + _scale, 1.0);
    _upp = (2.0 * _scale) / _WindowSize.y;
  } else {
    _MinPoint.Set(_center.X() - _scale, _center.Y() - _scale / wc, -1.0);
    _MaxPoint.Set(_center.X() + _scale, _center.Y() + _scale / wc, 1.0);
    _upp = (2.0 * _scale) / _WindowSize.x;
  }

  // Y
  {
    int numoflabels = _WindowSize.y / (3 * _charHeight);
    if (numoflabels < 1)
      numoflabels = 1;
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
  }

  {
    int steps = int(_MinPoint.X() / _GridStep) - 1;
    if (1 & steps)
      steps--;
    _GridMin.X(_GridStep * steps);
    steps = int(_MaxPoint.X() / _GridStep) + 1;
    _GridMax.X(_GridStep * steps);
  }

  {
    int steps = int(_MinPoint.Y() / _GridStep) - 1;
    _GridMin.Y(_GridStep * steps);
    steps = int(_MaxPoint.Y() / _GridStep) + 1;
    _GridMax.Y(_GridStep * steps);
  }

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

  float GridMinX = _GridMin.X();
  float GridMaxX = _GridMax.X();
  float GridMinY = _GridMin.Y();
  float GridMaxY = _GridMax.Y();
  float GridStep = _GridStep * retinaScale;

  if (_GridDrawWhat & eDrawGrid) {
    glColor3dv(GetColor(Grid));

    GLlines gll;
    for (double x = GridMinX; x < GridMaxX; x += GridStep) {
      gll.Vertex(x, GridMinY);
      gll.Vertex(x, GridMaxY);
    }
    for (double y = GridMinY; y < GridMaxY; y += GridStep) {
      gll.Vertex(GridMinX, y);
      gll.Vertex(GridMaxX, y);
    }
  }

  if (_GridDrawWhat & eDrawAxis) {
    GLlines gll;
    glColor3dv(GetColor(XAxis));
    glLineWidth(GetSize(AxisWidth));
    gll.Vertex(GridMinX, 0.0);
    gll.Vertex(GridMaxX, 0.0);
    glColor3dv(GetColor(YAxis));
    gll.Vertex(0.0, GridMinY);
    gll.Vertex(0.0, GridMaxY);
  }

  if (_GridDrawWhat & eDrawLabels) {
    glColor3dv(GetColor(Labels));
    static char bf[32];
    const double yline = _MinPoint.Y() + _upp * 4;
    if ((GridMinX > 0.0) || (GridMaxX < 0.0)) {
      for (double x = GridMinX; x < GridMaxX; x += 2 * GridStep) {
        sprintf(bf, _Labelformat, x);
        rtext(x, yline, bf);
      }
    } else {
      double x;
      for (x = GridMinX; x < -GridStep / 2.0; x += 2 * GridStep) {
        sprintf(bf, _Labelformat, x);
        rtext(x, yline, bf);
      }
      sprintf(bf, _Labelformat, 0.0);
      rtext(0.0, yline, bf);
      for (x = 2 * GridStep; x < GridMaxX; x += 2 * GridStep) {
        sprintf(bf, _Labelformat, x);
        rtext(x, yline, bf);
      }
    }
    const double xline = _MinPoint.X() + _upp * 4;
    const double minushift = _minusWidth * _upp;
    if ((GridMinY > 0.0) || (GridMaxX < 0.0)) {
      for (double y = GridMinY + 2 * GridStep; y < GridMaxY;
           y += GridStep) {
        sprintf(bf, _Labelformat, y);
        if ('-' != bf[0])
          rtext(xline + minushift, y, bf);
        else
          rtext(xline, y, bf);
      }
    } else {
      double y;
      for (y = GridMinY + 2 * GridStep; y < -GridStep * 0.5;
           y += GridStep) {
        sprintf(bf, _Labelformat, y);
        if ('-' != bf[0])
          rtext(xline + minushift, y, bf);
        else
          rtext(xline, y, bf);
      }
      if ((GridMinY < 0.0) && (GridMaxY > 0.0)) {

        sprintf(bf, _Labelformat, 0.0);
        rtext(xline+ minushift, 0.0, bf);
      }
      for (y = GridStep; y < GridMaxY; y += GridStep) {
        sprintf(bf, _Labelformat, y);
        if ('-' != bf[0])
          rtext(xline + minushift, y, bf);
        else
          rtext(xline, y, bf);
      }
    }
  }
}
