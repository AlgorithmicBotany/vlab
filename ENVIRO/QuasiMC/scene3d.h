/* scene3d.h - a 3d dimensional scene as a set of triangles and quads */

#ifndef _SCENE3D_H
#define _SCENE3D_H

typedef struct tagCELL {
  int num_primitives;
  int max_primitives;
  int *list; /* index of primitive - pointer to index in primitives array */
} CELL;

typedef struct tagGRID {
  int size[3]; /* number of nodes for X,Y,Z */
  int default_size[3];
  float cell_size[3];
  float bsph_centre[3]; /* bounding sphere for the entire scene */
  float bsph_radius;
  float bbox[6]; /* the six extremes that define a bounding box */
  float range[6];
  float maxdist; /* the distance of the largest extreme of bbox */
  int num_cells;
  int cell[3], cell_step[3]; /* used to find the voxels that are pierced */
  float smallest[3], smallest_step[3]; /* by the ray */
  CELL *cells;
} GRID;

typedef struct tagLIGHT {
  float dir[3], u[3], v[3]; /* directional light - no location is needed */
  float weight;
} LIGHT;

typedef struct tagMATERIAL {
  float reflectance[MAX_SPECTRUM_SAMPLES];
  float spec_power[MAX_SPECTRUM_SAMPLES];
  float transmittance[MAX_SPECTRUM_SAMPLES];
  float trans_power[MAX_SPECTRUM_SAMPLES];
  float Nt[MAX_SPECTRUM_SAMPLES]; /* index of refraction */
} MATERIAL;

typedef struct tagPRIMITIVE {
  int object_type;
  int plane2d;                /* which 2d plane to project polygon onto */
  unsigned int ray_signature; /* for intersection testing */
  float data[12];             /* maximum of four vertices (x,y,z) */
  float normal[3];
  float area;
  float avg_cos_angle; // the average cosine angle between direct incoming ray
                       // an object (used for projected area)
  float absorbed_flux[MAX_SPECTRUM_SAMPLES]; // how much light is absorbed from
                                             // light incident on surface
  float incident_flux[MAX_SPECTRUM_SAMPLES]; // how much light is incident on
                                             // surface
  float direct_hits[MAX_SPECTRUM_SAMPLES]; // the number of direct hits (at ray
                                           // depth = 1)
  MATERIAL *material[2];
} PRIMITIVE;

typedef struct tagSCENE {
  PRIMITIVE *primitives; /* the triangles and quads in the scene */
  int num_primitives;
  int max_primitives;

  SURFACE surfaces[MAX_SURFACES]; /* the surfaces in the scene - instances */
  int num_surfaces;

  MATERIAL *materials; /* the materials in the scene */
  int num_materials;
  int max_materials;

  LIGHT *lights; /* the lights in the scene - directional lights */
  int num_lights;
  int max_lights;
  float sum_light_weights; // sum of a light source weights
  float max_light_weight;  // maxium weight over all light sources

  GRID grid; /* uniform spatial subdivison grid */
} SCENE;

#ifdef __cplusplus
extern "C" {
#endif
void InitScene(SCENE *scene);
void ResetScene(SCENE *scene);
void FreeScene(SCENE *scene);
void ResetGrid(SCENE *scene, int x_size, int y_size, int z_size);

int AddObject(SCENE *scene, CTURTLE *turtle, Cmodule_type *module,
              int material_param);
int AddTriangle(SCENE *scene, float *vertices, int npars, float *up,
                Cmodule_type *module);
void FindBoundingSphere(SCENE *scene);
void FillGrid(SCENE *scene);
void BoundQueryObject(SCENE *scene, QUERY *query);

void AddMaterial(SCENE *scene, MATERIAL *material);
void ChangeMaterial(SCENE *scene, MATERIAL *material, int index);

void AddLight(SCENE *scene, LIGHT *light);
int SetLightSourceFile(SCENE *scene, char *filename);
LIGHT *RandomLight(SCENE *scene);

float IsIntersection(SCENE *scene, RAY *ray, int index, float mindist,
                     float *normal, PRIMITIVE **intersected);
int FindBoxIntersection(SCENE *scene, RAY *ray);
CELL *FindFirstCell(SCENE *scene, RAY *ray);
CELL *FindNextCell(SCENE *scene, RAY *ray);
#ifdef __cplusplus
}
#endif

#endif
