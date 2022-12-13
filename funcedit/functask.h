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




#ifndef __FUNCVIEWTASK_H__
#define __FUNCVIEWTASK_H__

class Ctrl;

class FuncViewTask : public GridTask
{
public:
  FuncViewTask(Ctrl*);
protected:
  Ctrl* _pFuncView;
};

class AddRemoveTask : public FuncViewTask
{
public:
  AddRemoveTask(Ctrl* pCtrl) : FuncViewTask(pCtrl) {}

  void ButtonDown(QMouseEvent*,int);
  void PointerMotion(QMouseEvent*,int);
  void ButtonUp(QMouseEvent*,int);
};

class DragPointTask : public FuncViewTask
{
public:
  DragPointTask(Ctrl* pCtrl) : FuncViewTask(pCtrl) {}

  void ButtonDown(QMouseEvent*,int);
  void PointerMotion(QMouseEvent*,int);
  void ButtonUp(QMouseEvent*,int);
private:
  int _selected;
};

class IncMultiplicityTask : public FuncViewTask {
public:
  IncMultiplicityTask(Ctrl *pCtrl) : FuncViewTask(pCtrl) {}

  void ButtonDown(QMouseEvent *,int);
  void PointerMotion(QMouseEvent *,int);
  void ButtonUp(QMouseEvent *,int);

};


#endif
