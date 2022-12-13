/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "terrain.h"

TerrainData *terrainData = NULL;
int _outputCount = 0;

#define _toVector3d(a) Vector3d((float)a.x, (float)a.y, (float)a.z)
void TerrainData::getVolume(Volume &v, QuadTree<TerrainPatch> *curr) {
  if (curr == NULL) {
    curr = LODGrid;
  }
  v.Adapt(_toVector3d(curr->data->topLeft));
  v.Adapt(_toVector3d(curr->data->bottomRight));

  if (curr->level != NUM_LEVELS - 1) {
    getVolume(v, curr->getNE());
    getVolume(v, curr->getNW());
    getVolume(v, curr->getSE());
    getVolume(v, curr->getSW());
  }
}

float TerrainData::getScale() { return TERRAIN_SCALE; }

bool TerrainData::IsTextured() { return _isTextured; }
int TerrainData::TextureId() { return _textureID; }

#define _DumpVector(a) '<' << a.x << ',' << a.y << ',' << a.z << '>'
void TerrainData::OutputUnionToPOVRay2(QuadTree<TerrainPatch> *curr,
                                       std::ostream &stream) {
  if (curr->level == NUM_LEVELS - 1) {
    stream
        << "object { Terrain_" << _outputCount
        << " }\n";
    ++_outputCount;
  } else {
    OutputUnionToPOVRay2(curr->getNE(), stream);
    OutputUnionToPOVRay2(curr->getNW(), stream);
    OutputUnionToPOVRay2(curr->getSE(), stream);
    OutputUnionToPOVRay2(curr->getSW(), stream);
  }
}
void TerrainData::OutputUnionToPOVRay(std::ostream &stream) {
  _outputCount = 0;
  stream << "//Terrain" << std::endl;
  OutputUnionToPOVRay2(LODGrid, stream);
  stream << "//End_Terrain" << std::endl;
}
void TerrainData::OutputToPOVRay(std::ostream &stream) {
  _outputCount = 0;
  // Output information for each patch at the highest level
  stream << "//Start Terrain Defines " << std::endl;
  OutputToPOVRay2(LODGrid, stream);
  stream << "//End Terrain Defines " << std::endl;
}

void TerrainData::OutputToPOVRay2(QuadTree<TerrainPatch> *curr,
                                  std::ostream &stream) {
  if (curr->level == NUM_LEVELS - 1) {
    stream << "#declare Terrain_" << _outputCount << " = mesh { " << std::endl;
    curr->data->OutputToPOVRay(stream, _isTextured);
    stream << "}" << std::endl;
    stream << std::endl;
    ++_outputCount;
  } else {
    OutputToPOVRay2(curr->getNE(), stream);
    OutputToPOVRay2(curr->getNW(), stream);
    OutputToPOVRay2(curr->getSE(), stream);
    OutputToPOVRay2(curr->getSW(), stream);
  }
}

// returns the Patch of terrain whos data covers that of the point in the xz
// plane set level to the maximum depth to go into the tree or to -1 for
// whatever patch is visible
QuadTree<TerrainPatch> *
TerrainData::getNearestPatch(V3d point, QuadTree<TerrainPatch> *terrainTree,
                             int level) {
  if (terrainTree->data->topLeft.z <= point.z) {
    if (terrainTree->data->bottomRight.z >= point.z) {
      if (terrainTree->data->topLeft.x <= point.x) {
        if (terrainTree->data->bottomRight.x >= point.x) {
          // Point is in the rectangle
          if (terrainTree->data->toBeDrawn && (level == -1))
            return terrainTree;

          if (terrainTree->level == level ||
              terrainTree->level == NUM_LEVELS - 1)
            return terrainTree;
          else {
            QuadTree<TerrainPatch> *NW, *NE, *SW, *SE;
            NW = getNearestPatch(point, terrainTree->NW, level);
            if (NW == NULL) {
              NE = getNearestPatch(point, terrainTree->NE, level);
              if (NE == NULL) {
                SW = getNearestPatch(point, terrainTree->SW, level);
                if (SW == NULL) {
                  SE = getNearestPatch(point, terrainTree->SE, level);
                  if (SE == NULL)
                    return NULL;
                  else
                    return SE;
                } else
                  return SW;
              } else
                return NE;
            } else
              return NW;
          }
        }
      }
    }
  }
  return NULL;
}

// Casts a ray from the worldSpacePoint In the Y Direction and returns the point
// of intersection with the terrain Algorithm from
// http://pages.cpsc.ucalgary.ca/~samavati/cpsc453/pdfs/week13.pdf and
// http://www.devmaster.net/wiki/Ray-triangle_intersection
bool TerrainData::terrainHeightAt(V3f worldSpacePoint,
                                  V3f &surfaceIntersectionPoint) {

  QuadTree<TerrainPatch> *patch = getNearestPatch(
      V3d(worldSpacePoint.x, worldSpacePoint.y, worldSpacePoint.z), LODGrid,
      NUM_LEVELS - 1);
  if (patch == NULL)
    return false;

  V3d N, A, B, C, P, O, D;
  double dist, alpha, beta, gamma, S, S1, S2;

  worldSpacePoint.y = (float)patch->data->maxHeight;
  O = V3d(worldSpacePoint.x, worldSpacePoint.y,
          worldSpacePoint.z); // Origin of the ray
  D = V3d(0.0, -1.0, 0.0);    // Direction of the ray

  for (int u = 0; u < patch->data->_numU - 1; ++u) {
    for (int v = 0; v < patch->data->_numV - 1; ++v) {
      // BOTTOM TRIANGLE
      N = (patch->data->_vertexNormals[u][v] +
           patch->data->_vertexNormals[u][v + 1] +
           patch->data->_vertexNormals[u + 1][v + 1]) /
          3.0;                            // The normal of the triangle
      A = patch->data->_dataPoints[u][v]; // A point on the triangle
      B = patch->data->_dataPoints[u][v + 1];
      C = patch->data->_dataPoints[u + 1][v + 1];

      dist = -((O - A) * N) / (D * N);
      P = O + dist * D; // Point in the plane of the triangle.

      // Now test weather this point its inside the triangle with barycentric
      // coordinates
      S = 0.5 * ((A - B) % (A - C)).Length();
      S1 = 0.5 * ((B - P) % (C - P)).Length();
      S2 = 0.5 * ((P - C) % (P - A)).Length();

      alpha = S1 / S;
      beta = S2 / S;
      gamma = 1 - alpha - beta;

      if (alpha >= 0.0 && alpha <= 1.0) {
        if (beta >= 0.0 && beta <= 1.0) {
          if (gamma >= 0.0 && gamma <= 1.0) {
            surfaceIntersectionPoint = V3f(P);
            return true;
          }
        }
      }

      // TOP TRIANGLE
      N = (patch->data->_vertexNormals[u][v] +
           patch->data->_vertexNormals[u + 1][v] +
           patch->data->_vertexNormals[u + 1][v + 1]) /
          3.0; // The normal of the triangle
      B = patch->data->_dataPoints[u + 1][v];

      // Now test weather this point its inside the triangle with barycentric
      // coordinates
      S = 0.5 * ((A - B) % (A - C)).Length();
      S1 = 0.5 * ((B - P) % (C - P)).Length();

      alpha = S1 / S;
      beta = S2 / S;
      gamma = 1 - alpha - beta;

      if (alpha >= 0.0 && alpha <= 1.0) {
        if (beta >= 0.0 && beta <= 1.0) {
          if (gamma >= 0.0 && gamma <= 1.0) {
            surfaceIntersectionPoint = V3f(P);
            return true;
          }
        }
      }
    }
  }
  return false;
}

bool TerrainData::terrainNormalAt(V3f worldSpacePoint, V3f &surfaceNormal) {

  QuadTree<TerrainPatch> *patch = getNearestPatch(
      V3d(worldSpacePoint.x, worldSpacePoint.y, worldSpacePoint.z), LODGrid,
      NUM_LEVELS - 1);
  if (patch == NULL)
    return false;

  V3d N, A, B, C, P, O, D;
  double dist, alpha, beta, gamma, S, S1, S2;

  worldSpacePoint.y = (float)patch->data->maxHeight;
  O = V3d(worldSpacePoint.x, worldSpacePoint.y,
          worldSpacePoint.z); // Origin of the ray
  D = V3d(0.0, -1.0, 0.0);    // Direction of the ray

  for (int u = 0; u < patch->data->_numU - 1; ++u) {
    for (int v = 0; v < patch->data->_numV - 1; ++v) {
      // BOTTOM TRIANGLE
      N = (patch->data->_vertexNormals[u][v] +
           patch->data->_vertexNormals[u][v + 1] +
           patch->data->_vertexNormals[u + 1][v + 1]) /
          3.0;                            // The normal of the triangle
      A = patch->data->_dataPoints[u][v]; // A point on the triangle
      B = patch->data->_dataPoints[u][v + 1];
      C = patch->data->_dataPoints[u + 1][v + 1];

      dist = -((O - A) * N) / (D * N); // Possible div by 0 should fix
      P = O + dist * D;                // Point in the plane of the triangle.

      // Now test weather this point its inside the triangle with barycentric
      // coordinates
      S = 0.5 * ((A - B) % (A - C)).Length();
      S1 = 0.5 * ((B - P) % (C - P)).Length();
      S2 = 0.5 * ((P - C) % (P - A)).Length();

      alpha = S1 / S;
      beta = S2 / S;
      gamma = 1 - alpha - beta;

      if (alpha >= 0.0 && alpha <= 1.0) {
        if (beta >= 0.0 && beta <= 1.0) {
          if (gamma >= 0.0 && gamma <= 1.0) {
            surfaceNormal = V3f(N);
            return true;
          }
        }
      }

      // TOP TRIANGLE
      N = (patch->data->_vertexNormals[u][v] +
           patch->data->_vertexNormals[u + 1][v] +
           patch->data->_vertexNormals[u + 1][v + 1]) /
          3.0; // The normal of the triangle
      B = patch->data->_dataPoints[u + 1][v];

      // Now test weather this point its inside the triangle with barycentric
      // coordinates
      S = 0.5 * ((A - B) % (A - C)).Length();
      S1 = 0.5 * ((B - P) % (C - P)).Length();

      alpha = S1 / S;
      beta = S2 / S;
      gamma = 1 - alpha - beta;

      if (alpha >= 0.0 && alpha <= 1.0) {
        if (beta >= 0.0 && beta <= 1.0) {
          if (gamma >= 0.0 && gamma <= 1.0) {
            surfaceNormal = V3f(N);
            return true;
          }
        }
      }
    }
  }
  return false;
}

void TerrainData::terrainVisibilityAll(VisibilityMode mode,
                                       QuadTree<TerrainPatch> *curr) {
  if (curr == 0)
    curr = LODGrid;

  curr->data->visibilityMode = mode;

  if (curr->level != NUM_LEVELS - 1) {
    terrainVisibilityAll(mode, curr->getNE());
    terrainVisibilityAll(mode, curr->getNW());
    terrainVisibilityAll(mode, curr->getSE());
    terrainVisibilityAll(mode, curr->getSW());
  }
}
void TerrainData::terrainVisibilityPatch(VisibilityMode mode, int ,
                                         V3f worldSpacePoint) {
  QuadTree<TerrainPatch> *patch = getNearestPatch(
      V3d(worldSpacePoint.x, worldSpacePoint.y, worldSpacePoint.z), LODGrid,
      -1);
  if (patch == NULL)
    return;

  else {
    terrainVisibilityAll(mode, patch);
  }
}
void TerrainData::scaleTerrainBy(float val, QuadTree<TerrainPatch> *curr) {
  if (curr == 0) {
    curr = LODGrid;
    fillTerrainData(LODGrid, 0, 0);
    TERRAIN_WIDTH = ORIGINAL_TERRAIN_WIDTH * val;
  }

  curr->data->scaleTerrainBy(val);

  if (curr->level != NUM_LEVELS - 1) {
    scaleTerrainBy(val, curr->getNE());
    scaleTerrainBy(val, curr->getNW());
    scaleTerrainBy(val, curr->getSE());
    scaleTerrainBy(val, curr->getSW());
  }
}

double TerrainData::calculateRi(int i) {
  if (i == NUM_LEVELS - 1) {
    Ri[i] = 0.0;
    return Ri[i];
  }
  if (i == NUM_LEVELS - 2) {
    Ri[i] = sqrt(2.0) * (TERRAIN_WIDTH / pow(2.0, (double)NUM_LEVELS - 1));
    return Ri[i];
  }

  Ri[i] = sqrt(2.0) * (TERRAIN_WIDTH / pow(2.0, (double)i + 1)) +
          calculateRi(i + 1);
  return Ri[i];
}

bool TerrainData::distCheck(V3d pos, int level) {
  pos.y = 0;
  double dist = (cameraPos - pos).Length();
  double Di = sqrt(2.0) * (TERRAIN_WIDTH / pow(2.0, (double)level));

  if (dist <= (Ri[level] + Di * (double)DISTANCE_CONSTANT) / 2.0)
    return true;
  else
    return false;
}

Cracks TerrainData::checkForCracks(QuadTree<TerrainPatch> *curr) {
  Cracks cracks;
  cracks.N = false;
  cracks.E = false;
  cracks.S = false;
  cracks.W = false;

  if (curr->level == 0 || curr->level == 1)
    return cracks;

  if (curr->isFrom == NW_QUAD) {
    if (curr->N != NULL) {
      if (curr->N->parent->data->toBeDrawn)
        cracks.N = true;
    }
    if (curr->W != NULL) {
      if (curr->W->parent->data->toBeDrawn)
        cracks.W = true;
    }
  } else if (curr->isFrom == NE_QUAD) {
    if (curr->N != NULL) {
      if (curr->N->parent->data->toBeDrawn)
        cracks.N = true;
    }
    if (curr->E != NULL) {
      if (curr->E->parent->data->toBeDrawn)
        cracks.E = true;
    }
  } else if (curr->isFrom == SE_QUAD) {
    if (curr->S != NULL) {
      if (curr->S->parent->data->toBeDrawn)
        cracks.S = true;
    }
    if (curr->E != NULL) {
      if (curr->E->parent->data->toBeDrawn)
        cracks.E = true;
    }
  } else if (curr->isFrom == SW_QUAD) {
    if (curr->S != NULL) {
      if (curr->S->parent->data->toBeDrawn)
        cracks.S = true;
    }
    if (curr->W != NULL) {
      if (curr->W->parent->data->toBeDrawn)
        cracks.W = true;
    }
  }

  return cracks;
}

void TerrainData::setDrawFlags(QuadTree<TerrainPatch> *curr) {
  if (curr->level == NUM_LEVELS - 1)
    curr->data->toBeDrawn = true;
  else {
    if (distCheck(curr->data->midPoint, curr->level)) {
      setDrawFlags(curr->getNE());
      setDrawFlags(curr->getNW());
      setDrawFlags(curr->getSE());
      setDrawFlags(curr->getSW());
    } else
      curr->data->toBeDrawn = true;
  }
}

void TerrainData::fillTerrainData(QuadTree<TerrainPatch> *curr, int rowOff,
                                  int colOff) {
  int numRows = 1 << (curr->level);

  if (curr->level < NUM_LEVELS) {
    curr->data->loadPatchFrom(
        dataPoints[curr->level], vertexNormals[curr->level], numU[curr->level],
        numV[curr->level], numRows, numRows, rowOff, colOff, uTile, vTile);
    fillTerrainData(curr->getNW(), rowOff * 2, colOff * 2);
    fillTerrainData(curr->getNE(), rowOff * 2, (colOff * 2) + 1);
    fillTerrainData(curr->getSW(), (rowOff * 2) + 1, colOff * 2);
    fillTerrainData(curr->getSE(), (rowOff * 2) + 1, (colOff * 2) + 1);
  }
}

void TerrainData::subDivide(QuadTree<TerrainPatch> *curr) {
  curr->spawnChildren();

  curr->getNW()->data = new TerrainPatch();
  curr->getNE()->data = new TerrainPatch();
  curr->getSW()->data = new TerrainPatch();
  curr->getSE()->data = new TerrainPatch();
}

void TerrainData::fillLeaves(QuadTree<TerrainPatch> *curr) {
  if (curr->getNW() == NULL && curr->getNE() == NULL && curr->getSW() == NULL &&
      curr->getSE() == NULL)
    subDivide(curr);
  else {
    fillLeaves(curr->getNW());
    fillLeaves(curr->getNE());
    fillLeaves(curr->getSW());
    fillLeaves(curr->getSE());
  }
}

void TerrainData::setGlobalPointers(QuadTree<TerrainPatch> *curr) {
  curr->setGlobalPointers();

  if (curr->level < NUM_LEVELS - 1) {
    setGlobalPointers(curr->getNW());
    setGlobalPointers(curr->getNE());
    setGlobalPointers(curr->getSW());
    setGlobalPointers(curr->getSE());
  }
}

void TerrainData::calculateResolutions() {
  for (int i = NUM_LEVELS - 1; i > 0; i--) {
    for (int u = 0; u < numU[i - 1]; ++u) {
      for (int v = 0; v < numV[i - 1]; ++v) {
        dataPoints[i - 1][u][v] = dataPoints[i][u * 2][v * 2];
      }
    }
  }
}

void TerrainData::loadTerrain(const char *viewLine) {
  Utils::Message("Loading Terrain Data\n");

  // Parse the information out of the string
  char fileName[80], displayGrid[10] = "off";

  sscanf(viewLine, "%s %i %f %f %s %i %f %f", fileName, &NUM_LEVELS,
                   &TERRAIN_SCALE, &DISTANCE_CONSTANT, displayGrid, &_textureID,
                   &uTile, &vTile);

  if (NUM_LEVELS <= 0) {
    NUM_LEVELS = 1;
    throw Exception("Invalid number of subdivision levels for terrain %s",
                    fileName);
  }
  if (TERRAIN_SCALE <= 0.0f) {
    TERRAIN_SCALE = 1.0;
    throw Exception("Invalid scale value for terrain  %s", fileName);
  }
  if (strncmp(displayGrid, "on", 2) == 0)
    showGrid = true;

  if (_textureID >= 0)
    _isTextured = true;
  else if (_textureID != -1)
    throw Exception("Invalid texture ID for terrain  %s", fileName);

  if (uTile < 0.0) {
    uTile = 1.0;
    throw Exception("Invalid Tiling value in U direction for terrain  %s",
                    fileName);
  }
  if (vTile < 0.0) {
    vTile = 1.0;
    throw Exception("Invalid Tiling value in V direction for terrain  %s",
                    fileName);
  }

  // Set up arrays
  dataPoints = new V3d **[NUM_LEVELS];
  vertexNormals = new V3d **[NUM_LEVELS];
  numU = new int[NUM_LEVELS];
  numV = new int[NUM_LEVELS];
  Ri = new double[NUM_LEVELS];

  loadTerrain2(fileName);
  calculateResolutions();
  calculateRi(0);

  TERRAIN_WIDTH *= TERRAIN_SCALE;

  LODGrid = new QuadTree<TerrainPatch>(0, 0, NONE);
  LODGrid->data = new TerrainPatch();

  for (int i = 0; i < NUM_LEVELS; ++i)
    fillLeaves(LODGrid);

  setGlobalPointers(LODGrid);
  fillTerrainData(LODGrid, 0, 0);

  cameraPos = V3d(0, 0, 0);

  Utils::Message("Load Successful\n");
}

void TerrainData::loadTerrain2(const char *fn) {
  ifstream File;
  File.open(fn);
  if (File) {
    File >> TERRAIN_WIDTH;
    ORIGINAL_TERRAIN_WIDTH = TERRAIN_WIDTH;
    for (int l = 0; l < NUM_LEVELS; ++l) {

      File >> numU[l] >> numV[l];

      // Allocate the memory for the data points
      dataPoints[l] = new V3d *[numU[l]];
      for (int i = 0; i < numU[l]; ++i)
        dataPoints[l][i] = new V3d[numV[l]];

      // Allocate the memory for the normals
      vertexNormals[l] = new V3d *[numU[l]];
      for (int i = 0; i < numU[l]; ++i)
        vertexNormals[l][i] = new V3d[numV[l]];

      // read in the data points
      for (int i = 0; i < numU[l]; ++i) {
        for (int j = 0; j < numV[l]; ++j) {
          File >> dataPoints[l][i][j].x >> dataPoints[l][i][j].y >>
              dataPoints[l][i][j].z;
          File >> vertexNormals[l][i][j].x >> vertexNormals[l][i][j].y >>
              vertexNormals[l][i][j].z;

          dataPoints[l][i][j] *= TERRAIN_SCALE;
        }
      }
    }
    File.close();
  } else {
    Utils::Error("Cant open terrain file");
  }
}

void TerrainData::markCracks(QuadTree<TerrainPatch> *curr) {
  if (curr->data->toBeDrawn)
    curr->data->cracks = checkForCracks(curr);
  else {
    markCracks(curr->getNE());
    markCracks(curr->getNW());
    markCracks(curr->getSE());
    markCracks(curr->getSW());
  }
}

int TerrainData::drawGridLOD(QuadTree<TerrainPatch> *curr) {
  int triCount = 0;
  if (curr->data->toBeDrawn)
    triCount += curr->data->draw(showGrid);
  else {
    triCount += drawGridLOD(curr->getNE());
    triCount += drawGridLOD(curr->getNW());
    triCount += drawGridLOD(curr->getSE());
    triCount += drawGridLOD(curr->getSW());
  }

  return triCount;
}

void TerrainData::clearDrawFlags(QuadTree<TerrainPatch> *curr) {
  curr->data->toBeDrawn = false;
  if (curr->level != NUM_LEVELS - 1) {
    clearDrawFlags(curr->getNE());
    clearDrawFlags(curr->getNW());
    clearDrawFlags(curr->getSE());
    clearDrawFlags(curr->getSW());
  }
}

void TerrainData::draw(CameraPosition camPos) {
  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

  float scale =
      (V3f(0, 0, 0) - camPos.position).Length() * (-1.0f / camPos.scale + 1.0f);
  V3f cameraPosf = camPos.position + scale * camPos.head;

  cameraPos = V3d(cameraPosf.x, cameraPosf.y, cameraPosf.z);
  clearDrawFlags(LODGrid);
  setDrawFlags(LODGrid);
  markCracks(LODGrid);
  drawGridLOD(LODGrid);

  glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
}
