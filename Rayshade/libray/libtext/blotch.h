/*
 * blotch.h
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
#ifndef BLOTCH_H

#define TextBlotchCreate(m, s)                                                 \
  TextCreate((TextRef)BlotchCreate(m, s), BlotchApply)
typedef struct {
  Float mix;
  Surface *surf;
} Blotch;

extern Blotch *BlotchCreate();
void BlotchApply(Blotch *blotch, Geom *prim, Ray *ray, Vector *pos,
                 Vector *norm, Vector *gnorm, Surface *surf);

#endif /* BLOTCH_H */
