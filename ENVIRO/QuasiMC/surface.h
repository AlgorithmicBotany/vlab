#ifndef __SURFACE_H__
#define __SURFACE_H__

#define MAX_PATCHES 4
#define MAX_SURFACES 5

typedef struct tagPATCH {
  float points[16][3];
  char name[32]; /* will be used for patch connections */
} PATCH;

typedef struct tagSUBSURFACE {
  float *points;
  int num_pts;
} SUBSURFACE;

typedef struct tagSURFACE {
  PATCH patches[MAX_PATCHES];
  SUBSURFACE subsurfaces[MAX_PATCHES];
  int num_patches, num_subdivides;
  float min[3], max[3];
  float connectionPoint[3], endPoint[3]; /* where turtle starts and ends */
  float headingVector[3], upVector[3];
  float scale;
} SURFACE;

#ifdef __cplusplus
extern "C" {
#endif
void InitSurface(SURFACE *surface);
int LoadSurface(SURFACE *surface, char *filename, int numsubdivides);
int SurfacePatches(SURFACE *surface);
#ifdef __cplusplus
}
#endif

#endif
