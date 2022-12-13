/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include <string>
#include "Point.h"
#include <math.h>
using namespace std;

class Edge {
public:
  Point *a, *b;

  Edge(Point *a, Point *b);

  Point getClosestPoint(Point point);
};
