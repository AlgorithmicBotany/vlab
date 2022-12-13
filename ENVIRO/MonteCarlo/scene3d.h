
#define POLYGON 1
#define MATERIAL 6
#define PUSHMATRIX 7
#define POPMATRIX 8
#define TRANSLATE 9
#define ROTATE 10
#define SCALE 11
#define MULTMATRIX 12
#define TRIANGLE 15

void InitializeObjects(void);

float DotProduct(float *vec1, float *vec2);

int ReadPrimitiveFile(char *name, int first_time);
primitive_type *AddObject(grid_type *grid, CTURTLE *tu,
                          Cmodule_type *next_symbol, int *top_mat,
                          int *bottom_mat);
primitive_type *AddTriangle(grid_type *grid, float *vertices,
                            Cmodule_type *next_symbol, int npars, int *top_mat,
                            int *bottom_mat);
void FillGrid(grid_type *grid);
void OutputPrimitives(grid_type *grid);

float IsIntersection(float *pt, float *dir, float mindist, primitive_type *prim,
                     float *norm);
