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



#ifndef __CANVAS_H__
#define __CANVAS_H__

class Canvas {
public:
  operator HDC() const { return _hdc; }
  void TextOut(int x, int y, const char *str, int len) {
    ::TextOut(_hdc, x, y, str, len);
  }

protected:
  Canvas() : _hdc(0) {}

  HDC _hdc;
};

class UpdateCanvas : public Canvas {
public:
  UpdateCanvas(HWND hwnd) : _hWnd(hwnd) { _hdc = GetDC(_hWnd); }
  ~UpdateCanvas() { ReleaseDC(_hWnd, _hdc); }

private:
  HWND _hWnd;
};

class PaintCanvas : public Canvas {
public:
  PaintCanvas(HWND hwnd) : _hwnd(hwnd) { _hdc = BeginPaint(_hwnd, &_ps); }
  ~PaintCanvas() { EndPaint(_hwnd, &_ps); }

private:
  HWND _hwnd;
  PAINTSTRUCT _ps;
};

class CurrentRC {
public:
  CurrentRC(const Canvas &cnv, HGLRC hrc) {
    assert(0 == _counter);
    if (!wglMakeCurrent(cnv, hrc))
      MessageBeep(0xFFFFFFFF);
#ifndef NDEBUG
    _counter++;
#endif
  }
  ~CurrentRC() {
#ifndef NDEBUG
    _counter--;
#endif
    wglMakeCurrent(NULL, NULL);
  }

private:
#ifndef NDEBUG
  static int _counter;
#endif
};

class Font {
public:
  Font(const char *fname, int size, bool italic, bool bold);
  ~Font();
  operator HFONT() const { return _hFont; }

private:
  HFONT _hFont;
};

class SelectGDIObject {
public:
  SelectGDIObject(HDC hdc, HGDIOBJ hObj) : _hdc(hdc) {
    _hPrevObj = SelectObject(_hdc, hObj);
  }
  ~SelectGDIObject() { SelectObject(_hdc, _hPrevObj); }

private:
  HDC _hdc;
  HGDIOBJ _hPrevObj;
};

#else
#error File already included
#endif
