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


GLfilledCircle::GLfilledCircle(void)
{
  double theta = 0.;
  for (int i = 0; i < 18; ++i) {
    _v[i][0] = cos(theta);
    _v[i][1] = sin(theta);
    theta += 0.349065850398866; // 20 degrees * pi/180, (20 = 360/18)
  }
}

void GLfilledCircle::Draw(WorldPoint center, double radius) const
{
  glPushMatrix();
  glTranslated(center.X(), center.Y(), center.Z());
  glScaled(radius, radius, radius);
  glBegin(GL_TRIANGLE_FAN);
  glVertex2d(0.,0.);
  for (int i = 0; i < 18; ++i)
    glVertex2dv(_v[i]);
  glVertex2dv(_v[0]);
  glEnd();
  glPopMatrix();
}

GLellipse::GLellipse(void)
{
  double theta = 0.;
  for (int i = 0; i < 18; ++i) {
    _v[i][0] = cos(theta);
    _v[i][1] = sin(theta);
    theta += 0.349065850398866; // 20 degrees * pi/180, (20 = 360/18)
  }
}

void GLellipse::Draw(WorldPoint center, double a, double b) const
{
  glPushMatrix();
  glTranslated(center.X(), center.Y(), center.Z());
  glScaled(a, b, 1.);
  glBegin(GL_LINE_LOOP);
  for (int i = 0; i < 18; ++i)
    glVertex2dv(_v[i]);
  //glVertex2dv(_v[0]);
  glEnd();
  glPopMatrix();
}
