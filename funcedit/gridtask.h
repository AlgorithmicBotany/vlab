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




#ifndef __GRIDTASK_H__
#define __GRIDTASK_H__

#include <QEvent>
#include "scrpnt.h"

class GridView;

class GridTask
{
public:
  GridTask(GridView* pView) : _pView(pView) {}


  virtual void ButtonDown(QMouseEvent*, int) = 0;
  virtual void PointerMotion(QMouseEvent*,int) = 0;
  virtual void ButtonUp(QMouseEvent*,int) = 0;
protected:
  GridView* _pView;
};

class GridViewIdleTask : public GridTask
{
public:
  GridViewIdleTask(GridView*);

  void ButtonDown(QMouseEvent*,int);
  void PointerMotion(QMouseEvent*,int);
  void ButtonUp(QMouseEvent*,int);
};

class GridViewTranslateTask : public GridTask
{
public:
  GridViewTranslateTask(GridView*); 

  void ButtonDown(QMouseEvent*,int);
  void PointerMotion(QMouseEvent*,int);
  void ButtonUp(QMouseEvent*,int);
private:
  ScreenPoint _lastPos;
};

class GridViewZoomTask : public GridTask
{
public:
  GridViewZoomTask(GridView*);

  void ButtonDown(QMouseEvent*,int);
  void PointerMotion(QMouseEvent*,int);
  void ButtonUp(QMouseEvent*,int);
private:
  int _lastY;
};

#endif
