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
#include <cstdlib>
#include <cmath>

#include <qgl.h>
#include <QMouseEvent>

#include "geometry.h"
#include "gridview.h"
#include "gridtask.h"

GridViewIdleTask::GridViewIdleTask(GridView *pView) : GridTask(pView) {}

void GridViewIdleTask::ButtonDown(QMouseEvent *,int) {}

void GridViewIdleTask::PointerMotion(QMouseEvent *,int) {}

void GridViewIdleTask::ButtonUp(QMouseEvent *,int) {}

GridViewTranslateTask::GridViewTranslateTask(GridView *pView)
    : GridTask(pView) {}

void GridViewTranslateTask::ButtonDown(QMouseEvent *pEv,int retinaScale) {
  _lastPos.Set(pEv->x()*retinaScale, pEv->y()*retinaScale);
}

void GridViewTranslateTask::PointerMotion(QMouseEvent *pEv,int retinaScale) {
  ScreenPoint newpos(pEv->x()*retinaScale, pEv->y()*retinaScale);
  ScreenPoint rel = newpos - _lastPos;
  _pView->MoveView(rel);
  _pView->Redraw();
  _lastPos = newpos;
}

void GridViewTranslateTask::ButtonUp(QMouseEvent *,int) {}

GridViewZoomTask::GridViewZoomTask(GridView *pView) : GridTask(pView) {}

void GridViewZoomTask::ButtonDown(QMouseEvent *pEv, int retinaScale) { _lastY = pEv->y()*retinaScale; }

void GridViewZoomTask::PointerMotion(QMouseEvent *pEv, int retinaScale) {
  int newy = pEv->y()*retinaScale;
  int rel = newy - _lastY;

  if (0 != rel) {
    _pView->Zoom(rel);
    _pView->Redraw();
    _lastY = newy;
  }
}

void GridViewZoomTask::ButtonUp(QMouseEvent *,int) {}
