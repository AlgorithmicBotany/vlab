/*
 * rotate.c
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
#include "rotate.h"

TransMethods *iRotateMethods;
void RotationMatrix();

/*
 * Create and return reference to Rotate structure.
 */
Rotate *RotateCreate() {
  Rotate *res;

  res = (Rotate *)Malloc(sizeof(Rotate));
  res->x = res->y = res->theta = 0.;
  res->z = 1.;
  return res;
}

/*
 * Return a pointer to collection of methods for the
 * Rotate transformation.
 */
TransMethods *RotateMethods() {
  if (iRotateMethods == (TransMethods *)NULL) {
    iRotateMethods = (TransMethods *)Malloc(sizeof(TransMethods));
    iRotateMethods->create = (TransCreateFunc *)RotateCreate;
    iRotateMethods->propagate = RotatePropagate;
  }
  return iRotateMethods;
}

/*
 * Given a Rotate structure and forward and inverse transformations,
 * propagate the information in the Rotate structure to the
 * transformations.
 */
void RotatePropagate(rotate, trans, itrans) Rotate *rotate;
RSMatrix *trans, *itrans;
{
  RotationMatrix(rotate->x, rotate->y, rotate->z, deg2rad(rotate->theta),
                 trans);
  /*
   * Build the inverse...
   */
  MatrixInvert(trans, itrans);
}

/*
 * Initialize a rotation matrix given an axis of rotation and an
 * angle.  Right-handed rotation is applied.
 */
void RotationMatrix(x, y, z, theta, trans) Float x, y, z, theta;
RSMatrix *trans;
{
  Float n1, n2, n3, sintheta, costheta;
  Vector vector;

  MatrixInit(trans);
  vector.x = x;
  vector.y = y;
  vector.z = z;

  if (VecNormalize(&vector) == 0.)
    RLerror(RL_WARN, "Degenerate rotation axis.\n");

  sintheta = sin(theta);
  costheta = cos(theta);

  n1 = vector.x;
  n2 = vector.y;
  n3 = vector.z;
  trans->matrix[0][0] = (Float)(n1 * n1 + (1. - n1 * n1) * costheta);
  trans->matrix[0][1] = (Float)(n1 * n2 * (1 - costheta) + n3 * sintheta);
  trans->matrix[0][2] = (Float)(n1 * n3 * (1 - costheta) - n2 * sintheta);
  trans->matrix[1][0] = (Float)(n1 * n2 * (1 - costheta) - n3 * sintheta);
  trans->matrix[1][1] = (Float)(n2 * n2 + (1 - n2 * n2) * costheta);
  trans->matrix[1][2] = (Float)(n2 * n3 * (1 - costheta) + n1 * sintheta);
  trans->matrix[2][0] = (Float)(n1 * n3 * (1 - costheta) + n2 * sintheta);
  trans->matrix[2][1] = (Float)(n2 * n3 * (1 - costheta) - n1 * sintheta);
  trans->matrix[2][2] = (Float)(n3 * n3 + (1 - n3 * n3) * costheta);
}
