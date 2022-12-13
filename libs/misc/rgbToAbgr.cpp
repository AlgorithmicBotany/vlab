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



#include "rgbToAbgr.h"
#include "xmemory.h"
#include <stdlib.h>

byte *rgb_to_abgr(byte *oldImg, int width, int height) {
  byte *newImg;
  byte *ptrNew;
  byte *ptrOld;
  int i;

  newImg = (byte *)xmalloc(sizeof(byte) * 4 * width * height);
  if (newImg == NULL)
    return NULL;

  ptrNew = newImg;
  ptrOld = oldImg;
  for (i = 0; i < width * height; i++) {
    ptrNew[0] = 255;
    ptrNew[1] = ptrOld[2];
    ptrNew[2] = ptrOld[1];
    ptrNew[3] = ptrOld[0];
    ptrNew += 4;
    ptrOld += 3;

#ifdef DONT_COMPILE
    /* copy red */
    *ptrNew = *ptrOld;
    ptrNew++;
    ptrOld++;
    /* copy green */
    *ptrNew = *ptrOld;
    ptrNew++;
    ptrOld++;
    /* copy blue */
    *ptrNew = *ptrOld;
    ptrNew++;
    ptrOld++;
    /* set alpha */
    *ptrNew = 255;
    ptrNew++;
#endif
  }

  return newImg;
}

byte *rgb_to_rgba(byte *oldImg, int width, int height) {
  byte *newImg;
  byte *ptrNew;
  byte *ptrOld;
  int i;

  newImg = (byte *)xmalloc(sizeof(byte) * 4 * width * height);
  if (newImg == NULL)
    return NULL;

  ptrNew = newImg;
  ptrOld = oldImg;
  for (i = 0; i < width * height; i++) {
    ptrNew[0] = ptrOld[0];
    ptrNew[1] = ptrOld[1];
    ptrNew[2] = ptrOld[2];
    ptrNew[3] = 255;
    ptrNew += 4;
    ptrOld += 3;

#ifdef DONT_COMPILE
    /* copy red */
    *ptrNew = *ptrOld;
    ptrNew++;
    ptrOld++;
    /* copy green */
    *ptrNew = *ptrOld;
    ptrNew++;
    ptrOld++;
    /* copy blue */
    *ptrNew = *ptrOld;
    ptrNew++;
    ptrOld++;
    /* set alpha */
    *ptrNew = 255;
    ptrNew++;
#endif
  }

  return newImg;
}
