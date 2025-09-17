/*
 * infinite.h
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
#ifndef INFINITE_H
#define INFINITE_H

#define LightInfiniteCreate(c, d)                                              \
  LightCreate((LightRef)InfiniteCreate(d), InfiniteMethods(), c)
typedef struct {
  Vector dir;
} Infinite;

extern Infinite *InfiniteCreate(Vector *dir);
extern LightMethods *InfiniteMethods();
int InfiniteIntens(Infinite *inf, Color *lcolor, ShadowCache *cache, Ray *ray,
                   Float dist, int noshadow, Color *color);
void InfiniteDirection(Infinite *lp, Vector *pos, Vector *dir, Float *dist);
void InfiniteMethodRegister(UserMethodType meth);

#endif /* INFINITE_H */
