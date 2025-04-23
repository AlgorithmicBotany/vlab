/*
 * wood.c
 *
 * Copyright (C) 1989, 1991, Craig E. Kolb, Robert F. Skinner
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
 * $Id: wood.c,v 1.1.1.1 2005/01/07 20:51:06 laneb Exp $
 *
 * $Log: wood.c,v $
 * Revision 1.1.1.1  2005/01/07 20:51:06  laneb
 * Initial import, straight port to Windows/Linux
 *
 * Revision 4.0  91/07/17  14:44:35  kolb
 * Initial version.
 *
 */
#include "texture.h"
#include "wood.h"

Wood *WoodCreate() {
  return (Wood *)NULL; /* No data associated with wood texture */
}

/*ARGSUSED*/
void
WoodApply(
	__attribute__((unused))  void *p_wood,
	__attribute__((unused))  Geom *prim,
	__attribute__((unused))  Ray *ray,
	 Vector *pos, __attribute__((unused)) Vector *norm, __attribute__((unused)) Vector *gnorm,
	 Surface *surf)
{
  Float red, grn, blu;
  Float chaos, brownLayer, greenLayer;
  Float perturb, brownPerturb, greenPerturb, grnPerturb;
  Float t;
  
  Wood *wood = (Wood *)p_wood;

  chaos = Chaos(pos, 7);
  t = sin(sin(8. * chaos + 7 * pos->x + 3. * pos->y));

  greenLayer = brownLayer = fabs(t);

  perturb = sin(40. * chaos + 50. * pos->z);
  perturb = fabs(perturb);

  brownPerturb = .6 * perturb + 0.3;
  greenPerturb = .2 * perturb + 0.8;
  grnPerturb = .15 * perturb + 0.85;
  grn = 0.5 * pow(fabs(brownLayer), 0.3);
  brownLayer = pow(0.5 * (brownLayer + 1.0), 0.6) * brownPerturb;
  greenLayer = pow(0.5 * (greenLayer + 1.0), 0.6) * greenPerturb;

  red = (0.5 * brownLayer + 0.35 * greenLayer) * 2. * grn;
  blu = (0.25 * brownLayer + 0.35 * greenLayer) * 2.0 * grn;
  grn *= max(brownLayer, greenLayer) * grnPerturb;

  surf->diff.r *= red;
  surf->diff.g *= grn;
  surf->diff.b *= blu;
  surf->amb.r *= red;
  surf->amb.g *= grn;
  surf->amb.b *= blu;
}
