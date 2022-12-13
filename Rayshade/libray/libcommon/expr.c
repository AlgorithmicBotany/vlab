/*
 * expr.c
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
#include "common.h"

Expr *TimeExpr, *FrameExpr;

static Expr *ExprCreate();
void ExprFree();

Expr *ExprFloatCreate(val, timevary) Float val;
int timevary;
{
  Expr *res;

  res = ExprCreate(FLOAT_EXPR, timevary);
  res->value = val;
  return res;
}

Expr *ExprReuseFloatCreate(val) Float val;
{
  Expr *res;

  /* this should get the expr from a magic list
   * that will reuse the exprs.
   * But not yet...
   */
  res = ExprCreate(FLOAT_EXPR, FALSE);
  res->value = val;
  return res;
}

Expr *ExprMalloc() { return (Expr *)Malloc(sizeof(Expr)); }

static Expr *ExprCreate(type, timevary) int type, timevary;
{
  Expr *res;

  res = ExprMalloc();
  res->type = type;
  res->timevary = timevary;
  res->symtab = FALSE;
  res->nparams = 0;
  res->params = (Expr **)NULL;
  res->function = (Float(*)())NULL;
  res->value = 0.0;
  res->timenow = -FAR_AWAY;
  return res;
}

Float ExprEval(expr) Expr *expr; /* Expression to evaluate */
{
  /*
   * If the expression is a time-varying function,
   * and its time is incorrect, evaluate at
   * the current time.
   */
  if (expr->timevary && expr->nparams &&
      !equal(TimeExpr->value, expr->timenow)) {
    if (expr->nparams == 1) {
      expr->value = (*expr->function)(ExprEval(expr->params[0]));
    } else if (expr->nparams == 2) {
      expr->value = (*expr->function)(ExprEval(expr->params[0]),
                                      ExprEval(expr->params[1]));
    } else if (expr->nparams == 3) {
      expr->value = (*expr->function)(ExprEval(expr->params[0]),
                                      ExprEval(expr->params[1]),
                                      ExprEval(expr->params[2]));
    } else if (expr->nparams == 4) {
      expr->value = (*expr->function)(
          ExprEval(expr->params[0]), ExprEval(expr->params[1]),
          ExprEval(expr->params[2]), ExprEval(expr->params[3]));
    } else if (expr->nparams == 5) {
      expr->value = (*expr->function)(
          ExprEval(expr->params[0]), ExprEval(expr->params[1]),
          ExprEval(expr->params[2]), ExprEval(expr->params[3]),
          ExprEval(expr->params[4]));
    } else if (expr->nparams > 5)
      RLerror(RL_PANIC, "Expression with > 5 args?\n");
    expr->timenow = TimeExpr->value;
  }
  return expr->value;
}

Expr *ExprFunctionCreate(fp, nparams, params, timevary) Float (*fp)();
int nparams, timevary;
Expr **params;
{
  Expr *res;

  res = ExprCreate(FLOAT_EXPR, timevary);
  res->function = fp;
  res->nparams = nparams;
  res->params = params;

  return res;
}

Expr *ExprResolve1(a, fp, timevary) Expr *a;
Float (*fp)();
int timevary;
{
  Expr **params, *res;

  if (!timevary && !a->timevary) {
    res = ExprFloatCreate((*fp)(a->value), FALSE);
    ExprFree(a);
    return res;
  } else {
    params = (Expr **)Malloc(sizeof(Expr *));
    params[0] = a;
    return ExprFunctionCreate(fp, 1, params, TRUE);
  }
}

Expr *ExprResolve2(a, b, fp, timevary) Expr *a, *b;
Float (*fp)();
int timevary;
{
  Expr **params, *res;
  if (!timevary && !a->timevary && !b->timevary) {
    res = ExprFloatCreate((*fp)(a->value, b->value), FALSE);
    ExprFree(a);
    ExprFree(b);
    return res;
  } else {
    params = (Expr **)Malloc(2 * sizeof(Expr *));
    params[0] = a;
    params[1] = b;
    return ExprFunctionCreate(fp, 2, params, TRUE);
  }
}

Expr *ExprResolve3(a, b, c, fp, timevary) Expr *a, *b, *c;
Float (*fp)();
int timevary;
{
  Expr **params, *res;
  if (!timevary && !a->timevary && !b->timevary && !c->timevary) {
    res = ExprFloatCreate((*fp)(a->value, b->value, c->value), FALSE);
    ExprFree(a);
    ExprFree(b);
    ExprFree(c);
    return res;
  } else {
    params = (Expr **)Malloc(3 * sizeof(Expr *));
    params[0] = a;
    params[1] = b;
    params[2] = c;
    return ExprFunctionCreate(fp, 3, params, TRUE);
  }
}

Expr *ExprResolve4(a, b, c, d, fp, timevary) Expr *a, *b, *c, *d;
Float (*fp)();
int timevary;
{
  Expr **params, *res;
  if (!timevary && !a->timevary && !b->timevary && !c->timevary &&
      !d->timevary) {
    res = ExprFloatCreate((*fp)(a->value, b->value, c->value, d->value), FALSE);
    ExprFree(a);
    ExprFree(b);
    ExprFree(c);
    ExprFree(d);
    return res;
  } else {
    params = (Expr **)Malloc(4 * sizeof(Expr *));
    params[0] = a;
    params[1] = b;
    params[2] = c;
    params[3] = d;
    return ExprFunctionCreate(fp, 4, params, TRUE);
  }
}

Expr *ExprResolve5(a, b, c, d, e, fp, timevary) Expr *a, *b, *c, *d, *e;
Float (*fp)();
int timevary;
{
  Expr **params, *res;
  if (!timevary && !a->timevary && !b->timevary && !c->timevary &&
      !d->timevary && !e->timevary) {
    res = ExprFloatCreate(
        (*fp)(a->value, b->value, c->value, d->value, e->value), FALSE);
    ExprFree(a);
    ExprFree(b);
    ExprFree(c);
    ExprFree(d);
    ExprFree(e);
    return res;
  } else {
    params = (Expr **)Malloc(5 * sizeof(Expr *));
    params[0] = a;
    params[1] = b;
    params[2] = c;
    params[3] = d;
    params[4] = e;
    return ExprFunctionCreate(fp, 5, params, TRUE);
  }
}

ExprAssoc *AssocCreate(lhs, expr, next) Float *lhs;
Expr *expr;
ExprAssoc *next;
{
  ExprAssoc *new;

  new = (ExprAssoc *)Malloc(sizeof(ExprAssoc));
  new->lhs = lhs;
  new->expr = expr;
  new->next = next;
  return new;
}

void TimeSet(time) Float time;
{ TimeExpr->value = time; }

void FrameSet(frame) Float frame;
{ FrameExpr->value = frame; }

void ExprFree(expr) Expr *expr;
{
  if (!expr->symtab) {
    if (expr->type == BUILTIN_EXPR && expr->params)
      free((voidstar)expr->params);
    free((voidstar)expr);
  }
}
