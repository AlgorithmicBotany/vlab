/*
 * transform.h
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
#ifndef TRANSFORM_H
#define TRANSFORM_H

typedef voidstar TransRef;
typedef TransRef TransCreateFunc();

/*
 * Transformation 'matrix'.
 */
typedef struct RSMatrix {
  Float matrix[3][3]; /* Rotation matrix */
  Vector translate;   /* Translation */
} RSMatrix;

typedef struct {
  TransRef (*create)(); /* Create it... */
  void (*propagate)();  /* Propagate changes to parameters */
} TransMethods;

/*
 * Transformation structure
 */
typedef struct Trans {
  TransRef tr;           /* transform data */
  TransMethods *methods; /* transform methods */
  ExprAssoc *assoc;      /* animated parameters */
  short animated;        /* is the transformation animated? */
  RSMatrix trans,        /* object space --> world space */
      itrans;            /* worldspace --> object space */
  struct Trans *next, *prev;
} Trans;

extern void MatrixMult(), MatrixCopy(), MatrixInit(), MatrixInvert(),
    TransCopy(), TransInit(), TransInvert(), TransCompose(Trans *t1, Trans *t2, Trans *res), VecTransform(),
    PointTransform(), NormalTransform();

extern Trans *TransCreate(TransRef tr, TransMethods *meth);

extern RSMatrix *MatrixCreate();

void RotationMatrix(), TranslationMatrix(), ScaleMatrix(), ArbitraryMatrix(),
    CoordSysTransform();
extern Float RayTransform();

extern void TransResolveAssoc();
extern void TransComposeList();
extern void TransPropagate(Trans *trans);
void TransFree(Trans *trans);
void TransAssoc(Trans *trans, Float *ptr, Expr *expr);

#endif /* TRANSFORM_H */
