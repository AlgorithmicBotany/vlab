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



#include <cassert>
#include <cmath>
#include <cstdlib>

#include <qgl.h>

#include "geometry.h"
#include "glutils.h"

#ifndef NDEBUG
bool GLprimitive::_exists = false;
#endif

void GLcircles::Record() {
  GLlist gllst(eListId, GL_COMPILE);
  GLlineloop gll;
  for (double alpha = 0.0; alpha < 2 * M_PI; alpha += M_PI / 30.0)
    glVertex2d(sin(alpha), cos(alpha));
}

void GLcircles::Draw(WorldPoint center, double radius) const {
  PushPopMatrix ppm;
  glTranslated(center.X(), center.Y(), center.Z());
  glScaled(radius, radius, radius);
  glCallList(eListId);
}
