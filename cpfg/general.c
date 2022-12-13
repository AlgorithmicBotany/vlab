
/* Generalized cylinders
   Specifed by control points (blackbox commands: @Gs - start spline
                                                  @Gc - continue
                                                  @Ge - end spline)

   Author: Radomir Mech  Dec 1994

   Modified: May 95 - the same behaviour as lines (pixel, polygon, cylinder)
                      polygon mode may need improvement
 */

#ifdef WIN32
#include "warningset.h"
#endif

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef LINUX
#include <limits.h>
#endif

#include "platform.h"
#include "generate.h"
#include "control.h"
#include "interpret.h"
#include "utility.h"
#include "quaternions.h"
#include "textures.h"
#include "general.h"

#include "test_malloc.h"

#define NOCONTOUR -1 /* id of an empty contour */

#define CONTOUR_DIM 3 /* maximum dimension of a contour */

typedef float CONTOUR_POINT[2 * CONTOUR_DIM]; /* two/three for the point,
                                                 two/three for its normal */
typedef float SOURCE_POINT[CONTOUR_DIM];

struct CONTOUR {
  int sides;
  CONTOUR_POINT *pts;
  struct CONTOUR *next;
};

typedef struct CONTOUR CONTOUR;

enum tagContourType { btRegular, btEndPoint };

typedef enum tagContourType ContourType;

struct CONTOUR_ARRAY {
  int id;
  int dim;     /* dimension: 2 or 3 */
  int num;     /* number of source control points */
  char closed; /* open or closed contour */
  ContourType type;
  signed char direction;
  SOURCE_POINT *source;
  CONTOUR *contours;
} contours[MAXCONTOURS] = {{0}};

/* global variables */
extern DRAWPARAM drawparam;
extern VIEWPARAM viewparam;

/* points are kept separately */
struct SPLINE {
  char type;    /* open or closed */
  char base;    /* Hermit/B-spline */
  int num;      /* num of processed control points  */
  int on_stack; /* num of control points on the stack */
  int default_rings;
  TURTLE tu; /* curent drawing point */
  float twist_angle;
};

typedef struct SPLINE SPLINE;

/* spline stack */

static SPLINE *spline_stack = NULL;
static int spline_stack_size = 0;
static int top_spline_stack;

/* control point stack */
/* very big, isn't it possible to store less than the full turtle?? */
typedef TURTLE CONTROLPOINT;
static CONTROLPOINT *point_stack = NULL;
static int point_stack_size = 0;
static int top_point_stack;

/* B-Spline matrix */
static const float m_B_spline[4][4] = {
    {-1.0f / 6.0f, 3.0f / 6.0f, -3.0f / 6.0f, 1.0f / 6.0f},
    {3.0f / 6.0f, -6.0f / 6.0f, 3.0f / 6.0f, 0.0f / 6.0f},
    {-3.0f / 6.0f, 0.0f / 6.0f, 3.0f / 6.0f, 0.0f / 6.0f},
    {1.0f / 6.0f, 4.0f / 6.0f, 1.0f / 6.0f, 0.0f / 6.0f}};
/* derivation of the previous (considering the T vector) for normals */
static const float m_B_spline_der[4][4] = {
    {0.0f / 2.0f, 0.0f / 6.0f, 0.0f / 6.0f, 0.0f / 6.0f},
    {-3.0f / 6.0f, 9.0f / 6.0f, -9.0f / 6.0f, 3.0f / 6.0f},
    {6.0f / 6.0f, -2.0f, 6.0f / 6.0f, 0.0f / 6.0f},
    {-3.0f / 6.0f, 0.0f / 6.0f, 3.0f / 6.0f, 0.0f / 6.0f}};

static struct {
  char filename[MaxCntrFname];
  int id;
} con_store[MAXCONTOURS];

static int con_stored;

/* prototypes */
static void interpolate_connection(SPLINE *spline, CONTROLPOINT *cntpts,
                                   TURTLE *turtle, int contours, DRAWPARAM *dr);
static int _ReadContour0000(const char *, FILE *, struct CONTOUR_ARRAY *);
static int _ReadContour0101(FILE *, struct CONTOUR_ARRAY *);
static int _ReadContour0102(FILE *, struct CONTOUR_ARRAY *);
static int _ReadContour0103(FILE *, struct CONTOUR_ARRAY *);

void PushPoint(const TURTLE *tu);
void PopPoint(TURTLE *tu);
CONTOUR *MakeContour(int index, int sides);
CONTOUR *FindContour(int index, int sides);

/***************************************************************************/
void reset_spline_stack(void) {
  /* allocate the spline stack if not done already */
  if (spline_stack == NULL) {
    if ((spline_stack = (SPLINE *)Malloc(TURTLESTACK_SIZE * sizeof(SPLINE))) ==
        NULL) {
      Message("Not enough memory for spline stack!\n");
      MyExit(0);
    }
    spline_stack_size = TURTLESTACK_SIZE;
  }

  top_spline_stack = 0;
  /* no control point */
  spline_stack[0].num = 0;
  /* nothing on the point stack */
  spline_stack[0].on_stack = 0;
  /* no type and base */
  spline_stack[0].type = 0;

  /* allocate the point stack if not done already */
  if (point_stack == NULL) {
    if ((point_stack = (CONTROLPOINT *)Malloc(2 * TURTLESTACK_SIZE *
                                              sizeof(CONTROLPOINT))) == NULL) {
      Message("Not enough memory for point stack!\n");
      MyExit(0);
    }
    point_stack_size = 2 * TURTLESTACK_SIZE;
  }

  top_point_stack = 0;
}

/***************************************************************************/
/* returns a valid index in the array of contours for a given id. If id is
   not found, the function returns the default circular contour (index 0)
   */
int get_contour_index(int id) {
  int i;

  for (i = 0; i < MAXCONTOURS; i++) {
    if (contours[i].id == NOCONTOUR)
      return DEF_CONTOUR; /* not found, use the default */

    if (contours[i].id == id)
      return i; /* found */
  }

  Message("Contour id %d not found, default used.\n", id);

  return DEF_CONTOUR; /* not found, use the default */
}

/***************************************************************************/
void FreeContours(void) {
  int i;
  CONTOUR *ptr, *ptr2;

  /* free contours */
  for (i = 0; i < MAXCONTOURS; i++) {
    contours[i].id = NOCONTOUR;
    if (contours[i].source != NULL) {
      Free(contours[i].source);
      contours[i].source = NULL;
    }

    ptr = contours[i].contours;

    while (ptr != NULL) {
      if (ptr->pts != NULL) {
        Free(ptr->pts);
        ptr->pts = NULL;
      }
      ptr2 = ptr;
      ptr = ptr->next;

      Free(ptr2);
      ptr2 = NULL;
    }

    contours[i].contours = NULL;
  }
}
/***************************************************************************/
void FreeSpline(void) {
  /* free spline stack */
  if (spline_stack != NULL) {
    Free(spline_stack);
    spline_stack = NULL;
  }

  if (point_stack != NULL) {
    Free(point_stack);
    point_stack = NULL;
  }

  FreeContours();
}

/***************************************************************************/
void evaluate_spline(SOURCE_POINT *pts, float *res, int dim, int num, float t,
                     const float mat[4][4]) {
  float tvec[4], vec[4], lt;
  int i, first, first2;
  int coord;

  first = (int)floor((double)t); /* index of the first control point */

  /* create the T vector */
  lt = t - first;
  tvec[3] = 1.0;

  for (i = 2; i >= 0; i--)
    tvec[i] = tvec[i + 1] * lt;

  /* multiply by the spline matrix */
  Vec4Mat4Mult(tvec, mat, vec);

  for (coord = 0; coord < dim; coord++) {
    first2 = first;

    /* multiply by the point coordinates */
    res[coord] = vec[0] * pts[first2][coord];

    for (i = 1; i < 4; i++) {
      if (first2 == num - 1)
        first2 = 0;
      else
        first2++;

      res[coord] += vec[i] * pts[first2][coord];
    }
  }
}

/********************************************************************/
CONTOUR *FindContour(int index, int sides) {
  CONTOUR *cont;
  extern DRAWPARAM drawparam;

  if ((cont = contours[index].contours) == NULL)
    Warning("Contours not created!\n", INTERNAL_LVL);

  for (;;) {
    if (cont->sides == sides)
      break;

    if (cont->sides > sides || cont->next == NULL) {
      if ((cont = MakeContour(index, sides)) == NULL) {
        /* get the closest existing value */
        /* for now, get the default one */
        cont = NULL;
      }

      return cont;
    }
    cont = cont->next;
  }

  return cont;
}

/***************************************************************************/
CONTOUR *MakeContour(int index, int sides) {
  float *lengths; /* array for storing partial lengths */
  int n = 10;     /* number of points per one unit of t */
  float invn, segments;
  float lastlen, lastP[CONTOUR_DIM], actP[CONTOUR_DIM], rot[2][2];
  float step, len;
  int i, j, j2, c, numpoints;
  float t, invlen, aux;
  int num = contours[index].num;
  int dim = contours[index].dim;
  CONTOUR *cont;
  CONTOUR **contour, *ptr;
  float angle;

  if (sides < 3)
    return NULL;

  if (sides > 1024)
    Message("Warning: the number of polygons around a generalized cylinder "
            " is %d!",
            sides);

  if (contours[index].contours != NULL) {
    /* find a position for the contour */
    contour = &contours[index].contours;

    while (*contour != NULL && (*contour)->sides < sides)
      *contour = (*contour)->next;

    ptr = *contour;

    if (ptr != NULL && ptr->sides == sides)
      /* contour already exists */
      return ptr;

    if ((*contour = Malloc(sizeof(CONTOUR))) == NULL) {
      Message("Cannot allocate memory for a contour\n");
      return NULL;
    }

    (*contour)->next = ptr;
    ptr = *contour; /* to save on referencing */
  } else {
    if ((contours[index].contours = Malloc(sizeof(CONTOUR))) == NULL) {
      Message("Cannot allocate memory for a contour\n");
      return NULL;
    }

    ptr = contours[index].contours;
    ptr->next = NULL;
  }

  cont = ptr;
  cont->sides = sides;

  if ((cont->pts = (CONTOUR_POINT *)Malloc((sides + 1) *
                                           sizeof(CONTOUR_POINT))) == NULL)
    Warning("Not enough memory for contour points\n", INTERNAL_LVL);

  if (contours[index].id == DEF_CONTOUR) {
    /* contour points */
    step = (float)(2.0 * M_PI) / (float)sides;
    angle = 0;

    for (i = 0; i < sides; i++) {
      cont->pts[i][0] = (float)cos(angle);
      cont->pts[i][1] = (float)sin(angle);

      cont->pts[i][0 + CONTOUR_DIM] = (float)cos(angle);
      cont->pts[i][1 + CONTOUR_DIM] = (float)sin(angle);

      /*if(2 < CONTOUR_DIM) <-- this is always true */
      {
        cont->pts[i][2] = 0;
        cont->pts[i][5] = 0;
      }

      angle += step;
    }

    if (contours[index].closed) {
      /* the last point is equal to the first */
      for (c = 0; c < 2 * CONTOUR_DIM; c++)
        cont->pts[sides][c] = cont->pts[0][c];
    }

    return cont;
  }

  invn = 1.0f / (float)n; /* step between two points */

  if ((lengths = (float *)Malloc(num * n * sizeof(float))) == NULL)
    return 0;

  /* fill the array of partial lengths */
  lastlen = 0.0; /* to make the first length 0 */

  evaluate_spline(contours[index].source, lastP, dim, num, 0.0, m_B_spline);

  if (contours[index].closed)
    segments = (float)num;
  else
    segments = (float)(num - 3);

  for (i = 0; i < segments; i++) {
    t = (float)i;
    for (j = 0; j < n; j++, t += invn) {
      evaluate_spline(contours[index].source, actP, dim, num, t, m_B_spline);

      len = 0;
      /* compute the length only in the first two dimensions */
      for (c = 0; c < 2; c++) {
        lastP[c] -= actP[c];
        len += lastP[c] * lastP[c];
      }

      lastlen = lengths[i * n + j] = lastlen + (float)sqrt(len);

      for (c = 0; c < dim; c++)
        lastP[c] = actP[c];
    }
  }

  if (contours[index].closed) {
    /* distance of the last point (lastP) and the first point -
                to get the total contour length */
    evaluate_spline(contours[index].source, actP, dim, num, 0.0, m_B_spline);

    len = 0;
    /* compute the length only in first two dimensions */
    for (c = 0; c < 2; c++) {
      lastP[c] -= actP[c];
      len += lastP[c] * lastP[c];
    }

    lastlen += (float)sqrt(len);
  }

  /* step along the contour to get numincontour contour points */
  step = lastlen / (float)sides;

  if (contours[index].closed) {
    /* make sure that the first point is on axis (1,0) */
    /* to achieve this determine the proper rotation.
    This roatation will not change the possible third coordinate. */
    rot[0][0] = actP[0];
    rot[0][1] = -actP[1];

    if ((invlen = rot[0][0] * rot[0][0] + rot[0][1] * rot[0][1]) <
        0.000000001) {
      /* no rotation */
      rot[0][0] = rot[1][1] = 1;
      rot[1][0] = rot[0][1] = 0;
    } else {
      invlen = 1.0f / (float)sqrt(invlen);
      rot[0][0] *= invlen;
      rot[0][1] *= invlen;

      rot[1][0] = -rot[0][1];
      rot[1][1] = rot[0][0];
    }
  } else {
    /* no rotation for open contours */
    rot[0][0] = rot[1][1] = 1;
    rot[1][0] = rot[0][1] = 0;
  }

  lastlen = 0.0;
  j = 0;

  if (contours[index].closed)
    numpoints = sides;
  else
    numpoints = sides + 1;

  for (i = 0; i < numpoints; i++) {
    if (i == 0) {
      /* find the first length[j] ,j>=0, that is above 0 */
      while (lengths[j] < 1e-10 / step && j < segments * n - 1)
        j++;

      t = 0;
    } else {
      while (lengths[j] < lastlen && j < segments * n - 1)
        j++;

      /* find the first length[j2] ,j2<j, which is lower than length[j] */
      j2 = j - 1;

      while (j2 > 0 && lengths[j] - lengths[j2] <= 1e-10 / step)
        j2--;

      t = (float)(j2) / (float)n;

      /* interpolate between the two neighbouring values */
      t += (lastlen - lengths[j2]) / (lengths[j] - lengths[j2]) / n;
    }

    /* contour point */
    evaluate_spline(contours[index].source, actP, dim, num, t, m_B_spline);

    /* rotate properly (the possible third coordinate stays the same) */
    for (c = 0; c < 2; c++)
      cont->pts[i][c] = actP[0] * rot[0][c] + actP[1] * rot[1][c];

    if (dim > 2)
      cont->pts[i][2] = actP[2];
    else
      cont->pts[i][2] = 0;

    /* contour normal - perpendicular to derivation vector */
    /* possible third coordinate is 0 */
    evaluate_spline(contours[index].source, actP, 2, num, t, m_B_spline_der);
    aux = actP[1];
    actP[1] = -actP[0];
    actP[0] = aux;

    /* rotate properly (the possible third coordinate stays the same)  */
    for (c = 0; c < 2; c++)
      cont->pts[i][c + CONTOUR_DIM] = actP[0] * rot[0][c] + actP[1] * rot[1][c];

    if ((invlen = cont->pts[i][CONTOUR_DIM] * cont->pts[i][CONTOUR_DIM] +
                  cont->pts[i][CONTOUR_DIM + 1] *
                      cont->pts[i][CONTOUR_DIM + 1]) == 0) {
      if (t == 0)
        t += 0.01f;
      else
        t -= 0.01f;

      evaluate_spline(contours[index].source, actP, 2, num, t, m_B_spline_der);
      aux = actP[1];
      actP[1] = -actP[0];
      actP[0] = aux;

      /* rotate properly (the possible third coordinate stays the same)  */
      for (c = 0; c < 2; c++)
        cont->pts[i][c + CONTOUR_DIM] =
            actP[0] * rot[0][c] + actP[1] * rot[1][c];

      /* even moving by a bit didn't help */
      if ((invlen = cont->pts[i][CONTOUR_DIM] * cont->pts[i][CONTOUR_DIM] +
                    cont->pts[i][CONTOUR_DIM + 1] *
                        cont->pts[i][CONTOUR_DIM + 1]) == 0) {
        Message("Warning! contour normal is zero!\n");
        invlen = 1;
      }
    }

    /* normalize */
    invlen = 1.0f / (float)sqrt(invlen);

    cont->pts[i][CONTOUR_DIM] *= invlen;
    cont->pts[i][CONTOUR_DIM + 1] *= invlen;

    cont->pts[i][CONTOUR_DIM + 2] = 0;

    lastlen += step;
  }

  if (contours[index].closed) {
    /* the last point is equal to the first */
    for (c = 0; c < 2 * CONTOUR_DIM; c++)
      cont->pts[sides][c] = cont->pts[0][c];
  }

  Free(lengths);
  lengths = NULL;
  return cont;
}

// [Pascal] keep opening the file until the size is stable
static FILE *testOpenFile(char *fname) {

#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *control_fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((control_fp == NULL) && (counter < 1000)) {
    control_fp = fopen(fname, "rb");
    counter++;
  }
  if (counter == 1000)
    fprintf(stderr,
            "WARNING testOpenFile: Can't open the file %s - using defaults.\n",
            fname);
  else {
    fseek(control_fp, 0, SEEK_END); // seek to end of file
    size = ftell(control_fp);       // get current file pointer

    while ((size == 0) || (current_size != size)) {
      current_size = size;
      fclose(control_fp);

      control_fp = fopen(fname, "r");
      while (control_fp == NULL) {
        control_fp = fopen(fname, "r");
        counter++;
      }
      fseek(control_fp, 0, SEEK_END); // seek to end of file
      size = ftell(control_fp);       // get current file pointer
    }
  }
  fseek(control_fp, 0L, SEEK_SET);
#else
  FILE *control_fp = fopen(fname, "rb");
#endif

  return control_fp;
}

/***************************************************************************/
/* Reads in the contour control points and create  numincontour contour
   points uniformly distributed around the contour
*/
int process_contour_file(char *filename, int index) {
  FILE *fp;
  if ((fp = testOpenFile(filename)) == NULL) {
    Message("Cannot open contour file %s.\n", filename);
    return 0;
  }

  char line[256]; /*, *token;*/
  int vmaj, vmin, r, side, i;
  struct CONTOUR_ARRAY *pContour = NULL;
  VERBOSE("Filename: %s\n", filename);
  /* open the contour file */
  if (fp == NULL) {
    Message("Cannot open the contour file %s.\n", filename);
    return 0;
  }
  /* get the first line */
  if (fgets(line, sizeof(line), fp) == NULL) {
    Message("Contour file %s is empty!\n", filename);
    fclose(fp);
    return 0;
  }

  if (2 == sscanf(line, "cver %d %d", &vmaj, &vmin)) {
    int version = 100 * vmaj + vmin;
    switch (version) {
    case 101:
      _ReadContour0101(fp, &(contours[index]));
      break;
    case 102:
      _ReadContour0102(fp, &(contours[index]));
      break;
    case 103:
      _ReadContour0103(fp, &(contours[index]));
      break;
    default:
      Message("Unknown contour format %d.%d in file\n", version / 100,
              version % 100, filename);
      return 0;
    }
  } else
    _ReadContour0000(line, fp, &(contours[index]));

  pContour = &(contours[index]);
  /* check the orientation of points, should be clockwise */
  side = 0;

  for (i = 1; i < pContour->num; i++) {
    /* the equation for the line defined by the two control points p1, p2
        is:  (p1y-p2y)x + (p2x-p1x)y + p1x.p2y - p1y.p2x = 0
        Now, test whether point [0,0] is on the left or right of this line:
        that is if (p1x.p2y - p1y.p2x) >0.
                add 1 for each true, subtract 1 for each false */
    if (pContour->source[i - 1][0] * pContour->source[i][1] -
            pContour->source[i - 1][1] * pContour->source[i][0] >
        0)
      side++;
    else
      side--;
  }
  /* also for the last pair */
  if (pContour->source[pContour->num - 1][0] * pContour->source[0][1] -
          pContour->source[pContour->num - 1][1] * pContour->source[0][0] >
      0)
    side++;
  else
    side--;

  if (side <= 0)
    pContour->direction = (char)(-1);
  else
    pContour->direction = (char)(1);

#ifdef XXXX
  /* if necessary, switch the order of points */
  if (side <= 0) {
    Message("Switching order of contour points.\n");
    /* for open contours, switch the order of points */
    /* for closed contours, make sure the first point stays 0 */
    for (i = 0; i < (num - 1) / 2; i++)
      for (c = 0; c < 3; c++) {
        aux = pContour->source[i + 1][c];
        pContour->source[i + 1][c] = pContour->source[num - 1 - i][c];
        pContour->source[num - 1 - i][c] = aux;
      }
  }
#endif

  /* create desired number of contour points and normals */
  if (MakeContour(index, drawparam.cylinder_sides) == NULL)
    r = 0;
  else
    r = 1;

  fclose(fp);

  return r;
}

static int _ReadContour0000(const char *firstline, FILE *fp,
                            struct CONTOUR_ARRAY *pContour) {
  int num, dim, i, r = 0;
  char type[9];
  /* getting the number of contour points and dimension */
  i = sscanf(firstline, "%d %d %8s", &num, &dim, type);
  if (i != 3) {
    return 0;
  }

  if (!(strcmp(type, "closed")))
    pContour->closed = 1;
  else
    pContour->closed = 0;

  VERBOSE("Contour dimension: %d\n", dim);

  /* maybe getting the type of the spline ??? */

  /* allocate the space for control points */
  assert(NULL == pContour->source);
  if ((pContour->source = (SOURCE_POINT *)Malloc(num * sizeof(SOURCE_POINT))) ==
      NULL) {
    Message("Cannot allocate memory for contour points!\n");
    return 0;
  }

  /* read all control points */
  for (i = 0; i < num; i++) {
    if (dim == 2) {
      if (fscanf(fp, "%f %f", &pContour->source[i][0],
                 &pContour->source[i][1]) == 2)
        continue;
      if (dim < CONTOUR_DIM)
        pContour->source[i][2] = 0.0;
    } else if (fscanf(fp, "%f %f %f", &pContour->source[i][0],
                      &pContour->source[i][1], &pContour->source[i][2]) == 3)
      continue;

    Message("Failed reading %d-th contour point!\n", i);
    r = 0;
    goto ret;
  }
  /* num point coordinates are in */

  pContour->dim = dim;
  pContour->num = num;

ret:;
  return r;
}

static int _ReadContour0101(FILE *fp, struct CONTOUR_ARRAY *pContour) {
  char line[128];
  int n1, pts, i, n;
  fgets(line, 127, fp); /* skip name: nnn */
  fgets(line, 127, fp);
  if (2 != sscanf(line, "points: %d %d", &n1, &pts)) {
    return 0;
  }
  fgets(line, 127, fp);
  /* type: open|closed */
  if (!strncmp(line + 6, "open", 4))
    pContour->closed = 0;
  else
    pContour->closed = 1;
  pContour->source = (SOURCE_POINT *)Malloc(pts * sizeof(SOURCE_POINT));
  if (NULL == pContour->source) {
    Message("Out of memory\n");
    return 0;
  }
  n = 0;
  for (i = 0; i < n1; i++) {
    float x, y, z;
    int m, res;
    res = fscanf(fp, "%f %f %f %d", &x, &y, &z, &m);
    if (res != 4) {
      Message("Error reading contour\n");
      return 0;
    }
    for (res = 0; res < m; res++) {
      pContour->source[n][0] = x;
      pContour->source[n][1] = y;
      pContour->source[n][2] = z;
      n++;
    }
  }
  pContour->dim = 3;
  pContour->num = pts;

  return 1;
}

static int _ReadContour0102(FILE *fp, struct CONTOUR_ARRAY *pContour) {
  char line[128];
  char type[10];
  int n1, pts, i, n;
  fgets(line, 127, fp); /* skip name: nnn */
  fgets(line, 127, fp);
  if (2 != sscanf(line, "points: %d %d", &n1, &pts)) {
    return 0;
  }
  fgets(line, 127, fp);
  if (1 != sscanf(line, "type: %10s", type)) {
    return 0;
  }
  if ('o' == type[0])
    pContour->closed = 0;
  else if ('c' == type[0])
    pContour->closed = 1;
  else {
    return 0;
  }
  if ('r' == type[1])
    pContour->type = btRegular;
  else if ('e' == type[1]) {
    pContour->type = btRegular;
    Message("Contours with end point interpolation not supported. Regular "
            "assumed\n");
  } else {
    return 0;
  }

  pContour->source = (SOURCE_POINT *)Malloc(pts * sizeof(SOURCE_POINT));
  if (NULL == pContour->source) {
    Message("Out of memory\n");
    return 0;
  }
  n = 0;
  for (i = 0; i < n1; i++) {
    float x, y, z;
    int m, res;
    res = fscanf(fp, "%f %f %f %d", &x, &y, &z, &m);
    if (res != 4) {
      Message("Error reading contour\n");
      return 0;
    }
    for (res = 0; res < m; res++) {
      pContour->source[n][0] = x;
      pContour->source[n][1] = y;
      pContour->source[n][2] = z;
      n++;
    }
  }
  pContour->dim = 3;
  pContour->num = pts;

  return 1;
}

static int _ReadContour0103(FILE *fp, struct CONTOUR_ARRAY *pContour) {
  char line[128];
  char type[10];
  int n1, pts, i, n;
  fgets(line, 127, fp); /* skip name: nnn */
  fgets(line, 127, fp);
  if (2 != sscanf(line, "points: %d %d", &n1, &pts)) {
    return 0;
  }
  fgets(line, 127, fp);
  if (1 != sscanf(line, "type: %10s", type)) {
    return 0;
  }
  if ('o' == type[0])
    pContour->closed = 0;
  else if ('c' == type[0])
    pContour->closed = 1;
  else {
    return 0;
  }
  if ('r' == type[1])
    pContour->type = btRegular;
  else if ('e' == type[1]) {
    pContour->type = btRegular;
    Message("Contours with end point interpolation not supported. Regular "
            "assumed\n");
  } else {
    return 0;
  }

  {
    int smpl;
    fgets(line, 127, fp);
    if (1 != sscanf(line, "samples: %d", &smpl)) {
      return 0;
    }
  }
  
  // ignore background
  {
    char c = fgetc(fp);
    if (c == 'b') {
      fgets(line, 128,fp);
      fgets(line, 128,fp);
    }
    ungetc(c, fp);
    c = fgetc(fp);
    if (c == 'm') {
      int markers;
      fscanf(fp, "markers: %d\n", &markers);
      for (int i = 0; i < markers; i++) {
	fgets(line, 128,fp);
      }
    }
    ungetc(c,fp);
  
  }

  

  pContour->source = (SOURCE_POINT *)Malloc(pts * sizeof(SOURCE_POINT));
  if (NULL == pContour->source) {
    Message("Out of memory\n");
    return 0;
  }
  n = 0;
  for (i = 0; i < n1; i++) {
    float x, y, z;
    int m, res;
    res = fscanf(fp, "%f %f %f %d", &x, &y, &z, &m);
    if (res != 4) {
      Message("Error reading contour\n");
      return 0;
    }
    for (res = 0; res < m; res++) {
      pContour->source[n][0] = x;
      pContour->source[n][1] = y;
      pContour->source[n][2] = z;
      n++;
    }
  }
  pContour->dim = 3;
  pContour->num = pts;

  return 1;
}

/***************************************************************************/
/* called from ReadViewdata. Allows to process contours after the viewfile is
   read and parameter cylinder_sides is known.
*/
void store_contour(char *filename, int id) {
  if (con_stored == MAXCONTOURS - 1) {
    Message("Warning! Maximal number of contours reached!\n");
    return;
  }

  strncpy(con_store[con_stored].filename, filename, MaxCntrFname);
  strncpy(clp.contourFileName[con_stored], con_store[con_stored].filename,
          MaxCntrFname);
  con_store[con_stored].filename[MaxCntrFname - 1] = 0;

  con_store[con_stored].id = id;
  con_stored++;
  clp.con_stored = con_stored;
}

/***************************************************************************/
int read_in_contour(char *filename, int id) {
  double angle = 0.0;
  float step;
  int i, index;
  extern DRAWPARAM drawparam;

  /* find the first available index */
  index = 0;

  while ((index < MAXCONTOURS)) {
    if (contours[index].id != NOCONTOUR)
      index++;
    else
      break;
  }

  if (index == MAXCONTOURS) {
    Message("Warning! No room for this contour!\n");
    return 0;
  }

  contours[index].id = id;
  if (index == DEF_CONTOUR) {
    contours[index].num = 4;
    contours[index].dim = 2;
    contours[index].closed = 1;

    /* default settings, not from a file */
    if ((contours[index].source = (SOURCE_POINT *)Malloc(
             (contours[index].num + 1) * sizeof(SOURCE_POINT))) == NULL) {
      Warning("Cannot initialize splines. Out of memory!\n", FATAL_LVL);
      return 0;
    }

    step = (float)(2.0 * M_PI) / contours[index].num;

    /* contour points */
    for (i = 0; i < contours[index].num; i++) {
      if (id == DEF_CONTOUR) {
        contours[index].source[i][0] = (float)cos(angle);
        contours[index].source[i][1] = (float)sin(angle);
      } else {
        /* this was used just for testing */
        contours[index].source[i][0] =
            (float)(cos(angle) * (1 + 0.2 * sin(angle * 5)));
        contours[index].source[i][1] =
            (float)(sin(angle) * (1 + 0.2 * sin(angle * 5)));
      }

      if (contours[index].dim < CONTOUR_DIM)
        contours[index].source[i][2] = 0;

      angle += step;
    }

    contours[index].direction = 1;
    MakeContour(index, drawparam.cylinder_sides);
    return 1;
  }
  /* otherwise read a contour from a file */
  if (!process_contour_file(filename, index)) {
    /* if there is something wrong with the file input */
    contours[index].id = NOCONTOUR;
    Free(contours[index].source);
    contours[index].source = NULL;
    return 0;
  }
  return 1;
}

/***************************************************************************/
/* global initialization done at the beginning of each interpretation step */
void InitializeSplines(void) {
  int i;

  for (i = 0; i < MAXCONTOURS; i++)
    contours[i].id = NOCONTOUR;

  con_stored = 0; /* number of contours read in the view file */
  clp.con_stored = 0;
  reset_spline_stack();
}

/***************************************************************************/
int process_contours(void) {
  int i;
  FreeContours();

  /* set values of the basic circular contour (points and their normals) */
  read_in_contour((char *)NULL, 0); /* no filename needed */
  /* process all contours */
  for (i = 0; i < con_stored; i++) {
    if (read_in_contour(con_store[i].filename, con_store[i].id) == 0)
      return 0;
  }
  return 1;
}

/***************************************************************************/
/* fills a contour: numincontour pairs of point (3 floats) and its normal
  (3 floats). Interpolates between two contours for tu1 and tu2 accorning
  to the value of the current radius
 */
void make_contour(float *r, const TURTLE *tu, int contour1, int contour2,
                  float length, float radius,  /* current radius */
                  const float *radius_tangent, /* 2D radius tangent */
                  float t,                     /* current t */
                  const DRAWPARAM *dr) {
  int i, j, dim;
  float pt[CONTOUR_DIM], aux, dir;
  CONTOUR *cont1, *cont2;

  /* get the higher of the two dimensions */
  dim = contours[contour1].dim > contours[contour2].dim
            ? contours[contour1].dim
            : contours[contour2].dim;

  cont1 = FindContour(contour1, tu->cylinder_sides);
  cont2 = FindContour(contour2, tu->cylinder_sides);
  if (cont1->sides != cont2->sides)
    Warning("Internal error: contour mismatch!\n", INTERNAL_LVL);

  if (t == 0 && contours[contour1].direction != contours[contour2].direction)
    Message("Warning: mixing clockwise and counter-clokwise contours! "
            "The meshes will be twisted!\n");

  if (dr->gllighting)
    dir = 1.0; /* OpenGL considers also the order of vertices */
  else
    dir = contours[contour1].direction; /* but we nee the normal pointing
        out in both cases */

  for (i = 0; i <= cont1->sides; i++) {
    /* interpolate contour vertices */
    for (j = 0; j < dim; j++)
      pt[j] = radius * (cont1->pts[i][j] * (1 - t) + cont2->pts[i][j] * t);

    for (j = 0; j < 3; j++)
      *(r + i * PITEM + POINT_X + j) =
          (float)(tu->position[j] + pt[0] * tu->up[j] - pt[1] * tu->left[j]);

    if (dim > 2)
      for (j = 0; j < 3; j++)
        *(r + i * PITEM + POINT_X + j) +=
            (float)(pt[2] * tu->heading[j] / length);

    if (dr->gllighting || dr->ourlighting) {
      /* interpolate contour normals */
      for (j = 0; j < dim; j++)
        pt[j] = radius * (cont1->pts[i][CONTOUR_DIM + j] * (1 - t) +
                          cont2->pts[i][CONTOUR_DIM + j] * t);

      for (j = 0; j < 3; j++)
        *(r + i * PITEM + NORMAL_X + j) =
            (float)(dir * (-radius_tangent[0] *
                           (pt[0] * tu->up[j] - pt[1] * tu->left[j])));

      /* the following statements may not be necessary */
      /*      if(dim>2)
      for(j=0; j<3; j++)
      *(r+ i*PITEM +NORMAL_X+j) += pt[2] * tu->heading[j];*/

      Normalize(r + i * PITEM + NORMAL_X);
    }

    if (dr->texture) {
      /* texels */
      aux = 1.0f - (float)i / (float)cont1->sides;
      if (aux < 0)
        aux = 0;

      *(r + i * PITEM + TEXTURE_S) = aux;

      *(r + i * PITEM + TEXTURE_T) = tu->tex_t;
    }
  }
}

/***************************************************************************/
void strip_to_mesh(float *contour1, const TURTLE *tu1, float *contour2,
                   const TURTLE *tu2, const DRAWPARAM *dr) {
  int i, sides;
  void (*TmeshVertex)(const float *, const DRAWPARAM *) = dr->tdd->TmeshVertex;
  short tu_col = tu1->color_index;
  short tu_col_back = tu1->color_index_back;
  short col1 = tu1->color_index;
  short col2;
  short col1_back = tu1->color_index_back;
  short col2_back;
  float cos_theta;
  float *p1;
  float *p2;

  col2 = (dr->render_mode == RM_INTERPOLATED) ? tu2->color_index : col1;
  col2_back =
      (dr->render_mode == RM_INTERPOLATED) ? tu2->color_index_back : col1_back;

  dr->tdd->StartTmesh();

  p1 = contour1;
  p2 = contour2;

  sides = FindContour(tu1->contour, tu1->cylinder_sides)->sides;

  for (i = 0; i <= sides; i++) {
    /* always two points */
    if (dr->ourlighting) {
      /* color must be interpolated */
      cos_theta = -(*(p1 + NORMAL_X) * dr->light_dir[0] +
                    *(p1 + NORMAL_Y) * dr->light_dir[1] +
                    *(p1 + NORMAL_Z) * dr->light_dir[2]);
      col1 = tu_col + (short)((float)dr->diff_coef * cos_theta);
      col1_back = tu_col_back + (short)((float)dr->diff_coef * cos_theta);

      /* color must be interpolated */
      cos_theta = -(*(p2 + NORMAL_X) * dr->light_dir[0] +
                    *(p2 + NORMAL_Y) * dr->light_dir[1] +
                    *(p2 + NORMAL_Z) * dr->light_dir[2]);
      col2 = tu_col + (short)((float)dr->diff_coef * cos_theta);
      col2_back = tu_col_back + (short)((float)dr->diff_coef * cos_theta);
    }

    p1[COLOR_FRONT] = col1;
    p1[COLOR_BACK] = col1_back;
    p2[COLOR_FRONT] = col2;
    p2[COLOR_BACK] = col2_back;

    (*TmeshVertex)(p2, dr);
    (*TmeshVertex)(p1, dr);

    p1 += PITEM;
    p2 += PITEM;
  }

  dr->tdd->EndTmesh();
}

/* ------------------------------------------------------------------------- */
/* adjusts the up and left vector to keep the segments from twist
   so the angle of prev_up with plane (prev_H,new_H) should be same as the
   angle of new_U with the same plane. Basically, a rotation moving prev_H
   to new_H is computed and prev_U is rotated by it (to get new_U).
     Angle beta subsequently rotates the up vector (for smoothly twisting
   shapes).

   */

void adjust_up_and_leftS(TURTLE *turtle_new, TURTLE *turtle_prev, float beta) {
  double rot[3];
  double len, angle, bendsin;
  double q[4];

  turtle_new->up[0] = turtle_prev->up[0];
  turtle_new->up[1] = turtle_prev->up[1];
  turtle_new->up[2] = turtle_prev->up[2];

  DCrossProduct(turtle_new->heading, turtle_prev->heading, rot);

  if ((len = sqrt(rot[0] * rot[0] + rot[1] * rot[1] + rot[2] * rot[2])) > 0.0) {

    angle = asin(len) * 0.5; /* angle of rotation around vec /2 */

    len = 1.0 / len;
    rot[0] *= len;
    rot[1] *= len;
    rot[2] *= len; /* the rotation is around rot - necessary to normalize */

    q[0] = cos(angle);
    q[1] = (bendsin = sin(angle)) * rot[0];
    q[2] = bendsin * rot[1];
    q[3] = bendsin * rot[2];

    rot_by_quat(turtle_new->up, q);
  }

  /* now perform rotation by angle beta around heading vector */
  if (beta != 0.0) {
    q[0] = cos(beta = beta * 0.5);
    q[1] = (bendsin = sin(beta)) * turtle_new->heading[0];
    q[2] = bendsin * turtle_new->heading[1];
    q[3] = bendsin * turtle_new->heading[2];

    rot_by_quat(turtle_new->up, q);
  }

  /* from up and heading get left */
  DCrossProduct(turtle_new->heading, turtle_new->up, turtle_new->left);
}

/***************************************************************************/
/* calculate angle by which the up vector of tu2 has to be rotated to
   minimize the twist with respect to tu1 */
float get_twist_angle(TURTLE *tu1, TURTLE *tu2) {
  double rot[3], up[3];
  double len, angle, bendsin;
  double q[4];

  up[0] = tu1->up[0];
  up[1] = tu1->up[1];
  up[2] = tu1->up[2];

  DCrossProduct(tu2->heading, tu1->heading, rot);

  if ((len = sqrt(rot[0] * rot[0] + rot[1] * rot[1] + rot[2] * rot[2])) >
      0.000001) {

    /* angle of rotation around vec /2 */
    angle = asin(len) * 0.5;

    len = 1.0 / len;
    rot[0] *= len;
    rot[1] *= len;
    rot[2] *= len; /* the rotation is around rot - necessary to normalize */

    q[0] = cos(angle);
    q[1] = (bendsin = sin(angle)) * rot[0];
    q[2] = bendsin * rot[1];
    q[3] = bendsin * rot[2];

    rot_by_quat(up, q);
  }
  /* find the angle of up and tu2->up */

  DCrossProduct(tu2->up, up, rot);
  len = sqrt(rot[0] * rot[0] + rot[1] * rot[1] + rot[2] * rot[2]);

  if (len > 1)
    len = 1.0;
  angle = asin(len);

  if (DDotProduct(tu2->up, up) < 0.0)
    angle = M_PI - angle;

  if (DDotProduct(tu2->left, up) < 0.0)
    angle = -angle;

  return angle;
}

/***************************************************************************/
/* Initialize a single spline */
void InitializeSpline(TURTLE *tu, int type, int rings) {
  if (type <= 0 || type > 4) {
    Message("Warning! Parameter following { is above 4!\n");
    return;
  }

  tu->spline_flag = 1;

  if (spline_stack == NULL || point_stack == NULL)
    reset_spline_stack();

  /* so far no control point */
  spline_stack[top_spline_stack].num = 0;

  /* nothing on the point stack */
  spline_stack[top_spline_stack].on_stack = 0;

  spline_stack[top_spline_stack].default_rings =
      rings <= 0 ? DEFAULT_RINGS : rings;

  /* determine the type and base */
  spline_stack[top_spline_stack].type = (type - 1) % SPLINE_TYPES;
  spline_stack[top_spline_stack].base = (type - 1) / SPLINE_TYPES;
}

/***************************************************************************/
/* Finish a single spline */
void FinishSpline(TURTLE *tu, DRAWPARAM *dr) {
  int i, num;

  if (!tu->spline_flag) {
    Message("Warning! Spline is already finished or hasn't been started!\n");
    return;
  }

  if (spline_stack[top_spline_stack].type == SPLINE_TYPE_CLOSED) {
    /* close the spline */

    /* get the number of the control points necessary to close the curve */
    switch (spline_stack[top_spline_stack].base) {
    case SPLINE_BASE_HERMIT:
      num = 1;
      break;
    case SPLINE_BASE_BSPLINE:
      num = 3;
      break;
    default:
      assert(!"Invalid stack base");
      num = -1;
    }

    for (i = 0; i < num; i++)
      SetControlPoint(
          &point_stack[top_point_stack -
                       spline_stack[top_spline_stack].on_stack + i + 1],
          dr, 0);
  }

  tu->spline_flag = 0;

  /* clean the point stack */
  for (i = 0; i < spline_stack[top_spline_stack].on_stack; i++)
    PopPoint(NULL);
  spline_stack[top_spline_stack].on_stack = 0;
}

/***************************************************************************/
/* set control point */
void SetControlPoint(TURTLE *tu, DRAWPARAM *dr, int contours) {
  int start_num, i;
  TURTLE turtle = *tu;

  if (!tu->spline_flag) {
    Message("Warning. "
            "."
            " not interpreted. Not inside polygon nor"
            " a generalized cylinder.\n");
    return;
  }

  if (contours == 0)
    contours = spline_stack[top_spline_stack].default_rings;

  switch (spline_stack[top_spline_stack].base) {
  case SPLINE_BASE_HERMIT:

    /* if it is the very first point, store it in the spline structure */
    if (spline_stack[top_spline_stack].num == 0) {
      spline_stack[top_spline_stack].tu = turtle;

      spline_stack[top_spline_stack].num++;

      /* if the curve is closed, push the point also on the point stack */
      if (spline_stack[top_spline_stack].type == SPLINE_TYPE_CLOSED) {
        PushPoint(tu);
        spline_stack[top_spline_stack].on_stack++;
      }

      /* return, no drawing yet */
      return;
    }

    start_num = 1;
    break;

  case SPLINE_BASE_BSPLINE:
    start_num = 3;

    if (spline_stack[top_spline_stack].num < start_num) {
      /* just add a new point to the stack */
      PushPoint(tu);
      spline_stack[top_spline_stack].num++;
      spline_stack[top_spline_stack].on_stack++;

      /* return, no drawing yet */
      return;
    }
    break;
  default:
    assert(!"Invalid value of spline base");
    start_num = -1;
  }

  /* draw the segment */

  switch (dr->line_style) {
  case LS_PIXEL:
    break;

  case LS_CYLINDER:
    dr->texture = is_valid_texture_index(tu->texture);
    /* no break ! */

  case LS_POLYGON:
    if (dr->gen_cyl_twist)
      /* get angle to prevent twist */
      spline_stack[top_spline_stack].twist_angle =
          get_twist_angle(&spline_stack[top_spline_stack].tu, &turtle);
    else {
      spline_stack[top_spline_stack].twist_angle = 0;
      adjust_up_and_leftS(&turtle, &spline_stack[top_spline_stack].tu, 0.0);
    }
    break;
  }

  if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol))
    interpolate_connection(&spline_stack[top_spline_stack],
                           &point_stack[top_point_stack - start_num + 1],
                           &turtle, contours, dr);

  spline_stack[top_spline_stack].tu = turtle;

  if (spline_stack[top_spline_stack].base == SPLINE_BASE_BSPLINE) {
    /* if the spline is closed and the number of processed
        points is just reaching the number of points necessary to store
        to be able to close the curve, keep the first start_num points
                on the stack and add start_num new points */
    if (spline_stack[top_spline_stack].type == SPLINE_TYPE_CLOSED &&
        spline_stack[top_spline_stack].num == start_num) {
      /* double the first start_num points for closing */

      /*
              WHAT AN UGLY HACK !!!
      */
      for (i = 0; i < start_num; i++)
        PushPoint(NULL);

      /* use all points except the first one */
      for (i = 0; i < start_num - 1; i++)
        point_stack[top_point_stack - 1 - i] =
            point_stack[top_point_stack - start_num - i];

      point_stack[top_point_stack] = *tu;
      spline_stack[top_spline_stack].on_stack += start_num;

      /* /END_HACK */
    } else {
      /* shift the last start_num points */
      for (i = 0; i < start_num - 1; i++)
        point_stack[top_point_stack - start_num + i + 1] =
            point_stack[top_point_stack - start_num + i + 2];

      point_stack[top_point_stack] = *tu;
    }
  }

  spline_stack[top_spline_stack].num++;
}

/***************************************************************************/
/* Initializing splines using @Gs (only hermite splines) */
void start_spline(TURTLE *turtle, DRAWPARAM *dr) {
  InitializeSpline(turtle,
                   1 + SPLINE_TYPE_OPEN + SPLINE_TYPES * SPLINE_BASE_HERMIT, 0);

  /* not to interpret f and F */
  turtle->spline_flag = 2;

  SetControlPoint(turtle, dr, 0);
}

/***************************************************************************/
/* Setting a control point using @Gc (only hermite splines) */
void continue_spline(TURTLE *turtle, DRAWPARAM *dr, int contours) {
  if (!turtle->spline_flag)
    InitializeSpline(
        turtle, 1 + SPLINE_TYPE_OPEN + SPLINE_TYPES * SPLINE_BASE_HERMIT, 0);

  SetControlPoint(turtle, dr, contours);
}

/***************************************************************************/
/* Finishing a spline using @Ge (only hermite splines) */
void finish_spline(TURTLE *turtle, DRAWPARAM *dr, int contours) {
  if (!turtle->spline_flag)
    InitializeSpline(
        turtle, 1 + SPLINE_TYPE_OPEN + SPLINE_TYPES * SPLINE_BASE_HERMIT, 0);

  SetControlPoint(turtle, dr, contours);

  FinishSpline(turtle, dr /*, spline_stack[top_spline_stack].type*/);
}

/***************************************************************************/
void PushPoint(const TURTLE *tu) {
  if (point_stack == NULL)
    reset_spline_stack();

  if (++top_point_stack == point_stack_size) {
    point_stack_size += 2 * TURTLESTACK_SIZE;
    Message("Point stack full. Reallocating to %d items.\n", point_stack_size);

    if ((point_stack = (CONTROLPOINT *)Realloc(
             point_stack, point_stack_size * sizeof(CONTROLPOINT))) == NULL)
      Warning("Not enough memory for point stack!\n", FATAL_LVL);
  }

  if (tu != NULL)
    point_stack[top_point_stack] = *tu;
}

/***************************************************************************/
void PopPoint(TURTLE *tu) {
  if (top_point_stack == -1) {
    Message("Warning! The point stack is already empty (pop ignored!)\n");
  } else {
    if (tu != NULL)
      *tu = point_stack[top_point_stack];
    top_point_stack--;
  }
}

/***************************************************************************/
void PushSpline(const TURTLE *tu) {
  int i;

  assert(NULL != tu);

  /* push only if there is a spline started */
  if (tu->spline_flag) {
    TURTLE toPush;
    if (spline_stack == NULL)
      reset_spline_stack();

    if (++top_spline_stack == spline_stack_size) {
      spline_stack_size += TURTLESTACK_SIZE;
      Message("Spline stack full. Reallocating to %d items.\n",
              spline_stack_size);

      if ((spline_stack = (SPLINE *)Realloc(
               spline_stack, spline_stack_size * sizeof(SPLINE))) == NULL)
        Warning("Not enough memory for spline stack!\n", FATAL_LVL);
    }

    if (top_spline_stack > 0)
      spline_stack[top_spline_stack] = spline_stack[top_spline_stack - 1];

    toPush = point_stack[top_point_stack - spline_stack[top_spline_stack].num];

    for (i = 0; i < spline_stack[top_spline_stack].num; i++)
      PushPoint(&toPush);
  }
}

/***************************************************************************/
void PopSpline(TURTLE *tu) {
  int i;

  assert(NULL != tu);

  /* pop only if there was a spline when the branch was started */
  if (tu->spline_flag) {
    if (top_spline_stack == -1) {
      Message("Warning! The spline stack already empty (pop ignored!)\n");
    } else {
      top_spline_stack--;

      for (i = 0; i < spline_stack[top_spline_stack].num; i++)
        PopPoint(NULL);
    }
  }
}

/***************************************************************************/
static float *contour1 = NULL;
static int in_contour1;

static float *contour2 = NULL;
static int in_contour2;

void UpdateContourArraySize(int con, float **cont1, float **cont2) {
  if (contour1 == NULL) {
    if ((contour1 = (float *)Malloc(PITEM * con * sizeof(float))) == NULL)
      Warning("Not enough emory for a contour.\n", INTERNAL_LVL);
    in_contour1 = con;
  } else if (in_contour1 < con) {
    if ((contour1 = (float *)Realloc(contour1, PITEM * con * sizeof(float))) ==
        NULL)
      Warning("Not enough emory for a contour.\n", INTERNAL_LVL);
    in_contour1 = con;
  }

  if (contour2 == NULL) {
    if ((contour2 = (float *)Malloc(PITEM * con * sizeof(float))) == NULL)
      Warning("Not enough emory for a contour.\n", INTERNAL_LVL);
    in_contour2 = con;
  } else if (in_contour2 < con) {
    if ((contour2 = (float *)Realloc(contour2, PITEM * con * sizeof(float))) ==
        NULL)
      Warning("Not enough emory for a contour.\n", INTERNAL_LVL);
    in_contour2 = con;
  }

  *cont1 = contour1;
  *cont2 = contour2;
}

/***************************************************************************/
/* generates a bunch of triangles to make a smooth connection between two given
   turtle's position. The heading vector is used to make the connection
   continuous with respect to possible previous parts.
   Delta width is necessary to make smooth width changes as well.

   Hermit curves (Pi points, Ri tangents):
   Q(t) = (2t^3-3t^2+1)P1 + (-2t^3+3t^2)P2 + (T^3-2t^2+t)R1+(t^3-t^2)R2
   Q(t) = (2.P1-2.P2+R1+R2)t^3 + (-3.P1+3.P2-2R1-R2)t^2 +
          (R1)t + P1.1

 */
extern float m_bezier[4][4];

static void interpolate_connection(SPLINE *spline, CONTROLPOINT *cntpts,
                                   TURTLE *turtle, int contours,
                                   DRAWPARAM *dr) {
  float *contour1ptr = NULL, *contour2ptr = NULL, *tmpptr = NULL;
  double len, len2;
  int i, j;
  int con_index1, con_index2, con;
  int texture;
  TURTLE tu1;
  TURTLE tu2, tu_end;
  float radius;
  float radius_tangent[2];

  float r, rstep, t;
  float pts[3][4]; /* coefficients for the Hermit polynomial
for three position coordinates */
  float rad[4][2]; /* coefficients for the Hermit polynomial
for radius and corresponding t */
  float tmp;
  float *contour1, *contour2;

  /* make sure the arrays contour1 and contour2 are big enough */
  con = turtle->cylinder_sides > spline->tu.cylinder_sides
            ? turtle->cylinder_sides
            : spline->tu.cylinder_sides;
  con++;

  UpdateContourArraySize(con, &contour1, &contour2);

  /* current position of the drawing turtle */
  tu1 = spline->tu;

  switch (spline->base) {
  case SPLINE_BASE_HERMIT:
    tu2 = *turtle;
    break;

  case SPLINE_BASE_BSPLINE:
    if (spline->num == 3) {

      /* compute even tu1 */
      tu1 = cntpts[1];

      /* 1/6[(1-t)^3, 3t^3 - 6t^2 + 4, -3t^3 + 3t^2 + 3t + 1, t^3] */
      for (i = 0; i < 3; i++)
        tu1.position[i] = (cntpts[0].position[i] + 4 * cntpts[1].position[i] +
                           cntpts[2].position[i]) /
                          6.0;

      /* derivation */
      /* 1/6[-3(1-t)^2, 9t^2 - 12t, -9t^2 + 6t + 3, 3t^2] */
      for (i = 0; i < 3; i++)
        tu1.heading[i] =
            (-3 * cntpts[0].position[i] + 3 * cntpts[2].position[i]) / 6.0;

      /* is the length equal to 0? This can happen in case few control
points coincide */
      if ((len2 = sqrt(tu1.heading[0] * tu1.heading[0] +
                       tu1.heading[1] * tu1.heading[1] +
                       tu1.heading[2] * tu1.heading[2])) > 0.0000001) {
        /* if not, normalize the vector */
        for (i = 0; i < 3; i++)
          tu1.heading[i] /= len2;

        adjust_up_and_leftS(&tu1, &cntpts[1], 0);
      } else {
        /* otherwise, use the heading vector of the second control point */
        tu1.heading[0] = cntpts[1].heading[0];
        tu1.heading[1] = cntpts[1].heading[1];
        tu1.heading[2] = cntpts[1].heading[2];
      }
    }
    spline->tu = tu1;

    tu2 = cntpts[2];
    /* compute tu2 using the Bezier Matrix */
    /* 1/6[(1-t)^3, 3t^3 - 6t^2 + 4, -3t^3 + 3t^2 + 3t + 1, t^3] */
    for (i = 0; i < 3; i++)
      tu2.position[i] = (cntpts[1].position[i] + 4 * cntpts[2].position[i] +
                         turtle->position[i]) /
                        6.0;

    /* derivation */
    /* 1/6[-3(1-t)^2, 9t^2 - 12t, -9t^2 + 6t + 3, 3t^2] */
    for (i = 0; i < 3; i++)
      tu2.heading[i] =
          (-3 * cntpts[1].position[i] + 3 * turtle->position[i]) / 6.0;

    /* is the length equal to 0? This can happen in case few control
points coincide */
    if ((len2 = sqrt(tu2.heading[0] * tu2.heading[0] +
                     tu2.heading[1] * tu2.heading[1] +
                     tu2.heading[2] * tu2.heading[2])) > 0.0000001) {
      /* if not, normalize the vector */
      for (i = 0; i < 3; i++)
        tu2.heading[i] /= len2;

      adjust_up_and_leftS(&tu2, &cntpts[2], spline->twist_angle);
    } else {
      /* otherwise, use the heading vector of the third control point */
      tu2.heading[0] = cntpts[2].heading[0];
      tu2.heading[1] = cntpts[2].heading[1];
      tu2.heading[2] = cntpts[2].heading[2];
    }

    break;
  }

  /* textured according to tu1 (the first control point for Hermite curve,
  or the second in case of B-splines */
  texture = tu1.texture;

  /* scale everything up with the distance */
  len = sqrt(DDistance(tu1.position, tu2.position));

  if (len < 0.00001)
    return;

  glFrontFace(GL_CCW);

  /* times tangent parameter to get a visualy nice bending */
  /* from a point */
#if CPFG_VERSION < 3400
  tu1.heading[0] *= len * tu2.tangent_parameter[0];
  tu1.heading[1] *= len * tu2.tangent_parameter[0];
  tu1.heading[2] *= len * tu2.tangent_parameter[0];
  /* to a point */
  tu2.heading[0] *= len * tu2.tangent_parameter[1];
  tu2.heading[1] *= len * tu2.tangent_parameter[1];
  tu2.heading[2] *= len * tu2.tangent_parameter[1];
#else
  tu1.heading[0] *= len * tu1.tangent_parameter[1];
  tu1.heading[1] *= len * tu1.tangent_parameter[1];
  tu1.heading[2] *= len * tu1.tangent_parameter[1];
  /* to a point */
  tu2.heading[0] *= len * tu2.tangent_parameter[0];
  tu2.heading[1] *= len * tu2.tangent_parameter[0];
  tu2.heading[2] *= len * tu2.tangent_parameter[0];
#endif

  tu_end = tu2;

  con_index1 = tu1.contour;
  con_index2 = tu_end.contour;

  if (contours > 1) {
    /* interpolate tu1 to tu2 using splines */

    switch (spline->base) {
    case SPLINE_BASE_HERMIT:
      /* setup Hermit coefficients */
      for (i = 0; i < 3; i++) {
        pts[i][0] = tu1.position[i];
        pts[i][1] = tu1.heading[i];
        tmp = tu1.position[i] - tu2.position[i];
        pts[i][2] = -3 * tmp - 2 * tu1.heading[i] - tu2.heading[i];
        pts[i][3] = 2 * tmp + tu1.heading[i] + tu2.heading[i];
      }
      break;

    case SPLINE_BASE_BSPLINE:
      /* setup B-spline coefficients */
      for (i = 0; i < 3; i++) {
        pts[i][0] = (cntpts[0].position[i] + 4 * cntpts[1].position[i] +
                     cntpts[2].position[i]) /
                    6.0;
        pts[i][1] =
            (-3 * cntpts[0].position[i] + 3 * cntpts[2].position[i]) / 6.0;
        pts[i][2] = (3 * cntpts[0].position[i] - 6 * cntpts[1].position[i] +
                     3 * cntpts[2].position[i]) /
                    6.0;
        pts[i][3] = (-cntpts[0].position[i] + 3 * cntpts[1].position[i] -
                     3 * cntpts[2].position[i] + turtle->position[i]) /
                    6.0;
      }
      break;
    }
  }

  if (spline->base == SPLINE_BASE_BSPLINE)
    *turtle = tu_end;

  /* multiplied by one half because we want to work with radius */
  tmp = 0.5 * (tu2.line_width - tu1.line_width);

#if CPFG_VERSION >= 3400
  tu2.radius_tangent[1][0] = tu2.radius_tangent[0][0];
  tu2.radius_tangent[1][1] = tu2.radius_tangent[0][1];

  tu2.radius_tangent[0][0] = tu1.radius_tangent[1][0];
  tu2.radius_tangent[0][1] = tu1.radius_tangent[1][1];
#endif

  if (tu2.scale_radius_tangents) {
    tu2.radius_tangent[0][1] *= len;
    tu2.radius_tangent[1][1] *= len;
  }

  /* if tangent is zero, use the default linear interpolation */
  if (tu2.radius_tangent[0][0] == 0 && tu2.radius_tangent[0][1] == 0) {
    tu2.radius_tangent[0][0] = 1;
    tu2.radius_tangent[0][1] = tmp;
  }

  if (tu2.radius_tangent[1][0] == 0 && tu2.radius_tangent[1][1] == 0) {
    tu2.radius_tangent[1][0] = 1;
    tu2.radius_tangent[1][1] = tmp;
  }

  if (contours > 1) {
    /* pair (t for main Hermit curve, radius) */
    rad[0][0] = 0; /* t starts from 0 and ends in 1 */
    rad[0][1] = 0.5 * tu1.line_width;

    rad[1][0] = tu2.radius_tangent[0][0];
    rad[1][1] = tu2.radius_tangent[0][1];

    rad[2][0] = 3 - 2 * tu2.radius_tangent[0][0] - tu2.radius_tangent[1][0];
    rad[2][1] =
        3 * tmp - 2 * tu2.radius_tangent[0][1] - tu2.radius_tangent[1][1];

    rad[3][0] = -2 + tu2.radius_tangent[0][0] + tu2.radius_tangent[1][0];
    rad[3][1] = -2 * tmp + tu2.radius_tangent[0][1] + tu2.radius_tangent[1][1];

    /* for make_contour,  */
    tu2.radius_tangent[0][1] /= len;
    tu2.radius_tangent[1][1] /= len;
  }

  switch (dr->line_style) {
  case LS_PIXEL:
    break;

  case LS_POLYGON:
    break;

  case LS_CYLINDER:
    if (dr->texture)
      dr->tdd->StartTexture(tu1.texture);

    make_contour(contour1, &tu1, con_index1, con_index2, len,
                 tu1.line_width * 0.5, tu2.radius_tangent[0], 0, dr);
    contour1ptr = contour1;
    contour2ptr = contour2;
    break;
  }

  if (contours > 1) {
    /* for make_contour() - line_width should be always the initial one */
    tu2.line_width = tu1.line_width;

    /* step based on the number of triangulated contours */
    rstep = 1.0 / contours;

    /* the last contour is done using the spline2 */
    for (i = 1, r = 0.0; i < contours; i++) {
      r += rstep;

      t = ((rad[3][0] * r + rad[2][0]) * r + rad[1][0]) * r + rad[0][0];
      radius = ((rad[3][1] * r + rad[2][1]) * r + rad[1][1]) * r + rad[0][1];

      radius_tangent[0] = (3 * rad[3][0] * r + 2 * rad[2][0]) * r + rad[1][0];
      radius_tangent[1] = (3 * rad[3][1] * r + 2 * rad[2][1]) * r + rad[1][1];

      radius_tangent[1] /= len;

      for (j = 0; j < 3; j++)
        tu2.position[j] =
            ((pts[j][3] * t + pts[j][2]) * t + pts[j][1]) * t + pts[j][0];

      /* derivative of the polynomial */
      for (j = 0; j < 3; j++)
        tu2.heading[j] = (3 * pts[j][3] * t + 2 * pts[j][2]) * t + pts[j][1];

      DNormalize(tu2.heading);

      /* update texel t coordinate */
      len2 = sqrt(DDistance(tu1.position, tu2.position));

      tu2.tex_t = tu1.tex_t + update_segment_texture(
                                  texture, (float)radius * 2.0, (float)len2);

      switch (dr->line_style) {
      case LS_PIXEL:
        dr->tdd->StartNode(&tu1, dr, &viewparam, 0, ' ');
        dr->tdd->EndNode(&tu2, dr, &viewparam, ' ');
        break;

      case LS_POLYGON:
        adjust_up_and_leftS(&tu2, &spline->tu, spline->twist_angle * t);

        len2 = sqrt(DDistance(tu1.position, tu2.position));

        for (j = 0; j < 3; j++)
          tu1.heading[j] = (tu2.position[j] - tu1.position[j]) / len2;

        dr->tdd->StartNode(&tu1, dr, &viewparam, len2, ' ');
        dr->tdd->EndNode(&tu2, dr, &viewparam, ' ');

        break;

      case LS_CYLINDER:
        adjust_up_and_leftS(&tu2, &spline->tu, spline->twist_angle * t);

        /* this should be good enough !! */
        make_contour(contour2ptr, &tu2, con_index1, con_index2, len, radius,
                     radius_tangent, t, dr);

        strip_to_mesh(contour1ptr, &tu1, contour2ptr, &tu2, dr);

        /* swap contour pointers */
        tmpptr = contour1ptr;
        contour1ptr = contour2ptr;
        contour2ptr = tmpptr;

        break;
      }

      tu1 = tu2;
    }
  }

  len2 = sqrt(DDistance(tu1.position, tu_end.position));

  turtle->tex_t = tu_end.tex_t =
      tu1.tex_t +
      update_segment_texture(texture, tu_end.line_width, (float)len2);

  switch (dr->line_style) {
  case LS_PIXEL:
    dr->tdd->StartNode(&tu1, dr, &viewparam, 0, ' ');
    dr->tdd->EndNode(&tu_end, dr, &viewparam, ' ');
    break;

  case LS_POLYGON:
    adjust_up_and_leftS(&tu2, &tu_end, spline->twist_angle);

    len2 = sqrt(DDistance(tu1.position, tu_end.position));

    for (j = 0; j < 3; j++)
      tu1.heading[j] = (tu_end.position[j] - tu1.position[j]) / len2;

    dr->tdd->StartNode(&tu1, dr, &viewparam, len2, ' ');
    dr->tdd->EndNode(&tu_end, dr, &viewparam, ' ');
    break;

  case LS_CYLINDER:
    adjust_up_and_leftS(&tu2, &tu_end, spline->twist_angle);

    make_contour(contour2ptr, &tu_end, con_index1, con_index2, len,
                 tu_end.line_width * 0.5, tu2.radius_tangent[1], 1.0, dr);

    strip_to_mesh(contour1ptr, &tu1, contour2ptr, &tu_end, dr);

    if (dr->texture)
      dr->tdd->EndTexture(tu1.texture);

    break;
  }

  glFrontFace(GL_CW);
}
