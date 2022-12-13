/*****************************************************************************
**
**  Functions for the environmental control
**  Author: Radomir Mech        March 94
**
**  Field function implemented:
**  - parameters read in ReadViewData (interpret.c) and stored in structure
**    environmentparam.
*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "environment.h"
#include "utility.h"
#include "log.h"

/**************************************************************************
 *  Local types and defines
 */

/* --------------- field types ----------------- */
#define FIELDBLOB 1

#define MAXBLOBIES MAXFIELDS

#define BLOBEXT "sdat" /* recognized extensions */

/* BLOB ********************************/
#define MAXBLOBPOINTS 100
#define MAXBLOBLINES 50

#define BLOBa -0.444444
#define BLOBb 1.888889
#define BLOBc -2.444444
#define DEFAULT_R 2.0 /* default radius of influence */

static struct BLOBPOINT {
  double P[3];       /* position */
  double R2;         /* R*R                 */
  double aR, bR, cR; /* a/R^6, b/R^4, c/R^2 */
  char is_ellipse;
  double axes_par[3][3]; /* parameters for P=(x,y,z) 0..x, 1..y, 2..z
                           to get det| P Ay Az| .. 0
                                  det|Ax  P Az| .. 1
                                  det|Ax Az  P| .. 2
                   all divided by det|Ax Ay Az| */
} blobpoints[MAXBLOBPOINTS];

static struct BLOBLINE {
  double P1[3], P2[3];  /* endpoints */
  double R1, R2;        /* their radius of influence */
  double R12, R22;      /* R1*R1, R2*R2 */
  double aR1, bR1, cR1; /* a/R1^6, b/R1^4, c/R1^2 */
  double aR2, bR2, cR2; /* a/R2^6, b/R2^4, c/R2^2 */
  double delta_R;       /* R2 -R1 */
  double v21[3];        /* (P2-P1)           */
  double v[3];          /* (P2-P1)/|P2-P1|^2 */
} bloblines[MAXBLOBLINES];

static struct BLOBIES {
  short first_point, last_point;
  short first_line, last_line;
} blobies[MAXBLOBIES];

static int number_of_blobies;

/* ------------------ global environmental structure  --------------------- */
struct ENVIRONMENTPARAM environmentparam;

/* ======================================================================== */

void FreeEnvironmentSpace(void) {}

/***************************************************************************
**
**     FIELD INITIALIZATION AND PARSING OF FIELD DATA FILES
**
****************************************************************************/

void InitializeEnvironmentParam(void) {
  environmentparam.number_of_movements = 0;
  environmentparam.number_of_fields = 0;
  number_of_blobies = 0;
}

/* ======================================================================== */
/*
**                     Parsing of sdat files
*/

/* specification of the mode - what type of object is currently parsed */
#define BNOMODE 0
#define BPOLYGON 1
#define BLINE 2
#define BELLIPSE 3

static double P1[3], P2[3]; /* origin or endpoints */
static double A[3][3];      /* vectors of axes Ax Ay Az */
static char was_a;
static double R1, R2; /* radiuses of influence */

/* --------------------------------------------------------------------- */
/* After q or a new object is encountered, values are stored in structures
** bloblines or blobpoints and some precomputed values are computed.
*/

static void finish_object(short index, char mode) {
  short i, k, j;
  double R, idet;

  switch (mode) {
  case BLINE:
    i = ++blobies[index].last_line;

    bloblines[i].P1[0] = P1[0];
    bloblines[i].P1[1] = P1[1];
    bloblines[i].P1[2] = P1[2];
    bloblines[i].P2[0] = P2[0];
    bloblines[i].P2[1] = P2[1];
    bloblines[i].P2[2] = P2[2];

    bloblines[i].R1 = R1;
    bloblines[i].R12 = R = R1 * R1;
    bloblines[i].cR1 = BLOBc * (R = 1.0 / R);
    bloblines[i].bR1 = BLOBb * R * R;
    bloblines[i].aR1 = BLOBa * R * R * R;

    bloblines[i].R2 = R2;
    bloblines[i].R22 = R = R2 * R2;
    bloblines[i].cR2 = BLOBc * (R = 1.0 / R);
    bloblines[i].bR2 = BLOBb * R * R;
    bloblines[i].aR2 = BLOBa * R * R * R;

    bloblines[i].delta_R = R2 - R1;

    bloblines[i].v21[0] = P2[0] - P1[0];
    bloblines[i].v21[1] = P2[1] - P1[1];
    bloblines[i].v21[2] = P2[2] - P1[2];

    R = 1.0 / (bloblines[i].v21[0] * bloblines[i].v21[0] +
               bloblines[i].v21[1] * bloblines[i].v21[1] +
               bloblines[i].v21[2] * bloblines[i].v21[2]);

    bloblines[i].v[0] = bloblines[i].v21[0] * R;
    bloblines[i].v[1] = bloblines[i].v21[1] * R;
    bloblines[i].v[2] = bloblines[i].v21[2] * R;
    break;
  case BELLIPSE:
    i = ++blobies[index].last_point;

    blobpoints[i].P[0] = P1[0];
    blobpoints[i].P[1] = P1[1];
    blobpoints[i].P[2] = P1[2];

    blobpoints[i].is_ellipse = was_a;
    if (blobpoints[i].is_ellipse) {
      /* here SHOULD be = */
      if ((idet = A[0][0] * A[1][1] * A[2][2] + A[0][1] * A[1][2] * A[2][0] +
                  A[0][2] * A[1][0] * A[2][1] - A[0][0] * A[1][2] * A[2][1] -
                  A[0][1] * A[1][0] * A[2][2] - A[0][2] * A[1][1] * A[2][0]) ==
          0) {
        Message("FIELD: given axes are in one plane!\n");
        blobpoints[i].is_ellipse = 0;
        break;
      }

      R1 = 1.0; /* when axes given, their lengths specify R
                        dunno why, but Blob has it so
      later - it might have been bug */

      idet = 1.0 / idet;

      for (k = 0; k < 3; k++)
        for (j = 0; j < 3; j++)
          blobpoints[i].axes_par[k][j] =
              idet *
              (A[(j + 1) % 3][(k + 1) % 3] * A[(j + 2) % 3][(k + 2) % 3] -
               A[(j + 1) % 3][(k + 2) % 3] * A[(j + 2) % 3][(k + 1) % 3]);
    }

    blobpoints[i].R2 = R = R1 * R1;
    blobpoints[i].cR = BLOBc * (R = 1.0 / R);
    blobpoints[i].bR = BLOBb * R * R;
    blobpoints[i].aR = BLOBa * R * R * R;

    break;
  }
  was_a = 0;
}

/* --------------------------------------------------------------------- */
/* Main routine for parsing a sdat file. Modified from Blob's s_model.c .
** A lot of things ignored - in such case a message is printed.
** Currently only lines (L,l) and ellipses (e) are parsed. Inside an object
** only o (specifies the point), r (radius of influence), and q (quit) are
** considered.
** Added: axes (a) are parsed (only for an ellipse=point)
*/

static int parse_sdat_file(short index, FILE *in) {
  char command;
  char buf[256];
  char mode;
  double R = DEFAULT_R;
  char *ptr = NULL;
  short line_P = 0, line_R = 0;
  short n = 0;

  mode = BNOMODE;
  was_a = 0;

  blobies[index].last_point = blobies[index].first_point - 1;
  blobies[index].last_line = blobies[index].first_line - 1;

  while (fgets(buf, sizeof buf, in)) {
    if (buf[0] == 0)
      break;
    ptr = buf + strspn(buf, " ");
    command = *(ptr++);

    switch (command) {
    case '\n':
      break;
    case '#': /* Comment */
      break;

    case 'L': /* Line specified by two points following L */
      finish_object(index, mode);
      mode = BLINE;

      R1 = R2 = R;
      if (sscanf(ptr, "%lf %lf %lf %lf %lf %lf %lf %lf", &P1[0], &P1[1], &P1[2],
                 &P2[0], &P2[1], &P2[2], &R1, &R2) < 6) {
        Message("Cannot read two line points! Line ignored!\n");
        mode = BNOMODE;
        break;
      }
      line_P = line_R = 2;
      finish_object(index, mode);
      break;

    case 'l': /* line */
      finish_object(index, mode);
      mode = BLINE;
      R1 = R2 = R;
      P1[0] = P1[1] = P1[2] = 0;
      P2[0] = 1.0;
      P2[1] = P2[2] = 0;
      break;

    case 'e': /* ellipse */
      finish_object(index, mode);
      mode = BELLIPSE;
      R1 = R; /* I think should be R, but... */
      P1[0] = P1[1] = P1[2] = 0;
      break;

    case 'o': /* origin */
      switch (mode) {
      case BELLIPSE:
        n = sscanf(ptr, "%lf %lf %lf", &P1[0], &P1[1], &P1[2]);
        break;
      case BLINE: {
        if (line_P == 0)
          n = sscanf(ptr, "%lf %lf %lf", &P1[0], &P1[1], &P1[2]);
        else if (line_P == 1)
          n = sscanf(ptr, "%lf %lf %lf", &P2[0], &P2[1], &P2[2]);
        line_P += n == 3;
      } break;
      }
      if (n != 3)
        Message("FIELD: Error reading origin!\n");
      break;

    case 'r': /* radius of influence */
      switch (mode) {
      case BNOMODE:
        sscanf(ptr, "%lf", &R);
        break;
      case BELLIPSE:
        sscanf(ptr, "%lf", &R1);
        break;
      case BLINE:
        if (line_R == 0)
          line_R = sscanf(ptr, "%lf %lf", &R1, &R2);
        else if (line_R == 1)
          line_R = 1 + sscanf(ptr, "%lf", &R2);
        break;
      }
      break;

    case 'q': /* quit */
      finish_object(index, mode);
      mode = BNOMODE;
      break;

    case 'p': /* polygon */
      finish_object(index, mode);
      mode = BPOLYGON;
      Message("FIELD: Polygons ignored!\n");
      break;
    case 'h':
      Message("FIELD: Vertex of a polygon ignored.\n");
      break;
    case 'a': /* axes Ax Ay Az (as columns) */
      if (sscanf(ptr, "%lf %lf %lf %lf %lf %lf %lf %lf %lf", &A[0][0], &A[1][0],
                 &A[2][0], &A[0][1], &A[1][1], &A[2][1], &A[0][2], &A[1][2],
                 &A[2][2]) != 9)
        Message("FIELD: Error reading axes .\n");
      else {
        was_a = 1;
      }
      break;
    case 'f':
      Message("FIELD: Force ignored.\n");
      break;
    case 'u':
      Message("FIELD: Function u ignored.\n");
      break;
    case 'b':
      Message("FIELD: Bend ignored.\n");
      break;
    case 'g':
      Message("FIELD: Gang ignored.\n");
      break;
    case 'F':
      Message("FIELD: Dynamic force ignored.\n");
      break;
    case 'c':
      Message("FIELD: Color ignored.\n");
      break;
    case 's':
      Message("FIELD: Surface properties ignored.\n");
      break;
    case 'v':
      Message("FIELD: Velocity ignored.\n");
      break;
    case 'n':
      Message("FIELD: Acceleration ignored.\n");
      break;

    default:
      Message("FIELD: Unknown command %c! Ignored!\n", command);
    } /* end switch(command) */
  }
  finish_object(index, mode);
  fclose(in);

  number_of_blobies++;
  return 1;
}

/* ======================================================================== */
#define EF environmentparam.fields[index]

/* ------------------------------------------------------------------------ */
/* Called from ReadViewData (interpret.c). Sets up fileds structures.
** Type of a filed is specified by the extension of filed input file.
** Currently, only rgb images and bloby sdat files are implemented.
**
** From L-system rules an index of a filed must be given to field functions.
** The index is the number of the filed in the view file - so the first one
** has index 1 and so on.
*/

int field_initialize(short index, char *tmpfile) {
  FILE *in;
  char *ptr;

  if ((ptr = strchr(tmpfile, '.')) == NULL) {
    Message("No extension in field filename %s !\n", tmpfile);
    return 0;
  }

  if (strcmp(++ptr, BLOBEXT) == 0) {
    if (number_of_blobies >= MAXBLOBIES) {
      Message("No room for another bloby field!\n");
      return 0;
    }

    EF.type = FIELDBLOB;

    if ((EF.index = number_of_blobies) == 0) {
      blobies[0].first_point = 0;
      blobies[0].first_line = 0;
    } else {
      blobies[number_of_blobies].first_line =
          blobies[number_of_blobies - 1].last_line + 1;
      blobies[number_of_blobies].first_point =
          blobies[number_of_blobies - 1].last_point + 1;
    }

    if ((in = fopen(tmpfile, "r")) == NULL) {
      Message("Bloby sdat file %s not opened!\n", tmpfile);
      return 0;
    }

    return parse_sdat_file((short)number_of_blobies, in); /* returns 1 if OK */
  } /* number_of_blobies incremented in parse_sdat_file */

  Message("Extension %s not recognized!\n", ptr);
  return 0;
}

/* ======================================================================== */
/******************** Evaluation of bloby fields ****************************/

/* ------------------------------------------------------------------------ */
/* Returns particular part (j) of unit vector from (x,y,z) to the point
** multiplied by field value in (x,y,z) - gradient IF j<3
** IF j==3 then just returns filed value
*/

static double get_point_field(short index, double x, double y, double z,
                              short j) {
  double r, val;
  double w[3], v[3];
  short k;

  v[0] = x - blobpoints[index].P[0];
  v[1] = y - blobpoints[index].P[1];
  v[2] = z - blobpoints[index].P[2];

  if (blobpoints[index].is_ellipse) {
    /* adjust according to Ax Ay Az */
    for (k = 0; k < 3; k++)
      w[k] = DDotProduct(blobpoints[index].axes_par[k], v);
  }

  if ((r = w[0] * w[0] + w[1] * w[1] + w[2] * w[2]) >= blobpoints[index].R2)
    return 0;

  val = 1.0 + r * (blobpoints[index].cR +
                   r * (blobpoints[index].bR + r * blobpoints[index].aR));

  if (j == 3)
    return val;                                /* only field value */
  return val * v[j] / sqrt(DDotProduct(v, v)); /* part of gradient vector */
}

/* ------------------------------------------------------------------------ */

static double get_line_field(short index, double x, double y, double z,
                             short j) {
  double r, t, R, val;
  double w[3];

  w[0] = x - bloblines[index].P1[0];
  w[1] = y - bloblines[index].P1[1];
  w[2] = z - bloblines[index].P1[2];

  if ((t = w[0] * bloblines[index].v[0] + w[1] * bloblines[index].v[1] +
           w[2] * bloblines[index].v[2]) <= 0.0) {
    /* only P1 */
    if ((r = w[0] * w[0] + w[1] * w[1] + w[2] * w[2]) >= bloblines[index].R12)
      return 0;

    val = 1.0 + r * (bloblines[index].cR1 +
                     r * (bloblines[index].bR1 + r * bloblines[index].aR1));
    if (j == 3)
      return val;                /* only field value */
    return val * w[j] / sqrt(r); /* part of gradient vector */
  }

  if (t >= 1.0) {
    /* only P2 */
    w[0] -= bloblines[index].v21[0];
    w[1] -= bloblines[index].v21[1];
    w[2] -= bloblines[index].v21[2];

    if ((r = w[0] * w[0] + w[1] * w[1] + w[2] * w[2]) >= bloblines[index].R22)
      return 0;

    val = 1.0 + r * (bloblines[index].cR2 +
                     r * (bloblines[index].bR2 + r * bloblines[index].aR2));
    if (j == 3)
      return val;                /* only field value */
    return val * w[j] / sqrt(r); /* part of gradient vector */
  }

  R = bloblines[index].R1 + t * bloblines[index].delta_R;
  R *= R; /* interpolation of R between R1 and R2 */

  w[0] -= t * bloblines[index].v21[0];
  w[1] -= t * bloblines[index].v21[1];
  w[2] -= t * bloblines[index].v21[2]; /* distance to the line segment */

  if ((r = w[0] * w[0] + w[1] * w[1] + w[2] * w[2]) >= R)
    return 0;

  t = r / R;

  val = 1.0 + t * (BLOBc + t * (BLOBb + t * BLOBa));

  if (j == 3)
    return val;                /* only field value */
  return val * w[j] / sqrt(r); /* part of gradient vector */
}

/* ------------------------------------------------------------------------ */
/* Returns value of a field specified by its index in a point (x,y,z) */

double field(short index, double x, double y, double z) {
  double val = 0;
  short i, ind;

  index--; /* in L-system it starts from 1 */

  if ((index < 0) || (index >= environmentparam.number_of_fields)) {
    Message("Field index %d out of range!\n", index);
    return 0;
  }

  x = (x - EF.mid[0]) * EF.ratio[0];
  y = (y - EF.mid[1]) * EF.ratio[1];

  ind = environmentparam.fields[index].index;

  switch (environmentparam.fields[index].type) {
  case FIELDBLOB:
    z = (z - EF.mid[2]) * EF.ratio[2];

    for (i = blobies[ind].first_point; i <= blobies[ind].last_point; i++)
      val += get_point_field(i, x, y, z, 3);

    for (i = blobies[ind].first_line; i <= blobies[ind].last_line; i++)
      val += get_line_field(i, x, y, z, 3);

    break;
  }

  return val;
}
