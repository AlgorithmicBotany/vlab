/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "Colour.h"

Colour::Colour() {
  r = 0;
  g = 0;
  b = 0;
}

Colour::Colour(double red, double green, double blue) {
  r = red;
  g = green;
  b = blue;
}

// Converts the given QColor into a Colour
Colour::Colour(QColor colour) {
  r = (double)colour.red() / 255.0;
  g = (double)colour.green() / 255.0;
  b = (double)colour.blue() / 255.0;
}

// clamp the values to be between 0 and 1
void Colour::clamp() {
  if (r < 0)
    r = 0;
  else if (r > 1)
    r = 1;
  if (g < 0)
    g = 0;
  else if (g > 1)
    g = 1;
  if (b < 0)
    b = 0;
  else if (b > 1)
    b = 1;
}

QColor Colour::toQColor() { return QColor(r * 255, g * 255, b * 255); }
