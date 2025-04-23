/*
 * symtab.h
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

#ifndef SYMTAB_H
#define SYMTAB_H

typedef Float (*FloatFuncPtr)();

struct SymtabPredefinedEntry {
  char *name;
  Float f;
  FloatFuncPtr fp;
  int type;
  int timevary;
  int params;
};

typedef union {
  Float (*fp)();
  Expr *expr;
} SymtabValue;

typedef struct SymtabEntry {
  char *name;
  SymtabValue value;
  int type;
  int timevary;
  int params;
  struct SymtabEntry *next;
} SymtabEntry;

extern Expr *ExprFloatSymtabFind(char *name);
extern SymtabEntry *SymtabFind(char *name), *SymtabBuiltinFind(char *name);
extern void SymtabAddEntry(char *name, int type, Expr *expr, Float (*fp)(), int timevary, int params);
void SymtabInit();
#endif /* SYMTAB_H */
