/*
 * box.h
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
 */
#ifndef BOX_H
#define BOX_H

#define GeomBoxCreate(a, b) GeomCreate((GeomRef)BoxCreate(a, b), BoxMethods())

/*
 * Box
 */
typedef struct box {
  Float bounds[2][3];
} Box;

extern Box *BoxCreate(Vector *v1, Vector *v2);
extern int BoxIntersect(), BoxNormal();
extern void BoxBounds(), BoxStats();
extern char *BoxName();
extern Methods *BoxMethods();
int BoxEnter(Box *box, Ray *ray, Float mind, Float hitd);

#endif /* BOX_H */
