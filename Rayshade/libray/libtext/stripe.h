/*
 * stripe.h
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
#ifndef STRIPE_H

#define TextStripeCreate(s, w, b, m)                                           \
  TextCreate((TextRef)StripeCreate(s, w, b, m), StripeApply)
typedef struct {
  Surface *surf;
  Mapping *mapping;
  Float width, bump;
} Stripe;

extern Stripe *StripeCreate();
extern void StripeApply();

#endif /* STRIPE_H */
