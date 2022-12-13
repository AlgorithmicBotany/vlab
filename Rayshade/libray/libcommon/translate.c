/*
 * translate.c
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
#include "translate.h"

TransMethods *iTranslateMethods;
void TranslationMatrix();

Translate *TranslateCreate() {
  Translate *res;

  res = (Translate *)Malloc(sizeof(Translate));
  res->x = res->y = res->z = 0.;
  return res;
}

TransMethods *TranslateMethods() {
  if (iTranslateMethods == (TransMethods *)NULL) {
    iTranslateMethods = (TransMethods *)Malloc(sizeof(TransMethods));
    iTranslateMethods->create = (TransCreateFunc *)TranslateCreate;
    iTranslateMethods->propagate = TranslatePropagate;
  }
  return iTranslateMethods;
}

void TranslatePropagate(translate, trans, itrans) Translate *translate;
RSMatrix *trans, *itrans;
{
  TranslationMatrix(translate->x, translate->y, translate->z, trans);
  /*
   * Build the inverse...
   */
  MatrixInit(itrans);
  itrans->translate.x = -translate->x;
  itrans->translate.y = -translate->y;
  itrans->translate.z = -translate->z;
}

void TranslationMatrix(x, y, z, mat) Float x, y, z;
RSMatrix *mat;
{
  MatrixInit(mat);
  mat->translate.x = x;
  mat->translate.y = y;
  mat->translate.z = z;
}
