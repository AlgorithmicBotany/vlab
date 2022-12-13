/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/
#define GL_SILENCE_DEPRECATION
#include "Surface.h"

// Initialize a default 4x4 surface
Surface::Surface(bool extendedFormat) {
  selectedPointI = 0;
  selectedPointJ = 0;
  selectedPatch = 0;

  contactPoint = Point();
  endPoint = Point();
  heading = Vector3(0, 1, 0);
  up = Vector3(0, 0, 1);
  size = 1;
  topColour = 1;
  topDiffuse = 0;
  bottomColour = 1;
  bottomDiffuse = 0;

  steditVersion = false;
  forceSteditVersion = extendedFormat;
}

// Loads the .s file with the given filename as a surface
bool Surface::load(string filename) {
  ifstream inFile;
  inFile.open(filename.c_str());
  if (!inFile.good())
    return false;

  clear(); // Clear the previous information

  double x, y, z;
  int c;
  steditVersion = false;
  string garbage, name, alPatch, aPatch, arPatch, lPatch, rPatch, blPatch,
      bPatch, brPatch, alDir, aDir, arDir, lDir, rDir, blDir, bDir, brDir;

  inFile >> garbage;
  if (garbage == "VERSION") {
    inFile >> garbage;
    if (garbage == "stedit")
      steditVersion = true;
    inFile >> garbage;
  }
  inFile >> garbage >> garbage >> garbage >> garbage >>
      garbage; // xmin xmax ymin ymax zmin zmax; unneccessary, it will be
               // recalculated later
  inFile >> garbage;
  if (garbage == "PRECISION")
    inFile >> garbage >> garbage >> garbage >> garbage >>
        garbage; // PRECISION		S: ### T: ###; This is garbage
  inFile >> garbage >> garbage >> x >> garbage >> y >> garbage >>
      z; // CONTACT POINT	X: ### Y: ### Z: ###
  contactPoint = Point(x, y, z);
  inFile >> garbage >> garbage >> garbage >> x >> garbage >> y >> garbage >>
      z; // END POINT		X: ### Y: ### Z: ###
  endPoint = Point(x, y, z);
  inFile >> garbage >> garbage >> x >> garbage >> y >> garbage >>
      z; // HEADING			X: ### Y: ### Z: ###
  heading = Vector3(x, y, z);
  inFile >> garbage >> garbage >> x >> garbage >> y >> garbage >>
      z; // UP				X: ### Y: ### Z: ###
  up = Vector3(x, y, z);
  inFile >> garbage >> x; // SIZE: ###
  size = x;
  while (inFile >> name) { // patchName
    inFile >> garbage >> garbage >> c >> garbage >>
        x; // TOP COLOR: ### DIFFUSE: ###
    topColour = c;
    topDiffuse = x;
    inFile >> garbage >> garbage >> c >> garbage >>
        x; // BOTTOM COLOR: ### DIFFUSE: ###
    bottomColour = c;
    bottomDiffuse = x;
    if (steditVersion) {
      inFile >> garbage >> alPatch >> alDir >> garbage >> aPatch >> aDir >>
          garbage >> arPatch >> arDir; // AL: ~ ~ A: ~ ~ AR: ~ ~
      inFile >> garbage >> lPatch >> lDir >> garbage >> rPatch >>
          rDir; // L: ~ ~ R: ~ ~
      inFile >> garbage >> blPatch >> blDir >> garbage >> bPatch >> bDir >>
          garbage >> brPatch >> brDir; // BL: ~ ~ B: ~ ~ BR: ~ ~
    } else {
      inFile >> garbage >> alPatch >> garbage >> aPatch >> garbage >>
          arPatch;                                      // AL: ~ A: ~ AR: ~
      inFile >> garbage >> lPatch >> garbage >> rPatch; // L: ~ R: ~
      inFile >> garbage >> blPatch >> garbage >> bPatch >> garbage >>
          brPatch; // BL: ~ B: ~ BR: ~
    }
    patches.push_back(Patch(name));
    patches.back().setAdjacencyStrings(alPatch, aPatch, arPatch, lPatch, rPatch,
                                       blPatch, bPatch, brPatch);
    patches.back().setAdjacencyDirections(alDir, aDir, arDir, lDir, rDir, blDir,
                                          bDir, brDir);
    for (unsigned int i = 0; i < 4; i++) { // Control Points
      for (unsigned int j = 0; j < 4; j++) {
        inFile >> x >> y >> z;
        patches.back().setPoint(j, i, Point(x, y, z));
      }
    }
  }
  inFile.close();
  for (unsigned int i = 0; i < patches.size(); i++) {
    for (unsigned int j = 0; j < patches.size(); j++) {
      patches.at(j).indexAdjacency(patches.at(i).getName(), i);
    }
  }
  return true;
}

// Saves the current surface as a .s file with the given filename
void Surface::save(string filename) {
  ofstream outFile;
  outFile.open(filename.c_str());
  outFile.precision(2); // Output values with two decimal places

  double xmax = -10001;
  double xmin = 10001;
  double ymax = -10001;
  double ymin = 10001;
  double zmax = -10001;
  double zmin = 10001;
  for (int i = 0; i < numRows(); i++) { // Look at the control points to find
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

  if (steditVersion || forceSteditVersion)
    outFile << "VERSION stedit" << endl;
  outFile << fixed << xmin << " " << xmax << "   " << ymin << " " << ymax
          << "   " << zmin << " " << zmax << endl;
  outFile << fixed << "CONTACT POINT  X: " << contactPoint.X()
          << " Y: " << contactPoint.Y() << " Z: " << contactPoint.Z() << endl;
  outFile << fixed << "END POINT  X: " << endPoint.X() << " Y: " << endPoint.Y()
          << " Z: " << endPoint.Z() << endl;
  outFile << fixed << "HEADING  X: " << heading.X() << " Y: " << heading.Y()
          << " Z: " << heading.Z() << endl;
  outFile << fixed << "UP  X: " << up.X() << " Y: " << up.Y() << " Z: " << up.Z()
          << endl;
  outFile << fixed << "SIZE: " << size << endl;
  for (unsigned int index = 0; index < patches.size(); index++) {
    outFile << patches.at(index).getName() << endl;
    outFile << fixed << "TOP COLOR: " << topColour << " DIFFUSE: " << topDiffuse
            << " BOTTOM COLOR: " << bottomColour
            << " DIFFUSE: " << bottomDiffuse << endl;
    if (steditVersion || forceSteditVersion) {
      outFile << "AL: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::AL))
              << " "
              << patches.at(index).getAdjacencyDirectionString(Patch::AL);
      outFile << " A: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::A))
              << " " << patches.at(index).getAdjacencyDirectionString(Patch::A);
      outFile << " AR: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::AR))
              << " " << patches.at(index).getAdjacencyDirectionString(Patch::AR)
              << endl;

      outFile << "L: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::L))
              << " " << patches.at(index).getAdjacencyDirectionString(Patch::L);
      outFile << " R: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::R))
              << " " << patches.at(index).getAdjacencyDirectionString(Patch::R)
              << endl;

      outFile << "BL: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::BL))
              << " "
              << patches.at(index).getAdjacencyDirectionString(Patch::BL);
      outFile << " B: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::B))
              << " " << patches.at(index).getAdjacencyDirectionString(Patch::B);
      outFile << " BR: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::BR))
              << " " << patches.at(index).getAdjacencyDirectionString(Patch::BR)
              << endl;
    } else {
      outFile << "AL: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::AL));
      outFile << " A: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::A));
      outFile << " AR: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::AR))
              << endl;

      outFile << "L: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::L));
      outFile << " R: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::R))
              << endl;

      outFile << "BL: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::BL));
      outFile << " B: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::B));
      outFile << " BR: "
              << getAdjacencyName(patches.at(index).getAdjacency(Patch::BR))
              << endl;
    }
    Point currentPoint;
    for (int i = 0; i < numRows(); i++) {
      for (int j = 0; j < numColumns(); j++) {
        currentPoint = *patches.at(index).getPoint(j, i);
        outFile << currentPoint.X() << " " << currentPoint.Y() << " "
                << currentPoint.Z() << "  ";
      }
      outFile << endl;
    }
  }
  outFile.close();
}

string Surface::getAdjacencyName(int index) {
  if (index < 0)
    return "~";
  else if ((size_t)index >= patches.size())
    return "?";
  else
    return patches.at(index).getName();
}

// Draw the surface as a wireframe
// The parameters are the colour and width of the wireframe lines
void Surface::draw(Colour colour, double width) {
  glLineWidth(width);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Set to wireframe
  glDisable(GL_ALPHA_TEST);                  // Turn of all this stuff
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_LIGHTING);
  glColor3f(colour.r, colour.g, colour.b);

  for (unsigned int i = 0; i < patches.size(); i++) {
    //if (patches.at(i).isVisible())
      patches.at(i).draw();
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Reset from wireframe
  glEnable(GL_LIGHTING);                     // Reenable stuff
  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_ALPHA_TEST);
}

// Draw the surface with triangles and textures
void Surface::draw(Colour blankColour, Colour selectionColour) {
  for (unsigned int i = 0; i < patches.size(); i++) {
    //if (patches.at(i).isVisible()) {
      if (patches.size() > 1 && (size_t)selectedPatch == i)
        glColor3f(selectionColour.r + 0.25, selectionColour.g + 0.25,
                  selectionColour.b + 0.25);
      else
        glColor3f(blankColour.r, blankColour.g, blankColour.b);
      patches.at(i).draw();
      // }
  }
}

// Draw the indicated patch
void Surface::drawPatch(int index) {
  if (isValidIndex(index))
    patches.at(index).draw();
}

// Draw the indicated patch
void Surface::drawPatchAdjacencyEdges(int index) {
  if (isValidIndex(index))
    patches.at(index).drawAdjacencyEdges();
}

// Draw the grid lines between the control points
void Surface::drawLines(double width, Colour colour) {
  for (unsigned int i = 0; i < patches.size(); i++) {
    // if (patches.at(i).isVisible()) {
      if (patches.size() > 1 && (size_t)selectedPatch == i)
        patches.at(i).drawLines(
            width, Colour(colour.r + 0.5, colour.g / 2, colour.b / 2));
      else
        patches.at(i).drawLines(width, colour);
      //}
  }
}

// Draw the control points
void Surface::drawPoints(double size, double contactSize, Colour colour,
                         Colour selectedColour, Colour contactColour,
                         bool symmetrical14, bool symmetrical113) {
  glColor3f(contactColour.r, contactColour.g, contactColour.b);
  contactPoint.draw(contactSize); // Draw the contact point at a larger size and
                                  // in a different colour than the other points
  for (unsigned int i = 0; i < patches.size(); i++) {
    //    if (patches.at(i).isVisible()) {
      if ((size_t)selectedPatch == i)
        patches.at(i).drawPoints(size, colour, selectedColour, selectedPointI,
                                 selectedPointJ, symmetrical14, symmetrical113);
      else
        patches.at(i).drawPoints(size, colour);
      //}
  }
}

// Draw the heading and up vectors
void Surface::drawVectors(double width, Colour colour) {
  Point point;
  Vector3 direction, axis;
  glLineWidth(width);
  glColor3f(colour.r, colour.g, colour.b);
  glBegin(GL_LINES);
  point = contactPoint; // Arrow shaft
  glVertex3f(point.X(), point.Y(), point.Z());
  point += heading * size;
  glVertex3f(point.X(), point.Y(), point.Z());
  glEnd();

  glPushMatrix(); // Arrow head cone
  GLUquadricObj *q = gluNewQuadric();
  glTranslated(point.X(), point.Y(), point.Z());
  Vector3 d = heading.normalize();
  Vector3 n = Vector3::cross(heading, up).normalize();
  Vector3 b = Vector3::cross(heading, n).normalize();
  GLfloat frameMatrix[16] = {d.Xf(), d.Yf(), d.Zf(), 0, n.Xf(), n.Yf(), n.Zf(), 0,
                             b.Xf(), b.Yf(), b.Zf(), 0, 0,   0,   0,   1};
  glMultMatrixf(frameMatrix); // Align with the heading vector
  glRotated(90, 0, 1, 0);
  gluCylinder(q, (size * sqrt(width)) / 32.0, 0, (size * sqrt(width)) / 16.0,
              24, 1);
  gluDeleteQuadric(q);
  glPopMatrix();

  glBegin(GL_LINES);
  point = contactPoint; // Arrow shaft
  glVertex3f(point.X(), point.Y(), point.Z());
  point += up * size;
  glVertex3f(point.X(), point.Y(), point.Z());
  glEnd();

  glPushMatrix(); // Arrow head cone
  GLUquadricObj *q2 = gluNewQuadric();
  glTranslated(point.X(), point.Y(), point.Z());
  d = up.normalize();
  n = Vector3::cross(up, heading).normalize();
  b = Vector3::cross(up, n).normalize();
  GLfloat frameMatrix2[16] = {d.Xf(), d.Yf(), d.Zf(), 0, n.Xf(), n.Yf(), n.Zf(), 0,
                              b.Xf(), b.Yf(), b.Zf(), 0, 0,   0,   0,   1};
  glMultMatrixf(frameMatrix2); // Align with the up vector
  glRotated(90, 0, 1, 0);
  gluCylinder(q2, (size * sqrt(width)) / 32.0, 0, (size * sqrt(width)) / 16.0,
              24, 1);
  gluDeleteQuadric(q2);
  glPopMatrix();
}

// Gets the control point at the given location in the control point array
Point *Surface::getPoint(int i, int j) {
  if (isValidIndex(selectedPatch))
    return patches.at(selectedPatch).getPoint(i, j);
  else
    return 0;
}

// Returns the currently selected point
Point *Surface::getSelectedPoint() {
  if (isValidIndex(selectedPatch))
    return patches.at(selectedPatch).getPoint(selectedPointI, selectedPointJ);
  else
    return 0;
}

// Returns the point symmetrically opposed to the selected point about the 1-4
// axis
Point *Surface::getSymmetryPoint14() {
  if (isValidIndex(selectedPatch))
    return patches.at(selectedPatch)
        .getPoint(numRows() - selectedPointI - 1, selectedPointJ);
  else
    return 0;
}

// Returns the point symmetrically opposed to the selected point about the 1-13
// axis
Point *Surface::getSymmetryPoint113() {
  if (isValidIndex(selectedPatch))
    return patches.at(selectedPatch)
        .getPoint(selectedPointI, numColumns() - selectedPointJ - 1);
  else
    return 0;
}

// Returns the point symmetrically opposed to the selected point about both axes
Point *Surface::getSymmetryPointOpposite() {
  if (isValidIndex(selectedPatch))
    return patches.at(selectedPatch)
        .getPoint(numRows() - selectedPointI - 1,
                  numColumns() - selectedPointJ - 1);
  else
    return 0;
}

Point Surface::getContactPoint() { return contactPoint; }

Point Surface::getEndPoint() { return endPoint; }

Vector3 Surface::getHeading() { return heading; }

Vector3 Surface::getUp() { return up; }

double Surface::getSize() { return size; }

void Surface::setContactPoint(Point point) { contactPoint = point; }

void Surface::setEndPoint(Point point) { endPoint = point; }

void Surface::setHeading(Vector3 vector) { heading = vector; }

void Surface::setUp(Vector3 vector) { up = vector; }

void Surface::setSize(double value) { size = value; }

// Change the value of an existing point
void Surface::setPoint(int i, int j, Point value) {
  if (isValidIndex(selectedPatch))
    patches.at(selectedPatch).setPoint(i, j, value);
}

// Subdivides the current surface
void Surface::subdivide(int samples) {
  //  std::cerr<<"Surface subdivide"<<std::endl;
  for (unsigned int i = 0; i < patches.size(); i++) {
    patches.at(i).subdivide(samples);
  }
  //    std::cerr<<"End Surface subdivide"<<std::endl;

}

// Sets the selected point to the point at the given location in the control
// point array
void Surface::selectPoint(int i, int j) {
  selectedPointI = i;
  selectedPointJ = j;
}

// Returns the number of rows in the surface
int Surface::numRows() {
  if (isValidIndex(selectedPatch))
    return patches.at(selectedPatch).numRows();
  else
    return 0;
}

// Returns the number of columns in the surface
int Surface::numColumns() {
  if (isValidIndex(selectedPatch))
    return patches.at(selectedPatch).numColumns();
  else
    return 0;
}

// Clears both the surface points and the control points
void Surface::clear() {
  patches.clear();
  selectedPatch = 0;
}

// Flip all the control points in the surface horizontally about the YZ plane
// through the contact point
void Surface::flipHorizontal() {
  if (isValidIndex(selectedPatch))
    patches.at(selectedPatch).flipHorizontal(contactPoint);
}

// Flip all the control points in the surface vertically about the XZ plane
// through the contact point
void Surface::flipVertical() {
  if (isValidIndex(selectedPatch))
    patches.at(selectedPatch).flipVertical(contactPoint);
}

// Flip all the control points in the surface depthwise about the XY plane
// through the contact point
void Surface::flipDepth() {
  if (isValidIndex(selectedPatch))
    patches.at(selectedPatch).flipDepth(contactPoint);
}

// Gets the i index of the selected point in the surface
int Surface::getSelectedPointI() { return selectedPointI; }

// Gets the j index of the selected point in the surface
int Surface::getSelectedPointJ() { return selectedPointJ; }

// Returns the name of the patch at the given index in the patch list
string Surface::getPatchName(int index) {
  if (isValidIndex(index))
    return patches.at(index).getName();
  else
    return "no_patch";
}

// Returns all the patch names as a vector
vector<string> Surface::getPatchNames() {
  vector<string> names;
  for (unsigned int i = 0; i < patches.size(); i++) {
    names.push_back(patches.at(i).getName());
  }
  return names;
}

// Sets the seleccted patch index to the given index
void Surface::setSelectedPatch(int index) {
  selectedPatch = index;
  if ((size_t)selectedPatch >= patches.size())
    selectedPatch = patches.size() - 1;
  else if (selectedPatch < 0)
    selectedPatch = 0;
}

// Returns the center point of the surface by averaging the centers of all the
// pathes
Point Surface::getCenter() {
  if (patches.empty())
    return Point(); // If there are no patches, return the origin
  Point center = Point();
  for (unsigned int i = 0; i < patches.size(); i++) {
    center += patches.at(i).getCenter(); // Add the centers of all the patches
  }
  return center /
         patches
             .size(); // Devide by the number pf patches to get the center point
}

// Changes the name of the patch at the given index to the given string
void Surface::changePatchName(string name, int index) {
  if (isValidIndex(index))
    patches.at(index).setName(name);
}

// Sets the visibility of the patch at the given index
void Surface::changePatchVisibility(bool visibility, int index) {
  if (isValidIndex(index))
    patches.at(index).setVisibility(visibility);
}

// Adds a new patch with the given name and the given shape (square or triangle)
void Surface::addNewPatch(string name, int shape) {
  patches.push_back(Patch(name, shape));
}

// Deletes the patch at the given index
void Surface::deletePatch(int index) {
  if (isValidIndex(index)) {
    for (unsigned int i = 0; i < patches.size(); i++) {
      patches.at(i).clearAdjacencies(
          index); // Remove references to the patch that was deleted
    }
    patches.erase(patches.begin() + index); // Delete the patch
  }
}

// Duplicates the patch at the given index, and gives it the given name
bool Surface::duplicatePatch(int index, string name) {
  if (isValidIndex(index)) {
    Patch duplicate = patches.at(index);
    duplicate.setName(name);
    duplicate.makeNewDisplayList(); // Give the new patch a unique display list
    patches.push_back(duplicate);
    return true; // Report success of the duplication operation
  }
  return false; // Return false if the duplication failed
}

// Returns the number of patches in the surface
int Surface::numPatches() { return patches.size(); }

// Moves the point at the corner of a patch corresponding to the given adjacency
// to the position of the given corner point
void Surface::updateCornerPoint(int patchIndex, Patch::Adjacency adj,
                                Point *corner) {
  int rows = patches.at(patchIndex).numRows();
  int columns = patches.at(patchIndex).numColumns();

  int index = patches.at(patchIndex).getAdjacency(adj);

  if (index != -1) {
    int adj2 =
        patches.at(patchIndex)
            .getAdjacencyDirection(adj); // Get the corner of the adjacent patch
                                         // that is part of this adjacency
    switch (adj2) {
    case Patch::AL: // Snap the above left to the given corner
      *patches.at(index).getPoint(0, 0) = *corner;
      break;
    case Patch::AR: // Snap the above right to the given corner
      *patches.at(index).getPoint(columns - 1, 0) = *corner;
      break;
    case Patch::BL: // Snap the below left to the given corner
      *patches.at(index).getPoint(0, rows - 1) = *corner;
      break;
    case Patch::BR: // Snap the below right to the given corner
      *patches.at(index).getPoint(columns - 1, rows - 1) = *corner;
      break;
    default:
      break;
    }
  }
}

// Moves the edge at the side of a patch corresponding to the given adjacency to
// the position of the edge The edge is given by an indicator of whether is is
// horizontal or not, and the index of the row or column
void Surface::updateEdge(int patchIndex, Patch::Adjacency adj, bool horizontal,
                         int edgeIndex) {
  int rows = patches.at(patchIndex).numRows();
  int columns = patches.at(patchIndex).numColumns();

  int index = patches.at(patchIndex).getAdjacency(adj);

  if (index != -1) {
    int adj2 = patches.at(patchIndex).getAdjacencyDirection(adj);
    switch (adj2) {
    case Patch::A:
      for (int i = 0; i < columns; i++) {
        if (horizontal)
          *patches.at(index).getPoint(i, 0) =
              *patches.at(patchIndex).getPoint(i, edgeIndex);
        else
          *patches.at(index).getPoint(i, 0) =
              *patches.at(patchIndex).getPoint(edgeIndex, i);
      }
      break;
    case Patch::L:
      for (int i = 0; i < columns; i++) {
        if (horizontal)
          *patches.at(index).getPoint(0, i) =
              *patches.at(patchIndex).getPoint(i, edgeIndex);
        else
          *patches.at(index).getPoint(0, i) =
              *patches.at(patchIndex).getPoint(edgeIndex, i);
      }
      break;
    case Patch::R:
      for (int i = 0; i < columns; i++) {
        if (horizontal)
          *patches.at(index).getPoint(columns - 1, i) =
              *patches.at(patchIndex).getPoint(i, edgeIndex);
        else
          *patches.at(index).getPoint(columns - 1, i) =
              *patches.at(patchIndex).getPoint(edgeIndex, i);
      }
      break;
    case Patch::B:
      for (int i = 0; i < columns; i++) {
        if (horizontal)
          *patches.at(index).getPoint(i, rows - 1) =
              *patches.at(patchIndex).getPoint(i, edgeIndex);
        else
          *patches.at(index).getPoint(i, rows - 1) =
              *patches.at(patchIndex).getPoint(edgeIndex, i);
      }
      break;
    default:
      break;
    }
  }
}

// Updates all edges and corners of the patches adjacent to the selected patch
// to match with the adjacencies assigned to the selected patch
void Surface::updateAdjacentEdges(int patchIndex) {
  if (isValidIndex(patchIndex)) {
    int rows = patches.at(patchIndex).numRows();
    int columns = patches.at(patchIndex).numColumns();

    updateCornerPoint(patchIndex, Patch::AL,
                      patches.at(patchIndex).getPoint(0, 0));
    updateEdge(patchIndex, Patch::A, true, 0);
    updateCornerPoint(patchIndex, Patch::AR,
                      patches.at(patchIndex).getPoint(columns - 1, 0));

    updateEdge(patchIndex, Patch::L, false, 0);
    updateEdge(patchIndex, Patch::R, false, columns - 1);

    updateCornerPoint(patchIndex, Patch::BL,
                      patches.at(patchIndex).getPoint(0, rows - 1));
    updateEdge(patchIndex, Patch::B, true, rows - 1);
    updateCornerPoint(patchIndex, Patch::BR,
                      patches.at(patchIndex).getPoint(columns - 1, rows - 1));
  }
}

// Checks if the given index is a valid index, that is, it is greater than or
// equal to zero, but does not exceed the number of patches
bool Surface::isValidIndex( int index) {
  return (index >= 0 && ((unsigned int)index < patches.size()));
}

// Translates the selected patch by the given vector
void Surface::translate(Vector3 translation) {
  if (isValidIndex(selectedPatch))
    patches.at(selectedPatch).translate(translation);
}

// Rotates the selected patch about the given axis by the given angle
void Surface::rotate(Vector3 axis, double angle) {
  if (isValidIndex(selectedPatch))
    patches.at(selectedPatch).rotate(axis, angle);
}

// Scales the selected patch by the given scale vector
void Surface::scale(Vector3 factor) {
  if (isValidIndex(selectedPatch))
    patches.at(selectedPatch).scale(factor);
}

// Returns the index of the selected patch
int Surface::getSelectedPatch() { return selectedPatch; }

// Returns a pointer to the patch at the given index
Patch *Surface::getPatch(int index) { return &patches.at(index); }
