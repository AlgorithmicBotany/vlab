/*
 * stripe.c
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
#include "texture.h"
#include "stripe.h"

Stripe *StripeCreate(surf, width, bump, mapping) Surface *surf;
Float width, bump;
Mapping *mapping;
{
  Stripe *stripe;

  stripe = (Stripe *)Malloc(sizeof(Stripe));
  stripe->surf = surf;
  stripe->mapping = mapping;
  stripe->width = width;
  stripe->bump = bump;
  return stripe;
}

void StripeApply(stripe, prim, pos, norm, gnorm, surf) Stripe *stripe;
Geom *prim;
Vector *pos, *norm, *gnorm;
Surface *surf;
{
  Vector dpdu, dpdv;
  Float fu, fv, u, v;

  TextToUV(stripe->mapping, prim, pos, gnorm, &u, &v, &dpdu, &dpdv);

  u -= floor(u);
  v -= floor(v);

  /*
   *    s s          s
   *   | | |        | |
   * 1 +-+------------+
   *   |X|\^^^^^^^^^^/| } s
   *   |X|<+--------+>|
   *   |X|<|        |>|
   *   |X|<|        |>|
   *   |X|<|        |>|
   * v |X|<|        |>|
   *   |X|<|        |>|
   *   |X|<|        |>|
   *   |X|<+--------+>|
   *   |X|/vvvvvvvvvv\| } s
   *   |X+------------+
   *   |XXXXXXXXXXXXXX| } s
   * 0 +--------------+
   *   0              1
   *	    u
   *
   * where ^ == positive fv, 0 fu, original surf.
   *	 v == negative fv, 0 fu, original surf.
   *	 > == positive fu, 0 fv, original surf.
   *	 < == negative fu, 0 fv, original surf.
   *   blank == 0 fu, 0 fv, original surf.
   *       X == 0 fu, 0 fv, alternate surf.
   * for stripe->bump > 0.  For stripe->bump < 0., change signs.
   */

  if (u > 2 * stripe->width && v > 2 * stripe->width &&
      u <= 1. - stripe->width && v <= 1. - stripe->width)
    /* flat surface */
    return;
  else if (u < stripe->width || v < stripe->width) {
    /* on the bottom of the bump. */
    *surf = *stripe->surf;
    return;
  }

  /*
   * Lower u & v edges are the 'flat' part of the bump --
   * make our lives simpler below by 'removing' this area
   * from u & v.
   */
  u = (u - stripe->width) / (1. - stripe->width);
  v = (v - stripe->width) / (1. - stripe->width);
  /*
   * Now the hard part -- where's the bump?
   */
  if (v < u) {
    if (v < 1. - u) {
      /* bottom */
      fu = 0.;
      fv = -stripe->bump;
    } else {
      /* right */
      fu = stripe->bump;
      fv = 0.;
    }
  } else {
    if (v < 1. - u) {
      /* left */
      fu = -stripe->bump;
      fv = 0.;
    } else {
      /* top */
      fu = 0.;
      fv = stripe->bump;
    }
  }

  MakeBump(norm, &dpdu, &dpdv, fu, fv);
}
