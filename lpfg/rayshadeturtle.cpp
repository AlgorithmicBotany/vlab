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



#include "turtle.h"

#include "materialset.h"
#include "glenv.h"
#include "projection.h"
#include "vector3d.h"
#include "lightsrc.h"
#include "surface.h"
#include "surfarr.h"
#include "BsurfaceObj.h"
#include "bsurfarr.h"
#include "texturearr.h"
#include "contourarr.h"
#include "utils.h"
#include "drawparam.h"
#include "mesharr.h" // MC - Dec. 2020 - for drawing meshes

#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <iomanip>
#include <math.h>

#ifdef WIN32
#include <float.h>
#define ISNAN(x) _isnan(x)
#else
#define ISNAN(x) std::isnan(x)
#endif

//  NOTE: Unsure what exactly the constant should be set as, but if it is set
//    too low, Rayshade may encounter problems with objects that are too small,
//    for various reasons.
const float RayshadeTurtle::epsilon = 0.0003f;

RayshadeTurtle::RayshadeTurtle(std::ofstream &target,
                               const Projection &currentProjection,
                               GLEnv &glEnv, std::string fname)
    : _target(target), _CurrentContour(0), _ContourId2(0), _blender(0.0f),
      _allowBranchGC(false), _PolygonStarted(false) {
  _textureV = 0.0f;
  _textureVCoeff = 1.0f;
  target.setf(std::ios::fixed, std::ios::floatfield);
  target.setf(std::ios::showpoint);
  //  Output data for SURFACES.
  target << "#ifndef NOSURFACES\n";
  //  NOTE: Can potentially filter out unused materials. Would need to go
  //    through the system on a first pass and keep track of which surfaces
  //    are used in order to do so.
  for (int i = 0; i < 256; i++) {
    PrintMaterial(glEnv, i);
  }
  target << "#endif\n";

  //  Output data for HEADER.
  target << "#ifndef NOHEADERS\n";
  PrintHeader(currentProjection, glEnv);
  target << "#endif\n";

  //  Output data for BBOX.
  target << "\n#ifdef BBOX\n";
  PrintBBOX(currentProjection, fname);
  target << "#else\n";
}

void RayshadeTurtle::operator=(const RayshadeTurtle &src) {
  Turtle::operator=(src);
  _CurrentContour = src._CurrentContour;
  _ContourId2 = src._ContourId2;
  _blender = src._blender;
  _gc = src._gc;
  _CurrentTexture = src._CurrentTexture;
  _textureV = src._textureV;
  _textureVCoeff = src._textureVCoeff;
}

void RayshadeTurtle::F(float v) {
  if (_gc.On()) {
    _GCF(v);
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    _NormalF(v);
}

/// Print the cylinder bounding the line produced when the turtle moves forward.
/// Assuming cylinder mode right now.
void RayshadeTurtle::_NormalF(float v) {
  const Vector3d oldPosition = _position;
  Turtle::F(v);
  //  Polygon header tag handled by StartPolygon.
  if (!_PolygonStarted) {
    if (Width() > epsilon && v > epsilon) {
      switch (drawparams.LineStyle()) {
      case DParams::lsCylinder: {
        _target << "cylinder s" << std::setfill('0') << std::setw(3) << _color;
        _target << std::setfill(' ') << std::setw(0);
        _target << " " << Width() / 2.0f << " ";
        _target << oldPosition.X() << " " << oldPosition.Y() << " "
                << oldPosition.Z() << "\n " << _position.X() << " "
                << _position.Y() << " " << _position.Z() << "\n";

        break;
      }
      case DParams::lsPolygon: {
        _target << "poly s" << std::setfill('0') << std::setw(3) << _color
                << "\n";
        _target << std::setfill(' ') << std::setw(0);

        Vector3d p1(oldPosition + (_left * (Width() / 2.0f)));
        Vector3d p2(oldPosition - (_left * (Width() / 2.0f)));
        Vector3d p3(_position - (_left * (Width() / 2.0f)));
        Vector3d p4(_position + (_left * (Width() / 2.0f)));

        _target << p1.X() << " " << p1.Y() << " " << p1.Z() << "\n";
        _target << p2.X() << " " << p2.Y() << " " << p2.Z() << "\n";
        _target << p3.X() << " " << p3.Y() << " " << p3.Z() << "\n";
        _target << p4.X() << " " << p4.Y() << " " << p4.Z() << "\n";

        break;
      }
      default: {
        _target << "cylinder s" << std::setfill('0') << std::setw(3) << _color;
        _target << std::setfill(' ') << std::setw(0);
        _target << " " << Width() / 20.0f << " ";
        _target << oldPosition.X() << " " << oldPosition.Y() << " "
                << oldPosition.Z() << "\n " << _position.X() << " "
                << _position.Y() << " " << _position.Z() << "\n";

        break;
      }
      }
    }
  }
}

void RayshadeTurtle::_GCF(float v) {
  //  This should only ever be called in between StartGC and EndGC.
  ASSERT(_gc.On());

  Turtle::F(v);

  OpenGLMatrix begin;
  OpenGLMatrix end;

  begin.Translate(_gc.Position().X(), _gc.Position().Y(), _gc.Position().Z());
  const float beginRotation[16] = {_gc.Left().X(),
                                   _gc.Left().Y(),
                                   _gc.Left().Z(),
                                   0.0f,
                                   _gc.Up().X(),
                                   _gc.Up().Y(),
                                   _gc.Up().Z(),
                                   0.0f,
                                   _gc.Heading().X(),
                                   _gc.Heading().Y(),
                                   _gc.Heading().Z(),
                                   0.0f,
                                   0.0f,
                                   0.0f,
                                   0.0f,
                                   1.0f};
  begin.Multiply(beginRotation);
  begin.Scale(0.5f * _gc.ScaleP(), 0.5f * _gc.ScaleQ(), 1.0f);

  end.Translate(_position.X(), _position.Y(), _position.Z());
  const float endRotation[16] = {
      _left.X(), _left.Y(), _left.Z(),    0.0f,         _up.X(),      _up.Y(),
      _up.Z(),   0.0f,      _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
      0.0f,      0.0f,      0.0f,         1.0f};
  end.Multiply(endRotation);
  end.Scale(0.5f * _ScaleP(), 0.5f * _ScaleQ(), 1.0f);

  float endtv = _textureV + v * _textureVCoeff;
  if (v == 0.f)
    endtv =  _textureV + _textureVCoeff;

  const Contour &beginCenter =
      contours.Get1(_gc.ContourId(), _gc.ContourId2(), _gc.Blender());
  const Contour &endCenter =
      contours.Get2(_CurrentContour, _ContourId2, _blender);

  /*
   *  Cylinder is made from strips.
   *
   *  v2--o--o--...
   *   |\ |\ |\
   *   | \| \| \
   *  v1--o--o--...
   */

  Vector3d vertex1;
  vertex1.Transform(beginCenter.Vertex(0), begin.Buf());
  Vector3d normal1;
  normal1.Transform(beginCenter.Normal(0), beginRotation);
  Vector3d vertex2;
  vertex2.Transform(endCenter.Vertex(0), end.Buf());
  Vector3d normal2;
  normal2.Transform(endCenter.Normal(0), endRotation);

  for (size_t i = 1; i < beginCenter.Divisions(); i++) {
    /*
     *  Each strip is made from 2 triangles.
     *
     *       end side
     *
     *      v2----v4--...
     *       |\   |
     * i - 1 | \  | i
     * side  |  \ | side
     *       |   \|
     *      v1----v3--...
     *
     *       begin side
     */
    Vector3d vertex3;
    vertex3.Transform(beginCenter.Vertex(i), begin.Buf());
    Vector3d normal3;
    normal3.Transform(beginCenter.Normal(i), beginRotation);
    Vector3d vertex4;
    vertex4.Transform(endCenter.Vertex(i), end.Buf());
    Vector3d normal4;
    normal4.Transform(endCenter.Normal(i), endRotation);

    //  If the normals are wrong, guess at what they should have been.
    if (normal1.Length() < epsilon)
      normal1 = ((vertex3 - vertex1) % (vertex1 - vertex2)).Normalize();
    if (normal2.Length() < epsilon)
      normal2 = ((vertex1 - vertex2) % (vertex2 - vertex4)).Normalize();
    if (normal3.Length() < epsilon)
      normal3 = ((vertex4 - vertex3) % (vertex3 - vertex1)).Normalize();
    if (normal4.Length() < epsilon)
      normal4 = ((vertex2 - vertex4) % (vertex4 - vertex3)).Normalize();

    PrintSurfaceUse();

    //  First triangle.
    _target << "triangle";
    if (_TextureOn())
      _target << "uv";
    _target << "\n";

    _target << vertex1.X() << " " << vertex1.Y() << " " << vertex1.Z() << " "
            << normal1.X() << " " << normal1.Y() << " " << normal1.Z();
    if (_TextureOn())
      _target << " " << ((i - 1) * 1.0f / (beginCenter.Divisions() - 1)) << " "
              << (1 - _textureV);
    _target << "\n";

    _target << vertex2.X() << " " << vertex2.Y() << " " << vertex2.Z() << " "
            << normal2.X() << " " << normal2.Y() << " " << normal2.Z();
    if (_TextureOn())
      _target << " " << ((i - 1) * 1.0f / (beginCenter.Divisions() - 1)) << " "
              << (1 - endtv);
    _target << "\n";

    _target << vertex3.X() << " " << vertex3.Y() << " " << vertex3.Z() << " "
            << normal3.X() << " " << normal3.Y() << " " << normal3.Z();
    if (_TextureOn())
      _target << " " << (i * 1.0f / (beginCenter.Divisions() - 1)) << " "
              << (1 - _textureV);
    _target << "\n";

    if (_TextureOn())
      _target << "texture image " << textures.getFilename(_CurrentTexture)
              << "\nmap uv\n";

    //  Second triangle.
    _target << "triangle";
    if (_TextureOn())
      _target << "uv";
    _target << "\n";

    _target << vertex2.X() << " " << vertex2.Y() << " " << vertex2.Z() << " "
            << normal2.X() << " " << normal2.Y() << " " << normal2.Z();
    if (_TextureOn())
      _target << " " << ((i - 1) * 1.0f / (beginCenter.Divisions() - 1)) << " "
              << (1 - endtv);
    _target << "\n";

    _target << vertex4.X() << " " << vertex4.Y() << " " << vertex4.Z() << " "
            << normal4.X() << " " << normal4.Y() << " " << normal4.Z();
    if (_TextureOn())
      _target << " " << (i * 1.0f / (beginCenter.Divisions() - 1)) << " "
              << (1 - endtv);
    _target << "\n";

    _target << vertex3.X() << " " << vertex3.Y() << " " << vertex3.Z() << " "
            << normal3.X() << " " << normal3.Y() << " " << normal3.Z();
    if (_TextureOn())
      _target << " " << (i * 1.0f / (beginCenter.Divisions() - 1)) << " "
              << (1 - _textureV);
    _target << "\n";

    if (_TextureOn())
      _target << "texture image " << textures.getFilename(_CurrentTexture)
              << "\nmap uv\n";

    //  Shift to the next edge and repeat.
    vertex1 = vertex3;
    normal1 = normal3;
    vertex2 = vertex4;
    normal2 = normal4;
  }

  //  Keep track of the _textureV that was used last, can make the texture
  //    between multiple cylinders look continuous.
  if (_TextureOn()) {
    _textureV = endtv;
    if (_textureV >= 1.0f)
      _textureV -= 1.0f;
  }
}

void RayshadeTurtle::Sphere(float radius) const {
  if (radius > epsilon) {
    _target << "sphere s" << std::setfill('0') << std::setw(3) << _color;
    _target << std::setfill(' ') << std::setw(0);
    _target << " " << radius << " " << _position.X() << " " << _position.Y()
            << " " << _position.Z() << "\n";
  }
}

void RayshadeTurtle::Circle(float radius) const {
  if (radius > epsilon) {
    _target << "disc s" << std::setfill('0') << std::setw(3) << _color;
    _target << std::setfill(' ') << std::setw(0);
    _target << " " << radius << " ";
    _target << _position.X() << " " << _position.Y() << " " << _position.Z()
            << " " << _up.X() << " " << _up.Y() << " " << _up.Z() << "\n";
  }
}
void RayshadeTurtle::CircleFront(float radius) const {
  if (radius > epsilon) {
    _target << "disc s" << std::setfill('0') << std::setw(3) << _color;
    _target << std::setfill(' ') << std::setw(0);
    _target << " " << radius << " ";
    _target << _position.X() << " " << _position.Y() << " " << _position.Z()
            << " " << 0 << " " << 0 << " " << 1 << "\n";
  }
}

void RayshadeTurtle::CircleB(float radius) const {
  if (radius > epsilon) {
    _target << "opendisc s" << std::setfill('0') << std::setw(3) << _color;
    _target << std::setfill(' ') << std::setw(0);
    _target << " " << radius + Width() / 2.0f << " " << radius - Width() / 2.0f
            << " ";
    _target << _position.X() << " " << _position.Y() << " " << _position.Z()
            << " " << _up.X() << " " << _up.Y() << " " << _up.Z() << "\n";
  }
}
void RayshadeTurtle::CircleFrontB(float radius) const {
  if (radius > epsilon) {
    _target << "opendisc s" << std::setfill('0') << std::setw(3) << _color;
    _target << std::setfill(' ') << std::setw(0);
    _target << " " << radius + Width() / 2.0f << " " << radius - Width() / 2.0f
            << " ";
    _target << _position.X() << " " << _position.Y() << " " << _position.Z()
            << " " << 0 << " " << 0 << " " << 1 << "\n";
  }
}

void RayshadeTurtle::Triangle(float a, float b) const {
  //  If a or b are 0, then this triangle would be degenerate.
  if (a > epsilon && b > epsilon) {
    PrintSurfaceUse();
    /*
     *        v3
     *       /  \
     *      /    \
     *     /  ^H  \
     *    /L  |    \
     *  v1<-Turtle--v2
     */
    Vector3d v1(_position + _left * a * 0.5f);
    Vector3d v2(_position - _left * a * 0.5f);
    Vector3d v3(_position + _heading * b);

    //  NOTE: Individual triangles will never have textures.
    _target << "triangle\n";

    _target << v1.X() << " " << v1.Y() << " " << v1.Z() << " " << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";

    _target << v2.X() << " " << v2.Y() << " " << v2.Z() << " " << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";

    _target << v3.X() << " " << v3.Y() << " " << v3.Z() << " " << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";
  }
}

void RayshadeTurtle::Rhombus(float a, float b) const {
  //  If a or b are 0, then this rhombus would be degenerate.
  if (a > epsilon && b > epsilon) {
    /*
     *      v2
     *     / \
     *    /   \
     *  v3     v4
     *    \ ^H/
     *   L \|/
     *   <--v1(Turtle)
     */
    Vector3d v1(_position);
    Vector3d v2(_position + _heading * a);
    Vector3d v3(_position + _heading * a * 0.5f + _left * b * 0.5f);
    Vector3d v4(_position + _heading * a * 0.5f - _left * b * 0.5f);

    //  NOTE: Individual rhombuses will never have textures.
    _target << "triangle\n";

    _target << v1.X() << " " << v1.Y() << " " << v1.Z() << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";

    _target << v2.X() << " " << v2.Y() << " " << v2.Z() << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";

    _target << v3.X() << " " << v3.Y() << " " << v3.Z() << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";

    _target << "triangle\n";

    _target << v1.X() << " " << v1.Y() << " " << v1.Z() << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";

    _target << v4.X() << " " << v4.Y() << " " << v4.Z() << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";

    _target << v2.X() << " " << v2.Y() << " " << v2.Z() << _up.X() << " "
            << _up.Y() << " " << _up.Z() << "\n";
  }
}

void RayshadeTurtle::PrintSurface(::Surface s, float sx, float sy,
                                  float sz) const {
  int i = 0;
  //  Iterate through each patch of the surface.
  while (s.IsValidPatchId(i)) {
    typedef std::vector<Vector3d> vectors;
    typedef std::vector<vectors> vectors2;
    vectors2 points;
    vectors2 normals;
    vectors2 textures;
    s.GetPatchGeometry(i, points, normals, textures);

    OpenGLMatrix rotationMatrix, translationMatrix, scaleMatrix;

    const float rotation[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                                _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                                _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                                0.0f,         0.0f,         0.0f,         1.0f};
    rotationMatrix.Multiply(rotation);

    translationMatrix.Translate(_position);

    scaleMatrix.Scale(sx, sy, sz);
    scaleMatrix.Scale(s.Scale(), s.Scale(), s.Scale());

    OpenGLMatrix transformMatrix;
    transformMatrix.Multiply(translationMatrix.Buf());
    transformMatrix.Multiply(rotationMatrix.Buf());
    transformMatrix.Multiply(scaleMatrix.Buf());

    //  Iterate through every quad of the patch.
    for (size_t j = 0; j < points.size() - 1; j++)
      for (size_t k = 0; k < points[j].size() - 1; k++) {
        /*
         *  j,k+1  j+1,k+1
         *  p2-----p3
         *   |\    |
         *   | \   |
         *   |  \  |
         *   |   \ |
         *   |    \|
         *  p0-----p1
         *  j,k   j+1,k
         */
        //  Points should be translated, scaled, and rotated.
        Vector3d point0 = points[j][k];
        point0.Transform(transformMatrix);
        Vector3d point1 = points[j + 1][k];
        point1.Transform(transformMatrix);
        Vector3d point2 = points[j][k + 1];
        point2.Transform(transformMatrix);
        Vector3d point3 = points[j + 1][k + 1];
        point3.Transform(transformMatrix);

        //  Normals should only be rotated.
        Vector3d normal0 = normals[j][k];
        normal0.Transform(rotationMatrix);
        normal0.Normalize();
        Vector3d normal1 = normals[j + 1][k];
        normal1.Transform(rotationMatrix);
        normal1.Normalize();
        Vector3d normal2 = normals[j][k + 1];
        normal2.Transform(rotationMatrix);
        normal2.Normalize();
        Vector3d normal3 = normals[j + 1][k + 1];
        normal3.Transform(rotationMatrix);
        normal3.Normalize();

        //  Texture coordinates are untouched.
        Vector3d texture0 = textures[j][k];
        Vector3d texture1 = textures[j + 1][k];
        Vector3d texture2 = textures[j][k + 1];
        Vector3d texture3 = textures[j + 1][k + 1];

        //  Check if the triangle 012 is degenerate (length of AB x BC would be
        //  0).
        if (((point1 - point0) % (point2 - point1)).Length() > epsilon &&
            !ISNAN(normal0.Length()) && !ISNAN(normal1.Length()) &&
            !ISNAN(normal2.Length())) {
          bool textured =
              !(texture0 == texture1 && texture1 == texture2) | _TextureOn();

          _target << "triangle";
          if (textured)
            _target << "uv";
          _target << "\n";

          _target << point0.X() << " " << point0.Y() << " " << point0.Z() << " "
                  << normal0.X() << " " << normal0.Y() << " " << normal0.Z();
          if (textured)
            _target << " " << texture0.Y() << " " << (1.0 - texture0.X());
          _target << "\n";

          //  Check if the triangle normals are consistent. If they are the
          //  same,
          //    print in order, if they are different, print out of order.
          bool _in_order =
              (((point1 - point0) % (point2 - point1)) * normal0) > 0;

          if (_in_order) {
            _target << point1.X() << " " << point1.Y() << " " << point1.Z()
                    << " " << normal1.X() << " " << normal1.Y() << " "
                    << normal1.Z();
            if (textured)
              _target << " " << texture1.Y() << " " << (1.0 - texture1.X());
            _target << "\n";
          }

          _target << point2.X() << " " << point2.Y() << " " << point2.Z() << " "
                  << normal2.X() << " " << normal2.Y() << " " << normal2.Z();
          if (textured)
            _target << " " << texture2.Y() << " " << (1.0 - texture2.X());
          _target << "\n";

          if (!_in_order) {
            _target << point1.X() << " " << point1.Y() << " " << point1.Z()
                    << " " << normal1.X() << " " << normal1.Y() << " "
                    << normal1.Z();
            if (textured)
              _target << " " << texture1.Y() << " " << (1.0 - texture1.X());
            _target << "\n";
          }
        }
        //  Check if the triangle 132 is degenerate (length of AB x BC would be
        //  0).
        if (((point2 - point1) % (point3 - point2)).Length() > epsilon &&
            !ISNAN(normal1.Length()) && !ISNAN(normal2.Length()) &&
            !ISNAN(normal3.Length())) {
          bool textured =
              !(texture3 == texture1 && texture1 == texture2) | _TextureOn();

          _target << "triangle";
          if (textured)
            _target << "uv";
          _target << "\n";

          _target << point1.X() << " " << point1.Y() << " " << point1.Z() << " "
                  << normal1.X() << " " << normal1.Y() << " " << normal1.Z();
          if (textured)
            _target << " " << texture1.Y() << " " << (1.0 - texture1.X());
          _target << "\n";

          //  Check if the triangle normals are consistent. If they are the
          //  same,
          //    print in order, if they are different, print out of order.
          bool _in_order =
              (((point2 - point1) % (point3 - point2)) * normal1) > 0;

          if (_in_order) {
            _target << point2.X() << " " << point2.Y() << " " << point2.Z()
                    << " " << normal2.X() << " " << normal2.Y() << " "
                    << normal2.Z();
            if (textured)
              _target << " " << texture2.Y() << " " << (1.0 - texture2.X());
            _target << "\n";
          }

          _target << point3.X() << " " << point3.Y() << " " << point3.Z() << " "
                  << normal3.X() << " " << normal3.Y() << " " << normal3.Z();
          if (textured)
            _target << " " << texture3.Y() << " " << (1.0 - texture3.X());
          _target << "\n";

          if (!_in_order) {
            _target << point2.X() << " " << point2.Y() << " " << point2.Z()
                    << " " << normal2.X() << " " << normal2.Y() << " "
                    << normal2.Z();
            if (textured)
              _target << " " << texture2.Y() << " " << (1.0 - texture2.X());
            _target << "\n";
          }
        }
      }
    i++;
  }
}

void RayshadeTurtle::InstanceSurfaces() const {
  for (int i = 0; surfaces.ValidId(i); i++) {
    std::stringstream sstm;
    sstm << surfaces.Get(i).Name();
    std::string name = sstm.str();
    int size[3] = {20, 20, 20};
    StartNewGrid(size, name);
    PrintSurface(surfaces.Get(i), 1, 1, 1);
    CompleteGrid();
  }
}

/// Print the target surface by decomposing it into triangles, then print
///   those individual triangles.
///   The individual triangles must be transformed by the translation of the
///   turtle, the rotation of the turtle, and the scale given by the sx, sy,
///   and sz parameters.
void RayshadeTurtle::Surface(int id, float sx, float sy, float sz) const {
  if (sx > epsilon && sy > epsilon && sz > epsilon) {
    PrintSurfaceUse();
    _target << "object " << surfaces.Get(id).Name() << "\n";
    _target << " scale " << sx << " " << sy << " " << sz << "\n";
    _target << " transform\n "
            << " " << -_left.X() << " " << -_left.Y() << " " << -_left.Z()
            << " "
            << " " << _heading.X() << " " << _heading.Y() << " " << _heading.Z()
            << " "
            << " " << _up.X() << " " << _up.Y() << " " << _up.Z() << "\n";
    _target << " translate " << _position.X() << " " << _position.Y() << " "
            << _position.Z() << "\n";
    int oldTexture = surfaces.TextureId(id);
    if (_TextureOn()) {
      if (_CurrentTexture != -1)
        surfaces.SetTextureId(id, _CurrentTexture);
      else
        surfaces.DisableTexture(id);
    }
    if (surfaces.IsTextured(id))
      _target << "texture image "
              << textures.getFilename(surfaces.TextureId(id)) << "\nmap uv\n";
    if (_TextureOn()) {
      surfaces.SetTextureId(id, oldTexture);
    }
  }
}

void RayshadeTurtle::DSurface(SurfaceObj surf, bool ) const {
  const class Surface &s(surf);
  PrintSurfaceUse();
  PrintSurface(s, 1, 1, 1);
}

void RayshadeTurtle::BSurface(int id, float sx, float sy, float sz) const {
  if (!bsurfaces.ValidId(id))
    return;

  const float rotation[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                              _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                              _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                              0.0f,         0.0f,         0.0f,         1.0f};

  OpenGLMatrix transformMatrix;
  transformMatrix.Translate(_position);
  transformMatrix.Multiply(rotation);
  transformMatrix.Scale(sx, sy, sz);

  b_wrapper &s(bsurfaces.Get(id));
  s.Transform(transformMatrix);

  s.Rayshade(GetUVPrecision(), _target, _TextureOn(),
             _TextureOn() ? textures.getFilename(_CurrentTexture) : "");
}

/// TODO: Implement.
void RayshadeTurtle::DBSurfaceS(BsurfaceObjS) const {}

/// TODO: Implement.
void RayshadeTurtle::DBSurfaceM(BsurfaceObjM) const {}

void RayshadeTurtle::StartPolygon() { _PolygonStarted = true; }
void RayshadeTurtle::EndPolygon() {
  _PolygonStarted = false;
  _target << "poly s" << std::setfill('0') << std::setw(3) << _color << "\n";
  _target << std::setfill(' ') << std::setw(0);
  for (size_t i = 0; i < polygonPoints.size(); i++)
    _target << polygonPoints[i].X() << " " << polygonPoints[i].Y() << " "
            << polygonPoints[i].Z() << "\n";
  polygonPoints.clear();
}
void RayshadeTurtle::PolygonPoint() { polygonPoints.push_back(_position); }

/// Print "applysurf s###" whenever the material in use changes in the model.
void RayshadeTurtle::IncColor() {
  Turtle::IncColor();
  PrintSurfaceUse();
}
void RayshadeTurtle::DecColor() {
  Turtle::DecColor();
  PrintSurfaceUse();
}
void RayshadeTurtle::SetColor(int c) {
  Turtle::SetColor(c);
  PrintSurfaceUse();
}

void RayshadeTurtle::_CapGC() {
    ASSERT(_gc.On());

    OpenGLMatrix bgn;
    bgn.Translate(_gc.Position().X(), _gc.Position().Y(), _gc.Position().Z());
    const float bgnrot[16] = {_gc.Left().X(),_gc.Left().Y(),_gc.Left().Z(),0.f,
                             _gc.Up().X(),_gc.Up().Y(),_gc.Up().Z(),0.f,
                             _gc.Heading().X(),_gc.Heading().Y(),_gc.Heading().Z(),0.f,
                             0.0f,0.0f,0.0f,1.0f};
    bgn.Multiply(bgnrot);
    bgn.Scale(0.5f * _gc.ScaleP(), 0.5f * _gc.ScaleQ(), 1.0f);

    const Contour &bgncntr =
        contours.Get1(_gc.ContourId(), _gc.ContourId2(), _gc.Blender());
 
    // check if a concave cap should be generated
    if (drawparams.ConcavePolygons() == DParams::concaveOn) {   
      std::vector<Vector3d> polygonPoints; 
      Vector3d v1;
      for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
        v1.Transform(bgncntr.Vertex(i), bgn.Buf());
        polygonPoints.push_back(v1);
       }

      // use the Polygon class to generate the triangles
      GLTesselate::Polygon polygon;
      auto triangleVertices = polygon.tesselate(polygonPoints);
      for (size_t i = 0; i < triangleVertices.size(); i += 3) {
        auto v1 = triangleVertices[i];
        auto v2 = triangleVertices[i+1];
        auto v3 = triangleVertices[i+2];

        _target << "triangle\n";
        _target << v1.X() << " " << v1.Y() << " " << v1.Z()
                << _gc.Heading().X() << " " << _gc.Heading().Y()
                << " " << _gc.Heading().Z() << "\n";
        _target << v2.X() << " " << v2.Y() << " " << v2.Z()
                << _gc.Heading().X() << " " << _gc.Heading().Y() 
                << " " << _gc.Heading().Z() << "\n";
        _target << v3.X() << " " << v3.Y() << " " << v3.Z() 
                << _gc.Heading().X() << " " << _gc.Heading().Y() 
                << " " << _gc.Heading().Z() << "\n";
      } 
        
    } else {
        // compute centroid
        Vector3d centroid(0,0,0);
        for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
          Vector3d v;
          v.Transform(bgncntr.Vertex(i),bgn.Buf());
          centroid += v;
        }
        centroid /= float(bgncntr.Divisions()-1);

        Vector3d v1, v2;
        for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
          v1.Transform(bgncntr.Vertex(i), bgn.Buf());
          v2.Transform(bgncntr.Vertex(i+1), bgn.Buf());

          _target << "triangle\n";
          _target << centroid.X() << " " << centroid.Y() << " " << centroid.Z()
                  << _gc.Heading().X() << " " << _gc.Heading().Y()
                  << " " << _gc.Heading().Z() << "\n";
          _target << v1.X() << " " << v1.Y() << " " << v1.Z()
                  << _gc.Heading().X() << " " << _gc.Heading().Y() 
                  << " " << _gc.Heading().Z() << "\n";
          _target << v2.X() << " " << v2.Y() << " " << v2.Z() 
                  << _gc.Heading().X() << " " << _gc.Heading().Y() 
                  << " " << _gc.Heading().Z() << "\n";
        }
    }
}

void RayshadeTurtle::StartGC() {
  if (_allowBranchGC) {
    _gc.End();
    _allowBranchGC = false;
  }
  if (_gc.On())
    Utils::Message(
        "StartGC called when generalized cylinder already started.\n");
  else {
    _gc.Start();
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);    
    if (drawparams.CappedCylinders() == DParams::cappedCylindersOn)
      _CapGC();
  }
}
void RayshadeTurtle::PointGC() {
  if (_gc.On()) {
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    Utils::Message(
        "PointGC called when there is no active generalized cylinder.\n");
}
void RayshadeTurtle::EndGC() {
  if (_gc.On()) {
    if (drawparams.CappedCylinders() == DParams::cappedCylindersOn)
      _CapGC();
    _gc.End();
  } else
    Utils::Message(
        "EndGC called when there is no active generalized cylinder.\n");
}

void RayshadeTurtle::StartBranch() {
  if (_gc.On() && _allowBranchGC == false) {
    _allowBranchGC = true;
  }
}

void RayshadeTurtle::SetWidth(float w) { ScaleContour(w, w); }

void RayshadeTurtle::ScaleContour(float p, float q) {
  Turtle::ScaleContour(p, q);
  _Scale.p = p;
  _Scale.q = q;
}

void RayshadeTurtle::ContourSides(int cs) {
  if (_gc.On())
    Utils::Message(
        "ContourSides: Called between StartGC and EndGC, ignored.\n");
  else {
    Turtle::ContourSides(cs);
  }
}

void RayshadeTurtle::ContourNormal(V3f n) {
  _gc.SetNormal(Vector3d(n.x,n.y,n.z));
}

void RayshadeTurtle::CurrentContour(int id) {
  if (id == 0 || contours.ValidId(id))
    _CurrentContour = id;
  else
    Utils::Message("Invalid contour id = %d\n", id);
}

void RayshadeTurtle::BlendContours(int id1, int id2, float b) {
  if (!contours.ValidId(id1)) {
    Utils::Message("BlendContours: Invalid contour id = %d. Module ignored.\n",
                   id1);
    return;
  }
  if (!contours.ValidId(id2)) {
    Utils::Message("BlendContours: Invalid contour id = %d. Module ignored.\n",
                   id2);
    return;
  }
  if (b < 0.0f) {
    Utils::Message("BlendContours: Blending factor %f < 0.0, assumed 0.0.\n");
    b = 0.0f;
  }
  if (b > 1.0f) {
    Utils::Message("BlendContours: Blending factor %f > 1.0, assumed 1.0.\n");
    b = 1.0f;
  }
  _CurrentContour = id1;
  _ContourId2 = id2;
  _blender = b;
}

/// Print the information for starting an object description.
///   Format is:
///     name <name>
///     grid # # #
/// NOTE: The grid numbers are used to determine the number of voxels
///   used by the rayshader at runtime. 40x40x40 is a high enough default
///   for most cases, but there could be special cases where either more or
///   less is more optimal. In the original rayshade, it's assumed that 40
///   is enough for a refined pass, while 20 is used for a more coarse pass.
void RayshadeTurtle::StartNewGrid(int *size, std::string fname) const {
  _target << "name " << fname << "\n";
  for (int i = 0; i < 3; i++)
    if (size[i] == 0)
      size[i] = 40;
  _target << "grid " << size[0] << " " << size[1] << " " << size[2] << "\n";
}

/// Print the end tag for an object description.
void RayshadeTurtle::CompleteGrid() const { _target << "end\n"; }

void RayshadeTurtle::Finish(const Projection &,
                            std::string fname) const {
  _target << "#endif\n\n";
  _target << "#ifndef NOHEADERS\n";
  _target << "object " << fname << "\n";
  //  NOTE: This transform value is for rotations specified in the drawparam
  //    configuration file. However, the drawparam class does not currently
  //    read for the rotation label, therefore no rotation can currently be
  //    inserted here. The default is the identity matrix, meaning no
  //    special rotation occurs.
  _target << " transform\n"
          << "  1 0 0\n"
          << "  0 1 0\n"
          << "  0 0 1\n";

  _target << "#endif\n";
}

/// Print the surface data for material x of the material set, withdrawn from
///   the materialset loaded into LPFG.
///   Format is:
///     surface s### ambient # # #
///                  diffuse # # #
///                  specular # # #
///                  specpow #
///                  transp #
///                  index #
///   Note that transp and index only appear when the transparency of a
///     material in the .ray file is less than 1.
/// NOTE: The material code can potentially be smart about which materials it
///   actually prints. In the CPFG version of rayshade output, it is checked
///   whether the surface is set, and keeps only the first unset surface,
///   which CPFG then labels as default. This change is rather irrelevant
///   when looking at harddrive space (1KB - 4KB), but matters greatly when
///   the file is meant to be human readable. However, surfaces that are
///   initialized via a ramp and then not used are still included, which
///   could be made more efficient, although this is somewhat unnecessary.
void RayshadeTurtle::PrintMaterial(GLEnv &glEnv, int x) {
  _target << "surface s" << std::setfill('0') << std::setw(3) << x << " ";

  MaterialSet *ms = glEnv.GetMaterialSet();

  //  For Materials.
  if (ms->Set()) {
    _target << "ambient " << ms->Ambient(x)[0] * ms->Ambient(x)[3] << " "
            << ms->Ambient(x)[1] * ms->Ambient(x)[3] << " "
            << ms->Ambient(x)[2] * ms->Ambient(x)[3] << "\n";

    _target << std::setfill(' ') << std::setw(13) << " ";
    _target << "diffuse " << ms->Diffuse(x)[0] * ms->Diffuse(x)[3] << " "
            << ms->Diffuse(x)[1] * ms->Diffuse(x)[3] << " "
            << ms->Diffuse(x)[2] * ms->Diffuse(x)[3] << "\n";

    _target << std::setw(13) << " ";
    _target << "specular " << ms->Specular(x)[0] * ms->Specular(x)[3] << " "
            << ms->Specular(x)[1] * ms->Specular(x)[3] << " "
            << ms->Specular(x)[2] * ms->Specular(x)[3] << "\n";

    _target << std::setw(13) << " ";
    _target << "specpow " << ms->Shininess(x) << "\n";

    if (ms->Transparency(x) < 1.0f) {
      _target << std::setw(13) << " ";
      _target << "transp " << (1.0f - ms->Transparency(x)) << "\n";
      _target << std::setw(13) << " ";
      _target << "index 1.0\n";
    }
  }
  //  For Colormaps.
  else {
    _target << "ambient " << 0 << " " << 0 << " " << 0 << "\n";

    _target << std::setfill(' ') << std::setw(13) << " ";
    _target << "diffuse " << glEnv.GetSolidColor(x).X() << " "
            << glEnv.GetSolidColor(x).Y() << " " << glEnv.GetSolidColor(x).Z()
            << "\n";

    _target << std::setw(13) << " ";
    _target << "specular " << 0 << " " << 0 << " " << 0 << "\n";

    _target << std::setw(13) << " ";
    _target << "specpow " << 1 << "\n";
  }
}

/// Print the header data for the file, withdrawn from the projection from
///   view.cpp.
///   Format is:
///     eyep # # #
///     lookp # # #
///     up 0 1 0
///     fov #
///     screen # #
///     background # # #
///
///     light # # # (type)
///   Only the light parameter can show up multiple times. Up is always 0 1 0.
void RayshadeTurtle::PrintHeader(const Projection &currentProjection,
                                 GLEnv &glEnv) {

  Vector3d eye = currentProjection.GetLookFrom();
  Vector3d look = currentProjection.GetLookAt();
  Vector3d up = currentProjection.ViewUp();

  _target << "eyep ";
  //  [1] Fake parallel projection by moving the camera very far back.
  //    The FOV must be adjusted by the same ratio.
  if (drawparams.ProjectionMode() == DParams::pmParallel)
    eye = eye + (eye - look) * 99;
  _target << eye.X() << " " << eye.Y() << " " << eye.Z() << "\n";

  _target << "lookp " << look.X() << " " << look.Y() << " " << look.Z() << "\n";

  _target << "up " << up.X() << " " << up.Y() << " " << up.Z() << "\n";

  float angle;
  if (drawparams.ProjectionMode() == DParams::pmPerspective)
    angle = currentProjection.GetRSFOV();
  else {
    angle = currentProjection.GetRSFOVParallel();
    float ratio = tan(angle / 180.0f * M_PI / 2.0f);
    //  This is where the FOV is scaled by the above [1] ratio. If the eye
    //    position is moved back further, this also must be changed.
    ratio /= 100;
    angle = atan(ratio) * 180.0f / M_PI * 2.0f;
  }
  _target << "fov " << angle << "\n";

  _target << "screen " << currentProjection.getWindowWidth() << " "
          << currentProjection.getWindowHeight() << "\n";

  Vector3d backgroundColor = glEnv.GetBgColor();
  _target << "background " << backgroundColor.X() << " " << backgroundColor.Y()
          << " " << backgroundColor.Z() << "\n";

  //  _lightsCount is incorrect, due to the fact that it keeps one default
  //    light, which is not the correct behaviour for this rayshade output
  //    program.
  int lightCount = glEnv.GetLightCount() - 1;
  if (lightCount == 0) {
    _target << "\n";
    _target << "light 1.0 ambient\n";
    //  NOTE: In the CPFG implementation of rayshade.c, if lights are not
    //    set in the scene, then an ambient light is set, along with a
    //    directional light withdrawn from drawparam, however, in LPFG,
    //    drawparam does not set a directional light to fall back on.
  } else {
    float ambient[3] = {0.0f, 0.0f, 0.0f};

    for (int i = 0; i < lightCount; i++) {
      float *currentAmbient = glEnv.GetLight(i).GetAmbient();
      ambient[0] += currentAmbient[0];
      ambient[1] += currentAmbient[1];
      ambient[2] += currentAmbient[2];
    }

    _target << "\n";
    _target << "light " << ambient[0] << " " << ambient[1] << " " << ambient[2]
            << " ambient\n";

    for (int i = 0; i < lightCount; i++) {
      LightSource currentLight = glEnv.GetLight(i);
      float *currentDiffuse = currentLight.GetDiffuse();

      //  Print the light diffuse's value.
      _target << "light " << currentDiffuse[0] << " " << currentDiffuse[1]
              << " " << currentDiffuse[2] << " ";

      float currentSpotlight[5];
      currentLight.GetSpotlight(currentSpotlight);
      float *currentPosition = currentLight.GetPosition();

      // MC - Nov. 2015 - the light position in lpfg is fixed, we need to undo
      // the view transformation before sending the position to rayshade (where
      // the view transformation will be applied again)
      GLfloat gl_modelview[16];
      glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview);
      OpenGLMatrix mvMatrix(gl_modelview);
      mvMatrix.Transpose(); // transpose because opengl is column-major order
      Vector3d lightPos(currentPosition[0], currentPosition[1],
                        currentPosition[2]);
      lightPos.Transform(mvMatrix); // apply the transformation

      //  Then, figure out the light's type.
      if (currentPosition[3] == 0.0f) {
        _target << "directional " << lightPos.X() << " " << lightPos.Y() << " "
                << lightPos.Z() << "\n";
      } else {
        // move the light position by camera pan
        lightPos += currentProjection.GetPan();

        // if point light...
        if (currentSpotlight[4] == 0.0f || currentSpotlight[4] == 180.0f) {
          _target << "point " << lightPos.X() << " " << lightPos.Y() << " "
                  << lightPos.Z() << "\n";
        } else {
          // apply the view transformation to spot light's direction
          Vector3d lightDir(currentSpotlight[0], currentSpotlight[1],
                            currentSpotlight[2]);
          lightDir.Transform(mvMatrix);
          Vector3d lightLookAt = lightPos + lightDir;
          // position
          _target << "spot " << lightPos.X() << " " << lightPos.Y() << " "
                  << lightPos.Z()
                  << "\n"
                  // pointing at
                  << lightLookAt.X() << " " << lightLookAt.Y() << " "
                  << lightLookAt.Z()
                  << " "
                  // coef, theta_in = theta_out
                  << currentSpotlight[3] << " " << currentSpotlight[4] << " "
                  << currentSpotlight[4] << "\n";
          // for spot lights, the lighting in lpfg and rayshade seem to not
          // always Is it because of a difference in implementation of spot
          // lights in OpenGL and Rayshade???
        }
      }
    }
  }
}

/// Print the bounding box data for the file, withdrawn from the projection
///   from view.cpp.
///   Format is:
///     name <objectname>.ray
///     box # # # # # #
void RayshadeTurtle::PrintBBOX(const Projection &currentProjection,
                               std::string fname) {
  _target << "name " << fname << "\n";

  float max[3];
  float min[3];

  min[0] = currentProjection.MinX();
  min[1] = currentProjection.MinY();
  min[2] = currentProjection.MinZ();

  max[0] = currentProjection.MaxX();
  max[1] = currentProjection.MaxY();
  max[2] = currentProjection.MaxZ();

  for (int i = 0; i < 3; i++) {
    if ((max[i] - min[i]) < 0.00001f)
      max[i] = min[i] + 0.00001f;
  }

  _target << "box " << min[0] << " " << min[1] << " " << min[2] << " " << max[0]
          << " " << max[1] << " " << max[2] << "\n";
}

void RayshadeTurtle::PrintSurfaceUse() const {
  _target << "applysurf s" << std::setfill('0') << std::setw(3) << _color
          << "\n";
  _target << std::setfill(' ') << std::setw(0);
}


void RayshadeTurtle::Mesh(int meshId, float sx, float sy, float sz) const
{
  // check if mesh exists
  if (!meshes.ValidMeshId(meshId))
  {
    Utils::Message("Mesh: Invalid id == %d. Module ignored.\n", meshId);
    return;
  }

  const float rot[16] = 
  {
    -_left.X(),		-_left.Y(),		-_left.Z(),		0.0f,
    _heading.X(),	_heading.Y(),	_heading.Z(),	0.0f,
    _up.X(),		_up.Y(),		_up.Z(),		0.0f,
    0.0f,			0.0f,			0.0f,			1.0f
  };

  OpenGLMatrix mtrx;
  mtrx.Translate(_position);
  mtrx.Multiply(rot);
  mtrx.Scale(sx, sy, sz);

  OpenGLMatrix nrmx(rot);
  // MC - Dec. 2020 - added check if texture id is set by Turtle
  // If so, its value overrides the one from the view file
  int texId = meshes.MeshTextureId(meshId);
  if (_CurrentTexture != -1)
      texId = _CurrentTexture;

  PrintSurfaceUse();

  meshes.GetMesh(meshId).DrawRayshade(mtrx, nrmx, _target, _color, texId);
}
/*
void RayshadeTurtle::AnimatedMesh(int meshId, float scale, float t) const
{
	// check if mesh exists
	if (!meshes.ValidAnimatedMeshId(meshId))
	{
	  Utils::Message("Animated Mesh: Invalid id == %d. Module ignored.\n", meshId);
	  return;
	}
	
  const float rot[16] = 
  {
    -_left.X(),		-_left.Y(),		-_left.Z(),		0.0f,
    _heading.X(),	_heading.Y(),	_heading.Z(),	0.0f,
    _up.X(),		_up.Y(),		_up.Z(),		0.0f,
    0.0f,			0.0f,			0.0f,			1.0f
  };

  OpenGLMatrix mtrx;
  mtrx.Translate(_position);
  mtrx.Multiply(rot);
  mtrx.Scale(scale, scale, scale);

  OpenGLMatrix nrmx(rot);
  // MC - Dec. 2020 - added check if texture id is set by Turtle
  // If so, its value overrides the one from the view file
  int texId = meshes.AnimatedMeshTextureId(meshId);
  if (_CurrentTexture != -1)
      texId = _CurrentTexture;
  meshes.GetAnimatedMesh(meshId).DrawRayshade(t, mtrx, nrmx, _target, _color, texId);

}
*/
