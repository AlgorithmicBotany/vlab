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
#include "configfile.h"
#include "drawparam.h"
#include "volume.h"
#include "glenv.h"
#include "surface.h"
#include "surfarr.h"
#include "BsurfaceObj.h"
#include "bsurfarr.h"
#include "contourarr.h"

PostscriptTurtle::PostscriptTurtle(std::ostream &trg, PsOutputStore &st,
                                   Volume vv,
                                   const Projection &currentProjection,
                                   DParams::ProjectionMode )
    : _store(st), _trg(trg), _polygonStarted(false),_polygonSuspended(false), 
       _polygonPoints(0), _CurrentContour(0), _ContourId2(0),
      _blender(0.0f), _allowBranchGC(false) {
  const float PageWidth = 8.0f;
  const float PageHeight = 10.0f;
  const int DPI = 72;
  vv.Scale(1.1f);

  Vector3d maxpoint(PageWidth * DPI, PageHeight * DPI, 0.0f);
  // the picture will fit into a page 8" wide and 10" high

  float w = currentProjection.MaxX() - currentProjection.MinX();
  float h = currentProjection.MaxY() - currentProjection.MinY();

  if (w < epsilon) {
    // so first handle extreme cases: null vv
    if (h < epsilon)
      _scale = PageHeight * DPI / epsilon;
    // zero width
    else
      _scale = PageHeight * DPI / h;
  }
  // zero height
  else if (h < epsilon)
    _scale = PageWidth * DPI / w;
  // and well behaved
  else if (h / w > PageHeight / PageWidth) {
    _scale = PageHeight * DPI / h;
    maxpoint.Set(PageHeight * DPI * w / h, PageHeight * DPI, 0.0f);
  } else {
    _scale = PageWidth * DPI / w;
    maxpoint.Set(PageWidth * DPI, PageWidth * DPI * h / w, 0.0f);
  }
  _translate = Vector3d(currentProjection.MinX(), currentProjection.MinY(),
                        currentProjection.MinZ());

  // get the current view matrix and transform the light "view"
  GLfloat gl_modelview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview);
  // reset translation
  for (int i = 0; i < 16; ++i)
    _rotation[i] = gl_modelview[i];

  _rotation[12] = 0;
  _rotation[13] = 0;
  _rotation[14] = 0;

  /* Calculate size of window (wx, wy), translation of L-system object
(tx, ty), and factor to scale L-system object (scale) */
  float sx, sy, tmp;
  float scale = _scale;
  int x1, x2, y1, y2;
  float wx, wy; /* size of the drawing area */
  sx = w;
  sy = h;

  scale = 576.0 / sx;
  tmp = 720.0 / sy;
  if (tmp >= scale) {
    wx = 576;
    wy = sy * scale;
  } else {
    scale = tmp;
    wy = 720;
    wx = sx * scale;
  }

  /* Any inaccuracies in the bounding box should be <=1 and are due to the
  assumption that the page size is 468x648pts and roundoff error. */
  x1 = (int)floor((576.0 - wx) / 2 + 18);
  x2 = (int)floor((float)x1 + wx);
  y1 = (int)floor((720.0 - wy) / 2 + 18);
  y2 = (int)floor((float)y1 + wy);
  _trg << "%%BoundingBox: " << x1 << " " << y1 << " "
       << " " << x2 << " " << y2 << std::endl;

  Vector3d translationVect(x1, y1, 0);
  _translateClipping = translationVect;

  _trg << drawparams.PostscriptLinecap() << " setlinecap" << std::endl;

  _trg << "newpath " << std::endl;
  _trg << (float)x1 << " " << (float)y1 << " moveto" << std::endl;
  _trg << 0 << " " << wy << " rlineto" << std::endl;
  _trg << wx << " " << 0 << " rlineto" << std::endl;
  _trg << 0 << " " << -wy << " rlineto" << std::endl;
  _trg << "closepath clip" << std::endl;

  Vector3d clr = gl.GetBgColor();

  _trg << clr.X() << ' ' << clr.Y() << ' ' << clr.Z() << " setrgbcolor"
       << std::endl;

  _trg << "fill" << std::endl;
}

void PostscriptTurtle::operator=(const PostscriptTurtle &src) {
  Turtle::operator=(src);
  _CurrentContour = src._CurrentContour;
  _ContourId2 = src._ContourId2;
  _blender = src._blender;
  _gc = src._gc;
  _CurrentTexture = src._CurrentTexture;
  _textureV = src._textureV;
  _textureVCoeff = src._textureVCoeff;
}

/// Called after interpretation, this function sorts all of the objects in the
///   storage by their depth, and then prints them from back to front so that
///   closer objects specifically print in front.
void PostscriptTurtle::Finish() {
  if (drawparams.ZBuffer()) {
    _store.Sort();
  }
  _store.PrintToStream(_trg);
  _store.Reset();
}

void PostscriptTurtle::F(float v) {
  if (_gc.On()) {
    _GCF(v);
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    _NormalF(v);
}
void PostscriptTurtle::_NormalF(float v) {
  _store.AddNew();
  _store << "newpath\n";
  Vector3d pt = _TransformPoint(_position);
  float z1 = pt.Z();
  _store << pt.X() << " " << pt.Y() << " moveto\n";
  Turtle::F(v);
  pt = _TransformPoint(_position);
  _store.SetDepth((pt.Z() + z1) / 2.0f);
  _store << pt.X() << " " << pt.Y() << " lineto\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  if (drawparams.LineStyle() == DParams::lsPolygon)
    _store << _TransformSize(Width()) << " setlinewidth\n";
  else if (drawparams.LineStyle() == DParams::lsCylinder)
    _store << _TransformSize(Width()) << " setlinewidth\n";
  else
    _store << Width() << " setlinewidth\n";
  _store << "stroke\n";
}
void PostscriptTurtle::_GCF(float v) {
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
  vertex1 = _TransformPoint(vertex1);
  Vector3d vertex2;
  vertex2.Transform(endCenter.Vertex(0), end.Buf());
  vertex2 = _TransformPoint(vertex2);

  for (size_t i = 1; i < beginCenter.Divisions(); i++) {
    /*
     *  Each strip is made from a quad.
     *
     *       end side
     *
     *      v2----v4--...
     *       |    |
     * i - 1 |    | i
     * side  |    | side
     *       |    |
     *      v1----v3--...
     *
     *       begin side
     */
    Vector3d vertex3;
    vertex3.Transform(beginCenter.Vertex(i), begin.Buf());
    vertex3 = _TransformPoint(vertex3);
    Vector3d vertex4;
    vertex4.Transform(endCenter.Vertex(i), end.Buf());
    vertex4 = _TransformPoint(vertex4);

    _store.AddNew();
    _store.SetDepth((0.5f * (vertex1 + vertex4)).Z());
    _store << "newpath"
           << "\n";
    _store << vertex1.X() << " " << vertex1.Y() << " moveto"
           << "\n";
    _store << vertex2.X() << " " << vertex2.Y() << " lineto"
           << "\n";
    _store << vertex4.X() << " " << vertex4.Y() << " lineto"
           << "\n";
    _store << vertex3.X() << " " << vertex3.Y() << " lineto"
           << "\n";

    Vector3d clr = gl.GetSolidColor(_color);
    _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor"
           << "\n";

    _store << "1.0 setlinewidth fill"
           << "\n";

    vertex1 = vertex3;
    vertex2 = vertex4;
  }
}

void PostscriptTurtle::_CapGC() {
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

        v1 = _TransformPoint(v1);
        v2 = _TransformPoint(v2);
        v3 = _TransformPoint(v3);

        _store.AddNew();
        _store.SetDepth((v1 + v2 + v3).Z()/3.f);
        _store << "newpath"
               << "\n";
        _store << v1.X() << " " << v1.Y() << " moveto"
               << "\n";
        _store << v2.X() << " " << v2.Y() << " lineto"
               << "\n";
        _store << v3.X() << " " << v3.Y() << " lineto"
               << "\n";

        Vector3d clr = gl.GetSolidColor(_color);
        _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor"
               << "\n";

        _store << "1.0 setlinewidth fill"
               << "\n";
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

        centroid = _TransformPoint(centroid);

        Vector3d v1, v2;
        for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
          v1.Transform(bgncntr.Vertex(i), bgn.Buf());
          v2.Transform(bgncntr.Vertex(i+1), bgn.Buf());

          v1 = _TransformPoint(v1);
          v2 = _TransformPoint(v2);

          _store.AddNew();
          _store.SetDepth((centroid + v1 + v2).Z()/3.f);
          _store << "newpath"
                 << "\n";
          _store << centroid.X() << " " << centroid.Y() << " moveto"
                 << "\n";
          _store << v1.X() << " " << v1.Y() << " lineto"
                 << "\n";
          _store << v2.X() << " " << v2.Y() << " lineto"
                 << "\n";

          Vector3d clr = gl.GetSolidColor(_color);
          _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor"
                 << "\n";

          _store << "1.0 setlinewidth fill"
                 << "\n";
        }
    }
}

void PostscriptTurtle::StartGC() {
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
void PostscriptTurtle::PointGC() {
  if (_gc.On()) {
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    Utils::Message(
        "PointGC called when there is no active generalized cylinder.\n");
}
void PostscriptTurtle::EndGC() {
  if (_gc.On()) {
    if (drawparams.CappedCylinders() == DParams::cappedCylindersOn)
      _CapGC();
    _gc.End();
  } else
    Utils::Message(
        "EndGC called when there is no active generalized cylinder.\n");
}

void PostscriptTurtle::StartBranch() {
  if (_gc.On() && _allowBranchGC == false) {
    _allowBranchGC = true;
  }
}

void PostscriptTurtle::CurrentContour(int id) {
  if (id == 0 || contours.ValidId(id))
    _CurrentContour = id;
  else
    Utils::Message("Invalid contour id = %d\n", id);
}
void PostscriptTurtle::BlendContours(int id1, int id2, float b) {
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
void PostscriptTurtle::SetWidth(float w) { ScaleContour(w, w); }
void PostscriptTurtle::ScaleContour(float p, float q) {
  Turtle::ScaleContour(p, q);
  _Scale.p = p;
  _Scale.q = q;
}
void PostscriptTurtle::ContourSides(int cs) {
  if (_gc.On())
    Utils::Message(
        "ContourSides: Called between StartGC and EndGC, ignored.\n");
  else {
    Turtle::ContourSides(cs);
  }
}
void PostscriptTurtle::ContourNormal(V3f n) {
  Turtle::ContourNormal(n);
}

void PostscriptTurtle::Circle(float r) const {
  _store.AddNew();
  /* TODO - when using projection, rotate to ellipse? */
  _store << "newpath\n";
  Vector3d pt = _TransformPoint(_position);
  _store.SetDepth(pt.Z());
  r = _TransformSize(r);
  _store << pt.X() << " " << pt.Y() << " " << r << " 0 360 arc\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << "fill\n";
}
void PostscriptTurtle::CircleB(float r) const {
  _store.AddNew();
  /* TODO - when using projection, rotate to ellipse? */
  _store << "newpath\n";
  Vector3d pt = _TransformPoint(_position);
  _store.SetDepth(pt.Z());
  r = _TransformSize(r);
  _store << pt.X() << " " << pt.Y() << " " << r << " 0 360 arc\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << _TransformSize(Width()) << " setlinewidth\n";
  _store << "stroke\n";
}

void PostscriptTurtle::CircleFront(float r) const {
  _store.AddNew();
  _store << "newpath\n";
  Vector3d pt = _TransformPoint(_position);
  _store.SetDepth(pt.Z());
  r = _TransformSize(r);
  _store << pt.X() << " " << pt.Y() << " " << r << " 0 360 arc\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << "fill\n";
}
void PostscriptTurtle::CircleFrontB(float r) const {
  _store.AddNew();
  _store << "newpath\n";
  Vector3d pt = _TransformPoint(_position);
  _store.SetDepth(pt.Z());
  r = _TransformSize(r);
  _store << pt.X() << " " << pt.Y() << " " << r << " 0 360 arc\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << _TransformSize(Width()) << " setlinewidth\n";
  _store << "stroke\n";
}

void PostscriptTurtle::Sphere(float r) const {
  _store.AddNew();
  _store << "newpath\n";
  Vector3d pt = _TransformPoint(_position);
  r = _TransformSize(r);
  _store.SetDepth(pt.Z() + r);
  _store << pt.X() << " " << pt.Y() << " " << r << " 0 360 arc\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << "fill\n";
}

void PostscriptTurtle::StartPolygon() {
  if (_polygonStarted)
    Utils::Message("SP: previous polygon not ended. Resetting polygon\n");
  _polygonStarted = true;
  _polygonPoints.clear();
}

void PostscriptTurtle::PolygonPoint() {
  if (!_polygonStarted)
    Utils::Message("PP: No polygon started. Module ignored.\n");
  else if (!_polygonSuspended)
    _polygonPoints.push_back(_TransformPoint(_position));
}

void PostscriptTurtle::EndPolygon() {
  if (!_polygonStarted)
    Utils::Message("EP: No polygon started. Module ignored.\n");
  else if (_polygonPoints.size() < 3)
    Utils::Message("EP: Polygon of fewer than three points not rendered.\n");
  else {
    _store.AddNew();
    //  This is not perfect, but we find the depth of the polygon by taking
    //    the average depth of every point in the polygon.
    float z = _TransformPoint(_polygonPoints[0]).Z();
    _store << "newpath\n";
    _store << _polygonPoints[0].X() << " " << _polygonPoints[0].Y()
           << " moveto\n";
    for (unsigned int i = 1; i < _polygonPoints.size(); i++) {
      _store << _polygonPoints[i].X() << " " << _polygonPoints[i].Y()
             << " lineto\n";
      z += _TransformPoint(_polygonPoints[i]).Z();
    }
    z /= _polygonPoints.size();
    _store.SetDepth(z);
    _store << "closepath\n";
    Vector3d clr = gl.GetSolidColor(_color);
    _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
    _store << "fill\n";
  }

  _polygonStarted = false;
  _polygonPoints.clear();
}

void PostscriptTurtle::Rhombus(float a, float b) const {
  _store.AddNew();
  _store << "newpath\n";
  Vector3d v = _position, vtx = _TransformPoint(v);
  _store.SetDepth(vtx.Z());
  _store << vtx.X() << " " << vtx.Y() << " moveto\n";

  v += _heading * a * 0.5f;
  v += _left * b * 0.5f;
  vtx = _TransformPoint(v);
  _store << vtx.X() << " " << vtx.Y() << " lineto\n";

  v = _position + _heading * a;
  vtx = _TransformPoint(v);
  _store << vtx.X() << " " << vtx.Y() << " lineto\n";

  v = _position + _heading * a * 0.5f;
  v -= _left * b * 0.5f;
  vtx = _TransformPoint(v);
  _store << vtx.X() << " " << vtx.Y() << " lineto\n";

  _store << "closepath"
         << "\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << "fill\n";
}

void PostscriptTurtle::Triangle(float a, float b) const {
  _store.AddNew();
  _store << "newpath\n";
  Vector3d vtx = _TransformPoint(_position + _left * a * 0.5f);
  _store.SetDepth(_TransformPoint(_position).Z());
  _store << vtx.X() << " " << vtx.Y() << " moveto\n";

  vtx = _TransformPoint(_position + _heading * b);
  _store << vtx.X() << " " << vtx.Y() << " lineto\n";

  vtx = _TransformPoint(_position - _left * a * 0.5f);
  _store << vtx.X() << " " << vtx.Y() << " lineto\n";

  _store << "closepath\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << "fill\n";
}

void PostscriptTurtle::Orient() const {
  _store.AddNew();
  _store << "fill\n";

  Vector3d v0 = _TransformPoint(_position),
           v1 = _TransformPoint(_position + _heading);
  _store.SetDepth(v0.Z());
  _store << "newpath\n";
  _store << v0.X() << " " << v0.Y() << " moveto\n";
  _store << v1.X() << " " << v1.Y() << " lineto\n";
  _store << "1.0 0.0 0.0 setrgbcolor\n";
  _store << "1.0 setlinewidth stroke\n";

  v1 = _TransformPoint(_position + _left);
  _store << "newpath\n";
  _store << v0.X() << " " << v0.Y() << " moveto\n";
  _store << v1.X() << " " << v1.Y() << " lineto\n";
  _store << "0.0 1.0 0.0 setrgbcolor\n";
  _store << "1.0 setlinewidth stroke\n";

  v1 = _TransformPoint(_position + _up);
  _store << "newpath\n";
  _store << v0.X() << " " << v0.Y() << " moveto\n";
  _store << v1.X() << " " << v1.Y() << " lineto\n";
  _store << "0.0 0.0 1.0 setrgbcolor\n";
  _store << "1.0 setlinewidth stroke\n";
}

void PostscriptTurtle::Label(const char *str) const {
  DrawParams::Font font = drawparams.GetFont();
  std::string font_name = font.Name();
  /* first letter should be capital */
  font_name[0] = toupper(font_name[0]);

#ifdef __APPLE__
  if (font_name.compare("Arial") == 0)
    font_name = "Helvetica";
#endif

  int font_size = font.Size();
  if ((font_size <= 0) || (font_size > 10000)) {
    Utils::Message("Warning font size is not valid, using default size\n");
    font_size = 20;
  }

  std::string weight = font.Bold();
  weight[0] = toupper(weight[0]);

  if (weight.compare("Medium") == 0)
    weight = "Bold";

  font_name = "/" + font_name + "-" + weight;
  if (font.Italic())
    font_name += "Italic";
  _store.AddNew();
  _store << font_name << " findfont\n";
  _store << font_size << " scalefont\n";
  _store << "setfont\n";
  Vector3d pt = _TransformPoint(_position);
  _store.SetDepth(pt.Z());
  _store << pt.X() << " " << pt.Y();
  _store << " moveto\n";
  Vector3d clr = gl.GetSolidColor(_color);
  _store << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor\n";
  _store << "(" << str << ") show\n";
}

void PostscriptTurtle::PrintSurface(::Surface s, float sx, float sy,
                                    float sz) const {
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

  for (int i = 0; s.IsValidPatchId(i); i++) {
    typedef std::vector<Vector3d> vectors;
    typedef std::vector<vectors> vectors2;
    vectors2 points;
    vectors2 normals;
    vectors2 textures;
    s.GetPatchGeometry(i, points, normals, textures);

    for (size_t j = 0; j < points.size() - 1; j++)
      for (size_t k = 0; k < points[j].size() - 1; k++) {
        Vector3d point0 = points[j][k];
        point0.Transform(transformMatrix);
        point0 = _TransformPoint(point0);
        Vector3d point1 = points[j + 1][k];
        point1.Transform(transformMatrix);
        point1 = _TransformPoint(point1);
        Vector3d point2 = points[j][k + 1];
        point2.Transform(transformMatrix);
        point2 = _TransformPoint(point2);
        Vector3d point3 = points[j + 1][k + 1];
        point3.Transform(transformMatrix);
        point3 = _TransformPoint(point3);
        Vector3d clr = gl.GetSolidColor(_color);

        _store.AddNew();
        _store.SetDepth((0.5f * (point0 + point3)).Z());
        _store << "newpath"
               << "\n";
        _store << point0.X() << " " << point0.Y() << " moveto"
               << "\n";
        _store << point1.X() << " " << point1.Y() << " lineto"
               << "\n";
        _store << point3.X() << " " << point3.Y() << " lineto"
               << "\n";
        _store << point2.X() << " " << point2.Y() << " lineto"
               << "\n";

        int xgrad =
            drawparams.PostscriptGradient() == DParams::gradientLeftToRight ? 1
                                                                            : 0;
        int ygrad =
            drawparams.PostscriptGradient() == DParams::gradientBottomToTop ? 1
                                                                            : 0;
        //  Add a very slight gradient to the surface. Not necessary,
        //    but makes the surface significantly more distinguishable.
        Vector3d color;
        if (xgrad == 0 && ygrad == 0)
          color = clr;
        else
          color = clr * (1 + drawparams.PostscriptGradientAmount() *
                                 ((float)(xgrad * j + ygrad * k) /
                                  (float)(xgrad * points.size() +
                                          ygrad * points[j].size())));

        _store << color.X() << " " << color.Y() << " " << color.Z()
               << " setrgbcolor"
               << "\n";
        _store << "1.0 setlinewidth fill"
               << "\n";
      }
  }
}

void PostscriptTurtle::Surface(int id, float sx, float sy, float sz) const {
  if (sx > epsilon && sy > epsilon && sz > epsilon) {
    ::Surface s = surfaces.Get(id);
    PrintSurface(s, sx, sy, sz);
  }
}
void PostscriptTurtle::DSurface(SurfaceObj surf, bool ) const {
  const class Surface &s(surf);
  PrintSurface(s, 1, 1, 1);
}
void PostscriptTurtle::BSurface(int id, float sx, float sy, float sz) const {
  if (!bsurfaces.ValidId(id))
    return;

  const float rotation[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                              _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                              _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                              0.0f,         0.0f,         0.0f,         1.0f};

  OpenGLMatrix transformMatrix;
  transformMatrix.Translate(_position);
  transformMatrix.Multiply(rotation);

  OpenGLMatrix translateMatrix, scaleMatrix;
  translateMatrix.Translate(-_translate);
  scaleMatrix.Scale(sx * _scale, sy * _scale, sz * _scale);

  transformMatrix.Multiply(translateMatrix.Buf());
  transformMatrix.Multiply(scaleMatrix.Buf());

  b_wrapper &s(bsurfaces.Get(id));
  s.Transform(transformMatrix);

  Vector3d clr = gl.GetSolidColor(_color);

  //  Not sure what the translation should actually be.
  // std::string bstring = "270 310 translate\n";
  std::string bstring = "";
  s.PostScript(GetUVPrecision(), bstring, clr);
  _store.AddNew();
  //  Shouldn't be 0, but currently the only model using a bsurface only
  //    uses a bsurface and nothing else. Not sure what the depth should be.
  _store.SetDepth(0);
  _store << bstring;
}

/** Other drawables that aren't going into the Postscript output **/
// void PostscriptTurtle::DBSurfaceS(BsurfaceObjS s) const
// void PostscriptTurtle::DBSurfaceM(BsurfaceObjM s) const
// void PostscriptTurtle::Terrain(CameraPosition camPos)

Vector3d PostscriptTurtle::_TransformPoint(Vector3d v) const {
  /* TODO - use current projection, not XY
   */

  v.Transform(_rotation);

  v -= _translate;
  v *= _scale;
  v += _translateClipping;

  return v;
}

float PostscriptTurtle::_TransformSize(float v) const { return v * _scale; }
