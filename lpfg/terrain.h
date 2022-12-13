/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

/*
        terrain.h
        Written by Steven Longay
        July 22 2008
*/

#ifndef __TERRAIN_H
#define __TERRAIN_H

#include "quadTree.h"
#include "terrainPatch.h"
#include "utils.h"
#include "exception.h"

#include <iostream>
#include <fstream>

using std::ifstream;

class TerrainData {
public:
  TerrainData() {
    NUM_LEVELS = 1;
    TERRAIN_SCALE = 1;
    DISTANCE_CONSTANT = 1;
    showGrid = false;
    _isTextured = false;
    _textureID = -1;
    uTile = vTile = 0.0;
  }

  // LPFG User functions
  bool terrainHeightAt(V3f worldSpacePoint, V3f &surfaceIntersectionPoint);
  bool terrainNormalAt(V3f worldSpacePoint, V3f &surfaceNormal);
  void terrainVisibilityAll(VisibilityMode, QuadTree<TerrainPatch> *curr = 0);
  void terrainVisibilityPatch(VisibilityMode, int, V3f);
  void scaleTerrainBy(float, QuadTree<TerrainPatch> *curr = 0);
  //

  void draw(CameraPosition);
  void OutputToPOVRay(std::ostream &stream);
  void OutputUnionToPOVRay(std::ostream &stream);
  void loadTerrain(const char *fn);
  bool IsTextured();
  int TextureId();
  void getVolume(Volume &, QuadTree<TerrainPatch> *curr = 0);
  float getScale();

private:
  void loadTerrain2(const char *fn);
  void OutputToPOVRay2(QuadTree<TerrainPatch> *curr, std::ostream &stream);
  void OutputUnionToPOVRay2(QuadTree<TerrainPatch> *curr, std::ostream &stream);
  void calculateResolutions();
  void fillLeaves(QuadTree<TerrainPatch> *curr);
  void subDivide(QuadTree<TerrainPatch> *curr);
  void fillTerrainData(QuadTree<TerrainPatch> *curr, int rowOff, int colOff);
  bool distCheck(V3d pos, int level);
  double calculateRi(int i);
  Cracks checkForCracks(QuadTree<TerrainPatch> *curr);
  int drawGridLOD(QuadTree<TerrainPatch> *curr);
  void setDrawFlags(QuadTree<TerrainPatch> *curr);
  void clearDrawFlags(QuadTree<TerrainPatch> *curr);
  void markCracks(QuadTree<TerrainPatch> *curr);
  void setGlobalPointers(QuadTree<TerrainPatch> *curr);
  QuadTree<TerrainPatch> *
  getNearestPatch(V3d point, QuadTree<TerrainPatch> *terrainTree, int);

  V3d cameraPos;
  QuadTree<TerrainPatch> *LODGrid;
  V3d ***dataPoints, ***vertexNormals;
  int *numU, *numV;

  float TERRAIN_WIDTH, ORIGINAL_TERRAIN_WIDTH, TERRAIN_SCALE, DISTANCE_CONSTANT;
  int NUM_LEVELS;
  bool showGrid;

  double *Ri;

  bool _isTextured;
  int _textureID;
  float uTile, vTile;
};

extern TerrainData *terrainData;

#endif
