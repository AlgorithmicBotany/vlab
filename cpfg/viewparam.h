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



#ifndef __VIEWPARAM_H_
#define __VIEWPARAM_H_

enum tGenerateOnViewChange { eOff, eOn, eTriggered };

typedef enum tGenerateOnViewChange tGenerateOnViewChange;

struct VIEWPARAM {
  double view_normal[3];
  float viewpoint[3]; //default 0,0,1
  double view_up[3];
  float vrp[3]; //default 0,0,0

  /* viewing angle for perspective projection */
  float viewing_angle; //45
  float front_dist; // -100000
  float back_dist; // 100000
  float initscale; //1
  float scale; // 1
  float min[3];// 0,0,0
  float max[3]; // 1,1,1

  /* coefficient to convert real coordinates to pixels */
  float real_to_pixel;
  int twist;
  int auto_window;
  float xRotation;
  float yRotation;
  float zRotation; // MC - added for mouse roll
  float xPan;
  float yPan;
  float objectRotation[3]; /* around x,y,and z axis */
  float objectScale[3]; //1,1,1
  short int xStart;
  short int yStart;
  short int xLast;
  short int yLast;
  char parallel_projection_on;
  char depth_cue_on;
  char z_buffer_on; //on
#if CPFG_VERSION >= 6500
  char antialiasing_on; //off
#endif
  char objects_include_turtle; /* for rayshade output of instantiated
 objects */
  char objects_format[20];     /* format string for including parameters
 in the name of rayshade objects */
  float zoom_min; //0.05
  float zoom_max; //50
  tGenerateOnViewChange GenerateOnViewChange;
  char concavePolygons; // 0 - Tesselator is turned off, 1 - Tesselator is
                        // turned on

  char *backgroundFilename;

};

typedef struct VIEWPARAM VIEWPARAM;

/* The structure defining the screen rectangle */ // MC - Moved from interpret.h
                                                  // to this header file

struct RECTANGLE {
  float left, right, bottom, top;
};

typedef struct RECTANGLE RECTANGLE;

#endif
