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

#include "drawparam.h"
#include "tropismarr.h"
#include "utils.h"
#include "lpfgparams.h"
#include "quaternion.h"

const float Turtle::epsilon = 0.00001f;

Turtle::Turtle()
    : _position(0.0f, 0.0f, 0.0f), _heading(0.0f, 1.0f, 0.0f),
      _up(0.0f, 0.0f, 1.0f), _left(-1.0f, 0.0f, 0.0f), _PrevHeading(_heading),
      _PrevUp(_up), _CurrentTexture(-1), _CurrentTextureSet(false),
      _textureV(0.0f), _textureVCoeff(1.0f), _color(1),
      _origin(0.0f, 0.0f, 0.0f), _xAxis(-1.0f, 0.0f, 0.0f), _yAxis(0.0f, 1.0f, 0.0f),
      _zAxis(0.0f, 0.0f, 1.0f),
      _scaleCartesian(1.f), _width(1.0f), _widthUp(1.f), _STropism(STropism) {}

void Turtle::F(float v) {
  _AdjustTropisms(Tropisms);
  _position += _heading * v;
  _PrevHeading = _heading;
  _PrevUp = _up;
  if (PolygonStarted())
    PolygonPoint();
  _Bend();
}

void Turtle::f(float v) {
  _AdjustTropisms(Tropisms);
  _position += _heading * v;
  if (PolygonStarted())
    PolygonPoint();
  _Bend();
}

void Turtle::G(float v) {
  bool waspolygon = PolygonStarted();
  if (waspolygon)
    SuspendPolygon();
  F(v);
  if (waspolygon)
    ResumePolygon();
}

void Turtle::g(float v) {
  bool waspolygon = PolygonStarted();
  if (waspolygon)
    SuspendPolygon();
  f(v);
  if (waspolygon)
    ResumePolygon();
}

void Turtle::Left(float angle) {
  angle = Deg2Rad(angle);
  const float c = cosf(angle);
  const float s = sinf(angle);
  Vector3d newhead(c * _heading.X() + s * _left.X(),
                   c * _heading.Y() + s * _left.Y(),
                   c * _heading.Z() + s * _left.Z());
  Vector3d newleft(c * _left.X() - s * _heading.X(),
                   c * _left.Y() - s * _heading.Y(),
                   c * _left.Z() - s * _heading.Z());
  _heading = newhead;
  _left = newleft;
}

void Turtle::Right(float angle) {
  angle = -Deg2Rad(angle);
  const float c = cosf(angle);
  const float s = sinf(angle);
  Vector3d newhead(c * _heading.X() + s * _left.X(),
                   c * _heading.Y() + s * _left.Y(),
                   c * _heading.Z() + s * _left.Z());
  Vector3d newleft(c * _left.X() - s * _heading.X(),
                   c * _left.Y() - s * _heading.Y(),
                   c * _left.Z() - s * _heading.Z());
  _heading = newhead;
  _left = newleft;
}

void Turtle::Down(float angle) {
  angle = -Deg2Rad(angle);
  if (!drawparams.CorrectedRotation())
    angle = -angle;
  const float c = cosf(angle);
  const float s = sinf(angle);
  Vector3d newhead(c * _heading.X() + s * _up.X(),
                   c * _heading.Y() + s * _up.Y(),
                   c * _heading.Z() + s * _up.Z());
  Vector3d newup(c * _up.X() - s * _heading.X(), c * _up.Y() - s * _heading.Y(),
                 c * _up.Z() - s * _heading.Z());
  _heading = newhead;
  _up = newup;
}

void Turtle::Up(float angle) {
  angle = Deg2Rad(angle);
  if (!drawparams.CorrectedRotation())
    angle = -angle;
  const float c = cosf(angle);
  const float s = sinf(angle);
  Vector3d newhead(c * _heading.X() + s * _up.X(),
                   c * _heading.Y() + s * _up.Y(),
                   c * _heading.Z() + s * _up.Z());
  Vector3d newup(c * _up.X() - s * _heading.X(), c * _up.Y() - s * _heading.Y(),
                 c * _up.Z() - s * _heading.Z());
  _heading = newhead;
  _up = newup;
}

void Turtle::RollL(float angle) {
  angle = -Deg2Rad(angle);
  if (!drawparams.CorrectedRotation())
    angle = -angle;
  const float c = cosf(angle);
  const float s = sinf(angle);
  Vector3d newleft(c * _left.X() + s * _up.X(), c * _left.Y() + s * _up.Y(),
                   c * _left.Z() + s * _up.Z());
  Vector3d newup(c * _up.X() - s * _left.X(), c * _up.Y() - s * _left.Y(),
                 c * _up.Z() - s * _left.Z());
  _left = newleft;
  _up = newup;
}

void Turtle::RollR(float angle) {
  angle = Deg2Rad(angle);
  if (!drawparams.CorrectedRotation())
    angle = -angle;
  const float c = cosf(angle);
  const float s = sinf(angle);
  Vector3d newleft(c * _left.X() + s * _up.X(), c * _left.Y() + s * _up.Y(),
                   c * _left.Z() + s * _up.Z());
  Vector3d newup(c * _up.X() - s * _left.X(), c * _up.Y() - s * _left.Y(),
                 c * _up.Z() - s * _left.Z());
  _left = newleft;
  _up = newup;
}

void Turtle::IncColor() {
  if (_color == LPFGParams::NumOfColors)
    _color = 1;
  else
    ++_color;
}

void Turtle::DecColor() {
  if (_color == 1)
    _color = LPFGParams::NumOfColors;
  else
    --_color;
}

void Turtle::SetColor(int c) {
  if (c < 1)
    return;
  if (c > LPFGParams::NumOfColors-1)
    return;
  _color = c;
}

void Turtle::SetWidth(float v) {
  _width = v;
  _widthUp = v;
}

void Turtle::Label(const char *) const {}

void Turtle::Query(const LstringIterator &) const {}

void Turtle::Circle(float) const {}

void Turtle::CircleB(float) const {}

void Turtle::Sphere(float) const {}

void Turtle::SetHead(const float *pH) {
  Vector3d head;
  head.Set(pH);
  if (head.Length() < epsilon) {
    Utils::Message("SetHead: Heading vector is null. Module ignored\n");
    return;
  }
  Vector3d up;
  up.Set(pH + 3);
  if (up.Length() < epsilon) {
    Utils::Message("SetHead: Up vector is null. Module ignored\n");
    return;
  }
  head.Normalize();
  up.Normalize();

  // MC - Feb. 2018 - apply current Cartesian coordinate system
  const float model_mat[16] = {-_xAxis.X(), -_xAxis.Y(), -_xAxis.Z(), 0.0f,
                               _yAxis.X(),  _yAxis.Y(),  _yAxis.Z(),  0.0f,
                               _zAxis.X(),  _zAxis.Y(),  _zAxis.Z(),  0.0f,
                               0.0f,        0.0f,        0.0f,        1.0f};
  head.Transform(model_mat);
  up.Transform(model_mat);

  // The prototype HLU is (1,0,0)-(0,1,0)-(0,0,1)
  Vector3d left = up % head;
  if (left.Length() < epsilon) {
    Utils::Message(
        "SetHead: Head and up vectors are parallel. Module ignored\n");
    return;
  }
  _heading.Set(head);
  _up.Set(up);
  _left.Set(left);
  _up = _heading % _left;
}

void Turtle::SetHeadOnly(V3f hd) {
  Vector3d oldHead = _heading, newHead(hd.x, hd.y, hd.z);
  if (newHead.Length() < epsilon) {
    Utils::Message("SetHeadOnly: Heading vector is null. Module ignored\n");
    return;
  }

  // MC - Feb. 2018 - apply current Cartesian coordinate system
  const float model_mat[16] = {-_xAxis.X(), -_xAxis.Y(), -_xAxis.Z(), 0.0f,
                               _yAxis.X(),  _yAxis.Y(),  _yAxis.Z(),  0.0f,
                               _zAxis.X(),  _zAxis.Y(),  _zAxis.Z(),  0.0f,
                               0.0f,        0.0f,        0.0f,        1.0f};
  newHead.Transform(model_mat);

  float cosrotation = oldHead.Normalize() * newHead.Normalize(); 
  if(std::abs(cosrotation) < (1.f-epsilon)) {
      Vector3d axisrotation = (oldHead % newHead).Normalize();
      float halfang = -0.5 * acosf(cosrotation);
      Quaternion q(cosf(halfang), sinf(halfang) * axisrotation);
      _heading.RotateBy(q);
      _up.RotateBy(q);
      _left.RotateBy(q);
  }
  else if (cosrotation < 0.f) {
     return Left(180.f);
  }
}

void Turtle::Sphere0() const { Sphere(_width * 0.5f); }

void Turtle::Circle0() const { Circle(_width * 0.5f); }

void Turtle::Line(V3f vv1, V3f vv2) {
  // MC - Feb. 2018 - apply current Cartesian coordinate system
  Vector3d v1(vv1.x, vv1.y, vv1.z);
  Vector3d v2(vv2.x, vv2.y, vv2.z);
  const float model_mat[16] = {-_xAxis.X(), -_xAxis.Y(), -_xAxis.Z(), 0.0f,
                               _yAxis.X(),  _yAxis.Y(),  _yAxis.Z(),  0.0f,
                               _zAxis.X(),  _zAxis.Y(),  _zAxis.Z(),  0.0f,
                               _origin.X(), _origin.Y(), _origin.Z(), 1.0f};
  v1 *= _scaleCartesian;
  v2 *= _scaleCartesian;
  v1.Transform(model_mat);
  v2.Transform(model_mat);

  Vector3d move(v2.X() - v1.X(), v2.Y() - v1.Y(), v2.Z() - v1.Z());
  const float l = move.Length();
  if (l < epsilon)
    return;
  _position.Set(v1.X(), v1.Y(), v1.Z());
  Vector3d head = _heading;
  _heading.Set(v2.X() - v1.X(), v2.Y() - v1.Y(), v2.Z() - v1.Z());
  _heading.Normalize();
  Vector3d left = _left;
  _left = _up % _heading;
  if (_left.Length() < epsilon)
    _left.Set(-_heading.Y(), _heading.X(), 0.0);
  if (_left.Length() < epsilon)
    _left.Set(1.0, 0.0, 0.0);
  else
    _left.Normalize();
  Vector3d up = _up;
  _up = _heading % _left;
  F(l);
  _heading = head;
  _up = up;
  _left = left;
}

void Turtle::LineTo(V3f vv) {
  // MC - Feb. 2018 - apply current Cartesian coordinate system
  Vector3d v(vv.x, vv.y, vv.z);
  const float model_mat[16] = {-_xAxis.X(), -_xAxis.Y(), -_xAxis.Z(), 0.0f,
                               _yAxis.X(),  _yAxis.Y(),  _yAxis.Z(),  0.0f,
                               _zAxis.X(),  _zAxis.Y(),  _zAxis.Z(),  0.0f,
                               _origin.X(), _origin.Y(), _origin.Z(), 1.0f};
  v *= _scaleCartesian;
  v.Transform(model_mat);

  Vector3d move(v.X() - _position.X(), v.Y() - _position.Y(),
                v.Z() - _position.Z());
  const float l = move.Length();
  if (l < epsilon)
    return;
  Vector3d head = _heading;
  _heading = move;
  _heading.Normalize();
  Vector3d left = _left;
  _left = _up % _heading;
  if (_left.Length() < epsilon)
    _left.Set(-_heading.Y(), _heading.X(), 0.0);
  if (_left.Length() < epsilon)
    _left.Set(1.0f, 0.0f, 0.0f);
  else
    _left.Normalize();
  Vector3d up = _up;
  _up = _heading % _left;
  F(l);
  _heading = head;
  _up = up;
  _left = left;
}

void Turtle::LineRel(V3f vv) {
  // MC - Feb. 2018 - apply current Cartesian coordinate system
  Vector3d v(vv.x, vv.y, vv.z);
  const float model_mat[16] = {-_xAxis.X(), -_xAxis.Y(), -_xAxis.Z(), 0.0f,
                               _yAxis.X(),  _yAxis.Y(),  _yAxis.Z(),  0.0f,
                               _zAxis.X(),  _zAxis.Y(),  _zAxis.Z(),  0.0f,
                               _origin.X(), _origin.Y(), _origin.Z(), 1.0f};
  v *= _scaleCartesian;
  v.Transform(model_mat);

  Vector3d move(v.X(), v.Y(), v.Z());
  const float l = move.Length();
  if (l < epsilon)
    return;
  Vector3d head = _heading;
  _heading.Set(v.X(), v.Y(), v.Z());
  _heading.Normalize();
  Vector3d left = _left;
  _left = _up % _heading;
  if (_left.Length() < epsilon)
    _left.Set(-_heading.Y(), _heading.X(), 0.0);
  if (_left.Length() < epsilon)
    _left.Set(1.0, 0.0, 0.0);
  else
    _left.Normalize();
  Vector3d up = _up;
  _up = _heading % _left;
  F(l);
  _heading = head;
  _up = up;
  _left = left;
}

void Turtle::Dump() const {
  Utils::Message("Position: %f, %f, %f\n", _position.X(), _position.Y(),
                 _position.Z());
  Utils::Message("Heading: %f, %f, %f\n", _heading.X(), _heading.Y(),
                 _heading.Z());
  Utils::Message("Left: %f, %f, %f\n", _left.X(), _left.Y(), _left.Z());
  Utils::Message("Up: %f, %f, %f\n", _up.X(), _up.Y(), _up.Z());
}

void Turtle::MoveTo(float x, float y, float z) {
  // MC - Feb. 2018 - apply current Cartesian coordinate system
  const float model_mat[16] = {-_xAxis.X(), -_xAxis.Y(), -_xAxis.Z(), 0.0f,
                               _yAxis.X(),  _yAxis.Y(),  _yAxis.Z(),  0.0f,
                               _zAxis.X(),  _zAxis.Y(),  _zAxis.Z(),  0.0f,
                               _origin.X(), _origin.Y(), _origin.Z(), 1.0f};
  Vector3d p(x, y, z);
  p.Scale(_scaleCartesian, _scaleCartesian, _scaleCartesian);
  p.Transform(model_mat);
  _position = p;
}

void Turtle::MoveRel(float dx, float dy, float dz) {
  // MC - Feb. 2018 - apply current Cartesian coordinate system
  const float model_mat[16] = {-_xAxis.X(), -_xAxis.Y(), -_xAxis.Z(), 0.0f,
                               _yAxis.X(),  _yAxis.Y(),  _yAxis.Z(),  0.0f,
                               _zAxis.X(),  _zAxis.Y(),  _zAxis.Z(),  0.0f,
                               _origin.X(), _origin.Y(), _origin.Z(), 1.0f};
  Vector3d rel(dx, dy, dz);
  rel.Scale(_scaleCartesian, _scaleCartesian, _scaleCartesian);
  rel.Transform(model_mat);
  _position += rel;
}

void Turtle::Surface(int, float, float, float) const {}

void Turtle::BSurface(int, float, float, float) const {}

void Turtle::DSurface(SurfaceObj, bool) const {}

void Turtle::DBSurfaceS(BsurfaceObjS) const {}

void Turtle::DBSurfaceM(BsurfaceObjM) const {}

void Turtle::InitSurface(int) const {}

static SurfaceObj _sObj;

void Turtle::SurfacePoint(int, int r, int c) {
  _sObj.Set(4 * c + r, _position);
}

void Turtle::DrawSurface(int) const { DSurface(_sObj, false); }

void Turtle::RollToVert() {
  const Vector3d yvec(0.0f, 1.0f, 0.0);
  _left = yvec % _heading;
  if (_left.Length() < epsilon)
    _left.Set(-1.0f, 0.0f, 0.0f);
  else
    _left.Normalize();
  _up = _heading % _left;
}

void Turtle::_AdjustTropisms(const TropismArray &tropisms) {
  if (tropisms.empty())
    return;

  Vector3d desired = tropisms.GlobalDirection();

  typedef std::vector<Tropism>::const_iterator TropismIter;
  bool tropismFound = false;
  for (TropismIter it = tropisms.begin(); it != tropisms.end(); ++it) {
    if (it->Type() == Tropism::trTropism) {
      tropismFound = true;
      if (it->SinGamma() != 0.0f) {
        Vector3d v = _heading % it->Direction();
        float len = v.Length();
        if (len < epsilon) {
          v = _left;
          len = 1.0f;
        }
        Vector3d v2 = it->Direction() % v;
        len = it->EsinGamma() / len;
        desired += v2 * len;
      }
    }
  }

  if (tropismFound) {
    Vector3d rot_vec = _heading % desired;
    _AdjustByOrthotropism(rot_vec);
  }
  _AdjustByTorque(tropisms);
}

void Turtle::_AdjustByOrthotropism(Vector3d v) {
  float len = v.Length();
  if (len < epsilon)
    return;

  if (len > 1.0f)
    len = 1.0f;

  float angle = -len * 0.5f;
  len = 1.0f / len;
  v *= len;

  const float bendsin = sinf(angle);
  v *= bendsin;
  Quaternion q(cosf(angle), v);
  _heading.RotateBy(q);

  _up.RotateBy(q);
  _left = _up % _heading;
}

void Turtle::_AdjustByTorque(const TropismArray &tropisms) {
  if (tropisms.TwistLength() > 0.0f) {
    Vector3d v = _heading % tropisms.Twist();
    if (v.Length() > epsilon) {
      Vector3d des_dir = v % _heading;
      des_dir.Normalize();
      des_dir *= tropisms.TwistLength();
      _up.RotateTo(des_dir);
      _left = _up % _heading;
      _PrevHeading = _heading;
      _PrevUp = _up;
    }
  }
}

void Turtle::SetElasticity(int id, float v) {
  if (!Tropisms.ValidId(id)) {
    Utils::Message("SetElasticity: Invalid tropism id = %d. Module ignored.\n",
                   id);
    Utils::Message("size: %d\n", Tropisms.size());
    return;
  }
  _TropismData.SetElasticity(id, v);
  Tropisms.SetElasticity(id, v);
}

void Turtle::IncElasticity(int id) {
  if (!Tropisms.ValidId(id)) {
    Utils::Message("IncElasticity: Invalid tropism id = %d. Module ignored.\n",
                   id);
    return;
  }
  Tropisms.IncElasticity(id);
  _TropismData.SetElasticity(id, Tropisms.GetElasticity(id));
}

void Turtle::DecElasticity(int id) {
  if (!Tropisms.ValidId(id)) {
    Utils::Message("DecElasticity: Invalid tropism id = %d. Module ignored.\n",
                   id);
    return;
  }
  Tropisms.DecElasticity(id);
  _TropismData.SetElasticity(id, Tropisms.GetElasticity(id));
}

void Turtle::UpdateTropisms() { _TropismData.UpdateTropisms(); }

void Turtle::TRotate(Vector3d &t, const Vector3d &v, const Vector3d &pv,
                     float asin, float acos) const {
  t.X(acos * v.X() + asin * pv.X());
  t.Y(acos * v.Y() + asin * pv.Y());
  t.Z(acos * v.Z() + asin * pv.Z());
}

void Turtle::ChDir(Vector3d &v1, Vector3d &v2, Vector3d &v3, float asin,
                   float acos) const {
  TRotate(v1, v2, v3, asin, acos);
  TRotate(v3, v3, v2, -asin, acos);
  v2 = v1;
}

void Turtle::_Bend() {
  if (_STropism.Elasticity() != 0.0f) {
    float acos = _STropism.Elasticity() * (_STropism.Direction() * _up);
    float asin = sqrtf(1.0f - acos * acos);
    Vector3d tmp;
    ChDir(tmp, _heading, _up, acos, asin);
    acos = _STropism.Elasticity() * (_STropism.Direction() * _left);
    asin = sqrtf(1.0f - acos * acos);
    ChDir(tmp, _heading, _left, acos, asin);
  }
}

void Turtle::CurrentContour(int) {}

void Turtle::StartGC() {}

void Turtle::PointGC() {}

void Turtle::EndGC() {}

// from mesh_unused.h/.cpp
// void Turtle::MeshPoint() {}

void Turtle::E1(float, const LstringIterator &) {}

void Turtle::E2(float, float, const LstringIterator &) {}

void Turtle::StartPolygon() {}

void Turtle::EndPolygon() {}

void Turtle::PolygonPoint() {}

void Turtle::Rhombus(float, float) const {}

void Turtle::Triangle(float, float) const {}

void Turtle::BlendContours(int, int, float) {}

void Turtle::CurrentTexture(int id) {
  if (id != -1 && !textures.IsValid(id)) {
    Utils::Message("CurrentTexture: Invalid texture id = %d. Module ignored\n",
                   id);
    _CurrentTextureSet = false;
  }
  else {
    _CurrentTexture = id;
    _CurrentTextureSet = true;
    _textureV = 0.0f;
  }
}

void Turtle::TextureVCoeff(float f) {
  if (f < 0.0f)
    Utils::Message("TextureVCoeff: parameter %f < 0. Module ignored\n");
  else
    _textureVCoeff = f;
}

void Turtle::Orient() const {}

void Turtle::ScaleContour(float p, float q) {
  // p width along left direction
  // q width along up direction
  _width = p;
  _widthUp = q;
}

void Turtle::Elasticity(float e) { _STropism.Elasticity(e); }

void Turtle::STropismDirection(V3f dir) {
  Vector3d dd(dir.x, dir.y, dir.z);
  _STropism.Direction(dd);
}

void Turtle::ContourSides(int) {}

void Turtle::CircleFront(float) const {}

void Turtle::CircleFrontB(float) const {}

void Turtle::CircleFront0() const { CircleFront(_width * 0.5f); }

void Turtle::SetUPrecision(int n) {
  if (!_uvPrecision.IsValidU(n)) {
    Utils::Message("SetUPrecision: Invalid value u=%d. Module ignored\n", n);
    return;
  }
  _uvPrecision.SetU(n);
}

void Turtle::SetVPrecision(int n) {
  if (!_uvPrecision.IsValidV(n)) {
    Utils::Message("SetUPrecision: Invalid value u=%d. Module ignored\n", n);
    return;
  }
  _uvPrecision.SetV(n);
}

void Turtle::Camera(void) {}

void Turtle::EA(int, const float *, const LstringIterator &) {}

void Turtle::RotateXYZ(V3f axis, float angle) {
  if (axis.Length() < epsilon)
    return;

  Vector3d _axis(axis.x, axis.y, axis.z);
  angle = -Deg2Rad(angle) * 0.5f; // half angle for quaternions
  Quaternion q(cosf(angle), _axis.Normalize(sinf(angle)));
  _heading.RotateBy(q);
  _up.RotateBy(q);
  _left.RotateBy(q);
}

void Turtle::RotateHLU(V3f axis, float angle) {
  if (axis.Length() < epsilon)
    return;

  Vector3d _axis = (axis.x * _heading) + (axis.y * _left) + (axis.z * _up);
  angle = -Deg2Rad(angle) * 0.5f; // half angle for quaternions
  Quaternion q(cosf(angle), _axis.Normalize(sinf(angle)));
  _heading.RotateBy(q);
  _up.RotateBy(q);
  _left.RotateBy(q);
}

void Turtle::Terrain(CameraPosition) const {}

void Turtle::PovRayStart(const char *, POVRayMeshMode)  {}

void Turtle::SetCoordinateSystem(float s) {
  _origin = _position;
  _xAxis = _left;
  _yAxis = _heading;
  _zAxis = _up;
  _scaleCartesian = s;
}

void Turtle::Mesh(int id, float sx, float sy, float sz) const
{}

//void Turtle::AnimatedMesh(int id, float s, float t) const
//{}

void Turtle::StartBranch() {}
