
/* for 3Doutput type */
#define POLYGONS 1
#define TRIANGLES 2

int ReadPrimitiveFile(char *name, int first_time);
void FillGrid(grid_type *grid);
void OutputPrimitives(grid_type *grid);
int IsInsideO(short *index, float *P);
void GetObstacleNormal(short index, float *P, float *norm);
