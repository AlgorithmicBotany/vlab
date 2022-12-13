/*
 * transform.c
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
#include "transform.h"
/*
 * Matrices are indexed row-first; that is:
 * matrix[ROW][COLUMN]
 */
/*
 * Allocate new structure that holds both object-to-world and
 * world-to-object space transformation structures.  It probably
 * should hold pointers to these structures.
 */
Trans *TransCreate(tr, meth) TransRef tr;
TransMethods *meth;
{
  Trans *res;

  res = (Trans *)share_malloc(sizeof(Trans));
  res->tr = tr;
  res->methods = meth;
  res->animated = FALSE;
  res->assoc = (ExprAssoc *)NULL;
  res->prev = res->next = (Trans *)NULL;
  MatrixInit(&res->trans);
  MatrixInit(&res->itrans);
  return res;
}

void TransFree(trans) Trans *trans;
{
  if (trans->tr)
    free((voidstar)trans->tr);
  free((voidstar)trans);
}

void TransAssoc(trans, ptr, expr) Trans *trans;
Float *ptr;
Expr *expr;
{
  if (expr->timevary) {
    /*
     * Gotta store the sucker.
     */
    trans->assoc = AssocCreate(ptr, expr, trans->assoc);
    trans->animated = TRUE;
  } else {
    *ptr = expr->value;
  }
  fflush(stderr);
}

/*
 * Allocate new transformation 'matrix'.
 */
RSMatrix *MatrixCreate() {
  RSMatrix *res;

  res = (RSMatrix *)share_malloc(sizeof(RSMatrix));
  MatrixInit(res);
  return res;
}

/*
 * Multiply m1 and m2, copy result into "res".
 */
void MatrixMult(t1, t2, res) RSMatrix *t1, *t2, *res;
{
  register int i;
  RSMatrix tmp;

  for (i = 0; i < 3; i++) {
    tmp.matrix[i][0] = t1->matrix[i][0] * t2->matrix[0][0] +
                       t1->matrix[i][1] * t2->matrix[1][0] +
                       t1->matrix[i][2] * t2->matrix[2][0];
    tmp.matrix[i][1] = t1->matrix[i][0] * t2->matrix[0][1] +
                       t1->matrix[i][1] * t2->matrix[1][1] +
                       t1->matrix[i][2] * t2->matrix[2][1];
    tmp.matrix[i][2] = t1->matrix[i][0] * t2->matrix[0][2] +
                       t1->matrix[i][1] * t2->matrix[1][2] +
                       t1->matrix[i][2] * t2->matrix[2][2];
  }

  tmp.translate.x = t1->translate.x * t2->matrix[0][0] +
                    t1->translate.y * t2->matrix[1][0] +
                    t1->translate.z * t2->matrix[2][0] + t2->translate.x;
  tmp.translate.y = t1->translate.x * t2->matrix[0][1] +
                    t1->translate.y * t2->matrix[1][1] +
                    t1->translate.z * t2->matrix[2][1] + t2->translate.y;
  tmp.translate.z = t1->translate.x * t2->matrix[0][2] +
                    t1->translate.y * t2->matrix[1][2] +
                    t1->translate.z * t2->matrix[2][2] + t2->translate.z;
  MatrixCopy(&tmp, res);
}

/*
 * Return transformation information to map the "coordinate system"
 * with the given origin, "up" vector, radius, and up axis lengths to
 * one in which the "up" vector is the Z axis and the x/y/up axes
 * have unit length.  This is useful for transforming a general
 * form of a primitive into a canonical, Z-axis aligned, unit size
 * primitive, facilitating intersection testing.
 * Assumes that "up" is normalized.
 */
void CoordSysTransform(origin, up, r, len, trans) Vector *origin, *up;
Float r, len;
Trans *trans;
{
  RSMatrix tmp;
  Vector atmp;

  ScaleMatrix(r, r, len, &trans->trans);
  if (1. - fabs(up->z) < EPSILON) {
    atmp.x = 1.;
    atmp.y = atmp.z = 0.;
  } else {
    atmp.x = up->y;
    atmp.y = -up->x;
    atmp.z = 0.;
  }
  /*
   * Might want to make sure that |up->z| is < 1.
   */
  RotationMatrix(atmp.x, atmp.y, atmp.z, -acos(up->z), &tmp);
  MatrixMult(&trans->trans, &tmp, &trans->trans);
  TranslationMatrix(origin->x, origin->y, origin->z, &tmp);
  MatrixMult(&trans->trans, &tmp, &trans->trans);
  MatrixInvert(&trans->trans, &trans->itrans);
}

void TransCopy(from, into) Trans *into, *from;
{
  MatrixCopy(&from->trans, &into->trans);
  MatrixCopy(&from->itrans, &into->itrans);
}

void TransInvert(from, into) Trans *into, *from;
{
  RSMatrix ttmp;
  /*
   * In case into == from...
   */
  if (from == into) {
    ttmp = from->trans;
    into->trans = from->itrans;
    into->itrans = ttmp;
  } else {
    into->trans = from->itrans;
    into->itrans = from->trans;
  }
}

/*
 * Copy a given transformation structure.
 */
void MatrixCopy(from, into) RSMatrix *into, *from;
{
  into->matrix[0][0] = from->matrix[0][0];
  into->matrix[0][1] = from->matrix[0][1];
  into->matrix[0][2] = from->matrix[0][2];
  into->matrix[1][0] = from->matrix[1][0];
  into->matrix[1][1] = from->matrix[1][1];
  into->matrix[1][2] = from->matrix[1][2];
  into->matrix[2][0] = from->matrix[2][0];
  into->matrix[2][1] = from->matrix[2][1];
  into->matrix[2][2] = from->matrix[2][2];
  into->translate = from->translate;
}

void TransInit(trans) Trans *trans;
{
  MatrixInit(&trans->trans);
  MatrixInit(&trans->itrans);
}

void TransCompose(t1, t2, res) Trans *t1, *t2, *res;
{
  MatrixMult(&t1->trans, &t2->trans, &res->trans);
  MatrixMult(&t2->itrans, &t1->itrans, &res->itrans);
}

/*
 * Initialize transformation structure.
 */
void MatrixInit(trans) RSMatrix *trans;
{
  trans->matrix[0][0] = trans->matrix[1][1] = trans->matrix[2][2] = 1.;
  trans->matrix[0][1] = trans->matrix[0][2] = trans->matrix[1][0] =
      trans->matrix[1][2] = trans->matrix[2][0] = trans->matrix[2][1] = 0.;
  trans->translate.x = trans->translate.y = trans->translate.z = 0.;
}

/*
 * Calculate inverse of the given transformation structure.
 */
void MatrixInvert(trans, inverse) RSMatrix *inverse, *trans;
{
  RSMatrix ttmp;
  Float d;
  extern int yylineno;

  ttmp.matrix[0][0] = trans->matrix[1][1] * trans->matrix[2][2] -
                      trans->matrix[1][2] * trans->matrix[2][1];
  ttmp.matrix[1][0] = trans->matrix[1][0] * trans->matrix[2][2] -
                      trans->matrix[1][2] * trans->matrix[2][0];
  ttmp.matrix[2][0] = trans->matrix[1][0] * trans->matrix[2][1] -
                      trans->matrix[1][1] * trans->matrix[2][0];

  ttmp.matrix[0][1] = trans->matrix[0][1] * trans->matrix[2][2] -
                      trans->matrix[0][2] * trans->matrix[2][1];
  ttmp.matrix[1][1] = trans->matrix[0][0] * trans->matrix[2][2] -
                      trans->matrix[0][2] * trans->matrix[2][0];
  ttmp.matrix[2][1] = trans->matrix[0][0] * trans->matrix[2][1] -
                      trans->matrix[0][1] * trans->matrix[2][0];

  ttmp.matrix[0][2] = trans->matrix[0][1] * trans->matrix[1][2] -
                      trans->matrix[0][2] * trans->matrix[1][1];
  ttmp.matrix[1][2] = trans->matrix[0][0] * trans->matrix[1][2] -
                      trans->matrix[0][2] * trans->matrix[1][0];
  ttmp.matrix[2][2] = trans->matrix[0][0] * trans->matrix[1][1] -
                      trans->matrix[0][1] * trans->matrix[1][0];

  d = trans->matrix[0][0] * ttmp.matrix[0][0] -
      trans->matrix[0][1] * ttmp.matrix[1][0] +
      trans->matrix[0][2] * ttmp.matrix[2][0];

  if (fabs(d) < EPSILON * EPSILON)
    RLerror(RL_PANIC, "Singular matrix.\n", yylineno);

  ttmp.matrix[0][0] /= d;
  ttmp.matrix[0][2] /= d;
  ttmp.matrix[1][1] /= d;
  ttmp.matrix[2][0] /= d;
  ttmp.matrix[2][2] /= d;

  d = -d;

  ttmp.matrix[0][1] /= d;
  ttmp.matrix[1][0] /= d;
  ttmp.matrix[1][2] /= d;
  ttmp.matrix[2][1] /= d;

  ttmp.translate.x = -(ttmp.matrix[0][0] * trans->translate.x +
                       ttmp.matrix[1][0] * trans->translate.y +
                       ttmp.matrix[2][0] * trans->translate.z);
  ttmp.translate.y = -(ttmp.matrix[0][1] * trans->translate.x +
                       ttmp.matrix[1][1] * trans->translate.y +
                       ttmp.matrix[2][1] * trans->translate.z);
  ttmp.translate.z = -(ttmp.matrix[0][2] * trans->translate.x +
                       ttmp.matrix[1][2] * trans->translate.y +
                       ttmp.matrix[2][2] * trans->translate.z);

  MatrixCopy(&ttmp, inverse);
}

/*
 * Apply a transformation to a point (translation affects the point).
 */
void PointTransform(vec, trans) Vector *vec;
RSMatrix *trans;
{
  Vector tmp;

  tmp.x = vec->x * trans->matrix[0][0] + vec->y * trans->matrix[1][0] +
          vec->z * trans->matrix[2][0] + trans->translate.x;
  tmp.y = vec->x * trans->matrix[0][1] + vec->y * trans->matrix[1][1] +
          vec->z * trans->matrix[2][1] + trans->translate.y;
  tmp.z = vec->x * trans->matrix[0][2] + vec->y * trans->matrix[1][2] +
          vec->z * trans->matrix[2][2] + trans->translate.z;
  *vec = tmp;
}

/*
 * 'c1x' is the X (0th) component of the first column, and so on.
 */
void ArbitraryMatrix(c1x, c2x, c3x, c1y, c2y, c3y, c1z, c2z, c3z, tx, ty, tz,
                     trans) Float c1x,
    c1y, c1z, c2x, c2y, c2z, c3x, c3y, c3z, tx, ty, tz;
RSMatrix *trans;
{
  trans->matrix[0][0] = c1x;
  trans->matrix[1][0] = c1y;
  trans->matrix[2][0] = c1z;

  trans->matrix[0][1] = c2x;
  trans->matrix[1][1] = c2y;
  trans->matrix[2][1] = c2z;

  trans->matrix[0][2] = c3x;
  trans->matrix[1][2] = c3y;
  trans->matrix[2][2] = c3z;

  trans->translate.x = tx;
  trans->translate.y = ty;
  trans->translate.z = tz;
}

/*
 * Apply transformation to a vector (translations have no effect).
 */
void VecTransform(vec, trans) Vector *vec;
RSMatrix *trans;
{
  Vector tmp;

  tmp.x = vec->x * trans->matrix[0][0] + vec->y * trans->matrix[1][0] +
          vec->z * trans->matrix[2][0];
  tmp.y = vec->x * trans->matrix[0][1] + vec->y * trans->matrix[1][1] +
          vec->z * trans->matrix[2][1];
  tmp.z = vec->x * trans->matrix[0][2] + vec->y * trans->matrix[1][2] +
          vec->z * trans->matrix[2][2];

  *vec = tmp;
}

/*
 * Transform normal -- multiply by the transpose of the given
 * matrix (which is the inverse of the 'desired' transformation).
 */
void NormalTransform(norm, it) Vector *norm;
RSMatrix *it;
{
  Vector onorm;

  onorm = *norm;

  norm->x = onorm.x * it->matrix[0][0] + onorm.y * it->matrix[0][1] +
            onorm.z * it->matrix[0][2];
  norm->y = onorm.x * it->matrix[1][0] + onorm.y * it->matrix[1][1] +
            onorm.z * it->matrix[1][2];
  norm->z = onorm.x * it->matrix[2][0] + onorm.y * it->matrix[2][1] +
            onorm.z * it->matrix[2][2];
  (void)VecNormalize(norm);
}

/*
 * Transform "ray" by transforming the origin point and direction vector.
 */
Float RayTransform(ray, trans) Ray *ray;
RSMatrix *trans;
{
  PointTransform(&ray->pos, trans);
  VecTransform(&ray->dir, trans);
  return VecNormalize(&ray->dir);
}

void TransPropagate(trans) Trans *trans;
{ (*trans->methods->propagate)(trans->tr, &trans->trans, &trans->itrans); }

void TransResolveAssoc(trans) Trans *trans;
{
  Trans *curtrans;
  ExprAssoc *curassoc;

  for (curtrans = trans; curtrans; curtrans = curtrans->next) {
    for (curassoc = curtrans->assoc; curassoc; curassoc = curassoc->next) {
      *curassoc->lhs = ExprEval(curassoc->expr);
    }
    if (curtrans->assoc)
      TransPropagate(curtrans);
  }
}

void TransComposeList(list, result) Trans *list, *result;
{
  TransCopy(list, result);
  for (list = list->next; list; list = list->next)
    TransCompose(list, result, result);
}
