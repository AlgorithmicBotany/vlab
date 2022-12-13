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



#ifndef __COMMON_H
#define __COMMON_H

#define FRAME_MIN_SIZE 12
#define FRAME_BORDER 5
#define SNAP_DELAY 500
#define SNAP_FILENAME "icon"
#define SNAPICON_W 120 // Snapicon control panel width
#define SNAPICON_H 45  // Snapicon control panel height
#define ACTUAL_W 140   // Snapframe default width
#define ACTUAL_H 140   // Snapframe default height

enum SizeMode { SizeActual, SizeDouble, SizeQuadruple, SizeAspect, SizeFree };

enum CursorState {
  OUT,
  IN,
  UP,
  DOWN,
  LEFT,
  RIGHT,
  UPLEFT,
  UPRIGHT,
  DOWNLEFT,
  DOWNRIGHT,
  MAIN
};

enum PixFormat { RGB, BMP, JPEG, PBM, PGM, PNG, PPM, XBM, XPM };

#endif
