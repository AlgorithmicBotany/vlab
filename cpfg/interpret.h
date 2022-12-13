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



#ifndef _CPFG_INTERPRET_
#define _CPFG_INTERPRET_

#include "environment.h"
#include "drawparam.h"
#include "viewparam.h"
#include "turtle.h"
#include "turtleDrawDispatcher.h"

/*
        Definitions of constants.
*/
#ifdef __cplusplus
extern "C" {
#endif

#define TWO_PI (M_PI * 2.0)
#define STEP_SIZE 1.0        /* the step length associated to F */
#define TURTLESTACK_SIZE 128 /* max depth of branches */

#define POLYSTACK_SIZE 5

/* The depth of the polystack and the max number of edges here
   are for polygons with fewer edges  at greater depth */

#define SIMPLE_FILL 1
#define INTERPOLATED_FILL 2
#define GOURAUD_SHADE 3
#define B_SPLINE 4
#define CLOSED_B_SPLINE 5
#define TWO_SIDED 6
#define WIREFRAME 7

/* new render modes */
enum {
  RM_WIREFRAME,
  RM_FILLED,
  RM_FLAT,
  RM_INTERPOLATED,
  RM_SHADED,
  RM_FAST,
  RM_SHADOWS, // MC - Oct. 15 - added shadows render mode
  RM_COUNT    /* must be the last one! */
};

/* line styles */
enum {
  LS_PIXEL,
  LS_POLYGON,
  LS_CYLINDER,
  LS_COUNT /* must be the last one! */
};

/* bicubic surface basis matrix id */
#define BEZIER 1
#define BSPLINE 2
#define CARDINAL 3

/* number of sides in the polygon drawn for circles */
#define CIRCLE_SIDES 10

/* type of rendering output */
enum {
  TYPE_ENVIRONMENT,
  TYPE_OPENGL,
  TYPE_RAYSHADE,
  TYPE_POSTSCRIPT,
  TYPE_VIEWVOLUME,
  TYPE_GLS,
  TYPE_OBJECT,
#if CPFG_VERSION >= 3200
  TYPE_SLAVE,
#endif
  TYPE_COUNT
};

struct SUBLSYSTEM {
  double id, scale;
  struct LSYSDATA_s *LsysPtr;
};

typedef struct SUBLSYSTEM SUBLSYSTEM;

void InitializeTurtle(DRAWPARAM *drawPtr, TURTLE *tu);
void Initialize_substr_turtle_array(void);
void Free_substr_turtle_array(void);

/* prototype from movements.c */
void movement_adjust(struct TURTLE *tu);

/* prototype from control.c */
void ProcessEnvironment(char *string, DRAWPARAM *drawparamPtr,
                        VIEWPARAM *viewparamPtr);

/* prototypes from interpret.c */
void InitializeStacks(void);
void FreeStacks(void);
int TurtleDrawInitialize(char *string, DRAWPARAM *drp, VIEWPARAM *vwp);
int TurtleDrawNsteps(int counter);
void TurtleDrawFinishUp(void);
void TurtleDraw(char *string, DRAWPARAM *dr, VIEWPARAM *vw);
void UpdateQueries(char *string, DRAWPARAM *dr, VIEWPARAM *vw);
void TurtleCopy(TURTLE *to, const TURTLE *from);
int ReadViewData(char *filename, DRAWPARAM *drawPtr, VIEWPARAM *viewPtr);
// return 1 if success, 0 otherwise;
#ifdef CONTEXT_SENSITIVE_HOMO
void InterpretHomomorphism(char c, char *curPtr, char **str, char *leftcontext,
                           char *rightcontext, int depth,
                           void ProcessSymbol(char c, char **str,
                                              char is_homo));
#else
void InterpretHomomorphism(char c, char *curPtr, char **str, int depth,
                           void ProcessSymbol(char c, char **str,
                                              char is_homo));
#endif
void InterpretSymbol(char c, char **str, char in_homo);

#ifdef __cplusplus
}
#endif

#endif /* _CPFG_INTERPRET_ */
