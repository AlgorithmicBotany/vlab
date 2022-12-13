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

#ifdef WIN32
#include <windows.h>
#endif

#include "point.h"

#ifdef _WINDOWS
class View;
typedef View *PView;
#else
class View;
class GLWidget;
typedef GLWidget *PView;
#endif

enum CursorId {
  curArrow = 0,
  curPan,
  curZoom,
  curFov,
  curRotate,
  curRoll,

  nCursorCount
};

class ViewTask {
public:
  ViewTask() : _cid(curArrow), _pView(0) {}
  ViewTask(PView pView, CursorId cid);
  virtual void ButtonDown(int, int) {}
  virtual void MouseMove(int, int);
  virtual void ButtonUp(int, int) {}

protected:
  void _SetCursor() {
#ifdef WIN32
    SetCursor(_cursor[_cid]);
#endif
  }
  const CursorId _cid;
  PView _pView;
#ifdef WIN32
  static HCURSOR _cursor[nCursorCount];
#endif
};

class ViewIdleTask : public ViewTask {
public:
  ViewIdleTask() : ViewTask(0, curArrow) {}

  ViewIdleTask(PView);
};

class ViewRotateTask : public ViewTask {
public:
  ViewRotateTask() : ViewTask(0, curArrow) {}

  ViewRotateTask(PView);
  void ButtonDown(int, int);
  void MouseMove(int, int);

private:
  Point _LastPos;
};

class ViewZoomTask : public ViewTask {
public:
  ViewZoomTask() : ViewTask(0, curArrow) {}
  ViewZoomTask(PView);
  void ButtonDown(int, int);
  void MouseMove(int, int);

protected:
  int _LastY;
};

class ViewFrustumTask : public ViewTask {
public:
  ViewFrustumTask() : ViewTask(0, curArrow) {}

  ViewFrustumTask(PView);
  void ButtonDown(int, int);
  void MouseMove(int, int);

protected:
  int _LastY;
};

class ViewRollTask : public ViewTask {
public:
  ViewRollTask() : ViewTask(0, curArrow) {}

  ViewRollTask(PView);
  void ButtonDown(int, int);
  void MouseMove(int, int);

protected:
  int _LastX;
};

class ViewPanTask : public ViewTask {
public:
  ViewPanTask() : ViewTask(0, curArrow) {}

  ViewPanTask(PView);
  void ButtonDown(int, int);
  void MouseMove(int, int);

private:
  Point _LastPos;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
