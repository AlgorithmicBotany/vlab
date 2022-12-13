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




#ifndef __VIEW_H__
#define __VIEW_H__

#include <qgl.h>
#include <QMenu>
#include <QShowEvent>
#include <QHideEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include "viewtask.h"
#include "scrpnt.h"

class Ctrl;
class Model;

class View : public QGLWidget {
  Q_OBJECT

 public:
  View(Ctrl* pCtrl, Model* pModel);
  virtual ~View();

  virtual void Move(ScreenPoint) = 0;
  virtual void Zoom(int) = 0;

  virtual void update();
  void closeEvent(QCloseEvent*);

  QAction* GRID;
  QAction* AXES;
  QAction* UP_VECTOR;
  QAction* HEADING_VECTOR;
  QAction* CONNECTION_POINT;
  QAction* END_POINT;
  QAction* CONTROL_POINTS;
  QAction* WIRE_MODEL;
  QAction* SOLID_MODEL;
  QAction* CONTROL_POLY;
  
  void setDrawMode(const unsigned int);

 protected slots:
  void toggleGrid();
  void toggleAxes();
  void toggleUpVector();
  void toggleHeadingVector();
  void toggleConnectionPoint();
  void toggleEndPoint();
  void toggleControlPoints();
  void toggleWireModel();
  void toggleSolidModel();
  void toggleControlPoly();

 protected:
  void _prepareContextMenu();

  virtual void _setView() = 0;

  virtual void paintGL();
  virtual void resizeGL(int, int) = 0;
  virtual void initializeGL() = 0;

  void mouseMoveEvent(QMouseEvent*);
  void mousePressEvent(QMouseEvent*);
  void mouseReleaseEvent(QMouseEvent*);
  void hideEvent(QHideEvent*);
  void showEvent(QShowEvent*);

  Ctrl*  _pCtrl;
  Model* _pModel;

  IdleViewTask      _idleTask;
  TranslateViewTask _translateTask;
  ZoomViewTask      _zoomTask;
  ViewTask*         _pCurrentTask;

  QMenu* _pContextMenu;

  double   _upp;
  double   _gridRange;
  
  unsigned int _mode;
};

#endif
