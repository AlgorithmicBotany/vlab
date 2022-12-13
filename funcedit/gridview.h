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




#ifndef __GRIDVIEW_H__
#define __GRIDVIEW_H__


#include "scrpnt.h"
#include "gridtask.h"
#include "geometry.h"

class GridView
{

public:
  GridView();
  virtual ~GridView() {}

  void MapScreenToWorld(int, int, WorldPoint&) const;
  void MapWorldToScreen(WorldPoint, int&, int&) const;
  void Redraw() {}
  void SetScale(double);
  double GetScale(void) const;

  void SetCenter(double, double, double);
  void SetCenter(WorldPoint);
  void MoveView(ScreenPoint);
  void Zoom(int);

protected:
  void GInit();
  void Resize();

  void _PaintGrid(int);
  void _SetView();

  virtual void rtext(double x, double y, char* bf) = 0;

  WorldPoint _MinPoint;
  WorldPoint _MaxPoint;
  WorldPoint _center;
  double _scale;
  double _upp;
  WorldPoint _GridMin;
  WorldPoint _GridMax;
  double _GridStep;

  char _Labelformat[16];

  struct
  {
    int x;
    int y;
  } _WindowSize;
  int _charWidth;
  int _charHeight;
  int _minusWidth;

  unsigned int _FontBaseList;

  enum
  {
    eGridColor = 0,
    eAxisColor = 1,
    eTextColor = 2,
    

    eLastColor
  };

  enum
  {
    eDrawGrid = 1,
    eDrawLabels = 1 << 1,
    eDrawAxis = 1 << 2
  };

  int _GridDrawWhat;

  GridTask* _pTask;
  GridViewIdleTask _IdleTask;
  GridViewTranslateTask _TranslateTask;
  GridViewZoomTask _ZoomTask;
};

#endif
