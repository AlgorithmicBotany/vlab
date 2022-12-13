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
#include <qmessagebox.h>
#include "ctrl.h"
#include "view.h"
#include <QApplication>
#include <QCloseEvent>
#include <QHideEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QShowEvent>
#include <iostream>

using namespace Qt;

View::View(Ctrl *pCtrl, Model *pModel)
    : QGLWidget(pCtrl), _pCtrl(pCtrl), _pModel(pModel),
      _idleTask(IdleViewTask(this)), _translateTask(TranslateViewTask(this)),
      _zoomTask(ZoomViewTask(this)), _pCurrentTask(&_idleTask),
      _pContextMenu(new QMenu(this)), _upp(0.0), _gridRange(1.0), _mode() {
  assert(pCtrl);
  assert(pModel);
  GRID = _pContextMenu->addAction("View Grid", this, SLOT(toggleGrid()));
  addAction(GRID);
  GRID->setCheckable(true);
  AXES = _pContextMenu->addAction("View Axes", this, SLOT(toggleAxes()));
  addAction(AXES);
  AXES->setCheckable(true);
  UP_VECTOR =
      _pContextMenu->addAction("View Up Vector", this, SLOT(toggleUpVector()));
  addAction(UP_VECTOR);
  UP_VECTOR->setCheckable(true);
  HEADING_VECTOR = _pContextMenu->addAction("View Heading Vector", this,
                                            SLOT(toggleHeadingVector()));
  addAction(HEADING_VECTOR);
  HEADING_VECTOR->setCheckable(true);
  CONNECTION_POINT = _pContextMenu->addAction("View Connection Point", this,
                                              SLOT(toggleConnectionPoint()));
  addAction(CONNECTION_POINT);
  CONNECTION_POINT->setCheckable(true);
  END_POINT =
      _pContextMenu->addAction("View End Point", this, SLOT(toggleEndPoint()));
  addAction(END_POINT);
  END_POINT->setCheckable(true);
  CONTROL_POINTS = _pContextMenu->addAction("View Control Points", this,
                                            SLOT(toggleControlPoints()));
  addAction(CONTROL_POINTS);
  CONTROL_POINTS->setCheckable(true);
  WIRE_MODEL = _pContextMenu->addAction("View Wire Frame Model", this,
                                        SLOT(toggleWireModel()));
  addAction(WIRE_MODEL);
  WIRE_MODEL->setCheckable(true);
  SOLID_MODEL = _pContextMenu->addAction("View Shaded Model", this,
                                         SLOT(toggleSolidModel()));
  addAction(SOLID_MODEL);
  SOLID_MODEL->setCheckable(true);
  CONTROL_POLY = _pContextMenu->addAction("View Control Polygon", this,
                                          SLOT(toggleControlPoly()));
  addAction(CONTROL_POLY);
  CONTROL_POLY->setCheckable(true);

 }

View::~View() {}

void View::update() {
  makeCurrent();
  _setView();
  updateGL();
}

void View::closeEvent(QCloseEvent *pEv) {
  // propogate the close event up to the parent widget
  this->close();
  pEv->ignore();
  _pCtrl->close();
}

void View::hideEvent(QHideEvent *) {
  if (isMinimized()) {
    if (!_pCtrl->isMinimized())
      _pCtrl->showMinimized();
  }
}

void View::showEvent(QShowEvent *) {
  if (isVisible()) {
    if (_pCtrl->isMinimized())
      _pCtrl->showNormal();
  }
}

void View::setDrawMode(const unsigned int m) {
  _mode = m;

  GRID->setChecked(!GRID->isChecked());
  AXES->setChecked(!AXES->isChecked());
  UP_VECTOR->setChecked(!UP_VECTOR->isChecked());
  HEADING_VECTOR->setChecked(!HEADING_VECTOR->isChecked());
  CONNECTION_POINT->setChecked(!CONNECTION_POINT->isChecked());
  END_POINT->setChecked(!END_POINT->isChecked());
  CONTROL_POINTS->setChecked(!CONTROL_POINTS->isChecked());
  WIRE_MODEL->setChecked(!WIRE_MODEL->isChecked());
  SOLID_MODEL->setChecked(!SOLID_MODEL->isChecked());
  CONTROL_POLY->setChecked(!CONTROL_POLY->isChecked());
}

void View::toggleGrid() {
  update();
}

void View::toggleAxes() { update(); }

void View::toggleUpVector() { update(); }

void View::toggleHeadingVector() { update(); }

void View::toggleConnectionPoint() { update(); }

void View::toggleEndPoint() { update(); }

void View::toggleControlPoints() { update(); }

void View::toggleWireModel() {

  if ((!SOLID_MODEL->isChecked()) && (!WIRE_MODEL->isChecked()))
    toggleSolidModel();

  update();
}

void View::toggleSolidModel() {
  _mode ^= SOLID_MODEL->isChecked();

  if ((SOLID_MODEL->isChecked() & _mode) && (WIRE_MODEL->isChecked() & _mode))
    toggleWireModel();

  update();
}

void View::toggleControlPoly() { update(); }

void View::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  _pModel->drawGrid();
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
  if (WIRE_MODEL->isChecked())
    _pModel->drawWireModel();
  if (SOLID_MODEL->isChecked())
    _pModel->drawSolidModel();
  if (CONTROL_POLY->isChecked())
    _pModel->drawControlPoly();

  glFlush();
}

void View::mouseMoveEvent(QMouseEvent *pEv) {
  _pCurrentTask->PointerMotion(pEv);
}

void View::mousePressEvent(QMouseEvent *pEv) {
  QWidget *topWidget = QApplication::topLevelAt(this->mapToGlobal(QPoint()));
  if (topWidget != nullptr)
    topWidget->raise();
  switch (pEv->button()) {
  case RightButton:
    _pContextMenu->exec(QCursor::pos());
    break;
  case Qt::LeftButton:
    if (pEv->modifiers() & Qt::AltModifier){
      _pCurrentTask = &_translateTask;
    }
    else if (pEv->modifiers() & Qt::ControlModifier){
      _pCurrentTask = &_zoomTask;
    }
    break;
  case MidButton:
    _pCurrentTask = &_zoomTask;
  default:
    break;
  }

  _pCurrentTask->ButtonDown(pEv);
}

void View::mouseReleaseEvent(QMouseEvent *pEv) {
  _pCurrentTask->ButtonUp(pEv);
  _pCurrentTask = &_idleTask;
}
