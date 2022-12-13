/* scene3d.c - implementation of a 3d scene */

#include "quasiMC.h"
#include "randquasimc.h" // this is needed to call RandU01 in the RandomLight function

#define DEFAULT_NUM_PRIMITIVES 256
#define DEFAULT_NUM_MATERIALS 8
#define DEFAULT_NUM_LIGHTS 8
#define DEFAULT_CELL_LIST_SIZE 128

#define TRIANGLE 3 /* defines a triangle - with 3 vertices */
#define POLYGON 4  /* defines a quad - with 4 vertices */

/* parameters for all scenes */
extern int verbose;

/* local prototypes */
void MatrixFromHeader(CTURTLE *tu, float *invmatrix);
int AddSurface(SCENE *scene, int id, float scale[3], float *invmatrix,
               float *up, Cmodule_type *module, int material_param);
int AddTriangleCylinder(SCENE *scene, float *invmatrix, float *up,
                        Cmodule_type *module);
int SetPrimitive(SCENE *scene, int obj_type, float *data, float *up,
                 int top_mat, int bot_mat);
void CheckPrimitivesSize(SCENE *scene);
int In2dPolygon(float pt[2], float vert[4][2]);
int In2dTriangle(float pt[2], float vert[4][2]);
void BoundObject(SCENE *scene, int obj_type, float *data);
void AddToCell(CELL *cell, int prim_index);
int triBoxOverlap(float boxcenter[3], float boxhalfsize[3],
                  float triverts[3][3]);

extern int numSides;
static float *ptsBot = NULL;
static float *ptsTop = NULL;

/* ------------------------------------------------------------------------- */

void InitScene(SCENE *scene) {
  if (scene->primitives != NULL)
    return;

  scene->max_primitives = DEFAULT_NUM_PRIMITIVES;
  scene->num_primitives = 0;
  if ((scene->primitives = (PRIMITIVE *)malloc(scene->max_primitives *
                                               sizeof(PRIMITIVE))) == NULL) {
    fprintf(stderr, "scene3d - cannot allocate memory for primitives\n");
    return;
  }

  scene->max_materials = DEFAULT_NUM_MATERIALS;
  scene->num_materials = 0;

  if ((scene->materials = (MATERIAL *)malloc(scene->max_materials *
                                             sizeof(MATERIAL))) == NULL) {
    fprintf(stderr, "scene3d - cannot allocate memory for materials\n");
    return;
  }

  scene->max_lights = DEFAULT_NUM_LIGHTS;
  scene->num_lights = 0;
  scene->sum_light_weights = 0.f;
  scene->max_light_weight = 0.f;
  if ((scene->lights = (LIGHT *)malloc(scene->max_lights * sizeof(LIGHT))) ==
      NULL) {
    fprintf(stderr, "scene3d - cannot allocate memory for lights\n");
    return;
  }

  memset(scene->surfaces, 0, sizeof(SURFACE) * MAX_SURFACES);
  scene->num_surfaces = 0;

  memset(scene->grid.size, 0, sizeof(int) * 3);
  memset(scene->grid.default_size, 0, sizeof(int) * 3);
  memset(scene->grid.cell_size, 0, sizeof(float) * 3);
  memset(scene->grid.bsph_centre, 0, sizeof(float) * 3);
  scene->grid.bsph_radius = 0.0;
  memset(scene->grid.bbox, 0, sizeof(float) * 6);
  memset(scene->grid.range, 0, sizeof(float) * 6);
  scene->grid.maxdist = 0.0;
  scene->grid.num_cells = 0;
  scene->grid.cells = NULL;

  ptsBot = NULL;
  ptsTop = NULL;

  return;
}

/* ------------------------------------------------------------------------- */

void ResetScene(SCENE *scene)
/* clears the scene of primitives - does not remove lights and materials. */
{
  int i;

  if (scene == NULL)
    return;

  scene->grid.size[X] = scene->grid.default_size[X];
  scene->grid.size[Y] = scene->grid.default_size[Y];
  scene->grid.size[Z] = scene->grid.default_size[Z];

  scene->num_primitives = 0;
  memset(scene->grid.bsph_centre, 0, sizeof(float) * 3);
  scene->grid.bsph_radius = 0.0;
  memset(scene->grid.bbox, 0, sizeof(float) * 6);
  memset(scene->grid.range, 0, sizeof(float) * 6);
  scene->grid.maxdist = 0.0;

  if (scene->grid.cells == NULL) {
    scene->grid.num_cells =
        scene->grid.size[X] * scene->grid.size[Y] * scene->grid.size[Z];

    if ((scene->grid.cells =
             (CELL *)malloc(sizeof(CELL) * scene->grid.num_cells)) == NULL) {
      fprintf(stderr, "scene3d - cannot allocate memory for grid");
      scene->grid.cells = NULL;
    }

    /* clear primitive list for each cell */
    for (i = 0; i < scene->grid.num_cells; i++) {
      scene->grid.cells[i].num_primitives = 0;
      scene->grid.cells[i].max_primitives = DEFAULT_CELL_LIST_SIZE;
      scene->grid.cells[i].list = NULL;
    }
  }

  if (ptsBot == NULL) {
    if ((ptsBot = (float *)malloc(numSides * sizeof(float) * 3)) == NULL) {
      fprintf(stderr, "scene3d - cannot allocate memory for cylinder points\n");
      return;
    }
  }
  if (ptsTop == NULL) {
    if ((ptsTop = (float *)malloc(numSides * sizeof(float) * 3)) == NULL) {
      fprintf(stderr, "scene3d - cannot allocate memory for cylinder points\n");
      return;
    }
  }

  return;
}

/* ------------------------------------------------------------------------- */

void FreeScene(SCENE *scene)
/* frees the primitives from memory */
{
  int i;

  if (scene == NULL)
    return;

  scene->num_primitives = 0;
  memset(scene->grid.bsph_centre, 0, sizeof(float) * 3);
  scene->grid.bsph_radius = 0.0;
  memset(scene->grid.bbox, 0, sizeof(float) * 6);
  memset(scene->grid.range, 0, sizeof(float) * 6);
  scene->grid.maxdist = 0.0;

  if (scene->primitives != NULL) {
    free(scene->primitives);
    scene->primitives = NULL;
  }

  scene->num_materials = 0;
  if (scene->materials != NULL) {
    free(scene->materials);
    scene->materials = NULL;
  }

  scene->num_lights = 0;
  if (scene->lights != NULL) {
    free(scene->lights);
    scene->lights = NULL;
  }
  scene->sum_light_weights = 0.0;
  scene->max_light_weight = 0.0;

  if (scene->grid.cells != NULL) {
    for (i = 0; i < scene->grid.num_cells; i++)
      if (scene->grid.cells[i].list != NULL)
        free(scene->grid.cells[i].list);
    free(scene->grid.cells);
    scene->grid.cells = NULL;
  }

  if (ptsBot != NULL) {
    free(ptsBot);
    ptsBot = NULL;
  }
  if (ptsTop != NULL) {
    free(ptsTop);
    ptsTop = NULL;
  }

  return;
}

/* ------------------------------------------------------------------------- */

void ResetGrid(SCENE *scene, int x_size, int y_size, int z_size) {
  int i;

  if (x_size == scene->grid.size[X] && y_size == scene->grid.size[Y] &&
      z_size == scene->grid.size[Z])
    return;

  scene->grid.size[X] = x_size;
  scene->grid.size[Y] = y_size;
  scene->grid.size[Z] = z_size;

  if (scene->grid.cells != NULL) {
    // reallocate memory;
    scene->grid.num_cells =
        scene->grid.size[X] * scene->grid.size[Y] * scene->grid.size[Z];

    if ((scene->grid.cells =
             (CELL *)realloc(scene->grid.cells,
                             sizeof(CELL) * scene->grid.num_cells)) == NULL) {
      fprintf(stderr, "scene3d - cannot re-allocate memory for grid");
      scene->grid.cells = NULL;
    }

    /* clear primitive list for each cell */
    for (i = 0; i < scene->grid.num_cells; i++) {
      scene->grid.cells[i].num_primitives = 0;
      scene->grid.cells[i].max_primitives = DEFAULT_CELL_LIST_SIZE;
      scene->grid.cells[i].list = NULL;
    }
  } else {
    scene->grid.num_cells =
        scene->grid.size[X] * scene->grid.size[Y] * scene->grid.size[Z];

    if ((scene->grid.cells =
             (CELL *)malloc(sizeof(CELL) * scene->grid.num_cells)) == NULL) {
      fprintf(stderr, "scene3d - cannot allocate memory for grid");
      scene->grid.cells = NULL;
    }

    /* clear primitive list for each cell */
    for (i = 0; i < scene->grid.num_cells; i++) {
      scene->grid.cells[i].num_primitives = 0;
      scene->grid.cells[i].max_primitives = DEFAULT_CELL_LIST_SIZE;
      scene->grid.cells[i].list = NULL;
    }
  }
}

/* ------------------------------------------------------------------------- */

void CheckPrimitivesSize(SCENE *scene) {
  if (scene->num_primitives >= scene->max_primitives) {
    scene->max_primitives *= 2;
    if ((scene->primitives = (PRIMITIVE *)realloc(
             scene->primitives, scene->max_primitives * sizeof(PRIMITIVE))) ==
        NULL) {
      fprintf(stderr, "scene3d - cannot reallocate memory for primitives\n");
      return;
    }
    if (verbose >= 2)
      fprintf(stderr, "scene3d - reallocated memory for primitives\n");
  }
  return;
}

/* ------------------------------------------------------------------------- */

int AddObject(SCENE *scene, CTURTLE *turtle, Cmodule_type *module,
              int material_param)
/* returns index of object in the array of primitives */
{
  float invmatrix[16];
  float pars[12];
  float pt[4][3];
  float surface_scale[3];
  float ang_in_rads;
  int top_mat, bot_mat;

  /* check if at least two parameters are given */
  if (module->num_params >= 2) {
    MatrixFromHeader(turtle, invmatrix);

    /* if one parameter is 0, polygon has area = 0, unless its a surface with id
     * == 0 */
    if ((module->symbol[0] != 'S') && (module->symbol[0] != 'U') &&
        (module->params[0].value <= 0.f || module->params[1].value <= 0.f)) {
      fprintf(stderr, "QuasiMC - Warning: object has area = 0.0\n");
      return (-1);
    }

    memset(pt, 0, sizeof(float) * 4 * 3);
  } else {
    fprintf(stderr, "QuasiMC - object needs at least two parameters\n");
    return (-1);
  }

  /* check for material parameters defined in the module */
  // I moved the code into the conditionals that depend on the type of module:
  // P, T, U or S for P or T modules assume the third and fourth parameters have
  // material indicies, but this will not work for S modules (the 'material
  // parameter' values is needed for these) see AddSurface for U module the 5th
  // and 6th parameters are the material ones

  /* rhombus */
  if (module->symbol[0] == 'P') {
    if (module->num_params >= 3)
      top_mat = (int)module->params[2].value;
    else
      top_mat = 0;
    if (module->num_params >= 4)
      bot_mat = (int)module->params[3].value;
    else
      bot_mat = top_mat;

    pt[1][Z] = module->params[0].value;
    pt[3][Z] = -module->params[0].value;

    pt[1][Y] = pt[3][Y] = module->params[1].value / 2.0f;
    pt[2][Y] = module->params[1].value;

    Transform3Point(pt[0], invmatrix, pars + 0);
    Transform3Point(pt[1], invmatrix, pars + 3);
    Transform3Point(pt[2], invmatrix, pars + 6);
    Transform3Point(pt[3], invmatrix, pars + 9);

    return (SetPrimitive(scene, POLYGON, pars, turtle->up, top_mat, bot_mat));
  }

  /* isosceles triangle */
  if (module->symbol[0] == 'T') {
    if (module->num_params >= 3)
      top_mat = (int)module->params[2].value;
    else
      top_mat = 0;
    if (module->num_params >= 4)
      bot_mat = (int)module->params[3].value;
    else
      bot_mat = top_mat;

    /* Y axis is along heading */
    /* thus param[1] should go to Y coordinate */
    /* param[0] to Z coordinate */

    pt[0][Z] = -module->params[0].value;
    pt[1][Z] = module->params[0].value;
    pt[2][Y] = module->params[1].value;

    Transform3Point(pt[0], invmatrix, pars + 0);
    Transform3Point(pt[1], invmatrix, pars + 3);
    Transform3Point(pt[2], invmatrix, pars + 6);

    /* it is faster not to use inv matrix */
    return (SetPrimitive(scene, TRIANGLE, pars, turtle->up, top_mat, bot_mat));
  }

  /* triangle */
  if (module->symbol[0] == 'U') {
    // check if at least four parameters are given: length1, length2, height,
    // angle
    if (module->num_params < 4) {
      fprintf(stderr, "QuasiMC - the 'U' module requires four parameters.\n");
      return (-1);
    }

    // check for material parameters, in this case the 5th and 6th
    if (module->num_params >= 5)
      top_mat = (int)module->params[4].value;
    else
      top_mat = 0;
    if (module->num_params >= 6)
      bot_mat = (int)module->params[5].value;
    else
      bot_mat = top_mat;

    // check if the height is not zero
    if (module->params[2].value <= 0.0001f) {
      fprintf(stderr,
              "QuasiMC - Warning: object has area = 0, as height = 0\n");
      return (-1);
    }
    // check if width is nonzero
    if (module->params[0].value + module->params[1].value <= 0.0001f) {
      fprintf(stderr, "QuasiMC - Warning: object has area = 0, as width = 0\n");
      return (-1);
    }
    // if the half width is too small, set it to a minimum value
    if (module->params[0].value <= 0.001f)
      module->params[0].value = 0.001f;
    if (module->params[1].value <= 0.001f)
      module->params[1].value = 0.001f;

    // check if the angle is not zero
    if (module->params[3].value < -90.0 || module->params[3].value > 90.0) {
      fprintf(stderr, "QuasiMC - the angle for an arbitrary triangle must "
                      "satisfy -90 <= angle <= 90.\n");
      return (-1);
    }

    ang_in_rads = module->params[3].value * M_PIf / 180.f;

    // length1 + length2 = the length of the edge perpendicular to the heading
    // vector length1 is the length of the 'left' side, and determines the first
    // point
    pt[0][Z] = module->params[0].value * cosf(-M_PIf * 0.5f - ang_in_rads);
    pt[0][Y] = module->params[0].value * sinf(-M_PIf * 0.5f - ang_in_rads);

    // length2 is the length of the 'right' side, and determines the second
    // point
    pt[1][Z] = module->params[1].value * cosf(M_PIf * 0.5f - ang_in_rads);
    pt[1][Y] = module->params[1].value * sinf(M_PIf * 0.5f - ang_in_rads);

    // height determines the third point of the trianlge
    pt[2][Y] = module->params[2].value;

    Transform3Point(pt[0], invmatrix, pars + 0);
    Transform3Point(pt[1], invmatrix, pars + 3);
    Transform3Point(pt[2], invmatrix, pars + 6);

    return (SetPrimitive(scene, TRIANGLE, pars, turtle->up, top_mat, bot_mat));
  }

  /* surface */
  if (module->symbol[0] == 'S') {
    if (module->num_params == 2) {
      surface_scale[0] = module->params[1].value;
      surface_scale[1] = module->params[1].value;
      surface_scale[2] = module->params[1].value;
    } else if (module->num_params == 3) {
      // to match cpfg/lpfg, switch the X and Z scaling factors
      surface_scale[0] = 1.0f;
      surface_scale[1] = module->params[2].value;
      surface_scale[2] = module->params[1].value;
    } else {
      // to match cpfg/lpfg, switch the X and Z scaling factors
      surface_scale[0] = module->params[3].value;
      surface_scale[1] = module->params[2].value;
      surface_scale[2] = module->params[1].value;
    }

    // this is visualized in the debug window
    // if (verbose >= 4)
    // fprintf (stderr, "scene3d - adding surface (%g, %g, %g, %g)\n",
    // module->params[0].value, surface_scale[0],
    // surface_scale[1], surface_scale[2]);

    if (module->params[0].value < 0 ||
        module->params[0].value >= scene->num_surfaces) {
      fprintf(stderr, "QuasiMC - surface id=%g does not exist\n",
              module->params[0].value);
      return (-1);
    }

    if (module->params[1].value <= 0) {
      fprintf(
          stderr,
          "QuasiMC - Warning: surface scale is less than or equal to zero\n");
      return (-1);
    }

    return (AddSurface(scene, (int)module->params[0].value, surface_scale,
                       invmatrix, turtle->up, module, material_param));
  }

  /* internode as a triangle cylinder */
  if (module->symbol[0] == 'I') {
    return (AddTriangleCylinder(scene, invmatrix, turtle->up, module));
  }

  /* unknown object */
  // fprintf (stderr, "scene3d - module must be T, P or S\n");
  return (-1);
}

/* ------------------------------------------------------------------------- */

int AddTriangle(SCENE *scene, float *vertices, int npars, float *up,
                Cmodule_type *module)
/* used for adding arbitrary polygons from cpfg - returns index of object in the
   array of primitives */
{
  int top_mat, bot_mat;

  /* check for material parameters defined in the module */
  if (module->num_params >= npars)
    top_mat = (int)module->params[npars - 1].value;
  else
    top_mat = 0;
  if (module->num_params >= npars + 1)
    bot_mat = (int)module->params[npars].value;
  else
    bot_mat = top_mat;

  return (SetPrimitive(scene, TRIANGLE, vertices, up, top_mat, bot_mat));
}

/* ------------------------------------------------------------------------- */

int AddSurface(SCENE *scene, int id, float scale[3], float *invmatrix,
               float *up, Cmodule_type *module, int material_param) {
  SURFACE *surface;
  SUBSURFACE *sub_s;
  float pars[12];
  float vertex[3];
  int i, j, k, pt, success;

  success = -1;

  surface = &scene->surfaces[id];

  for (i = 0; i < surface->num_patches; i++) {
    sub_s = &surface->subsurfaces[i];
    for (j = 0; j < sub_s->num_pts - 1; j++) {
      pt = j * sub_s->num_pts * 3;
      for (k = 0; k < sub_s->num_pts - 1; k++) {
        if (scale[0] != 1.0f || scale[1] != 1.0f || scale[2] != 1.0f) {
          vertex[0] = sub_s->points[pt + k * 3 + 0] * scale[0];
          vertex[1] = sub_s->points[pt + k * 3 + 1] * scale[1];
          vertex[2] = sub_s->points[pt + k * 3 + 2] * scale[2];
          Transform3Point(vertex, invmatrix, pars + 0);

          vertex[0] = sub_s->points[pt + (k + 1) * 3 + 0] * scale[0];
          vertex[1] = sub_s->points[pt + (k + 1) * 3 + 1] * scale[1];
          vertex[2] = sub_s->points[pt + (k + 1) * 3 + 2] * scale[2];
          Transform3Point(vertex, invmatrix, pars + 3);

          vertex[0] =
              sub_s->points[pt + (k + sub_s->num_pts + 1) * 3 + 0] * scale[0];
          vertex[1] =
              sub_s->points[pt + (k + sub_s->num_pts + 1) * 3 + 1] * scale[1];
          vertex[2] =
              sub_s->points[pt + (k + sub_s->num_pts + 1) * 3 + 2] * scale[2];
          Transform3Point(vertex, invmatrix, pars + 6);

          vertex[0] =
              sub_s->points[pt + (k + sub_s->num_pts) * 3 + 0] * scale[0];
          vertex[1] =
              sub_s->points[pt + (k + sub_s->num_pts) * 3 + 1] * scale[1];
          vertex[2] =
              sub_s->points[pt + (k + sub_s->num_pts) * 3 + 2] * scale[2];
          Transform3Point(vertex, invmatrix, pars + 9);
        } else {
          Transform3Point(&sub_s->points[pt + k * 3], invmatrix, pars + 0);
          Transform3Point(&sub_s->points[pt + (k + 1) * 3], invmatrix,
                          pars + 3);
          Transform3Point(&sub_s->points[pt + (k + sub_s->num_pts + 1) * 3],
                          invmatrix, pars + 6);
          Transform3Point(&sub_s->points[pt + (k + sub_s->num_pts) * 3],
                          invmatrix, pars + 9);
        }

        // split the patch into two triangles
        // success = SetPrimitive (scene, TRIANGLE, pars, up, 0, 1);
        success = AddTriangle(scene, pars, material_param, up, module);
        if (success == -1)
          return (-1);

        pars[3] = pars[9];
        pars[4] = pars[10];
        pars[5] = pars[11];
        // success = SetPrimitive (scene, TRIANGLE, pars, up, 0, 1);
        success = AddTriangle(scene, pars, material_param, up, module);
        if (success == -1)
          return (-1);
      }
    }
  }

  return (success);
}

/* ------------------------------------------------------------------------- */

int AddTriangleCylinder(SCENE *scene, float *invmatrix, float *up,
                        Cmodule_type *module) {
  float triVerts[9];
  float theta;
  int i, success;

  // check if at least two parameters are given: radius and height
  if (module->num_params < 2) {
    fprintf(stderr, "QuasiMC - the 'I' module requires two parameters.\n");
    return (-1);
  }

  // check if the radius is not zero
  if (module->params[0].value <= 0.0001f) {
    fprintf(stderr, "QuasiMC - Warning: object has area = 0, as radius = 0\n");
    return (-1);
  }
  // check if height is nonzero
  if (module->params[1].value <= 0.0001f) {
    fprintf(stderr, "QuasiMC - Warning: object has area = 0, as height = 0\n");
    return (-1);
  }

  theta = 0.f;
  for (int i = 0; i < numSides; i++) {
    // scale by radius
    *(ptsBot + i * 3 + X) = cosf(theta) * module->params[0].value * 0.5f;
    *(ptsBot + i * 3 + Y) = 0.f;
    *(ptsBot + i * 3 + Z) = sinf(theta) * module->params[0].value * 0.5f;
    // add height
    *(ptsTop + i * 3 + X) = *(ptsBot + i * 3 + X);
    *(ptsTop + i * 3 + Y) = *(ptsBot + i * 3 + Y) + module->params[1].value;
    *(ptsTop + i * 3 + Z) = *(ptsBot + i * 3 + Z);

    theta += 2.f * M_PIf / (float)numSides;
  }

  // add triangles representing a cylinder
  // two triangles per side
  for (i = 0; i < numSides; i++) {

    Transform3Point(ptsBot + i * 3, invmatrix, triVerts + 0);
    Transform3Point(ptsTop + i * 3, invmatrix, triVerts + 3);
    Transform3Point(ptsTop + ((numSides - 1 + i) % numSides) * 3, invmatrix,
                    triVerts + 6);
    if ((success = AddTriangle(scene, triVerts, 3, up, module)) == -1)
      return (-1);

    Transform3Point(ptsBot + i * 3, invmatrix, triVerts + 0);
    Transform3Point(ptsTop + ((numSides - 1 + i) % numSides) * 3, invmatrix,
                    triVerts + 3);
    Transform3Point(ptsBot + ((numSides - 1 + i) % numSides) * 3, invmatrix,
                    triVerts + 6);
    if ((success = AddTriangle(scene, triVerts, 3, up, module)) == -1)
      return (-1);
  }

  return (success);
}

/* ------------------------------------------------------------------------- */

void MatrixFromHeader(CTURTLE *tu, float *invmatrix) {
  float left[3];
  int i;

  MakeUnitMatrix(invmatrix);

  CrossProduct(tu->heading, tu->up, left);

  for (i = 0; i < 3; i++) {
    invmatrix[access(0, i)] = tu->up[i];
    invmatrix[access(1, i)] = tu->heading[i];
    invmatrix[access(2, i)] = left[i];
    invmatrix[access(3, i)] = tu->position[i];
  }
  return;
}

/* ------------------------------------------------------------------------- */

int SetPrimitive(SCENE *scene, int obj_type, float *data, float *up,
                 int top_mat, int bot_mat)
/* returns index of object */
{
  PRIMITIVE *prim;
  float vec1[3], vec2[3];
  int i, float_num;

  /* check if enough room for new primitive */
  CheckPrimitivesSize(scene);

  prim = &scene->primitives[scene->num_primitives];
  float_num = obj_type * 3;

  /* bound this object in the scene */
  BoundObject(scene, obj_type, data);

  for (i = 0; i < float_num; i += 3) {
    prim->data[i + X] = data[i + X];
    prim->data[i + Y] = data[i + Y];
    prim->data[i + Z] = data[i + Z];
  }

  // get the polygon/triangle normal, and calculate the area
  for (i = X; i <= Z; i++) {
    vec2[i] = data[2 * 3 + i] - data[0 * 3 + i];
    vec1[i] = data[1 * 3 + i] - data[0 * 3 + i];
  }
  CrossProduct(vec1, vec2, prim->normal);

  // the area of a paralleogram is the magnitude (sqrt of dot product of vector
  // itself) of the cross product vector, AB cross AC. In this case the
  // primitive's normal
  prim->area = sqrtf(prim->normal[0] * prim->normal[0] +
                     prim->normal[1] * prim->normal[1] +
                     prim->normal[2] * prim->normal[2]);

  // the area of a triangle is half of the parallelogram
  if (obj_type == TRIANGLE)
    prim->area *= 0.5f;

  // ensure this is not a degenerate polygon by checking if area is greater than
  // zero.
  if (prim->area <= 0.0) {
    fprintf(stderr,
            "QuasiMC - WARNING: received polygon with an area of zero.\n");
    return (-1);
  }

  // normalize and invert, if necessary, the primitive's normal
  Normalize(prim->normal);
  if (DotProduct(prim->normal, up) < 0.0) {
    prim->normal[0] *= -1.0;
    prim->normal[1] *= -1.0;
    prim->normal[2] *= -1.0;
  }

  /* find the 2d plane to project onto - plane with maximum area */
  prim->plane2d = MAX3Di(fabs(prim->normal[X]), fabs(prim->normal[Y]),
                         fabs(prim->normal[Z]));

  prim->object_type = obj_type;
  prim->ray_signature = 0;
  prim->avg_cos_angle = 0.f;
  memset(prim->absorbed_flux, 0, sizeof(float) * MAX_SPECTRUM_SAMPLES);
  memset(prim->incident_flux, 0, sizeof(float) * MAX_SPECTRUM_SAMPLES);
  memset(prim->direct_hits, 0, sizeof(float) * MAX_SPECTRUM_SAMPLES);


  if ((top_mat >= 0) && (top_mat < scene->num_materials))
    prim->material[0] = &scene->materials[top_mat];
  else
    prim->material[0] = &scene->materials[0];
  if ((bot_mat >= 0) && (bot_mat < scene->num_materials))
    prim->material[1] = &scene->materials[bot_mat];
  else
    prim->material[1] = prim->material[0];

  return (scene->num_primitives++);
}

/* ------------------------------------------------------------------------- */

void FindBoundingSphere(SCENE *scene)
/* computes the bounding sphere for the entire scene */
{
  float vec[3], radius;

  // the box must be made slightly bigger so the ray can be placed inside it
  // (+/- 0.1 to bbox) this is a bug and there should be a better way to
  // implement this. April 2012 - added check to see if bbox range is not to
  // small in x,y,z, and adjusted grid size accordingly this is to avoid the
  // case where there is a flat leaf and grid size is high but range is almost
  // zero
  scene->grid.bbox[0] -= 0.1f;
  scene->grid.bbox[3] += 0.1f;
  vec[X] = scene->grid.bbox[3] - scene->grid.bbox[0];
  // if (vec[X] <= 0.22f)
  //  scene->grid.size[X] = 1;

  scene->grid.bbox[1] -= 0.1f;
  scene->grid.bbox[4] += 0.1f;
  vec[Y] = scene->grid.bbox[4] - scene->grid.bbox[1];
  if (vec[Y] <= 0.22f)
    scene->grid.size[Y] = 1;

  scene->grid.bbox[2] -= 0.1f;
  scene->grid.bbox[5] += 0.1f;
  vec[Z] = scene->grid.bbox[5] - scene->grid.bbox[2];
  // if (vec[Z] <= 0.22f)
  //  scene->grid.size[Z] = 1;

  scene->grid.range[X] = vec[X];
  scene->grid.range[Y] = vec[Y];
  scene->grid.range[Z] = vec[Z];
  scene->grid.maxdist = 2.0f * (vec[X] + vec[Y] + vec[Z]);

  radius =
      (float)sqrt(vec[X] * vec[X] + vec[Y] * vec[Y] + vec[Z] * vec[Z]) / 2.0f;

  /* check if sphere around bounding box is smaller then the current one */
  if (radius < scene->grid.bsph_radius) {
    scene->grid.bsph_radius = radius;
    scene->grid.bsph_centre[X] =
        0.5f * (scene->grid.bbox[0] + scene->grid.bbox[3]);
    scene->grid.bsph_centre[Y] =
        0.5f * (scene->grid.bbox[1] + scene->grid.bbox[4]);
    scene->grid.bsph_centre[Z] =
        0.5f * (scene->grid.bbox[2] + scene->grid.bbox[5]);
  }

  if (verbose >= 1) {
    fprintf(stderr, "\nQuasiMC - bounding box of scene is:\n");
    fprintf(stderr, "\tfrom (%g, %g, %g) to (%g, %g, %g)\n",
            scene->grid.bbox[0], scene->grid.bbox[1], scene->grid.bbox[2],
            scene->grid.bbox[3], scene->grid.bbox[4], scene->grid.bbox[5]);

    fprintf(stderr, "QuasiMC - bounding sphere of scene is:\n");
    fprintf(stderr, "\t(%g, %g, %g) with radius %g\n",
            scene->grid.bsph_centre[X], scene->grid.bsph_centre[Y],
            scene->grid.bsph_centre[Z], scene->grid.bsph_radius);
  }
  return;
}

/* ------------------------------------------------------------------------- */

void BoundObject(SCENE *scene, int obj_type, float *data)
/* updates the size of the bounding box and the bounding sphere
   and places it into the grid */
{
  float max[3], min[3], radius, aux;
  int i;

  /* check max and min of bounding box */
  for (i = X; i <= Z; i++) {
    max[i] = MAX3D(data[i], data[i + 3], data[i + 6]);
    if (obj_type == POLYGON)
      max[i] = MAX2D(max[i], data[i + 9]);

    min[i] = MIN3D(data[i], data[i + 3], data[i + 6]);
    if (obj_type == POLYGON)
      min[i] = MIN2D(min[i], data[i + 9]);
  }

  if (scene->grid.bsph_radius == 0.0)
  // if (scene->num_primitives == 0)
  {
    for (i = X; i <= Z; i++) {
      scene->grid.bbox[i] = min[i];
      scene->grid.bbox[i + 3] = max[i];

      /* centre of sphere is first point */
      scene->grid.bsph_centre[i] = data[i];
    }
  } else {
    for (i = X; i <= Z; i++) {
      scene->grid.bbox[i] = MIN2D(min[i], scene->grid.bbox[i]);
      scene->grid.bbox[i + 3] = MAX2D(max[i], scene->grid.bbox[i + 3]);
    }
  }

  /* add points of object to bounding sphere - max is used as temp variable */
  for (i = 0; i < obj_type * 3; i += 3) {
    max[X] = data[i + X] - scene->grid.bsph_centre[X];
    max[Y] = data[i + Y] - scene->grid.bsph_centre[Y];
    max[Z] = data[i + Z] - scene->grid.bsph_centre[Z];

    aux = max[X] * max[X] + max[Y] * max[Y] + max[Z] * max[Z];

    if (aux > (scene->grid.bsph_radius * scene->grid.bsph_radius)) {
      aux = (float)sqrt(aux);
      radius = (aux - scene->grid.bsph_radius) / 2.0f;
      scene->grid.bsph_radius += radius;

      scene->grid.bsph_centre[X] += max[X] / aux * radius;
      scene->grid.bsph_centre[Y] += max[Y] / aux * radius;
      scene->grid.bsph_centre[Z] += max[Z] / aux * radius;
    }
  }

  return;
}

/* ------------------------------------------------------------------------- */

void BoundQueryObject(SCENE *scene, QUERY *query)
/* updates the size of the bounding box and the bounding sphere,
 * but the query object is not added into the grid.  this way it
 * is not consider in the intersection tests */
{
  float data[12];
  float max[3], min[3], radius, aux;
  int i, obj_type;

  // construct the four vertices of parallelogram from query
  data[0] = query->pos[0];
  data[1] = query->pos[1];
  data[2] = query->pos[2];
  data[3] = query->pos[0] + query->edge1[0];
  data[4] = query->pos[1] + query->edge1[1];
  data[5] = query->pos[2] + query->edge1[2];
  data[6] = query->pos[0] + query->edge1[0] + query->edge2[0];
  data[7] = query->pos[1] + query->edge1[1] + query->edge2[1];
  data[8] = query->pos[2] + query->edge1[2] + query->edge2[2];
  data[9] = query->pos[0] + query->edge2[0];
  data[10] = query->pos[1] + query->edge2[1];
  data[11] = query->pos[2] + query->edge2[2];

  obj_type = POLYGON;

  // check max and min of bounding box - object type must be a polygon!
  for (i = X; i <= Z; i++) {
    max[i] = MAX3D(data[i], data[i + 3], data[i + 6]);
    if (obj_type == POLYGON)
      max[i] = MAX2D(max[i], data[i + 9]);

    min[i] = MIN3D(data[i], data[i + 3], data[i + 6]);
    if (obj_type == POLYGON)
      min[i] = MIN2D(min[i], data[i + 9]);
  }

  // check if this is the first object to be added
  if (scene->grid.bsph_radius == 0.0)
  // if (scene->num_primitives == 0)
  {
    for (i = X; i <= Z; i++) {
      scene->grid.bbox[i] = min[i];
      scene->grid.bbox[i + 3] = max[i];

      // centre of sphere is first point
      scene->grid.bsph_centre[i] = data[i];
    }
    // scene->grid.bsph_radius = 0.0;
  } else {
    for (i = X; i <= Z; i++) {
      scene->grid.bbox[i] = MIN2D(min[i], scene->grid.bbox[i]);
      scene->grid.bbox[i + 3] = MAX2D(max[i], scene->grid.bbox[i + 3]);
    }
  }

  // add points of object to bounding sphere - max is used as temp variable
  for (i = 0; i < obj_type * 3; i += 3) {
    max[X] = data[i + X] - scene->grid.bsph_centre[X];
    max[Y] = data[i + Y] - scene->grid.bsph_centre[Y];
    max[Z] = data[i + Z] - scene->grid.bsph_centre[Z];

    aux = max[X] * max[X] + max[Y] * max[Y] + max[Z] * max[Z];

    if (aux > (scene->grid.bsph_radius * scene->grid.bsph_radius)) {
      aux = (float)sqrt(aux);
      radius = (aux - scene->grid.bsph_radius) / 2.0f;
      scene->grid.bsph_radius += radius;

      scene->grid.bsph_centre[X] += max[X] / aux * radius;
      scene->grid.bsph_centre[Y] += max[Y] / aux * radius;
      scene->grid.bsph_centre[Z] += max[Z] / aux * radius;
    }
  }

  return;
}

/* ------------------------------------------------------------------------- */

void FillGrid(SCENE *scene)
/* fill the cells of the grid with all objects in the scene */
{
  PRIMITIVE *prim;
  CELL *cell;
  float boxcenter[3]; // next three for computing trianlge-box intersection
  float boxhalfsize[3];
  float triverts[3][3];
  float bbox[6];
  int range[3][2];
  int i, x, y, z;

  if (scene->grid.cells == NULL)
    return;

  if (verbose >= 1)
    fprintf(stderr, "QuasiMC - grid size is %dx%dx%d\n", scene->grid.size[X],
            scene->grid.size[Y], scene->grid.size[Z]);

  /* clear primitive list for each cell */
  for (i = 0; i < scene->grid.num_cells; i++)
    scene->grid.cells[i].num_primitives = 0;

  scene->grid.cell_size[X] =
      (float)scene->grid.range[X] / (float)scene->grid.size[X];
  scene->grid.cell_size[Y] =
      (float)scene->grid.range[Y] / (float)scene->grid.size[Y];
  scene->grid.cell_size[Z] =
      (float)scene->grid.range[Z] / (float)scene->grid.size[Z];

  /* put triangles and rhombuses into grid */
  for (i = 0; i < scene->num_primitives; i++) {
    prim = &scene->primitives[i];

    /* determine bounding box of triangle/rhombus */
    for (x = X; x <= Z; x++) {
      /* set (x,y,z) of top-right corner */
      bbox[x + 3] = MAX3D(prim->data[x], prim->data[x + 3], prim->data[x + 6]);
      if (prim->object_type == POLYGON)
        bbox[x + 3] = MAX2D(bbox[x + 3], prim->data[x + 9]);

      /* set (x,y,z) of bottom-left corner */
      bbox[x] = MIN3D(prim->data[x], prim->data[x + 3], prim->data[x + 6]);
      if (prim->object_type == POLYGON)
        bbox[x] = MIN2D(bbox[x], prim->data[x + 9]);
    }

    /* determine range of grid cells the trianlge/rhombus occupies */
    for (x = X; x <= Z; x++) {
      // if the grid range is not zero, calculate occupied grid cells
      // grid.range is the distance from bottom-left to top-right corner
      if (scene->grid.range[x] > 0.0) {
        // from lowest
        range[x][0] = (int)floor((bbox[x] - 0.0001 - scene->grid.bbox[x]) /
                                 scene->grid.cell_size[x]);
        range[x][0] = MAX2D(0, range[x][0]);
        // to highest
        range[x][1] = (int)ceil((bbox[x + 3] + 0.0001 - scene->grid.bbox[x]) /
                                scene->grid.cell_size[x]);
        range[x][1] = MIN2D(scene->grid.size[x], range[x][1]);
      } else {
        // else, take all the cells for this dimension
        range[x][0] = 0;
        range[x][1] = scene->grid.size[x];
      }
    }

    //	fprintf (stderr, "RANGE: \n");
    //	fprintf (stderr, "\tX: %d,%d\n", range[X][0], range[X][1]);
    //	fprintf (stderr, "\tY: %d,%d\n", range[Y][0], range[Y][1]);
    //	fprintf (stderr, "\tZ: %d,%d\n", range[Z][0], range[Z][1]);

    boxhalfsize[X] = 0.50001f * scene->grid.cell_size[X];
    boxhalfsize[Y] = 0.50001f * scene->grid.cell_size[Y];
    boxhalfsize[Z] = 0.50001f * scene->grid.cell_size[Z];

    // iterate through the cells that the triangle/rhombus occupies
    // and add the trianlge's/rhombus' index into the cell's list
    for (x = range[X][0]; x < range[X][1]; x++)
      for (y = range[Y][0]; y < range[Y][1]; y++)
        for (z = range[Z][0]; z < range[Z][1]; z++) {
          cell =
              &scene->grid.cells[x * scene->grid.size[Y] * scene->grid.size[Z] +
                                 y * scene->grid.size[Z] + z];

          // check if the triangle/rhombus actually occupies the cell,
          // otherwise the bounding box of the triangle/rhombus is occupying the
          // cell calculate center of this box
          boxcenter[X] = x * scene->grid.cell_size[X] + boxhalfsize[X] +
                         scene->grid.bbox[X];
          boxcenter[Y] = y * scene->grid.cell_size[Y] + boxhalfsize[Y] +
                         scene->grid.bbox[Y];
          boxcenter[Z] = z * scene->grid.cell_size[Z] + boxhalfsize[Z] +
                         scene->grid.bbox[Z];

          triverts[0][X] = prim->data[X];
          triverts[0][Y] = prim->data[Y];
          triverts[0][Z] = prim->data[Z];
          triverts[1][X] = prim->data[X + 3];
          triverts[1][Y] = prim->data[Y + 3];
          triverts[1][Z] = prim->data[Z + 3];
          triverts[2][X] = prim->data[X + 6];
          triverts[2][Y] = prim->data[Y + 6];
          triverts[2][Z] = prim->data[Z + 6];

          // fprintf(stderr, "boxcentre: %g %g %g\n", boxcenter[X],
          // boxcenter[Y], boxcenter[Z]); fprintf(stderr, "boxhalfsize: %g %g
          // %g\n", boxhalfsize[X], boxhalfsize[Y], boxhalfsize[Z]);
          // fprintf(stderr, "triverts: %g %g %g\n", triverts[0][X],
          // triverts[0][Y], triverts[0][Z]); fprintf(stderr, "triverts: %g %g
          // %g\n", triverts[1][X], triverts[1][Y], triverts[1][Z]);
          // fprintf(stderr, "triverts: %g %g %g\n", triverts[2][X],
          // triverts[2][Y], triverts[2][Z]);

          // use Moller's triangle-box intersection function.
          if (triBoxOverlap(boxcenter, boxhalfsize, triverts))
            AddToCell(cell, i);
          else if (prim->object_type == POLYGON) {
            // if this is a rhombus, test the other side of it
            triverts[0][X] = prim->data[X + 0];
            triverts[0][Y] = prim->data[Y + 0];
            triverts[0][Z] = prim->data[Z + 0];
            triverts[1][X] = prim->data[X + 9];
            triverts[1][Y] = prim->data[Y + 9];
            triverts[1][Z] = prim->data[Z + 9];
            triverts[2][X] = prim->data[X + 6];
            triverts[2][Y] = prim->data[Y + 6];
            triverts[2][Z] = prim->data[Z + 6];

            if (triBoxOverlap(boxcenter, boxhalfsize, triverts))
              AddToCell(cell, i);
          }
        }
  }

  if (verbose >= 1) {
    // reuse local variable
    bbox[0] = 0.0;
    bbox[1] = 0.0;
    for (i = 0; i < scene->grid.num_cells; i++) {
      bbox[0] += scene->grid.cells[i].num_primitives;
      bbox[1] += scene->grid.cells[i].num_primitives == 0 ? 1.0f : 0.0f;
    }
    bbox[0] /= (float)scene->grid.num_cells;
    fprintf(stderr, "QuasiMC - average number of primitives per voxel: %g\n",
            bbox[0]);
    fprintf(stderr, "QuasiMC - number of empty voxels: %g\n", bbox[1]);
  }

  return;
}

/* ------------------------------------------------------------------------- */

void AddToCell(CELL *cell, int prim_index)
/* add the primitive's index into the cell */
{
  if (cell->list == NULL) {
    if ((cell->list = (int *)malloc(sizeof(int) * cell->max_primitives)) ==
        NULL) {
      fprintf(stderr, "scene3d - cannot allocate memory for cell list\n");
      return;
    }
  } else if (cell->num_primitives == cell->max_primitives) {
    if (verbose >= 3)
      fprintf(stderr, "scene3d - reallocating memory for cell list\n");
    cell->max_primitives *= 2;
    if ((cell->list = (int *)realloc(
             cell->list, sizeof(int) * cell->max_primitives)) == NULL) {
      fprintf(stderr, "scene3d - cannot reallocate memory for cell list\n");
      return;
    }
  }

  cell->list[cell->num_primitives] = prim_index;
  cell->num_primitives++;
  return;
}

/* ------------------------------------------------------------------------- */

float IsIntersection(SCENE *scene, RAY *ray, int index, float mindist,
                     float *normal, PRIMITIVE **intersected)
/* checks if there is an intersection of the ray and primitive at index */
{
  PRIMITIVE *prim;
  float pt2d[2], vert2d[4][2];
  float t, denom;
  int i, j;

  if ((index < 0) || (index >= scene->num_primitives)) {
    fprintf(stderr, "scene3d - no primitive exists with index %d\n", index);
    return (-1.0);
  }

  prim = &scene->primitives[index];

  if (prim->ray_signature == ray->signature)
    return (-1);
  else
    prim->ray_signature = ray->signature;

  *intersected = prim;
  t = -1.0;

  /* solve t = (-dp - n.o) / (n.d) */
  /* first calculate the denominator - if it is 0 then no intersection */
  denom = prim->normal[X] * ray->dir[X] + prim->normal[Y] * ray->dir[Y] +
          prim->normal[Z] * ray->dir[Z];
  if ((denom > -EPSILON) && (denom < EPSILON))
    return (-1.0);

  /* dp is the constant of the polygon's supporting plane */
  /* compute t */
  t = ((prim->normal[X] * prim->data[X] + prim->normal[Y] * prim->data[Y] +
        prim->normal[Z] * prim->data[Z]) -
       (prim->normal[X] * ray->pt[X] + prim->normal[Y] * ray->pt[Y] +
        prim->normal[Z] * ray->pt[Z])) /
      denom;

  /* if t is too close or too far then no need to test further */
  if ((t < EPSILON) || (t > mindist))
    return (-1.0);

  /* project the triangle/polygon onto a 2d plane */
  j = 0;
  if (prim->plane2d == X) {
    /* the intersection point on the plane X = 0 */
    pt2d[0] = ray->pt[Y] + t * ray->dir[Y];
    pt2d[1] = ray->pt[Z] + t * ray->dir[Z];

    /* vertices of the 2d projection */
    for (i = 0; i < prim->object_type; i++) {
      vert2d[i][0] = prim->data[j + Y];
      vert2d[i][1] = prim->data[j + Z];
      j += 3;
    }
  } else if (prim->plane2d == Y) {
    /* the intersection point on the plane Y = 0 */
    pt2d[0] = ray->pt[X] + t * ray->dir[X];
    pt2d[1] = ray->pt[Z] + t * ray->dir[Z];

    /* vertices of the 2d projection */
    for (i = 0; i < prim->object_type; i++) {
      vert2d[i][0] = prim->data[j + X];
      vert2d[i][1] = prim->data[j + Z];
      j += 3;
    }
  } else {
    /* the intersection point on the plane Z = 0 */
    pt2d[0] = ray->pt[X] + t * ray->dir[X];
    pt2d[1] = ray->pt[Y] + t * ray->dir[Y];

    /* vertices of the 2d projection */
    for (i = 0; i < prim->object_type; i++) {
      vert2d[i][0] = prim->data[j + X];
      vert2d[i][1] = prim->data[j + Y];
      j += 3;
    }
  }

  /* check if the 2d point is in the 2d plane */
  if (prim->object_type == POLYGON) {
    if (!In2dPolygon(pt2d, vert2d))
      return (-1.0);
  } else if (prim->object_type == TRIANGLE) {
    if (!In2dTriangle(pt2d, vert2d))
      return (-1.0);
  } else
    return (-1.0);

  /* return the normal of this triangle/polygon */
  normal[X] = prim->normal[X];
  normal[Y] = prim->normal[Y];
  normal[Z] = prim->normal[Z];

  if (verbose >= 3) {
    fprintf(stderr, "scene3d - intersection found (index = %d).\n", index);
    fprintf(stderr, "scene3d - returned t = %g\n", t);
  }

  return (t);
}

/* ------------------------------------------------------------------------- */

int In2dPolygon(float pt[2], float vert[4][2])
/* checks if pt is inside the 2d polygon defined by verts */
{
  int signA, signB;

  signA =
      ((vert[1][Y] - vert[0][Y]) * pt[X] + (vert[0][X] - vert[1][X]) * pt[Y] +
       (vert[1][X] * vert[0][Y] - vert[1][Y] * vert[0][X])) >= 0
          ? 1
          : -1;

  signB =
      ((vert[2][Y] - vert[1][Y]) * pt[X] + (vert[1][X] - vert[2][X]) * pt[Y] +
       (vert[2][X] * vert[1][Y] - vert[2][Y] * vert[1][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return (0);

  signA =
      ((vert[3][Y] - vert[2][Y]) * pt[X] + (vert[2][X] - vert[3][X]) * pt[Y] +
       (vert[3][X] * vert[2][Y] - vert[3][Y] * vert[2][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return (0);

  signB =
      ((vert[0][Y] - vert[3][Y]) * pt[X] + (vert[3][X] - vert[0][X]) * pt[Y] +
       (vert[0][X] * vert[3][Y] - vert[0][Y] * vert[3][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return (0);

  return (1);
}

/* ------------------------------------------------------------------------- */

int In2dTriangle(float pt[2], float vert[4][2])
/* checks if pt is inside the triangle defined by the first three vertices  */
{
  int signA, signB;

  signA =
      ((vert[1][Y] - vert[0][Y]) * pt[X] + (vert[0][X] - vert[1][X]) * pt[Y] +
       (vert[1][X] * vert[0][Y] - vert[1][Y] * vert[0][X])) >= 0
          ? 1
          : -1;

  signB =
      ((vert[2][Y] - vert[1][Y]) * pt[X] + (vert[1][X] - vert[2][X]) * pt[Y] +
       (vert[2][X] * vert[1][Y] - vert[2][Y] * vert[1][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return (0);

  signA =
      ((vert[0][Y] - vert[2][Y]) * pt[X] + (vert[2][X] - vert[0][X]) * pt[Y] +
       (vert[0][X] * vert[2][Y] - vert[0][Y] * vert[2][X])) >= 0
          ? 1
          : -1;

  if (signA != signB)
    return (0);

  return (1);
}

/* ------------------------------------------------------------------------- */

int FindBoxIntersection(SCENE *scene, RAY *ray)
/* returns true if the ray intersects the bounding box of the scene */
{
  float dist[3];
  int i, index;

  /* find distance to the closest X,Y,Z face */
  for (i = X; i <= Z; i++) {
    if (ray->dir[i] < -1e-10)
      dist[i] = (scene->grid.bbox[i] + scene->grid.range[i] -
                 (ray->pt[i] -= scene->grid.maxdist * ray->dir[i])) /
                ray->dir[i];
    else if (ray->dir[i] > 1e-10)
      dist[i] = (scene->grid.bbox[i] -
                 (ray->pt[i] -= scene->grid.maxdist * ray->dir[i])) /
                ray->dir[i];
    else
      dist[i] = 0.0;
  }

  /* pick the biggest of the three distances */
  index = MAX3Di(dist[X], dist[Y], dist[Z]);

  if (dist[index] < 0.0)
    return (FALSE);

  /* place pt into the grid for the intersection test */
  ray->pt[X] += ray->dir[X] * dist[index] * 1.00001f;
  ray->pt[Y] += ray->dir[Y] * dist[index] * 1.00001f;
  ray->pt[Z] += ray->dir[Z] * dist[index] * 1.00001f;

  /* check if pt is in the box */
  if (index != X) {
    if (ray->pt[X] <= scene->grid.bbox[X] ||
        ray->pt[X] >= scene->grid.bbox[X + 3])
      return (FALSE);
  }
  if (index != Y) {
    if (ray->pt[Y] <= scene->grid.bbox[Y] ||
        ray->pt[Y] >= scene->grid.bbox[Y + 3])
      return (FALSE);
  }
  if (index != Z) {
    if (ray->pt[Z] <= scene->grid.bbox[Z] ||
        ray->pt[Z] >= scene->grid.bbox[Z + 3])
      return (FALSE);
  }

  return (TRUE);
}

/* ------------------------------------------------------------------------- */

CELL *FindFirstCell(SCENE *scene, RAY *ray)
/* returns the initial cell that was pierced by the ray */
{
  GRID *grid;
  int i;

  grid = &scene->grid;

  /* find the first cell that is intersecting with the ray. */
  grid->cell[X] = (int)((ray->pt[X] - grid->bbox[X]) / grid->cell_size[X]);
  grid->cell[Y] = (int)((ray->pt[Y] - grid->bbox[Y]) / grid->cell_size[Y]);
  grid->cell[Z] = (int)((ray->pt[Z] - grid->bbox[Z]) / grid->cell_size[Z]);

  if ((grid->cell[X] < 0) || (grid->cell[Y] < 0) || (grid->cell[Z] < 0))
    return (NULL);

  if ((grid->cell[X] >= grid->size[X]) || (grid->cell[Y] >= grid->size[Y]) ||
      (grid->cell[Z] >= grid->size[Z]))
    return (NULL);

  for (i = X; i <= Z; i++)
    if (ray->dir[i] < -EPSILON) {
      grid->cell_step[i] = -1;
      grid->smallest[i] =
          (grid->bbox[i] + (float)grid->cell[i] * grid->cell_size[i] -
           ray->pt[i]) /
          ray->dir[i];
      grid->smallest_step[i] = -grid->cell_size[i] / ray->dir[i];
    } else if (ray->dir[i] > EPSILON) {
      grid->cell_step[i] = 1;
      grid->smallest[i] =
          (grid->bbox[i] + ((float)grid->cell[i] + 1.0f) * grid->cell_size[i] -
           ray->pt[i]) /
          ray->dir[i];
      grid->smallest_step[i] = grid->cell_size[i] / ray->dir[i];
    } else {
      grid->cell_step[i] = 0;
      grid->smallest[i] = 1e30f;
      grid->smallest_step[i] = 0.0;
    }

  return (&grid->cells[grid->cell[X] * grid->size[Y] * grid->size[Z] +
                       grid->cell[Y] * grid->size[Z] + grid->cell[Z]]);
}

/* ------------------------------------------------------------------------- */

CELL *FindNextCell(SCENE *scene, __attribute__((unused))RAY *ray)
/* returns the next cell pierced by the ray after the first one */
{
  GRID *grid;
  int index;

  grid = &scene->grid;

  index = MIN3Di(grid->smallest[X], grid->smallest[Y], grid->smallest[Z]);

  if (grid->cell_step[index] == 0)
    return (NULL);

  grid->smallest[index] += grid->smallest_step[index];
  grid->cell[index] += grid->cell_step[index];

  if ((grid->cell[index] < 0) || (grid->cell[index] >= grid->size[index]))
    return (NULL);

  return (&grid->cells[grid->cell[X] * grid->size[Y] * grid->size[Z] +
                       grid->cell[Y] * grid->size[Z] + grid->cell[Z]]);
}

/* ------------------------------------------------------------------------- */

void AddMaterial(SCENE *scene, MATERIAL *material)
/* adds a material to the scene - indexed by triangles/polygons */
{
  MATERIAL *mat;
  int i;

  /* check if there is space for new material */
  if (scene->num_materials >= scene->max_materials) {
    scene->max_materials *= 2;
    if ((scene->materials = (MATERIAL *)realloc(
             scene->materials, scene->max_materials * sizeof(MATERIAL))) ==
        NULL) {
      fprintf(stderr, "scene3d - cannot reallocate memory for materials\n");
      return;
    }
    if (verbose >= 2)
      fprintf(stderr, "scene3d - reallocated memory for materials\n");
  }

  mat = &scene->materials[scene->num_materials];

  for (i = 0; i < MAX_SPECTRUM_SAMPLES; i++) {
    mat->reflectance[i] = material->reflectance[i];
    mat->spec_power[i] = material->spec_power[i];
    mat->transmittance[i] = material->transmittance[i];
    mat->trans_power[i] = material->trans_power[i];
    mat->Nt[i] = material->Nt[i];
  }
  scene->num_materials++;
  return;
}

/* ------------------------------------------------------------------------- */

void ChangeMaterial(SCENE *scene, MATERIAL *material, int index)
/* change the material properties at index - index */
{
  MATERIAL *mat;
  int i;

  if ((index < 0) || (index > scene->num_materials)) {
    fprintf(stderr,
            "scene3d - cannot change material properties "
            "for index %d\n",
            index);
    return;
  }

  mat = &scene->materials[index];

  for (i = 0; i < MAX_SPECTRUM_SAMPLES; i++) {
    mat->reflectance[i] = material->reflectance[i];
    mat->spec_power[i] = material->spec_power[i];
    mat->transmittance[i] = material->transmittance[i];
    mat->trans_power[i] = material->trans_power[i];
    mat->Nt[i] = material->Nt[i];
  }
  return;
}

/* ------------------------------------------------------------------------- */

void AddLight(SCENE *scene, LIGHT *light)
/* adds a directional light source to the scene */
{
  LIGHT *light_ptr;
  float vec[3];

  // check if there is space for new light source
  if (scene->num_lights >= scene->max_lights) {
    scene->max_lights *= 2;
    if ((scene->lights = (LIGHT *)realloc(
             scene->lights, scene->max_lights * sizeof(LIGHT))) == NULL) {
      fprintf(stderr, "scene3d - cannot reallocate memory for light sources\n");
      return;
    }
    if (verbose >= 2)
      fprintf(stderr, "scene3d - reallocated memory for light sources\n");
  }

  // sum up the weights of all light sources
  scene->sum_light_weights += light->weight;

  // save maximum for visualization
  if (light->weight > scene->max_light_weight)
    scene->max_light_weight = light->weight;

  // add this light into the list of light sources
  light_ptr = &scene->lights[scene->num_lights];

  light_ptr->weight = light->weight;

  light_ptr->dir[X] = light->dir[X];
  light_ptr->dir[Y] = light->dir[Y];
  light_ptr->dir[Z] = light->dir[Z];
  Normalize(light_ptr->dir);

  /* form an orthogonal basis */
  vec[0] = 1.0;
  vec[1] = 0.0;
  vec[2] = 0.0;
  CrossProduct(light_ptr->dir, vec, light_ptr->u);
  if (DotProduct(light_ptr->u, light_ptr->u) < 0.0001) {
    vec[0] = 0.0;
    vec[1] = 1.0;
    vec[2] = 0.0;
    CrossProduct(light_ptr->dir, vec, light_ptr->u);
  }
  Normalize(light_ptr->u);
  CrossProduct(light_ptr->dir, light_ptr->u, light_ptr->v);

  scene->num_lights++;

  return;
}

/* ------------------------------------------------------------------------- */

LIGHT *RandomLight(SCENE *scene)
/* returns a random light source */
{
  float random; //, sum_weights;
  int i;

  if (scene->num_lights == 1)
    return (&scene->lights[0]);

  
  i = 0;
  random = scene->sum_light_weights * RandU01(RANDQMC_START);
  while ((random -= scene->lights[i].weight) > 0)
    if (++i == scene->num_lights - 1)
      break;

  return (&scene->lights[i]);
}
/* ------------------------------------------------------------------------- */
int SetLightSourceFile(SCENE *scene, char *filename) {
  static char input_line[128];
  float p1, p2, p3, p4; // parameters from file: can have 3 or 4
  LIGHT light;
  FILE *lightfile;
  char *token;
  int line_num;

  if ((lightfile = fopen(filename, "r")) == NULL)
    return (0);

  line_num = 0;
  while (!feof(lightfile)) {
    ++line_num;

    if (fgets(input_line, sizeof(input_line), lightfile) == NULL)
      break;

    // read first input for light source: altitude or x
    if ((token = strtok(input_line, ",; \t")) == NULL) {
      fprintf(stderr,
              "QuasiMC - (line: %d) first parameter missing for light source "
              "direction\n",
              line_num);
      continue;
    }
    p1 = (float)atof(token);

    // read second input for light source: azimuth or y
    if ((token = strtok(NULL, ",; \t")) == NULL) {
      fprintf(stderr,
              "QuasiMC - (line: %d) second parameter missing for light source "
              "direction\n",
              line_num);
      continue;
    }
    p2 = (float)atof(token);

    // read third input for light source: weight or z
    if ((token = strtok(NULL, ",; \t")) == NULL) {
      fprintf(stderr,
              "QuasiMC - (line: %d) third parameter missing for light source "
              "direction\n",
              line_num);
      continue;
    }
    p3 = (float)atof(token);

    // read last input for light source: nothing or weight
    p4 = -1e11;
    if ((token = strtok(NULL, ",; \t")) != NULL) {
      p4 = (float)atof(token);
    }

    if (p4 < 0) // (fscanf(lightfile, "%lf %lf %lf", &altitude, &azimuth,
                // &weight) == 3)
    {
      /* convert to cartesian from spherical coordiantes. y has to be negative.
       * x and z are fine */
      light.dir[X] = (float)(sin(p2) * sin(p1));
      if (fabs(light.dir[X]) < 1e-5)
        light.dir[X] = 0.0;
      light.dir[Y] = (float)(-cos(p1));
      if (fabs(light.dir[Y]) < 1e-5)
        light.dir[Y] = 0.0;
      light.dir[Z] = (float)(cos(p2) * sin(p1));
      if (fabs(light.dir[Z]) < 1e-5)
        light.dir[Z] = 0.0;

      light.weight = (float)p3;

      AddLight(scene, &light);
    } else // if (fscanf(lightfile, "%lf %lf %lf %lf", &x, &y, &z, &weight) ==
           // 4)
    {
      light.dir[X] = (float)p1;
      if (fabs(light.dir[X]) < 1e-5)
        light.dir[X] = 0.0;
      light.dir[Y] = (float)p2;
      if (fabs(light.dir[Y]) < 1e-5)
        light.dir[Y] = 0.0;
      light.dir[Z] = (float)p3;
      if (fabs(light.dir[Z]) < 1e-5)
        light.dir[Z] = 0.0;

      light.weight = (float)p4;

      AddLight(scene, &light);
    }
  }
  fclose(lightfile);
  return (1);
}
/* ------------------------------------------------------------------------- */

// FOLLOWING CODE IS PUBLIC DOMAIN!!!
// see http://www.cs.lth.se/home/Tomas_Akenine_Moller/code/
// used here without modification.
/********************************************************/

/* AABB-triangle overlap test code                      */

/* by Tomas Akenine-Möller                              */

/* Function: int triBoxOverlap(float boxcenter[3],      */

/*          float boxhalfsize[3],float triverts[3][3]); */

/* History:                                             */

/*   2001-03-05: released the code in its first version */

/*   2001-06-18: changed the order of the tests, faster */

/*                                                      */

/* Acknowledgement: Many thanks to Pierre Terdiman for  */

/* suggestions and discussions on how to optimize code. */

/* Thanks to David Hunt for finding a ">="-bug!         */

/********************************************************/

#define CROSS(dest, v1, v2)                                                    \
  dest[0] = v1[1] * v2[2] - v1[2] * v2[1];                                     \
  dest[1] = v1[2] * v2[0] - v1[0] * v2[2];                                     \
  dest[2] = v1[0] * v2[1] - v1[1] * v2[0];

#define DOT(v1, v2) (v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2])

#define SUB(dest, v1, v2)                                                      \
  dest[0] = v1[0] - v2[0];                                                     \
  dest[1] = v1[1] - v2[1];                                                     \
  dest[2] = v1[2] - v2[2];

#define FINDMINMAX(x0, x1, x2, min, max)                                       \
  min = max = x0;                                                              \
  if (x1 < min)                                                                \
    min = x1;                                                                  \
  if (x1 > max)                                                                \
    max = x1;                                                                  \
  if (x2 < min)                                                                \
    min = x2;                                                                  \
  if (x2 > max)                                                                \
    max = x2;

int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3]) {
  int q;
  float vmin[3], vmax[3], v;
  for (q = X; q <= Z; q++) {
    v = vert[q];
    if (normal[q] > 0.0f) {
      vmin[q] = -maxbox[q] - v; // -NJMP-
      vmax[q] = maxbox[q] - v;  // -NJMP-
    } else {
      vmin[q] = maxbox[q] - v;  // -NJMP-
      vmax[q] = -maxbox[q] - v; // -NJMP-
    }
  }

  if (DOT(normal, vmin) > 0.0f)
    return 0; // -NJMP-
  if (DOT(normal, vmax) >= 0.0f)
    return 1; // -NJMP-

  return 0;
}

/*======================== X-tests ========================*/

#define AXISTEST_X01(a, b, fa, fb)                                             \
  p0 = a * v0[Y] - b * v0[Z];                                                  \
  p2 = a * v2[Y] - b * v2[Z];                                                  \
  if (p0 < p2) {                                                               \
    min = p0;                                                                  \
    max = p2;                                                                  \
  } else {                                                                     \
    min = p2;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

#define AXISTEST_X2(a, b, fa, fb)                                              \
  p0 = a * v0[Y] - b * v0[Z];                                                  \
  p1 = a * v1[Y] - b * v1[Z];                                                  \
  if (p0 < p1) {                                                               \
    min = p0;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

/*======================== Y-tests ========================*/

#define AXISTEST_Y02(a, b, fa, fb)                                             \
  p0 = -a * v0[X] + b * v0[Z];                                                 \
  p2 = -a * v2[X] + b * v2[Z];                                                 \
  if (p0 < p2) {                                                               \
    min = p0;                                                                  \
    max = p2;                                                                  \
  } else {                                                                     \
    min = p2;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

#define AXISTEST_Y1(a, b, fa, fb)                                              \
  p0 = -a * v0[X] + b * v0[Z];                                                 \
  p1 = -a * v1[X] + b * v1[Z];                                                 \
  if (p0 < p1) {                                                               \
    min = p0;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

/*======================== Z-tests ========================*/

#define AXISTEST_Z12(a, b, fa, fb)                                             \
  p1 = a * v1[X] - b * v1[Y];                                                  \
  p2 = a * v2[X] - b * v2[Y];                                                  \
  if (p2 < p1) {                                                               \
    min = p2;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p2;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

#define AXISTEST_Z0(a, b, fa, fb)                                              \
  p0 = a * v0[X] - b * v0[Y];                                                  \
  p1 = a * v1[X] - b * v1[Y];                                                  \
  if (p0 < p1) {                                                               \
    min = p0;                                                                  \
    max = p1;                                                                  \
  } else {                                                                     \
    min = p1;                                                                  \
    max = p0;                                                                  \
  }                                                                            \
  rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];                             \
  if (min > rad || max < -rad)                                                 \
    return 0;

int triBoxOverlap(float boxcenter[3], float boxhalfsize[3],
                  float triverts[3][3]) {
  /*    use separating axis theorem to test overlap between triangle and box */
  /*    need to test for overlap in these directions: */
  /*    1) the {x,y,z}-directions (actually, since we use the AABB of the
   * triangle */
  /*       we do not even need to test these) */
  /*    2) normal of the triangle */
  /*    3) crossproduct(edge from tri, {x,y,z}-directin) */
  /*       this gives 3x3=9 more tests */
  float v0[3], v1[3], v2[3];
  //   float axis[3];
  float min, max, p0, p1, p2, rad, fex, fey,
      fez; // -NJMP- "d" local variable removed
  float normal[3], e0[3], e1[3], e2[3];

  /* This is the fastest branch on Sun */
  /* move everything so that the boxcenter is in (0,0,0) */
  SUB(v0, triverts[0], boxcenter);
  SUB(v1, triverts[1], boxcenter);
  SUB(v2, triverts[2], boxcenter);

  /* compute triangle edges */
  SUB(e0, v1, v0); /* tri edge 0 */
  SUB(e1, v2, v1); /* tri edge 1 */
  SUB(e2, v0, v2); /* tri edge 2 */

  /* Bullet 3:  */
  /*  test the 9 tests first (this was faster) */
  fex = fabsf(e0[X]);
  fey = fabsf(e0[Y]);
  fez = fabsf(e0[Z]);
  AXISTEST_X01(e0[Z], e0[Y], fez, fey);
  AXISTEST_Y02(e0[Z], e0[X], fez, fex);
  AXISTEST_Z12(e0[Y], e0[X], fey, fex);

  fex = fabsf(e1[X]);
  fey = fabsf(e1[Y]);
  fez = fabsf(e1[Z]);
  AXISTEST_X01(e1[Z], e1[Y], fez, fey);
  AXISTEST_Y02(e1[Z], e1[X], fez, fex);
  AXISTEST_Z0(e1[Y], e1[X], fey, fex);

  fex = fabsf(e2[X]);
  fey = fabsf(e2[Y]);
  fez = fabsf(e2[Z]);
  AXISTEST_X2(e2[Z], e2[Y], fez, fey);
  AXISTEST_Y1(e2[Z], e2[X], fez, fex);
  AXISTEST_Z12(e2[Y], e2[X], fey, fex);

  /* Bullet 1: */
  /*  first test overlap in the {x,y,z}-directions */
  /*  find min, max of the triangle each direction, and test for overlap in */
  /*  that direction -- this is equivalent to testing a minimal AABB around */
  /*  the triangle against the AABB */

  /* test in X-direction */
  FINDMINMAX(v0[X], v1[X], v2[X], min, max);
  if (min > boxhalfsize[X] || max < -boxhalfsize[X])
    return 0;

  /* test in Y-direction */
  FINDMINMAX(v0[Y], v1[Y], v2[Y], min, max);
  if (min > boxhalfsize[Y] || max < -boxhalfsize[Y])
    return 0;

  /* test in Z-direction */
  FINDMINMAX(v0[Z], v1[Z], v2[Z], min, max);
  if (min > boxhalfsize[Z] || max < -boxhalfsize[Z])
    return 0;

  /* Bullet 2: */
  /*  test if the box intersects the plane of the triangle */
  /*  compute plane equation of triangle: normal*x+d=0 */
  CROSS(normal, e0, e1);

  if (!planeBoxOverlap(normal, v0, boxhalfsize))
    return 0; // -NJMP-

  return 1; /* box and triangle overlaps */
}
