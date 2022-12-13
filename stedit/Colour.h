/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <QColor>

class Colour {
public:
  double r, g, b;
  Colour();
  Colour(double red, double green, double blue);
  Colour(QColor colour);
  void clamp();
  QColor toQColor();

  Colour &operator+=(Colour &colour) {
    r += colour.r;
    g += colour.g;
    b += colour.b;
    return *this;
  }

  Colour operator+(Colour &colour) {
    Colour result = *this;
    result += colour;
    return result;
  }

  Colour &operator*=(double factor) {
    r *= factor;
    g *= factor;
    b *= factor;
    return *this;
  }

  Colour operator*(double factor) {
    Colour result = *this;
    result *= factor;
    return result;
  }

  Colour &operator*=(Colour &colour) {
    r *= colour.r;
    g *= colour.g;
    b *= colour.b;
    return *this;
  }

  Colour operator*(Colour &colour) {
    Colour result = *this;
    result *= colour;
    return result;
  }
};
