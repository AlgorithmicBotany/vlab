/*
 * cloud.h
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
#ifndef CLOUDTEXT_H

#define TextCloudCreate(s, h, l, n, c, o, t)                                   \
  TextCreate((TextRef)CloudTextCreate(s, h, l, n, c, o, t), CloudTextApply)
typedef struct {
  Float beta, omega, lambda, scale, cthresh, range, transcale, maxval;
  int octaves;
} CloudText;

extern CloudText *CloudTextCreate();
void CloudTextApply(CloudText *cloud, Geom *prim, Ray *ray, Vector *pos,
                    Vector *norm, Vector *gnorm, Surface *surf);

#endif /* CLOUDTEXT_H */
