/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include <fstream>

#include <cassert>

#include <windows.h>

#include "utils.h"
#include "ddb.h"

void DDB::_AdjustBytes(Utils::RawMemory &mem, int w, int h) const {
  int pad = w * 3 % 4;
  assert(pad != 0);
  pad = 4 - pad;
  const int rowsz = w * 3 + pad;
  const char *bfsrc = mem + rowsz;
  char *bftrg = mem + 3 * w;
  for (int row = 1; row < h; ++row) {
    memmove(bftrg, bfsrc, 3 * w);
    bfsrc += rowsz;
    bftrg += 3 * w;
  }
}

void DDB::SaveBMP(const char *fnm, HDC hdc) const {
  BITMAPINFO bmi;
  bmi.bmiHeader.biSize = sizeof(BITMAPINFO);
  bmi.bmiHeader.biWidth = _w;
  bmi.bmiHeader.biHeight = _h;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biBitCount = 24;
  bmi.bmiHeader.biCompression = BI_RGB;

  Utils::RawMemory mem(_w * _h * 4);
  GetDIBits(hdc, _hBmp, 0, _h, mem, &bmi, DIB_RGB_COLORS);

  std::ofstream trg(fnm, std::ios::out | std::ios::binary);

  BITMAPFILEHEADER bmfh;
  {
    union {
      WORD w;
      char c[2];
    } u;
    u.c[0] = 'B';
    u.c[1] = 'M';
    bmfh.bfType = u.w;

    bmfh.bfSize =
        sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 3 * _w * _h;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
  }

  trg.write(reinterpret_cast<const char *>(&bmfh), sizeof(BITMAPFILEHEADER));

  trg.write(reinterpret_cast<const char *>(&bmi), sizeof(BITMAPINFOHEADER));

  trg.write(mem, mem.Size());
}
