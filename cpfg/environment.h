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



#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include "movements.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAXFIELDS 10 /* maximum number of environmental fields */

struct FIELDPARAM {
  char type;
  short index;
  double ratio[3];
  double mid[3];
};

struct ENVIRONMENTPARAM {
  short number_of_movements;
  struct MOVEMENTPARAM movements[MAXMOVEMENTS];
  short number_of_fields;
  struct FIELDPARAM fields[MAXFIELDS];
};

/* prototypes */
void InitializeEnvironmentParam(void);
int field_initialize(short NTr, char *tmpfile);
double field(short index, double x, double y, double z);
double field_gx(short index, double x, double y, double z);
double field_gy(short index, double x, double y, double z);
double field_gz(short index, double x, double y, double z);
void FreeEnvironmentSpace(void);

extern struct ENVIRONMENTPARAM environmentparam;

#ifdef __cplusplus
}
#endif

#endif
