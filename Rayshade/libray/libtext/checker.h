/*
 * checker.h
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
#ifndef CHECKER_H

#define TextCheckerCreate(s) TextCreate((TextRef)CheckerCreate(s), CheckerApply)
typedef struct {
  Surface *surf; /* Alternate surface */
} Checker;

extern Checker *CheckerCreate();
void CheckerApply(Checker *blotch, Geom *prim, Ray *ray, Vector *pos,
                  Vector *norm, Vector *gnorm, Surface *surf);

#endif /* CHECKER_H */
