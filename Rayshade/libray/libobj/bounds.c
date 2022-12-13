/*
 * bounds.c
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

/*
 * Check for intersection between bounding box and the given ray.
 * If there is an intersection between mindist and *maxdist along
 * the ray, *maxdist is replaced with the distance to the point of
 * intersection, and TRUE is returned.  Otherwise, FALSE is returned.
 *
 * If this routine is used to check for intersection with a volume
 * rather than a "hollow" box, one should first determine if
 * (ray->pos + mindist * ray->dir) is inside the bounding volume, and
 * call BoundsIntersect() only if it is not.
 */
int BoundsIntersect(ray, bounds, mindist, maxdist) Ray *ray;
Float bounds[2][3], mindist, *maxdist;
{
  Float t, tmin, tmax;
  Float dir, pos;

  tmax = *maxdist;
  tmin = mindist;

  dir = ray->dir.x;
  pos = ray->pos.x;

  if (dir < 0) {
    t = (bounds[LOW][X] - pos) / dir;
    if (t < tmin)
      return FALSE;
    if (t <= tmax)
      tmax = t;
    t = (bounds[HIGH][X] - pos) / dir;
    if (t >= tmin) {
      if (t > tmax)
        return FALSE;
      tmin = t;
    }
  } else if (dir > 0.) {
    t = (bounds[HIGH][X] - pos) / dir;
    if (t < tmin)
      return FALSE;
    if (t <= tmax)
      tmax = t;
    t = (bounds[LOW][X] - pos) / dir;
    if (t >= tmin) {
      if (t > tmax)
        return FALSE;
      tmin = t;
    }
  } else if (pos < bounds[LOW][X] || pos > bounds[HIGH][X])
    return FALSE;

  dir = ray->dir.y;
  pos = ray->pos.y;

  if (dir < 0) {
    t = (bounds[LOW][Y] - pos) / dir;
    if (t < tmin)
      return FALSE;
    if (t <= tmax)
      tmax = t;
    t = (bounds[HIGH][Y] - pos) / dir;
    if (t >= tmin) {
      if (t > tmax)
        return FALSE;
      tmin = t;
    }
  } else if (dir > 0.) {
    t = (bounds[HIGH][Y] - pos) / dir;
    if (t < tmin)
      return FALSE;
    if (t <= tmax)
      tmax = t;
    t = (bounds[LOW][Y] - pos) / dir;
    if (t >= tmin) {
      if (t > tmax)
        return FALSE;
      tmin = t;
    }
  } else if (pos < bounds[LOW][Y] || pos > bounds[HIGH][Y])
    return FALSE;

  dir = ray->dir.z;
  pos = ray->pos.z;

  if (dir < 0) {
    t = (bounds[LOW][Z] - pos) / dir;
    if (t < tmin)
      return FALSE;
    if (t <= tmax)
      tmax = t;
    t = (bounds[HIGH][Z] - pos) / dir;
    if (t >= tmin) {
      if (t > tmax)
        return FALSE;
      tmin = t;
    }
  } else if (dir > 0.) {
    t = (bounds[HIGH][Z] - pos) / dir;
    if (t < tmin)
      return FALSE;
    if (t <= tmax)
      tmax = t;
    t = (bounds[LOW][Z] - pos) / dir;
    if (t >= tmin) {
      if (t > tmax)
        return FALSE;
      tmin = t;
    }
  } else if (pos < bounds[LOW][Z] || pos > bounds[HIGH][Z])
    return FALSE;

  /*
   * If tmin == mindist, then there was no "near"
   * intersection farther than EPSILON away.
   */
  if (tmin == mindist) {
    if (tmax < *maxdist) {
      *maxdist = tmax;
      return TRUE;
    }
  } else {
    if (tmin < *maxdist) {
      *maxdist = tmin;
      return TRUE;
    }
  }
  return FALSE; /* hit, but not closer than maxdist */
}

/*
 * Transform an object's bounding box by the given transformation
 * matrix.
 */
void BoundsTransform(trans, objbounds) RSMatrix *trans;
Float objbounds[2][3];
{
  Float bounds[2][3], a, b;
  int i, j;

  /*
   * Can't (and shouldn't) do anything with unbounded objects.
   */
  if (objbounds[LOW][X] > objbounds[HIGH][X])
    return;

  bounds[LOW][X] = bounds[HIGH][X] = trans->translate.x;
  bounds[LOW][Y] = bounds[HIGH][Y] = trans->translate.y;
  bounds[LOW][Z] = bounds[HIGH][Z] = trans->translate.z;

  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      a = trans->matrix[j][i] * objbounds[LOW][j];
      b = trans->matrix[j][i] * objbounds[HIGH][j];
      if (a < b) {
        bounds[LOW][i] += a;
        bounds[HIGH][i] += b;
      } else {
        bounds[LOW][i] += b;
        bounds[HIGH][i] += a;
      }
    }
  }
  BoundsCopy(bounds, objbounds);
}

void BoundsInit(bounds) Float bounds[2][3];
{
  bounds[LOW][X] = bounds[LOW][Y] = bounds[LOW][Z] = FAR_AWAY;
  bounds[HIGH][X] = bounds[HIGH][Y] = bounds[HIGH][Z] = -FAR_AWAY;
}

void BoundsCopy(from, into) Float into[2][3], from[2][3];
{
  into[LOW][X] = from[LOW][X];
  into[LOW][Y] = from[LOW][Y];
  into[LOW][Z] = from[LOW][Z];
  into[HIGH][X] = from[HIGH][X];
  into[HIGH][Y] = from[HIGH][Y];
  into[HIGH][Z] = from[HIGH][Z];
}

#define SetIfLess(a, b) (a = ((a) < (b) ? (a) : (b)))
#define SetIfGreater(a, b) (a = ((a) > (b) ? (a) : (b)))

/*
 * Find bounding box of the union of two bounding boxes.
 */
void BoundsEnlarge(old, new) Float old[2][3], new[2][3];
{
  SetIfLess(old[LOW][X], new[LOW][X]);
  SetIfLess(old[LOW][Y], new[LOW][Y]);
  SetIfLess(old[LOW][Z], new[LOW][Z]);
  SetIfGreater(old[HIGH][X], new[HIGH][X]);
  SetIfGreater(old[HIGH][Y], new[HIGH][Y]);
  SetIfGreater(old[HIGH][Z], new[HIGH][Z]);
}

void BoundsPrint(box, fp) Float box[2][3];
FILE *fp;
{
  fprintf(fp, "\tX: %f to %f\n", box[LOW][X], box[HIGH][X]);
  fprintf(fp, "\tY: %f to %f\n", box[LOW][Y], box[HIGH][Y]);
  fprintf(fp, "\tZ: %f to %f\n", box[LOW][Z], box[HIGH][Z]);
}
