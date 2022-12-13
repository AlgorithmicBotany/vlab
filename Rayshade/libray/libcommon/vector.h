/*
 * vector.h
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
#ifndef VECTOR_H
#define VECTOR_H
/*
 * Constants used in projecting onto planes
 */
#define XNORMAL (char)0
#define YNORMAL (char)1
#define ZNORMAL (char)2

/*
 * Maximum length
 */
#define FAR_AWAY 1.0E+14

typedef struct {
  Float u, v; /* 2D point */
} Vec2d;

typedef struct Vector {
  Float x, y, z; /* 3D point */
} Vector;

/*
 * Linked list of points
 */
typedef struct PointList {
  Vector vec;             /* Vector data */
  struct PointList *next; /* Next in list */
} PointList;

/*
 * Project a point in 3-space to the plane whose normal is indicated by "i."
 */
#define VecProject(r, p, i)                                                    \
  {                                                                            \
    switch (i) {                                                               \
    case XNORMAL:                                                              \
      r.u = (p).y;                                                             \
      r.v = (p).z;                                                             \
      break;                                                                   \
    case YNORMAL:                                                              \
      r.u = (p).x;                                                             \
      r.v = (p).z;                                                             \
      break;                                                                   \
    case ZNORMAL:                                                              \
      r.u = (p).x;                                                             \
      r.v = (p).y;                                                             \
      break;                                                                   \
    }                                                                          \
  }

#define dotp(a, b) (((a)->x * (b)->x) + ((a)->y * (b)->y) + ((a)->z * (b)->z))
#define VecSub(a, b, r)                                                        \
  (r)->x = (a).x - (b).x, (r)->y = (a).y - (b).y, (r)->z = (a).z - (b).z
#define VecAdd(a, b, r)                                                        \
  (r)->x = (a).x + (b).x, (r)->y = (a).y + (b).y, (r)->z = (a).z + (b).z
#define VecScale(s, a, r)                                                      \
  (r)->x = (s) * (a).x, (r)->y = (s) * (a).y, (r)->z = (s) * (a).z
#define VecComb(s1, v1, s2, v2, r)                                             \
  (r)->x = (s1) * (v1).x + (s2) * (v2).x,                                      \
  (r)->y = (s1) * (v1).y + (s2) * (v2).y,                                      \
  (r)->z = (s1) * (v1).z + (s2) * (v2).z
#define VecAddScaled(v1, s, v2, r)                                             \
  (r)->x = (v1).x + (s) * (v2).x, (r)->y = (v1).y + (s) * (v2).y,              \
  (r)->z = (v1).z + (s) * (v2).z

extern void VecCross(), VecCoordSys(), MakeBump();
extern Float VecNormCross(), VecNormalize();
extern int Refract();

#endif /* VECTOR_H */
