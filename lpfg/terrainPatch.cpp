/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "terrainPatch.h"

TerrainPatch::TerrainPatch() {
  _dataPoints = _vertexNormals = NULL;
  _texCoords = NULL;
  _numU = _numV = 0;
  toBeDrawn = false;
  cracks.N = false;
  cracks.E = false;
  cracks.S = false;
  cracks.W = false;

  switch (drawparams.RenderMode()) {
  case DParams::rmWireframe:
    visibilityMode = Wireframe;
    break;
  default:
    visibilityMode = Shaded;
  }
}

void TerrainPatch::scaleTerrainBy(float val) {
  int uStart = 0, vStart = 0, uEnd = _numU, vEnd = _numV;
  for (int u = uStart; u < uEnd; ++u) {
    for (int v = vStart; v < vEnd; ++v) {
      _dataPoints[u][v] *= val;
    }
  }
  topLeft *= val;
  bottomRight *= val;
  midPoint *= val;
  maxHeight *= val;
}

#define _DumpVector(a) '<' << a.x << ',' << a.y << ',' << a.z << '>'
#define _DumpVector2(a) '<' << a.x << ',' << a.y << '>'

void TerrainPatch::OutputToPOVRay(std::ostream &stream, bool textured) {
  int uStart = 0, vStart = 0, uEnd = _numU - 1, vEnd = _numV - 1;
  for (int u = uStart; u < uEnd; ++u) {
    for (int v = vStart; v < vEnd; ++v) {
      // If the angle between the normals of A and D is greater then the angle
      // between B and C we want to triangulate between A and D to avoid making
      // a cross in the wrong way a-b | | c-d
      if (_vertexNormals[u][v] * _vertexNormals[u + 1][v + 1] <
          _vertexNormals[u + 1][v] * _vertexNormals[u][v + 1]) {
        stream << "\t smooth_triangle { " << _DumpVector(_dataPoints[u][v + 1])
               << ',' << _DumpVector(_vertexNormals[u][v + 1]) << ','
               << _DumpVector(_dataPoints[u + 1][v]) << ','
               << _DumpVector(_vertexNormals[u + 1][v]) << ','
               << _DumpVector(_dataPoints[u][v]) << ','
               << _DumpVector(_vertexNormals[u][v]);
        if (textured)
          stream << " uv_vectors " << _DumpVector2(_texCoords[u][v + 1]) << ','
                 << _DumpVector2(_texCoords[u + 1][v]) << ','
                 << _DumpVector2(_texCoords[u][v]);

        stream << '}' << std::endl;

        stream << "\t smooth_triangle { " << _DumpVector(_dataPoints[u][v + 1])
               << ',' << _DumpVector(_vertexNormals[u][v + 1]) << ','
               << _DumpVector(_dataPoints[u + 1][v + 1]) << ','
               << _DumpVector(_vertexNormals[u + 1][v + 1]) << ','
               << _DumpVector(_dataPoints[u + 1][v]) << ','
               << _DumpVector(_vertexNormals[u + 1][v]);

        if (textured)
          stream << " uv_vectors " << _DumpVector2(_texCoords[u][v + 1]) << ','
                 << _DumpVector2(_texCoords[u + 1][v + 1]) << ','
                 << _DumpVector2(_texCoords[u + 1][v]);

        stream << '}' << std::endl;
      } else {
        stream << "\t smooth_triangle { " << _DumpVector(_dataPoints[u + 1][v])
               << ',' << _DumpVector(_vertexNormals[u + 1][v]) << ','
               << _DumpVector(_dataPoints[u][v]) << ','
               << _DumpVector(_vertexNormals[u][v]) << ','
               << _DumpVector(_dataPoints[u + 1][v + 1]) << ','
               << _DumpVector(_vertexNormals[u + 1][v + 1]);
        if (textured)
          stream << " uv_vectors " << _DumpVector2(_texCoords[u + 1][v]) << ','
                 << _DumpVector2(_texCoords[u][v]) << ','
                 << _DumpVector2(_texCoords[u + 1][v + 1]);

        stream << '}' << std::endl;

        stream << "\t smooth_triangle { " << _DumpVector(_dataPoints[u][v])
               << ',' << _DumpVector(_vertexNormals[u][v]) << ','
               << _DumpVector(_dataPoints[u][v + 1]) << ','
               << _DumpVector(_vertexNormals[u][v + 1]) << ','
               << _DumpVector(_dataPoints[u + 1][v + 1]) << ','
               << _DumpVector(_vertexNormals[u + 1][v + 1]);
        if (textured)
          stream << " uv_vectors " << _DumpVector2(_texCoords[u][v]) << ','
                 << _DumpVector2(_texCoords[u][v + 1]) << ','
                 << _DumpVector2(_texCoords[u + 1][v + 1]);
        stream << '}' << std::endl;
      }
    }
  }
}

void TerrainPatch::drawGrid() {
  V3d dTopLeft = V3d(midPoint.x, topLeft.y, midPoint.z) +
                 0.95 * (topLeft - V3d(midPoint.x, topLeft.y, midPoint.z));
  V3d dBottomRight =
      V3d(midPoint.x, topLeft.y, midPoint.z) +
      0.95 * (bottomRight - V3d(midPoint.x, topLeft.y, midPoint.z));

  glDisable(GL_LIGHTING);

  glBegin(GL_LINES);

  if (cracks.N)
    glColor3f(1.0, 0.0, 0.0);
  else
    glColor3f(1.0, 1.0, 0.0);
  glVertex3d(dTopLeft.x, dTopLeft.y, dTopLeft.z);
  glVertex3d(dBottomRight.x, dTopLeft.y, dTopLeft.z);

  if (cracks.E)
    glColor3f(1.0, 0.0, 0.0);
  else
    glColor3f(1.0, 1.0, 0.0);
  glVertex3d(dBottomRight.x, dTopLeft.y, dTopLeft.z);
  glVertex3d(dBottomRight.x, dBottomRight.y, dBottomRight.z);

  if (cracks.S)
    glColor3f(1.0, 0.0, 0.0);
  else
    glColor3f(1.0, 1.0, 0.0);
  glVertex3d(dBottomRight.x, dBottomRight.y, dBottomRight.z);
  glVertex3d(dTopLeft.x, dBottomRight.y, dBottomRight.z);

  if (cracks.W)
    glColor3f(1.0, 0.0, 0.0);
  else
    glColor3f(1.0, 1.0, 0.0);
  glVertex3d(dTopLeft.x, dBottomRight.y, dBottomRight.z);
  glVertex3d(dTopLeft.x, dTopLeft.y, dTopLeft.z);

  glEnd();

  glEnable(GL_LIGHTING);
}

void TerrainPatch::loadPatchFrom(V3d **PdataPoints, V3d **PvertexNormals,
                                 int PnumU, int PnumV, int numRows, int numCols,
                                 int row, int col, float uTile, float vTile) {
  int uPerCol = (PnumU - 1) / numCols + 1;
  int vPerRow = (PnumV - 1) / numRows + 1;
  int Ustart = (uPerCol - 1) * col;
  int Vstart = (vPerRow - 1) * row;

  _numU = uPerCol;
  _numV = vPerRow;

  if (_dataPoints == NULL) {
    _dataPoints = new V3d *[uPerCol];
    for (int i = 0; i < uPerCol; ++i)
      _dataPoints[i] = new V3d[vPerRow];
  }

  if (_vertexNormals == NULL) {
    _vertexNormals = new V3d *[uPerCol];
    for (int i = 0; i < uPerCol; ++i)
      _vertexNormals[i] = new V3d[vPerRow];
  }

  if (_texCoords == NULL) {
    _texCoords = new V2d *[uPerCol];
    for (int i = 0; i < uPerCol; ++i)
      _texCoords[i] = new V2d[vPerRow];
  }

  int i, j;
  i = 0;
  for (int u = Ustart; u < Ustart + uPerCol; ++u) {
    j = 0;
    for (int v = Vstart; v < Vstart + vPerRow; ++v) {
      _dataPoints[i][j] = PdataPoints[u][v];
      _vertexNormals[i][j] = PvertexNormals[u][v];
      _texCoords[i][j] = V2d(((double)u * uTile) / ((double)PnumU - 1.0),
                             ((double)v * vTile) / ((double)PnumV - 1.0));

      if (i == 0 && j == 0) {
        topLeft = _dataPoints[i][j];
        bottomRight = _dataPoints[i][j];
        maxHeight = topLeft.y;
      }

      if (_dataPoints[i][j].x < topLeft.x)
        topLeft.x = _dataPoints[i][j].x;
      if (_dataPoints[i][j].z < topLeft.z)
        topLeft.z = _dataPoints[i][j].z;

      if (_dataPoints[i][j].x > bottomRight.x)
        bottomRight.x = _dataPoints[i][j].x;
      if (_dataPoints[i][j].z > bottomRight.z)
        bottomRight.z = _dataPoints[i][j].z;

      if (_dataPoints[i][j].y > topLeft.y)
        topLeft.y = _dataPoints[i][j].y;
      if (_dataPoints[i][j].y < bottomRight.y)
        bottomRight.y = _dataPoints[i][j].y;

      ++j;
    }
    ++i;
  }
  // Calculate Midpoint
  midPoint = topLeft + 0.5 * (bottomRight - topLeft);
  maxHeight = topLeft.y;

  topLeft.y = midPoint.y;
  bottomRight.y = midPoint.y;
}

int TerrainPatch::draw(bool showGrid) {
  DParams::RenderMode oldMode = drawparams.RenderMode();

  if (visibilityMode == Wireframe && oldMode != DParams::rmWireframe) {
    // setPolygonType
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glShadeModel(GL_SMOOTH);
  } else if (visibilityMode == Shaded && oldMode != DParams::rmShaded) {
    // setPolygonType
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);
  } else if (visibilityMode == Shaded && oldMode != DParams::rmShadows) {
    // setPolygonType
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);
  }

  if (visibilityMode != Hidden) {
    drawSolid();
    drawFixedBoarders();
    fixCorners();

    if (showGrid)
      drawGrid();

    // toBeDrawn = false;
    cracks.N = false;
    cracks.E = false;
    cracks.S = false;
    cracks.W = false;
    // Reset Polygon Type
    switch (oldMode) {
    case DParams::rmWireframe:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glShadeModel(GL_SMOOTH);
      break;
    case DParams::rmFilled:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glShadeModel(GL_FLAT);
      break;
    case DParams::rmShaded:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glShadeModel(GL_SMOOTH);
      break;
    case DParams::rmShadows:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glShadeModel(GL_SMOOTH);
      break;
    }
  }

  return _numU * _numV * 2; // number of triangles drawn
}

void TerrainPatch::fixCorners() {
  glBegin(GL_TRIANGLES);

  if (cracks.N && !cracks.W) {
    glTexCoord2d(_texCoords[0][0].x, _texCoords[0][0].y);
    glNormal3d(_vertexNormals[0][0].x, _vertexNormals[0][0].y,
               _vertexNormals[0][0].z);
    glVertex3d(_dataPoints[0][0].x, _dataPoints[0][0].y, _dataPoints[0][0].z);

    glTexCoord2d(_texCoords[0][1].x, _texCoords[0][1].y);
    glNormal3d(_vertexNormals[0][1].x, _vertexNormals[0][1].y,
               _vertexNormals[0][1].z);
    glVertex3d(_dataPoints[0][1].x, _dataPoints[0][1].y, _dataPoints[0][1].z);

    glTexCoord2d(_texCoords[1][1].x, _texCoords[1][1].y);
    glNormal3d(_vertexNormals[1][1].x, _vertexNormals[1][1].y,
               _vertexNormals[1][1].z);
    glVertex3d(_dataPoints[1][1].x, _dataPoints[1][1].y, _dataPoints[1][1].z);
  }
  if (cracks.N && !cracks.E) {
    glTexCoord2d(_texCoords[_numU - 1][0].x, _texCoords[_numU - 1][0].y);
    glNormal3d(_vertexNormals[_numU - 1][0].x, _vertexNormals[_numU - 1][0].y,
               _vertexNormals[_numU - 1][0].z);
    glVertex3d(_dataPoints[_numU - 1][0].x, _dataPoints[_numU - 1][0].y,
               _dataPoints[_numU - 1][0].z);

    glTexCoord2d(_texCoords[_numU - 2][1].x, _texCoords[_numU - 2][1].y);
    glNormal3d(_vertexNormals[_numU - 2][1].x, _vertexNormals[_numU - 2][1].y,
               _vertexNormals[_numU - 2][1].z);
    glVertex3d(_dataPoints[_numU - 2][1].x, _dataPoints[_numU - 2][1].y,
               _dataPoints[_numU - 2][1].z);

    glTexCoord2d(_texCoords[_numU - 1][1].x, _texCoords[_numU - 1][1].y);
    glNormal3d(_vertexNormals[_numU - 1][1].x, _vertexNormals[_numU - 1][1].y,
               _vertexNormals[_numU - 1][1].z);
    glVertex3d(_dataPoints[_numU - 1][1].x, _dataPoints[_numU - 1][1].y,
               _dataPoints[_numU - 1][1].z);

    //
  }

  if (cracks.E && !cracks.N) {
    glTexCoord2d(_texCoords[_numU - 2][0].x, _texCoords[_numU - 2][0].y);
    glNormal3d(_vertexNormals[_numU - 2][0].x, _vertexNormals[_numU - 2][0].y,
               _vertexNormals[_numU - 2][0].z);
    glVertex3d(_dataPoints[_numU - 2][0].x, _dataPoints[_numU - 2][0].y,
               _dataPoints[_numU - 2][0].z);

    glTexCoord2d(_texCoords[_numU - 2][1].x, _texCoords[_numU - 2][1].y);
    glNormal3d(_vertexNormals[_numU - 2][1].x, _vertexNormals[_numU - 2][1].y,
               _vertexNormals[_numU - 2][1].z);
    glVertex3d(_dataPoints[_numU - 2][1].x, _dataPoints[_numU - 2][1].y,
               _dataPoints[_numU - 2][1].z);

    glTexCoord2d(_texCoords[_numU - 1][0].x, _texCoords[_numU - 1][0].y);
    glNormal3d(_vertexNormals[_numU - 1][0].x, _vertexNormals[_numU - 1][0].y,
               _vertexNormals[_numU - 1][0].z);
    glVertex3d(_dataPoints[_numU - 1][0].x, _dataPoints[_numU - 1][0].y,
               _dataPoints[_numU - 1][0].z);
  }

  if (cracks.E && !cracks.S) {
    glTexCoord2d(_texCoords[_numU - 2][_numV - 2].x,
                 _texCoords[_numU - 2][_numV - 2].y);
    glNormal3d(_vertexNormals[_numU - 2][_numV - 2].x,
               _vertexNormals[_numU - 2][_numV - 2].y,
               _vertexNormals[_numU - 2][_numV - 2].z);
    glVertex3d(_dataPoints[_numU - 2][_numV - 2].x,
               _dataPoints[_numU - 2][_numV - 2].y,
               _dataPoints[_numU - 2][_numV - 2].z);

    glTexCoord2d(_texCoords[_numU - 2][_numV - 1].x,
                 _texCoords[_numU - 2][_numV - 1].y);
    glNormal3d(_vertexNormals[_numU - 2][_numV - 1].x,
               _vertexNormals[_numU - 2][_numV - 1].y,
               _vertexNormals[_numU - 2][_numV - 1].z);
    glVertex3d(_dataPoints[_numU - 2][_numV - 1].x,
               _dataPoints[_numU - 2][_numV - 1].y,
               _dataPoints[_numU - 2][_numV - 1].z);

    glTexCoord2d(_texCoords[_numU - 1][_numV - 1].x,
                 _texCoords[_numU - 1][_numV - 1].y);
    glNormal3d(_vertexNormals[_numU - 1][_numV - 1].x,
               _vertexNormals[_numU - 1][_numV - 1].y,
               _vertexNormals[_numU - 1][_numV - 1].z);
    glVertex3d(_dataPoints[_numU - 1][_numV - 1].x,
               _dataPoints[_numU - 1][_numV - 1].y,
               _dataPoints[_numU - 1][_numV - 1].z);
    //
  }

  if (cracks.S && !cracks.E) {
    glTexCoord2d(_texCoords[_numU - 1][_numV - 2].x,
                 _texCoords[_numU - 1][_numV - 2].y);
    glNormal3d(_vertexNormals[_numU - 1][_numV - 2].x,
               _vertexNormals[_numU - 1][_numV - 2].y,
               _vertexNormals[_numU - 1][_numV - 2].z);
    glVertex3d(_dataPoints[_numU - 1][_numV - 2].x,
               _dataPoints[_numU - 1][_numV - 2].y,
               _dataPoints[_numU - 1][_numV - 2].z);

    glTexCoord2d(_texCoords[_numU - 2][_numV - 2].x,
                 _texCoords[_numU - 2][_numV - 2].y);
    glNormal3d(_vertexNormals[_numU - 2][_numV - 2].x,
               _vertexNormals[_numU - 2][_numV - 2].y,
               _vertexNormals[_numU - 2][_numV - 2].z);
    glVertex3d(_dataPoints[_numU - 2][_numV - 2].x,
               _dataPoints[_numU - 2][_numV - 2].y,
               _dataPoints[_numU - 2][_numV - 2].z);

    glTexCoord2d(_texCoords[_numU - 1][_numV - 1].x,
                 _texCoords[_numU - 1][_numV - 1].y);
    glNormal3d(_vertexNormals[_numU - 1][_numV - 1].x,
               _vertexNormals[_numU - 1][_numV - 1].y,
               _vertexNormals[_numU - 1][_numV - 1].z);
    glVertex3d(_dataPoints[_numU - 1][_numV - 1].x,
               _dataPoints[_numU - 1][_numV - 1].y,
               _dataPoints[_numU - 1][_numV - 1].z);
  }

  if (cracks.S && !cracks.W) {
    glTexCoord2d(_texCoords[1][_numV - 2].x, _texCoords[1][_numV - 2].y);
    glNormal3d(_vertexNormals[1][_numV - 2].x, _vertexNormals[1][_numV - 2].y,
               _vertexNormals[1][_numV - 2].z);
    glVertex3d(_dataPoints[1][_numV - 2].x, _dataPoints[1][_numV - 2].y,
               _dataPoints[1][_numV - 2].z);

    glTexCoord2d(_texCoords[0][_numV - 2].x, _texCoords[0][_numV - 2].y);
    glNormal3d(_vertexNormals[0][_numV - 2].x, _vertexNormals[0][_numV - 2].y,
               _vertexNormals[0][_numV - 2].z);
    glVertex3d(_dataPoints[0][_numV - 2].x, _dataPoints[0][_numV - 2].y,
               _dataPoints[0][_numV - 2].z);

    glTexCoord2d(_texCoords[0][_numV - 1].x, _texCoords[0][_numV - 1].y);
    glNormal3d(_vertexNormals[0][_numV - 1].x, _vertexNormals[0][_numV - 1].y,
               _vertexNormals[0][_numV - 1].z);
    glVertex3d(_dataPoints[0][_numV - 1].x, _dataPoints[0][_numV - 1].y,
               _dataPoints[0][_numV - 1].z);
  }

  if (cracks.W && !cracks.N) {
    glTexCoord2d(_texCoords[0][0].x, _texCoords[0][0].y);
    glNormal3d(_vertexNormals[0][0].x, _vertexNormals[0][0].y,
               _vertexNormals[0][0].z);
    glVertex3d(_dataPoints[0][0].x, _dataPoints[0][0].y, _dataPoints[0][0].z);

    glTexCoord2d(_texCoords[1][1].x, _texCoords[1][1].y);
    glNormal3d(_vertexNormals[1][1].x, _vertexNormals[1][1].y,
               _vertexNormals[1][1].z);
    glVertex3d(_dataPoints[1][1].x, _dataPoints[1][1].y, _dataPoints[1][1].z);

    glTexCoord2d(_texCoords[1][0].x, _texCoords[1][0].y);
    glNormal3d(_vertexNormals[1][0].x, _vertexNormals[1][0].y,
               _vertexNormals[1][0].z);
    glVertex3d(_dataPoints[1][0].x, _dataPoints[1][0].y, _dataPoints[1][0].z);
  }
  if (cracks.W && !cracks.S) {
    glTexCoord2d(_texCoords[0][_numV - 1].x, _texCoords[0][_numV - 1].y);
    glNormal3d(_vertexNormals[0][_numV - 1].x, _vertexNormals[0][_numV - 1].y,
               _vertexNormals[0][_numV - 1].z);
    glVertex3d(_dataPoints[0][_numV - 1].x, _dataPoints[0][_numV - 1].y,
               _dataPoints[0][_numV - 1].z);

    glTexCoord2d(_texCoords[1][_numV - 1].x, _texCoords[1][_numV - 1].y);
    glNormal3d(_vertexNormals[1][_numV - 1].x, _vertexNormals[1][_numV - 1].y,
               _vertexNormals[1][_numV - 1].z);
    glVertex3d(_dataPoints[1][_numV - 1].x, _dataPoints[1][_numV - 1].y,
               _dataPoints[1][_numV - 1].z);

    glTexCoord2d(_texCoords[1][_numV - 2].x, _texCoords[1][_numV - 2].y);
    glNormal3d(_vertexNormals[1][_numV - 2].x, _vertexNormals[1][_numV - 2].y,
               _vertexNormals[1][_numV - 2].z);
    glVertex3d(_dataPoints[1][_numV - 2].x, _dataPoints[1][_numV - 2].y,
               _dataPoints[1][_numV - 2].z);
    //
  }

  glEnd();
}
void TerrainPatch::drawFixedBoarders() {

  if (cracks.N) {
    int v = 0;
    glBegin(GL_TRIANGLES);
    for (int u = 0; u < _numU - 2; u += 2) {
      // bot_left
      if (u != 0) {
        glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
        glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                   _vertexNormals[u][v + 1].z);
        glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                   _dataPoints[u][v + 1].z);

        glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
        glNormal3d(_vertexNormals[u + 1][v + 1].x,
                   _vertexNormals[u + 1][v + 1].y,
                   _vertexNormals[u + 1][v + 1].z);
        glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                   _dataPoints[u + 1][v + 1].z);

        glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
        glNormal3d(_vertexNormals[u][v].x, _vertexNormals[u][v].y,
                   _vertexNormals[u][v].z);
        glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y,
                   _dataPoints[u][v].z);
      }

      // middle
      glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
      glNormal3d(_vertexNormals[u + 1][v + 1].x, _vertexNormals[u + 1][v + 1].y,
                 _vertexNormals[u + 1][v + 1].z);
      glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                 _dataPoints[u + 1][v + 1].z);

      glTexCoord2d(_texCoords[u + 2][v].x, _texCoords[u + 2][v].y);
      glNormal3d(_vertexNormals[u + 2][v].x, _vertexNormals[u + 2][v].y,
                 _vertexNormals[u + 2][v].z);
      glVertex3d(_dataPoints[u + 2][v].x, _dataPoints[u + 2][v].y,
                 _dataPoints[u + 2][v].z);

      glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
      glNormal3d(_vertexNormals[u][v].x, _vertexNormals[u][v].y,
                 _vertexNormals[u][v].z);
      glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y, _dataPoints[u][v].z);

      // bot_Right
      if (u != _numU - 3) {
        glTexCoord2d(_texCoords[u + 2][v + 1].x, _texCoords[u + 2][v + 1].y);
        glNormal3d(_vertexNormals[u + 2][v + 1].x,
                   _vertexNormals[u + 2][v + 1].y,
                   _vertexNormals[u + 2][v + 1].z);
        glVertex3d(_dataPoints[u + 2][v + 1].x, _dataPoints[u + 2][v + 1].y,
                   _dataPoints[u + 2][v + 1].z);

        glTexCoord2d(_texCoords[u + 2][v].x, _texCoords[u + 2][v].y);
        glNormal3d(_vertexNormals[u + 2][v].x, _vertexNormals[u + 2][v].y,
                   _vertexNormals[u + 2][v].z);
        glVertex3d(_dataPoints[u + 2][v].x, _dataPoints[u + 2][v].y,
                   _dataPoints[u + 2][v].z);

        glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
        glNormal3d(_vertexNormals[u + 1][v + 1].x,
                   _vertexNormals[u + 1][v + 1].y,
                   _vertexNormals[u + 1][v + 1].z);
        glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                   _dataPoints[u + 1][v + 1].z);
      }
    }
    glEnd();
  }

  if (cracks.S) {
    int v = _numV - 2;
    glBegin(GL_TRIANGLES);
    for (int u = 0; u < _numU - 2; u += 2) {
      // top_left
      if (u != 0) {
        glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
        glNormal3d(_vertexNormals[u + 1][v].x, _vertexNormals[u + 1][v].y,
                   _vertexNormals[u + 1][v].z);
        glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                   _dataPoints[u + 1][v].z);

        glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
        glNormal3d(_vertexNormals[u][v].x, _vertexNormals[u][v].y,
                   _vertexNormals[u][v].z);
        glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y,
                   _dataPoints[u][v].z);

        glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
        glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                   _vertexNormals[u][v + 1].z);
        glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                   _dataPoints[u][v + 1].z);
      }
      // middle
      glTexCoord2d(_texCoords[u + 2][v + 1].x, _texCoords[u + 2][v + 1].y);
      glNormal3d(_vertexNormals[u + 2][v + 1].x, _vertexNormals[u + 2][v + 1].y,
                 _vertexNormals[u + 2][v + 1].z);
      glVertex3d(_dataPoints[u + 2][v + 1].x, _dataPoints[u + 2][v + 1].y,
                 _dataPoints[u + 2][v + 1].z);

      glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
      glNormal3d(_vertexNormals[u + 1][v].x, _vertexNormals[u + 1][v].y,
                 _vertexNormals[u + 1][v].z);
      glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                 _dataPoints[u + 1][v].z);

      glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
      glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                 _vertexNormals[u][v + 1].z);
      glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                 _dataPoints[u][v + 1].z);

      // Top_Right
      if (u != _numU - 3) {
        glTexCoord2d(_texCoords[u + 2][v + 1].x, _texCoords[u + 2][v + 1].y);
        glNormal3d(_vertexNormals[u + 2][v + 1].x,
                   _vertexNormals[u + 2][v + 1].y,
                   _vertexNormals[u + 2][v + 1].z);
        glVertex3d(_dataPoints[u + 2][v + 1].x, _dataPoints[u + 2][v + 1].y,
                   _dataPoints[u + 2][v + 1].z);

        glTexCoord2d(_texCoords[u + 2][v].x, _texCoords[u + 2][v].y);
        glNormal3d(_vertexNormals[u + 2][v].x, _vertexNormals[u + 2][v].y,
                   _vertexNormals[u + 2][v].z);
        glVertex3d(_dataPoints[u + 2][v].x, _dataPoints[u + 2][v].y,
                   _dataPoints[u + 2][v].z);

        glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
        glNormal3d(_vertexNormals[u + 1][v].x, _vertexNormals[u + 1][v].y,
                   _vertexNormals[u + 1][v].z);
        glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                   _dataPoints[u + 1][v].z);
      }
    }
    glEnd();
  }

  if (cracks.W) {
    int u = 0;
    glBegin(GL_TRIANGLES);
    for (int v = 0; v < _numV - 2; v += 2) {

      // top right
      if (v != 0) {
        glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
        glNormal3d(_vertexNormals[u][v].x, _vertexNormals[u][v].y,
                   _vertexNormals[u][v].z);
        glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y,
                   _dataPoints[u][v].z);

        glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
        glNormal3d(_vertexNormals[u + 1][v + 1].x,
                   _vertexNormals[u + 1][v + 1].y,
                   _vertexNormals[u + 1][v + 1].z);
        glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                   _dataPoints[u + 1][v + 1].z);

        glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
        glNormal3d(_vertexNormals[u + 1][v].x, _vertexNormals[u + 1][v].y,
                   _vertexNormals[u + 1][v].z);
        glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                   _dataPoints[u + 1][v].z);
      }
      // middle
      glTexCoord2d(_texCoords[u][v + 2].x, _texCoords[u][v + 2].y);
      glNormal3d(_vertexNormals[u][v + 2].x, _vertexNormals[u][v + 2].y,
                 _vertexNormals[u][v + 2].z);
      glVertex3d(_dataPoints[u][v + 2].x, _dataPoints[u][v + 2].y,
                 _dataPoints[u][v + 2].z);

      glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
      glNormal3d(_vertexNormals[u + 1][v + 1].x, _vertexNormals[u + 1][v + 1].y,
                 _vertexNormals[u + 1][v + 1].z);
      glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                 _dataPoints[u + 1][v + 1].z);

      glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
      glNormal3d(_vertexNormals[u][v].x, _vertexNormals[u][v].y,
                 _vertexNormals[u][v].z);
      glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y, _dataPoints[u][v].z);

      // bottom right
      if (v != _numV - 3) {
        glTexCoord2d(_texCoords[u + 1][v + 2].x, _texCoords[u + 1][v + 2].y);
        glNormal3d(_vertexNormals[u + 1][v + 2].x,
                   _vertexNormals[u + 1][v + 2].y,
                   _vertexNormals[u + 1][v + 2].z);
        glVertex3d(_dataPoints[u + 1][v + 2].x, _dataPoints[u + 1][v + 2].y,
                   _dataPoints[u + 1][v + 2].z);

        glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
        glNormal3d(_vertexNormals[u + 1][v + 1].x,
                   _vertexNormals[u + 1][v + 1].y,
                   _vertexNormals[u + 1][v + 1].z);
        glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                   _dataPoints[u + 1][v + 1].z);

        glTexCoord2d(_texCoords[u][v + 2].x, _texCoords[u][v + 2].y);
        glNormal3d(_vertexNormals[u][v + 2].x, _vertexNormals[u][v + 2].y,
                   _vertexNormals[u][v + 2].z);
        glVertex3d(_dataPoints[u][v + 2].x, _dataPoints[u][v + 2].y,
                   _dataPoints[u][v + 2].z);
      }
    }
    glEnd();
  }

  if (cracks.E) {
    int u = _numU - 2;
    glBegin(GL_TRIANGLES);
    for (int v = 0; v < _numV - 2; v += 2) {
      // top left
      if (v != 0) {
        glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
        glNormal3d(_vertexNormals[u][v].x, _vertexNormals[u][v].y,
                   _vertexNormals[u][v].z);
        glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y,
                   _dataPoints[u][v].z);

        glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
        glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                   _vertexNormals[u][v + 1].z);
        glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                   _dataPoints[u][v + 1].z);

        glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
        glNormal3d(_vertexNormals[u + 1][v].x, _vertexNormals[u + 1][v].y,
                   _vertexNormals[u + 1][v].z);
        glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                   _dataPoints[u + 1][v].z);
      }

      // middle
      glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
      glNormal3d(_vertexNormals[u + 1][v].x, _vertexNormals[u + 1][v].y,
                 _vertexNormals[u + 1][v].z);
      glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                 _dataPoints[u + 1][v].z);

      glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
      glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                 _vertexNormals[u][v + 1].z);
      glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                 _dataPoints[u][v + 1].z);

      glTexCoord2d(_texCoords[u + 1][v + 2].x, _texCoords[u + 1][v + 2].y);
      glNormal3d(_vertexNormals[u + 1][v + 2].x, _vertexNormals[u + 1][v + 2].y,
                 _vertexNormals[u + 1][v + 2].z);
      glVertex3d(_dataPoints[u + 1][v + 2].x, _dataPoints[u + 1][v + 2].y,
                 _dataPoints[u + 1][v + 2].z);

      // bottom right
      if (v != _numV - 3) {

        glTexCoord2d(_texCoords[u + 1][v + 2].x, _texCoords[u + 1][v + 2].y);
        glNormal3d(_vertexNormals[u + 1][v + 2].x,
                   _vertexNormals[u + 1][v + 2].y,
                   _vertexNormals[u + 1][v + 2].z);
        glVertex3d(_dataPoints[u + 1][v + 2].x, _dataPoints[u + 1][v + 2].y,
                   _dataPoints[u + 1][v + 2].z);

        glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
        glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                   _vertexNormals[u][v + 1].z);
        glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                   _dataPoints[u][v + 1].z);

        glTexCoord2d(_texCoords[u][v + 2].x, _texCoords[u][v + 2].y);
        glNormal3d(_vertexNormals[u][v + 2].x, _vertexNormals[u][v + 2].y,
                   _vertexNormals[u][v + 2].z);
        glVertex3d(_dataPoints[u][v + 2].x, _dataPoints[u][v + 2].y,
                   _dataPoints[u][v + 2].z);
      }
    }
    glEnd();
  }
}

void TerrainPatch::drawSolid() {
  // Draw the interior, the border will be handled differently to avoid cracks
  int uStart, vStart, uEnd, vEnd;
  if (cracks.N)
    vStart = 1;
  else
    vStart = 0;

  if (cracks.S)
    vEnd = _numV - 2;
  else
    vEnd = _numV - 1;

  if (cracks.W)
    uStart = 1;
  else
    uStart = 0;

  if (cracks.E)
    uEnd = _numU - 2;
  else
    uEnd = _numU - 1;

  bool flipped = false;
  for (int u = uStart; u < uEnd; ++u) {
    glBegin(GL_TRIANGLE_STRIP);
    for (int v = vStart; v < vEnd; ++v) {
      // If the angle between the normals of A and D is greater then the angle
      // between B and C we want to triangulate between A and D to avoid making
      // a cross in the wrong way a-b | | c-d
      if (_vertexNormals[u][v] * _vertexNormals[u + 1][v + 1] <
          _vertexNormals[u + 1][v] * _vertexNormals[u][v + 1]) {
        if (v == vStart || flipped == true) // Do all 4 points
        {
          glEnd();
          glBegin(GL_TRIANGLE_STRIP);
          // A
          glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
          glNormal3d(-_vertexNormals[u][v].x, -_vertexNormals[u][v].y,
                     -_vertexNormals[u][v].z);
          glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y,
                     _dataPoints[u][v].z);
          // B
          glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
          glNormal3d(-_vertexNormals[u + 1][v].x, -_vertexNormals[u + 1][v].y,
                     -_vertexNormals[u + 1][v].z);
          glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                     _dataPoints[u + 1][v].z);
          // C
          glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
          glNormal3d(-_vertexNormals[u][v + 1].x, -_vertexNormals[u][v + 1].y,
                     -_vertexNormals[u][v + 1].z);
          glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                     _dataPoints[u][v + 1].z);
          // D
          glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
          glNormal3d(-_vertexNormals[u + 1][v + 1].x,
                     -_vertexNormals[u + 1][v + 1].y,
                     -_vertexNormals[u + 1][v + 1].z);
          glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                     _dataPoints[u + 1][v + 1].z);

          flipped = false;
        } else // Just do C and D
        {
          // C
          glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
          glNormal3d(-_vertexNormals[u][v + 1].x, -_vertexNormals[u][v + 1].y,
                     -_vertexNormals[u][v + 1].z);
          glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                     _dataPoints[u][v + 1].z);
          // D
          glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
          glNormal3d(-_vertexNormals[u + 1][v + 1].x,
                     -_vertexNormals[u + 1][v + 1].y,
                     -_vertexNormals[u + 1][v + 1].z);
          glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                     _dataPoints[u + 1][v + 1].z);
        }
      } else {
        if (v == vStart || flipped == false) // Do all 4 points
        {
          glEnd();
          glBegin(GL_TRIANGLE_STRIP);

          // B
          glTexCoord2d(_texCoords[u + 1][v].x, _texCoords[u + 1][v].y);
          glNormal3d(_vertexNormals[u + 1][v].x, _vertexNormals[u + 1][v].y,
                     _vertexNormals[u + 1][v].z);
          glVertex3d(_dataPoints[u + 1][v].x, _dataPoints[u + 1][v].y,
                     _dataPoints[u + 1][v].z);
          // A
          glTexCoord2d(_texCoords[u][v].x, _texCoords[u][v].y);
          glNormal3d(_vertexNormals[u][v].x, _vertexNormals[u][v].y,
                     _vertexNormals[u][v].z);
          glVertex3d(_dataPoints[u][v].x, _dataPoints[u][v].y,
                     _dataPoints[u][v].z);

          // D
          glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
          glNormal3d(_vertexNormals[u + 1][v + 1].x,
                     _vertexNormals[u + 1][v + 1].y,
                     _vertexNormals[u + 1][v + 1].z);
          glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                     _dataPoints[u + 1][v + 1].z);
          // C
          glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
          glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                     _vertexNormals[u][v + 1].z);
          glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                     _dataPoints[u][v + 1].z);

          flipped = true;

        } else // Just do C and D
        {

          // D
          glTexCoord2d(_texCoords[u + 1][v + 1].x, _texCoords[u + 1][v + 1].y);
          glNormal3d(_vertexNormals[u + 1][v + 1].x,
                     _vertexNormals[u + 1][v + 1].y,
                     _vertexNormals[u + 1][v + 1].z);
          glVertex3d(_dataPoints[u + 1][v + 1].x, _dataPoints[u + 1][v + 1].y,
                     _dataPoints[u + 1][v + 1].z);
          // C
          glTexCoord2d(_texCoords[u][v + 1].x, _texCoords[u][v + 1].y);
          glNormal3d(_vertexNormals[u][v + 1].x, _vertexNormals[u][v + 1].y,
                     _vertexNormals[u][v + 1].z);
          glVertex3d(_dataPoints[u][v + 1].x, _dataPoints[u][v + 1].y,
                     _dataPoints[u][v + 1].z);
        }
      }
    }
    glEnd();
  }
}
