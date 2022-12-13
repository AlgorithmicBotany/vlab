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



#define GL_SILENCE_DEPRECATION
#ifdef __APPLE__
#include <OpenGL/glu.h>
#include <qgl.h>
#else
#ifdef WIN32
#include <GL/glew.h>
#include <windows.h>
#endif
#include <GL/glu.h>
#endif
#include <algorithm>
using std::max;

#include "utils.h"
#include "quaternion.h"
#include "projection.h"
#include "glutils.h"
#include "glenv.h"

const float Projection::_MinimumNearClipping = 0.001f;
const float Projection::_DefaultScale = 1.1f;
const float Projection::_epsilon = 0.0001f;

Projection::Projection() {
    _sratio = _minWidth / _minHeight;
    Reset(); 
}

void Projection::Reset() {
  _fov = 45.0f;
  _scale = drawparams.Scale();
  _minWidth = _minHeight = 1.1f;
  _vratio = _minWidth / _minHeight;
  _HalfDepth = 1.1f;
  _ZShift = 1.0f;

  _viewPan = Vector3d(0, 0, 0);
  ResetPan();
  ResetRotation();
}

void Projection::ResetRotation() {
  _viewDir = Vector3d(0, 0, -1);
  _viewUp = Vector3d(0, 1, 0);
  _viewLeft = _viewDir % _viewUp;
  _upRotationAxis = _viewUp;
}

void Projection::SetRotation(const Vector3d &dir, const Vector3d &up) {
  _viewDir = dir;
  _viewUp = up;
  _viewLeft = dir % up;
  _viewUp = _viewLeft % dir;
  _upRotationAxis = _viewUp;
}

void Projection::ResetRoll() {
  // Rotate about _viewDir so that _viewLeft is perpendicular to the y-axis
  // This means that we rotate by theta, where tan(theta) = 2 * (ly / uy)
  float theta = 0.5f * atan2(2.0f * _viewLeft.Y(), _viewUp.Y());
  Quaternion rot(cos(Deg2Rad(theta)), sin(Deg2Rad(theta)) * _viewDir);
  ApplyRotation(rot);
}

void Projection::SetModifiers(const WindowParams::ViewModifiers &vm) {
  _scale = vm.scale;
  if (_scale == 0.0)
    _scale = drawparams.Scale();
  _fov = vm.fov;

  _viewPan = vm.viewPan;

  _viewDir = vm.viewDir;
  _viewDir.Normalize();
  _viewUp = vm.viewUp;
  // MC - August 2016 - check if the angle between Dir and Up is almost zero
  if (fabs(_viewDir * _viewUp) >= 1.f - Projection::_epsilon) {
    _viewUp.Set(0.f, -_viewDir.Z(), _viewDir.Y());
  }
  _viewLeft = (_viewDir % _viewUp).Normalize();
  _viewUp = (_viewLeft % _viewDir).Normalize();
  _upRotationAxis = _viewUp;

  _ZShift = vm.ZShift;
  // If vm.ZShift was zero, that means that we must recalculate it.
  if (_ZShift <= 0)
    _ZShift = _minWidth * _scale / tanf(Deg2Rad(_fov * 0.5f)) + _HalfDepth;
}

void Projection::SetVolumeAndPos(std::pair<Volume, ViewPos> pr,
                                 const Clipping &clip)
// This function sets the view and bounding box of the scene.
// The view can be specified in the view file using the "view:" command.
// The bounding box is computed by lpfg using the vvturtle.
// If the "box:" command is specified in the view file,
// this function is not called, and SetVolume() is called only.
{
  SetVolume(pr.first, clip, true);
  if (pr.second.IsInitialized()) {
    _viewDir = pr.second.Head();
    _viewUp = pr.second.Up();
    _viewDir.Normalize();
    _viewUp.Normalize();
    _viewLeft = _viewDir % _viewUp;
    _upRotationAxis = _viewUp;

    if ((_viewCenter - pr.second.Pos()).Length() < _HalfDepth)
      // The camera is in the scene, so we have to adjust the clipping planes
      _HalfDepth = max(_HalfDepth, _ZShift);

    _viewPan = (pr.second.Pos() + _ZShift * _viewDir) - _viewCenter;
  }
}

void Projection::SetVolume(Volume v, const Clipping &clip, bool adjustVolume) {
  const float MinDim = 0.1f;
  const float HalfDepthCoef = 0.525f;
  const float MinHalfDepth = 0.5f;
  _viewCenter.X(v.CX());
  _viewCenter.Y(v.CY());
  _viewCenter.Z(v.CZ());
  _boundingBox = v;
  Vector3d v_min(v.Min());
  Vector3d v_max(v.Max());

  // MC - Sept. 2016 - if no "box:" command was specified and
  // the view direction is not the default viewing direction,
  // compute view volume from new viewing direction
  Vector3d default_view(0, 0, -1);
  if (adjustVolume && fabs(_viewDir * default_view) < 1.f - Projection::_epsilon) {

    // translate bounding box so centre is at origin
    v_min -= _viewCenter;
    v_max -= _viewCenter;

    // specify all the points in the bounding box
    Vector3d pts[8];
    pts[0] = Vector3d(v_min.X(), v_min.Y(), v_min.Z());
    pts[1] = Vector3d(v_max.X(), v_min.Y(), v_min.Z());
    pts[2] = Vector3d(v_min.X(), v_max.Y(), v_min.Z());
    pts[3] = Vector3d(v_max.X(), v_max.Y(), v_min.Z());
    pts[4] = Vector3d(v_min.X(), v_min.Y(), v_max.Z());
    pts[5] = Vector3d(v_max.X(), v_min.Y(), v_max.Z());
    pts[6] = Vector3d(v_min.X(), v_max.Y(), v_max.Z());
    pts[7] = Vector3d(v_max.X(), v_max.Y(), v_max.Z());

    // rotate the points of the bounding box (assuming the vectors are unit
    // length)
    Quaternion q(1.f + _viewDir * default_view, _viewDir % default_view);
    q = q.Normalize();
    for (int i = 0; i < 8; i++) {
      pts[i].RotateBy(q);
    }

    // find the minimum and maximum of rotated bounding box
    v_min = pts[0];
    v_max = pts[7];
    for (int i = 0; i < 8; i++) {
      if (pts[i].X() < v_min.X())
        v_min.X(pts[i].X());
      else if (pts[i].X() > v_max.X())
        v_max.X(pts[i].X());

      if (pts[i].Y() < v_min.Y())
        v_min.Y(pts[i].Y());
      else if (pts[i].Y() > v_max.Y())
        v_max.Y(pts[i].Y());

      if (pts[i].Z() < v_min.Z())
        v_min.Z(pts[i].Z());
      else if (pts[i].Z() > v_max.Z())
        v_max.Z(pts[i].Z());
    }

    // save the new bounding box (translate back to original centre)
    v_min += _viewCenter;
    v_max += _viewCenter;
    _boundingBox.Set(v_min.X(),v_max.X(),
                     v_min.Y(),v_max.Y(),
                     v_min.Z(),v_max.Z());
  }

  _minWidth = 0.5f * (v_max.X() - v_min.X());
  _minHeight = 0.5f * (v_max.Y() - v_min.Y());
  if (0.0f == _minWidth)
    _minWidth = MinDim * _minHeight;
  if (0.0f == _minHeight)
    _minHeight = MinDim * _minWidth;
  if (0.0f == _minWidth)
    _minWidth = _minHeight = MinDim;
  _HalfDepth = HalfDepthCoef * Distance(v_min, v_max);
  if (0.0f == _HalfDepth)
    _HalfDepth = MinHalfDepth;

  if (clip.Specified()) {
    float b = clip.Back();
    float f = clip.Front();
    // MC - Nov. 2016 - The center of rotation is always the center of the
    // object, and should not be affected by positions of the clipping planes.
    // float cz = (b+f)*0.5f;
    //_viewCenter.Z(cz);
    _HalfDepth = fabsf((b - f) * 0.5f); // why not HalfDepthCoef?
  }

  _ZShift = _minWidth * _scale / tanf(Deg2Rad(_fov * 0.5f)) + _HalfDepth;
  _vratio = _minWidth / _minHeight;
}

void Projection::OutputVolume(std::ostream &trg) const {
  trg << MinX() << ' ' << MaxX() << ' ' << MinY() << ' ' << MaxY() << ' '
      << MinZ() << ' ' << MaxZ() << std::endl;
}

void Projection::OutputBoundingBox(std::ostream &trg) const {
  trg << _boundingBox.MinX() << ' ' << _boundingBox.MaxX() << ' '
      << _boundingBox.MinY() << ' ' << _boundingBox.MaxY() << ' '
      << _boundingBox.MinZ() << ' ' << _boundingBox.MaxZ() << std::endl;
}

void Projection::HCenter(Volume v, DParams::ProjectionMode mode) {
  // doing a full center for now
  // [PASCAL] performing only a center on X axis
  _viewPan.X(v.Center().X() - _viewCenter.X());
  Apply(mode);
}

void Projection::Scale(Volume v, const Clipping &clip,
                       DParams::ProjectionMode mode) {
  SetVolume(v, clip);
  Apply(mode);
}

void Projection::PanBy(int x, int y) {
  _viewPan -= _upp * (static_cast<float>(y) * _viewUp +
                      static_cast<float>(x) * _viewLeft);
}

void Projection::ZoomBy(float z) {
  _scale *= z;
  if (_scale < drawparams.MinZoom())
    _scale = drawparams.MinZoom();
  else if (_scale > drawparams.MaxZoom())
    _scale = drawparams.MaxZoom();
}

void Projection::RollBy(float r) {
  Quaternion rot(cos(Deg2Rad(r)), sin(Deg2Rad(r)) * _viewDir);
  ApplyRotation(rot);
}

void Projection::FrustumBy(float z) {
  const float PrevZSh = _ZShift;
  const float onc = _ZShift - _HalfDepth;
  float nc = onc;
  nc *= z;
  if (nc < _MinimumNearClipping)
    nc = _MinimumNearClipping;
  _ZShift = nc + _HalfDepth;
  const float gr = PrevZSh * nc / (onc * _ZShift);
  _minWidth *= gr;
  _minHeight *= gr;
  // _vratio does not change
}

void Projection::RotateBy(float rx, float ry) {
  // const static Vector3d YAXIS(0,1,0), XAXIS(1,0,0);
  // MC - Sept. 2016 - rotate around the initial view's "Up" direction instead
  // of YAXIS
  Quaternion rot(cos(Deg2Rad(ry)), sin(Deg2Rad(ry)) * _viewLeft);
  rot = rot * Quaternion(cos(Deg2Rad(rx)),
                         sin(Deg2Rad(rx)) * _upRotationAxis); // * YAXIS);
  ApplyRotation(rot);
}

void Projection::Resize(int w, int h) {
  ASSERT(w > 0);
  ASSERT(h > 0);
  _WindowSize.w = w;
  _WindowSize.h = h;
  _sratio = float(w) / float(h);
}

float Projection::MinX(void) const {
  if (_sratio > _vratio) // window wider than viewspace => scaled by _minHeight
    return -_sratio * _minHeight * _scale +
           ((_viewCenter + _viewPan) * _viewLeft);
  else // scaled by _minWidth
    return -_minWidth * _scale + ((_viewCenter + _viewPan) * _viewLeft);
}

float Projection::MaxX(void) const {
  if (_sratio > _vratio) // window wider than viewspace => scaled by _minHeight
    return _sratio * _minHeight * _scale +
           ((_viewCenter + _viewPan) * _viewLeft);
  else // scaled by _minWidth
    return _minWidth * _scale + ((_viewCenter + _viewPan) * _viewLeft);
}

float Projection::MinY(void) const {
  if (_sratio > _vratio) // window wider than viewspace => scaled by _minHeight
    return -_minHeight * _scale + ((_viewCenter + _viewPan) * _viewUp);
  else // scaled by _minWidth
    return -(_minWidth * _scale / _sratio) +
           ((_viewCenter + _viewPan) * _viewUp);
}

float Projection::MaxY(void) const {
  if (_sratio > _vratio) // window wider than viewspace => scaled by _minHeight
    return _minHeight * _scale + ((_viewCenter + _viewPan) * _viewUp);
  else // scaled by _minWidth
    return (_minWidth * _scale / _sratio) +
           ((_viewCenter + _viewPan) * _viewUp);
}

float Projection::Width() const {
  if (_sratio > _vratio)
    return _minHeight * 2 * _scale * _sratio;
  else
    return _minWidth * 2 * _scale;
}

void Projection::Apply(DParams::ProjectionMode mode) {
  if (DParams::pmParallel == mode)
    _ApplyParallel();
  else
    _ApplyPerspective();

  if (!drawparams.StationaryLights())
    gl.ResetLightPositions();
}

void Projection::PickRegion(float x, float y, float w, float h,
                            DParams::ProjectionMode mode) {
  glLoadIdentity();
  GLint viewport[4] = {0, 0, _WindowSize.w, _WindowSize.h};
  gluPickMatrix(x, viewport[3] - y, w, h, viewport);
  if (DParams::pmParallel == mode)
    _SetParallelProjection();
  else
    _SetPerspectiveProjection();
}

void Projection::_ApplyPerspective() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  _SetPerspectiveProjection();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  _SetPerspectiveModelview();
  glViewport(0, 0, _WindowSize.w, _WindowSize.h);
}

void Projection::_SetPerspectiveProjection() {
  const float fov = Rad2Deg(2.0f * atan(_scale * tan(Deg2Rad(_fov / 2.0f))));

  float nearClip = _ZShift - _HalfDepth, farClip = _ZShift + _HalfDepth;
  if (nearClip < _MinimumNearClipping)
    nearClip = _MinimumNearClipping;

  if (_sratio > _vratio) // window wider than it is high
  {
    gluPerspective(fov, _sratio, nearClip, farClip);
    _upp = 2.0f * _ZShift * tan(Deg2Rad(fov / 2.0f)) / _WindowSize.h;
  } else // window is higher than it is wide
  {
    // calculate the Y field of view: tan(fovy/2) = (1/_sratio) * tan(fovx/2)
    float yfov = Rad2Deg(2.0f * atan(tan(Deg2Rad(fov / 2.0f)) / _sratio));
    gluPerspective(yfov, _sratio, nearClip, farClip);
    _upp = 2.0f * _ZShift * tan(Deg2Rad(yfov / 2.0f)) / _WindowSize.h;
  }
}

void Projection::_SetPerspectiveModelview() {
  Vector3d _viewLookAt = _viewCenter + _viewPan;
  Vector3d _viewPos = _viewLookAt - _ZShift * _viewDir;
  gluLookAt(_viewPos.X(), _viewPos.Y(), _viewPos.Z(), _viewLookAt.X(),
            _viewLookAt.Y(), _viewLookAt.Z(), _viewUp.X(), _viewUp.Y(),
            _viewUp.Z());
}

void Projection::_ApplyParallel() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  _SetParallelProjection();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  _SetParallelModelview();
  glViewport(0, 0, _WindowSize.w, _WindowSize.h);
}

void Projection::_SetParallelProjection() {
  float nearClip = _ZShift - _HalfDepth, farClip = _ZShift + _HalfDepth;
  if (nearClip < _MinimumNearClipping)
    nearClip = _MinimumNearClipping;

  if (_sratio > _vratio) // window wider than it is high
  {
    float ySize = _minHeight * _scale, xSize = ySize * _sratio;
    glOrtho(-xSize, xSize, -ySize, ySize, nearClip, farClip);
    _upp = 2 * ySize / _WindowSize.h;
  } else // window is higher than it is wide
  {
    float xSize = _minWidth * _scale, ySize = xSize / _sratio;
    glOrtho(-xSize, xSize, -ySize, ySize, nearClip, farClip);
    _upp = 2 * ySize / _WindowSize.h;
  }
}

void Projection::_SetParallelModelview() {
  Vector3d _viewLookAt = _viewCenter + _viewPan;
  Vector3d _viewPos = _viewLookAt - _ZShift * _viewDir;
  gluLookAt(_viewPos.X(), _viewPos.Y(), _viewPos.Z(), _viewLookAt.X(),
            _viewLookAt.Y(), _viewLookAt.Z(), _viewUp.X(), _viewUp.Y(),
            _viewUp.Z());
}

void Projection::DrawAxis() const {
  const int AxisViewportSize = 30;
  glPPSM ppmm(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0, 0, 0, _viewDir.X(), _viewDir.Y(), _viewDir.Z(), _viewUp.X(),
            _viewUp.Y(), _viewUp.Z());

  glPPSM pppm(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.05f, 1.05f, -1.05f, 1.05f, -1.05f, 1.05f);

  glViewport(0, 0, AxisViewportSize, AxisViewportSize);
  glClear(GL_DEPTH_BUFFER_BIT);
  {
    glLineWidth(2.0f);
    GLprimitive lines(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0, 0, 1);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(1, 0, 0);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0, 1, 0);
  }
  glViewport(0, 0, _WindowSize.w, _WindowSize.h);
  glLineWidth(drawparams.WireframeLineWidth());
}

Vector3d Projection::Unproject(int x, int y, double z) const {
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

  GLdouble projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  GLfloat winX = (float)x, winY = (float)(viewport[3] - y);

  GLdouble pos[3];
  gluUnProject(winX, winY, z, modelview, projection, viewport, &pos[0], &pos[1],
               &pos[2]);

  return Vector3d((float)(pos[0]), (float)(pos[1]), (float)(pos[2]));
}

Vector3d Projection::Project(double x, double y, double z) const {
  GLdouble modelview[16];
  glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

  GLdouble projection[16];
  glGetDoublev(GL_PROJECTION_MATRIX, projection);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  GLdouble pos[3];
  gluProject((GLdouble)x, (GLdouble)y, (GLdouble)z,
             modelview, projection, viewport,
             &pos[0], &pos[1], &pos[2]);

  return Vector3d((float)(pos[0]), (float)(pos[1]), (float)(pos[2]));
}

void Projection::ApplyRotation(Quaternion rot) {
  _viewDir.RotateBy(rot);
  _viewUp.RotateBy(rot);
  _viewLeft.RotateBy(rot);
}
void Projection::ApplyPan(Vector3d pan) {
  Vector3d pantx = pan.X() * _viewLeft + pan.Y() * _viewUp - pan.Z() * _viewDir;
  _viewPan += pantx;
}

void Projection::ComputeViewVolume(const Vector3d &lightDir, Volume &lightVV)
// used to compute view volume from a given direction (e.g., from a light
// source)
{
  const float MinDim = 0.1f;
  const float HalfDepthCoef = 0.525f;
  const float MinHalfDepth = 0.5f;

  Vector3d v_min(_boundingBox.Min());
  Vector3d v_max(_boundingBox.Max());

  Vector3d default_view(0, 0, -1);
  if (fabs(lightDir * default_view) < 1.f - Projection::_epsilon) {

    // find all the points in the bounding box
    Vector3d pts[8];
    pts[0] = Vector3d(v_min.X(), v_min.Y(), v_min.Z());
    pts[1] = Vector3d(v_max.X(), v_min.Y(), v_min.Z());
    pts[2] = Vector3d(v_min.X(), v_max.Y(), v_min.Z());
    pts[3] = Vector3d(v_max.X(), v_max.Y(), v_min.Z());
    pts[4] = Vector3d(v_min.X(), v_min.Y(), v_max.Z());
    pts[5] = Vector3d(v_max.X(), v_min.Y(), v_max.Z());
    pts[6] = Vector3d(v_min.X(), v_max.Y(), v_max.Z());
    pts[7] = Vector3d(v_max.X(), v_max.Y(), v_max.Z());

    // rotate the points of the bounding box (assuming the vectors are unit
    // length)
    Quaternion q(1.f + lightDir * default_view, lightDir % default_view);
    q = q.Normalize();
    for (int i = 0; i < 8; i++) {
      pts[i].RotateBy(q);
    }

    // find the minimum and maximum of rotated bounding box
    v_min = pts[0];
    v_max = pts[7];
    for (int i = 0; i < 8; i++) {
      if (pts[i].X() < v_min.X())
        v_min.X(pts[i].X());
      else if (pts[i].X() > v_max.X())
        v_max.X(pts[i].X());

      if (pts[i].Y() < v_min.Y())
        v_min.Y(pts[i].Y());
      else if (pts[i].Y() > v_max.Y())
        v_max.Y(pts[i].Y());

      if (pts[i].Z() < v_min.Z())
        v_min.Z(pts[i].Z());
      else if (pts[i].Z() > v_max.Z())
        v_max.Z(pts[i].Z());
    }
  }

  float minWidth = 0.5f * (v_max.X() - v_min.X());
  float minHeight = 0.5f * (v_max.Y() - v_min.Y());
  if (0.0f == minWidth)
    minWidth = MinDim * minHeight;
  if (0.0f == _minHeight)
    minHeight = MinDim * minWidth;
  if (0.0f == _minWidth)
    minWidth = minHeight = MinDim;
  float HalfDepth = HalfDepthCoef * Distance(v_min, v_max);
  if (0.0f == HalfDepth)
    HalfDepth = MinHalfDepth;

  const float DefaultFov = 45.0f; // default from drawparam.cpp
  float ZShift = minWidth / tanf(Deg2Rad(DefaultFov * 0.5f));
  float minZ = ZShift - HalfDepth;
  float maxZ = ZShift + 2.f * HalfDepth;
  if (minZ < _MinimumNearClipping)
    minZ = _MinimumNearClipping;

  // scale such that the entire object will be rendered
  const float DefaultScale = 1.41421356f;
  const float sratio = 1.f; // the shadow map is always square
  float vratio = minWidth / minHeight;
  float minX, maxX, minY, maxY;
  if (sratio > vratio) { // window wider than viewspace
    minX = -sratio * minHeight * DefaultScale;
    maxX = sratio * minHeight * DefaultScale;
    minY = -minHeight * DefaultScale;
    maxY = minHeight * DefaultScale;
  } else {
    minX = -minWidth * DefaultScale;
    maxX = minWidth * DefaultScale;
    minY = -(minWidth * DefaultScale / sratio);
    maxY = (minWidth * DefaultScale / sratio);
  }

  lightVV.Set(minX, maxX, minY, maxY, minZ, maxZ);
  return;
}
