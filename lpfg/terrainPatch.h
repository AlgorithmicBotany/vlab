/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

/*
TerrainPatch.h
 Header file representing a single patch of terrain for use with
a Level of Detail algorithm for drawing terrain.

By: Steven Longay
June 13th 2008

Algorithm used is based off of that explained in the paper
Level of Detail for Terrain Geometry Images, 2007
Duncan Andrew Keith Mc Roberts and Alexandre Hardy
*/

#ifndef _TERR_PATCH
#define _TERR_PATCH

#include <iostream>
#include "glutils.h"
#include "drawparam.h"
#include "include/lintrfc.h"

using std::cout;
using std::endl;

struct Cracks {
  bool N, E, S, W;
};

class TerrainPatch {
public:
  TerrainPatch();
  void loadPatchFrom(V3d **PdataPoints, V3d **PvertexNormals, int PnumU,
                     int PnumV, int numRows, int numCols, int, int, float,
                     float);
  void OutputToPOVRay(std::ostream &stream, bool textured);
  int draw(bool showGrid);
  void drawGrid();
  void scaleTerrainBy(float);

  bool toBeDrawn;
  VisibilityMode visibilityMode;

  V3d topLeft;
  V3d bottomRight;
  V3d midPoint;
  double maxHeight;

  int _numU, _numV;
  V3d **_dataPoints;
  V3d **_vertexNormals;
  V2d **_texCoords;

  Cracks cracks;

private:
  void drawSolid();
  void fixCorners();
  void drawFixedBoarders();
};

#endif
