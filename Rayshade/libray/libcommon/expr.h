/*
 * expr.h
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb, Rod G. Bogart
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
#ifndef EXPR_H
#define EXPR_H

#define FLOAT_EXPR 0
#define BUILTIN_EXPR 1

typedef struct Expr {
  short type,   /* Expr type (float/builtin/time) */
      timevary, /* does the expr vary over time? */
      symtab,   /* is the expr in the symtab? */
      nparams;  /* # of params, if builtin */
  Float value,  /* float val/last eval if timeexpr */
      timenow,  /* time of last evaluation */
      (*function)();
  struct Expr **params; /* parameters, if a builtin */
} Expr;

typedef struct ExprAssoc {
  Float *lhs;             /* left-hand side */
  Expr *expr;             /* right-hand side */
  struct ExprAssoc *next; /* Next in list */
} ExprAssoc;

extern Float ExprEval();

extern Expr *ExprResolve1(), *ExprResolve2(), *ExprResolve3(), *ExprResolve4(),
    *ExprResolve5(), *ExprFloatCreate();
extern Expr *TimeExpr, *FrameExpr, *ExprReuseFloatCreate();

extern ExprAssoc *AssocCreate();
extern void TimeSet();
extern void FrameSet();

#endif /* EXPR_H */
