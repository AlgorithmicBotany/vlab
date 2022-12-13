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




#ifndef __VIEWTASK_H__
#define __VIEWTASK_H__
#define GL_SILENCE_DEPRECATION
#include <qevent.h>
#include "scrpnt.h"
#include "geometry.h"
#include "model.h"

class Ctrl;
class View;
class PerspView;
class OrthoView;
class Model;

class ViewTask {
 public:
  ViewTask(View* pView) : _pView(pView) {}
  virtual ~ViewTask() {}

  virtual void ButtonDown(QMouseEvent*)    {}
  virtual void PointerMotion(QMouseEvent*) {}
  virtual void ButtonUp(QMouseEvent*)      {}
  virtual void DoubleClick(QMouseEvent*)   {}

 protected:
  View* _pView;
};

class IdleViewTask : public ViewTask {
 public:
  IdleViewTask(View* pView) : ViewTask(pView) {}
};

class TranslateViewTask : public ViewTask {
 public:
  TranslateViewTask(View* pView);

  void ButtonDown(QMouseEvent*);
  void PointerMotion(QMouseEvent*);

 private:
  ScreenPoint _lastPos;
};

class ZoomViewTask : public ViewTask {
 public:
  ZoomViewTask(View* pView);

  void ButtonDown(QMouseEvent*);
  void PointerMotion(QMouseEvent*);

 private:
  int _lastY;
};

class EditPerspViewTask : public ViewTask {
 public:
  EditPerspViewTask(PerspView* pView, Ctrl* pCtrl, Model* pModel);
  virtual ~EditPerspViewTask() {}

  void ButtonDown(QMouseEvent*);
  void ButtonUp(QMouseEvent*);
  void DoubleClick(QMouseEvent*);
  void PointerMotion(QMouseEvent*);  

  void movePointMode();
  void movePatchMode();
  void splitMode();
  void mergeMode();
  void deleteMode();

 protected:
  WorldPoint  _lastPos;
  WorldPoint* _pSelected;
  WorldPoint* _pSelFirst;
  Ctrl*       _pCtrl;

  enum {
    MOVE_POINT,
    MOVE_PATCH,
    SPLIT,
    MERGE,
    REMOVE
  } _editMode;

  Model* _pModel;
  bool   _stickyMode;
  Model::BezierPoint* _pStickyPoint;
};

class RotatePerspViewTask : public ViewTask {
 public:
  RotatePerspViewTask(PerspView* pView);

  void ButtonDown(QMouseEvent*);
  void PointerMotion(QMouseEvent*);

 private:
  ScreenPoint _lastPos;
};

#endif
