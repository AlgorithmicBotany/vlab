/* cell type */
#define DATA 0
#define OBSTACLE 1
#define SOURCE 2
#define CONSUMER 4 /* can be added to any cell using bit OR */

typedef float DATA_TYPE;
#define MAXOBSTACLESINCELL 3

struct CELL_TYPE {
  char flag; /* data/obstacle/source */
  DATA_TYPE val;
  short obstacle[MAXOBSTACLESINCELL];
  /* index of obstacles for exact intersection */
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
  DATA_TYPE min, max; /* in case of image input */
  char *outimagename;
  char *out3Dname;
};
typedef struct grid_type grid_type;

CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z);
void FreeFieldStructures(void);
