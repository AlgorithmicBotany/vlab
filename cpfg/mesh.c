/********************************************************************/
/* MESHES
   triangulates meshes
 ********************************************************************/

#ifdef WIN32
#include "warningset.h"
#endif

#include <string.h>

#ifndef WIN32
#include <unistd.h>
#endif

#include "control.h"
#include "interpret.h"
#include "mesh.h"
#include "log.h"

float point1[2][PITEM];
float point2[2][PITEM]; /* two pairs of points (pt, normal, texels,
                           and color index) */

float *prev_pair;        /* previous pair */
float *curr_pair = NULL; /* current pair - NULL is default */
int indx;                /* first or second in a pair */

char first_pair;

/********************************************************************/
void voidStartTmesh(void) {}

void voidEndTmesh(void) {}

/********************************************************************/
/* Function: StartTmeshT                                            */
/* Initializes tmesh                                                */
/********************************************************************/

void StartTmeshT(void) {
  prev_pair = point1[0];
  curr_pair = point2[0];

  indx = 0;
  first_pair = 1; /* nothing is draw after the first pair of points */
}

/********************************************************************/
/* Function: TmeshVertexT                                           */
/* Stores current vertex and if possible outputs pair of triangles  */
/********************************************************************/

void TmeshVertexT(const float *point, const DRAWPARAM *dr) {
  float *tmp;

  if (curr_pair == NULL) {
    Message("Mesh not started!\n");
    return;
  }
  if (indx > 1) {
    indx = 0;
    tmp = curr_pair;
    curr_pair = prev_pair;
    prev_pair = tmp;
    first_pair = 0;
  }

  /* point */
  curr_pair[indx * PITEM + POINT_X] = point[POINT_X];
  curr_pair[indx * PITEM + POINT_Y] = point[POINT_Y];
  curr_pair[indx * PITEM + POINT_Z] = point[POINT_Z];

  if ((dr->gllighting) || (dr->ourlighting)) {
    curr_pair[indx * PITEM + NORMAL_X] = point[NORMAL_X];
    curr_pair[indx * PITEM + NORMAL_Y] = point[NORMAL_Y];
    curr_pair[indx * PITEM + NORMAL_Z] = point[NORMAL_Z];
  }

  if (dr->texture) {
    curr_pair[indx * PITEM + TEXTURE_S] = point[TEXTURE_S];
    curr_pair[indx * PITEM + TEXTURE_T] = point[TEXTURE_T];
  }

  curr_pair[indx * PITEM + COLOR_FRONT] = point[COLOR_FRONT];
  curr_pair[indx * PITEM + COLOR_BACK] = point[COLOR_BACK];
  curr_pair[indx * PITEM + DRAW_LINE] = 1;

  indx += 1;

  if ((indx > 1) && (!first_pair)) {
    /* draw two triangles */
    curr_pair[DRAW_LINE] = 0;
    dr->tdd->RenderTriangle(prev_pair, curr_pair, prev_pair + PITEM, dr);
    curr_pair[DRAW_LINE] = 1;

    prev_pair[PITEM + DRAW_LINE] = 0;
    /* second triangle */
    dr->tdd->RenderTriangle(prev_pair + PITEM, curr_pair, curr_pair + PITEM,
                            dr);
  }
}

/********************************************************************/
/* Function: EndTmeshT                                              */
/* ends tmesh                                                       */
/********************************************************************/

void EndTmeshT(void) { curr_pair = NULL; }
