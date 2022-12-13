
#define POLYGON 1
#define MESH 2
#define PRISM 3
#define CONE 4
#define SPHERE 5
#define MATERIAL 6
#define PUSHMATRIX 7
#define POPMATRIX 8
#define TRANSLATE 9
#define ROTATE 10
#define SCALE 11
#define MULTMATRIX 12
#define RECTANGLE 13
#define CYLINDER 14

void InitializeObjects(void);

int ReadPrimitiveFile(char *name, int first_time);
int AddObject(grid_type *grid, CTURTLE *tu, Cmodule_type *next_symbol);
void FillGrid(grid_type *grid);
void OutputPrimitives(grid_type *grid);

void DetermineBoundingBox(primitive_type *prim);
int IsInObject(float *pt, float surf_dist, primitive_type *prim, float *norm);
float IsIntersection(float *pt, float *dir, float mindist, float surface_dist,
                     primitive_type *prim, float *norm);
