/*
 * mist.h
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
#ifndef MIST_H

#define AtmosMistCreate(c, t, z, s)                                            \
  AtmosCreate((AtmosRef)MistCreate(c, t, z, s), MistApply)
typedef struct Mist {
  Color color;       /* Mist color */
  Color trans;       /* R, G, B transmissivity */
  Float scale, zero; /* Height scale, start Z */
} Mist;

extern Mist *MistCreate();
extern void MistApply();

#endif /* MIST_H */
