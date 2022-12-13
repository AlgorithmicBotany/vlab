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



#ifndef __MOVEMENTS_H__
#define __MOVEMENTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#define OLDTROPISMS /* for compatibility reasons old stuff is still present */

#define MAXMOVEMENTS 10 /* Maximum number of used movements */

/* movement types */
enum eMovementType { mtTropism, mtTwist };

typedef enum eMovementType MovementType;

#define TROPISMW "tropism"
#define TWISTW "twist"

struct MOVEMENTPARAM {
  MovementType type;
  double dir[3];
  double gamma;
  double elasticity;
  double initial_elasticity; /* set in the view file */
  double elasticity_incr;
};

struct MOVEMENTPARAM2 { /* things which can be changed in productions */
  double elasticity;    /* and must be thus stored on the turtle's stack */
};

/*** prototypes ***/
int read_movement(char *tropims_type, int type);
void update_movement(short index, double delta_elast);
double get_double(void);

#ifdef __cplusplus
}
#endif

#endif
