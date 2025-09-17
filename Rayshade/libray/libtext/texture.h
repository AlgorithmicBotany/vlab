/*
 * texture.h
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
#ifndef TEXTURE_H
#define TEXTURE_H

#include "libobj/geom.h"
#include "libsurf/surface.h"
#include "mapping.h"

/*
 * Surface properties
 */
#define COLOR 0 /* AMBIENT, DIFFUSE, SPECULAR and BODY */
#define AMBIENT 1
#define DIFFUSE 2
#define SPECULAR 3
#define BODY 4
#define REFLECT 5
#define TRANSP 6
#define SPECPOW 7
#define BUMP 8
#define INDEX 9

#define TextPointToModel(p) PointTransform(p, &model2text.itrans)
#define TextPointToPrim(p) PointTransform(p, &prim2text.itrans)
#define TextPointToWorld(p) PointTransform(p, &world2text.itrans)
#define TextRayToModel(p) RayTransform(r, &model2text.itrans)
#define TextRayToPrim(r) RayTransform(r, &prim2text.itrans)
#define TextRayToWorld(r) RayTransform(r, &world2text.itrans)
#define TextNormToModel(n) NormalTransform(n, &model2text.trans)
#define TextNormToPrim(n) NormalTransform(n, &prim2text.trans)
#define TextNormToWorld(n) NormalTransform(n, &world2text.trans)

#define ModelPointToText(p) PointTransform(p, &model2text.trans)
#define ModelNormToText(n) NormalTransform(n, &model2text.itrans)
#define ModelRayToText(r) RayTransform(r, &model2text.trans)

typedef char *TextRef;

/*
 * Texture structure
 */
typedef struct Texture {
  TextRef data;         /* Texturing info */
  void (*method)();     /* method */
  Trans *trans;         /* transformation info */
  short animtrans;      /* is the transformation animated? */
  struct Texture *next; /* next in list */
} Texture;

extern Texture *TextCreate(TextRef data, void (*meth)(void *, Geom *, Ray *, Vector *, Vector *, Vector *, Surface *)), *TextAppend(Texture *text, Texture *list);
extern void DNoise3(), VfBm(), TextApply(), MakeBump(), Wrinkled();
extern Float Noise3(), Noise2(), Chaos(), Marble(), fBm();
extern int TileValue();
Color *ColormapRead();

extern Trans model2text, prim2text, world2text;
extern void TextToUV();
#endif /* TEXTURE_H */
