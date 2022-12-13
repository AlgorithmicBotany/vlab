/*
 * hf.c
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
#include "hf.h"

static Methods *iHfMethods = NULL;
static char hfName[] = "heighfield";

static void integrate_grid(), QueueTri();
static int DDA2D(), CheckCell();
static Float intHftri();
static float minalt(), maxalt();

typedef struct {
  int stepX, stepY;
  Float tDX, tDY;
  float minz, maxz;
  int outX, outY;
  Vector cp, pDX, pDY;
} Trav2D;

static hfTri *CreateHfTriangle(), *GetQueuedTri();

unsigned long HFTests, HFHits;

Hf *HfCreate(filename) char *filename;
{
  Hf *hf;
  FILE *fp;
  float val, *maxptr, *minptr;
  int i, j;

  fp = fopen(filename, "r");
  if (fp == (FILE *)NULL) {
    RLerror(RL_ABORT, "Cannot open heightfield file \"%s\".\n", filename);
    return (Hf *)NULL;
  }

  hf = (Hf *)Malloc(sizeof(Hf));
  /*
   * Make the following an option someday.
   */
  hf->BestSize = BESTSIZE;
  /*
   * Store the inverse for faster computation.
   */
  hf->iBestSize = 1. / (float)hf->BestSize;
  /*
   * Get HF size.
   */
  if (fread((char *)&hf->size, sizeof(int), 1, fp) == 0) {
    RLerror(RL_ABORT, "Cannot read height field size.\n");
    return (Hf *)NULL;
  }

  hf->data = (float **)share_malloc(hf->size * sizeof(float *));
  for (i = 0; i < hf->size; i++) {
    hf->data[i] = (float *)share_malloc(hf->size * sizeof(float));
    /*
     * Read in row of HF data.
     */
    if (fread((char *)hf->data[i], sizeof(float), hf->size, fp) !=
        (size_t)hf->size) {
      RLerror(RL_ABORT, "Not enough heightfield data.\n");
      return (Hf *)NULL;
    }
    for (j = 0; j < hf->size; j++) {
      val = hf->data[i][j];
      if (val <= HF_UNSET) {
        hf->data[i][j] = HF_UNSET;
        /*
         * Don't include the point in min/max
         * calculations.
         */
        continue;
      }
      if (val > hf->maxz)
        hf->maxz = val;
      if (val < hf->minz)
        hf->minz = val;
    }
  }
  (void)fclose(fp);
  /*
   * Allocate levels of grid.  hf->levels = log base BestSize of hf->size
   */
  for (i = hf->size, hf->levels = 0; i > hf->BestSize;
       i /= hf->BestSize, hf->levels++)
    ;
  hf->levels++;
  hf->qsize = CACHESIZE;
  hf->q = (hfTri **)Calloc((unsigned)hf->qsize, sizeof(hfTri *));
  hf->qtail = 0;

  hf->lsize = (int *)share_malloc(hf->levels * sizeof(int));
  hf->spacing = (float *)share_malloc(hf->levels * sizeof(float));
  hf->boundsmax = (float ***)share_malloc(hf->levels * sizeof(float **));
  hf->boundsmin = (float ***)share_malloc(hf->levels * sizeof(float **));

  hf->spacing[0] = hf->size - 1;
  hf->lsize[0] = (int)hf->spacing[0];
  hf->boundsmax[0] = (float **)share_malloc(hf->lsize[0] * sizeof(float *));
  hf->boundsmin[0] = (float **)share_malloc(hf->lsize[0] * sizeof(float *));
  /*
   * Compute initial bounding boxes
   */
  for (i = 0; i < hf->lsize[0]; i++) {
    hf->boundsmax[0][i] = (float *)share_malloc(hf->lsize[0] * sizeof(float));
    hf->boundsmin[0][i] = (float *)share_malloc(hf->lsize[0] * sizeof(float));
    maxptr = hf->boundsmax[0][i];
    minptr = hf->boundsmin[0][i];
    for (j = 0; j < hf->lsize[0]; j++) {
      *maxptr++ = maxalt(i, j, hf->data) + EPSILON;
      *minptr++ = minalt(i, j, hf->data) - EPSILON;
    }
  }

  for (i = 1; i < hf->levels; i++) {
    hf->spacing[i] = hf->spacing[i - 1] * hf->iBestSize;
    hf->lsize[i] = (int)hf->spacing[i];
    if ((Float)hf->lsize[i] != hf->spacing[i])
      hf->lsize[i]++;
    hf->boundsmax[i] = (float **)share_malloc(hf->lsize[i] * sizeof(float *));
    hf->boundsmin[i] = (float **)share_malloc(hf->lsize[i] * sizeof(float *));
    for (j = 0; j < hf->lsize[i]; j++) {
      hf->boundsmax[i][j] = (float *)share_malloc(hf->lsize[i] * sizeof(float));
      hf->boundsmin[i][j] = (float *)share_malloc(hf->lsize[i] * sizeof(float));
    }
    integrate_grid(hf, i);
  }

  hf->boundbox[LOW][X] = hf->boundbox[LOW][Y] = 0;
  hf->boundbox[HIGH][X] = hf->boundbox[HIGH][Y] = 1;
  hf->boundbox[LOW][Z] = hf->minz;
  hf->boundbox[HIGH][Z] = hf->maxz;

  return hf;
}

Methods *HfMethods() {
  if (iHfMethods == (Methods *)NULL) {
    iHfMethods = MethodsCreate();
    iHfMethods->create = (GeomCreateFunc *)HfCreate;
    iHfMethods->methods = HfMethods;
    iHfMethods->name = HfName;
    iHfMethods->intersect = HfIntersect;
    iHfMethods->normal = HfNormal;
    iHfMethods->uv = HfUV;
    iHfMethods->bounds = HfBounds;
    iHfMethods->stats = HfStats;
    iHfMethods->checkbounds = TRUE;
    iHfMethods->closed = FALSE;
  }
  return iHfMethods;
}

/*
 * Intersect ray with height field.
 */
int HfIntersect(hf, ray, mindist, maxdist) Hf *hf;
Ray *ray;
Float mindist, *maxdist;
{
  Vector hitpos;
  Float offset;
  Trav2D trav;

  HFTests++;

  /*
   * Find where we hit the hf cube.
   */
  VecAddScaled(ray->pos, mindist, ray->dir, &hitpos);
  if (OutOfBounds(&hitpos, hf->boundbox)) {
    offset = *maxdist;
    if (!BoundsIntersect(ray, hf->boundbox, mindist, &offset))
      return FALSE;
    hitpos.x = ray->pos.x + ray->dir.x * offset;
    hitpos.y = ray->pos.y + ray->dir.y * offset;
    hitpos.z = ray->pos.z + ray->dir.z * offset;
  } else
    hitpos = ray->pos;
  /*
   * Find out in which cell "hitpoint" is.
   */
  if (equal(hitpos.x, 1.))
    hitpos.x -= EPSILON;
  if (equal(hitpos.y, 1.))
    hitpos.y -= EPSILON;

  if (ray->dir.x < 0.) {
    trav.stepX = trav.outX = -1;
    trav.tDX = -1. / (ray->dir.x * hf->spacing[hf->levels - 1]);
  } else if (ray->dir.x > 0.) {
    trav.stepX = 1;
    trav.outX = hf->lsize[hf->levels - 1];
    /*
     * (1./size) / ray
     */
    trav.tDX = 1. / (ray->dir.x * hf->spacing[hf->levels - 1]);
  }

  if (ray->dir.y < 0.) {
    trav.stepY = trav.outY = -1;
    trav.tDY = -1. / (ray->dir.y * hf->spacing[hf->levels - 1]);
  } else if (ray->dir.y > 0.) {
    trav.stepY = 1;
    trav.outY = hf->lsize[hf->levels - 1];
    trav.tDY = 1. / (ray->dir.y * hf->spacing[hf->levels - 1]);
  }

  trav.pDX.x = ray->dir.x * trav.tDX;
  trav.pDX.y = ray->dir.y * trav.tDX;
  trav.pDX.z = ray->dir.z * trav.tDX;
  trav.pDY.x = ray->dir.x * trav.tDY;
  trav.pDY.y = ray->dir.y * trav.tDY;
  trav.pDY.z = ray->dir.z * trav.tDY;

  trav.cp = hitpos;
  trav.minz = hf->minz;
  trav.maxz = hf->maxz;
  if (DDA2D(hf, &ray->pos, &ray->dir, hf->levels - 1, &trav, maxdist)) {
    HFHits++;
    return TRUE;
  }
  return FALSE;
}

/*
 * Traverse the grid using a modified DDA algorithm.  If the extent of
 * the ray over a cell intersects the bounding volume defined by the
 * four corners of the cell, either recurse or perform ray/surface
 * intersection test.
 */
static int DDA2D(hf, pos, ray, level, trav, maxdist) Hf *hf;
Vector *pos, *ray;
int level;
Trav2D *trav;
Float *maxdist;
{
  int x, y, size, posZ;
  float **boundsmin, **boundsmax, spacing;
  Float tX, tY;
  Trav2D newtrav;
  Vector nxp, nyp;

  size = hf->lsize[level];
  spacing = hf->spacing[level];

  posZ = (ray->z > 0.);

  x = trav->cp.x * hf->spacing[level];
  if (x == size)
    x--;
  y = trav->cp.y * hf->spacing[level];
  if (y == size)
    y--;
  boundsmax = hf->boundsmax[level];
  boundsmin = hf->boundsmin[level];

  if (trav->outX > size)
    trav->outX = size;
  if (trav->outY > size)
    trav->outY = size;
  if (trav->outX < 0)
    trav->outX = -1;
  if (trav->outY < 0)
    trav->outY = -1;

  if (ray->x < 0.) {
    tX = (x / spacing - trav->cp.x) / ray->x;
  } else if (ray->x > 0.)
    tX = ((x + 1) / spacing - trav->cp.x) / ray->x;
  else
    tX = FAR_AWAY;
  if (ray->y < 0.) {
    tY = (y / spacing - trav->cp.y) / ray->y;
  } else if (ray->y > 0.)
    tY = ((y + 1) / spacing - trav->cp.y) / ray->y;
  else
    tY = FAR_AWAY;

  nxp.x = trav->cp.x + tX * ray->x;
  nxp.y = trav->cp.y + tX * ray->y;
  nxp.z = trav->cp.z + tX * ray->z;

  nyp.x = trav->cp.x + tY * ray->x;
  nyp.y = trav->cp.y + tY * ray->y;
  nyp.z = trav->cp.z + tY * ray->z;

  do {
    if (tX < tY) {
      if ((posZ && trav->cp.z <= boundsmax[y][x] && nxp.z >= boundsmin[y][x]) ||
          (!posZ && trav->cp.z >= boundsmin[y][x] &&
           nxp.z <= boundsmax[y][x])) {
        if (level) {
          /*
           * Recurse -- compute constants
           * needed for next level.
           * Nicely enough, this just
           * involves a few multiplications.
           */
          newtrav = *trav;
          newtrav.tDX *= hf->iBestSize;
          newtrav.tDY *= hf->iBestSize;
          newtrav.maxz = boundsmax[y][x];
          newtrav.minz = boundsmin[y][x];
          if (ray->x < 0.)
            newtrav.outX = hf->BestSize * x - 1;
          else
            newtrav.outX = hf->BestSize * (x + 1);
          if (ray->y < 0.)
            newtrav.outY = hf->BestSize * y - 1;
          else
            newtrav.outY = hf->BestSize * (y + 1);
          newtrav.pDX.x *= hf->iBestSize;
          newtrav.pDX.y *= hf->iBestSize;
          newtrav.pDX.z *= hf->iBestSize;
          newtrav.pDY.x *= hf->iBestSize;
          newtrav.pDY.y *= hf->iBestSize;
          newtrav.pDY.z *= hf->iBestSize;
          if (DDA2D(hf, pos, ray, level - 1, &newtrav, maxdist))
            return TRUE;
        } else if (CheckCell(x, y, hf, ray, pos, maxdist))
          return TRUE;
      }
      x += trav->stepX; /* Move in X */
      if (*maxdist < tX || x == trav->outX)
        /* If outside, quit */
        return FALSE;
      tX += trav->tDX;      /* Update position on ray */
      trav->cp = nxp;       /* cur pos gets next pos */
      nxp.x += trav->pDX.x; /* Compute next pos */
      nxp.y += trav->pDX.y;
      nxp.z += trav->pDX.z;
    } else {
      if ((posZ && trav->cp.z <= boundsmax[y][x] && nyp.z >= boundsmin[y][x]) ||
          (!posZ && trav->cp.z >= boundsmin[y][x] &&
           nyp.z <= boundsmax[y][x])) {
        if (level) {
          /* Recurse */
          newtrav = *trav;
          newtrav.tDX *= hf->iBestSize;
          newtrav.tDY *= hf->iBestSize;
          newtrav.maxz = boundsmax[y][x];
          newtrav.minz = boundsmin[y][x];
          if (ray->x < 0.)
            newtrav.outX = hf->BestSize * x - 1;
          else
            newtrav.outX = hf->BestSize * (x + 1);
          if (ray->y < 0.)
            newtrav.outY = hf->BestSize * y - 1;
          else
            newtrav.outY = hf->BestSize * (y + 1);
          newtrav.pDX.x *= hf->iBestSize;
          newtrav.pDX.y *= hf->iBestSize;
          newtrav.pDX.z *= hf->iBestSize;
          newtrav.pDY.x *= hf->iBestSize;
          newtrav.pDY.y *= hf->iBestSize;
          newtrav.pDY.z *= hf->iBestSize;
          if (DDA2D(hf, pos, ray, level - 1, &newtrav, maxdist))
            return TRUE;
        } else if (CheckCell(x, y, hf, ray, pos, maxdist))
          return TRUE;
      }
      y += trav->stepY;
      if (*maxdist < tY || y == trav->outY)
        return FALSE;
      tY += trav->tDY;
      trav->cp = nyp;
      nyp.x += trav->pDY.x;
      nyp.y += trav->pDY.y;
      nyp.z += trav->pDY.z;
    }
  } while ((trav->cp.x <= 1. && trav->cp.y <= 1.) &&
           ((posZ && trav->cp.z <= trav->maxz) ||
            (!posZ && trav->cp.z >= trav->minz)));

  /*
   * while ((we're inside the horizontal bounding box)
   *		(usually caught by outX & outY, but
   *		 it's possible to go "too far" due to
   *		 the fact that our levels of grids do
   *		 not "nest" exactly if gridsize%BestSize != 0)
   *	  and
   *	  ((if ray->z is positive and we haven't gone through
   *	   the upper bounding plane) or
   *	  (if ray->z is negative and we haven't gone through
   *	   the lower bounding plane)));
   */

  return FALSE;
}

/*
 * Check for ray/cell intersection
 */
static int CheckCell(x, y, hf, ray, pos, maxdist) int x, y;
Hf *hf;
Vector *ray, *pos;
Float *maxdist;
{
  hfTri *tri1, *tri2;
  Float d1, d2;

  d1 = d2 = FAR_AWAY;

  if ((tri1 = CreateHfTriangle(hf, x, y, x + 1, y, x, y + 1, TRI1)))
    d1 = intHftri(ray, pos, tri1);
  if ((tri2 = CreateHfTriangle(hf, x + 1, y, x + 1, y + 1, x, y + 1, TRI2)))
    d2 = intHftri(ray, pos, tri2);

  if (d1 == FAR_AWAY && d2 == FAR_AWAY)
    return FALSE;

  if (d1 < d2) {
    if (d1 < *maxdist) {
      hf->hittri = *tri1;
      *maxdist = d1;
      return TRUE;
    }
    return FALSE;
  }

  if (d2 < *maxdist) {
    hf->hittri = *tri2;
    *maxdist = d2;
    return TRUE;
  }
  return FALSE;
}

static hfTri *CreateHfTriangle(hf, x1, y1, x2, y2, x3, y3, which) Hf *hf;
int x1, y1, x2, y2, x3, y3, which;
{
  hfTri *tri;
  Float xid, yid;
  Vector tmp1, tmp2;

  /*
   * Don't use triangles with "unset" vertices.
   */
  if (hf->data[y1][x1] == HF_UNSET || hf->data[y2][x2] == HF_UNSET ||
      hf->data[y3][x3] == HF_UNSET)
    return (hfTri *)0;

  xid = (Float)x1 / (Float)(hf->size - 1);
  yid = (Float)y1 / (Float)(hf->size - 1);

  if ((tri = GetQueuedTri(hf, xid, yid, which)) != (hfTri *)0)
    return tri;

  tri = (hfTri *)Malloc(sizeof(hfTri));

  tri->type = which;
  tri->v1.x = xid;
  tri->v1.y = yid;
  tri->v1.z = hf->data[y1][x1];
  tri->v2.x = (Float)x2 / (Float)(hf->size - 1);
  tri->v2.y = (Float)y2 / (Float)(hf->size - 1);
  tri->v2.z = hf->data[y2][x2];
  tri->v3.x = (Float)x3 / (Float)(hf->size - 1);
  tri->v3.y = (Float)y3 / (Float)(hf->size - 1);
  tri->v3.z = hf->data[y3][x3];

  tmp1.x = tri->v2.x - tri->v1.x;
  tmp1.y = tri->v2.y - tri->v1.y;
  tmp1.z = tri->v2.z - tri->v1.z;
  tmp2.x = tri->v3.x - tri->v1.x;
  tmp2.y = tri->v3.y - tri->v1.y;
  tmp2.z = tri->v3.z - tri->v1.z;

  (void)VecNormCross(&tmp1, &tmp2, &tri->norm);

  tri->d = -dotp(&tri->v1, &tri->norm);

  QueueTri(hf, tri);
  return tri;
}

/*
 * Intersect ray with right isoscoles triangle, the hypotenuse of which
 * has slope of -1.
 */
static Float intHftri(ray, pos, tri) hfTri *tri;
Vector *pos, *ray;
{
  Float u, v, dist, xpos, ypos;

  u = dotp(&tri->norm, pos) + tri->d;
  v = dotp(&tri->norm, ray);

  if ((u <= 0. || v > -EPSILON) && (u >= 0. && v < EPSILON))
    return FAR_AWAY;

  dist = -u / v;

  if (dist < EPSILON)
    return FAR_AWAY;

  xpos = pos->x + dist * ray->x;
  ypos = pos->y + dist * ray->y;

  if (tri->type == TRI1 && xpos >= tri->v1.x && ypos >= tri->v1.y &&
      xpos + ypos <= tri->v2.x + tri->v2.y)
    return dist;
  if (tri->type == TRI2 && xpos <= tri->v2.x && ypos <= tri->v2.y &&
      xpos + ypos >= tri->v1.x + tri->v1.y)
    return dist;
  return FAR_AWAY;
}

/*
 * Compute normal to height field.
 */
/*ARGSUSED*/
int HfNormal(Hf *hf, __attribute__((unused)) Vector *pos, Vector *nrm,
             Vector *gnrm) {
  *gnrm = *nrm = hf->hittri.norm;
  return FALSE;
}

/*ARGSUSED*/
void HfUV(__attribute__((unused)) Hf *hf, Vector *pos,
          __attribute__((unused)) Vector *norm, Vec2d *uv, Vector *dpdu,
          Vector *dpdv)

{
  uv->u = pos->x;
  uv->v = pos->y;
  if (dpdu) {
    dpdu->x = 1.;
    dpdu->y = dpdv->z = 0.;
    dpdv->x = dpdv->z = 0.;
    dpdv->y = 1.;
  }
}

/*
 * Compute heightfield bounding box.
 */
void HfBounds(hf, bounds) Hf *hf;
Float bounds[2][3];
{
  /*
   * By default, height fields are centered at (0.5, 0.5, 0.)
   */
  bounds[LOW][X] = bounds[LOW][Y] = 0;
  bounds[HIGH][X] = bounds[HIGH][Y] = 1;
  bounds[LOW][Z] = hf->minz;
  bounds[HIGH][Z] = hf->maxz;
}

/*
 * Build min/max altitude value arrays for the given grid level.
 */
static void integrate_grid(hf, level) Hf *hf;
int level;
{
  int i, j, k, l, ii, ji;
  float max_alt, min_alt;
  float **maxinto, **mininto, **frommax, **frommin, *minptr, *maxptr;
  int insize, fromsize, fact;

  maxinto = hf->boundsmax[level];
  mininto = hf->boundsmin[level];
  insize = hf->lsize[level];
  frommax = hf->boundsmax[level - 1];
  frommin = hf->boundsmin[level - 1];
  fact = hf->BestSize;
  fromsize = hf->lsize[level - 1];

  ii = 0;

  for (i = 0; i < insize; i++) {
    ji = 0;
    for (j = 0; j < insize; j++) {
      max_alt = HF_UNSET;
      min_alt = -HF_UNSET;
      for (k = 0; k <= fact; k++) {
        if (ii + k >= fromsize)
          continue;
        maxptr = &frommax[ii + k][ji];
        minptr = &frommin[ii + k][ji];
        for (l = 0; l <= fact; l++, maxptr++, minptr++) {
          if (ji + l >= fromsize)
            continue;
          if (*maxptr > max_alt)
            max_alt = *maxptr;
          if (*minptr < min_alt)
            min_alt = *minptr;
        }
      }
      maxinto[i][j] = max_alt + EPSILON;
      mininto[i][j] = min_alt - EPSILON;
      ji += fact;
    }
    ii += fact;
  }
}

/*
 * Place the given triangle in the triangle cache.
 */
static void QueueTri(hf, tri) Hf *hf;
hfTri *tri;
{
  if (hf->q[hf->qtail]) /* Free old triangle data */
    free((voidstar)hf->q[hf->qtail]);
  hf->q[hf->qtail] = tri;                  /* Put on tail */
  hf->qtail = (hf->qtail + 1) % hf->qsize; /* Increment tail */
}

/*
 * Search list of cached trianges to see if this triangle has been
 * cached.  If so, return a pointer to it.  If not, return null pointer.
 */
static hfTri *GetQueuedTri(hf, x, y, flag) Hf *hf;
Float x, y;
int flag;
{
  register int i;
  register hfTri **tmp;

  for (i = 0, tmp = hf->q; i < hf->qsize; i++, tmp++) {
    if (*tmp && (*tmp)->v1.x == x && (*tmp)->v1.y == y && (*tmp)->type == flag)
      return *tmp; /* vertices & flag match, return it */
  }

  return (hfTri *)0;
}

/*
 * Return maximum height of cell indexed by y,x.  This could be done
 * as a macro, but many C compliers will choke on it.
 */
static float minalt(y, x, data) int x, y;
float **data;
{
  float min_alt;

  min_alt = min(data[y][x], data[y + 1][x]);
  min_alt = min(min_alt, data[y][x + 1]);
  min_alt = min(min_alt, data[y + 1][x + 1]);
  return min_alt;
}

/*
 * Return maximum cell height, as above.
 */
static float maxalt(y, x, data) int x, y;
float **data;
{
  float max_alt;

  max_alt = max(data[y][x], data[y + 1][x]);
  max_alt = max(max_alt, data[y][x + 1]);
  max_alt = max(max_alt, data[y + 1][x + 1]);
  return max_alt;
}

char *HfName() { return hfName; }

void HfStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = HFTests;
  *hits = HFHits;
}

void HfMethodRegister(meth) UserMethodType meth;
{
  if (iHfMethods)
    iHfMethods->user = meth;
}
