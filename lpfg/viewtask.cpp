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
#ifdef WIN32
#include "view.h"
#else
#include "viewLnx.h"
#endif

#ifdef WIN32
#include "resource.h"
#endif

#ifdef WIN32
HCURSOR ViewTask::_cursor[] = {NULL, NULL, NULL, NULL, NULL, NULL};
#endif

ViewTask::ViewTask(PView pView, CursorId cid) : _cid(cid), _pView(pView) {
#ifdef WIN32
  if (NULL == _cursor[0]) {
    _cursor[curArrow] = LoadCursor(NULL, IDC_ARROW);
    _cursor[curPan] =
        LoadCursor(pView->GetInstance(), MAKEINTRESOURCE(IDC_PAN));
    _cursor[curZoom] =
        LoadCursor(pView->GetInstance(), MAKEINTRESOURCE(IDC_ZOOM));
    _cursor[curFov] =
        LoadCursor(pView->GetInstance(), MAKEINTRESOURCE(IDC_FOV));
    _cursor[curRotate] =
        LoadCursor(pView->GetInstance(), MAKEINTRESOURCE(IDC_ROTATE));
    _cursor[curRoll] =
        LoadCursor(pView->GetInstance(), MAKEINTRESOURCE(IDC_ROTATE));
  }
#endif
}

void ViewTask::MouseMove(int, int) { _SetCursor(); }

ViewIdleTask::ViewIdleTask(PView pView) : ViewTask(pView, curArrow) {}

ViewPanTask::ViewPanTask(PView pView) : ViewTask(pView, curPan) {}

void ViewPanTask::ButtonDown(int x, int y) {
  _LastPos.x = x;
  _LastPos.y = y;
}

void ViewPanTask::MouseMove(int x, int y) {
  int dx = x - _LastPos.x;
  int dy = _LastPos.y - y;
  if (dx != 0 || dy != 0) {
    _pView->PanBy(dx, dy);
    _LastPos.x = x;
    _LastPos.y = y;
#ifdef WIN32
    _SetCursor();
#endif
  }
}

ViewZoomTask::ViewZoomTask(PView pView) : ViewTask(pView, curZoom) {}

void ViewZoomTask::ButtonDown(int, int y) { _LastY = y; }

void ViewZoomTask::MouseMove(int, int y) {
  int dy = y - _LastY;
  if (dy != 0) {
    _pView->ZoomBy(dy);
    _LastY = y;
#ifdef WIN32
    _SetCursor();
#endif
  }
}

ViewFrustumTask::ViewFrustumTask(PView pView) : ViewTask(pView, curFov) {}

void ViewFrustumTask::ButtonDown(int, int y) { _LastY = y; }

void ViewFrustumTask::MouseMove(int, int y) {
  int dy = y - _LastY;
  if (dy != 0) {
    _pView->FrustumBy(dy);
    _LastY = y;
#ifdef WIN32
    _SetCursor();
#endif
  }
}

ViewRollTask::ViewRollTask(PView pView) : ViewTask(pView, curRotate) {}

void ViewRollTask::ButtonDown(int x, int) { _LastX = x; }

void ViewRollTask::MouseMove(int x, int) {
  int dx = _LastX - x;
  if (dx != 0) {
    _pView->RollBy(dx);
    _LastX = x;
#ifdef WIN32
    _SetCursor();
#endif
  }
}

ViewRotateTask::ViewRotateTask(PView pView) : ViewTask(pView, curRotate) {}

void ViewRotateTask::ButtonDown(int x, int y) {
  _LastPos.x = x;
  _LastPos.y = y;
}

void ViewRotateTask::MouseMove(int x, int y) {
  int dx = x - _LastPos.x;
  int dy = _LastPos.y - y;
  if (dx != 0 || dy != 0) {
    _pView->RotateBy(dx, dy);
    _LastPos.x = x;
    _LastPos.y = y;
#ifdef WIN32
    _SetCursor();
#endif
  }
}
