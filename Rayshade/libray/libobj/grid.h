/*
 * grid.h
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb
 * All rights reserved.
 *
 * This software may be freely copied, modified, and redistributed
 * provided that this copyright notice is preserved on all copies.
 *
 * You may not distribute this software, in whole or in part, as part of
 * any commercial product without the express consent of the authors.
 *
 * There is no warranty or other guarantee of fitness of this software
 * for any purpose.  It is provided solely "as is".
 *
 *
 */
#ifndef GRID_H
#define GRID_H
#include "intersect.h"
#define GeomGridCreate(x, y, z)                                                \
  GeomCreate((GeomRef)GridCreate(x, y, z), GridMethods())
/*
 * Convert from voxel number along X/Y/Z to corresponding coordinate.
 */
#define voxel2x(g, x) ((x)*g->voxsize[0] + g->bounds[0][0])
#define voxel2y(g, y) ((y)*g->voxsize[1] + g->bounds[0][1])
#define voxel2z(g, z) ((z)*g->voxsize[2] + g->bounds[0][2])
/*
 * And vice-versa.
 */
#define x2voxel(g, x) (((x)-g->bounds[0][0]) / g->voxsize[0])
#define y2voxel(g, y) (((y)-g->bounds[0][1]) / g->voxsize[1])
#define z2voxel(g, z) (((z)-g->bounds[0][2]) / g->voxsize[2])

/*
 * Grid object
 */
typedef struct {
  short xsize, ysize, zsize; /* # of voxels along each axis */
  Float bounds[2][3];        /* bounding box */
  Float voxsize[3];          /* size of a voxel */
  struct Geom *unbounded,    /* unbounded objects */
      *objects;              /* all bounded objects */
  struct GeomList ****cells; /* Voxels */
} Grid;

extern char *GridName();
extern void *GirdBounds();
extern int GridIntersect(), GridConvert();
extern Grid *GridCreate(int x, int y, int z);
extern Methods *GridMethods();

#endif /* GRID_H */
