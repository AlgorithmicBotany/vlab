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



#ifndef COLOUR_H
#define COLOUR_H

#include <cmath>
#include <iostream>

#include <qgl.h>

#define UNDEFINED -1
#define PI 3.1415927
#define PRECISION 0.001

class Colour {
public:
  Colour(GLfloat r = 0, GLfloat g = 0, GLfloat b = 0);

  inline GLfloat h() { return hsv[0]; };
  inline GLfloat s() { return hsv[1]; };
  inline GLfloat v() { return hsv[2]; };
  inline GLfloat r() { return rgb[0]; };
  inline GLfloat g() { return rgb[1]; };
  inline GLfloat b() { return rgb[2]; };
  inline GLfloat *cv() { return rgb; }; // do not use for assigning!!!

  void h(GLfloat);
  void s(GLfloat);
  void v(GLfloat);
  void r(GLfloat);
  void g(GLfloat);
  void b(GLfloat);
  void setrgb(GLfloat *);
  void sethsv(GLfloat, GLfloat, GLfloat);
  void setto(Colour *);

private:
  void convertHSV();
  void convertRGB();

  GLfloat rgb[3];
  GLfloat hsv[3];
};

bool eq(GLfloat, GLfloat); // floating point equality test

#endif
