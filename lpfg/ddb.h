/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __DDB_H__
#define __DDB_H__

class DDB {
public:
  DDB(HDC hdc, int w, int h) : _w(w), _h(h) {
    _hBmp = CreateCompatibleBitmap(hdc, _w, _h);
  }
  ~DDB() { DeleteObject(_hBmp); }
  operator HBITMAP() const { return _hBmp; }
  void SaveBMP(const char *fnm, HDC hdc) const;
  bool SaveRGB(const char *fnm, HDC hdc) const;

private:
  void _AdjustBytes(Utils::RawMemory &mem, int w, int h) const;
  int _w, _h;
  HBITMAP _hBmp;
};

#endif
