/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <GL/glu.h>
#endif
#include "Colour.h"
#include "Patch.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
#include "Point.h"

class Surface {
private:
  vector<Patch> patches; // List of patches for the surface
  int selectedPatch;     // The index of the currently selected patch
  Point contactPoint;    // The point where the surface should contact to the
                         // turtle path
  Point endPoint; // The point the 'turtle' will reach after drawing the surface
  Vector3 heading; // The heading vector of the turtle when drawing the surface
  Vector3 up;      // The up vector of the turtle when drawing the surface
  double size; // A scaling factor used when drawing the surface; specifies the
               // length of one unit
  int topColour; // This stuff is probably unneccessary due to the presence of
                 // textures, but we need to remember it
  double topDiffuse;
  int bottomColour;
  double bottomDiffuse;
  bool steditVersion;
  bool forceSteditVersion;

  string getAdjacencyName(int index);
  bool isValidIndex(int index);

public:
  int selectedPointI; // The i index of the selected point in the surface
  int selectedPointJ; // The j index of the selected point in the surface

  Surface(bool extendedFormat = false);
  bool load(string filename);
  void save(string filename);
  Point *getPoint(int i, int j);
  Point *getSelectedPoint();
  Point *getSymmetryPoint14();
  Point *getSymmetryPoint113();
  Point *getSymmetryPointOpposite();
  Point getContactPoint();
  Point getEndPoint();
  Vector3 getHeading();
  Vector3 getUp();
  double getSize();
  void setContactPoint(Point point);
  void setEndPoint(Point point);
  void setHeading(Vector3 vector);
  void setUp(Vector3 vector);
  void setSize(double value);
  void setPoint(int i, int j, Point value);
  void subdivide(int samples);
  void draw(Colour blankColour, Colour selectedColour);
  void draw(Colour colour, double width);
  void drawPatch(int index);
  void drawPatchAdjacencyEdges(int index);
  void drawLines(double width, Colour colour);
  void drawPoints(double size, double contactSize, Colour colour,
                  Colour selectedColour, Colour contactColour,
                  bool symmetrical14, bool symmetrical113);
  void drawVectors(double size, Colour colour);
  void selectPoint(int i, int j);
  int numRows();
  int numColumns();
  int numPatches();
  void clear();
  void flipHorizontal();
  void flipVertical();
  void flipDepth();
  int getSelectedPointI();
  int getSelectedPointJ();
  string getPatchName(int index);
  vector<string> getPatchNames();
  void setSelectedPatch(int index);
  Point getCenter();
  void changePatchName(string name, int index);
  void changePatchVisibility(bool visibility, int index);
  void addNewPatch(string name, int shape);
  void deletePatch(int index);
  bool duplicatePatch(int index, string name);
  void updateAdjacentEdges(int patchIndex);
  void updateCornerPoint(int patchIndex, Patch::Adjacency adj, Point *corner);
  void updateEdge(int patchIndex, Patch::Adjacency adj, bool horizontal,
                  int index);
  void translate(Vector3 translation);
  void rotate(Vector3 axis, double angle);
  void scale(Vector3 factor);
  int getSelectedPatch();
  Patch *getPatch(int index);

  // Consider two surfaces equal if their patches are equal
  bool operator==(Surface s) {
    if (patches.size() != s.patches.size())
      return false;
    for (unsigned int i = 0; i < patches.size(); i++) {
      if (patches.at(i) != s.patches.at(i))
        return false;
    }
    return true;
  }

  bool operator!=(Surface s) { return !(*this == s); }
};
