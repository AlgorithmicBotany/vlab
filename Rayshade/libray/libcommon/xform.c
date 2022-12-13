/*
 * xform.c
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
#include "xform.h"

TransMethods *iXformMethods;

/*
 * Create and return reference to an Xform structure.
 * In this case, we return a NULL pointer, as the Xform
 * structure does not hold any data.
 */
Xform *XformCreate() { return (Xform *)NULL; }

/*
 * Return a pointer to collection of methods for the
 * Xform transformation.
 */
TransMethods *XformMethods() {
  if (iXformMethods == (TransMethods *)NULL) {
    iXformMethods = (TransMethods *)Malloc(sizeof(TransMethods));
    iXformMethods->create = (TransCreateFunc *)XformCreate;
    iXformMethods->propagate = XformPropagate;
  }
  return iXformMethods;
}

/*
 * Given an Xform structure and forward and inverse transformations,
 * propagate the information in the Xform structure to the
 * transformations.
 * In this case, the information "in" the Xform structure is
 * actually stored in the forward transformation; the Xform
 * points to NULL.
 */
void XformPropagate(__attribute__((unused)) Xform *xform, RSMatrix *trans, RSMatrix *itrans)
{
  /*
   * The Xform methods change the forward trans
   * directly, so it's already all set.
   * Build the inverse...
   */
  MatrixInvert(trans, itrans);
}
