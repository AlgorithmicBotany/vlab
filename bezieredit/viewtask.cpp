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



#include "viewtask.h"
#include "model.h"
#include "perspview.h"
#include "view.h"
#include <QMouseEvent>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <iostream>

TranslateViewTask::TranslateViewTask(View *pView) : ViewTask(pView) {}

void TranslateViewTask::ButtonDown(QMouseEvent *pEv) {
  _lastPos.Set(pEv->x(), pEv->y());
}

void TranslateViewTask::PointerMotion(QMouseEvent *pEv) {
  ScreenPoint newPos(pEv->x(), pEv->y());
  _pView->Move(newPos - _lastPos);
  _pView->update();
  _lastPos = newPos;
}

ZoomViewTask::ZoomViewTask(View *pView) : ViewTask(pView) {}

void ZoomViewTask::ButtonDown(QMouseEvent *pEv) { _lastY = pEv->y(); }

void ZoomViewTask::PointerMotion(QMouseEvent *pEv) {
  int newY = pEv->y();
  int rel = newY - _lastY;

  if (rel) {
    _pView->Zoom(rel);
    _pView->update();
    _lastY = newY;
  }
}

EditPerspViewTask::EditPerspViewTask(PerspView *pView, Ctrl *pCtrl,
                                     Model *pModel)
    : ViewTask(dynamic_cast<View *>(pView)), _pSelected(NULL), _pSelFirst(NULL),
      _pCtrl(pCtrl), _editMode(MOVE_POINT), _pModel(pModel), _stickyMode(false),
      _pStickyPoint(NULL) {}

void EditPerspViewTask::ButtonDown(QMouseEvent *pEv) {
  _lastPos = dynamic_cast<PerspView *>(_pView)->mapScreenToWorld(
      ScreenPoint(pEv->x(), pEv->y()));
  _pSelected = dynamic_cast<PerspView *>(_pView)->getSelection(pEv);

  // if we have no point selected, go no further
  if (!_pSelected)
    return;

  // do whatever is appropriate given the selected mode.
  switch (_editMode) {

  case SPLIT:
    _pModel->splitPatchesOfPoint(static_cast<Model::BezierPoint *>(_pSelected));
    _pSelected = NULL;
    _pCtrl->updateViews();
    break;

  case MERGE:
    if (_pSelFirst) {
      _pModel->mergePatchesOfPoints(
          static_cast<Model::BezierPoint *>(_pSelFirst),
          static_cast<Model::BezierPoint *>(_pSelected));
      static_cast<Model::BezierPoint *>(_pSelFirst)->selectedForMerge = false;
      _pSelFirst = NULL;
      _pCtrl->updateViews();
    } else {
      _pSelFirst = _pSelected;
      static_cast<Model::BezierPoint *>(_pSelFirst)->selectedForMerge = true;
      _pCtrl->updateViews();
    }
    _pSelected = NULL;
    break;

  case REMOVE:
    _pModel->deletePatchesOfPoint(
        static_cast<Model::BezierPoint *>(_pSelected));
    _pSelected = NULL;
    _pCtrl->updateNames();
    _pCtrl->updateViews();
    break;

  case MOVE_POINT:
  case MOVE_PATCH:
    // do nothing, this case is handled in the pointer motion
    static_cast<Model::BezierPoint *>(_pSelected)->selected = true;
  default:
    // do nothing
    break;
  }
}

void EditPerspViewTask::DoubleClick(QMouseEvent *pEv) {
  _pSelected = dynamic_cast<PerspView *>(_pView)->getSelection(pEv);
  if (_pSelected) {
    if (static_cast<Model::BezierPoint *>(_pSelected)->sticky) {
      static_cast<Model::BezierPoint *>(_pSelected)->sticky = false;
      _stickyMode = false;
      _pStickyPoint = NULL;
      _pCtrl->updateViews();
    } else {
      if (_stickyMode) {
        _pStickyPoint->sticky = false;
      }
      static_cast<Model::BezierPoint *>(_pSelected)->sticky = true;
      _stickyMode = true;
      _pStickyPoint = static_cast<Model::BezierPoint *>(_pSelected);
      _pCtrl->updateViews();
    }
  }
  _pSelected = NULL;
}

void EditPerspViewTask::movePointMode() {
  _editMode = MOVE_POINT;
  if (_pSelFirst) {
    static_cast<Model::BezierPoint *>(_pSelFirst)->selectedForMerge = false;
    _pCtrl->updateViews();
    _pSelFirst = NULL;
  }
}

void EditPerspViewTask::movePatchMode() {
  _editMode = MOVE_PATCH;
  if (_pSelFirst) {
    static_cast<Model::BezierPoint *>(_pSelFirst)->selectedForMerge = false;
    _pCtrl->updateViews();
    _pSelFirst = NULL;
  }
}

void EditPerspViewTask::splitMode() {
  _editMode = SPLIT;
  if (_pSelFirst) {
    static_cast<Model::BezierPoint *>(_pSelFirst)->selectedForMerge = false;
    _pCtrl->updateViews();
    _pSelFirst = NULL;
  }
}

void EditPerspViewTask::mergeMode() {
  if (_editMode == MERGE)
    return;

  _editMode = MERGE;
  if (_pSelFirst) {
    static_cast<Model::BezierPoint *>(_pSelFirst)->selectedForMerge = false;
    _pCtrl->updateViews();
    _pSelFirst = NULL;
  }
}

void EditPerspViewTask::deleteMode() {
  _editMode = REMOVE;
  if (_pSelFirst) {
    static_cast<Model::BezierPoint *>(_pSelFirst)->selectedForMerge = false;
    _pCtrl->updateViews();
    _pSelFirst = NULL;
  }
}

void EditPerspViewTask::PointerMotion(QMouseEvent *pEv) {
  if (_pSelected) {
    WorldPoint pt = dynamic_cast<PerspView *>(_pView)->mapScreenToWorld(
        ScreenPoint(pEv->x(), pEv->y()));

    GLint viewport[4];
    GLdouble mvmatrix[16];
    GLdouble projmatrix[16];
    GLdouble nwx = 0.0, nwy = 0.0, nwz = 0.0;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
    glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

    GLint realy = viewport[3] - pEv->y() - 1;

    // get _pSelected projected to build the distance ratio
    GLdouble spx = 0.0, spy = 0.0, spz = 0.0;
    gluProject(_pSelected->X(), _pSelected->Y(), _pSelected->Z(), mvmatrix,
               projmatrix, viewport, &spx, &spy, &spz);

    // get near coords
    gluUnProject(pEv->x(), realy, spz, mvmatrix, projmatrix, viewport, &nwx,
                 &nwy, &nwz);

    WorldPoint wp(nwx - _pSelected->X(), nwy - _pSelected->Y(),
                  nwz - _pSelected->Z());

    switch (_editMode) {
    case MOVE_POINT:
      _pModel->movePoint(static_cast<Model::BezierPoint *>(_pSelected), wp);
      break;
    case MOVE_PATCH:
      _pModel->movePatchesOfPoint(static_cast<Model::BezierPoint *>(_pSelected),
                                  wp);
      break;
    default:
      break;
    }
    _lastPos = pt;
    _pCtrl->updateViews();
  }
}

void EditPerspViewTask::ButtonUp(QMouseEvent *pEv) {
  if (_pSelected) {
    static_cast<Model::BezierPoint *>(_pSelected)->selected = false;
    _pSelected = NULL;
  }
  pEv->accept();
}

RotatePerspViewTask::RotatePerspViewTask(PerspView *pView)
    : ViewTask(dynamic_cast<View *>(pView)) {}

void RotatePerspViewTask::ButtonDown(QMouseEvent *pEv) {
  _lastPos.Set(pEv->x(), pEv->y());
}

void RotatePerspViewTask::PointerMotion(QMouseEvent *pEv) {
  ScreenPoint newPos(pEv->x(), pEv->y());
  dynamic_cast<PerspView *>(_pView)->Rotate(newPos - _lastPos);
  _pView->update();
  _lastPos = newPos;
}
