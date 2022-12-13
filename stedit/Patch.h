/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

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

#include "Colour.h"
#include <string>
#include <vector>
using namespace std;
#include "Point.h"

class Patch {
public:
  enum Adjacency { AL, A, AR, L, R, BL, B, BR };
  Patch(string name = "Patch_1", int shape = Patch::Square);

  static const int Square = 0; // Patch shape IDs
  static const int Triangle = 1;

  Point *getPoint(int i, int j);
  void setPoint(int i, int j, Point value);
  void addPoint(int i, Point value);
  void subdivide(int samples);
  void draw();
  void drawLines(double width, Colour colour);
  void drawPoints(double size, Colour colour);
  void drawPoints(double size, Colour colour, Colour selectedColour,
                  int selectedI, int selectedJ, bool symmetrical14,
                  bool symmetrical113);
  void drawAdjacencyEdges();
  void addRow(vector<Point> row);
  int numRows();
  int numColumns();
  void clear();
  void clearRows();
  void flipHorizontal(Point pivot);
  void flipVertical(Point pivot);
  void flipDepth(Point pivot);
  string getName();
  void setName(string newName);
  void setVisibility(bool visibility);
  Point getCenter();
  void setAdjacencyStrings(string alPatch, string aPatch, string arPatch,
                           string lPatch, string rPatch, string blPatch,
                           string bPatch, string brPatch);
  void setAdjacencyDirections(string alDir, string aDir, string arDir,
                              string lDir, string rDir, string blDir,
                              string bDir, string brDir);
  void indexAdjacency(string adjacencyName, int index);
  bool isVisible();
  void translate(Vector3 translation);
  void rotate(Vector3 axis, double angle);
  void scale(Vector3 factor);
  void makeNewDisplayList();
  void clearAdjacencies(int index);

  int getAdjacency(Adjacency adj);
  int getAdjacencyDirection(Adjacency adj);
  string getAdjacencyDirectionString(Adjacency adj);
  void setAdjacency(int value, int adj);
  void setAdjacencyDirection(int value, int adj);

  // Consider two patches equal simply if their control points have the same
  // positions, and they have the same adjacency information
  bool operator==(Patch p) {
    if (adjacencies != p.adjacencies)
      return false;
    if (adjacencyDirections != p.adjacencyDirections)
      return false;
    for (int i = 0; i < numRows(); i++) {
      for (int j = 0; j < numColumns(); j++) {
        if (controlPoints.at(i).at(j) != p.controlPoints.at(i).at(j))
          return false;
      }
    }
    return true;
  }

  bool operator!=(Patch p) { return !(*this == p); }

private:
  vector<Point> calcBezier(const vector<Point> points, const int samples);
  vector<vector<Point>> transpose(vector<vector<Point>> surface);
  void recalcSurface(int samples);
  void calcFaceNormals();
  Vector3 getVertexNormal(int i, int j);
  int stringToAdj(string str);
  string adjToString(int adj);

  vector<vector<Point>> controlPoints; // List of control points for the surface
  vector<vector<Point>>
      surface; // List of subdivided points for drawing the surface
  // List of normals of all the faces in the surface.
  // The first two vectors represent the index location of the upper left point
  // of the faces. The third vector contains the normals of the two faces that
  // fill the grid square at that location. (in the order: lower left, upper
  // right)
  vector<vector<vector<Vector3>>> faceNormals;

  GLuint displayList; // GL display list containing the information of the
                      // subdivided patch
  string name; // The name of the patch, used to store adjacency information
  int shape;   // The initial shape of the patch, also used to determine texture
               // map orientation
  bool visible; // Specifies whether or not the patch is currently visible

  int adjacencies[8];         // Adjacencies
  int adjacencyDirections[8]; // The side of each patch that is adjacent to this
                              // one
  string adjacencyStrings[8]; // Temporary adjacency strings (for before the
                              // index is known)
};
