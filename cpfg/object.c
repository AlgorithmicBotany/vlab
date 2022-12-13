/***********************************************************************/
/*                           object.c               		       */
/*                                                                     */
/* Outputs l-system geometry in the Wavefront object format. This      */
/* allows l-system models to be imported into applications such as     */
/* Maya.                                                               */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
/*
    CREATED: March 2002 BY: Martin Fuhrer, fuhrer@cpsc.ucalgary.ca
    v1.0 March 2002 Initial build.  Material and texture information
       are currently not included in the object file (Maya is not able
       to read material information from an object file.  Alias/Wavefront,
       are you listening?)
    v1.1 April 2002 Duplicate vertices in meshes are welded together.
*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "platform.h"
#include "control.h"
#include "generate.h"
#include "interpret.h"
#include "utility.h"
#include "object.h"
#include "patch.h"
#include "textures.h"
#include "mesh.h"
#include "irisGL.h"
#include "material.h"
#include "compactmesh.h"

#include "test_malloc.h"

#define MAX_GROUPS 1000 /* maximum depth of [] blocks in l-system */
#define MAX_SURFACES 1000
#define MESH_SIZE 1000

/*********** prototypes ***************/
int objSetup(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objStartNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, float /* length */,
                  char /* symbol */);
void objEndNode(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* symbol */);
void objStartBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objEndBranch(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objStartPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objEndPolygon(POLYGON *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objSetColour(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void objSetTexture(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objSetLineWidth(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *);
void objCircle2D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                 float /* radius */);
void objCircle3D(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
                 float /* radius */);
void objCircleB2D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                  float diameter, float width);
void objCircleB3D(const TURTLE *tu, const DRAWPARAM *dr, const VIEWPARAM *vw,
                  float diameter, float width);
void objSphere(const TURTLE *, const DRAWPARAM *, VIEWPARAM *,
               float /* radius */);
void objLabel(const TURTLE *, DRAWPARAM *, const VIEWPARAM *,
              const char * /* label */, int /* parameters */,
              const float * /* values */); /* JH1 */
void objBlackBox(const TURTLE *, const DRAWPARAM *, const VIEWPARAM *,
                 const StringModule * /*module*/,
                 const StringModule * /*submodule*/);
void objPredefinedSurface(TURTLE *, DRAWPARAM *, VIEWPARAM *, char /* ID */,
                          double, double, double /* scale */);
void objLdefinedSurface(StringModule *, TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objFinishUp(TURTLE *, DRAWPARAM *, VIEWPARAM *);
void objRenderTriangle(const float *p1, const float *p2, const float *p3,
                       const DRAWPARAM *);
void objDefineMaterial(TURTLE *, DRAWPARAM *, VIEWPARAM *, Material *);

void objColor(int color);
void objStartTmesh(void);
void objNormal(const float normal[3]);
void objTmeshVertex(const float position[3]);
void objEndTmesh(void);
int objStartTexture(int index);
void objSetTexCoord(const float coords[2]);
void objEndTexture(int index);
void objStartNewGrid(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw, int *size);

void obj_cleanUp();

/* The structure for dispatching rendering routines */
static turtleDrawDispatcher objDrawRoutines = {
    objSetup,        objStartNode,         objEndNode,         objStartBranch,
    objEndBranch,    objStartPolygon,      objEndPolygon,      objSetColour,
    objSetLineWidth, objCircle2D,          objCircle3D,        objSphere,
    objBlackBox,     objPredefinedSurface, objLdefinedSurface, objLabel,
    objFinishUp,     objRenderTriangle,    objStartTexture,    objEndTexture,
    StartTmeshT,     TmeshVertexT,         EndTmeshT,          objCircleB2D,
    objCircleB3D};

/* flag indicating that nodes should be joined */
static int connectNode;

static int active_color;  /* active color applied to surfaces */
static int current_color; /* current color state of turtle  */

/* local prototypes */
static void obj_printSurface();
static void obj_printSurfaceNum(int desiredSurface);
static void obj_startCylinder(TURTLE *tu, double length);
static void obj_endCylinder(TURTLE *tu, DRAWPARAM *dr);
static void o_object(FILE *fp, TURTLE *tu, char desired_surface,
                     double scale_factor, const DRAWPARAM *dr);
static void obj_transformPoint(float pos[3]);
static void obj_transformNormal(float normal[3]);
static void obj_setTransformation(const double *left, const double *heading,
                                  const double *up, const double *position, 
                                  double scale_factor);
static void obj_initTasks();
static void obj_createSurfaces(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw);
static void obj_deleteCurSurface();
static void obj_deleteCurGroup();

static void obj_circle (const DRAWPARAM *dr, const double *left, 
                        const double *heading, const double *up,
                        const double *position, float diameter); 
static void obj_circleB (const DRAWPARAM *dr, const double *left, 
                         const double *heading, const double *up,
                         const double *position,
                         float diameter, float width);

static char modelName[1024];
static char comment[512];

static FILE *fp;
char obj_creating_surfaces;

typedef float Point3d[3];

MeshStruct *groupArray[MAX_GROUPS];
MeshStruct *curGroup;
int groupNum;

MeshStruct *surfaceArray[MAX_GROUPS];
MeshStruct *curSurface;
int surfaceNum;

int totalVertexCount;
int totalGroupCount;

static float renderScale;
static float pntTmnMatrix[4][4]; /* point transformation matrix */
static float norTmnMatrix[4][4]; /* normal transformation matrix */

/********************************************************************/
/* Function: entSetup                                               */
/* return non-zero if there are problems                            */
/********************************************************************/

int objSetup(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {

  fp = fopen(clp.savefilename[SAVE_OBJ], "w");
  if (0 == fp)
    return 1;

  renderScale = dr->rayshade_scale;
  obj_initTasks();

  obj_createSurfaces(tu, dr, vw); /* create bicubic surfaces  */

  /* strip directory from filename and extract name of model (no extension) */
  strcpy(modelName, clp.savefilename[SAVE_OBJ]);
  stripDirectory(modelName);
  changeExtension(modelName, "\0");

  sprintf(comment,
          "## Wavefront Object .obj (generated by cpfg)\n"
          "## Model: \"%s\"\n\n",
          modelName);

  fprintf(fp, "%s\n", comment);

  /* no segment created yet */
  connectNode = FALSE;
  current_color = tu->color_index;
  obj_printSurface();

  return 0;
}

/********************************************************************/
/* Function: objStartNode                                           */
/* Called when F or f encountered in l-system                       */
/********************************************************************/

void objStartNode(TURTLE *tu, __attribute__((unused)) DRAWPARAM *dr,
                  __attribute__((unused)) VIEWPARAM *vw, float length,
                  __attribute__((unused)) char symbol) {
  obj_startCylinder(tu, length);
  connectNode = TRUE;
}

/********************************************************************/
/* Function: objEndNode                                             */
/* Called when the next F, f, or other drawing symbol is parsed     */
/********************************************************************/

void objEndNode(TURTLE *tu, DRAWPARAM *dr,
                __attribute__((unused)) VIEWPARAM *vw,
                __attribute__((unused)) char symbol) {
  if (connectNode) {
    obj_endCylinder(tu, dr);
    connectNode = FALSE;
  }
}

/********************************************************************/
/* Function: objStartBranch                                         */
/* Called when [ encountered in l-system                            */
/********************************************************************/

void objStartBranch(__attribute__((unused)) TURTLE *tu,
                    __attribute__((unused)) DRAWPARAM *dr,
                    __attribute__((unused)) VIEWPARAM *vw) {
  objCreateTriMesh(MESH_SIZE);
}

/********************************************************************/
/* Function: objEndBranch                                           */
/* Called when ] encountered in l-system                            */
/********************************************************************/

void objEndBranch(__attribute__((unused)) TURTLE *tu,
                  __attribute__((unused)) DRAWPARAM *dr,
                  __attribute__((unused)) VIEWPARAM *vw) {
  objWriteTriMesh();
}

/********************************************************************/
/* Function: objStartPolygon                                        */
/* No action                                                        */
/********************************************************************/

void objStartPolygon(__attribute__((unused)) POLYGON *polygon,
                     __attribute__((unused)) TURTLE *tu,
                     __attribute__((unused)) DRAWPARAM *dr,
                     __attribute__((unused)) VIEWPARAM *vw) {
  obj_printSurface();
}

/********************************************************************/
/* Function: objEndPolygon                                          */
/* Output polygon vertices                                          */
/********************************************************************/

void objEndPolygon(__attribute__((unused)) POLYGON *polygon,
                   __attribute__((unused)) TURTLE *tu,
                   __attribute__((unused)) DRAWPARAM *dr,
                   __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: objSetColour                                           */
/* No action                                                        */
/********************************************************************/

void objSetColour(__attribute__((unused)) const TURTLE *tu,
                  __attribute__((unused)) const DRAWPARAM *dr,
                  __attribute__((unused)) const VIEWPARAM *vw) {}

/********************************************************************/
/* Function: objSetTexture                                          */
/* No action                                                        */
/********************************************************************/

void objSetTexture(__attribute__((unused)) TURTLE *tu,
                   __attribute__((unused)) DRAWPARAM *dr,
                   __attribute__((unused)) VIEWPARAM *vw) {}

/********************************************************************/
/* Function: objSetLineWidth                                        */
/* No action                                                        */
/********************************************************************/

void objSetLineWidth(__attribute__((unused)) const TURTLE *tu,
                     __attribute__((unused)) const DRAWPARAM *dr,
                     __attribute__((unused)) const VIEWPARAM *vw) {}

/********************************************************************/
/* Function: objCircle2D                                            */
/********************************************************************/

// local function to output circle for Circle2D,3D and CircleB
static void obj_circle (const DRAWPARAM *dr, const double *left, 
                        const double *heading, const double *up,
                        const double *position, float diameter) {

  int i; // vertex count, triangle count
  int sides = dr->cylinder_sides;

  int nv = sides + 1; // number of vertices
  int nt = sides; // number of triangles

  float radius = diameter / 2;
  MeshStruct *curMesh = curGroup;

  float *v, *n, *tex; // pointers to the mesh data arrays
  int *t;

  if (((curMesh->verElements) + nv * 3 > (curMesh->verMax) * 3) ||
      ((curMesh->triElements) + nt * 3 > (curMesh->triMax) * 3)) 
    objExpandTriMeshV(nt, nv);

  v = (curMesh->meshVertices) + (curMesh->verElements);
  t = (curMesh->meshTriangles) + (curMesh->triElements);
  n = (curMesh->meshNormals) + (curMesh->verElements);
  tex = (curMesh->meshTexture) + (curMesh->texElements);

  // generate vertices
  // first, centroid of circle
  v[0] = position[0];
  v[1] = position[1];
  v[2] = position[2];
  v += 3;
  // next, one vertex per side
  obj_setTransformation(left, heading, up, position, radius);
  for (i = 0; i < sides; i++) {
    // phi describes longitudinal position (0 - 2pi)  
    float phi = (float)i * (1.0 / sides) * 2.0 * M_PI;
    v[0] = cosf(phi);
    v[1] = sinf(phi);
    v[2] = 0.f;
    obj_transformPoint(v);
    v += 3;     
  }

  // assign vertex indicies to triangles
  for (i = 0; i < sides; i++) {
    // always starting at centre of circle
    t[0] = curMesh->verNum; 
    t[1] = curMesh->verNum + i+1;
    t[2] = i == sides-1 ? curMesh->verNum+1 : curMesh->verNum + i+2;
    t += 3;
  }

  // texture coordinates
  tex[0] = 0.5f;
  tex[1] = 0.5f;
  tex += 2;
  for (i = 0; i < sides; i++) {
    float phi = (float)i * (1.0 / sides) * 2.0 * M_PI;
    tex[0] = 0.5f + cosf(phi)*0.5f;
    tex[1] = 0.5f + sinf(phi)*0.5f;
    tex += 2;
  }

  // normals
  n[0] = up[0];
  n[1] = up[1];
  n[2] = up[2];
  Normalize(n);
  n += 3;
  for (i = 0; i < sides; i++) {
    n[0] = up[0];
    n[1] = up[1];
    n[2] = up[2];
    Normalize(n);
    n += 3;
  }

  curMesh->triElements += nt * 3;
  curMesh->verElements += nv * 3;
  curMesh->triNum += nt;
  curMesh->verNum += nv;
}



void objCircle2D(__attribute__((unused)) const TURTLE *tu,
                 __attribute__((unused)) const DRAWPARAM *dr,
                 __attribute__((unused)) VIEWPARAM *vw,
                 __attribute__((unused)) float diameter) {
    const double l[3] = {-1.,0.,0.};
    const double h[3] = {0.,1.,0.};
    const double u[3] = {0.,0.,1.};
    obj_circle(dr,l,h,u,tu->position,diameter);
}

/********************************************************************/
/* Function: objCircle3D                                            */
/********************************************************************/

void objCircle3D(const TURTLE *tu, const DRAWPARAM *dr, VIEWPARAM *vw,
                 float diameter) {
    obj_circle(dr,tu->left,tu->heading,tu->up,tu->position,diameter);
}

/********************************************************************/
/* Function: objCircleB2D                                            */
/********************************************************************/

// local function to output circle for CircleB2D,B3D
// It could be part of one function obj_circle(...)
// that accepts an outer and inner radius 
static void obj_circleB (const DRAWPARAM *dr, const double *left, 
                         const double *heading, const double *up,
                         const double *position,
                         float diameter, float width) {

  int sides = dr->cylinder_sides;

  int nv = sides*2; // number of vertices
  int nt = sides*2; // number of triangles

  float radius = diameter / 2;
  MeshStruct *curMesh = curGroup;

  float *v, *n, *tex; // pointers to the mesh data arrays
  int *t;

  if (((curMesh->verElements) + nv * 3 > (curMesh->verMax) * 3) ||
      ((curMesh->triElements) + nt * 3 > (curMesh->triMax) * 3)) 
    objExpandTriMeshV(nt, nv);

  v = (curMesh->meshVertices) + (curMesh->verElements);
  t = (curMesh->meshTriangles) + (curMesh->triElements);
  n = (curMesh->meshNormals) + (curMesh->verElements);
  tex = (curMesh->meshTexture) + (curMesh->texElements);

  // generate vertices
  obj_setTransformation(left, heading, up, position, 1.);
  for (int i = 0; i < sides; i++) {
    // phi describes longitudinal position (0 - 2pi)  
    float phi = (float)i / (float)sides * 2.f * M_PI;
    // inner vertex
    v[0] = cosf(phi) * (radius - width * 0.5f);
    v[1] = sinf(phi) * (radius - width * 0.5f);
    v[2] = 0.f;
    obj_transformPoint(v);
    v += 3;
    // outer vertex
    v[0] = cosf(phi) * (radius + width * 0.5f);
    v[1] = sinf(phi) * (radius + width * 0.5f);
    v[2] = 0.f;
    obj_transformPoint(v);
    v += 3;
  }

  // assign vertex indicies to triangles
  for (int i = 0; i < sides; i++) {
    t[0] = curMesh->verNum + i*2+0; 
    t[1] = curMesh->verNum + i*2+1;
    t[2] = i == sides-1 ? curMesh->verNum : curMesh->verNum + i*2+2;
    t += 3;
    t[0] = curMesh->verNum + i*2+1; 
    t[1] = i == sides-1 ? curMesh->verNum + 1 : curMesh->verNum + i*2+3;
    t[2] = i == sides-1 ? curMesh->verNum : curMesh->verNum + i*2+2;
    t += 3;
  }

  // texture coordinates
  // generated so that the image is mapped to the triangle strip
  // bottom row of pixels is (innerR,0,0) to (outerR,0,0)
  // This is different from gluDisk...
  for (int i = 0; i < sides; i++) {
    tex[0] = 0.f;
    tex[1] = (float)i / (float)sides;
    tex += 2;
    tex[0] = 1.f;
    tex[1] = (float)i / (float)sides;
    tex += 2;
  }

  // normals
  for (int i = 0; i < sides; i++) {
    n[0] = up[0];
    n[1] = up[1];
    n[2] = up[2];
    Normalize(n);
    n += 3;
    n[0] = up[0];
    n[1] = up[1];
    n[2] = up[2];
    Normalize(n);
    n += 3;
  }

  curMesh->triElements += nt * 3;
  curMesh->verElements += nv * 3;
  curMesh->triNum += nt;
  curMesh->verNum += nv;
}

void objCircleB2D(__attribute__((unused)) const TURTLE *tu,
                  __attribute__((unused)) const DRAWPARAM *dr,
                  __attribute__((unused)) const VIEWPARAM *vw,
                  __attribute__((unused)) float diameter,
                  __attribute__((unused)) float width) {
  const double l[3] = {-1.,0.,0.};
  const double h[3] = {0.,1.,0.};
  const double u[3] = {0.,0.,1.};
  obj_circleB(dr,l,h,u,tu->position,diameter,width);

}

void objCircleB3D(__attribute__((unused)) const TURTLE *tu,
                  __attribute__((unused)) const DRAWPARAM *dr,
                  __attribute__((unused)) const VIEWPARAM *vw,
                  __attribute__((unused)) float diameter,
                  __attribute__((unused)) float width) {
  obj_circleB(dr,tu->left,tu->heading,tu->up,tu->position,diameter,width);
}

/********************************************************************/
/* Function: objSphere                                              */
/********************************************************************/

void objSphere(const TURTLE *tu, const DRAWPARAM *dr, VIEWPARAM *vw,
               float diameter) {
  int i, j, vc = 0, tc = 0; // vertex count, triangle count
  int vp = 0;               // vertex pointer
  int sides = dr->cylinder_sides;

  int nv = 4 * sides * sides - 2 * sides; // number of vertices
  int nt = 2 * sides * sides - 2 * sides; // number of triangles

  float radius = diameter / 2;
  MeshStruct *curMesh = curGroup;

  float *v, *n, *tex; // pointers to the mesh data arrays
  int *t;
  int cntVertices = 0;
  int cntNormals = 0;

  if (sides <= 2) // sphere with only two sides is a circle
    objCircle2D(tu, dr, vw, diameter);

  if (((curMesh->verElements) + nv * 3 > (curMesh->verMax) * 3) ||
      ((curMesh->triElements) + nt * 3 > (curMesh->triMax) * 3))
    objExpandTriMeshV(nt, nv);

  v = (curMesh->meshVertices) + (curMesh->verElements);
  t = (curMesh->meshTriangles) + (curMesh->triElements);
  n = (curMesh->meshNormals) + (curMesh->verElements);
  tex = (curMesh->meshTexture) + (curMesh->texElements);

  // generate vertices
  for (i = 0; i < sides; i++) {
    for (j = 0; j < sides; j++) {
      // theta describes latitudinal position from pole to pole (0 - pi)
      // phi describes longitudinal position (0 - 2pi)
      float theta = (float)i * (1.0 / sides) * M_PI;
      float phi = (float)j * (1.0 / sides) * 2.0 * M_PI;
      float ntheta = (float)(i + 1) * (1.0 / sides) * M_PI;
      float nphi = (float)(j + 1) * (1.0 / sides) * 2.0 * M_PI;

      PolarTo3d(theta, phi, radius, v);
      v += 3;
      cntVertices += 3;

      if (i != 0) {
        PolarTo3d(theta, nphi, radius, v);
        v += 3;
        cntVertices += 3;
      }

      if (i != sides - 1) {
        PolarTo3d(ntheta, phi, radius, v);
        v += 3;
        cntVertices += 3;
      }

      PolarTo3d(ntheta, nphi, radius, v);
      v += 3;
      cntVertices += 3;

      vc += 4 - (i == 0 || i == sides - 1);
    }
  }
  v -= cntVertices;
  for (i = 0; i < cntVertices; i += 3) {
    v[i + 0] += tu->position[0];
    v[i + 1] += tu->position[1];
    v[i + 2] += tu->position[2];
  }
  v += cntVertices;

  // triangle indices to vertices
  for (i = 0; i < sides; i++) {
    for (j = 0; j < sides; j++) {
      // top strip of triangles (joined at the north pole)
      if (i == 0) {
        t[0] = curMesh->verNum + vp;
        t[1] = curMesh->verNum + vp + 1;//2;
        t[2] = curMesh->verNum + vp + 2;//1;
        t += 3;
        tc++;
        vp += 3;

      }

      // bottom strip of triangles (joined at the south pole)
      else if (i == sides - 1) {
        t[0] = curMesh->verNum + vp;
        t[1] = curMesh->verNum + vp + 2;//1;
        t[2] = curMesh->verNum + vp + 1;//2;
        t += 3;
        tc++;
        vp += 3;
      }

      // triangles in centre strips around sphere
      else {
        t[0] = curMesh->verNum + vp;
        t[1] = curMesh->verNum + vp + 2;//3;
        t[2] = curMesh->verNum + vp + 3;//2;
        t += 3;

        t[0] = curMesh->verNum + vp;
        t[1] = curMesh->verNum + vp + 3;//1;
        t[2] = curMesh->verNum + vp + 1;//3;
        t += 3;

        tc += 2;
        vp += 4;
      }
    }
  }

  // texture coordinates
  for (i = 0; i < sides; i++) {
    for (j = 0; j < sides; j++) {
      float alpha = 0.5f;

      tex[0] = (float)i * (1.0 / sides) - alpha;
      tex[1] = (float)j * (1.0 / sides) - alpha;
      tex += 2;

      if (i != 0) {
        tex[0] = (float)i * (1.0 / sides) - alpha;
        tex[1] = (float)((j + 1) % sides) * (1.0 / sides) - alpha;
        tex += 2;
      }

      if (i != sides - 1) {
        tex[0] = (float)(i + 1) * (1.0 / sides) - alpha;
        tex[1] = (float)j * (1.0 / sides) - alpha;
        tex += 2;
      }

      tex[0] = (float)(i + 1) * (1.0 / sides) - alpha;
      tex[1] = (float)((j + 1) % sides) * (1.0 / sides) - alpha;
      tex += 2;
    }
  }

  // normals
  for (i = 0; i < sides; i++) {
    for (j = 0; j < sides; j++) {
      float theta = (float)i * (1.0 / sides) * M_PI;
      float phi = (float)j * (1.0 / sides) * 2.0 * M_PI;
      float ntheta = (float)(i + 1) * (1.0 / sides) * M_PI;
      float nphi = (float)(j + 1) * (1.0 / sides) * 2.0 * M_PI;

      PolarTo3d(theta, phi, radius, n);
      Normalize(n);
      n += 3;
      cntNormals += 3;

      if (i != 0) {
        PolarTo3d(theta, nphi, radius, n);
        Normalize(n);
        n += 3;
        cntNormals += 3;
      }

      if (i != sides - 1) {
        PolarTo3d(ntheta, phi, radius, n);
        Normalize(n);
        n += 3;
        cntNormals += 3;
      }

      PolarTo3d(ntheta, nphi, radius, n);
      Normalize(n);
      n += 3;
      cntNormals += 3;
    }
  }
  // The reversal of normals is not necessary...
  // The winding order just had to be changed
  //n -= cntNormals;
  //for (i = 0; i < cntNormals; i += 3) {
  //  n[i + 0] *= -1;
  //  n[i + 1] *= -1;
  //  n[i + 2] *= -1;
  //}
  //v += cntNormals;

  curMesh->triElements += nt * 3;
  curMesh->verElements += nv * 3;
  curMesh->triNum += nt;
  curMesh->verNum += nv;

}

/********************************************************************/
/* Function: objBLackBox                                            */
/********************************************************************/

void objBlackBox(__attribute__((unused)) const TURTLE *tu,
                 __attribute__((unused)) const DRAWPARAM *dr,
                 __attribute__((unused)) const VIEWPARAM *vw,
                 __attribute__((unused)) const StringModule *module,
                 __attribute__((unused)) const StringModule *submodule) {}

/********************************************************************/
/* Function: objLabel                   JH1                         */
/********************************************************************/

void objLabel(__attribute__((unused)) const TURTLE *tu,
              __attribute__((unused)) DRAWPARAM *dr,
              __attribute__((unused)) const VIEWPARAM *vw,
              __attribute__((unused)) const char *label,
              __attribute__((unused)) int parameteri,
              __attribute__((unused)) const float values[]) {}

/********************************************************************/
/* Function: objStartTexture                                        */
/* Initializes texture                                              */
/********************************************************************/

int objStartTexture(__attribute__((unused)) int index) { return 1; }

void obj_finish_up_texture(__attribute__((unused)) int index,
                           __attribute__((unused)) int map_type) {}

/********************************************************************/
void objEndTexture(__attribute__((unused)) int index) {}

/********************************************************************/
/* Function: objPredefinedSurface                                   */
/********************************************************************/

void objPredefinedSurface(TURTLE *tu, DRAWPARAM *dr,
                          __attribute__((unused)) VIEWPARAM *vw, char id,
                          double scale, __attribute__((unused)) double sy,
                          __attribute__((unused)) double sz) {
  if (scale != 0.0)
    o_object(fp, tu, id, scale, dr);
}

/********************************************************************/
/* Function: objLdefinedSurface                                     */
/* No action                                                        */
/********************************************************************/

void objLdefinedSurface(StringModule *module, TURTLE *tu, DRAWPARAM *dr,
                        VIEWPARAM *vw) {
  dr->gllighting = 1;
  dr->vertexbound = 1;
  dr->ourlighting = 0;
  dr->texture = 0;

  SurfaceTmeshDraw(module, tu, dr, vw, StartTmeshT, TmeshVertexT, EndTmeshT);
}

/********************************************************************/
void objStartNewGrid(__attribute__((unused)) TURTLE *tu,
                     __attribute__((unused)) DRAWPARAM *dr,
                     __attribute__((unused)) VIEWPARAM *vw,
                     __attribute__((unused)) int *size) {
  // int c;

  /* no segment created yet */
  connectNode = FALSE;
}

/********************************************************************/
/* Function: objDefineMaterial                                      */
/* No action                                                        */
/********************************************************************/

void objDefineMaterial(__attribute__((unused)) TURTLE *tu,
                       __attribute__((unused)) DRAWPARAM *dr,
                       __attribute__((unused)) VIEWPARAM *vw,
                       __attribute__((unused)) Material *mat) {
  return;
}

/********************************************************************/
/* Function: objFinishUp                                            */
/* No action                                                        */
/********************************************************************/

void objFinishUp(__attribute__((unused)) TURTLE *tu,
                 __attribute__((unused)) DRAWPARAM *dr,
                 __attribute__((unused)) VIEWPARAM *vw) {
  objWriteTriMesh();
  obj_cleanUp();
  if (clp.savefp[SAVE_OBJ] != stdin) {
    fclose(fp);
    fclose(clp.savefp[SAVE_OBJ]);
  }
  clp.savefp[SAVE_OBJ] = NULL;
}

/********************************************************************/
/* Function: objRenderTriangle                                      */
/* "renders" triangle                                               */
/********************************************************************/

void objRenderTriangle(const float *p1, const float *p2, const float *p3,
                       const DRAWPARAM *dr) {
  int c;
  float vec1[3], vec2[3], normal[3];
  float *v, *n, *tex;
  int *t;
  MeshStruct *curMesh;

  if (obj_creating_surfaces)
    curMesh = curSurface;
  else
    curMesh = curGroup;

  if (((curMesh->verElements) + 9 > (curMesh->verMax) * 3 ||
       (curMesh->triElements) + 3 > (curMesh->triMax) * 3) &&
      (!obj_creating_surfaces))
    objExpandTriMesh(MESH_SIZE);

  v = (curMesh->meshVertices) + (curMesh->verElements);
  t = (curMesh->meshTriangles) + (curMesh->triElements);
  n = (curMesh->meshNormals) + (curMesh->verElements);
  tex = (curMesh->meshTexture) + (curMesh->texElements);

  // create triangle (pointers to vertices)
  for (c = 0; c < 3; c++)
    t[c] = (curMesh->verNum) + c; // + totalVertexCount + 1;  //debug
  curMesh->triElements += 3;

  for (c = 0; c < 3; c++) {
    v[c] = p1[c];
    v[c + 3] = p2[c];
    v[c + 6] = p3[c];
  }
  curMesh->verElements += 9;

  // get the triangle normal
  for (c = 0; c < 3; c++) {
    vec1[c] = p2[ePOINT * 3 + c] - p1[ePOINT * 3 + c];
    vec2[c] = p3[ePOINT * 3 + c] - p1[ePOINT * 3 + c];
  }
  CrossProduct(vec1, vec2, normal);

  // determine vertex normals
  for (c = 0; c < 3; c++) {
    n[c] = p1[eNORMAL * 3 + c];
    n[c + 3] = p2[eNORMAL * 3 + c];
    n[c + 6] = p3[eNORMAL * 3 + c];
  }

  if (DotProduct(normal, p1 + eNORMAL * 3) <= 0) {
    for (c = 0; c < 3; c++)
      n[c] = -n[c];
  }

  if (DotProduct(normal, p2 + eNORMAL * 3) <= 0) {
    for (c = 0; c < 3; c++)
      n[3 + c] = -n[3 + c];
  }

  if (DotProduct(normal, p3 + eNORMAL * 3) <= 0) {
    for (c = 0; c < 3; c++)
      n[6 + c] = -n[6 + c];
  }

  if (dr->texture) {
    for (c = 0; c < 2; c++) {
      tex[c] = p1[eTEXTURE * 3 + c];
      tex[c + 2] = p2[eTEXTURE * 3 + c];
      tex[c + 4] = p3[eTEXTURE * 3 + c];
    }

    curMesh->texElements += 6;
  }

  curMesh->verNum += 3;
  curMesh->triNum += 1;
}

/*********************************************************************/
static double StartPos[3];
static double StartH[3];
static double StartU[3];
static double StartL[3];
static double StartWidth;
static double StartLength;
static int StartIndex;
static float StartTexT;

static void obj_startCylinder(TURTLE *tu, double length) {
  int c;

  StartIndex = tu->color_index;
  StartWidth = tu->line_width;
  StartLength = length;

  for (c = 0; c < 3; c++) {
    StartPos[c] = tu->position[c];
    StartH[c] = tu->heading[c];
    StartU[c] = tu->up[c];
    StartL[c] = tu->left[c];
  }

  StartTexT = tu->tex_t;
}

static void obj_endCylinder(TURTLE *tu, DRAWPARAM *dr) {
  int i, j;
  float add_t;
  double EndWidth, vec[3];

  int conesides, conevertices, curstep, conedatapoints;
  int *verts;
  float theta, thetastep, endRadius, startRadius;
  Point3d *positionData, *polyNormals, *vertexNormals;

  float *v, *n;
  int *t;

  for (j = 0; j < 3; j++)
    vec[j] = tu->position[j] - StartPos[j];

  /* test for degenerated cone */
  if (vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2] == 0)
    return;

  obj_setTransformation(StartL, StartH, StartU, StartPos, 1);

  /* cylinder or cone? */
  EndWidth = tu->line_width;
  conesides = dr->cylinder_sides;
  thetastep = 2 * M_PI / (float)conesides;
  conevertices = 6 * conesides;
  conedatapoints = 2 * conesides;

  if ((curGroup->verElements) + conedatapoints * 3 > (curGroup->verMax) * 3 ||
      (curGroup->triElements) + conedatapoints * 3 > (curGroup->triMax) * 3)
    objExpandTriMesh(MESH_SIZE);

  verts = (int *)Malloc(conevertices * sizeof(int));
  positionData = (Point3d *)Malloc(conedatapoints * sizeof(Point3d));
  polyNormals = (Point3d *)Malloc(conesides * sizeof(Point3d));
  vertexNormals = (Point3d *)Malloc(conedatapoints * sizeof(Point3d));

  if (!verts || !positionData || !polyNormals || !vertexNormals) {
    Message("Error: [object.c] Out of memory.\n");

    if (verts)
      Free(verts);
    if (positionData)
      Free(positionData);
    if (polyNormals)
      Free(polyNormals);
    if (vertexNormals)
      Free(vertexNormals);
    obj_cleanUp();
    MyExit(1);
  }

  for (i = 0; i < conesides; i++) {
    verts[i * 6] = i * 2;
    verts[i * 6 + 1] = i * 2 + 1;
    verts[i * 6 + 2] = i * 2 + 3;
    verts[i * 6 + 3] = i * 2 + 3;
    verts[i * 6 + 4] = i * 2 + 2;
    verts[i * 6 + 5] = i * 2;
  }
  verts[conevertices - 2] = 0;
  verts[conevertices - 3] = 1;
  verts[conevertices - 4] = 1;

  endRadius = (float)EndWidth / 2.0f;
  startRadius = (float)StartWidth / 2.0f;
  for (i = 0, curstep = 0, theta = 0; i < conedatapoints;
       i += 2, theta = ++curstep * thetastep) {
    positionData[i][0] = endRadius * cos(theta);
    positionData[i][1] = StartLength;
    positionData[i][2] = -endRadius * sin(theta);
    obj_transformPoint(positionData[i]);
  }

  for (i = 1, curstep = 0, theta = 0; i < conedatapoints;
       i += 2, theta = ++curstep * thetastep) {
    positionData[i][0] = startRadius * cos(theta);
    positionData[i][1] = 0;
    positionData[i][2] = -startRadius * sin(theta);
    obj_transformPoint(positionData[i]);
  }

  /* compute polygon normals */
  for (i = 0; i < conedatapoints; i += 2) {
    Point3d v1, v2, normal;
    for (j = 0; j < 3; j++) {
      if (i == conedatapoints - 2)
        v1[j] = positionData[0][j] - positionData[i][j];
      else
        v1[j] = positionData[i + 2][j] - positionData[i][j];
      v2[j] = positionData[i + 1][j] - positionData[i][j];
    }
    CrossProduct(v2, v1, normal);
    Normalize(normal);
    for (j = 0; j < 3; j++)
      polyNormals[i / 2][j] = normal[j];
  }

  /* compute vertex normals */
  for (j = 0; j < 3; j++)
    vertexNormals[0][j] = vertexNormals[1][j] =
        (polyNormals[conesides - 1][j] + polyNormals[0][j]) / 2.0f;
  Normalize(vertexNormals[0]);
  Normalize(vertexNormals[1]);

  for (i = 1; i < conesides; i++) {
    for (j = 0; j < 3; j++)
      vertexNormals[i * 2][j] = vertexNormals[i * 2 + 1][j] =
          (polyNormals[i - 1][j] + polyNormals[i][j]) / 2.0f;
    Normalize(vertexNormals[i * 2]);
    Normalize(vertexNormals[i * 2 + 1]);
  }

  if ((curGroup->verElements) + conedatapoints * 3 > (curGroup->verMax) * 3 ||
      ((curGroup->triElements) + (conesides * 2) > (curGroup->triMax) * 3))
    objExpandTriMesh(MESH_SIZE);

  v = (curGroup->meshVertices) + (curGroup->verElements);
  t = (curGroup->meshTriangles) + (curGroup->triElements);
  n = (curGroup->meshNormals) + (curGroup->verElements);

  for (i = 0; i < conevertices; i++)
    t[i] = (curGroup->verNum) + verts[i];

  for (i = 0; i < conedatapoints; i++) {
    for (j = 0; j < 3; j++) {
      v[i * 3 + j] = positionData[i][j];
      n[i * 3 + j] = vertexNormals[i][j];
    }
  }

  curGroup->triElements += conevertices;
  curGroup->verElements += conedatapoints * 3;
  curGroup->triNum += conesides * 2;
  curGroup->verNum += conedatapoints;

  if (verts)
    Free(verts);
  if (positionData)
    Free(positionData);
  if (polyNormals)
    Free(polyNormals);
  if (vertexNormals)
    Free(vertexNormals);

  if (is_valid_texture_index(tu->texture)) {
    add_t = update_segment_texture(tu->texture, tu->line_width, 1.0);
  }
}

void obj_createSurfaces(TURTLE *tu, DRAWPARAM *dr, VIEWPARAM *vw) {
  obj_creating_surfaces = 1;
  o_objects(fp, tu, dr, vw);
  obj_creating_surfaces = 0;
}

/* Print the current color state of the turtle, if it is not already the
active color */

static void obj_printSurface() {
  if (active_color != current_color)
    obj_printSurfaceNum(current_color);
}

/* Print a desired color or surface state */

static void obj_printSurfaceNum(__attribute__((unused)) int surfaceNum) {}

/* Instantiate an object/surface at the place specified by the turtle
for the purpose of ray-tracing. */

static void o_object(__attribute__((unused)) FILE *fp, TURTLE *tu,
                     char desired_surface, double scale_factor,
                     const DRAWPARAM *dr) {
  int i, j;
  float p1[14], p2[14], p3[14];

  MeshStruct *surface = 0; // 2012.03.19 PBdR: Added initialization

  for (i = 0; i < surfaceNum; i++)
    if (surfaceArray[i]->id == (int)desired_surface)
      surface = surfaceArray[i];

  obj_setTransformation(tu->left, tu->heading, tu->up, tu->position,
                        scale_factor);

  for (i = 0; i < surface->triNum; i++) {
    /* transform and store surface points and normals */
    for (j = 0; j < 3; j++) {
      p1[j] = surface->meshVertices[i * 9 + j];
      p2[j] = surface->meshVertices[i * 9 + 3 + j];
      p3[j] = surface->meshVertices[i * 9 + 6 + j];

      p1[3 + j] = surface->meshNormals[i * 9 + j];
      p2[3 + j] = surface->meshNormals[i * 9 + 3 + j];
      p3[3 + j] = surface->meshNormals[i * 9 + 6 + j];
    }

    obj_transformPoint(p1);
    obj_transformPoint(p2);
    obj_transformPoint(p3);

    obj_transformNormal(p1 + 3);
    obj_transformNormal(p2 + 3);
    obj_transformNormal(p3 + 3);

    for (j = 0; j < 2; j++) {
      p1[6 + j] = surface->meshTexture[i * 6 + j];
      p2[6 + j] = surface->meshTexture[i * 6 + 2 + j];
      p3[6 + j] = surface->meshTexture[i * 6 + 4 + j];
    }

    /* for future implementation of tex_t and tex_s */
    for (j = 8; j < 14; j++)
      p1[j] = p2[j] = p3[j] = 0;

    objRenderTriangle(p1, p2, p3, dr);
  }
}

void obj_transformPoint(float point[3]) {
  int i;
  float point4[4], result[4];
  for (i = 0; i < 3; i++)
    point4[i] = point[i];
  point4[3] = 1;
  Mat4Vec4Mult(pntTmnMatrix, point4, result);
  for (i = 0; i < 3; i++)
    point[i] = result[i];
}

void obj_transformNormal(float normal[3]) {
  int i;
  float normal4[4], result[4];
  for (i = 0; i < 3; i++)
    normal4[i] = normal[i];
  normal4[3] = 1;
  Mat4Vec4Mult(norTmnMatrix, normal4, result);
  for (i = 0; i < 3; i++)
    normal[i] = result[i];
}

void obj_setTransformation(const double *left, const double *heading,
                           const double *up, const double *position, 
                           double scale_factor) {
  float rotMatrix[4][4];
  float sclMatrix[4][4];
  float trlMatrix[4][4];
  float tmpMatrix[4][4];
  float norTmnMatrix3[3][3];
  float pntTmnMatrix3[3][3];
  int i, j;

  IdentityMat4(rotMatrix);
  IdentityMat4(sclMatrix);
  IdentityMat4(trlMatrix);
  IdentityMat4(tmpMatrix);

  if (scale_factor != 1) {
    sclMatrix[0][0] = sclMatrix[1][1] = sclMatrix[2][2] = scale_factor;
    sclMatrix[3][3] = 1;

    for (j = 0; j < 3; j++) {
      rotMatrix[0][j] = left[j];
      rotMatrix[1][j] = heading[j];
      rotMatrix[2][j] = up[j];
      rotMatrix[j][3] = 0;
    }
    rotMatrix[3][3] = 1.0;
    TransposeMat4(rotMatrix);
    for (j = 0; j < 3; j++) {
      rotMatrix[j][0] *= -1;
      rotMatrix[j][2] *= 1; // should this be -1 ?
    }

    for (j = 0; j < 3; j++) {
      trlMatrix[j][j] = 1;
      trlMatrix[j][3] = position[j];
    }
    trlMatrix[3][3] = 1.0;

    MatMult4x4(trlMatrix, rotMatrix, tmpMatrix);

    MatMult4x4(tmpMatrix, sclMatrix, pntTmnMatrix);
  } else {
    for (j = 0; j < 3; j++) {
      rotMatrix[0][j] = left[j];
      rotMatrix[1][j] = heading[j];
      rotMatrix[2][j] = up[j];
      rotMatrix[j][3] = 0;
    }
    rotMatrix[3][3] = 1.0;
    TransposeMat4(rotMatrix);
    for (j = 0; j < 3; j++) {
      rotMatrix[j][0] *= -1;
      rotMatrix[j][2] *= 1; // should this be -1 ?
    }

    for (j = 0; j < 3; j++) {
      trlMatrix[j][j] = 1;
      trlMatrix[j][3] = position[j];
    }
    trlMatrix[3][3] = 1.0;

    MatMult4x4(trlMatrix, rotMatrix, pntTmnMatrix);
  }

  /* get normal transformation matrix */
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      pntTmnMatrix3[i][j] = pntTmnMatrix[i][j];

  Inverse(pntTmnMatrix3, norTmnMatrix3);
  Transpose(norTmnMatrix3);

  IdentityMat4(norTmnMatrix);
  for (i = 0; i < 3; i++)
    for (j = 0; j < 3; j++)
      norTmnMatrix[i][j] = norTmnMatrix3[i][j];
}

/********************************************************************/
/* Puts appropriate routines in the dispatch table and makes other  */
/* settings depending on drawing and viewing parameters, such as    */
/* the drawing parameter shade mode.                                */
/********************************************************************/
turtleDrawDispatcher *objSetDispatcher(__attribute__((unused)) DRAWPARAM *dr,
                                       __attribute__((unused)) VIEWPARAM *vw) {
  return (&objDrawRoutines);
}

void obj_initTasks() {
  float tmatrix[4][4] = {
      {1, 0, 0, 0}, {0, 1, 0, 500}, {0, 0, 1, 0}, {0, 0, 0, 1}};
  float point[4] = {0, 0, 0, 1};
  float result[4];
  int i;

  for (i = 0; i < MAX_GROUPS; i++)
    groupArray[i] = 0;

  groupNum = 0;
  curGroup = 0;

  totalVertexCount = 0;
  totalGroupCount = 0;

  IdentityMat4(pntTmnMatrix);
  IdentityMat4(norTmnMatrix);
  objCreateTriMesh(MESH_SIZE);
  active_color = -1;

  // debug
  Mat4Vec4Mult(tmatrix, point, result);
}

void obj_deleteMesh(MeshStruct *group) {
  if (group->meshVertices)
    Free(group->meshVertices);
  if (group->meshNormals)
    Free(group->meshNormals);
  if (group->meshTriangles)
    Free(group->meshTriangles);
  if (group->meshTexture)
    Free(group->meshTexture);
  if (group->meshSurface)
    Free(group->meshSurface);
}

void obj_deleteCurGroup() {
  if (curGroup == 0 || groupNum <= 0) {
    Message("Warning [object.c]: No groups to delete.\n");
    return;
  }

  obj_deleteMesh(curGroup);

  Free(curGroup);
  groupNum--;
  groupArray[groupNum] = 0;

  if (groupNum - 1 >= 0)
    curGroup = groupArray[groupNum - 1];
  else
    curGroup = 0;
}

void obj_cleanUp() {
  while (groupNum != 0)
    obj_deleteCurGroup();
  while (surfaceNum != 0)
    obj_deleteCurSurface();
}

/* Create a triangular mesh structure with maxTriangles triangles */

void objCreateTriMesh(int maxTriangles) {
  if (groupNum == MAX_GROUPS) {
    Message("Error [object.c]: Cannot create a new group. Increase "
           "MAX_GROUPS in object.c\n and recompile.\n");
    obj_cleanUp();
    MyExit(1);
  }

  groupArray[groupNum] = (MeshStruct *)Malloc(sizeof(MeshStruct));
  curGroup = groupArray[groupNum];
  groupNum++;

  curGroup->verElements = 0;
  curGroup->triElements = 0;
  curGroup->texElements = 0;
  curGroup->verNum = 0;
  curGroup->triNum = 0;
  curGroup->verMax = maxTriangles * 3;
  curGroup->triMax = maxTriangles;
  curGroup->id = totalGroupCount;

  curGroup->meshVertices =
      (float *)Malloc(curGroup->verMax * 3 * sizeof(float));
  curGroup->meshNormals = (float *)Malloc(curGroup->verMax * 3 * sizeof(float));
  curGroup->meshTexture = (float *)Malloc(curGroup->verMax * 2 * sizeof(float));
  curGroup->meshSurface = (float *)Malloc(curGroup->verMax * 3 * sizeof(float));
  curGroup->meshTriangles = (int *)Malloc(curGroup->triMax * 3 * sizeof(int));
}

/* Expand the mesh by adding addTriangles new triangles and addVertices new
vertices.  This function must at times reallocate memory for the mesh, so any
existing pointers to the mesh arrays could be invalidated. */

void objExpandTriMesh(int addTriangles) {
  int addVertices = addTriangles * 3;
  objExpandTriMeshV(addTriangles, addVertices);
}

void objExpandTriMeshV(int addTriangles, int addVertices) {
  int newTriMax = curGroup->triMax + addTriangles;
  int newVerMax = curGroup->verMax + addVertices;

  curGroup->meshVertices =
      (float *)realloc(curGroup->meshVertices, newVerMax * 3 * sizeof(float));
  curGroup->meshNormals =
      (float *)realloc(curGroup->meshNormals, newVerMax * 3 * sizeof(float));
  curGroup->meshTriangles =
      (int *)realloc(curGroup->meshTriangles, newTriMax * 3 * sizeof(int));
  curGroup->meshTexture =
      (float *)realloc(curGroup->meshTexture, newVerMax * 2 * sizeof(float));
  curGroup->meshSurface =
      (float *)realloc(curGroup->meshSurface, newVerMax * 3 * sizeof(float));

  if (!(curGroup->meshVertices) || !(curGroup->meshNormals) ||
      !(curGroup->meshTriangles) || !(curGroup->meshTexture) ||
      !(curGroup->meshSurface)) {
    Message("Error: [object.c] Out of memory.\n");
    obj_cleanUp();
    MyExit(1);
  }

  curGroup->triMax = newTriMax;
  curGroup->verMax = newVerMax;
}

/* Save and delete the triangular mesh structure */

void objWriteTriMesh() {
  int i, offset;

  if (curGroup == 0) {
    Message("Warning: [object.c] No group to write out.\n");
    return;
  }

  curGroup->verNum = compactMesh(curGroup->triNum, curGroup->meshTriangles,
                                 curGroup->verNum, curGroup->meshVertices,
                                 curGroup->meshNormals, curGroup->meshTexture);

  fprintf(fp, "g group%d\n\n", totalGroupCount);

  /* write the vertices */
  for (i = 0; i < (curGroup->verNum) * 3; i = i + 3) {
    if (renderScale == 1)
      fprintf(fp, "v %g %g %g\n", curGroup->meshVertices[i],
              curGroup->meshVertices[i + 1], curGroup->meshVertices[i + 2]);
    else
      fprintf(fp, "v %g %g %g\n", renderScale * curGroup->meshVertices[i],
              renderScale * curGroup->meshVertices[i + 1],
              renderScale * curGroup->meshVertices[i + 2]);
  }

  /* write the vertex normals */
  for (i = 0; i < (curGroup->verNum) * 3; i = i + 3)
    fprintf(fp, "vn %g %g %g\n", curGroup->meshNormals[i],
            curGroup->meshNormals[i + 1], curGroup->meshNormals[i + 2]);

  /* write the vertex texture coordinates */
  for (i = 0; i < (curGroup->verNum) * 2; i = i + 2)
    fprintf(fp, "vt %g %g\n", curGroup->meshTexture[i],
            curGroup->meshTexture[i + 1]);

  /* identify triangles by writing indices to the vertices */
  offset = 1 + totalVertexCount;
  for (i = 0; i < (curGroup->triNum) * 3; i = i + 3)
    fprintf(fp, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
            curGroup->meshTriangles[i] + offset,
            curGroup->meshTriangles[i] + offset,
            curGroup->meshTriangles[i] + offset,
            curGroup->meshTriangles[i + 1] + offset,
            curGroup->meshTriangles[i + 1] + offset,
            curGroup->meshTriangles[i + 1] + offset,
            curGroup->meshTriangles[i + 2] + offset,
            curGroup->meshTriangles[i + 2] + offset,
            curGroup->meshTriangles[i + 2] + offset);
  fprintf(fp, "\n");

  totalVertexCount += curGroup->verNum;
  totalGroupCount++;
  obj_deleteCurGroup();
}

void obj_deleteCurSurface() {
  if (curSurface == 0 || surfaceNum <= 0) {
    Message("Warning [object.c]: No surfaces to delete.\n");
    return;
  }

  obj_deleteMesh(curSurface);

  Free(curSurface);
  surfaceNum--;
  surfaceArray[surfaceNum] = 0;

  if (surfaceNum - 1 >= 0)
    curSurface = surfaceArray[surfaceNum - 1];
  else
    curSurface = 0;
}

/* Create a bicubic surface mesh */

void objCreateSurfaceMesh(int maxTriangles, char surfaceIdentifier) {
  if (surfaceNum == MAX_SURFACES) {
    Message("Error [object.c]: Cannot create a new surface. Increase "
           "MAX_SURFACES in object.c\n and recompile.\n");
    obj_cleanUp();
    MyExit(1);
  }

  surfaceArray[surfaceNum] = (MeshStruct *)Malloc(sizeof(MeshStruct));
  curSurface = surfaceArray[surfaceNum];

  surfaceNum++;

  curSurface->verElements = 0;
  curSurface->triElements = 0;
  curSurface->texElements = 0;
  curSurface->verNum = 0;
  curSurface->triNum = 0;
  curSurface->verMax = maxTriangles * 3;
  curSurface->triMax = maxTriangles;
  curSurface->id = (int)surfaceIdentifier;

  curSurface->meshVertices =
      (float *)Malloc(curSurface->verMax * 3 * sizeof(float));
  curSurface->meshNormals =
      (float *)Malloc(curSurface->verMax * 3 * sizeof(float));
  curSurface->meshTexture =
      (float *)Malloc(curSurface->verMax * 2 * sizeof(float));
  curSurface->meshSurface =
      (float *)Malloc(curSurface->verMax * 3 * sizeof(float));
  curSurface->meshTriangles =
      (int *)Malloc(curSurface->triMax * 3 * sizeof(int));
}
