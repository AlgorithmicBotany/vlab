/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "Vector3.h"
#include "quaternion.h"
static const double epsilon = 0.0001f;
Vector3::Vector3() {
  x = 0;
  y = 0;
  z = 0;
}

// 2D constructor
Vector3::Vector3(double newx, double newy) {
  x = newx;
  y = newy;
  z = 0;
}

// 3D constructor
Vector3::Vector3(double newx, double newy, double newz) {
  x = newx;
  y = newy;
  z = newz;
}

Vector3::Vector3(const Vector3 &v) {
  x = v.X();
  y = v.Y();
  z = v.Z();
}

// Returns a normalized copy of the current vector
Vector3 Vector3::normalize() {
  Vector3d v = this->toVector3d();
  Vector3d norm = v.Normalize();
  return fromVector3d(norm);
  /*
  double length = getMagnitude();
  if (length == 0)
    return Vector3(x, y, z);
  double newx = x / length;
  double newy = y / length;
  double newz = z / length;
  return Vector3(newx, newy, newz);
  */
}

// Rotates the current vector about the given axis by the given angle
void Vector3::rotate(Vector3 axis, double angle) {
  //axis = axis.normalize();
  angle *= M_PI / 180.0; // degrees to radians

  
  Vector3d v = this->toVector3d();
  Vector3d axis3d = axis.toVector3d();
  if(std::abs(angle) > epsilon) {
    double halfang = -0.5 * angle;
    Quaternion q(cosf(halfang), sinf(halfang) * axis3d);
    v.RotateBy(q);
  }
  else if (angle < 0.f) {
    v = -1 * v;
  }
  x = v.X();
  y = v.Y();
  z = v.Z();

  return;
  
  x = std::round(v.X()*10000000.)/10000000.;
  y = std::round(v.Y()*10000000.)/10000000.;
  z = std::round(v.Z()*10000000.)/10000000.;
  
  return;

  /*
  std::cerr<<"Rotate 1: "<<x<<" - "<<y<<" - "<<z<<std::endl;
  
  axis = axis.normalize();
  angle *= M_PI / 180.0; // degrees to radians
  double cosA = cos(angle);
  double sinA = sin(angle);
  double omcA = 1 - cosA;
  // Insanely obtuse 3D vector rotation matrix in expanded form
  // (http://www.cprogramming.com/tutorial/3d/rotation.html)
  double newX = (omcA * pow(axis.x, 2) + cosA) * v.X() +
    ((omcA * axis.x * axis.y) + (sinA * axis.z)) * v.Y() +
    ((omcA * axis.x * axis.z) - (sinA * axis.y)) * v.Z();
  double newY = ((omcA * axis.x * axis.y) - (sinA * axis.z)) * v.X() +
    (omcA * pow(axis.y, 2) + cosA) * v.Y() +
    ((omcA * axis.y * axis.z) + (sinA * axis.x)) * v.Z();
  double newZ = ((omcA * axis.x * axis.z) + (sinA * axis.y)) * v.X() +
    ((omcA * axis.y * axis.z) - (sinA * axis.x)) * v.Y() +
    (omcA * pow(axis.z, 2) + cosA) * v.Z();
  std::cerr<<"Rotate 2: "<<newX<<" - "<<newY<<" - "<<newZ<<std::endl;
  */
}

// Returns the magnitude of the current vector
double Vector3::getMagnitude() {
  return pow((pow(x, 2) + pow(y, 2) + pow(z, 2)), 0.5);
}

// Static function to get the cross product of two vectors
Vector3 Vector3::cross(Vector3 u, Vector3 v) {
  Vector3 result;
  result.x = u.y * v.z - u.z * v.y;
  result.y = u.z * v.x - u.x * v.z;
  result.z = u.x * v.y - u.y * v.x;
  return result;
}

// Gets the angle between this vector and the given vector in radians
double Vector3::angleBetween(Vector3 vec) {
  return acos(normalize() * vec.normalize());
}

// Returns true if this vector is the zero vector
bool Vector3::isZero() { return (x == 0 && y == 0 && z == 0); }
