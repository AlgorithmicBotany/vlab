/*
 * grid.c
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
#include "geom.h"
#include "grid.h"

static Methods *iGridMethods = NULL;
static char gridName[] = "grid";

static unsigned long raynumber = 1; /* Current "ray number". */
                                    /* (should be "grid number") */
static void engrid(), GridFreeVoxels();
static int pos2grid(), CheckVoxel();

Grid *GridCreate(x, y, z) int x, y, z;
{
  Grid *grid;

  if (x < 1 || y < 1 || z < 1) {
    RLerror(RL_WARN, "Invalid grid specification.\n");
    return (Grid *)NULL;
  }
  grid = (Grid *)share_calloc(1, sizeof(Grid));
  grid->xsize = x;
  grid->ysize = y;
  grid->zsize = z;
  return grid;
}

char *GridName() { return gridName; }

/*
 * Intersect ray with grid, returning distance from "pos" to
 * nearest intersection with an object in the grid.  Returns 0.
 * if no intersection.
 */
int GridIntersect(grid, ray, hitlist, mindist, maxdist) Grid *grid;
Ray *ray;
HitList *hitlist;
Float mindist, *maxdist;
{
  GeomList *list;
  Geom *obj;
  int hit;
  Float offset, tMaxX, tMaxY, tMaxZ;
  Float tDeltaX, tDeltaY, tDeltaZ, *raybounds[2][3];
  int stepX, stepY, stepZ, outX, outY, outZ, x, y, z;
  Vector curpos, nXp, nYp, nZp, np, pDeltaX, pDeltaY, pDeltaZ;
  unsigned long counter;

  hit = FALSE;
  /*
   * Check unbounded objects.
   */
  for (obj = grid->unbounded; obj; obj = obj->next) {
    if (intersect(obj, ray, hitlist, mindist, maxdist))
      hit = TRUE;
  }

  /*
   * If outside of the bounding box, check to see if we
   * hit it.
   */
  VecAddScaled(ray->pos, mindist, ray->dir, &curpos);
  if (OutOfBounds(&curpos, grid->bounds)) {
    offset = *maxdist;
    if (!BoundsIntersect(ray, grid->bounds, mindist, &offset))
      /*
       * Ray never hit grid space.
       */
      return hit;
    /*
     * else
     *	The ray enters voxel space before it hits
     * 	an unbounded object.
     */
    VecAddScaled(ray->pos, offset, ray->dir, &curpos);
  } else
    offset = mindist;

  counter = raynumber++;

  /*
   * tMaxX is the absolute distance from the ray origin we must move
   *		to get to the next voxel in the X
   *		direction.  It is incrementally updated
   * 		by DDA as we move from voxel-to-voxel.
   * tDeltaX is the relative amount along the ray we move to
   *		get to the next voxel in the X direction. Thus,
   *		when we decide to move in the X direction,
   * 		we increment tMaxX by tDeltaX.
   */
  x = x2voxel(grid, curpos.x);
  if (x == grid->xsize)
    x--;
  if (fabs(ray->dir.x) < EPSILON) {
    tMaxX = FAR_AWAY;
    raybounds[LOW][X] = &curpos.x;
    raybounds[HIGH][X] = &np.x;
    tDeltaX = 0.;
  } else if (ray->dir.x < 0.) {
    tMaxX = offset + (voxel2x(grid, x) - curpos.x) / ray->dir.x;
    tDeltaX = grid->voxsize[X] / -ray->dir.x;
    stepX = outX = -1;
    raybounds[LOW][X] = &np.x;
    raybounds[HIGH][X] = &curpos.x;
  } else {
    tMaxX = offset + (voxel2x(grid, x + 1) - curpos.x) / ray->dir.x;
    tDeltaX = grid->voxsize[X] / ray->dir.x;
    stepX = 1;
    outX = grid->xsize;
    raybounds[LOW][X] = &curpos.x;
    raybounds[HIGH][X] = &np.x;
  }

  y = y2voxel(grid, curpos.y);
  if (y == grid->ysize)
    y--;

  if (fabs(ray->dir.y) < EPSILON) {
    tMaxY = FAR_AWAY;
    raybounds[LOW][Y] = &curpos.y;
    raybounds[HIGH][Y] = &np.y;
    tDeltaY = 0.;
  } else if (ray->dir.y < 0.) {
    tMaxY = offset + (voxel2y(grid, y) - curpos.y) / ray->dir.y;
    tDeltaY = grid->voxsize[Y] / -ray->dir.y;
    stepY = outY = -1;
    raybounds[LOW][Y] = &np.y;
    raybounds[HIGH][Y] = &curpos.y;
  } else {
    tMaxY = offset + (voxel2y(grid, y + 1) - curpos.y) / ray->dir.y;
    tDeltaY = grid->voxsize[Y] / ray->dir.y;
    stepY = 1;
    outY = grid->ysize;
    raybounds[LOW][Y] = &curpos.y;
    raybounds[HIGH][Y] = &np.y;
  }

  z = z2voxel(grid, curpos.z);
  if (z == grid->zsize)
    z--;
  if (fabs(ray->dir.z) < EPSILON) {
    tMaxZ = FAR_AWAY;
    raybounds[LOW][Z] = &curpos.z;
    raybounds[HIGH][Z] = &np.z;
    tDeltaZ = 0.;
  } else if (ray->dir.z < 0.) {
    tMaxZ = offset + (voxel2z(grid, z) - curpos.z) / ray->dir.z;
    tDeltaZ = grid->voxsize[Z] / -ray->dir.z;
    stepZ = outZ = -1;
    raybounds[LOW][Z] = &np.z;
    raybounds[HIGH][Z] = &curpos.z;
  } else {
    tMaxZ = offset + (voxel2z(grid, z + 1) - curpos.z) / ray->dir.z;
    tDeltaZ = grid->voxsize[Z] / ray->dir.z;
    stepZ = 1;
    outZ = grid->zsize;
    raybounds[LOW][Z] = &curpos.z;
    raybounds[HIGH][Z] = &np.z;
  }

  VecScale(tDeltaX, ray->dir, &pDeltaX);
  VecScale(tDeltaY, ray->dir, &pDeltaY);
  VecScale(tDeltaZ, ray->dir, &pDeltaZ);

  VecAddScaled(ray->pos, tMaxX, ray->dir, &nXp);
  VecAddScaled(ray->pos, tMaxY, ray->dir, &nYp);
  VecAddScaled(ray->pos, tMaxZ, ray->dir, &nZp);

  while (TRUE) {
    list = grid->cells[x][y][z];
    if (tMaxX < tMaxY && tMaxX < tMaxZ) {
      if (list) {
        np = nXp;
        if (CheckVoxel(list, ray, raybounds, hitlist, counter, offset, maxdist))
          hit = TRUE;
      }
      x += stepX;
      if (*maxdist < tMaxX || x == outX)
        break;
      tMaxX += tDeltaX;
      curpos = nXp;
      nXp.x += pDeltaX.x;
      nXp.y += pDeltaX.y;
      nXp.z += pDeltaX.z;
    } else if (tMaxZ < tMaxY) {
      if (list) {
        np = nZp;
        if (CheckVoxel(list, ray, raybounds, hitlist, counter, offset, maxdist))
          hit = TRUE;
      }
      z += stepZ;
      if (*maxdist < tMaxZ || z == outZ)
        break;
      tMaxZ += tDeltaZ;
      curpos = nZp;
      nZp.x += pDeltaZ.x;
      nZp.y += pDeltaZ.y;
      nZp.z += pDeltaZ.z;
    } else {
      if (list) {
        np = nYp;
        if (CheckVoxel(list, ray, raybounds, hitlist, counter, offset, maxdist))
          hit = TRUE;
      }
      y += stepY;
      if (*maxdist < tMaxY || y == outY)
        break;
      tMaxY += tDeltaY;
      curpos = nYp;
      nYp.x += pDeltaY.x;
      nYp.y += pDeltaY.y;
      nYp.z += pDeltaY.z;
    }
  }
  return hit;
}

/*
 * Intersect ray with objects in grid cell.  Note that there are a many ways
 * to speed up this routine, all of which uglify the code to a large extent.
 */
static int CheckVoxel(list, ray, raybounds, hitlist, counter, mindist,
                      maxdist) GeomList *list;
Ray *ray;
Float *raybounds[2][3];
HitList *hitlist;
unsigned long counter;
Float mindist, *maxdist;
{
  Geom *obj;
  int hit;
  Float lx, hx, ly, hy, lz, hz;

  lx = *raybounds[LOW][X];
  hx = *raybounds[HIGH][X];
  ly = *raybounds[LOW][Y];
  hy = *raybounds[HIGH][Y];
  lz = *raybounds[LOW][Z];
  hz = *raybounds[HIGH][Z];

  hit = FALSE;

  do {
    obj = list->obj;
    /*
     * If object's counter is greater than or equal to the
     * number associated with the current grid,
     * don't bother checking again.  In addition, if the
     * bounding box of the ray's extent in the voxel does
     * not intersect the bounding box of the object, don't bother.
     */
#ifdef SHAREDMEM
    if (*obj->counter < counter &&
#else
    if (obj->counter < counter &&
#endif
        obj->bounds[LOW][X] <= hx && obj->bounds[HIGH][X] >= lx &&
        obj->bounds[LOW][Y] <= hy && obj->bounds[HIGH][Y] >= ly &&
        obj->bounds[LOW][Z] <= hz && obj->bounds[HIGH][Z] >= lz) {
#ifdef SHAREDMEM
      *obj->counter = counter;
#else
      obj->counter = counter;
#endif
      if (intersect(obj, ray, hitlist, mindist, maxdist))
        hit = TRUE;
    }
  } while ((list = list->next) != (GeomList *)0);

  return hit;
}

int GridConvert(grid, objlist) Grid *grid;
Geom *objlist;
{
  int num;

  /*
   * Keep linked list of all bounded objects in grid; it may come
   * in handy.
   */
  grid->objects = objlist;
  for (num = 0; objlist; objlist = objlist->next)
    num += objlist->prims;

  return num;
}

void GridBounds(grid, bounds) Grid *grid;
Float bounds[2][3];
{
  Geom *ltmp;
  int x, y;

  BoundsInit(bounds);
  /*
   * For each object on the list,
   * compute its bounds...
   */
  /*
   * Find bounding box of bounded objects and get list of
   * unbounded objects.
   */
  grid->unbounded =
      GeomComputeAggregateBounds(&grid->objects, grid->unbounded, grid->bounds);

  BoundsCopy(grid->bounds, bounds);

  grid->voxsize[X] =
      (grid->bounds[HIGH][X] - grid->bounds[LOW][X]) / grid->xsize;
  grid->voxsize[Y] =
      (grid->bounds[HIGH][Y] - grid->bounds[LOW][Y]) / grid->ysize;
  grid->voxsize[Z] =
      (grid->bounds[HIGH][Z] - grid->bounds[LOW][Z]) / grid->zsize;

  if (grid->cells == (GeomList ****)NULL) {
    /*
     * Allocate voxels.
     */
    grid->cells =
        (GeomList ****)share_malloc(grid->xsize * sizeof(GeomList ***));
    for (x = 0; x < grid->xsize; x++) {
      grid->cells[x] =
          (GeomList ***)share_malloc(grid->ysize * sizeof(GeomList **));
      for (y = 0; y < grid->ysize; y++)
        grid->cells[x][y] = (GeomList **)share_calloc((unsigned)grid->zsize,
                                                      sizeof(GeomList *));
    }
  } else {
    /*
     * New frame...
     * Free up the objlists in each voxel.
     */
    GridFreeVoxels(grid);
  }

  /*
   * objlist now holds a linked list of bounded objects.
   */
  for (ltmp = grid->objects; ltmp != (Geom *)0; ltmp = ltmp->next)
    engrid(ltmp, grid);
}

static void GridFreeVoxels(grid) Grid *grid;
{
  int x, y, z;
  GeomList *cell, *next;

  for (x = 0; x < grid->xsize; x++) {
    for (y = 0; y < grid->ysize; y++) {
      for (z = 0; z < grid->zsize; z++) {
        for (cell = grid->cells[x][y][z]; cell; cell = next) {
          next = cell->next;
          free((voidstar)cell);
        }
        grid->cells[x][y][z] = (GeomList *)NULL;
      }
    }
  }
}

Methods *GridMethods() {
  if (iGridMethods == (Methods *)NULL) {
    iGridMethods = MethodsCreate();
    iGridMethods->methods = GridMethods;
    iGridMethods->create = (GeomCreateFunc *)GridCreate;
    iGridMethods->intersect = GridIntersect;
    iGridMethods->name = GridName;
    iGridMethods->convert = GridConvert;
    iGridMethods->bounds = GridBounds;
    iGridMethods->checkbounds = FALSE;
    iGridMethods->closed = TRUE;
  }
  return iGridMethods;
}

/*
 * Place an object in a grid.
 */
static void engrid(obj, grid) Geom *obj;
Grid *grid;
{
  int x, y, z, low[3], high[3];
  GeomList *ltmp;

  /*
   * This routine should *never* be passed an unbounded object, but...
   */
  if (!pos2grid(grid, obj->bounds[LOW], low) ||
      !pos2grid(grid, obj->bounds[HIGH], high) ||
      obj->bounds[LOW][X] > obj->bounds[HIGH][X]) {
    /*
     * Geom is partially on wholly outside of
     * grid -- this should never happen, but just
     * in case...
     */
    RLerror(RL_ABORT, "Engrid got an unbounded object?!\n");
    return;
  }

  /*
   * For each voxel that intersects the object's bounding
   * box, add pointer to this object to voxel's linked list.
   */
  for (x = low[X]; x <= high[X]; x++) {
    for (y = low[Y]; y <= high[Y]; y++) {
      for (z = low[Z]; z <= high[Z]; z++) {
        ltmp = (GeomList *)share_malloc(sizeof(GeomList));
        ltmp->obj = obj;
        ltmp->next = grid->cells[x][y][z];
        grid->cells[x][y][z] = ltmp;
      }
    }
  }
}

/*
 * Convert 3D point to index into grid's voxels.
 */
static int pos2grid(grid, pos, index) Grid *grid;
Float pos[3];
int index[3];
{
  index[X] = (int)(x2voxel(grid, pos[0]));
  index[Y] = (int)(y2voxel(grid, pos[1]));
  index[Z] = (int)(z2voxel(grid, pos[2]));

  if (index[X] == grid->xsize)
    index[X]--;
  if (index[Y] == grid->ysize)
    index[Y]--;
  if (index[Z] == grid->zsize)
    index[Z]--;

  if (index[X] < 0 || index[X] >= grid->xsize || index[Y] < 0 ||
      index[Y] >= grid->ysize || index[Z] < 0 || index[Z] >= grid->zsize)
    return FALSE;
  return TRUE;
}

void GridMethodRegister(meth) UserMethodType meth;
{
  if (iGridMethods)
    iGridMethods->user = meth;
}
