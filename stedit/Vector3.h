/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <math.h>
#include "Globals.h"
#include "vector3d.h"
class Vector3d; 
class Vector3 {
private:
  double x, y, z;
public:
  double X()const{ return x;}
  double Y()const{ return y;}
  double Z()const{ return z;}

  GLfloat Xf(){ return static_cast<GLfloat>(x);}
  GLfloat Yf(){ return static_cast<GLfloat>(y);}
  GLfloat Zf(){ return static_cast<GLfloat>(z);}

  void setX(const double newX){ x = newX;}
  void setY(const double newY){ y = newY;}
  void setZ(const double newZ){ z = newZ;}

  Vector3d toVector3d(){
    return Vector3d(x,y,z);
  }
  Vector3 fromVector3d(const Vector3d &v){
    return Vector3(v.X(),v.Y(), v.Z());
  }
  Vector3();
  Vector3(const Vector3 &v);
  Vector3(double x, double y);
  Vector3(double x, double y, double z);
  Vector3 normalize();
  void rotate(Vector3 axis, double angle);
  double getMagnitude();
  static Vector3 cross(Vector3 u, Vector3 v);
  double angleBetween(Vector3 vec);
  bool isZero();

  Vector3 operator+(Vector3 v) {
    Vector3d v3d = v.toVector3d();
    Vector3d this3d = toVector3d();
    Vector3d sum = v3d + this3d;
    return fromVector3d(sum);
    /*    
    double newX = x + v.x;
    double newY = y + v.y;
    double newZ = z + v.z;
    return Vector3(newX, newY, newZ);
    */
  }

  Vector3 &operator+=(Vector3 &v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }

  Vector3 operator-(Vector3 v) {
    Vector3d v3d = v.toVector3d();
    Vector3d this3d = toVector3d();
    Vector3d sum = this3d - v3d;
    return fromVector3d(sum);
    /*
    double newX = x - v.x;
    double newY = y - v.y;
    double newZ = z - v.z;
    return Vector3(newX, newY, newZ);
    */
  }

  Vector3 operator*(double c) {
    Vector3d this3d = toVector3d();
    Vector3d prod =  c * this3d ;
    return fromVector3d(prod);
    /*
    double newX = c * x;
    double newY = c * y;
    double newZ = c * z;
    return Vector3(newX, newY, newZ);
    */
  }

  Vector3 operator/(double c) {
    Vector3d this3d = toVector3d();
    this3d /= c ;
    return fromVector3d(this3d);
    /*
    double newX = x / c;
    double newY = y / c;
    double newZ = z / c;
    return Vector3(newX, newY, newZ);
    */
  }

  // Dot product
  double operator*(Vector3 v) {
    Vector3d v3d = v.toVector3d();
    Vector3d this3d = toVector3d();
    double prod = this3d * v3d;
    return prod;

    //    return x * v.x + y * v.y + z * v.z;
  }
};
