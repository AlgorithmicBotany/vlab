/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "Patch.h"
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
#include <QDebug>
// Initialize a default 4x4 surface
Patch::Patch(string name, int patchShape) {
  shape = patchShape;
  if (!(shape == Patch::Square || shape == Patch::Triangle)) {
    qDebug() << "Invalid patch shape; defaulting to square";
    shape = Patch::Square;
  }

  if (shape == Patch::Square) {
    for (int x = -15; x <= 15;
         x += 10) { // Points arrayed from -1.5 to 1.5 by default
      controlPoints.push_back(vector<Point>());
      for (int y = 15; y >= -15; y -= 10) {
        controlPoints.at((x + 15) / 10)
            .push_back(Point((double)x / 10.0, (double)y / 10.0, 0));
      }
    }
  } else { // The only other option is triangular
    for (int x = 0; x < 4; x++) {
      controlPoints.push_back(vector<Point>());
      for (int y = 0; y < 4; y++) {
        controlPoints.at(x).push_back(Point(x - y, 1.5 - min(x, y), 0));
      }
    }
  }

  for (int i = 0; i < 8; i++) {
    adjacencies[i] = -1;
    adjacencyDirections[i] = -1;
  }
  displayList = glGenLists(1);
  this->name = name;
  visible = true;
}

void Patch::setAdjacencyStrings(string alPatch, string aPatch, string arPatch,
                                string lPatch, string rPatch, string blPatch,
                                string bPatch, string brPatch) {
  adjacencyStrings[AL] = alPatch;
  adjacencyStrings[A] = aPatch;
  adjacencyStrings[AR] = arPatch;
  adjacencyStrings[L] = lPatch;
  adjacencyStrings[R] = rPatch;
  adjacencyStrings[BL] = blPatch;
  adjacencyStrings[B] = bPatch;
  adjacencyStrings[BR] = brPatch;
}

void Patch::setAdjacencyDirections(string alDir, string aDir, string arDir,
                                   string lDir, string rDir, string blDir,
                                   string bDir, string brDir) {
  adjacencyDirections[AL] = stringToAdj(alDir);
  adjacencyDirections[A] = stringToAdj(aDir);
  adjacencyDirections[AR] = stringToAdj(arDir);
  adjacencyDirections[L] = stringToAdj(lDir);
  adjacencyDirections[R] = stringToAdj(rDir);
  adjacencyDirections[BL] = stringToAdj(blDir);
  adjacencyDirections[B] = stringToAdj(bDir);
  adjacencyDirections[BR] = stringToAdj(brDir);
}

int Patch::stringToAdj(string str) {
  if (str == "AL")
    return AL;
  else if (str == "A")
    return A;
  else if (str == "AR")
    return AR;
  else if (str == "L")
    return L;
  else if (str == "R")
    return R;
  else if (str == "BL")
    return BL;
  else if (str == "B")
    return B;
  else if (str == "BR")
    return BR;
  else
    return -1;
}

string Patch::adjToString(int adj) {
  if (adj == AL)
    return "AL";
  else if (adj == A)
    return "A";
  else if (adj == AR)
    return "AR";
  else if (adj == L)
    return "L";
  else if (adj == R)
    return "R";
  else if (adj == BL)
    return "BL";
  else if (adj == B)
    return "B";
  else if (adj == BR)
    return "BR";
  else
    return "~";
}

// Find anny occurences of the given adjacency name in the list of adjacency
// strings and set  that adjacency to the given index when found
void Patch::indexAdjacency(string adjacencyName, int index) {
  if (adjacencyStrings[AL] == adjacencyName)
    adjacencies[AL] = index;
  if (adjacencyStrings[A] == adjacencyName)
    adjacencies[A] = index;
  if (adjacencyStrings[AR] == adjacencyName)
    adjacencies[AR] = index;
  if (adjacencyStrings[L] == adjacencyName)
    adjacencies[L] = index;
  if (adjacencyStrings[R] == adjacencyName)
    adjacencies[R] = index;
  if (adjacencyStrings[BL] == adjacencyName)
    adjacencies[BL] = index;
  if (adjacencyStrings[B] == adjacencyName)
    adjacencies[B] = index;
  if (adjacencyStrings[BR] == adjacencyName)
    adjacencies[BR] = index;
}

// Subdivide the given set of control points by the given number of samples
vector<Point> Patch::calcBezier(const vector<Point> points, const int samples) {
  vector<Point> finalPoints;
  if (points.size() > 1) {
    vector<Point> currentPoints;
    vector<Point> newPoints;
    Point resultPoint;
    for (int i = 0; i <= samples; i++) {
      double u = (double)i / (double)samples; // Calculate the current u
      currentPoints.clear(); // Clear current points and make a copy of the
                             // given set of points
      for (unsigned int i = 0; i < points.size(); i++) {
        currentPoints.push_back(points.at(i));
      }
      while (currentPoints.size() >
             1) { // Keep subdividing using the Casteljau method until there is
                  // only one point left
        for (unsigned int i = 0; i < currentPoints.size() - 1; i++) {
          resultPoint = (currentPoints.at(i) * (1 - u)) +
                        (currentPoints.at(i + 1) *
                         u); // Calculate the position of the subdivided point
          newPoints.push_back(resultPoint); // Add it to the list of new points
        }
        currentPoints = newPoints; // The new points are now the current points
        newPoints.clear();         // This is no longer needed
      }
      finalPoints.push_back(
          currentPoints.at(0)); // We have found the subdivision point, push it
                                // onto the list of final points
    }
  }
  return finalPoints;
}

// Returns a transposed copy of the this surface
vector<vector<Point>> Patch::transpose(
    vector<vector<Point>> surface) { // swap the rows and columns of a surface
  vector<Point> column;
  vector<vector<Point>> transSurface;
  for (unsigned int j = 0; j < surface.at(0).size(); j++) {
    for (unsigned int i = 0; i < surface.size(); i++) {
      column.push_back(
          surface.at(i).at(j)); // Get the jth column of the surface
    }
    transSurface.push_back(
        column);    // Push the column onto the transposed surface as a row
    column.clear(); // Clear the column before getting the next one
  }
  return transSurface;
}

// Subdivide the surface by the given number of subdivision samples
void Patch::recalcSurface(int samples) {
  surface.clear();
  vector<vector<Point>> subdividedSurface;
  for (unsigned int i = 0; i < controlPoints.size();
       i++) { // Subdivide in one direction...
    subdividedSurface.push_back(calcBezier(controlPoints.at(i), samples));
  }
  subdividedSurface =
      transpose(subdividedSurface); // Then transpose the surface...
  for (unsigned int i = 0; i < subdividedSurface.size();
       i++) { // And subdivide in the same direction again...
    surface.push_back(calcBezier(subdividedSurface.at(i), samples));
  }
  surface = transpose(surface); // And transpose back
}

// Get the normal of the vertex at the given location
Vector3 Patch::getVertexNormal(int i, int j) {
  Vector3 normal = Vector3();
  // [PASCAL] test if i and j are greater than 0 should be done outside this function
  // needs to be fixed
  if ((i<=0) || (j <=0))
    return normal.normalize();
  // Boundary conditions: Only count this triangle if it exists
  if ((unsigned int) i <= faceNormals.size() &&
      (unsigned int) j <= faceNormals.at(0).size()) {
    normal += faceNormals.at(i - 1).at(j - 1).at(0); // Below right triangles
    normal += faceNormals.at(i - 1).at(j - 1).at(1);
  }
  if ((unsigned int) i <= faceNormals.size() &&
      (unsigned int) j < faceNormals.at(0).size()) {
    normal += faceNormals.at(i - 1).at(j).at(0); // Above right triangle
  }
  if ((unsigned int) i < faceNormals.size() &&
      (unsigned int) j < faceNormals.at(0).size()) {
    normal += faceNormals.at(i).at(j).at(0); // Above left triangles
    normal += faceNormals.at(i).at(j).at(1);
  }
  if ((unsigned int) i < faceNormals.size() &&
      (unsigned int) j <= faceNormals.at(0).size()) {
    normal += faceNormals.at(i).at(j - 1).at(1); // Below left triangle
  }

  return normal.normalize(); // Return the normalized normal
}

// Calculates the data structure containing the normal vectors of all the faces
void Patch::calcFaceNormals() {
  Point p1, p2, p3;
  faceNormals.clear(); // clear the old information
  for (unsigned int i = 0; i < surface.size() - 1; i++) {
    faceNormals.push_back(vector<vector<Vector3>>());
    for (unsigned int j = 0; j < surface.at(0).size() - 1; j++) {
      faceNormals.at(i).push_back(vector<Vector3>());
      p1 = surface.at(i).at(
          j); // Calculate the normal of the upper triangle in this quad
      p2 = surface.at((i + 1)).at(j);
      p3 = surface.at((i + 1)).at((j + 1));
      faceNormals.at(i).at(j).push_back(p1.getNormal(p2, p3));
      p1 = surface.at(i).at(
          j); // Calculate the normal of the lower triangle in this quad
      p2 = surface.at(i).at((j + 1));
      p3 = surface.at((i + 1)).at((j + 1));
      faceNormals.at(i).at(j).push_back(p1.getNormal(p3, p2));
    }
  }
}

// Subdivides the current surface
void Patch::subdivide(int samples) {
   int i = 0, j = 0;
   //     std::cerr<<"Subdivide: ";
   //  std::cerr<< controlPoints.at(i).at(j).X()<<" - "<< controlPoints.at(i).at(j).Y()<<" - "<< controlPoints.at(i).at(j).Z()<<std::endl;

   recalcSurface(samples);
   //  std::cerr<<"End Subdivide: ";
   //  std::cerr<< controlPoints.at(i).at(j).X()<<" - "<< controlPoints.at(i).at(j).Y()<<" - "<< controlPoints.at(i).at(j).Z()<<std::endl;

  glNewList(displayList, GL_COMPILE); // Update the display list
  Point p1, p2, p3, p4;
  Vector3 n;
  // Loop through all but the last row and column, since we're drawing faces
  // from the current point to the next points across and down
  int numSurfaceRows = surface.size() - 1;
  int numSurfaceColumns = surface.at(0).size() - 1;
  if (surface.size() > 0) {

    calcFaceNormals();

    glBegin(GL_QUADS);
    for (int i = 0; i < numSurfaceRows; i++) {
      for (int j = 0; j < numSurfaceColumns; j++) {
        p1 = surface.at(i).at(j); // Draw the upper triangle in this quad
        p2 = surface.at(i + 1).at(j);
        p3 = surface.at(i + 1).at(j + 1);
        p4 = surface.at(i).at(j + 1);

        n = getVertexNormal(i, j);
        glNormal3f(n.X(), n.Y(), n.Z());
        glTexCoord2f((float)i / (float)(numSurfaceRows),
                     1 - (float)j /
                             (float)(numSurfaceColumns)); // Calculate the
                                                          // texture coordinates
        glVertex3f(p1.X(), p1.Y(), p1.Z());

        n = getVertexNormal(i + 1, j);
        glNormal3f(n.X(), n.Y(), n.Z());
        glTexCoord2f((float)(i + 1) / (float)(numSurfaceRows),
                     1 - (float)j / (float)(numSurfaceColumns));
        glVertex3f(p2.X(), p2.Y(), p2.Z());

        n = getVertexNormal(i + 1, j + 1);
        glNormal3f(n.X(), n.Y(), n.Z());
        glTexCoord2f((float)(i + 1) / (float)(numSurfaceRows),
                     1 - (float)(j + 1) / (float)(numSurfaceColumns));
        glVertex3f(p3.X(), p3.Y(), p3.Z());

        n = getVertexNormal(i, j + 1);
        glNormal3f(n.X(), n.Y(), n.Z());
        glTexCoord2f((float)(i) / (float)(numSurfaceRows),
                     1 - (float)(j + 1) / (float)(numSurfaceColumns));
        glVertex3f(p4.X(), p4.Y(), p4.Z());
      }
    }
    glEnd();
  }
  glEndList();

}

void Patch::draw() { glCallList(displayList); }

// Draw the grid lines between the control points
void Patch::drawLines(double width, Colour colour) {
  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      glLineWidth(width);
      glColor3f(colour.r, colour.g, colour.b);
      if (i < (int) controlPoints.size() - 1) { // Horizontal grid lines
        glBegin(GL_LINES);
        glVertex3f(controlPoints.at(i).at(j).X(), controlPoints.at(i).at(j).Y(),
                   controlPoints.at(i).at(j).Z());
        glVertex3f(controlPoints.at(i + 1).at(j).X(),
                   controlPoints.at(i + 1).at(j).Y(),
                   controlPoints.at(i + 1).at(j).Z());
        glEnd();
      }
      if (j < (int) controlPoints.at(i).size() - 1) { // Vertical grid lines
        glBegin(GL_LINES);
        glVertex3f(controlPoints.at(i).at(j).X(), controlPoints.at(i).at(j).Y(),
                   controlPoints.at(i).at(j).Z());
        glVertex3f(controlPoints.at(i).at(j + 1).X(),
                   controlPoints.at(i).at(j + 1).Y(),
                   controlPoints.at(i).at(j + 1).Z());
        glEnd();
      }
    }
  }
}

// Draw the control points
void Patch::drawPoints(double size, Colour colour) {
  glColor3f(colour.r, colour.g, colour.b);
  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      controlPoints.at(i).at(j).draw(size);
    }
  }
}

// Draw the control points, and highlight the selected point
void Patch::drawPoints(double size, Colour colour, Colour selectedColour,
                       int selectedI, int selectedJ, bool symmetrical14,
                       bool symmetrical113) {
  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      // Highlight the selected point
      if ((&controlPoints.at(i).at(j) ==
           &controlPoints.at(selectedI).at(selectedJ)) ||
          (symmetrical14 &&
           &controlPoints.at(i).at(j) ==
               &controlPoints.at(numRows() - selectedI - 1).at(selectedJ)) ||
          (symmetrical113 &&
           &controlPoints.at(i).at(j) ==
               &controlPoints.at(selectedI).at(numColumns() - selectedJ - 1)) ||
          (symmetrical14 && symmetrical113 &&
           &controlPoints.at(i).at(j) ==
               &controlPoints.at(numRows() - selectedI - 1)
                    .at(numColumns() - selectedJ - 1)))
        glColor3f(selectedColour.r, selectedColour.g, selectedColour.b);
      else
        glColor3f(colour.r, colour.g, colour.b);
      Point p = controlPoints.at(i).at(j);
      p.draw(size);
    }
  }
}

// Draws colour coded lines and points along the edges of the patch, used to
// visualize patch adjacency
void Patch::drawAdjacencyEdges() {
  glLineWidth(5);
  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      glBegin(GL_LINES);
      if (j == 0 && i < numColumns() - 1) { // Top row
        glColor3f(1, 0, 0);
        glVertex3f(controlPoints.at(i).at(j).X(), controlPoints.at(i).at(j).Y(),
                   controlPoints.at(i).at(j).Z());
        glVertex3f(controlPoints.at(i + 1).at(j).X(),
                   controlPoints.at(i + 1).at(j).Y(),
                   controlPoints.at(i + 1).at(j).Z());
      } else if (j == numRows() - 1 && i < numColumns() - 1) { // Bottom row
        glColor3f(0, 0, 1);
        glVertex3f(controlPoints.at(i).at(j).X(), controlPoints.at(i).at(j).Y(),
                   controlPoints.at(i).at(j).Z());
        glVertex3f(controlPoints.at(i + 1).at(j).X(),
                   controlPoints.at(i + 1).at(j).Y(),
                   controlPoints.at(i + 1).at(j).Z());
      }
      if (i == 0 && j < numRows() - 1) { // Left column
        glColor3f(0, 1, 0);
        glVertex3f(controlPoints.at(i).at(j).X(), controlPoints.at(i).at(j).Y(),
                   controlPoints.at(i).at(j).Z());
        glVertex3f(controlPoints.at(i).at(j + 1).X(),
                   controlPoints.at(i).at(j + 1).Y(),
                   controlPoints.at(i).at(j + 1).Z());
      } else if (i == numColumns() - 1 && j < numRows() - 1) { // Right column
        glColor3f(1, 0, 1);
        glVertex3f(controlPoints.at(i).at(j).X(), controlPoints.at(i).at(j).Y(),
                   controlPoints.at(i).at(j).Z());
        glVertex3f(controlPoints.at(i).at(j + 1).X(),
                   controlPoints.at(i).at(j + 1).Y(),
                   controlPoints.at(i).at(j + 1).Z());
      }
      glEnd();
      glColor3f(1, 1, 0); // Corner points
      if (j == 0 && i == 0)
        controlPoints.at(i).at(j).draw(8);
      glColor3f(1, 0, 0.5);
      if (j == 0 && i == numColumns() - 1)
        controlPoints.at(i).at(j).draw(8);
      glColor3f(0, 1, 1);
      if (j == numRows() - 1 && i == 0)
        controlPoints.at(i).at(j).draw(8);
      glColor3f(0.5, 0, 1);
      if (j == numRows() - 1 && i == numColumns() - 1)
        controlPoints.at(i).at(j).draw(8);
    }
  }
}

// Gets the control point at the given location in the control point array
Point *Patch::getPoint(int i, int j) { return &controlPoints.at(i).at(j); }

// Change the value of an existing point
void Patch::setPoint(int i, int j, Point value) {
  //  std::cerr<<"setPoint"<<std::endl;
  controlPoints.at(i).at(j) = value;
}

// Add a new point to the end of the given row
void Patch::addPoint(int i, Point value) {
  controlPoints.at(i).push_back(value);
}

// Adds the given vector of points as a row in the surface. If it's not the same
// length as the other rows, it will explode!!
void Patch::addRow(vector<Point> row) { surface.push_back(row); }

// Returns the number of rows in the surface
int Patch::numRows() { return controlPoints.size(); }

// Returns the number of columns in the surface
int Patch::numColumns() { return controlPoints.at(0).size(); }

// Clears both the surface points and the control points
void Patch::clear() {
  surface.clear();
  controlPoints.clear();
}

// Clears the contents of the rows of control points, but maintains the
// structure so that new information can be added
void Patch::clearRows() {
  for (unsigned int i = 0; i < controlPoints.size(); i++) {
    controlPoints.at(i).clear();
  }
}

// Flip all the control points in the surface horizontally about the YZ plane
// through the contact point
void Patch::flipHorizontal(Point pivot) {
  //    std::cerr<<"flipHorizontal"<<std::endl;

  for ( int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      controlPoints.at(i).at(j).setX(-controlPoints.at(i).at(j).X() + 2 * pivot.X());
    }
  }
}

// Flip all the control points in the surface vertically about the XZ plane
// through the contact point
void Patch::flipVertical(Point pivot) {
  //      std::cerr<<"flipVertical"<<std::endl;

  for ( int i = 0; i < numRows(); i++) {
    for ( int j = 0; j < numColumns(); j++) {
      controlPoints.at(i).at(j).setY(-controlPoints.at(i).at(j).Y() + 2 * pivot.Y());
    }
  }
}

// Flip all the control points in the surface depthwise about the XY plane
// through the contact point
void Patch::flipDepth(Point pivot) {
  //        std::cerr<<"flipDepth"<<std::endl;

  for ( int i = 0; i < numRows(); i++) {
    for ( int j = 0; j < numColumns(); j++) {
      controlPoints.at(i).at(j).setZ(-controlPoints.at(i).at(j).Z() + 2 * pivot.Z());
    }
  }
}

string Patch::getName() { return name; }

// Returns the center point of the patch, based on the min and max coordinates
// of the control points
Point Patch::getCenter() {
  double xmax = -10001;
  double xmin = 10001;
  double ymax = -10001;
  double ymin = 10001;
  double zmax = -10001;
  double zmin = 10001;
  for ( int i = 0; i < numRows(); i++) { // Look at the control points to find
                                        // the current maximum coordinates
    for (int j = 0; j < numColumns(); j++) {
      if (getPoint(i, j)->X() > xmax)
        xmax = getPoint(i, j)->X();
      if (getPoint(i, j)->X() < xmin)
        xmin = getPoint(i, j)->X();
      if (getPoint(i, j)->Y() > ymax)
        ymax = getPoint(i, j)->Y();
      if (getPoint(i, j)->Y() < ymin)
        ymin = getPoint(i, j)->Y();
      if (getPoint(i, j)->Z() > zmax)
        zmax = getPoint(i, j)->Z();
      if (getPoint(i, j)->Z() < zmin)
        zmin = getPoint(i, j)->Z();
    }
  }
  //  std::cerr<<xmin<<" - "<<xmax<<" - "<<xmin + xmax<<" - "<<zmin<<" - "<<zmax<<" - "<<zmin + zmax<<std::endl;
  return Point((xmin + xmax) / 2.0, (ymin + ymax) / 2.0,
               (zmin + zmax) /
                   2.0); // Get the center based on the min and max coordinates
}

// Sets the patch's name to the given string
void Patch::setName(string newName) { name = newName; }

void Patch::setVisibility(bool visibility) { visible = visibility; }

int Patch::getAdjacency(Adjacency adj) { return adjacencies[adj]; }

int Patch::getAdjacencyDirection(Adjacency adj) {
  return adjacencyDirections[adj];
}

string Patch::getAdjacencyDirectionString(Adjacency adj) {
  return adjToString(adjacencyDirections[adj]);
}

void Patch::setAdjacency(int value, int adj) { adjacencies[adj] = value; }

void Patch::setAdjacencyDirection(int value, int adj) {
  adjacencyDirections[adj] = value;
}

bool Patch::isVisible() { return visible; }

// Translates the patch by the given vector
void Patch::translate(Vector3 translation) {
  //        std::cerr<<"translate"<<std::endl;

  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      controlPoints.at(i).at(j) += translation;
    }
  }
}

// Rotates the selected patch about the given axis by the given angle
void Patch::rotate(Vector3 axis, double angle) {
  Point center = getCenter();
  //  std::cerr<<"Center: "<<center.X()<<" - "<<center.Y()<<" - "<<center.Z()<<std::endl;
  Vector3 controlVector;
  //  std::cerr<<"Rotate"<<std::endl;
  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      //[Pascal] I don't understand why there is a translation before the rotation
      //controlVector = controlPoints.at(i).at(j) - center;
      Point cp = controlPoints.at(i).at(j);
      controlVector = Vector3(cp.X(),cp.Y(),cp.Z());
      controlVector.rotate(axis, angle);
      /*      if ((i ==0) && (j == 1))
	std::cerr<<controlVector.X()<<" - "<<controlVector.Y()<<" - "<<controlVector.Z()<<std::endl;
      */
      // [Pascal] And a translation back
      //Point newControlPoint = Point(controlVector.X(), controlVector.Y(), controlVector.Z()) + center;
      Point newControlPoint = Point(controlVector.X(), controlVector.Y(), controlVector.Z());
      controlPoints.at(i).at(j) = Point(newControlPoint.X(), newControlPoint.Y(),newControlPoint.Z());
      //      if ((i ==0) && (j == 1))
      //	std::cerr<< controlPoints.at(i).at(j).X()<<" - "<< controlPoints.at(i).at(j).Y()<<" - "<< controlPoints.at(i).at(j).Z()<<std::endl;
      
    }
  }
}

// Scales the patch by the given scale vector
void Patch::scale(Vector3 factor) {
  //        std::cerr<<"scale"<<std::endl;

  Point center = getCenter();
  Vector3 centerVector = Vector3(center.X(), center.Y(), center.Z());

  Point currentPoint;
  for (int i = 0; i < numRows(); i++) {
    for (int j = 0; j < numColumns(); j++) {
      currentPoint =
          controlPoints.at(i).at(j) - centerVector; // Translate to the origin
      currentPoint *= factor;                   // Scale
      controlPoints.at(i).at(j) = currentPoint + centerVector; // Translate back
    }
  }
}

// Creates a new display list for this patch so that the surfaces of duplicated
// patches are unique
void Patch::makeNewDisplayList() { displayList = glGenLists(1); }

// Removes any adjacencies to the patch with the given index
void Patch::clearAdjacencies(int index) {
  for (int i = 0; i < 8; i++) {
    if (adjacencies[i] == index) {
      adjacencies[i] = -1;
      adjacencyDirections[i] = -1;
    } else if (adjacencies[i] >
               index) { // Decrement higher indices to match new values after
                        // current index is removed
      adjacencies[i]--;
    }
  }
}
