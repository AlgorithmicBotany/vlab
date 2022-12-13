/*
 * scale.c
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
#include "common.h"
#include "scale.h"

TransMethods *iScaleMethods;
void ScaleMatrix();

Scale *ScaleCreate() {
  Scale *res;

  res = (Scale *)Malloc(sizeof(Scale));
  res->x = res->y = res->z = 1.;
  return res;
}

TransMethods *ScaleMethods() {
  if (iScaleMethods == (TransMethods *)NULL) {
    iScaleMethods = (TransMethods *)Malloc(sizeof(TransMethods));
    iScaleMethods->create = (TransCreateFunc *)ScaleCreate;
    iScaleMethods->propagate = ScalePropagate;
  }
  return iScaleMethods;
}

void ScalePropagate(scale, trans, itrans) Scale *scale;
RSMatrix *trans, *itrans;
{
  if (equal(scale->x, 0.) || equal(scale->y, 0.) || equal(scale->z, 0.))
    RLerror(RL_PANIC, "Degenerate scale %g %g %g\n", scale->x, scale->y,
            scale->z);
  ScaleMatrix(scale->x, scale->y, scale->z, trans);
  /*
   * Build the inverse
   */
  MatrixInit(itrans);
  itrans->matrix[0][0] = 1. / scale->x;
  itrans->matrix[1][1] = 1. / scale->y;
  itrans->matrix[2][2] = 1. / scale->z;
}

void ScaleMatrix(x, y, z, mat) Float x, y, z;
RSMatrix *mat;
{
  MatrixInit(mat);
  mat->matrix[0][0] = x;
  mat->matrix[1][1] = y;
  mat->matrix[2][2] = z;
}
