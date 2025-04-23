/*
 * builtin.c
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

Float SumExpr(a, b) Float a, b;
{ return a + b; }
Float DiffExpr(a, b) Float a, b;
{ return a - b; }

Float MultExpr(a, b) Float a, b;
{ return a * b; }
Float DivideExpr(a, b) Float a, b;
{ return a / b; }

Float ModExpr(a, b) Float a, b;
{ return (Float)((int)a % (int)b); }

Float NegateExpr(a) Float a;
{ return -a; }

Float PowExpr(a, b) Float a, b;
{
  if (a == 0.0 && b == 0.0)
    return 1.0;
  if (a < 0.0 && equal(b, (int)b) && ((int)b % 2) == 0)
    return -pow(-a, b);
  return pow(a, b);
}

Float LinearTime(starttime, startval, endtime, endval) Float starttime, endtime,
    startval, endval;
{
  if (TimeExpr->value < starttime)
    return startval;
  if (TimeExpr->value > endtime)
    return endval;
  if (equal(endtime, starttime))
    return startval;
  return startval + (endval - startval) * (TimeExpr->value - starttime) /
                        (endtime - starttime);
}
