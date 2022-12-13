/* obstacles.c - implementation of primitives that can be loaded from a
   file outside of the plant model */

#include <stdio.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "scene3d.h"
#include "obstacles.h"

#define POLYGON 1
#define MATERIAL 6
#define PUSHMATRIX 7
#define POPMATRIX 8
#define TRANSLATE 9
#define ROTATE 10
#define SCALE 11
#define MULTMATRIX 12
#define TRIANGLE 15

typedef struct syntax_item {
  char flag;
  char *keyword;
};

struct syntax_item syn[] = {
    {POLYGON, "polygon"},       {TRIANGLE, "triangle"},
    {MATERIAL, "material"},     {PUSHMATRIX, "pushmatrix"},
    {POPMATRIX, "popmatrix"},   {TRANSLATE, "translate"},
    {ROTATE, "rotate"},         {SCALE, "scale"},
    {MULTMATRIX, "multmatrix"}, {-1, NULL} /* must be the last one */
};
