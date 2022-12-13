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
#include "surfarr.h"
#include "drawparam.h"
#include "volume.h"
#include "bsurfarr.h"
#include "BsurfaceObj.h"
#include "mesharr.h" // MC - Dec. 2020 - for drawing meshes

void ViewTurtle::F(float v) {
  switch (drawparams.LineStyle()) {
  case DParams::lsPolygon: {
    Vector3d p = _position + _left * (0.5f * Width());
    _volume.Adapt(p);
    p = _position - _left * (0.5f * Width());
    _volume.Adapt(p);
  } break;
  case DParams::lsCylinder: {
    if (!_genCylStarted) {
        _prevGenCylWid = Width();
        _prevGenCylWidUp = WidthUp();
    }
    Vector3d p = _position + _left * 0.5f * _prevGenCylWid * _leftGenCylPive;
    _volume.Adapt(p);
    p = _position - _left * 0.5f * _prevGenCylWid * _leftGenCylNive;
    _volume.Adapt(p);
    p = _position + _up * 0.5f * _prevGenCylWidUp * _upGenCylPive;
    _volume.Adapt(p);
    p = _position - _up * 0.5f * _prevGenCylWidUp * _upGenCylNive;
    _volume.Adapt(p);
  } break;
  case DParams::lsPixel:
    _volume.Adapt(_position);
    break;
  }
  Turtle::F(v);
  switch (drawparams.LineStyle()) {
  case DParams::lsPolygon: {
    Vector3d p = _position + _left * (0.5f * Width());
    _volume.Adapt(p);
    p = _position - _left * (0.5f * Width());
    _volume.Adapt(p);
  } break;
  case DParams::lsCylinder: {
    Vector3d p = _position + _left * 0.5f * Width() * _leftGenCylPive;
    _volume.Adapt(p);
    p = _position - _left * 0.5f * Width() * _leftGenCylNive;
    _volume.Adapt(p);
    p = _position + _up * 0.5f * WidthUp() * _upGenCylPive;
    _volume.Adapt(p);
    p = _position - _up * 0.5f * WidthUp() * _upGenCylNive;
    _volume.Adapt(p);
    if (_genCylStarted) {
        _prevGenCylWid = Width();
        _prevGenCylWidUp = WidthUp();
    }
  } break;
  case DParams::lsPixel:
    _volume.Adapt(_position);
    break;
  }
}

void ViewTurtle::f(float v) {
  _volume.Adapt(_position);
  Turtle::f(v);
  _volume.Adapt(_position);
}

void ViewTurtle::Circle(float r) const {
  Vector3d tmp(_position);
  tmp += _heading * r;
  _volume.Adapt(tmp);
  tmp = _position - _heading * r;
  _volume.Adapt(tmp);
  tmp = _position + _left * r;
  _volume.Adapt(tmp);
  tmp = _position - _left * r;
  _volume.Adapt(tmp);
}

void ViewTurtle::CircleB(float r) const {
  Circle(r + Width()*0.5f); 
}

void ViewTurtle::Sphere(float r) const {
  Vector3d tmp(_position);
  tmp.AddX(r);
  _volume.Adapt(tmp);
  tmp = _position;
  tmp.AddY(r);
  _volume.Adapt(tmp);
  tmp = _position;
  tmp.AddZ(r);
  _volume.Adapt(tmp);
  tmp = _position;
  tmp.AddX(-r);
  _volume.Adapt(tmp);
  tmp = _position;
  tmp.AddY(-r);
  _volume.Adapt(tmp);
  tmp = _position;
  tmp.AddZ(-r);
  _volume.Adapt(tmp);
}

void ViewTurtle::CircleFront(float r) const { Sphere(r); }

void ViewTurtle::CircleFrontB(float r) const { Sphere(r + Width()*0.5f); }

void ViewTurtle::Surface(int id, float sx, float sy, float sz) const {
  if (surfaces.ValidId(id)) {
    ::Surface s = surfaces.Get(id);
    s.Scale3(sx*s.Scale(), sy*s.Scale(), sz*s.Scale());
    AdaptSurf(s);
  }
}

void ViewTurtle::AdaptSurf(::Surface &s) const {
  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};
  OpenGLMatrix mm;
  mm.Translate(_position);
  mm.Multiply(rot);
  s.Transform(mm);
  Volume vol;
  s.GetVolume(vol);
  _volume.Adapt(vol);
}

void ViewTurtle::DSurface(SurfaceObj s, bool) const {
  ::Surface srf(s);
  AdaptSurf(srf);
}

void ViewTurtle::BSurface(int id, float sx, float sy, float sz) const {
  if (!bsurfaces.ValidId(id))
    return;

  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};
  OpenGLMatrix mm;
  mm.Translate(_position);
  mm.Multiply(rot);
  mm.Scale(sx, sy, sz);
  Volume vol;
  b_wrapper s(bsurfaces.Get(id));
  s.Transform(mm);
  s.GetVolume(vol);
  _volume.Adapt(vol);
}

void ViewTurtle::DBSurfaceS(BsurfaceObjS s) const {
  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};
  OpenGLMatrix mm;
  mm.Translate(_position);
  mm.Multiply(rot);
  Volume vol;
  b_wrapper surface(GetBsurface(s));
  surface.Transform(mm);
  surface.GetVolume(vol);
  _volume.Adapt(vol);
}

void ViewTurtle::DBSurfaceM(BsurfaceObjM s) const {
  const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};
  OpenGLMatrix mm;
  mm.Translate(_position);
  mm.Multiply(rot);
  Volume vol;
  b_wrapper surface(GetBsurface(s));
  surface.Transform(mm);
  surface.GetVolume(vol);
  _volume.Adapt(vol);
}

void ViewTurtle::Rhombus(float a, float b) const {
  Vector3d v(_position);
  _volume.Adapt(v);
  v += _heading * a;
  _volume.Adapt(v);
  v = _position + _heading * a * 0.5f;
  v += _left * b * 0.5f;
  _volume.Adapt(v);
  v -= _left * b;
  _volume.Adapt(v);
}

void ViewTurtle::Triangle(float a, float b) const {
  Vector3d v(_position);
  _volume.Adapt(v);
  v += _heading * b;
  _volume.Adapt(v);
  v = _position;
  v += _left * a * 0.5f;
  _volume.Adapt(v);
  v -= _left * a;
  _volume.Adapt(v);
}

void ViewTurtle::StartPolygon() { _polygonStarted = true; }

void ViewTurtle::EndPolygon() { _polygonStarted = false; }

void ViewTurtle::PolygonPoint() {
  if (_polygonStarted)
    _volume.Adapt(_position);
}

void ViewTurtle::Camera() {
  _camera.Set(_position, _heading, _left, _up);
}

void ViewTurtle::Terrain(CameraPosition) const {
  if (terrainData != NULL) {
    printf("Calculating terrain volume");
    terrainData->getVolume(_volume);
  } 
}

void ViewTurtle::CurrentContour(int id) {
  if (!contours.ValidId(id)) {
    return;
  }

  // get the min and max points of bounding box
  const Contour &contour = contours.Get(id);
  Vector3d minPt = contour.GetMinPt();
  Vector3d maxPt = contour.GetMaxPt();

  // find the scaling values in the positive and negative quadrants for both
  // left and up directions
  _leftGenCylPive = maxPt.X() > 0.f ? maxPt.X() : 0.f;
  _leftGenCylNive = minPt.X() < 0.f ? -minPt.X() : 0.f;
  _upGenCylPive = maxPt.Y() > 0.f ? maxPt.Y() : 0.f;
  _upGenCylNive = minPt.Y() < 0.f ? -minPt.Y() : 0.f;
}

void ViewTurtle::BlendContours(int id1, int id2, float x) {
  if (!contours.ValidId(id1)) {
    return;
  }
  if (!contours.ValidId(id2)) {
    return;
  }
  if (x < 0.0f) {
    x = 0.0f;
  } else if (x > 1.0f) {
    x = 1.0f;
  }

  // get the min and max pts of bounding box for both curves
  const Contour &contour1 = contours.Get(id1);
  Vector3d minPt1 = contour1.GetMinPt();
  Vector3d maxPt1 = contour1.GetMaxPt();

  const Contour &contour2 = contours.Get(id2);
  Vector3d minPt2 = contour2.GetMinPt();
  Vector3d maxPt2 = contour2.GetMaxPt();

  // blend between them
  Vector3d minPt = (1.f - x) * minPt1 + x * minPt2;
  Vector3d maxPt = (1.f - x) * maxPt1 + x * maxPt2;

  // find the scaling values in the positive and negative quadrants for both
  // left and up directions
  _leftGenCylPive = maxPt.X() > 0.f ? maxPt.X() : 0.f;
  _leftGenCylNive = minPt.X() < 0.f ? -minPt.X() : 0.f;
  _upGenCylPive = maxPt.Y() > 0.f ? maxPt.Y() : 0.f;
  _upGenCylNive = minPt.Y() < 0.f ? -minPt.Y() : 0.f;
}

void ViewTurtle::StartGC()
{
    _genCylStarted = true;
    _prevGenCylWid = Width();
    _prevGenCylWidUp = WidthUp();
}

void ViewTurtle::EndGC()
{
    _genCylStarted = false;
}

void ViewTurtle::Mesh(int meshId, float sx, float sy, float sz) const
{
	if (meshes.ValidMeshId(meshId))
	{
		const class Mesh &obj = meshes.GetMesh(meshId);
		Volume vol = obj.GetBoundingBox();

		// scale the bounding box
		Vector3d min = vol.Min();
		Vector3d max = vol.Max();
		vol.Set(sx * min.X(),sx * max.X(),
                sy * min.Y(),sy * max.Y(),
                sz * min.Z(),sz * max.Z());
 
		// rotate it
		const float rot[16] =
		{
		-_left.X(),		-_left.Y(),		-_left.Z(),		0.0f,
		_heading.X(),	_heading.Y(),	_heading.Z(),	0.0f,
		_up.X(),		_up.Y(),		_up.Z(),		0.0f,
		0.0f,		0.0f,		0.0f,		1.0f
		};
		Vector3d min_rot (vol.Min(), rot);
		vol.Adapt(min_rot);
		Vector3d max_rot (vol.Max(), rot);
		vol.Adapt(max_rot);

		// translate it
		vol.Translate(_position);

		// adapte view volume
		_volume.Adapt(vol);
	}
}

/*
void ViewTurtle::AnimatedMesh(int meshId, float scale, float t) const
{
	if (meshes.ValidAnimatedMeshId(meshId))
	{
		const class MeshConstraintAnimation &obj = meshes.GetAnimatedMesh(meshId);
		Volume vol = obj.GetBoundingBox();

		// scale the bounding box
		Vector3d min = scale * vol.Min();
		Vector3d max = scale * vol.Max();
		vol.Set(min.X(),max.X(),min.Y(),max.Y(),min.Z(),max.Z());

		// rotate it
		const float rot[16] =
		{
		-_left.X(),		-_left.Y(),		-_left.Z(),		0.0f,
		_heading.X(),	_heading.Y(),	_heading.Z(),	0.0f,
		_up.X(),		_up.Y(),		_up.Z(),		0.0f,
		0.0f,		0.0f,		0.0f,		1.0f
		};
		Vector3d min_rot (vol.Min(), rot);
		vol.Adapt(min_rot);
		Vector3d max_rot (vol.Max(), rot);
		vol.Adapt(max_rot);

		// translate it
		vol.Translate(_position);

		// adapte view volume
		_volume.Adapt(vol);
	}
}
*/
