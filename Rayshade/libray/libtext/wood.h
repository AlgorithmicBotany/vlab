/*
 * wood.h
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
 *
 */
#ifndef WOOD_H

#define TextWoodCreate() TextCreate((TextRef)WoodCreate(), WoodApply)

typedef char Wood;

extern Wood *WoodCreate();
void
WoodApply(
	 Wood *wood,
	 Geom *prim,
	 Ray *ray,
	 Vector *pos, Vector *norm, Vector *gnorm,
	 Surface *surf);

#endif /* WOOD_H */
