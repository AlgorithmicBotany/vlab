struct primitive_type {
  int flag;
  int id;              /* to prevent testing of an object with itself */
  float data[9];       /* parameters of the object */
  float matrix[16];    /* to convert point into primitive coordinates */
  float invmatrix[16]; /* to convert normals into grid coordinates */
  float bbox[3 + 3];   /* bounding box (coordinates of left lower front
                          corner plus lenghts along axes x,y, and z in
                          world coordinates - coordinates of the grid) */
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
  int size[3];    /* size of the grid (in nodes) */
  float range[3]; /* size in coordinates */
  float pos[3];   /* position of lower left front corner */
  CELL_TYPE *data;
};
typedef struct grid_type grid_type;

CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z);
