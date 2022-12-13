
typedef CELL_TYPE LEAF_TYPE; /* CELL_TYPE is defined in density3d.h */

void FreeOctreeStructures(void);
int InitializeOctree(int level);
CELL_TYPE *GetLeafPt(double *pt);
