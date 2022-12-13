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


#define GL_SILENCE_DEPRECATION

#include "perspview.h"
#include "config.h"

#include <iostream>
#include <QMouseEvent>
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
using namespace Qt;

PerspView::PerspView(Ctrl *pCtrl, Model *pModel)
    : View(pCtrl, pModel), firstResize(true),
      _rotateTask(RotatePerspViewTask(this)),
      _editTask(new EditPerspViewTask(this, pCtrl, pModel)), _min(), _max(),
      _center(), _rotX(25.0), _rotY(-35.0), _panX(0.0), _panY(0.0), _range(1.1),
      _zShift(4.7), _scalefactor(1.0), _useOrtho(true) {
  setWindowTitle("Perspective View");


  _orthoView = _pContextMenu->addAction("Use Orthogonal View", this,
                                        SLOT(switchToOrthoView()));
  addAction(_orthoView);
  _orthoView->setCheckable(true);
  _perspView = _pContextMenu->addAction("Use Perspective View", this,
                                        SLOT(switchToPerspView()));
  addAction(_perspView);
  _perspView->setCheckable(true);
  _orthoView->setChecked(true);
}

PerspView::~PerspView() { delete _editTask; }

void PerspView::switchToOrthoView() {
  useOrtho(true);
  _orthoView->setChecked(true);
  _perspView->setChecked(false);

  update();
}

void PerspView::switchToPerspView() {
  useOrtho(false);
  _orthoView->setChecked(false);
  _perspView->setChecked(true);

  update();
}

WorldPoint PerspView::mapScreenToWorld(ScreenPoint s) const {
  return WorldPoint(_min.X() + _upp * s.x(), _min.Y() + _upp * s.y(), 0.0);
}

WorldPoint *PerspView::getSelection(QMouseEvent *pEv) {
  Model::BezierPoint *pSelected = NULL;
  makeCurrent();

   GLuint *gl_selection = new GLuint[_pModel->getPointCount()];

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  glSelectBuffer(4 * _pModel->getPointCount(), gl_selection);

  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName((GLuint)NULL);
  glMatrixMode(GL_PROJECTION);

  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(pEv->x(), viewport[3] - pEv->y(), Config::getSelectionSize(),
                Config::getSelectionSize(), viewport);

  double range = _selRange;
  double zRange = _selZRange;
  double ratio = _selRatio;

  if (_useOrtho) {
    double vrange = range;
    if (ratio > 1.0) {
      glOrtho(-vrange * ratio, vrange * ratio, -vrange, vrange,
              _zShift - zRange, _zShift + zRange);
    } else {
      glOrtho(-vrange, vrange, -vrange / ratio, vrange / ratio,
              _zShift - zRange, _zShift + zRange);
    }
  } else {
    if (ratio > 1.0) {
      glFrustum(-range * ratio, range * ratio, -range, range, _zShift - zRange,
                _zShift + zRange);
    } else {
      glFrustum(-range, range, -range / ratio, range / ratio, _zShift - zRange,
                _zShift + zRange);
    }
  }

  glMatrixMode(GL_MODELVIEW);

  _pModel->useSelectSize(true);
  if (CONTROL_POINTS->isChecked())
    _pModel->drawControlPoints();
  if (CONNECTION_POINT->isChecked())
    _pModel->drawConnectPoint();
  if (END_POINT->isChecked())
    _pModel->drawEndPoint();
  _pModel->useSelectSize(false);

  glFlush();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  int hits = glRenderMode(GL_RENDER);
  glMatrixMode(GL_MODELVIEW);

  if (hits > 0) {
    GLuint sel = gl_selection[3];
    pSelected = _pModel->getPoint(sel);
    for (int i = 7; i < hits * 4; i += 4) {
      Model::BezierPoint *pt = _pModel->getPoint(gl_selection[i]);
      if (pt->sticky) {
        pSelected = pt;
      }
    }
  } else {
    pSelected = NULL;
  }

  delete[] gl_selection;

  return pSelected;
}

void PerspView::Move(ScreenPoint s) {
  _panX += _upp * s.x();
  _panY -= _upp * s.y();
  update();
}

void PerspView::Zoom(int factor) {
  _scalefactor += factor;
  update();
}

void PerspView::Rotate(ScreenPoint s) {
  _rotX += 0.5 * s.y();
  if (_rotX >= 360.0)
    _rotX -= 360.0;
  else if (_rotX < 0.0)
    _rotX += 360.0;

  _rotY += 0.5 * s.x();
  if (_rotY >= 360.0)
    _rotY -= 360.0;
  else if (_rotY < 0.0)
    _rotY += 360.0;

  update();
}

double PerspView::getZNear() { return -_pModel->getScale() * 1.5; }

double PerspView::getZFar() { return _pModel->getScale() * 1.5; }

void PerspView::mousePressEvent(QMouseEvent *pEv) {
  if (pEv->button() == Qt::LeftButton &&
      (pEv->modifiers() & Qt::ShiftModifier)) {
    _pCurrentTask = &_rotateTask;
    _pCurrentTask->ButtonDown(pEv);
  } else if ((pEv->button() == Qt::LeftButton) &&
             (pEv->modifiers() == Qt::NoModifier)) {
    _pCurrentTask = _editTask;
    _pCurrentTask->ButtonDown(pEv);
  } else
    View::mousePressEvent(pEv);
}

void PerspView::mouseDoubleClickEvent(QMouseEvent *pEv) {
  _pCurrentTask = _editTask;
  _pCurrentTask->DoubleClick(pEv);
}

void PerspView::initializeGL() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MAP2_VERTEX_3);

  GLfloat pos0[] = {0, 100, 100, 1};

  glLightfv(GL_LIGHT0, GL_POSITION, pos0);

  glLightfv(GL_LIGHT0, GL_AMBIENT, Config::getAmbientColour());
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Config::getAmbientColour());

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  glEnable(GL_AUTO_NORMAL);
  glEnable(GL_LIGHT0);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
}

void PerspView::resizeGL(int, int) { _setView(); }

void PerspView::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (GRID->isChecked())
    _pModel->drawGrid();
  if (AXES->isChecked())
    _pModel->drawAxes();
  if (UP_VECTOR->isChecked())
    _pModel->drawUpVector();
  if (HEADING_VECTOR->isChecked())
    _pModel->drawHeadingVector();
  if (CONNECTION_POINT->isChecked())
    _pModel->drawConnectPoint();
  if (END_POINT->isChecked())
    _pModel->drawEndPoint();
  if (CONTROL_POINTS->isChecked())
    _pModel->drawControlPoints();

  glEnable(GL_LIGHTING);
  if (WIRE_MODEL->isChecked())
    _pModel->drawWireModel();
  if (SOLID_MODEL->isChecked())
    _pModel->drawSolidModel();
  glDisable(GL_LIGHTING);

  if (CONTROL_POLY->isChecked())
    _pModel->drawControlPoly();
  glFlush();
}

void PerspView::_setView() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  static double zRange = 0.0;
  if (firstResize) {
    _range = _pModel->findLargestWidth();
    _zShift = _range;
    firstResize = false;
    _scalefactor = 1.0;
  }
  double ratio = double(width()) / double(height());
  double range = _range * pow(1.003, _scalefactor);
  zRange = _pModel->findLargestWidth();

  _selRange = range;
  _selZRange = zRange;
  _selRatio = ratio;

  if (_useOrtho) {
    double vrange = range;
    if (ratio > 1.0) {
      glOrtho(-vrange * ratio, vrange * ratio, -vrange, vrange,
              _zShift - zRange, _zShift + zRange);
      _gridRange = range * _zShift / (_zShift - 1.1);
      _upp = 2.0 * _gridRange / height();
    } else {
      glOrtho(-vrange, vrange, -vrange / ratio, vrange / ratio,
              _zShift - zRange, _zShift + zRange);
      _gridRange = range * _zShift / (_zShift - 1.1);
      _upp = 2.0 * _gridRange / width();
    }
  } else {
    if (ratio > 1.0) {
      glFrustum(-range * ratio, range * ratio, -range, range, _zShift - zRange,
                _zShift + zRange);
      _gridRange = range * _zShift / (_zShift - 1.1);
      _upp = 2.0 * _gridRange / height();
    } else {
      glFrustum(-range, range, -range / ratio, range / ratio, _zShift - zRange,
                _zShift + zRange);
      _gridRange = range * _zShift / (_zShift - 1.1);
      _upp = 2.0 * _gridRange / width();
    }
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glViewport(0, 0, width(), height());

  glTranslated(_panX, _panY, -_zShift);
  glRotated(_rotX, 1.0, 0.0, 0.0);
  glRotated(_rotY, 0.0, 1.0, 0.0);
}
