/*
 * disc.h
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
#ifndef DISC_H
#define DISC_H

#define GeomDiscCreate(ro, ri, p, n)                                           \
  GeomCreate((GeomRef)DiscCreate(ro, ri, p, n), DiscMethods())

typedef struct disc {
  Float outer_radius, inner_radius, d; /* Radius squared, plane constant */
  Vector norm, pos;                    /* Normal, center */
} Disc;

extern Disc *DiscCreate();
extern Methods *DiscMethods();
extern int DiscIntersect(), DiscEnter();
extern void DiscBounds(), DiscUV(), DiscStats();
extern char *DiscName();
int DiscNormal(Disc *disc, __attribute__((unused)) Vector *pos, Vector *nrm,
               Vector *gnrm);
#endif /* DISC_H */
