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



#ifndef _TURTLEDRAWDISPATCHER_H__
#define _TURTLEDRAWDISPATCHER_H__

#include "drawparam.h"
#include "viewparam.h"
#include "turtle.h"
#include "polygon.h"
#include "control.h"

#ifdef __cplusplus
extern "C" {
#endif
//[PASCAL] we have some issue with variables that are set to const
// this needs to be checked
struct turtleDrawDispatcher {
  int (*Setup)(TURTLE *, DRAWPARAM *, VIEWPARAM *);
  void (*StartNode)(TURTLE *, DRAWPARAM *, VIEWPARAM *, float length,
                    char symbol);
  void (*EndNode)(TURTLE *, DRAWPARAM *, VIEWPARAM *, char symbol);
  void (*StartBranch)(TURTLE *, DRAWPARAM *, VIEWPARAM *);
  void (*EndBranch)(TURTLE *, DRAWPARAM *, VIEWPARAM *);
  void (*StartPolygon)(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
  void (*EndPolygon)(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
  void (*SetColour)(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
  void (*SetLineWidth)(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
  void (*Circle2D)(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                   float radius);
  void (*Circle3D)(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                   float radius);
  void (*Sphere)(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                 float diameter);
  void (*BlackBox)(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                   const StringModule *module, const StringModule *submodule);
  void (*PredefinedSurface)(TURTLE *, DRAWPARAM *, VIEWPARAM *, char id,
                            double scaleX, double scaleY, double scaleZ);
  void (*LdefinedSurface)(StringModule *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
  void (*Label)(const TURTLE *, DRAWPARAM *, const VIEWPARAM *, const char *,
                int,
                const float *); // JH1
  void (*FinishUp)(TURTLE *, DRAWPARAM *, VIEWPARAM *);
  void (*RenderTriangle)(const float *, const float *, const float *,
                         const DRAWPARAM *);
  int (*StartTexture)(int);
  void (*EndTexture)(int);
  void (*StartTmesh)(void);
  void (*TmeshVertex)(const float *, const DRAWPARAM *);
  void (*EndTmesh)(void);
  void (*CircleB2D)(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                    float radius, float width);
  void (*CircleB3D)(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                    float radius, float width);
};

typedef struct turtleDrawDispatcher turtleDrawDispatcher;
#ifdef __cplusplus
}
#endif

#endif
