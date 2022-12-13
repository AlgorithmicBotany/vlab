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



#include <climits>
#include <cmath>
#include <cstdlib>

#include "ctrl.h"
#include "dynarray.h"
#include "functask.h"
#include "geometry.h"
#include "gridview.h"
#include "model.h"
#include <QMouseEvent>

FuncViewTask::FuncViewTask(Ctrl *pCtrl) : GridTask(pCtrl), _pFuncView(pCtrl) {}

void AddRemoveTask::ButtonDown(QMouseEvent *pEv, int retinaScale) {
  WorldPoint wp;
  _pView->MapScreenToWorld(pEv->x()*retinaScale, pEv->y()*retinaScale, wp);
  int selected = _pFuncView->FindClosest(wp);
  if (-1 == selected)
    _pFuncView->AddPoint(wp);
  else
    _pFuncView->DeletePoint(selected);
  _pView->Redraw();
}

void AddRemoveTask::PointerMotion(QMouseEvent *, int) {}

void AddRemoveTask::ButtonUp(QMouseEvent *,int ) {}

void DragPointTask::ButtonDown(QMouseEvent *pEv, int retinaScale) {
  WorldPoint wp;
  _pView->MapScreenToWorld(pEv->x()*retinaScale, pEv->y()*retinaScale, wp);
  _selected = _pFuncView->FindClosest(wp);
}

void DragPointTask::PointerMotion(QMouseEvent *pEv,int retinaScale) {
  if (-1 != _selected) {
    WorldPoint wp;
    _pView->MapScreenToWorld(pEv->x()*retinaScale, pEv->y()*retinaScale, wp);
    _pFuncView->MovePoint(_selected, wp);
    _pView->Redraw();
  }
}

void DragPointTask::ButtonUp(QMouseEvent *,int) {}

void IncMultiplicityTask::ButtonDown(QMouseEvent *pEv,int retinaScale) {
  WorldPoint wp;
  _pView->MapScreenToWorld(pEv->x()*retinaScale, pEv->y()*retinaScale, wp);
  int selected = _pFuncView->FindClosest(wp);
  if (-1 != selected)
    _pFuncView->IncPointMultiplicity(selected);
  _pView->Redraw();
}

void IncMultiplicityTask::PointerMotion(QMouseEvent *,int) {}

void IncMultiplicityTask::ButtonUp(QMouseEvent *,int) {}
