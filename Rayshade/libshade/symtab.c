/*
 * symtab.c
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

#include "rayshade.h"
#include "symtab.h"
#include "builtin.h"

static struct SymtabPredefinedEntry SymtabPredefined[] = {
    {"pi", 3.141592, NULL, FLOAT_EXPR, FALSE, 0},
    {"dtor", 0.017453, NULL, FLOAT_EXPR, FALSE, 0},
    {"rtod", 57.29578, NULL, FLOAT_EXPR, FALSE, 0},
    {"cos", 0.0, (Float(*)())cos, BUILTIN_EXPR, FALSE, 1},
    {"sin", 0.0, (Float(*)())sin, BUILTIN_EXPR, FALSE, 1},
    {"tan", 0.0, (Float(*)())tan, BUILTIN_EXPR, FALSE, 1},
    {"sqrt", 0.0, (Float(*)())sqrt, BUILTIN_EXPR, FALSE, 1},
    {"acos", 0.0, (Float(*)())acos, BUILTIN_EXPR, FALSE, 1},
    {"asin", 0.0, (Float(*)())asin, BUILTIN_EXPR, FALSE, 1},
    {"atan", 0.0, (Float(*)())atan, BUILTIN_EXPR, FALSE, 1},
    {"hypot", 0.0, (Float(*)())hypot, BUILTIN_EXPR, FALSE, 2},
    {"time", 0.0, NULL, FLOAT_EXPR, TRUE, 0},
    {"frame", 0.0, NULL, FLOAT_EXPR, TRUE, 0},
    {"linear", 0.0, LinearTime, BUILTIN_EXPR, TRUE, 4},
    {NULL, 0.0, NULL, 0, 0, 0}};

SymtabEntry *Symtab = (SymtabEntry *)NULL;

void SymtabAddEntry();

void SymtabInit() {
  int i;

  for (i = 0; SymtabPredefined[i].name; i++) {
    if (SymtabPredefined[i].type == BUILTIN_EXPR)
      SymtabAddEntry(SymtabPredefined[i].name, SymtabPredefined[i].type, NULL,
                     SymtabPredefined[i].fp, SymtabPredefined[i].timevary,
                     SymtabPredefined[i].params);
    else
      SymtabAddEntry(
          SymtabPredefined[i].name, SymtabPredefined[i].type,
          ExprFloatCreate(SymtabPredefined[i].f, SymtabPredefined[i].timevary),
          NULL, SymtabPredefined[i].timevary, 0);
  }
  TimeExpr = ExprFloatSymtabFind("time");
  FrameExpr = ExprFloatSymtabFind("frame");
}

void SymtabAddEntry(name, type, expr, fp, timevary, params) char *name;
Expr *expr;
Float (*fp)();
int type, timevary, params;
{
  SymtabEntry *res;

  if (SymtabFind(name) != (SymtabEntry *)NULL)
    RLerror(RL_ABORT, "Symbol %s redefined.\n", name);

  res = (SymtabEntry *)Malloc(sizeof(SymtabEntry));
  res->name = strsave(name);
  res->type = type;
  res->timevary = timevary;
  switch (type) {
  case FLOAT_EXPR:
    res->value.expr = expr;
    expr->symtab = TRUE;
    break;
  case BUILTIN_EXPR:
    res->value.fp = fp;
    break;
  default:
    RLerror(RL_WARN, "Type %d not implemented!!!", type);
  }
  res->params = params;
  res->next = Symtab;
  Symtab = res;
}

SymtabEntry *SymtabFind(name) char *name;
{
  SymtabEntry *res;
  for (res = Symtab; res; res = res->next) {
    if (!strcmp(res->name, name))
      return res;
  }
  /*error*/
  return NULL;
}

Expr *ExprFloatSymtabFind(name) char *name;
{
  SymtabEntry *res;

  if ((res = SymtabFind(name)) == NULL)
    RLerror(RL_PANIC, "Symbol %s not defined!\n", name);
  if (res->type != FLOAT_EXPR)
    RLerror(RL_PANIC, "Symbol %s is not a float expression!\n", name);
  return res->value.expr;
}

SymtabEntry *SymtabBuiltinFind(name) char *name;
{
  SymtabEntry *res;

  if ((res = SymtabFind(name)) == NULL)
    RLerror(RL_PANIC, "Symbol %s not defined!\n", name);
  if (res->type != BUILTIN_EXPR)
    RLerror(RL_PANIC, "Symbol %s is not a built in function!\n", name);
  return res;
}
