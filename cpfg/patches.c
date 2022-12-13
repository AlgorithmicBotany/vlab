/******************************************************************************
 *
 *  Bicubic Surfaces - Jim Hanan
 *
 ******************************************************************************
 *
 *  Modifications:
 *		October 1990 - allowed patch precision on a per patch basis
 *		January 1991 - added routines for L-system defined Bezier
 patches *		May 1991 - converted routines for L-system defined
 Surface patches *		Sept 1991 - converted routines for rendering
 dispatcher *		Nov 1991 - added object routines for xpoly output
 *
 *              March-June 1994 BY: Radek
 *	                   ansi standard
 ******************************************************************************
 *  Problems:
 *      - Shading should be moved to the rendering routines ie irisGL.c
 *        An interface including bottom and top colour would be necessary
 ******************************************************************************
 *
 * These functions calculate points and normals of a parametric bicubic surface,
 * and draw the surface as required by pfg.
 *
 *
 * The program uses forward differences to calculate the surface points and
 * normals.  We work with
 *		x(s,t) = S Cx T(Transpose)
 *  where
 *		Cx = M Px M(Transpose)
 *		M = the Bezier Matrix
 *
 *  We calculate
 *		D00 = E(Epsilon) Cx E(Sigma)(Transpose) for surface points,
 *		with E(Epsilon) or E(Sigma)(Transpose) replaced by their
 *		derivatives for the calculation of surface tangents.
 *
 * Values for x(0,t) can be calculated by using the first row of D00
 * as f 0, delta f 0, delta2 f 0, delta3 f 0  and repeating the following
 * 1/Sigma times with n initially 0.
 *
 *		f n+1 = f n + delta f n,
 *		delta f n+1 = delta f n + delta2 f n,
 *		delta2 f n+1 = delta2 f n + delta3 f n.
 *
 * To iterate over s, we repeat the following 1/Epsilon times on D00
 *
 *		Row 1 = Row 1 + Row 2,
 *		Row 2 = Row 2 + Row 3,
 *		Row 3 = Row 3 + Row 4,
 *
 *  using the previous iteration for t after each.
 *  This process is followed for surface points and tangents in each
 *  of the s and t directions.
 *
 ******************************************************************************

 ******************************************************************************
 *
 * Defines and Declarations
 *
 *****************************************************************************/

// to avoid GLu Warning due to deprecated function on MacOs
#define GL_SILENCE_DEPRECATION

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "interpret.h"
#include "generate.h"
#include "control.h"
#include "utility.h"

#include "patch.h"
#include "tsurfaces.h"
#include "rayshade.h"
#include "textures.h"
#include "mesh.h"
#include "indices.h"

#include "test_malloc.h"

/* Change negative values to positive, all others stay the same */
#define absf(x) (((x) < 0.) ? -(x) : (x))

/* surface indexes */
enum eIndex { TOP = 0, BOTTOM = 1 };

/* neighbour indexes */
enum eNeighbour { nAL = 0, nA, nAR, nL, nR, nBL, nB, nBR };

/* Define M array to use ( m_bezier or m_B_spline ) */
#define M_ARRAY m_bezier
#define M_ARRAY_T m_bezier_t

/* Define maximum number of steps for S and T parameters */
enum ePrecistion { S_COUNT = 10, T_COUNT = 10 };

/* maximum storage for L-system defined precision is now
   equal to S_COUNTxT_COUNT because the 1-st array item is used
   for computing an L-system defined surface. */

double sqrt(double); /* square root function from math library */

/* Bezier matrix and Transpose */
static const double m_bezier[4][4] = {
    {-1., 3., -3., 1.}, {3., -6., 3., 0.}, {-3., 3., 0., 0.}, {1., 0., 0., 0.}};

static const double m_bezier_t[4][4] = {
    {-1., 3., -3., 1.}, {3., -6., 3., 0.}, {-3., 3., 0., 0.}, {1., 0., 0., 0.}};

/* Cardinal matrix and Transpose */
static const double m_cardinal[4][4] = {{-0.5, 1.5, -1.5, 0.5},
                                        {1.0, -2.5, 2.0, -0.5},
                                        {-0.5, 0.0, 0.5, 0.0},
                                        {0.0, 1.0, 0.0, 0.0}};

static const double m_cardinal_t[4][4] = {{-0.5, 1.0, -0.5, 0.},
                                          {1.5, -2.5, 0.0, 1.},
                                          {-1.5, 2.0, 0.5, 0.},
                                          {0.5, -0.5, 0.0, 0.}};

/* B-Spline matrix and Transpose */
/* not used at this time */
static const double m_B_spline[4][4] = {{-1. / 6., 3. / 6., -3. / 6., 1. / 6.},
                                        {3. / 6., -6. / 6., 3. / 6., 0. / 6.},
                                        {-3. / 6., 0. / 6., 3. / 6., 0. / 6.},
                                        {1. / 6., 4. / 6., 1. / 6., 0. / 6.}};

static const double m_B_spline_t[4][4] = {
    {-1. / 6., 3. / 6., -3. / 6., 1. / 6.},
    {3. / 6., -6. / 6., 0. / 6., 4. / 6.},
    {-3. / 6., 3. / 6., 3. / 6., 1. / 6.},
    {1. / 6., 0. / 6., 0. / 6., 0. / 6.}};

/* E arrays */
static double e_epsilon[4][4] = {
    {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}};
static double e_sigma_t[4][4] = {
    {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}};

/* e array for calculating tangents in t direction */
static double t_e_epsilon[4][4] = {
    {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}};

/* e array for calculating tangents in s direction */
static double t_e_sigma_t[4][4] = {
    {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}, {0., 0., 0., 0.}};

/* D00 array - for coordinates x, y, and z and
**			   components point, stangent and ttangent */
static double d00[3][3][4][4];

/* Patch Name Array */
static char patch_name[PATCHES][31];

// Now define in comlineparam.h

#ifndef WIN32
#define _MAX_FNAME 32
#endif

struct sSurfaceInfo {
  char filename[_MAX_FNAME];
  float size;
  char id;
  int s_precision;
  int t_precision;
  int texture;
};

typedef struct sSurfaceInfo SurfaceInfo;

SurfaceInfo surface_info[SURFACES];

/* Neighbour Structure */
/* contains flag and name indicating neighbours of a patch */
/* in each of 8 possible directions AL, A, AR, L, R, BL, B, and BR */
/* averaged flag indicates whether normals at vertices along */
/* neighbouring edges have been averaged or not */
static struct Neighbour {
  int flag;
  int averaged;
  char name[41];
  int index;
} neighbour[PATCHES][8];

/* Patch color Array */
static int patch_color[SURFACES][PATCHES][2];

/* Patch surface diffusion Array */
static float patch_diffuse[SURFACES][PATCHES][2];

/* Surface point array - for surface, */
/*                       patch, */
/*                       s, */
/*						 t, */
/*                       component; point, normal, texture coords,
                                    stangent, ttangent coordinate; x,y,z */
static float point[SURFACES][PATCHES][S_COUNT + 1][T_COUNT + 1][5][3];

/* patch precision */
static int s_precision[SURFACES];
static int t_precision[SURFACES];

/* surface texture
   if -1 use the turtle texture
   if 0  no texture, even if there is one in turtle
   if >0 overrides the texture specified in turtle */
static int s_textures[SURFACES];
/* type of texel coordinates stored in points
   0 none yet
   1 texture per patch
   2 texture per whole surface */
static char t_textures[SURFACES];

/* Number of patches */
static int patches[SURFACES];

/* Surface identifiers */
static char surface_identifier[SURFACES] = {'~'};

/* Surface end points */
static float end_point[SURFACES][3];

/* Control Point Arrays */
static double control_point_x[SURFACES][PATCHES][4][4];
static double control_point_y[SURFACES][PATCHES][4][4];
static double control_point_z[SURFACES][PATCHES][4][4];

/* to determine boundaries of a surface */
static double min_b[SURFACES][3], max[SURFACES][3];

/* for NURBS, the coordinates have to be stored together */
static float control_point[SURFACES][PATCHES][4][4][3];

/* NURBS texture cordinates */
static float N_texels[4][4][2] = {
    {{0.0f, 0.0f}, {1.0f / 3.0f, 0.0f}, {2.0f / 3.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.0f, 1.0f / 3.0f},
     {1.0f / 3.0f, 1.0f / 3.0f},
     {2.0f / 3.0f, 1.0f / 3.0f},
     {1.0f, 1.0f / 3.0f}},
    {{0.0f, 2.0f / 3.0f},
     {1.0f / 3.0f, 2.0f / 3.0f},
     {2.0f / 3.0f, 2.0f / 3.0f},
     {1.0f, 2.0f / 3.0f}},
    {{0.0f, 1.0f}, {1.0f / 3.0f, 1.0f}, {2.0f / 3.0f, 1.0f}, {1.0f, 1.0f}}};

/* Control point extremes, offsets and scale */
static float extreme[3][2]; /* currently not used */
static float cp_offset[3];
static float cp_scale;
/* don't change to double before changing %f to %lf in scanfs.
   The same for the following three vectors. */

/* Surface orientation */
float heading[3];  /* x, y, and z co-ordinates of heading vector */
float left[3];     /* x, y, and z co-ordinates of left vector */
float upvector[3]; /* x, y, and z co-ordinates of up vector */

/* End of File flag */
static int eof;

/* patch index */
static int current;

/* current surface index */
int surface_number = 1;
/* the zero-th is used for L-system defined surface */
static const int LDSIndex = 0;

/* File pointer declarations */
FILE *control_fp;

/* global declarations for the blackbox P functions */
struct surfacePatch {
  double x[4][4];
  double y[4][4];
  double z[4][4];
  Colorindex col_index[4][4];
  int assigned[4][4]; /* flags for assignment error check */
  float patchID;
  int basis;
  struct surfacePatch *nextPatch;
};
typedef struct surfacePatch surfacePatch;

static surfacePatch *patchList = NULL;
static surfacePatch *currentPatch = NULL;

/* local prototypes */
static int initialize_patch(const char *fname, float surface_size,
                            char surface_id);
static void initialize_matrices(int sprecision, int tprecision);
static void determine_transform(float surface_size);
static void calculate_d00(double geomx[4][4], double geomy[4][4],
                          double geomz[4][4], const double m_array[4][4],
                          const double m_array_t[4][4]);
static void calculate_surface(int surface_number);
static void set_texture_coordinates(int surface_number, int tex_index);
static void get_patch(void);
static int check_connections(void);
static void calculate_neighbouring_vertex_normals(void);
static void assign_normal(int patch1, int s1, int t1, int patch2, int s2,
                          int t2);
static void add_normals(int patch1, int s1, int t1, int patch2, int s2, int t2);
static void calculate_normal(int p, int s, int t);
static void transform_end_point(int surface_index, TURTLE *tu, float line_scale,
                                double scale);
static surfacePatch *FindSurfacePatch(double patchID);
static void determine_vertex_shading(int surface, int p, int s, int t,
                                     Colorindex *color_index,
                                     Colorindex *color_index_back, TURTLE *tu,
                                     const DRAWPARAM *dr, VIEWPARAM *vw);

/********************************************************************/
/* returns -1 if not found */
int FindSurfaceIndex(char surface_id) {
  int surface_index;

  for (surface_index = 1; surface_index < surface_number; surface_index++)
    if (surface_identifier[surface_index] == surface_id)
      return surface_index;

  return -1;
}

/* ------------------------------------------------------------*/

void InitializeSurfaces(void) { surface_number = 1; }

/* Initialization for the blackbox P functions */
void InitSurfacePatches(void) {}

int rereadsurfacefile(int i) {
  VERBOSE("Surf filename: %s\n", surface_info[i].filename);
  return read_surface(surface_info[i].filename, surface_info[i].size,
                      surface_info[i].id, surface_info[i].s_precision,
                      surface_info[i].t_precision, surface_info[i].texture);
}

// [Pascal] keep opening the file until the size is stable
static FILE *testOpenFile(const char *fname) {
#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((fp == NULL) && (counter < 1000)) {
    fp = fopen(fname, "rb");
    counter++;
  }
  if (counter == 1000)
    fprintf(stderr, "WARNING: Can't open the view file %s - using defaults.\n",
            fname);
  else {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp);       // get current file pointer

    while ((size == 0) || (current_size != size)) {
      current_size = size;
      fclose(fp);

      fp = fopen(fname, "r");
      while (fp == NULL) {
        fp = fopen(fname, "r");
        counter++;
      }
      fseek(fp, 0, SEEK_END); // seek to end of file
      size = ftell(fp);       // get current file pointer
    }
  }
  fseek(fp, 0L, SEEK_SET);
#else
  FILE *fp = fopen(fname, "rb");
#endif

  return fp;
}


int read_surface(const char *datafile, float surface_size, char surface_id,
                 int s_input_precision, int t_input_precision, int s_texture) {

  int result_connection = 0;
  /* Check that array size is not exceeded */
  if (surface_number >= SURFACES) {
    Message("Maximum number of patches (%d) exceeded.\n", SURFACES);
    Message("Change SURFACES in patch.c and recompile for more.\n");
    return 0;
    MyExit(4);
  }

  /* Fill an entry in the sufrace_info array */

  strncpy(surface_info[surface_number].filename, datafile, 32);

  surface_info[surface_number].size = surface_size;
  surface_info[surface_number].id = surface_id;
  surface_info[surface_number].s_precision = s_input_precision;
  surface_info[surface_number].t_precision = t_input_precision;
  surface_info[surface_number].texture = s_texture;

  /* Assign initial precision values */
  s_precision[surface_number] = s_input_precision;
  t_precision[surface_number] = t_input_precision;

  /* Assign texture */
  s_textures[surface_number] = s_texture;
  t_textures[surface_number] = NO_TEXELS_YET;

  // while (result_connection == 0) {
    /* Pass control point file name to initialize routine */
    if (!initialize_patch(datafile, surface_size, surface_id))
      //continue;
      return 0;

    /* Loop until no more control points are found in the file */
    while (!eof) {

      /* Calculate d00's for this patch */
      calculate_d00(control_point_x[surface_number][current],
                    control_point_y[surface_number][current],
                    control_point_z[surface_number][current], m_bezier,
                    m_bezier_t);

      /* Calculate surface points for this patch */
      calculate_surface(surface_number);

      /* Get control points for next patch */
      get_patch();
    }

    /* assign number of patchs in surface for later use */
    patches[surface_number] = current;

    /* check described connections between patch to ensure consistency */
    result_connection = check_connections();
    if (!result_connection)
      return 0;
    //}

  /* Average vertex normals for neighbouring patches */
  calculate_neighbouring_vertex_normals();

  /* Increment surface index for next surface */
  surface_number++;

  /* Close file */
  fclose(control_fp);
  clp.initSurface = 1;
  return 1;
}


static int initialize_patch(const char *fname, float surface_size,
                            char surface_id) {
  VERBOSE("Initializing for %s ... \n", fname);

  // [PASCAL]
  // open the file twice to check if the size is consistent
  if ((control_fp = testOpenFile(fname)) == NULL)
    return 0;

  if (surface_id == '~') {
    if (surface_number > 1) {
      Message("Segment surface with id ~ must be the first surface "
              "specified in the view file.\nOtherwise it is ignored!\n");
      return 0;
    }
  } else {
    if (FindSurfaceIndex(surface_id) != -1) {
      Message("Surface id %c already used! The surface ignored!\n", surface_id);
      return 0;
    }

    if (FindTSurfaceIndex(surface_id) != -1) {
      Message("Surface id %c already used (for a tsurface)! The surface "
              "ignored!\n",
              surface_id);
      return 0;
    }
  }

  /* Save Id for later use */
  surface_identifier[surface_number] = surface_id;

  /* Set end of file flag */
  eof = FALSE;

  /* Determine scaleing and offset factors */
  determine_transform(surface_size);

  /* Set up patch index for surface array */
  current = -1;

  /* Initialize e_epsilon and e_sigma_t */
  initialize_matrices(s_precision[surface_number], t_precision[surface_number]);

  /* Get points for first patch */
  get_patch();

  return 1;
}

/* Initialize e_epsilon and e_sigma_t matrices */
static void initialize_matrices(int sprecision, int tprecision) {
  double epsilon, sigma;

  /* Define Step Size and number of steps for S and T parameters */
  epsilon = 1.0 / (double)sprecision;
  sigma = 1.0 / (double)tprecision;

  e_epsilon[0][3] = 1.0;
  e_epsilon[1][0] = epsilon * epsilon * epsilon;
  e_epsilon[1][1] = epsilon * epsilon;
  e_epsilon[1][2] = epsilon;
  e_epsilon[2][0] = 6.0 * epsilon * epsilon * epsilon;
  e_epsilon[2][1] = 2.0 * epsilon * epsilon;
  e_epsilon[3][0] = 6.0 * epsilon * epsilon * epsilon;
  e_sigma_t[3][0] = 1.0;
  e_sigma_t[0][1] = sigma * sigma * sigma;
  e_sigma_t[1][1] = sigma * sigma;
  e_sigma_t[2][1] = sigma;
  e_sigma_t[0][2] = 6.0 * sigma * sigma * sigma;
  e_sigma_t[1][2] = 2.0 * sigma * sigma;
  e_sigma_t[0][3] = 6.0 * sigma * sigma * sigma;

  /* Initialize t_e_epsilon and t_e_sigma_t */
  t_e_epsilon[0][2] = 1.0;
  t_e_epsilon[1][0] = 3.0 * epsilon * epsilon;
  t_e_epsilon[1][1] = 2.0 * epsilon;
  t_e_epsilon[2][0] = 6.0 * epsilon * epsilon;
  t_e_sigma_t[2][0] = 1.0;
  t_e_sigma_t[0][1] = 3.0 * sigma * sigma;
  t_e_sigma_t[1][1] = 2.0 * sigma;
  t_e_sigma_t[0][2] = 6.0 * sigma * sigma;

  /* the rest is 0 (initialized so) */
}

/* Determine Scaling and translation factors */
static void determine_transform(float surface_size) {
  int flag, i;
  float max_dimension;
  float x_value, y_value, z_value;
  int s_temp, t_temp;

  flag = 0;
  max_dimension = 0;

  for (i = 0; i < 3 && flag != EOF; i++) {
    flag = fscanf(control_fp, "%f %f", &extreme[i][0], &extreme[i][1]);
    VERBOSE("Coord: %d  Min: %f  Max: %f  ", i, extreme[i][0], extreme[i][1]);
    if (flag == EOF)
      return;
  }

  flag = fscanf(control_fp, " PRECISION  S: %d T: %d\n", &s_temp, &t_temp);
  if (flag == EOF)
    return;
  if (flag == 0) {
    /* no precision statement found; average assumed */
    s_temp = S_COUNT / 2;
    t_temp = T_COUNT / 2;
  }

  /* if not defined in view file use patch precisions */
  if (s_precision[surface_number] == 0) {
    s_precision[surface_number] = s_temp;
  }
  if (t_precision[surface_number] == 0) {
    t_precision[surface_number] = s_temp;
  }

  /* make sure precisions are in range */
  if (s_precision[surface_number] < 1) {
    Message("S precision below minimum: %d used.\n", 1);
    s_precision[surface_number] = 1;
  }
  if (s_precision[surface_number] > S_COUNT) {
    Message("S precision exceeds maximum: %d used.\n", S_COUNT);
    s_precision[surface_number] = S_COUNT;
  }
  if (t_precision[surface_number] < 1) {
    Message("T precision below minimum: %d used.\n", 1);
    t_precision[surface_number] = 1;
  }
  if (t_precision[surface_number] > T_COUNT) {
    Message("T precision exceeds maximum: %d used.\n", T_COUNT);
    t_precision[surface_number] = T_COUNT;
  }

  VERBOSE(" PRECISION  S: %d T: %d\n", s_precision[surface_number],
          t_precision[surface_number]);

  flag = fscanf(control_fp, " CONTACT POINT  X: %f Y: %f Z: %f\n",
                &cp_offset[eX], &cp_offset[eY], &cp_offset[eZ]);
  if (flag == EOF)
    return;
  VERBOSE(" CONTACT POINT  X: %f Y: %f Z: %f\n", cp_offset[eX], cp_offset[eY],
          cp_offset[eZ]);

  flag = fscanf(control_fp, " END POINT  X: %f Y: %f Z: %f\n", &x_value,
                &y_value, &z_value);
  if (flag == EOF)
    return;
  VERBOSE(" END POINT  X: %f Y: %f Z: %f\n", x_value, y_value, z_value);

  flag = fscanf(control_fp, "HEADING  X: %f Y: %f Z: %f\n", &heading[eX],
                &heading[eY], &heading[eZ]);
  if (flag == EOF)
    return;
  VERBOSE("HEADING  X: %f Y: %f Z: %f\n", heading[eX], heading[eY],
          heading[eZ]);
  Normalize(heading);

  flag = fscanf(control_fp, "UP  X: %f Y: %f Z: %f\n", &upvector[eX],
                &upvector[eY], &upvector[eZ]);
  if (flag == EOF)
    return;
  VERBOSE("UP  X: %f Y: %f Z: %f\n", upvector[eX], upvector[eY], upvector[eZ]);
  Normalize(upvector);

  /* crossproduct of heading and up vector gives left vector */
  CrossProduct(heading, upvector, left);

  /* crossproduct of heading and left vector gives up vector */
  CrossProduct(left, heading, upvector);

  flag = fscanf(control_fp, "SIZE: %f\n", &max_dimension);
  VERBOSE("SIZE: %f\n", max_dimension);
  if (flag == EOF)
    return;

  /* Scale the surface so that the maximum dimension has the desired size */
  cp_scale = surface_size / max_dimension;

  VERBOSE(" Scale: %f  Surface Size: %f\n", cp_scale, surface_size);
  /* Transform endpoint for later use */

  /* Translate and scale */
  x_value -= cp_offset[eX];
  x_value *= cp_scale;
  y_value -= cp_offset[eY];
  y_value *= cp_scale;
  z_value -= cp_offset[eZ];
  z_value *= cp_scale;

  /* Rotate */
  end_point[surface_number][eX] =
      x_value * left[eX] + y_value * heading[eX] + z_value * upvector[eX];

  end_point[surface_number][eY] =
      x_value * left[eY] + y_value * heading[eY] + z_value * upvector[eY];

  end_point[surface_number][eZ] =
      x_value * left[eZ] + y_value * heading[eZ] + z_value * upvector[eZ];
}

/* Calculate d00's for this patch */
static void calculate_d00(double geomx[4][4], double geomy[4][4],
                          double geomz[4][4], const double m_array[4][4],
                          const double m_array_t[4][4]) {
  int coord;
  double temp[4][4]; /* temporary array */
  double c[3][4][4]; /* C array */

  VERBOSE("Calculating d00 ...\n");

  /* Calculate D00 for point and tangent components  */
  /* for each of the X, Y, and Z coordinates. */
  for (coord = 0; coord <= 2; coord++) {
    /* Calculate C = M P M(Transpose)
     **  Where M is the Bezier Matrix, Cardinal, or B-Spline Matrix. */
    switch (coord) {
    case eX:
      MatMult4x4D(m_array, geomx, temp);
      break;
    case eY:
      MatMult4x4D(m_array, geomy, temp);
      break;
    case eZ:
      MatMult4x4D(m_array, geomz, temp);
      break;
    }
    MatMult4x4D(temp, m_array_t, c[coord]);

    /* Calculate D00 = E(Epsilon) C E(Sigma)(Transpose) */
    /* for points */
    MatMult4x4D(e_epsilon, c[coord], temp);
    MatMult4x4D(temp, e_sigma_t, d00[coord][ePOINT]);

    /* Calculate D00 = T_E(Epsilon) C E(Sigma)(Transpose) */
    /* for t tangents */
    MatMult4x4D(t_e_epsilon, c[coord], temp);
    MatMult4x4D(temp, e_sigma_t, d00[coord][2]);

    /* Calculate D00 = E(Epsilon) C T_E(Sigma)(Transpose) */
    /* for s tangents */
    MatMult4x4D(e_epsilon, c[coord], temp);
    MatMult4x4D(temp, t_e_sigma_t, d00[coord][1]);
  }
}

/* Set texture coordinates for all points */
static void set_texture_coordinates(int surface_number, int tex_type) {
  int s, t, current;
  double tex_s, tex_t, s_step, t_step; /* for texture coordinates */

  for (current = 0; current <= patches[surface_number]; current++) {
    /* calculate texture coordinates */
    if (tex_type == TEXELS_PER_SURFACE) {
      /* texture coordinates per surface */
      for (s = 0; s <= s_precision[surface_number]; s++) {
        for (t = 0; t <= t_precision[surface_number]; t++) {
          point[surface_number][current][s][t][eTEXTURE][TEX_S] =
              ((double)point[surface_number][current][s][t][ePOINT][eX] -
               min_b[surface_number][eX]) /
              (max[surface_number][eX] - min_b[surface_number][eX]);
          point[surface_number][current][s][t][eTEXTURE][TEX_T] =
              ((double)point[surface_number][current][s][t][ePOINT][eY] -
               min_b[surface_number][eY]) /
              (max[surface_number][eY] - min_b[surface_number][eY]);
        }
      }
    } else {
      /* texture coordinates per patch */
      s_step = 1.0 / (float)s_precision[surface_number];
      t_step = 1.0 / (float)t_precision[surface_number];

      for (s = 0, tex_s = 0.0; s <= s_precision[surface_number];
           s++, tex_s += s_step) {
        for (t = 0, tex_t = 0.0; t <= t_precision[surface_number];
             t++, tex_t += t_step) {
          point[surface_number][current][s][t][eTEXTURE][TEX_S] = tex_s;
          point[surface_number][current][s][t][eTEXTURE][TEX_T] = tex_t;
        }
      }
    }
  }
}

/* Calculate surface points for this patch */
static void calculate_surface(int surface_number) {
  const float PARTS = 1.0f;
  const float BASIC = 0.5;
  int coord, comp, s, t, i, j;
  /* f and delta f's for each coordinate and component */
  double f[3][3];
  double delta_f[3][3];
  double delta2_f[3][3];
  double delta3_f[3][3];
  /* temporaries for normal calculation */
  double normal_x, normal_y, normal_z, length;
  const int indexes[4] = {ePOINT, eSTANGENT, eTTANGENT, eNORMAL};

  VERBOSE("Calculating surface points ...\n");

  for (coord = 0; coord <= 2; coord++) {
    for (comp = 0; comp <= 2; comp++) {
      for (s = 0; s <= s_precision[surface_number]; s++) {
        f[coord][comp] = d00[coord][comp][0][0];
        delta_f[coord][comp] = d00[coord][comp][0][1];
        delta2_f[coord][comp] = d00[coord][comp][0][2];
        delta3_f[coord][comp] = d00[coord][comp][0][3];
        /* Determine points of constant s */
        for (t = 0; t <= t_precision[surface_number]; t++) {
          point[surface_number][current][s][t][indexes[comp]][coord] =
              f[coord][comp];
          if (indexes[comp] == ePOINT) {
            if (current == 0 && s == 0 && t == 0) {
              /* set the initial values */
              min_b[surface_number][coord] = max[surface_number][coord] =
                  f[coord][comp];
            } else {
              /* check min and max point */
              if (f[coord][comp] < min_b[surface_number][coord])
                min_b[surface_number][coord] = f[coord][comp];
              if (f[coord][comp] > max[surface_number][coord])
                max[surface_number][coord] = f[coord][comp];
            }
          }
          f[coord][comp] += delta_f[coord][comp];
          delta_f[coord][comp] += delta2_f[coord][comp];
          delta2_f[coord][comp] += delta3_f[coord][comp];
        }
        /* Get f and delta f's for this iteration */
        /* Calculate f and Delta f's for next iteration */
        for (i = 0; i <= 2; i++) {
          for (j = 0; j <= 3; j++)
            d00[coord][comp][i][j] += d00[coord][comp][i + 1][j];
        }
      }
    }
  }

  /* calculate vertex normals */
  for (s = 0; s <= s_precision[surface_number]; s++) {
    for (t = 0; t <= t_precision[surface_number]; t++) {

      normal_x = point[surface_number][current][s][t][eSTANGENT][eY] *
                     point[surface_number][current][s][t][eTTANGENT][eZ] -
                 point[surface_number][current][s][t][eSTANGENT][eZ] *
                     point[surface_number][current][s][t][eTTANGENT][eY];
      normal_y = point[surface_number][current][s][t][eSTANGENT][eZ] *
                     point[surface_number][current][s][t][eTTANGENT][eX] -
                 point[surface_number][current][s][t][eSTANGENT][eX] *
                     point[surface_number][current][s][t][eTTANGENT][eZ];
      normal_z = point[surface_number][current][s][t][eSTANGENT][eX] *
                     point[surface_number][current][s][t][eTTANGENT][eY] -
                 point[surface_number][current][s][t][eSTANGENT][eY] *
                     point[surface_number][current][s][t][eTTANGENT][eX];
      length = (float)sqrt((double)(normal_x * normal_x + normal_y * normal_y +
                                    normal_z * normal_z));
      if (length == 0.0) {
        /* add code */ /* how should degenerate patches be handled */
        length = 1.0;
        normal_y = 1.0;
      }

      point[surface_number][current][s][t][eNORMAL][eX] = normal_x / length;
      point[surface_number][current][s][t][eNORMAL][eY] = normal_y / length;
      point[surface_number][current][s][t][eNORMAL][eZ] = normal_z / length;
    }
  }

  /* check for the four corners: adjust the normal a little */
  /* This isn't very nice, but surfaces may be reimplemented anyways */
  /* at least it prevents getting strange effects in the corners */
  s = s_precision[surface_number];
  t = t_precision[surface_number];

  for (coord = 0; coord <= 2; coord++)
    point[surface_number][current][0][0][eNORMAL][coord] =
        point[surface_number][current][0][0][eNORMAL][coord] * BASIC +
        point[surface_number][current][0][1][eNORMAL][coord] * PARTS +
        point[surface_number][current][1][0][eNORMAL][coord] * PARTS;

  Normalize(point[surface_number][current][0][0][eNORMAL]);

  for (coord = 0; coord <= 2; coord++)
    point[surface_number][current][s][0][eNORMAL][coord] =
        point[surface_number][current][s][0][eNORMAL][coord] * BASIC +
        point[surface_number][current][s][1][eNORMAL][coord] * PARTS +
        point[surface_number][current][s - 1][0][eNORMAL][coord] * PARTS;

  Normalize(point[surface_number][current][s][0][eNORMAL]);

  for (coord = 0; coord <= 2; coord++)
    point[surface_number][current][s][t][eNORMAL][coord] =
        point[surface_number][current][s][t][eNORMAL][coord] * BASIC +
        point[surface_number][current][s][t - 1][eNORMAL][coord] * PARTS +
        point[surface_number][current][s - 1][t][eNORMAL][coord] * PARTS;

  Normalize(point[surface_number][current][s][t][eNORMAL]);

  for (coord = 0; coord <= 2; coord++)
    point[surface_number][current][0][t][eNORMAL][coord] =
        point[surface_number][current][0][t][eNORMAL][coord] * BASIC +
        point[surface_number][current][0][t - 1][eNORMAL][coord] * PARTS +
        point[surface_number][current][1][t][eNORMAL][coord] * PARTS;

  Normalize(point[surface_number][current][0][t][eNORMAL]);
}

/* Get control points for next patch */
static void get_patch(void) {
  int flag;
  int i, j;
  float x_value, y_value, z_value;

  /* Increment Patch Index */
  current++;
  if (current >= PATCHES) {
    Message("Maximum number of PATCHES exceeded. ");
    Message("To increase, edit patch.c and recompile\n");
    MyExit(1);
  }

  /* Get Patch Name */
  flag = fscanf(control_fp, "%s\n", patch_name[current]);

  /* Get colors and diffusion factors */
  flag = fscanf(control_fp,
                "TOP COLOR: %d DIFFUSE: %f BOTTOM COLOR: %d DIFFUSE: %f\n",
                &patch_color[surface_number][current][TOP],
                &patch_diffuse[surface_number][current][TOP],
                &patch_color[surface_number][current][BOTTOM],
                &patch_diffuse[surface_number][current][BOTTOM]);

  if (flag == EOF) {
    eof = TRUE;
    current--;
    return;
  } else {
    VERBOSE("Getting control points for %s ...\n", patch_name[current]);
    VERBOSE(" TOP COLOR: %d DIFFUSE: %f BOTTOM COLOR: %d DIFFUSE: %f\n",
            patch_color[surface_number][current][TOP],
            patch_diffuse[surface_number][current][TOP],
            patch_color[surface_number][current][BOTTOM],
            patch_diffuse[surface_number][current][BOTTOM]);
  }

  /* Get neighbouring patch names above */
  flag =
      fscanf(control_fp, "AL: %s A: %s AR: %s\n", neighbour[current][nAL].name,
             neighbour[current][nA].name, neighbour[current][nAR].name);

  if (flag == EOF) {
    eof = TRUE;
    current--;
    return;
  } else
    VERBOSE("AL: %s A: %s AR: %s \n", neighbour[current][nAL].name,
            neighbour[current][nA].name, neighbour[current][nAR].name);

  /* Get neighbouring patch names on the same level */
  flag = fscanf(control_fp, "L: %s R: %s\n", neighbour[current][nL].name,
                neighbour[current][nR].name);

  if (flag == EOF) {
    eof = TRUE;
    current--;
    return;
  } else
    VERBOSE("L: %s R: %s \n", neighbour[current][nL].name,
            neighbour[current][nR].name);

  /* Get neighbouring patch names below */
  flag =
      fscanf(control_fp, "BL: %s B: %s BR: %s\n", neighbour[current][nBL].name,
             neighbour[current][nB].name, neighbour[current][nBR].name);

  if (flag == EOF) {
    eof = TRUE;
    current--;
    return;
  } else
    VERBOSE("BL: %s B: %s BR: %s \n", neighbour[current][nBL].name,
            neighbour[current][nB].name, neighbour[current][nBR].name);

  /* Get control points */
  for (i = 0; i <= 3 && eof != TRUE; i++) {
    for (j = 0; j <= 3 && eof != TRUE; j++) {
      flag = fscanf(control_fp, "%f %f %f", &x_value, &y_value, &z_value);
      if (clp.debug)
        Message(" %d %d X= %f Y= %f Z= %f \n", i, j, x_value, y_value, z_value);
      if (flag == EOF) {
        eof = TRUE;
        current--;
      }
      /* Transform points */

      /* Translate and scale */
      x_value -= cp_offset[eX];
      x_value *= cp_scale;
      y_value -= cp_offset[eY];
      y_value *= cp_scale;
      z_value -= cp_offset[eZ];
      z_value *= cp_scale;

      /* Rotate */
      /* contro_point array has j and i switched to obtain
      proper normal for NURBS */
      control_point[surface_number][current][j][i][eX] =
          control_point_x[surface_number][current][i][j] =
              x_value * left[eX] + y_value * heading[eX] +
              z_value * upvector[eX];

      control_point[surface_number][current][j][i][eY] =
          control_point_y[surface_number][current][i][j] =
              x_value * left[eY] + y_value * heading[eY] +
              z_value * upvector[eY];

      control_point[surface_number][current][j][i][eZ] =
          control_point_z[surface_number][current][i][j] =
              x_value * left[eZ] + y_value * heading[eZ] +
              z_value * upvector[eZ];
    }
  }
}

/* Check patch neighbour connections for consistency */
static int check_connections(void) {
  int patch_index;
  int error_flag;
  int n, p;
  struct Neighbour next;
  static char *location[] = {"Above Left ", "Above",      "Above Right",
                             "Left",        "Right",      "Below Left",
                             "Below",       "Below Right"};

  error_flag = FALSE;
  /* loop over all patches */
  for (p = 0; p <= current; p++) {
    /* loop over all neighbours */
    for (n = nAL; n <= nBR; n++) {
      next = neighbour[p][n];
      next.flag = FALSE;
      next.averaged = FALSE;
      /* if name is not null check it out */
      if (strcmp(next.name, "~") != 0) {
        patch_index = 0;
        while (patch_index <= current &&
               strcmp(patch_name[patch_index], next.name) != 0)
          patch_index++;
        /* if found make sure the corresponding neighbour matches */
        if (patch_index <= current) {
          next.flag = TRUE;
          next.index = patch_index;
          if (strcmp(neighbour[next.index][7 - n].name, patch_name[p]) != 0) {
            Message("%s %s %s %s neighbour mismatch.\n", patch_name[p],
                    location[n], next.name, location[7 - n]);
            error_flag = TRUE;
          }
        } else {
          Message("No patch %s found for %s %s.\n", next.name, patch_name[p],
                  location[n]);
          error_flag = TRUE;
        }
      }
      neighbour[p][n] = next;
      if (clp.debug)
        Message("p= %d n= %d flag= %d index= %d name= %s\n", p, n, next.flag,
                next.index, next.name);
    }
  }
  if (error_flag) {
    Message("\nExiting due to neighbour errors.\n");
    return 0;
    //	MyExit(0);
  }
  return 1;
}

/* calculate vertex normals at patch boundaries
** by averaging the normals of neighbouring vertices */
static void calculate_neighbouring_vertex_normals(void) {
  int s, t, p, n;
  struct Neighbour next;

  VERBOSE("Calculating neighbouring vertex normals ...\n");

  /* loop over all patches */
  for (p = 0; p <= current; p++) {
    /* Loop over all possible neighbours of each patch */
    /* adding in neighbouring vertex normals where necessary */
    for (n = nAL; n <= nBR; n++) {
      next = neighbour[p][n];
      /* If there is a neighbour in this direction and
      ** averaging hasn't occurred yet, add normals */
      if (next.flag && !next.averaged) {
        /* Perform additions appropriate to the direction */
        switch (n) {
        case nAL:
          add_normals(p, 0, 0, next.index, s_precision[surface_number],
                      t_precision[surface_number]);
          break;
        case nA:
          for (t = 0; t <= t_precision[surface_number]; t++)
            add_normals(p, 0, t, next.index, s_precision[surface_number], t);
          break;
        case nAR:
          add_normals(p, 0, t_precision[surface_number], next.index,
                      s_precision[surface_number], 0);
          break;
        case nL:
          for (s = 0; s <= s_precision[surface_number]; s++)
            add_normals(p, s, 0, next.index, s, t_precision[surface_number]);
          break;
        case nR:
          for (s = 0; s <= s_precision[surface_number]; s++)
            add_normals(p, s, t_precision[surface_number], next.index, s, 0);
          break;
        case nBL:
          add_normals(p, s_precision[surface_number], 0, next.index, 0,
                      t_precision[surface_number]);
          break;
        case nB:
          for (t = 0; t <= t_precision[surface_number]; t++)
            add_normals(p, s_precision[surface_number], t, next.index, 0, t);
          break;
        case nBR:
          add_normals(p, s_precision[surface_number],
                      t_precision[surface_number], next.index, 0, 0);
          break;
        default:
          break;
        }
      }
    }

    /* Now normalize where additions occurred */
    for (n = nAL; n <= nBR; n++) {
      next = neighbour[p][n];
      /* If there is a neighbour in this direction and
      ** averaging hasn't occurred yet, normalize and
      ** assign to neighbouring vertices */
      if (next.flag && !next.averaged) {
        /* Perform additions appropriate to the direction */
        switch (n) {
        case nAL:
          calculate_normal(p, 0, 0);
          assign_normal(p, 0, 0, next.index, s_precision[surface_number],
                        t_precision[surface_number]);
          break;
        case nA:
          for (t = 0; t <= t_precision[surface_number]; t++) {
            calculate_normal(p, 0, t);
            assign_normal(p, 0, t, next.index, s_precision[surface_number], t);
          }
          break;
        case nAR:
          calculate_normal(p, 0, t_precision[surface_number]);
          assign_normal(p, 0, t_precision[surface_number], next.index,
                        s_precision[surface_number], 0);
          break;
        case nL:
          for (s = 0; s <= s_precision[surface_number]; s++) {
            calculate_normal(p, s, 0);
            assign_normal(p, s, 0, next.index, s, t_precision[surface_number]);
          }
          break;
        case nR:
          for (s = 0; s <= s_precision[surface_number]; s++) {
            calculate_normal(p, s, t_precision[surface_number]);
            assign_normal(p, s, t_precision[surface_number], next.index, s, 0);
          }
          break;
        case nBL:
          calculate_normal(p, s_precision[surface_number], 0);
          assign_normal(p, s_precision[surface_number], 0, next.index, 0,
                        t_precision[surface_number]);
          break;
        case nB:
          for (t = 0; t <= t_precision[surface_number]; t++) {
            calculate_normal(p, s_precision[surface_number], t);
            assign_normal(p, s_precision[surface_number], t, next.index, 0, t);
          }
          break;
        case nBR:
          calculate_normal(p, s_precision[surface_number],
                           t_precision[surface_number]);
          assign_normal(p, s_precision[surface_number],
                        t_precision[surface_number], next.index, 0, 0);
          break;
        default:
          break;
        }
        /* mark this patch averaged */
        next.averaged = TRUE;
        /* mark this neighbour's edge averaged */
        neighbour[next.index][7 - n].averaged = TRUE;
      }
    }
  }
}

static void add_normals(int patch1, int s1, int t1, int patch2, int s2,
                        int t2) {
  int coord;

  /* add x, y, and z components of normals */
  for (coord = eX; coord <= eZ; coord++)
    point[surface_number][patch1][s1][t1][eNORMAL][coord] +=
        point[surface_number][patch2][s2][t2][eNORMAL][coord];
}

static void calculate_normal(int p, int s, int t) {
  float inv_length;
  float *normal = point[surface_number][p][s][t][eNORMAL];

  /* calculate 1.0/length */
  inv_length = 1.0 / (float)sqrt((double)(normal[eX] * normal[eX] +
                                          normal[eY] * normal[eY] +
                                          normal[eZ] * normal[eZ]));

  /* normalize */
  normal[eX] *= inv_length;
  normal[eY] *= inv_length;
  normal[eZ] *= inv_length;
}

static void assign_normal(int patch1, int s1, int t1, int patch2, int s2,
                          int t2) {
  int coord;

  /* assign x, y, and z components of normals */
  for (coord = eX; coord <= eZ; coord++)
    point[surface_number][patch2][s2][t2][eNORMAL][coord] =
        point[surface_number][patch1][s1][t1][eNORMAL][coord];
}

/* Change the current position to the surface's transformed end point */
void determine_end_point(TURTLE *tu, char desired_surface, double scale) {
  int surface_index;
  float line_scale;

  /* Determine index of desired surface, if none return */
  if ((surface_index = FindSurfaceIndex(desired_surface)) == -1)
    return;
  if (surface_index != 0)
    line_scale = scale;
  else
    line_scale = tu->line_width;

  /* Move the current position to the surface's transformed end point */
  transform_end_point(surface_index, tu, line_scale, scale);
}

/* Change the current position to the surface's transformed end point */
static void transform_end_point(int surface_index, TURTLE *tu, float line_scale,
                                double scale) {
  float x_value, y_value, z_value;

  /* Transform end point into turtle's coordinate system */
  x_value = end_point[surface_index][eX] * line_scale * tu->left[eX] +
            end_point[surface_index][eY] * scale * tu->heading[eX] +
            end_point[surface_index][eZ] * line_scale * tu->up[eX] +
            tu->position[eX];

  y_value = end_point[surface_index][eX] * line_scale * tu->left[eY] +
            end_point[surface_index][eY] * scale * tu->heading[eY] +
            end_point[surface_index][eZ] * line_scale * tu->up[eY] +
            tu->position[eY];

  z_value = end_point[surface_index][eX] * line_scale * tu->left[eZ] +
            end_point[surface_index][eY] * scale * tu->heading[eZ] +
            end_point[surface_index][eZ] * line_scale * tu->up[eZ] +
            tu->position[eZ];

  /* Assign to turtle position */
  tu->position[eX] = x_value;
  tu->position[eY] = y_value;
  tu->position[eZ] = z_value;
}

/***********************************************************************/
/* tells the rayshade output, that a second object should be used, if the
   current texture is deined per patch */
int r_use_second_object(char desired_surface) {
  int surface_index;

  /* Determine index of desired surface, if none return 0 */
  if ((surface_index = FindSurfaceIndex(desired_surface)) == -1)
    return 0;

  if (s_textures[surface_index] == NOTEXTURE)
    return 1;
  else
    return 0;
}

/* Output the surfaces as rayshade objects. Each time it is drawn by
the turtle, an instantiation of the object will be added to the file */

void r_objects(FILE *fp, TURTLE *tu, const DRAWPARAM *dr, VIEWPARAM *vw) {
  int surface_index;
  char gll, ourl, is_texture_per_patch;

  is_texture_per_patch = (find_texture_per_patch() >= 0);

  for (surface_index = 0; surface_index < surface_number; surface_index++) {
    if (surface_identifier[surface_index] != '~') {
      DRAWPARAM tmpDP = *dr;
      /* store the changed values */
      gll = tmpDP.gllighting;
      ourl = tmpDP.ourlighting;
      tmpDP.gllighting = 1;
      tmpDP.ourlighting = 0;

      if (s_textures[surface_index] == NOTEXTURE)
        t_textures[surface_index] = TEXELS_PER_SURFACE;
      else
        t_textures[surface_index] = texture_type(s_textures[surface_index] - 1);

      /* identify surface */
      fprintf(fp, "name %c grid 20 20 20\n", surface_identifier[surface_index]);

      draw_surface_tmesh(tu, surface_identifier[surface_index], &tmpDP, vw,
                         StartTmeshT, TmeshVertexT, EndTmeshT);

      /* complete definition */
      fprintf(fp, "end\n\n");

      /* we don't know ahead, if the texture will be per path or per surface.
      Output also the second one, with texture coordinates per patch. */
      if (s_textures[surface_index] == NOTEXTURE && is_texture_per_patch) {
        t_textures[surface_index] = TEXELS_PER_PATCH;

        /* identify surface */
        fprintf(fp, "name %c2 grid 20 20 20\n",
                surface_identifier[surface_index]);

        draw_surface_tmesh(tu, surface_identifier[surface_index], &tmpDP, vw,
                           StartTmeshT, TmeshVertexT, EndTmeshT);

        /* complete definition */
        fprintf(fp, "end\n\n");
      }

      tmpDP.gllighting = gll;
      tmpDP.ourlighting = ourl;
    }
  }
}

/* Find L-system defined Surface patch */
/* Allocate space if not found */
static surfacePatch *FindSurfacePatch(double patchID) {
  surfacePatch *ptr, *last;
  int row, col;

  /* see if the currrent patch is the one wanted */
  if (currentPatch != NULL && currentPatch->patchID == patchID) {
    return currentPatch;
  }
  /* otherwise check those already allocated */
  ptr = patchList;
  last = NULL;
  while (ptr != NULL) {
    if (ptr->patchID == patchID) {
      return ptr;
    }
    last = ptr;
    ptr = last->nextPatch;
  }

  /* not found, so allocate */
  ptr = (surfacePatch *)Malloc(sizeof(surfacePatch));
  ptr->nextPatch = NULL;
  ptr->patchID = patchID;

  /* add to existing list */
  if (last != NULL) {
    last->nextPatch = ptr;
  } else {
    patchList = ptr;
  }

  /* initialize all points to the origin */
  for (row = 0; row <= 3; row++) {
    for (col = 0; col <= 3; col++) {
      ptr->assigned[row][col] = FALSE;
      ptr->x[row][col] = 0.0;
      ptr->y[row][col] = 0.0;
      ptr->z[row][col] = 0.0;
    }
  }
  return ptr;
}

/* Initialize L-system defined Surface patch */
void SurfacePatchInit(const StringModule *module) {
  int row, col;

  /* find the patch or create if necessary */
  currentPatch = FindSurfacePatch(module->actual[0].value);

  /* use assigned basis; if not specified use Bezier */
  if (module->parameters >= 2 && module->actual[1].value >= BEZIER &&
      module->actual[1].value <= CARDINAL) {
    currentPatch->basis = (int)module->actual[1].value;
  } else {
    currentPatch->basis = BEZIER;
  }

  /* initialize all points to the origin */
  for (row = 0; row <= 3; row++) {
    for (col = 0; col <= 3; col++) {
      currentPatch->assigned[row][col] = FALSE;
      currentPatch->x[row][col] = 0.0;
      currentPatch->y[row][col] = 0.0;
      currentPatch->z[row][col] = 0.0;
    }
  }
}

/* Record control point in L-system defined Surface patch */
void SurfacePatchControlPoint(const StringModule *module,
                              const TURTLE *turtle) {
  int row, col;

  if (module->parameters < 3) {
    Message("WARNING: Control point (@PC) with too few parameters ignored.\n");
    return;
  }

  /* find the patch or create if necessary */
  currentPatch = FindSurfacePatch(module->actual[0].value);
  row = (int)module->actual[1].value;
  col = (int)module->actual[2].value;

  if (currentPatch->assigned[row][col]) {
    Message("WARNING: Control point %.0f,%d,%d being reassigned.\n",
            module->actual[0].value, row, col);
  }
  currentPatch->assigned[row][col] = TRUE;
  currentPatch->x[row][col] = turtle->position[0];
  currentPatch->y[row][col] = turtle->position[1];
  currentPatch->z[row][col] = turtle->position[2];
  currentPatch->col_index[row][col] = turtle->color_index;
}

/* Send L-system defined Surface patch to rendering routines */
void SurfacePatchDraw(
    StringModule *module, TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw,
    void (*StartPatches)(TURTLE *, DRAWPARAM *, VIEWPARAM *, int, int, int),
    /* parameters: tu, dr, vw, s- and t-precision, basisID */
    void (*RenderPatch)(TURTLE *, DRAWPARAM *, VIEWPARAM *, double[4][4],
                        double[4][4], double[4][4]),
    /* parameters: tu, dr, vw, x-, y- and z-control points*/
    void (*EndPatches)(TURTLE *, DRAWPARAM *, VIEWPARAM *)
    /* parameters: tu, dr, vw */) {
  int row, col, errorFlag;
  int sprecision, tprecision;

  /* find the patch */
  currentPatch = FindSurfacePatch(module->actual[0].value);

  /* check that all points have been assigned */
  errorFlag = FALSE;
  for (row = 0; row <= 3; row++) {
    for (col = 0; col <= 3; col++) {
      if (!currentPatch->assigned[row][col]) {
        Message("WARNING: Control point %.0f,%d,%d not assigned.\n",
                module->actual[0].value, row, col);
        errorFlag = TRUE;
      }
    }
  }
  if (!errorFlag) {
    /* use precision parameters if present */
    if (module->parameters >= 3) {
      sprecision = (int)module->actual[1].value;
      tprecision = (int)module->actual[2].value;
      /* check for out of range precision */
      if (tprecision > T_COUNT) {
        Message("WARNING: Maximum tprecision %d used instead of %d.\n", T_COUNT,
                tprecision);
        tprecision = T_COUNT;
      }
      if (sprecision > S_COUNT) {
        Message("WARNING: Maximum sprecision %d used instead of %d.\n", S_COUNT,
                sprecision);
        sprecision = S_COUNT;
      }
    } else {
      /* no precision parameters found; average assumed */
      sprecision = S_COUNT / 2;
      tprecision = T_COUNT / 2;
    }

    (*StartPatches)(tu, dr, vw, sprecision, tprecision, currentPatch->basis);

    (*RenderPatch)(tu, dr, vw, currentPatch->x, currentPatch->y,
                   currentPatch->z);

    (*EndPatches)(tu, dr, vw);
  }
}

/**************************************************************************/
void NURBS_surface(int surface_index, const DRAWPARAM *dr) {
  int p;
  GLfloat knots[8] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};
  GLUnurbsObj *theNurb;

  theNurb = gluNewNurbsRenderer();

  switch (dr->render_mode) {
  case RM_WIREFRAME:
#ifndef MESA
    gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
#endif
    break;
  default:
    gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);
  }

  gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, dr->nurbs_sampling);

  /* nurbs out of view volume will not be tesselated */
  gluNurbsProperty(theNurb, GLU_CULLING, GL_TRUE);

  if (dr->gllighting)
    glEnable(GL_AUTO_NORMAL);

  /* loop over all patches */
  for (p = 0; p <= patches[surface_index]; p++) {
    gluBeginSurface(theNurb);
    gluNurbsSurface(theNurb, 8, knots, 8, knots, 4 * 3, 3,
                    &control_point[surface_index][p][0][0][0], 4, 4,
                    GL_MAP2_VERTEX_3);

    if (dr->texture)
      gluNurbsSurface(theNurb, 8, knots, 8, knots, 4 * 2, 2, &N_texels[0][0][0],
                      4, 4, GL_MAP2_TEXTURE_COORD_2);

    gluEndSurface(theNurb);
  }

  glDisable(GL_AUTO_NORMAL);
}

/**************************************************************************/
/* Send L-system defined Surface patch to rendering routines */
/* broken down into Tmesh vertices with colour */
void SurfaceTmeshDraw(StringModule *module, TURTLE *tu, DRAWPARAM *dr,
                      VIEWPARAM *vw,
                      void (*StartTmesh)(void), /* no parameters */
                      void (*TmeshVertex)(const float *, const DRAWPARAM *),
                      /* parameters: point(pos, normal, and texels), color,
                         and renderind specification */
                      void (*EndTmesh)(void)) /* no parameters */
{
  int row, col, errorFlag;
  /*int indexes[4] = { ePOINT, eSTANGENT, eTTANGENT, eNORMAL }; */
  Colorindex temp[T_COUNT + 1];
  Colorindex temp_back[T_COUNT + 1];
  int tex_index;

  int s, t;

  /* find the patch */
  currentPatch = FindSurfacePatch(module->actual[0].value);

  /* check that all points have been assigned */
  errorFlag = FALSE;
  for (row = 0; row <= 3; row++) {
    for (col = 0; col <= 3; col++) {
      if (!currentPatch->assigned[row][col]) {
        Message("WARNING: Control point %.0f,%d,%d not assigned.\n",
                module->actual[0].value, row, col);
        errorFlag = TRUE;
      }
    }
  }
  if (errorFlag)
    return;

  /* use precision parameters if present */
  if (module->parameters >= 3) {
    s_precision[LDSIndex] = (int)module->actual[1].value;
    t_precision[LDSIndex] = (int)module->actual[2].value;
    /* check for out of range precision */
    if (t_precision[LDSIndex] > T_COUNT) {
      Message("WARNING: Maximum tprecision %d used instead of %d.\n", T_COUNT,
              t_precision[LDSIndex]);
      t_precision[LDSIndex] = T_COUNT;
    }
    if (s_precision[LDSIndex] > S_COUNT) {
      Message("WARNING: Maximum sprecision %d used instead of %d.\n", S_COUNT,
              s_precision[LDSIndex]);
      s_precision[LDSIndex] = S_COUNT;
    }
  } else {
    /* no precision parameters found; average assumed */
    s_precision[LDSIndex] = S_COUNT / 2;
    t_precision[LDSIndex] = T_COUNT / 2;
  }

  /* calculate and draw shaded surface */
  initialize_matrices(s_precision[LDSIndex], t_precision[LDSIndex]);
  /* according to  appropriate basis matrices */
  switch (currentPatch->basis) {
  case BEZIER:
    calculate_d00(currentPatch->x, currentPatch->y, currentPatch->z, m_bezier,
                  m_bezier_t);
    break;
  case BSPLINE:
    calculate_d00(currentPatch->x, currentPatch->y, currentPatch->z, m_B_spline,
                  m_B_spline_t);
    break;
  case CARDINAL:
    calculate_d00(currentPatch->x, currentPatch->y, currentPatch->z, m_cardinal,
                  m_cardinal_t);
    break;
  default:
    break;
  }

  VERBOSE("Calculating and drawing L-system controlled surface ...\n");
  /* Calculate surface points for this patch */
  calculate_surface(LDSIndex);

  patches[LDSIndex] = 0;

  tex_index = tu->texture;

  if (is_valid_texture_index(tex_index))
    set_texture_coordinates(LDSIndex, texture_type(tex_index));

  dr->texture = dr->tdd->StartTexture(tex_index);

  /* NURBS are slower */
  if ((dr->output_type == TYPE_OPENGL) && (dr->nurbs_sampling > 0.0))
    NURBS_surface(LDSIndex, dr);
  else {
    if (dr->render_mode == RM_INTERPOLATED) {
      for (t = 0; t <= t_precision[LDSIndex]; t++) {
        temp[t] = tu->color_index;
        temp_back[t] = tu->color_index_back;
      }
    }

    for (s = 0; s < s_precision[LDSIndex]; s++) {
      (*StartTmesh)();

      for (t = 0; t <= t_precision[LDSIndex]; t++) {
        if (dr->ourlighting)
          if (s == 0)
            determine_vertex_shading(LDSIndex, 0, s, t, &temp[t], &temp_back[t],
                                     tu, dr, vw);

        /* be aware, that this writes to another array (for stangent) */
        //[PASCAL] next 2 lines are wrong: COLOR_FRONT = 8 and COLOR_BACK = 9
        // while point is an array of 3 elements only => it generates an out of
        // bound I changed to remove the warning, this code it's probably never
        // called point[LDSIndex][0][s][t][ePOINT][COLOR_FRONT] = temp[t];
        // point[LDSIndex][0][s][t][ePOINT][COLOR_BACK] = temp_back[t];
        point[LDSIndex][0][s][t][ePOINT][COLOR_FRONT] = temp[t];
        point[LDSIndex][0][s][t][ePOINT][COLOR_BACK] = temp_back[t];

        (*TmeshVertex)(point[LDSIndex][0][s][t][ePOINT], dr);

        if (dr->ourlighting)
          determine_vertex_shading(LDSIndex, 0, s + 1, t, &temp[t],
                                   &temp_back[t], tu, dr, vw);

        /* be aware, that this writes to another array (for stangent) */
        // [PASCAL] same comment as above
        // point[LDSIndex][0][s + 1][t][ePOINT][COLOR_FRONT] = temp[t];
        // point[LDSIndex][0][s + 1][t][ePOINT][COLOR_BACK] = temp_back[t];
        point[LDSIndex][0][s + 1][t][ePOINT][COLOR_FRONT] = temp[t];
        point[LDSIndex][0][s + 1][t][ePOINT][COLOR_BACK] = temp_back[t];

        (*TmeshVertex)(point[LDSIndex][0][s + 1][t][ePOINT], dr);
      }
      (*EndTmesh)();
    }
  }

  if (dr->texture)
    dr->tdd->EndTexture(tex_index);
}

/********************************************************************/
/* draw_surface_patch passes patches making up the id surface to    */
/* the given rendering routines                                     */
/********************************************************************/

void draw_surface_patch(
    TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, char desired_surface,
    void (*StartPatches)(const TURTLE *, DRAWPARAM *, VIEWPARAM *, int, int,
                         int),
    void (*RenderPatch)(TURTLE *, DRAWPARAM *, VIEWPARAM *, double[4][4],
                        double[4][4], double[4][4]),
    void (*EndPatches)(TURTLE *, DRAWPARAM *, VIEWPARAM *)) {
  int p;
  int surface_index;

  /* is the surface a tsurface? */
  if ((surface_index = FindTSurfaceIndex(desired_surface)) != -1) {
    draw_tsurface(tu, desired_surface, dr, vw);
    return;
  }

  /* Determine index of desired surface, if none return */
  if ((surface_index = FindSurfaceIndex(desired_surface)) == -1)
    return;

  (*StartPatches)(tu, dr, vw, s_precision[surface_index],
                  t_precision[surface_index], BEZIER);

  for (p = 0; p <= patches[surface_index]; p++) {
    (*RenderPatch)(tu, dr, vw, control_point_x[surface_index][p],
                   control_point_y[surface_index][p],
                   control_point_z[surface_index][p]);
  }

  (*EndPatches)(tu, dr, vw);
}

/**************************************************************************/
int GetSurfaceTexture(char desired_surface, TURTLE *tu) {
  int surface_index;

  /* is the surface a tsurface? */
  if ((surface_index = FindTSurfaceIndex(desired_surface)) != -1) {
    return tu->texture;
  }

  /* Determine index of desired surface, if none return 0 */
  if ((surface_index = FindSurfaceIndex(desired_surface)) == -1)
    return 0;

  return s_textures[surface_index] == NOTEXTURE ? tu->texture
                                                : s_textures[surface_index] - 1;
}

/**************************************************************************/
/* draw_surface_tmesh passes tmesh vertices, composed of points           */
/* if necessary, color, normal, or texture coordinates are computed for   */
/* each vertex                                                            */
/**************************************************************************/

void draw_surface_tmesh(TURTLE *tu, char desired_surface, DRAWPARAM *dr,
                        VIEWPARAM *vw,
                        void (*StartTmesh)(void), /* no parameters */
                        void (*TmeshVertex)(const float *, const DRAWPARAM *),
                        /* parameters: point  */
                        void (*EndTmesh)(void)) /* no parameters */
{
  int p;
  int s, t;
  int surface_index;
  int tex_index = -1;
  Colorindex temp[T_COUNT + 1];
  Colorindex temp_back[T_COUNT + 1];

  /* is the surface a tsurface? */
  if ((surface_index = FindTSurfaceIndex(desired_surface)) != -1) {
    draw_tsurface(tu, desired_surface, dr, vw);
    return;
  }

  /* Determine index of desired surface, if none return */
  if ((surface_index = FindSurfaceIndex(desired_surface)) == -1)
    return;

  if (dr->output_type == TYPE_RAYSHADE) {
    /* in rayshade output, the surfaces are instantiated, so the texture
                index is currently not known */
    if (t_textures[surface_index] != NO_TEXELS_YET) {
      set_texture_coordinates(surface_index, t_textures[surface_index]);
      dr->texture = 1;
    } else
      dr->texture = 0;
  } else {
    tex_index = s_textures[surface_index] == NOTEXTURE
                    ? tu->texture
                    : s_textures[surface_index] - 1;

    if (is_valid_texture_index(tex_index) &&             /* is valid texture */
        ((t_textures[surface_index] == NO_TEXELS_YET) || /* not set yet  OR */
         (texture_type(tex_index) != t_textures[surface_index]))) {
      /* change of the type */

      set_texture_coordinates(surface_index, texture_type(tex_index));
    }

    dr->texture = dr->tdd->StartTexture(tex_index);
  }

  /* NURBS are slower */
  if ((dr->output_type == TYPE_OPENGL) && (dr->nurbs_sampling > 0.0))
    NURBS_surface(surface_index, dr);
  else {
    if (dr->render_mode == RM_INTERPOLATED)
      for (t = 0; t <= t_precision[surface_index]; t++) {
        temp[t] = tu->color_index;
        temp_back[t] = tu->color_index_back;
      }

    /* loop over all patches */
    for (p = 0; p <= patches[surface_index]; p++) {
      for (s = 0; s < s_precision[surface_index]; s++) {

        (*StartTmesh)();

        for (t = 0; t <= t_precision[surface_index]; t++) {
          if (dr->ourlighting)
            if (s == 0)
              determine_vertex_shading(surface_index, p, s, t, &temp[t],
                                       &temp_back[t], tu, dr, vw);

          /* be aware, that this writes to another array (for stangent) */
          //[PASCAL] next 2 lines are wrong: COLOR_FRONT = 8 and COLOR_BACK = 9
          // while point is an array of 3 elements only => it generates an out
          // of bound I changed to remove the warning, this code it's probably
          // never called point[surface_index][p][s][t][ePOINT][COLOR_FRONT] =
          // temp[t]; point[surface_index][p][s][t][ePOINT][COLOR_BACK] =
          // temp_back[t];
          point[surface_index][p][s][t][ePOINT][COLOR_FRONT] = temp[t];
          point[surface_index][p][s][t][ePOINT][COLOR_BACK] = temp_back[t];

          (*TmeshVertex)(point[surface_index][p][s][t][ePOINT], dr);

          if (dr->ourlighting)
            determine_vertex_shading(surface_index, p, s + 1, t, &temp[t],
                                     &temp_back[t], tu, dr, vw);

          /* be aware, that this writes to another array (for stangent) */
          // [PASCAL] Same comment as above
          // point[surface_index][p][s + 1][t][ePOINT][COLOR_FRONT] = temp[t];
          // point[surface_index][p][s + 1][t][ePOINT][COLOR_BACK] =
          // temp_back[t];
          point[surface_index][p][s + 1][t][ePOINT][COLOR_FRONT] = temp[t];
          point[surface_index][p][s + 1][t][ePOINT][COLOR_BACK] = temp_back[t];

          (*TmeshVertex)(point[surface_index][p][s + 1][t][ePOINT], dr);
        }
        (*EndTmesh)();
      }
    }
  }

  if (dr->output_type != TYPE_RAYSHADE) {
    if (dr->texture)
      dr->tdd->EndTexture(tex_index);
  }
}

/* Set shading parameters for this vertex (Iris GL version) */
/* probably should go to rendering routines */
static void determine_vertex_shading(int surface, int p, int s, int t,
                                     Colorindex *color_index,
                                     Colorindex *color_index_back, TURTLE *tu,
                                     const DRAWPARAM *dr, VIEWPARAM *vw) {
  int color_value;
  int hue;
  float intensity;
  float diffuse;
  float vertex_lamp, vertex_view;
  float n_x, n_y, n_z, distance;
  int side;
  Colorindex index;

  /* use turtle colour if simple fill */
  if (dr->render_mode == RM_FILLED) {
    *color_index = tu->color_index;
    *color_index_back = tu->color_index_back;
    return;
  }

  /* otherwise, Determine shading at the vertex */

  /* Transform the normal, then perform the shading calculation */
  n_x = point[surface][p][s][t][eNORMAL][eX] * tu->left[eX] +
        point[surface][p][s][t][eNORMAL][eY] * tu->heading[eX] +
        point[surface][p][s][t][eNORMAL][eZ] * tu->up[eX];

  n_y = point[surface][p][s][t][eNORMAL][eX] * tu->left[eY] +
        point[surface][p][s][t][eNORMAL][eY] * tu->heading[eY] +
        point[surface][p][s][t][eNORMAL][eZ] * tu->up[eY];

  n_z = point[surface][p][s][t][eNORMAL][eX] * tu->left[eZ] +
        point[surface][p][s][t][eNORMAL][eY] * tu->heading[eZ] +
        point[surface][p][s][t][eNORMAL][eZ] * tu->up[eZ];

  distance = 1.0 / (float)sqrt((double)(n_x * n_x + n_y * n_y + n_z * n_z));
  n_x *= distance;
  n_y *= distance;
  n_z *= distance;

  /* Calculate dot product of vertex normal and view normal */
  vertex_view = n_x * vw->view_normal[eX] + n_y * vw->view_normal[eY] +
                n_z * vw->view_normal[eZ];

  /* Determine which side we are on */
  side = vertex_view < 0.0 ? 0 : 1;

  /* Calculate dot product of vertex normal and lamp normal */
  vertex_lamp = n_x * dr->light_dir[eX] + n_y * dr->light_dir[eY] +
                n_z * dr->light_dir[eZ];

  /* Determine color value on scale of 0-255 using polygon color if
   ** non-zero */
  color_value = (patch_color[surface][p][side] != 0)
                    ? patch_color[surface][p][side]
                    : (tu->color_index - clp.colormap);

  /* Determine hue of color */
  hue = (int)(color_value / 64);

  /* Determine intensity of color requested on a scale of 0-1 */
  intensity = (color_value - hue * 64.0) / 64.0;

  /* Use given diffuse if not zero */
  diffuse = (patch_diffuse[surface][p][side] != 0.0)
                ? patch_diffuse[surface][p][side]
                : dr->diffuse;

  /* Determine color indices based on intensity,
  ** ambient light, and the contribution of diffuse
  ** light at this viewing angle. */

  index = (int)(64. * intensity * (dr->ambient + diffuse * absf(vertex_lamp)));

  if (index < 1)
    index = 0;
  if (index > 63)
    index = 63;

  index += hue * 64 + clp.colormap;

  *color_index = index;

  if (dr->double_sided) {
    /* Determine color value on scale of 0-255 using polygon color if
     ** non-zero */
    color_value = (patch_color[surface][p][side] != 0)
                      ? patch_color[surface][p][side]
                      : (tu->color_index_back - clp.colormap);

    /* Determine hue of color */
    hue = (int)(color_value / 64);

    /* Determine intensity of color requested on a scale of 0-1 */
    intensity = (color_value - hue * 64.0) / 64.0;

    /* Use given diffuse if not zero */
    diffuse = (patch_diffuse[surface][p][side] != 0.0)
                  ? patch_diffuse[surface][p][side]
                  : dr->diffuse;

    /* Determine color indices based on intensity,
    ** ambient light, and the contribution of diffuse
    ** light at this viewing angle. */

    index =
        (int)(64. * intensity * (dr->ambient + diffuse * absf(vertex_lamp)));

    if (index < 1)
      index = 0;
    if (index > 63)
      index = 63;

    index += hue * 64 + clp.colormap;
  }

  *color_index_back = index;
}

/* Generate a "blueprint" Wavefront object of the surface and store it in
memory. Each time the surface is required by the model, the blueprint is
transformed and the transformed surface written out to the object file. */

void objCreateSurfaceMesh(int maxTriangles, char surfaceIdentifier);

void o_objects(__attribute__((unused)) FILE *fp, TURTLE *tu, DRAWPARAM *dr,
               VIEWPARAM *vw) {
  int surface_index, maxTriangles;
  char gll, ourl, is_texture_per_patch;

  is_texture_per_patch = (find_texture_per_patch() >= 0);

  for (surface_index = 0; surface_index < surface_number; surface_index++) {
    char surfaceIdentifier = surface_identifier[surface_index];
    if (surfaceIdentifier != '~') {
      /* store the changed values */
      gll = dr->gllighting;
      ourl = dr->ourlighting;
      dr->gllighting = 1;
      dr->ourlighting = 0;

      if (s_textures[surface_index] == NOTEXTURE)
        t_textures[surface_index] = TEXELS_PER_SURFACE;
      else
        t_textures[surface_index] = texture_type(s_textures[surface_index] - 1);

      maxTriangles = (patches[surface_index] + 1) * s_precision[surface_index] *
                     (t_precision[surface_index] + 1) * 4;
      objCreateSurfaceMesh(maxTriangles, surfaceIdentifier);

      draw_surface_tmesh(tu, surface_identifier[surface_index], dr, vw,
                         StartTmeshT, TmeshVertexT, EndTmeshT);

      dr->gllighting = gll;
      dr->ourlighting = ourl;
    }
  }
}
