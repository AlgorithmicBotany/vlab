/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include "Vector3.h"
#include <math.h>

class Point {
private:
  double x, y, z;
public:
  double X()const{ return x;}
  double Y()const{ return y;}
  double Z()const { return z;}

  GLfloat Xf(){ return static_cast<GLfloat>(x);}
  GLfloat Yf(){ return static_cast<GLfloat>(y);}
  GLfloat Zf(){ return static_cast<GLfloat>(z);}

  
  void setX(const double newX){ x = newX;}
  void setY(const double newY){ y = newY;}
  void setZ(const double newZ){ z = newZ;}

  Point();
  Point(double x, double y);
  Point(double x, double y, double z);
  Point(const Point &p){
    x = p.X();
    y = p.Y();
    z = p.Z();
  }
  
  
  double distanceTo(Point a);
  Vector3 getNormal(Point a, Point b);
  void draw(double size);

  Point operator+(const Point &p) {
    double newX = x + p.X();
    double newY = y + p.Y();
    double newZ = z + p.Z();
    return Point(newX, newY, newZ);
  }
  Point &operator+=(const Point &p) {
    x += p.X();
    y += p.Y();
    z += p.Z();
    return *this;
  }

  Point operator+(const Vector3 &v) {
    double newX = x + v.X();
    double newY = y + v.Y();
    double newZ = z + v.Z();
    return Point(newX, newY, newZ);
  }

  Point &operator+=(const Vector3 &v) {
    x += v.X();
    y += v.Y();
    z += v.Z();
    return *this;
  }

Vector3
operator-(const Point &p) {
  double newX = x - p.X();
  double newY = y - p.Y();
  double newZ = z - p.Z();
  return Vector3(newX, newY, newZ);
}

  
Point operator-(const Vector3 &v) {
  double newX = x - v.X();
  double newY = y - v.Y();
  double newZ = z - v.Z();
  return Point(newX, newY, newZ);
}

Point operator*(const double c) {
  double newX = c * x;
  double newY = c * y;
  double newZ = c * z;
  return Point(newX, newY, newZ);
}

Point &operator*=(const double c) {
  x *= c;
  y *= c;
  z *= c;
  return *this;
}

 Point operator*=(const Vector3 &c) {
   x *= c.X();
   y *= c.Y();
   z *= c.Z();
  return *this;
}

Point operator/(const double c) {
  double newX = x / c;
  double newY = y / c;
  double newZ = z / c;
  return Point(newX, newY, newZ);
}

Point &operator/=(const double c) {
  x /= c;
  y /= c;
  z /= c;
  return *this;
}

bool operator==(const Point &p) { return (x == p.X()) && (y == p.Y()) && (z == p.Z()); }

bool operator!=(const Point &p) { return (x != p.X()) || (y != p.Y()) || (z != p.Z()); }
}
;
