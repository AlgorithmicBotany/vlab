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



#ifndef _DRAWPARAM_H_
#define _DRAWPARAM_H_

#ifdef __cplusplus
extern "C" {
#endif

struct turtleDrawDispatcher;

struct DRAWPARAM {
  struct turtleDrawDispatcher *tdd;

  float step_size; // default 1.0
  float scale_factor; // default 1.0
  float scale_multiplier; // default 1.0
  float angle_increment; //default 45
  double cosangle;
  double sinangle;
  float line_width; //default 1
  float width_increment; // default 0
  int cue_range; // default 0
  int shade_mode; //default SIMPLE_FILL
  char render_mode; //default RM_FILLED
  char line_style; // default LS_PIXEL
  char tapered_lines_on; //default 1
  float nurbs_sampling;
  float light_dir[3]; //default 1., 0., 0.
  int diff_coef; //default 10
  double tropism_dir[3]; //default 0,1,0
  float elasticity; //default 0
  float elasticity_increment; //default 0
  float ambient;        //ambient light for shading surfaces default 0.15
  float diffuse;        // diffuse light for shading surfaces default 0.75
  int color_index;      // for the front side of a surface default 128
  int color_index_back; // for the back side of a surface default 128
  int color_increment; // default 1
  int color_increment_back; //for the back side of a surface default 1
  char double_sided;        //are double-sided surfaces used? default 0
  char line_type; //default '~'
  char fontname[100]; //default -*-courier-bold-r-*-*-12-*-*-*-*-*-*-*
  int font_size;
  int italic;
  int bold;
  char *font_name;
  struct WinFont {
    char Name[80];// default Arial
    int size; // default 12
    char bold; // default 0
    char italic; // default 0
  } winfont; 
  int ps_linecap;     /* define the pattern of linecap (0 or 1) */ //default  1
  int cylinder_sides; /*number of polygons around the circumference */ // min 3
  char ourlighting;   /*make your own color computations */
  char gllighting;    /*output normals */
  char vertexbound;   /*output color or normal for each vertex */
  char texture;       /*output texture coordinates */
  char output_type;   /*where does the rendering go - screen, inventor, ... */
  char env_output_symbol; /*will the next symbol be output to the */
                          /* environment? */
  char gen_cyl_twist; // default 0
  int interpretation_step; //default -1
  char interpretation_past_cut; //default 1
  float rayshade_scale; // default 1.0
  char environment_display; /*process the environment before (1) or after */ //default 0
  /* (0 - default) the interpretation step */
  /* interpretation step  */

  // MC - Oct. 15 - added options for shadow map command
  int shadow_map_size; //default 1024
  float shadow_color[3]; // default 0.2,0.2,.04
  float shadow_offset[2]; // default 10.0
  int openGL_2;
};

typedef struct DRAWPARAM DRAWPARAM;

#ifdef __cplusplus
}
#endif

#endif
