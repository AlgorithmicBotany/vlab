/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

#include "Vector3.h"
#include <math.h>
#include <vector>
using namespace std;
#include "Point.h"
#include "Edge.h"
#include "Globals.h"

class Triangle {
private:
  Point *a, *b, *c;       // The corner points of the triangle
  Point valA, valB, valC; // The actual values of the triangle's corners, for
                          // temporary storage only
  Point texCoordA, texCoordB,
      texCoordC; // The texture coordinates of the triangle's points

public:
  Triangle(Point *a, Point *b, Point *c, double imageRatio);
  Triangle(Point *a, Point *b, Point *c, Point texCoordA, Point texCoordB,
           Point texCoordC);
  bool containsPoint(Point point);
  bool containsPointExcludingEdges(Point point);
  bool containsVertex(Point *vertex);
  Edge getOppositeEdge(Point *vertex);
  Point getClosestPoint(Point point);
  Point *get(int i);
  Point getVal(int i);
  Point getTexCoord(int i);
  void set(int i, Point *newPoint);
  void set(int i, Point *newPoint, Point texCoord);
  vector<Triangle *> splitAt(Point *point);
  void saveValues();
  void restoreValues();
  void draw();
  void drawLines(double width);
};

