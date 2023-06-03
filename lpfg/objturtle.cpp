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
#include "contourarr.h"
#include "texturearr.h"
#include "surfarr.h"
#include "BsurfaceObj.h"
#include "bsurfarr.h"
#include "objout.h"
#include "utils.h"
#include "mesharr.h" // MC - Dec. 2020 - for drawing meshes

ObjTurtle::ObjTurtle(ObjOutputStore &trg)
    : _CurrentContour(0), _ContourId2(0), _blender(0.0f),
      _allowBranchGC(false), _trg(trg),
      _PolygonStarted(false), _divisions(divUnspecified) {
  _textureV = 0.0f;
  _textureVCoeff = 1.0f;
}

void ObjTurtle::operator=(const ObjTurtle &src) {
  Turtle::operator=(src);
  _CurrentContour = src._CurrentContour;
  _ContourId2 = src._ContourId2;
  _blender = src._blender;
  _gc = src._gc;
  _CurrentTexture = src._CurrentTexture;
  _textureV = src._textureV;
  _textureVCoeff = src._textureVCoeff;
}

void ObjTurtle::Sphere(float radius) const {
  int SphereSlices = _divisions == divUnspecified ? drawparams.ContourDivisions() : _divisions;
  int SphereStacks = (SphereSlices + 1) / 2;
  _trg.NewGroup();
  // top
  {
    const PolarPoint topp(0.0f, 0.5f * M_PI);
    Vector3d top(topp);
    const size_t tonx = _trg.Normal(top);
    //  Not sure how to map textures to spheres, so following:
    //    http://www.mvps.org/directx/articles/spheremap.htm
    //    which outlines how to map to a sphere using the normal, but in
    //    DirectX.
    Vector3d texco(top);
    texco += Vector3d(M_PI, M_PI, 0);
    texco *= (1 / 2 / M_PI);
    top *= radius;
    top += _position;
    const size_t topx = _trg.Vertex(top, texco);
    float lat = 0.5f * M_PI - M_PI / SphereStacks;

    PolarPoint pp(-M_PI, lat);
    Vector3d pv(pp);
    size_t pnx = _trg.Normal(pv);
    Vector3d texco2(pv);
    texco2 += Vector3d(M_PI, M_PI, 0);
    texco2 *= (1 / 2 / M_PI);
    pv *= radius;
    pv += _position;
    size_t pvx = _trg.Vertex(pv, texco2);
    const size_t fvx = pvx;
    const size_t fnx = pnx;

    for (float lngtd = -M_PI + 2.0f * M_PI / SphereSlices; lngtd < M_PI;
         lngtd += 2.0f * M_PI / SphereSlices) {
      PolarPoint p(lngtd, lat);
      Vector3d v(p);
      size_t nx = _trg.Normal(v);
      Vector3d texco3(v);
      texco3 += Vector3d(M_PI, M_PI, 0);
      texco3 *= (1 / 2 / M_PI);
      v *= radius;
      v += _position;
      size_t vx = _trg.Vertex(v, texco3);
      _trg.Triangle(topx, tonx, pvx, pnx, vx, nx, _color, _CurrentTexture);
      pvx = vx;
      pnx = nx;
    }
    _trg.Triangle(topx, tonx, pvx, pnx, fvx, fnx, _color, _CurrentTexture);
  }

  // Middle part
  for (float lat = 0.5f * M_PI - M_PI / SphereStacks;
       lat > -0.5f * M_PI + M_PI / SphereStacks; lat -= M_PI / SphereStacks) {
    for (float lngtd = -M_PI + 2.0f * M_PI / SphereSlices;
         lngtd < M_PI + 0.001f; lngtd += 2.0f * M_PI / SphereSlices) {
      PolarPoint p1(lngtd, lat);
      PolarPoint p2(lngtd, lat - M_PI / SphereStacks);
      PolarPoint p3(lngtd + 2.0f * M_PI / SphereSlices, lat);
      PolarPoint p4(lngtd + 2.0f * M_PI / SphereSlices,
                    lat - M_PI / SphereStacks);
      Vector3d v1(p1);
      Vector3d v2(p2);
      Vector3d v3(p3);
      Vector3d v4(p4);
      Vector3d vt1(p1);
      Vector3d vt2(p2);
      Vector3d vt3(p3);
      Vector3d vt4(p4);
      size_t n1 = _trg.Normal(v1);
      size_t n2 = _trg.Normal(v2);
      size_t n3 = _trg.Normal(v3);
      size_t n4 = _trg.Normal(v4);
      vt1 += Vector3d(M_PI, M_PI, 0);
      vt1 *= (1 / 2 / M_PI);
      v1 *= radius;
      v1 += _position;
      size_t x1 = _trg.Vertex(v1, vt1);
      vt2 += Vector3d(M_PI, M_PI, 0);
      vt2 *= (1 / 2 / M_PI);
      v2 *= radius;
      v2 += _position;
      size_t x2 = _trg.Vertex(v2, vt2);
      vt3 += Vector3d(M_PI, M_PI, 0);
      vt3 *= (1 / 2 / M_PI);
      v3 *= radius;
      v3 += _position;
      size_t x3 = _trg.Vertex(v3, vt3);
      vt4 += Vector3d(M_PI, M_PI, 0);
      vt4 *= (1 / 2 / M_PI);
      v4 *= radius;
      v4 += _position;
      size_t x4 = _trg.Vertex(v4, vt4);
      _trg.Quad(x1, n1, x2, n2, x4, n4, x3, n3, _color, _CurrentTexture);
    }
  }

  // Bottom
  {
    const PolarPoint btmp(0.0f, -0.5f * M_PI);
    Vector3d btm(btmp);
    const size_t btnx = _trg.Normal(btm);
    Vector3d texco(btm);
    texco += Vector3d(M_PI, M_PI, 0);
    texco *= (1 / 2 / M_PI);
    btm *= radius;
    btm += _position;
    const size_t btmx = _trg.Vertex(btm, texco);
    float lat = -0.5f * M_PI + M_PI / SphereStacks;

    PolarPoint pp(-M_PI, lat);
    Vector3d pv(pp);
    size_t pnx = _trg.Normal(pv);
    Vector3d texco2(pv);
    texco2 += Vector3d(M_PI, M_PI, 0);
    texco2 *= (1 / 2 / M_PI);
    pv *= radius;
    pv += _position;
    size_t pvx = _trg.Vertex(pv, texco2);
    const size_t fvx = pvx;
    const size_t fnx = pnx;

    for (float lngtd = -M_PI + 2.0f * M_PI / SphereSlices; lngtd < M_PI;
         lngtd += 2.0f * M_PI / SphereSlices) {
      PolarPoint p(lngtd, lat);
      Vector3d v(p);
      size_t nx = _trg.Normal(v);
      Vector3d texco3(v);
      texco3 += Vector3d(M_PI, M_PI, 0);
      texco3 *= (1 / 2 / M_PI);
      v *= radius;
      v += _position;
      size_t vx = _trg.Vertex(v, texco3);
      _trg.Triangle(pvx, pnx, btmx, btnx, vx, nx, _color, _CurrentTexture);
      pvx = vx;
      pnx = nx;
    }
    _trg.Triangle(pvx, pnx, btmx, btnx, fvx, fnx, _color, _CurrentTexture);
  }
}

/// Print "usemtl s###" whenever the material in use changes in the model.
void ObjTurtle::IncColor() {
  Turtle::IncColor();
  _trg.PrintMaterialUse(_color, _CurrentTexture);
}
void ObjTurtle::DecColor() {
  Turtle::DecColor();
  _trg.PrintMaterialUse(_color, _CurrentTexture);
}
void ObjTurtle::SetColor(int c) {
  Turtle::SetColor(c);
  _trg.PrintMaterialUse(_color, _CurrentTexture);
}

void ObjTurtle::F(float v) {
  if (_gc.On()) {
    _GCF(v);
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    _NormalF(v);
}

void ObjTurtle::_NormalF(float v) {
  const Vector3d oldpos = _position;
  const float step = M_PI * 2.0f / LPFGParams::DefaultContourDivisions / 2.0f;

  Turtle::F(v);

  if (!_PolygonStarted) {
    _trg.NewGroup();
    const float rot[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                           _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                           _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                           0.0f,         0.0f,         0.0f,         1.0f};
    OpenGLMatrix rotationMatrix;
    rotationMatrix.Multiply(rot);

    for (float alpha = 0.0f; alpha <= M_PI * 2.0f + epsilon; alpha += step) {
      Vector3d n1(cosf(alpha), 0.0f, sinf(alpha));
      n1.Transform(rotationMatrix);
      Vector3d n2(cosf(alpha + step), 0.0f, sinf(alpha + step));
      n2.Transform(rotationMatrix);

      size_t nx1 = _trg.Normal(n1);
      size_t nx2 = _trg.Normal(n2);

      Vector3d p1 = n1 * (Width() / 2.0f) + oldpos;
      Vector3d p2 = n2 * (Width() / 2.0f) + oldpos;
      Vector3d p3 = n1 * (Width() / 2.0f) + _position;
      Vector3d p4 = n2 * (Width() / 2.0f) + _position;

      size_t vx1 = _trg.Vertex(p1, Vector3d(0, 0, 0));
      size_t vx2 = _trg.Vertex(p2, Vector3d(1, 0, 0));
      size_t vx3 = _trg.Vertex(p3, Vector3d(0, 1, 0));
      size_t vx4 = _trg.Vertex(p4, Vector3d(1, 1, 0));

      _trg.Quad(vx1, nx1, vx2, nx2, vx4, nx2, vx3, nx1, _color,
                _CurrentTexture);
    }
  }
}

void ObjTurtle::_GCF(float v) {
  ASSERT(_gc.On());

  // I think it is better to make the entire GC a group,
  // instead of each segement, so move NewGroup() to StartGC
  // _trg.NewGroup();

  Turtle::F(v);

  OpenGLMatrix bgn;
  OpenGLMatrix end;
  {
    bgn.Translate(_gc.Position().X(), _gc.Position().Y(), _gc.Position().Z());
    {
      const float rot[16] = {_gc.Left().X(),
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
      bgn.Multiply(rot);
    }
    bgn.Scale(0.5f * _gc.ScaleP(), 0.5f * _gc.ScaleQ(), 1.0f);
    end.Translate(_position.X(), _position.Y(), _position.Z());
    {
      const float rot[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                             _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                             _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                             0.0f,         0.0f,         0.0f,         1.0f};
      end.Multiply(rot);
    }
    end.Scale(0.5f * _ScaleP(), 0.5f * _ScaleQ(), 1.0f);
  }

  const float bgnnrm[16] = {_gc.Left().X(),
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
  const float endnrm[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                            _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                            _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                            0.0f,         0.0f,         0.0f,         1.0f};

  // MC - March 4, 2020 - support for textures
  float endtv = _textureV + v * _textureVCoeff;
  if (v == 0.f)
    endtv =  _textureV + _textureVCoeff;

  {
    const Contour &bgncntr =
        contours.Get1(_gc.ContourId(), _gc.ContourId2(), _gc.Blender());
    const Contour &endcntr =
        contours.Get2(_CurrentContour, _ContourId2, _blender);
    Vector3d vtx1;
    vtx1.Transform(bgncntr.Vertex(0), bgn.Buf());
    Vector3d nrm1;
    nrm1.Transform(bgncntr.Normal(0), bgnnrm);
    Vector3d vtx2;
    vtx2.Transform(endcntr.Vertex(0), end.Buf());
    Vector3d nrm2;
    nrm2.Transform(endcntr.Normal(0), endnrm);
    // MC - March 4, 2020 - added texture coordinate output for gen. cylinders
    Vector3d tex1(0, 0, 0);
    if (_TextureOn()) {
      tex1.X(0.0f);
      tex1.Y(_textureV);
    }
    Vector3d tex2(1, 0, 0);
    if (_TextureOn()) {
      tex2.X(0.0f);
      tex2.Y(endtv);
    }
    std::pair<size_t, size_t> vx1 = _trg.VertexTexCoord(vtx1, tex1);
    size_t nx1 = _trg.Normal(nrm1);
    std::pair<size_t, size_t> vx2 = _trg.VertexTexCoord(vtx2, tex2);
    size_t nx2 = _trg.Normal(nrm2);
    for (size_t i = 1; i < bgncntr.Divisions(); ++i) {
      Vector3d vtx3;
      vtx3.Transform(bgncntr.Vertex(i), bgn.Buf());
      Vector3d nrm3;
      nrm3.Transform(bgncntr.Normal(i), bgnnrm);

      Vector3d vtx4;
      vtx4.Transform(endcntr.Vertex(i), end.Buf());
      Vector3d nrm4;
      nrm4.Transform(endcntr.Normal(i), endnrm);

      Vector3d tex3(0, 1, 0);
      if (_TextureOn()) {
        tex3.X(i * 1.0f / (bgncntr.Divisions() - 1));
        tex3.Y(_textureV);
      }

      Vector3d tex4(1, 1, 0);
      if (_TextureOn()) {
        tex4.X(i * 1.0f / (bgncntr.Divisions() - 1));
        tex4.Y(endtv);
      }

      std::pair<size_t, size_t> vx3 = _trg.VertexTexCoord(vtx3, tex3);
      size_t nx3 = _trg.Normal(nrm3);

      std::pair<size_t, size_t> vx4 = _trg.VertexTexCoord(vtx4, tex4);
      size_t nx4 = _trg.Normal(nrm4);

      // old code was outputing quads, but, in lpfg, generalized cylinders
      // sometimes end at a single vertex, so there should be an option
      // to switch between quads and triangles.
#if 0
      _trg.Quad(vx1.first, nx1, vx1.second,
                vx2.first, nx2, vx2.second,
                vx4.first, nx4, vx4.second,
                vx3.first, nx3, vx3.second,
                _color, _CurrentTexture);
#else
      if (vx1 != vx2 && vx1 != vx3 && vx2 != vx3) {
        _trg.Triangle(vx1.first, nx1, vx1.second, vx2.first, nx2, vx2.second,
                      vx3.first, nx3, vx3.second, _color, _CurrentTexture);
      }
      if (vx2 != vx4 && vx2 != vx3 && vx4 != vx3) {
        _trg.Triangle(vx2.first, nx2, vx2.second, vx4.first, nx4, vx4.second,
                      vx3.first, nx3, vx3.second, _color, _CurrentTexture);
      }
#endif
      vx1 = vx3;
      nx1 = nx3;
      vx2 = vx4;
      nx2 = nx4;
    }
  }
  if (_TextureOn()) {
    _textureV = endtv;
    if (_textureV > 1.0f)
      _textureV -= 1.0f;
  }
}

void ObjTurtle::_CapGC(bool endCap) {
    ASSERT(_gc.On());

    // Moved to StartGC
    //_trg.NewGroup();

    OpenGLMatrix bgn;
    bgn.Translate(_gc.Position().X(), _gc.Position().Y(), _gc.Position().Z());
    const float bgnrot[16] = {_gc.Left().X(),_gc.Left().Y(),_gc.Left().Z(),0.f,
                             _gc.Up().X(),_gc.Up().Y(),_gc.Up().Z(),0.f,
                             _gc.Heading().X(),_gc.Heading().Y(),_gc.Heading().Z(),0.f,
                             0.0f,0.0f,0.0f,1.0f};
    bgn.Multiply(bgnrot);
    bgn.Scale(0.5f * _gc.ScaleP(), 0.5f * _gc.ScaleQ(), 1.0f);

    //  No way to automatically and intelligently assign UV coordinates to
    //    arbitrary polygons, so we have to just leave it as 0.
    Vector3d vtex(0,0,0);
  
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

      // use the GLU tesselator to generate the triangles
      GLTesselate::Polygon polygon;
      auto triangleVertices = polygon.tesselate(polygonPoints);
      for (size_t i = 0; i < triangleVertices.size(); i += 3) {
        auto v1 = triangleVertices[i];
        auto v2 = triangleVertices[i+1];
        auto v3 = triangleVertices[i+2];

        auto v1ID = _trg.Vertex(v1, Vector3d(0, 0, 0));
        auto v2ID = _trg.Vertex(v2, Vector3d(0, 0, 0));
        auto v3ID = _trg.Vertex(v3, Vector3d(0, 0, 0));

        if (endCap)
          _trg.Triangle(v1ID,v3ID,v2ID,_color,_CurrentTexture);
        else
          _trg.Triangle(v1ID,v2ID,v3ID,_color,_CurrentTexture);
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
        size_t centroidID = _trg.Vertex(centroid,vtex);

        size_t nID = _trg.Normal(_gc.Heading());

        Vector3d v1, v2;
        for (size_t i = 0; i < bgncntr.Divisions()-1; ++i) {
          v1.Transform(bgncntr.Vertex(i), bgn.Buf());
          size_t v1ID = _trg.Vertex(v1,vtex);
          v2.Transform(bgncntr.Vertex(i+1), bgn.Buf());
          size_t v2ID = _trg.Vertex(v2,vtex);

          if (endCap)
            _trg.Triangle(centroidID, nID, v2ID, nID, v1ID, nID, _color, _CurrentTexture);
          else
            _trg.Triangle(centroidID, nID, v1ID, nID, v2ID, nID, _color, _CurrentTexture);
        }
    }

}

void ObjTurtle::StartGC() {
  _trg.PushGroup();
  _trg.NewGroup();
  if (_allowBranchGC) {
    _gc.End();
    _allowBranchGC = false;
  }
  if (!_gc.On()) {
    _gc.Start();
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
    if (drawparams.CappedCylinders() == DParams::cappedCylindersOn)
        _CapGC(false);
  } else
    Utils::Message("StartGC: cylinder already started. Module ignored.\n");
}

void ObjTurtle::PointGC() {
  if (_gc.On()) {
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    Utils::Message("PointGC: cylinder not started. Module ignored.\n");
}

void ObjTurtle::EndGC() {
  if (_gc.On()) {
    if (drawparams.CappedCylinders() == DParams::cappedCylindersOn)
      _CapGC(true);
    _gc.End();
    _trg.PopGroup();
  }
  else
    Utils::Message("EndGC: cylinder not started. Module ignored.\n");
}

void ObjTurtle::StartBranch() {
  if (_gc.On() && _allowBranchGC == false) {
    _allowBranchGC = true;
  }
}

void ObjTurtle::BlendContours(int id1, int id2, float b) {
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
    Utils::Message("BlendContours: Blending factor %f < 0.0, assumed 0.0\n");
    b = 0.0f;
  } else if (b > 1.0f) {
    Utils::Message("BlendContours: Blending factor %f > 1.0, assumed 1.0\n");
    b = 1.0f;
  }
  _CurrentContour = id1;
  _ContourId2 = id2;
  _blender = b;
}

void ObjTurtle::CurrentContour(int id) {
  if (!contours.ValidId(id)) {
    Utils::Message("CurrentContour: Invalid id = %d. Module ignored.\n", id);
    return;
  }
  _CurrentContour = id;
}

void ObjTurtle::SetWidth(float w) { ScaleContour(w, w); }

void ObjTurtle::ScaleContour(float p, float q) {
  Turtle::ScaleContour(p, q);
  _Scale.p = p;
  _Scale.q = q;
}

void ObjTurtle::CurrentTexture(int id) {
  Turtle::CurrentTexture(id);
  _trg.PrintMaterialUse(_color, _CurrentTexture);
}

void ObjTurtle::Rhombus(float a, float b) const {
  size_t nx = _trg.Normal(_up);
  Vector3d v(_position);
  size_t vx1 = _trg.Vertex(v, Vector3d(0.5, 0, 0));
  v += _heading * a * 0.5f;
  v += _left * b * 0.5f;
  size_t vx2 = _trg.Vertex(v, Vector3d(0, 0.5, 0));
  v = _position + _heading * a;
  size_t vx3 = _trg.Vertex(v, Vector3d(0.5, 1, 0));
  v = _position + _heading * a * 0.5f;
  v -= _left * b * 0.5f;
  size_t vx4 = _trg.Vertex(v, Vector3d(1, 0.5, 0));

  _trg.Quad(vx1, nx, vx2, nx, vx3, nx, vx4, nx, _color, _CurrentTexture);
}

void ObjTurtle::Triangle(float a, float b) const {
  size_t nx = _trg.Normal(_up);
  Vector3d v(_position);
  v += _left * a * 0.5f;
  size_t vx1 = _trg.Vertex(v, Vector3d(0, 0, 0));
  v = _position + _heading * b;
  size_t vx2 = _trg.Vertex(v, Vector3d(0.5, 1, 0));
  v = _position - _left * a * 0.5f;
  size_t vx3 = _trg.Vertex(v, Vector3d(1, 0, 0));

  _trg.Triangle(vx1, nx, vx2, nx, vx3, nx, _color, _CurrentTexture);
}

void ObjTurtle::Surface(int id, float sx, float sy, float sz) const {
  if (!surfaces.ValidId(id)) {
    Utils::Message("Surface: Invalid id == %d. Module ignored.\n", id);
    return;
  }
  OpenGLMatrix mtrx;
  mtrx.Translate(_position);

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  mtrx.Multiply(rot);

  mtrx.Scale(sx, sy, sz);
  mtrx.Scale(surfaces.Get(id).Scale(), surfaces.Get(id).Scale(),
             surfaces.Get(id).Scale());
  OpenGLMatrix nrmx(rot);
  // MC - Dec. 2020 - added check if texture id is set by Turtle
  // If so, its value overrides the one from the view file
  int texId = surfaces.TextureId(id);
  if (_TextureOn())
      texId = _CurrentTexture;
  surfaces.Get(id).DrawObj(mtrx, nrmx, _trg, _color, texId);
}

void ObjTurtle::DSurface(SurfaceObj surf, bool ) const {
  const class Surface &s(surf);

  OpenGLMatrix transformMatrix;
  transformMatrix.Translate(_position);

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  transformMatrix.Multiply(rot);
  OpenGLMatrix nrmx(rot);
  s.DrawObj(transformMatrix, nrmx, _trg, _color, _CurrentTexture);
}

void ObjTurtle::BSurface(int id, float sx, float sy, float sz) const {
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

  s.DrawObj(GetUVPrecision(), _trg, _color, _CurrentTexture);
}

void ObjTurtle::DBSurfaceS(BsurfaceObjS) const {}

void ObjTurtle::DBSurfaceM(BsurfaceObjM) const {}

void ObjTurtle::ContourSides(int cs) {
  if (cs < LPFGParams::MinContourDivisions) {
    _divisions = LPFGParams::MinContourDivisions;
  } else if (cs > LPFGParams::MaxContourDivisions - 1) {
    _divisions = LPFGParams::MaxContourDivisions;
  } else
    _divisions = cs;
}

void ObjTurtle::StartPolygon() { _PolygonStarted = true; }
void ObjTurtle::EndPolygon() {
  _PolygonStarted = false;

  if (drawparams.ConcavePolygons() == DParams::concaveOn) {
    // use the GLU tesselator to generate the triangle mesh
    GLTesselate::Polygon polygon;
    auto triangleVertices = polygon.tesselate(polygonPoints);
    for (size_t i = 0; i < triangleVertices.size(); i += 3) {
        auto v1 = triangleVertices[i];
        auto v2 = triangleVertices[i+1];
        auto v3 = triangleVertices[i+2];

        auto v1ID = _trg.Vertex(v1, Vector3d(0, 0, 0));
        auto v2ID = _trg.Vertex(v2, Vector3d(0, 0, 0));
        auto v3ID = _trg.Vertex(v3, Vector3d(0, 0, 0));

        _trg.Triangle(v1ID,v2ID,v3ID,_color,_CurrentTexture);
    } 
  } else {
    vector<size_t> v;
    //  No way to automatically and intelligently assign UV coordinates to
    //    arbitrary polygons, so we have to just leave it as 0.
    for (size_t i = 0; i < polygonPoints.size(); i++)
      v.push_back(_trg.Vertex(polygonPoints[i], Vector3d(0, 0, 0)));
    _trg.Polygon(v, _color, _CurrentTexture);
    polygonPoints.clear();
  }
}
void ObjTurtle::PolygonPoint() { polygonPoints.push_back(_position); }

void ObjTurtle::Circle(float radius) const {
  const float step = M_PI * 2.0f / LPFGParams::DefaultContourDivisions / 2.0f;
  _trg.NewGroup();
  ObjOutputStore::TriangleFanObj triangle(_trg);
  triangle.Normal(_up);
  triangle.Vertex(_position, Vector3d(0.5, 0.5, 0), _color, _CurrentTexture);

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};
  OpenGLMatrix rotationMatrix;
  rotationMatrix.Multiply(rot);

  for (float alpha = 0.0f; alpha <= M_PI * 2.0f + epsilon; alpha += step) {
    Vector3d p(cosf(alpha) * radius, sinf(alpha) * radius, 0.0f);
    p.Transform(rotationMatrix);
    p += _position;
    //  Assign the UV coordinates into a circle on the texture, matching the
    //  circle itself.
    triangle.Vertex(p,
                    Vector3d(0.5 + cosf(alpha) / 2, 0.5 + sinf(alpha) / 2, 0),
                    _color, _CurrentTexture);
  }
}

void ObjTurtle::CircleB(float radius) const {
  const float step = M_PI * 2.0f / LPFGParams::DefaultContourDivisions / 2.0f;
  _trg.NewGroup();
  ObjOutputStore::QuadStripObj quad(_trg);
  quad.Normal(_up);

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};
  OpenGLMatrix rotationMatrix;
  rotationMatrix.Multiply(rot);

  //  In order to have the normals for the quad strip point in the correct
  //    direction, the discs have to be printed in counterclockwise rotation.
  for (float alpha = 0.0f; alpha >= -M_PI * 2.0f - epsilon; alpha -= step) {
    Vector3d p1(cosf(alpha) * (radius - Width() / 2),
                sinf(alpha) * (radius - Width() / 2), 0.0f);
    Vector3d p2(cosf(alpha) * (radius + Width() / 2),
                sinf(alpha) * (radius + Width() / 2), 0.0f);
    p1.Transform(rotationMatrix);
    p2.Transform(rotationMatrix);
    p1 += _position;
    p2 += _position;
    quad.Vertex(p1, Vector3d(0.5, 0.5, 0), _color, _CurrentTexture);
    quad.Vertex(p2, Vector3d(0.5 + cosf(alpha) / 2, 0.5 + sinf(alpha) / 2, 0),
                _color, _CurrentTexture);
  }
}
void ObjTurtle::CircleFront(float radius) const {
  const float step = M_PI * 2.0f / LPFGParams::DefaultContourDivisions / 2.0f;
  _trg.NewGroup();
  ObjOutputStore::TriangleFanObj triangle(_trg);
  triangle.Normal(Vector3d(0, 0, 1));
  triangle.Vertex(_position, Vector3d(0.5, 0.5, 0), _color, _CurrentTexture);

  for (float alpha = 0.0f; alpha <= M_PI * 2.0f + epsilon; alpha += step) {
    Vector3d p(cosf(alpha) * radius, sinf(alpha) * radius, 0.0f);
    p += _position;
    triangle.Vertex(p,
                    Vector3d(0.5 + cosf(alpha) / 2, 0.5 + sinf(alpha) / 2, 0),
                    _color, _CurrentTexture);
  }
}
void ObjTurtle::CircleFrontB(float radius) const {
  const float step = M_PI * 2.0f / LPFGParams::DefaultContourDivisions / 2.0f;
  _trg.NewGroup();
  ObjOutputStore::QuadStripObj quad(_trg);
  quad.Normal(Vector3d(0, 0, 1));

  //  In order to have the normals for the quad strip point in the correct
  //    direction, the discs have to be printed in counterclockwise rotation.
  for (float alpha = 0.0f; alpha >= -M_PI * 2.0f - epsilon; alpha -= step) {
    Vector3d p1(cosf(alpha) * (radius - Width() / 2),
                sinf(alpha) * (radius - Width() / 2), 0.0f);
    Vector3d p2(cosf(alpha) * (radius + Width() / 2),
                sinf(alpha) * (radius + Width() / 2), 0.0f);
    p1 += _position;
    p2 += _position;
    quad.Vertex(p1, Vector3d(0.5, 0.5, 0), _color, _CurrentTexture);
    quad.Vertex(p2, Vector3d(0.5 + cosf(alpha) / 2, 0.5 + sinf(alpha) / 2, 0),
                _color, _CurrentTexture);
  }
}

void ObjTurtle::Mesh(int meshId, float sx, float sy, float sz) const
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
  meshes.GetMesh(meshId).DrawObj(mtrx, nrmx, _trg, _color, texId);
}
/*
void ObjTurtle::AnimatedMesh(int meshId, float scale, float t) const
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
  meshes.GetAnimatedMesh(meshId).DrawObj(t, mtrx, nrmx, _trg, _color, texId);

}
*/
