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



#include "colour.h"

// ==================== Class: Colour =====================
// -------------------- Construction -------------------
Colour::Colour(GLfloat r, GLfloat g, GLfloat b) {
  rgb[0] = r;
  rgb[1] = g;
  rgb[2] = b;
  convertHSV();
}

// -------------------- Set Functions -------------------
void Colour::h(GLfloat H) {
  hsv[0] = H;
  convertRGB();
}

void Colour::s(GLfloat S) {
  hsv[1] = S;
  convertRGB();
}

void Colour::v(GLfloat V) {
  hsv[2] = V;
  convertRGB();
}

void Colour::r(GLfloat R) {
  rgb[0] = R;
  convertHSV();
}

void Colour::g(GLfloat G) {
  rgb[1] = G;
  convertHSV();
}

void Colour::b(GLfloat B) {
  rgb[2] = B;
  convertHSV();
}

void Colour::setrgb(GLfloat *c) {
  rgb[0] = c[0];
  rgb[1] = c[1];
  rgb[2] = c[2];
  convertHSV();
}

void Colour::sethsv(GLfloat H, GLfloat S, GLfloat V) {
  hsv[0] = H;
  hsv[1] = S;
  hsv[2] = V;
  convertRGB();
}

void Colour::setto(Colour *c) {
  hsv[0] = c->h();
  hsv[1] = c->s();
  hsv[2] = c->v();
  rgb[0] = c->r();
  rgb[1] = c->g();
  rgb[2] = c->b();
}

// -------------------- Conversion Functions --------------------

// Some interesting HSV Properties:
// --------------------
//         white: V=1, S=0, H=UNDEFINED
//         black: V=0, S irrelevent, H=UNDEFINED
//         greys: V e(0,1), S=0, H=UNDEFINED
// pure pigments: V=1, S=1, H e[0,360]

// Convert to RGB from HSV derived from algorithm in F&VD
void Colour::convertRGB() {
  int i;
  GLfloat f, p, q, t, h, s, v;
  h = hsv[0];
  s = hsv[1];
  v = hsv[2];
  if (h == UNDEFINED)
    rgb[0] = rgb[1] = rgb[2] = v;
  else {
    if (eq(h, 360.00))
      h = 0.00;
    h /= 60.00;
    i = (int)h; // floor
    f = h - i;  // remainder
    p = v * (1 - s);
    q = v * (1 - (s * f));
    t = v * (1 - (s * (1 - f)));
    switch (i) { // break down to 6 cases
    case 0:
      rgb[0] = v;
      rgb[1] = t;
      rgb[2] = p;
      break;
    case 1:
      rgb[0] = q;
      rgb[1] = v;
      rgb[2] = p;
      break;
    case 2:
      rgb[0] = p;
      rgb[1] = v;
      rgb[2] = t;
      break;
    case 3:
      rgb[0] = p;
      rgb[1] = q;
      rgb[2] = v;
      break;
    case 4:
      rgb[0] = t;
      rgb[1] = p;
      rgb[2] = v;
      break;
    case 5:
      rgb[0] = v;
      rgb[1] = p;
      rgb[2] = q;
      break;
    }
  }
}

// Convert to HSV from RGB derived from algorithm in F&VD
void Colour::convertHSV() {
  int max = 0, min = 0;
  GLfloat delta;

  if ((eq(rgb[0], 0.00)) && (eq(rgb[1], 0.00)) && (eq(rgb[2], 0.00))) {
    hsv[0] = UNDEFINED;
    hsv[1] = 0;
    hsv[2] = 0;
  } else if ((eq(rgb[0], 1.00)) && (eq(rgb[1], 1.00)) && (eq(rgb[2], 1.00))) {
    hsv[0] = UNDEFINED;
    hsv[1] = 0;
    hsv[2] = 1;
  } else {
    for (int i = 1; i < 3; i++) {
      if (rgb[i] < rgb[min])
        min = i;
      if (rgb[i] > rgb[max])
        max = i;
    }

    delta = rgb[max] - rgb[min];
    hsv[2] = rgb[max];
    if (!eq(hsv[2], 0.00))
      hsv[1] = delta / rgb[max];
    else
      hsv[1] = 0.00;
    if (eq(hsv[1], 0.00))
      hsv[0] = UNDEFINED;
    else {
      switch (max) {
      case 0: // between yellow and magenta
        hsv[0] = (rgb[1] - rgb[2]) / delta;
        break;
      case 1: // between cyan and yellow
        hsv[0] = 2 + ((rgb[2] - rgb[0]) / delta);
        break;
      case 2: // between magenta and cyan
        hsv[0] = 4 + ((rgb[0] - rgb[1]) / delta);
        break;
      }
      hsv[0] *= 60.00;

      while (hsv[0] < 0)
        hsv[0] += 360.00;
      while (hsv[0] > 360)
        hsv[0] -= 360.00;
    }
  }
}

// ================== Extra Functions ===================
// returns:{ true if a is within range of precision of b
//         { false ow
bool eq(GLfloat a, GLfloat b) {
  if ((a < (b + PRECISION)) && (a > (b - PRECISION)))
    return true;
  return false;
}
