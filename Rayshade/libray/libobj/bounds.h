/*
 * bounds.h
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
#ifndef BOUNDS_H
#define BOUNDS_H

/*
 * Used to make indices into bounding box arrays more readable.
 */
#define X 0
#define Y 1
#define Z 2
#define LOW 0
#define HIGH 1

/*
 * If minimum X is greater than maximum, then
 * is considered to be unbounded.
 */

#define UNBOUNDED(o) ((o)->bounds[LOW][X] > (o)->bounds[HIGH][X])

/*
 * Is the point p outside of the bounding box "b"?
 */
#define OutOfBounds(p, b)                                                      \
  ((p)->x < b[0][0] || (p)->x > b[1][0] || (p)->y < b[0][1] ||                 \
   (p)->y > b[1][1] || (p)->z < b[0][2] || (p)->z > b[1][2])

extern void BoundsCopy(), BoundsPrint(), BoundsInit(), BoundsEnlarge(),
    BoundsTransform();

extern int BoundsIntersect();
#endif /* BOUNDS_H */
