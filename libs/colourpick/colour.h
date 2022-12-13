#ifndef COLOUR_H
#define COLOUR_H

#include <iostream>
#include <cmath>

#include <qgl.h>

#define UNDEFINED -1
#define PI 3.1415927 // mmm...pie...
#define PRECISION 0.001 

class Colour{
 public:
  Colour(GLfloat r=0,GLfloat g=0,GLfloat b=0);

  inline GLfloat h(){return hsv[0];};
  inline GLfloat s(){return hsv[1];};
  inline GLfloat v(){return hsv[2];};
  inline GLfloat r(){return rgb[0];};
  inline GLfloat g(){return rgb[1];};
  inline GLfloat b(){return rgb[2];};  
  inline GLfloat* cv(){return rgb;}; //do not use for assigning!!!
      
  void h(GLfloat);
  void s(GLfloat);
  void v(GLfloat);
  void r(GLfloat);
  void g(GLfloat);
  void b(GLfloat);
  void setrgb(GLfloat*);
  void sethsv(GLfloat,GLfloat,GLfloat);
  void setto(Colour*);

 private:
  void convertHSV();
  void convertRGB();

  GLfloat rgb[3];
  GLfloat hsv[3];
};

bool eq(GLfloat,GLfloat); // floating point equality test

#endif
