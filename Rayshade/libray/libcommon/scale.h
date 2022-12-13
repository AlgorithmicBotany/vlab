/*
 * scale.h
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
#ifndef SCALE_H
#define SCALE_H
#include "transform.h"
#define TransScaleCreate() TransCreate((TransRef)ScaleCreate(), ScaleMethods())

#define TransScaleSetX(t, v) TransAssoc(t, &((Scale *)t->tr)->x, v)
#define TransScaleSetY(t, v) TransAssoc(t, &((Scale *)t->tr)->y, v)
#define TransScaleSetZ(t, v) TransAssoc(t, &((Scale *)t->tr)->z, v)

typedef Vector Scale;

extern Scale *ScaleCreate();
extern TransMethods *ScaleMethods();
extern void ScalePropagate();

#endif
