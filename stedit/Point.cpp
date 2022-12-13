/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "Point.h"
#ifdef __APPLE__

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#else

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#endif

Point::Point() {
  x = 0;
  y = 0;
  z = 0;
}

// 2D constructor
Point::Point(double x, double y) {
  this->x = x;
  this->y = y;
  this->z = 0;
}

// 3D constructor
Point::Point(double x, double y, double z) {
  this->x = x;
  this->y = y;
  this->z = z;
}

// Get the distance to a point from this one
double Point::distanceTo(Point a) {
  double distance = pow(
			(pow((x - a.X()), 2.0) + pow((y - a.Y()), 2.0) + pow((z - a.Z()), 2.0)), 0.5);
  return distance;
}

// Get the normal based on the triangle formed by this point and the given
// neighbours
Vector3 Point::getNormal(Point a, Point b) {
  Vector3 v1 = a - *this;
  Vector3 v2 = b - *this;
  Vector3 c = Vector3::cross(v1, v2);
  c = c / c.getMagnitude();
  return c;
}

// Draw the point using the given size
void Point::draw(double size) {
  glPointSize(size);
  glBegin(GL_POINTS);
  glVertex3f(x, y, z);
  glEnd();
}
