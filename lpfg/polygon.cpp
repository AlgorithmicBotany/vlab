/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#include "polygon.h"
#include "asrt.h"
#include "glutils.h"
#include "glenv.h"
#include "drawparam.h"
#include "utils.h"
#include <iostream>

#ifndef CALLBACK
#define CALLBACK
#endif

namespace GLDraw {

// A global array for extra vertices created by GLUtesselator
// Each entry is GLdouble[6]: (x,y,z,r,g,b)
// So this implementation is not thread safe!
static GLdouble tessVertices[64][6];
static unsigned int numTessVertices;

void CALLBACK tessError(GLenum errorCode) {
  const GLubyte *errorString;
  errorString = gluErrorString(errorCode);
  Utils::Message("[ERROR]: %s\n",errorString);
  exit(0);
}

/// This is only a basic combine callback for the glTesselator. It takes in the
///   new vertex given by the tesselator, and passes it through.
void CALLBACK tessCombine(GLdouble coords[3], const GLdouble *vertexData[4],
        const GLfloat weight[4], GLdouble **outData) {
  // GLdouble *newVertex = new GLdouble[6];
  // that "new" above causes a memory leak, because it is not deleted
  // so replace with static array (tessVertices)...
  GLdouble newVertex[6];
  newVertex[0] = coords[0];
  newVertex[1] = coords[1];
  newVertex[2] = coords[2];

  for (int i = 3; i < 6; i++) {
    newVertex[i] = 0.0;
    for (int j = 0; j < 4; j++) {
      // gluTesselator sets vertexData pointer to NULL if weight != 0.0
      if (weight[j] > 0.0) {
        newVertex[i] += weight[j] * vertexData[j][i];
      }
    }
  }
  
  *outData = GLDraw::tessVertices[GLDraw::numTessVertices];
  if (GLDraw::numTessVertices < 64)
    ++GLDraw::numTessVertices;
  else {
    Utils::Message("GLU Tesselator reached maximum (64) vertices in GLDraw::tessCombine\n");
  }
}

GLDraw::Polygon::Polygon() : _p(), _n(), _c(), _count(0), _started(false) {
  //  Make one Tesselator, and keep it forever. This is MUCH faster than
  //    making a new Tesselator on every end polygon call.
  tess = gluNewTess();
  if (!tess) {
    Utils::Error("Warning: Cannot initialize tesselator. Concave polygons will "
                 "not be drawn\n");
  }

  //  Use only the basic glBegin and glEnd functions, as the Tesselator is
  //    not doing anything special to the object other than allowing certain
  //    shapes.
  gluTessCallback(tess, GLU_TESS_BEGIN, (void(CALLBACK *)())glBegin);
  gluTessCallback(tess, GLU_TESS_END, (void(CALLBACK *)())glEnd);
  gluTessCallback(tess, GLU_TESS_ERROR, (void(CALLBACK *)())tessError);

  // Use glVertex3fv and not glVertex3dv because the polygon's vertices are
  // stored as floats and not doubles
  gluTessCallback(tess, GLU_TESS_VERTEX, (void(CALLBACK *)())glVertex3fv);
  //  Use custom combine function since no basic combine is defined in OpenGL.
  gluTessCallback(tess, GLU_TESS_COMBINE, (void(CALLBACK *)())tessCombine);

  // reset the global variable for number of extra vertices created by tess
  GLDraw::numTessVertices = 0;
}

GLDraw::Polygon::~Polygon() { gluDeleteTess(tess); }

void GLDraw::Polygon::Start() {
  _started = true;
  _count = 0;
  _p.clear();
  _n.clear();
  _c.clear();
}

void GLDraw::Polygon::AddVertex(Vector3d p, Vector3d n, int c) {
  ASSERT(Started());
  ASSERT(!Full());
  _p.push_back(p);
  _n.push_back(n);
  _c.push_back(c);
  ++_count;
}

Vector3d GLDraw::Polygon::calculateAutomaticNormals(){
  Vector3d firstEdge = _p[1] - _p[0];
  Vector3d lastEdge = _p[2] - _p[1];
  Vector3d normal = firstEdge % lastEdge;
  if (normal.Length() < 0.00001){
    Utils::Message("Warning: Automatically computed normal: normals are closed to zero and cannot be normalized\n");
  }
  else{
    normal.Normalize();
  }
  if (normal.IsNull()){
    Utils::Message("Warning: Automatically computed normal: the first 2 edges are most likely colinear\n");
  }
  return normal;
}

void GLDraw::Polygon::End() {
  ASSERT(Started());

  _started = false;

  if (_count < 3)
    return;
 
  if (drawparams.ConcavePolygons() == DParams::concaveOn) {
    //  We do not need the Tesselator context (where NULL is written) due
    //    to the fact that we already have a list of the points.
    gluTessBeginPolygon(tess, NULL);
    gluTessBeginContour(tess);
    if (drawparams.AutomaticNormals()){
      Vector3d normal = calculateAutomaticNormals();
      gluTessNormal(tess, normal.X(), normal.Y(), normal.Z());
    }
    else{
      gluTessNormal(tess, _n[0].X(), _n[0].Y(), _n[0].Z());
    }
    GLdouble coords[3];
    for (int i = 0; i < _count; ++i) {
      coords[0] = _p[i].X();
      coords[1] = _p[i].Y();
      coords[2] = _p[i].Z();

      gl.SetColor(_c[i]);
      gluTessVertex(tess, coords, &_p[i]);
    }

    gluTessEndContour(tess);
    gluTessEndPolygon(tess);
  } else {
    if (drawparams.AutomaticNormals()){
      Vector3d normal = calculateAutomaticNormals();
      GLprimitive polygon(GL_POLYGON);
      for (int i = 0; i < _count; ++i){
        gl.SetColor(_c[i]);
        glNormal3fv(normal);
        glVertex3fv(_p[i]);
      }

    }
    else{
      GLprimitive polygon(GL_POLYGON);

      for (int i = 0; i < _count; ++i) {
        gl.SetColor(_c[i]);
        glNormal3fv(_n[i]);
        glVertex3fv(_p[i]);
      }
    }
  }
}

} // namespace GLDraw


// Another polygon class that uses GLU tesselator to find vertices
// for concave polygons. It is used for outputting the concave
// polygon as a triangle mesh (and not for rendering).
namespace GLTesselate {

// Array to hold new vertex data created when edges intersect.
// Stored as GLdouble[6]: x y z r g b
// because of the tessPolygonCombine callback.
static GLdouble tessVertices[64][6];
static unsigned int numTessVertices;

// Array of vertices in the tesselated polygon.
// Each entry is the vertex of a triangle,
// so vertices are in groups of 3
static std::vector<Vector3d> triangleVertices;

void CALLBACK tessPolygonBegin(GLenum which)
{
  return;
}

void CALLBACK tessPolygonEnd(void)
{
  return;
}

void CALLBACK tessError(GLenum errorCode) {
  const GLubyte *errorString;
  errorString = gluErrorString(errorCode);
  Utils::Message("[ERROR]: %s\n",errorString);
  exit(0);
}

void CALLBACK tessPolygonEdgeFlag(GLboolean b)
{
  return;
}

void CALLBACK tessPolygonCombine(GLdouble coords[3], const GLdouble *vertexData[4],
        const GLfloat weight[4], GLdouble **outData) {
  GLdouble newVertex[6];
  newVertex[0] = coords[0];
  newVertex[1] = coords[1];
  newVertex[2] = coords[2];

  for (int i = 3; i < 6; i++) {
    newVertex[i] = 0.0;
    for (int j = 0; j < 4; j++) {
      // gluTesselator sets vertexData pointer to NULL if weight != 0.0
      if (weight[j] > 0.0) {
        newVertex[i] += weight[j] * vertexData[j][i];
      }
    }
  }

  *outData = GLTesselate::tessVertices[GLTesselate::numTessVertices];
  if (GLTesselate::numTessVertices < 64)
    ++GLTesselate::numTessVertices;
  else {
    Utils::Message("GLU Tesselator reached maximum (64) vertices in GLTesselate::tessCombine\n");
  }
}

void CALLBACK tessPolygonVertex(const GLvoid *vertex_data) {

  const GLfloat *pt = (const GLfloat *) vertex_data;

  Vector3d v(*pt,*(pt+1),*(pt+2));
  triangleVertices.push_back(v);
}

GLTesselate::Polygon::Polygon() {
  // create the tesselator
  tess = gluNewTess();
  if (!tess) {
    Utils::Error("Warning: Cannot initialize tesselator. Concave polygons will "
                 "not be outputted.\n");
  }

  // Use custom callback functions
  gluTessCallback(tess, GLU_TESS_BEGIN, (void(CALLBACK *)())tessPolygonBegin);
  gluTessCallback(tess, GLU_TESS_END, (void(CALLBACK *)())tessPolygonEnd);
  gluTessCallback(tess, GLU_TESS_ERROR, (void(CALLBACK *)())tessError);
  gluTessCallback(tess, GLU_TESS_VERTEX, (void(CALLBACK *)())tessPolygonVertex);
  gluTessCallback(tess, GLU_TESS_COMBINE, (void(CALLBACK *)())tessPolygonCombine);
  // force gluTesselator to return independent triangles
  // and no triangle fans or strips
  gluTessCallback(tess, GLU_TESS_EDGE_FLAG, (void(CALLBACK *)())tessPolygonEdgeFlag);
}

GLTesselate::Polygon::~Polygon() { gluDeleteTess(tess); }

const std::vector<Vector3d> & GLTesselate::Polygon::tesselate(std::vector<Vector3d> &vertices) {

  // reset the global variable for number of extra vertices created by tess
  GLTesselate::numTessVertices = 0;
  GLTesselate::triangleVertices.clear();

  if (vertices.size() < 3)
    return GLTesselate::triangleVertices;
 
  gluTessBeginPolygon(tess, NULL);
  gluTessBeginContour(tess);
    
  // too bad the polygon vertices need to be cast to double...
  GLdouble vertex[3];
  for (size_t i = 0; i < vertices.size(); ++i) {
    vertex[0] = (GLdouble) vertices[i].X();
    vertex[1] = (GLdouble) vertices[i].Y();
    vertex[2] = (GLdouble) vertices[i].Z();

    gluTessVertex(tess, vertex, &vertices[i]);
  }

  gluTessEndContour(tess);
  gluTessEndPolygon(tess);

  return GLTesselate::triangleVertices;
}

} // end namespace GLTesselate



// -------------------------------------------------------------
// The triangulation of concave polygons using GLTesselate
// is not great because it is not thread safe. It uses global
// arrays to store the triangles and sends a pointer back to this
// global array (even though the global array has file scope!).
// In addition, the code is duplicated (to a high degree)
// from the GLDraw code (GLU tesselator is used in both classes).
// Consequently, I've put here an implementation of the ear-clipping
// algorithm that could be used instead of the GLU tesselator.
// The functions below, however, use V3f (the vector class from
// lpfg models) and not Vector3d (the vector class internal to lpfg).
#if 0
bool Polygon::vertexInTriangle(const V3f &p, const V3f &v1, const V3f &v2, const V3f &v3, const V3f &normal) {
    // assume CCW order of vertices
    // find vectors connecting to p
    V3f a = (v1 - p).Normalized();
    V3f b = (v2 - p).Normalized();
    V3f c = (v3 - p).Normalized();

    // if sum of angles is < 2pi, point is inside
    float angle1 = std::abs(std::atan2((a % b) * normal, a * b));
    float angle2 = std::abs(std::atan2((b % c) * normal, b * c));
    float angle3 = std::abs(std::atan2((c % a) * normal, c * a));
    float sum = angle1 + angle2 + angle3;
    if (sum >= 2.*M_PI-1e-5)
        return true;
    else
        return false;
}

float Polygon::signedAngleBetweenVectors (const V3f &vl, const V3f &v, const V3f &vr, const V3f &normal) {
    V3f a = (vl - v).Normalized();
    V3f b = (vr - v).Normalized();
    return std::atan2((a % b) * normal, a * b);
}

void Polygon::tesselate(const std::vector<V3f> &polygon, const V3f &normal, std::vector<size_t> &triangleIDs) {
// input: polygon vertices, polygon normal (assume CCW order of vertices)
// output: triangle indicies, a 1-D array storing sets of indicies in groups of three

    std::vector<size_t> pID; // polygon vertex indices (could be std::list)
    std::unordered_set<size_t> rID; // reflex-vertex indices
    std::unordered_set<size_t> eID; // ear-vertex indices

    if (polygon.size() < 3)
        produce;

    for (size_t i = 0; i < polygon.size(); i++) {
        // put indicies in array
        pID.push_back(i);
        // check if vertex i is reflex
        size_t il = i == 0 ? polygon.size()-1 : i-1;
        size_t ir = i+1 >= polygon.size() ? 0 : i+1;
        float angle = signedAngleBetweenVectors(polygon[il],polygon[i],polygon[ir],normal);
        if (angle <= 0)
            rID.insert(i);
    }

    // make list of ears (triangles without vertices inside)
    for (size_t i = 0; i < polygon.size(); i++) {
        // skip reflex vertices (only convex vertices can be ears)
        if (rID.count(i))
            continue;

        size_t il = i == 0 ? polygon.size()-1 : i-1;
        size_t ir = i+1 >= polygon.size() ? 0 : i+1;

        // test for reflex vertices in triangle il, i, ir
        bool found = false;
        for (const auto & r : rID) {
            if (vertexInTriangle(polygon[r],polygon[il],polygon[i],polygon[ir],normal)) {
                found = true;
                break;
            }
        }
        if (!found)
            eID.insert(i);
    }

    // triangulate by ear clipping
    while (pID.size() >= 3) {
        // get index of ear vertex
        size_t eID0 = *eID.begin();
        // find its position in vertex list
        size_t i = 0;
        while (pID[i] != eID0 && i < pID.size()) {
            ++i;
        }
        // get neighbours
        size_t il = i == 0 ? pID.size()-1 : i-1;
        size_t ir = i+1 >= pID.size() ? 0 : i+1;

        // save triangle il,i,ir
        triangleIDs.push_back(pID[il]);
        triangleIDs.push_back(pID[i]);
        triangleIDs.push_back(pID[ir]);

        // remove vertex i from ear and polygon index lists
        eID.erase(eID0);
        // update il and ir because the size of pID decreased
        // if il was pointing to last element, subtract one 
        if (il == pID.size())
            --il;
        // decrease ir (unless ir == 0)
        if (ir > 0)
            --ir; 
        pID.erase(std::begin(pID) + i);

        // update status of adjacent vertex il
        size_t ill = pID[il == 0 ? pID.size()-1 : il-1];
        size_t ilr = pID[il+1 >= pID.size() ? 0: il+1];
        il = pID[il];

        float anglel = signedAngleBetweenVectors(polygon[ill],polygon[il],polygon[ilr],normal);
        // check if left vertex is now a reflex vertex
        if (anglel <= 0) {
            rID.insert(il);
        }
        else {
            // remove from list of reflex if found
            rID.erase(il);

            // check if vertex is an ear now
            bool found = false;
            for (const auto & r : rID) {
              if (vertexInTriangle(polygon[r],polygon[ill],polygon[il],polygon[ilr],normal)) {
                  found = true;
                  break;
              }
            }
            if (!found) {
              eID.insert(il);
            } else {
              eID.erase(il);
            }
        }

        // update status of adjacent vertex ir
        size_t irl = pID[ir == 0 ? pID.size()-1 : ir-1];
        size_t irr = pID[ir+1 >= pID.size() ? 0: ir+1];
        ir = pID[ir];

        float angler = signedAngleBetweenVectors(polygon[irl],polygon[ir],polygon[irr],normal);
        // check if left vertex is now a reflex vertex
        if (angler <= 0) {
            rID.insert(ir);
        }
        else {
            // remove from list of reflex if found
            rID.erase(ir);

            // check if vertex is an ear now
            bool found = false;
            for (const auto & r : rID) {
              if (vertexInTriangle(polygon[r],polygon[irl],polygon[ir],polygon[irr],normal)) {
                  found = true;
                  break;
              }
            }
            if (!found) {
              eID.insert(ir);
            } else {
              eID.erase(ir);
            }
        }
    }
}
#endif
