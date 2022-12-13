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



#ifndef __TURTLE_H__
#define __TURTLE_H__

#include "movements.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The structure describing the turtle */

struct TURTLE {
  double position[3];
  double heading[3];
  double left[3];
  double up[3];
  float line_width;
  float elasticity;
  struct MOVEMENTPARAM2 movements[MAXMOVEMENTS];
  float scale_factor;
  int color_index;
  int color_index_back;
  double prev_heading[3]; /* for the previous segment */
  double prev_up[3];
  int texture;                /* current texture index */
  float tex_t;                /* current texture t coordinate */
  int contour;                /* current contour */
  float tangent_parameter[2]; /* parameters by which generalized cylinder
                                 tangents of a curve finishing (0) and
                                 starting (1) in a control point are
                                 multiplied */
  float radius_tangent[2][2]; /* tangents of a Hermite cubic interpolating
                                 radii at control points of a generalized
                                 cylinder */
  char scale_radius_tangents;
  char spline_flag;   /* drawing splines: F,f,G,g is interpreted as
                         for polygons */
  int cylinder_sides; /* number of polygons around the circumference */
};

typedef struct TURTLE TURTLE;

void adjust_up_and_left(TURTLE *turtle);

#ifdef __cplusplus
}
#endif

#endif
