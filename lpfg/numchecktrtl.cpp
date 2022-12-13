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



#include "turtle.h"
#include "utils.h"

#ifdef _WINDOWS
#include <float.h>
#else
#include <math.h>
#endif

void NumericCheckTurtle::F(float v) {
  _Check(v, "F");
  Turtle::F(v);
}

void NumericCheckTurtle::f(float v) {
  _Check(v, "f");
  Turtle::f(v);
}

void NumericCheckTurtle::Left(float v) {
  _Check(v, "Left");
  Turtle::Left(v);
}

void NumericCheckTurtle::Right(float v) {
  _Check(v, "Right");
  Turtle::Right(v);
}

void NumericCheckTurtle::Up(float v) {
  _Check(v, "Up");
  Turtle::Up(v);
}

void NumericCheckTurtle::Down(float v) {
  _Check(v, "Down");
  Turtle::Down(v);
}

void NumericCheckTurtle::RollR(float v) {
  _Check(v, "RollR");
  Turtle::RollR(v);
}

void NumericCheckTurtle::RollL(float v) {
  _Check(v, "RollL");
  Turtle::RollL(v);
}

void NumericCheckTurtle::Line(V3f a, V3f b) { Turtle::Line(a, b); }

void NumericCheckTurtle::LineTo(V3f v) { Turtle::LineTo(v); }

void NumericCheckTurtle::LineRel(V3f v) { Turtle::LineRel(v); }

void NumericCheckTurtle::MoveTo(float x, float y, float z) {
  Turtle::MoveTo(x, y, z);
}

void NumericCheckTurtle::MoveRel(float dx, float dy, float dz) {
  Turtle::MoveRel(dx, dy, dz);
}

void NumericCheckTurtle::SetHead(const float *pF) { Turtle::SetHead(pF); }

void NumericCheckTurtle::SetHeadOnly(V3f hd) { Turtle::SetHeadOnly(hd); }

void NumericCheckTurtle::Sphere(float r) const{
  _Check(r, "Sphere");
  if (r < 0)
    Utils::Message("Sphere(%f)\n", r);
}

void NumericCheckTurtle::RotateXYZ(V3f axis, float angle) {
  float axis_length = axis.Length();
#ifdef WIN32
  if (!_finite(axis_length) || fabs(axis_length) < epsilon)
#else
  if (!std::isfinite(axis_length) || fabs(axis_length) < epsilon)
#endif
    Utils::Message("RotateXYZ: axis has length %f\n", axis_length);

#ifdef WIN32
  if (!_finite(angle))
#else
  if (!std::isfinite(angle))
#endif
    Utils::Message("RotateXYZ: angle of %f\n", axis_length);

  Turtle::RotateXYZ(axis, angle);
}

void NumericCheckTurtle::RotateHLU(V3f axis, float angle) {
  float axis_length = axis.Length();
#ifdef WIN32
  if (!_finite(axis_length) || fabs(axis_length) < epsilon)
#else
  if (!std::isfinite(axis_length) || fabs(axis_length) < epsilon)
#endif
    Utils::Message("RotateHLU: axis has length %f\n", axis_length);

#ifdef WIN32
  if (!_finite(angle))
#else
  if (!std::isfinite(angle))
#endif
    Utils::Message("RotateHLU: angle of %f\n", axis_length);

  Turtle::RotateHLU(axis, angle);
}

void NumericCheckTurtle::_Check(float v, const char *mname) const {
#ifdef WIN32
  if (!_finite(v))
#else
  if (!std::isfinite(v))
#endif
    Utils::Message("%s(%f)\n", mname, v);
  else if (fabs(v) < epsilon)
    Utils::Message("%s(%f)\n", mname, v);
}
