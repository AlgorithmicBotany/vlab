/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "Edge.h"

Edge::Edge(Point *a, Point *b) {
  this->a = a;
  this->b = b;
}

// Gets the closest point on the line segment represented by this edge to the
// given point
Point Edge::getClosestPoint(Point point) {
  Vector3 ap = point - *a; // Vector from a to the point
  Vector3 ab = *b - *a;    // Vector from a to b
  double abab = ab * ab;   // Dot products
  double apab = ap * ab;
  double t =
      apab /
      abab; // Distance along the line to the closest point, from zero to one

  if (t < 0.0)
    t = 0.0; // Clamp this to be on the line segment
  else if (t > 1.0)
    t = 1.0;

  Point closest = *a + (ab * t);
  return closest;
}
