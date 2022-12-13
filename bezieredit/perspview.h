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




#ifndef __PERSPVIEW_H__
#define __PERSPVIEW_H__

#include "view.h"
#include "ctrl.h"
#include "model.h"
#include "geometry.h"
#include "scrpnt.h"

#include <QMouseEvent>

class PerspView : public View {
  Q_OBJECT

 public:
  PerspView(Ctrl* pCtrl, Model* pModel);
  virtual ~PerspView();

  void useOrtho(bool on) {_useOrtho = on;}

 protected slots:
  void switchToOrthoView();
  void switchToPerspView();

 public:
  void snapToXY() {_rotX = 0;  _rotY = 0;}
  void snapToYZ() {_rotX = 90; _rotY = 0;}
  void snapToZX() {_rotX = 0;  _rotY = 90;}

  WorldPoint  mapScreenToWorld(ScreenPoint s) const;
  WorldPoint* getSelection(QMouseEvent* pEv);

  void movePointMode() {_editTask->movePointMode();}
  void movePatchMode() {_editTask->movePatchMode();}
  void splitMode()     {_editTask->splitMode();}
  void mergeMode()     {_editTask->mergeMode();}
  void deleteMode()    {_editTask->deleteMode();}

  void Move(ScreenPoint);
  void Zoom(int);
  void Rotate(ScreenPoint);

  double getZNear();
  double getZFar();

  bool firstResize;

 protected:
  void mousePressEvent(QMouseEvent*);
  void mouseDoubleClickEvent(QMouseEvent*);

  void initializeGL();
  void resizeGL(int, int);
  void paintGL();

  void _setView();

  RotatePerspViewTask _rotateTask;
  EditPerspViewTask*  _editTask;

  WorldPoint _min;
  WorldPoint _max;
  WorldPoint _center;

  double _rotX;
  double _rotY;
  double _panX;
  double _panY;
  double _range;
  double _zShift;
  double _scalefactor;

  bool _useOrtho;
  QAction*  _orthoView;
  QAction*  _perspView;

  double _selRange;
  double _selZRange;
  double _selRatio;
};

#endif
