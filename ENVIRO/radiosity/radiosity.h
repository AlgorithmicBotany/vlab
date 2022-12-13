#define MAX_SPECTRUM_SAMPLES 5

struct material_type {
  float reflectance[MAX_SPECTRUM_SAMPLES];
  float spec_power[MAX_SPECTRUM_SAMPLES];
  float transmittance[MAX_SPECTRUM_SAMPLES];
  float trans_power[MAX_SPECTRUM_SAMPLES];
  float Nt[MAX_SPECTRUM_SAMPLES]; /* index of refraction */
};
typedef struct material_type material_type;

struct primitive_type {
  unsigned char flag;
  float intensity[MAX_SPECTRUM_SAMPLES];
  /* intensity received by the primitive */
  float data[12]; /* parameters of the object */
  float normal[3];
  char ci;                    /* index of projection */
  material_type *material[2]; /* top and bottom */
  unsigned long ray_signature;
};
typedef struct primitive_type primitive_type;

/* cell type */
#define EMPTY 0
#define OBSTACLE 1
#define OBJECT 2

/* grid */
struct OBJECT_LIST_TYPE {
  primitive_type *prim;
  struct OBJECT_LIST_TYPE *next;
};
typedef struct OBJECT_LIST_TYPE OBJECT_LIST_TYPE;

struct CELL_TYPE {
  OBJECT_LIST_TYPE *list;
};
typedef struct CELL_TYPE CELL_TYPE;

#define X 0
#define Y 1
#define Z 2

struct grid_type {
  int size[3];       /* size of the grid (in nodes) */
  float range[3];    /* size in coordinates */
  float pos[3];      /* position of lower left front corner */
  float bbox[3 + 3]; /* tight box around objects in the grid */
  float bsph_C[3];   /* center of the bounding sphere */
  float bsph_r;      /* radius of the bounding sphere */
  float maxdist;
  double grid_to_real[3];
  CELL_TYPE *data;
};
typedef struct grid_type grid_type;

CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z);
