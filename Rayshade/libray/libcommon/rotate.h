/*
 * rotate.h
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
#ifndef ROTATE_H
#define ROTATE_H

#define TransRotateCreate()                                                    \
  TransCreate((TransRef)RotateCreate(), RotateMethods())

#define TransRotateSetX(t, v) TransAssoc(t, &((Rotate *)t->tr)->x, v)
#define TransRotateSetY(t, v) TransAssoc(t, &((Rotate *)t->tr)->y, v)
#define TransRotateSetZ(t, v) TransAssoc(t, &((Rotate *)t->tr)->z, v)
#define TransRotateSetTheta(t, v) TransAssoc(t, &((Rotate *)t->tr)->theta, v)

typedef struct {
  Float x, y, z, theta;
} Rotate;

extern Rotate *RotateCreate();
extern TransMethods *RotateMethods();
extern void RotatePropagate();

#endif /* ROTATE_H */
