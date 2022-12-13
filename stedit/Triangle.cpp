/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "Triangle.h"
#ifdef __APPLE__

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

#else

#ifdef WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

#endif

Triangle::Triangle(Point *a, Point *b, Point *c, double imageRatio) {
  this->a = a;
  this->b = b;
  this->c = c;
  // If the texture coordinates are not given as arguments, calculate them
  if (imageRatio < 1) {
    texCoordA =
        Point((a->X() + WIN_SIZE * imageRatio) / (WIN_SIZE * imageRatio * 2),
              (a->Y() + WIN_SIZE) / (WIN_SIZE * 2));
    texCoordB =
        Point((b->X() + WIN_SIZE * imageRatio) / (WIN_SIZE * imageRatio * 2),
              (b->Y() + WIN_SIZE) / (WIN_SIZE * 2));
    texCoordC =
        Point((c->X() + WIN_SIZE * imageRatio) / (WIN_SIZE * imageRatio * 2),
              (c->Y() + WIN_SIZE) / (WIN_SIZE * 2));
  } else {
    texCoordA =
        Point((a->X() + WIN_SIZE) / (WIN_SIZE * 2),
              (a->Y() + WIN_SIZE / imageRatio) / ((WIN_SIZE * 2) / imageRatio));
    texCoordB =
        Point((b->X() + WIN_SIZE) / (WIN_SIZE * 2),
              (b->Y() + WIN_SIZE / imageRatio) / ((WIN_SIZE * 2) / imageRatio));
    texCoordC =
        Point((c->X() + WIN_SIZE) / (WIN_SIZE * 2),
              (c->Y() + WIN_SIZE / imageRatio) / ((WIN_SIZE * 2) / imageRatio));
  }
}

Triangle::Triangle(Point *a, Point *b, Point *c, Point texCoordA,
                   Point texCoordB, Point texCoordC) {
  this->a = a;
  this->b = b;
  this->c = c;
  this->texCoordA = texCoordA;
  this->texCoordB = texCoordB;
  this->texCoordC = texCoordC;
}

// Checks if the given point is inside the triangle
bool Triangle::containsPoint(Point point) {
  if ((point == *a) || (point == *b) || (point == *c))
    return true;

  Vector3 va = point - *a;
  Vector3 vb = point - *b;
  Vector3 vc = point - *c;
  va = va.normalize();
  vb = vb.normalize();
  vc = vc.normalize();
  Vector3 resultant = va + vb + vc;
  return resultant.getMagnitude() <= 1.0; // Equality to 1 indicates an edge
}

// Checks if the given point is inside the triangle, but doesn't take into
// account edges or corner points
bool Triangle::containsPointExcludingEdges(Point point) {
  Vector3 va = point - *a;
  Vector3 vb = point - *b;
  Vector3 vc = point - *c;
  va = va.normalize();
  vb = vb.normalize();
  vc = vc.normalize();
  Vector3 resultant = va + vb + vc;
  return resultant.getMagnitude() < 1.0;
}

// Checks if the triangle has the given vertex
bool Triangle::containsVertex(Point *vertex) {
  return a == vertex || b == vertex || c == vertex;
}

// Gets the edge of the triangle that does NOT contain the given vertex
Edge Triangle::getOppositeEdge(Point *vertex) {
  if (!containsVertex(vertex))
    return Edge(0, 0); // Error

  Point *p1, *p2;
  if (vertex == a) {
    p1 = b;
    p2 = c;
  } else if (vertex == b) {
    p1 = c;
    p2 = a;
  } else {
    p1 = a;
    p2 = b;
  }
  return Edge(p1, p2);
}

// Gets the closest point on the edges of the triangle to the given point
Point Triangle::getClosestPoint(Point point) {
  Point p1 = Edge(a, b).getClosestPoint(
      point); // Get the closest point on each edge of the triangle
  Point p2 = Edge(b, c).getClosestPoint(point);
  Point p3 = Edge(c, a).getClosestPoint(point);

  double dist1 = p1.distanceTo(point); // Get the distances
  double dist2 = p2.distanceTo(point);
  double dist3 = p3.distanceTo(point);

  if (dist1 <= dist2 && dist1 <= dist3)
    return p1; // Return the point that's actually closest
  else if (dist2 <= dist3)
    return p2;
  else
    return p3;
}

// Splits the triangle into three triangles by cutting it from each vertex to
// the given point Only functions correctly if the point is inside the triangle
vector<Triangle *> Triangle::splitAt(Point *point) {
  vector<Triangle *> result;
  // Use barycentric coordinates to determine the texture coordinate of the new
  // point based on the coordinates of the old points
  double mag = (b->X() - a->X()) * (c->Y() - a->Y()) - (c->X() - a->X()) * (b->Y() - a->Y());
  double b1 = ((b->X() - point->X()) * (c->Y() - point->Y()) -
               (c->X() - point->X()) * (b->Y() - point->Y())) /
              mag;
  double b2 = ((c->X() - point->X()) * (a->Y() - point->Y()) -
               (a->X() - point->X()) * (c->Y() - point->Y())) /
              mag;
  double b3 = ((a->X() - point->X()) * (b->Y() - point->Y()) -
               (b->X() - point->X()) * (a->Y() - point->Y())) /
              mag;
  Point texCoordP = (texCoordA * b1 + texCoordB * b2 + texCoordC * b3);
  result.push_back(new Triangle(point, a, b, texCoordP, texCoordA,
                                texCoordB)); // Create the three new triangles
  result.push_back(new Triangle(point, b, c, texCoordP, texCoordB, texCoordC));
  result.push_back(new Triangle(point, c, a, texCoordP, texCoordC, texCoordA));
  return result;
}

// Save the values of the triangle points so that the information is not lost if
// the pointers die. Used for undoing and redoing
void Triangle::saveValues() {
  valA = *a;
  valB = *b;
  valC = *c;
}

// Restore the saved values to the actual points
void Triangle::restoreValues() {
  a->setX(valA.X());
  a->setY(valA.Y());
  b->setX(valB.X());
  b->setY(valB.Y());
  c->setX(valC.X());
  c->setY(valC.Y());
}

// Gets the vertex with the given index (a=1, b=2, c=3)
Point *Triangle::get(int i) {
  if (i == 1)
    return a;
  if (i == 2)
    return b;
  if (i == 3)
    return c;
  return 0; // Return null if the index is invalid
}

// Gets the vertex value with the given index (a=1, b=2, c=3)
Point Triangle::getVal(int i) {
  if (i == 1)
    return valA;
  if (i == 2)
    return valB;
  if (i == 3)
    return valC;
  else
    return Point();
}

// Gets the texture coordinate with the given index (a=1, b=2, c=3)
Point Triangle::getTexCoord(int i) {
  if (i == 1)
    return texCoordA;
  if (i == 2)
    return texCoordB;
  if (i == 3)
    return texCoordC;
  else
    return Point();
}

// Sets the vertex with the given index to the given vertex
void Triangle::set(int i, Point *newPoint) {
  if (i == 1)
    a = newPoint;
  if (i == 2)
    b = newPoint;
  if (i == 3)
    c = newPoint;
}

// Sets the vertex with the given index to the given vertex, and sets its
// texture coordinate to the given value
void Triangle::set(int i, Point *newPoint, Point texCoord) {
  if (i == 1) {
    a = newPoint;
    texCoordA = texCoord;
  }
  if (i == 2) {
    b = newPoint;
    texCoordB = texCoord;
  }
  if (i == 3) {
    c = newPoint;
    texCoordC = texCoord;
  }
}

// Draw the triangle with the correct texture coordinates
void Triangle::draw() {
  glBegin(GL_TRIANGLES);
  glTexCoord2f(texCoordA.X(), texCoordA.Y());
  glVertex2f(a->X(), a->Y());
  glTexCoord2f(texCoordB.X(), texCoordB.Y());
  glVertex2f(b->X(), b->Y());
  glTexCoord2f(texCoordC.X(), texCoordC.Y());
  glVertex2f(c->X(), c->Y());
  glEnd();
}

// Draw the triangle's lines with the given width, without textures or alpha
// blending
void Triangle::drawLines(double width) {
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glLineWidth(width);
  glBegin(GL_LINE_LOOP);
  glVertex2f(a->X(), a->Y());
  glVertex2f(b->X(), b->Y());
  glVertex2f(c->X(), c->Y());
  glEnd();
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
}
