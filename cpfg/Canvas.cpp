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



#include "warningset.h"

#include <assert.h>
#include <windows.h>
#include <string.h>

#include "myexception.h"
#include "canvas.h"

#ifndef NDEBUG
int CurrentRC::_counter = 0;
#endif

Font::Font(const char *fontname, int size, bool italic, bool bold) {
  char bf[33];
  strncpy(bf, fontname, 32);
  bf[32] = 0;
  LOGFONT lf;
  {
    lf.lfHeight = -size;
    lf.lfWidth = lf.lfEscapement = lf.lfOrientation = 0;
    lf.lfWeight = bold ? FW_BOLD : FW_NORMAL;
    lf.lfItalic = italic;
    lf.lfUnderline = lf.lfStrikeOut = false;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH & FF_DONTCARE;
    strcpy(lf.lfFaceName, bf);
  }

  _hFont = CreateFontIndirect(&lf);

  if (NULL == _hFont)
    throw MyException("Cannot create font");
}

Font::~Font() { DeleteObject(_hFont); }
