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

#ifndef __PROJECTION_H__
#define __PROJECTION_H__
#include <ostream>
#include <algorithm>

#include "volume.h"
#include "viewpos.h"
#include "drawparam.h"

class Clipping;

class Projection {
public:
  Projection();
  void Resize(int, int);
  void Reset();
  void ResetRotation();
  void ResetRoll();
  void ResetZoom() { _scale = _DefaultScale; }
  void ResetPan() { _viewPan.Set(0.0f, 0.0f, 0.0f); }
  void Apply(DParams::ProjectionMode); // Returns in the MODELVIEW model

  Vector3d Unproject(int, int, double) const;
  Vector3d Project(double, double, double) const;

  Vector3d GetLookFrom(void) const { return GetLookAt() - _ZShift * _viewDir; }
  Vector3d GetLookAt(void) const { return _viewCenter + _viewPan; }

  void OutputVolume(std::ostream &) const;
  void OutputBoundingBox(std::ostream &) const;

  Vector3d ViewNormal() const { return _viewDir; }
  Vector3d ViewUp() const { return _viewUp; }
  Vector3d ViewLeft() const { return _viewLeft; }
  Vector3d GetPan() const { return _viewPan; }
  float GetFOV() const { return _fov; }
  float GetZShift() const { return _ZShift; }
  void HCenter(Volume, DParams::ProjectionMode);
  void Scale(Volume, const Clipping &, DParams::ProjectionMode);
  void SetModifiers(const WindowParams::ViewModifiers &);

  // Affects only the current matrix (should be PROJECTION)
  void PickRegion(float, float, float, float, DParams::ProjectionMode);

  void DrawAxis() const;

  void RotateBy(float, float);
  void ZoomBy(float);
  void FrustumBy(float);
  void PanBy(int, int);
  void RollBy(float);
  void SetVolumeAndPos(std::pair<Volume, ViewPos>, const Clipping &);
  void SetVolume(Volume, const Clipping &, bool adjustVolume = false);

  // MC - compute view volume given incoming light direction
  void ComputeViewVolume(const Vector3d &lightDir, Volume &lightVV);

  float MinX() const;
  float MaxX() const;
  float MinY() const;
  float MaxY() const;
  float MinZ() const { return -_HalfDepth + (_viewPan * _viewDir); }
  float MaxZ() const { return +_HalfDepth + (_viewPan * _viewDir); }
  float Scale() const { return _scale; }

  void SetZoom(float scale) { _scale = scale; }
  void SetPan(const Vector3d &pan) { _viewPan = pan; }
  void SetRotation(const Vector3d &dir, const Vector3d &up);

  float ZShift() const { return _ZShift; }
  Vector3d GetCenter() const { return _viewCenter - _viewPan; }
  float Width() const;
  float Depth() const { return _HalfDepth * 2; }

  void ApplyRotation(Quaternion rot);
  void ApplyPan(Vector3d pan);

  // Getters needed for the FBX Exporter
  double getGLFOV() const {
    double fov = Rad2Deg(2 * atan(_scale * tan(Deg2Rad(_fov / 2.))));
    if (_sratio > _vratio)
      return fov;
    else
      return Rad2Deg(2 * atan(tan(Deg2Rad(fov / 2.)) / _sratio));
  };
  double getGLNearClip() const { return _ZShift - _HalfDepth; };
  double getGLFarClip() const { return _ZShift + _HalfDepth; };
  int getWindowWidth() const { return _WindowSize.w; };
  int getWindowHeight() const { return _WindowSize.h; };
  Vector3d getGLCameraPos() const {
    Vector3d _viewLookAt = _viewCenter + _viewPan;
    return _viewLookAt - _ZShift * _viewDir;
  };
  Vector3d getGLLookAt() const { return _viewCenter + _viewPan; };
  Vector3d getGLCameraUP() const { return _viewUp; }

  /// Returns the XFOV of the perspective model. If the window is more wide
  ///   than tall, scale the FOV by the ratio of the two to get the XFOV
  ///   specifically.
  double GetRSFOV() const {
    double ratio = tan(Deg2Rad(_fov / 2.0));

    if (_sratio > _vratio)
      ratio *= _sratio;

    return Rad2Deg(2.0f * atan(_scale * ratio));
  };
  /// Returns the XFOV that would give the same image as the parallel model.
  ///   This needs to be scaled by the window dimension that matters for the
  ///   scaling of the model itself.
  double GetRSFOVParallel() const {
    float base;

    if (_sratio < _vratio)
      base = _minWidth;
    else
      base = _minHeight * _sratio;

    return Rad2Deg(2.0f * atan(_scale * base / _ZShift));
  };

  float MinWidth() const { return _minWidth; }
  float MinHeight() const { return _minHeight; }

  void print(std::ostream &os) const {
    os << "Window size : " << _WindowSize.w << " - " << _WindowSize.h
       << std::endl;
    os << "Window size ratio : " << _sratio << std::endl;
    os << "View center : " << _viewCenter.X() << " - " << _viewCenter.Y()
       << " - " << _viewCenter.Z() << std::endl;
    os << "Minimum near clipping : " << _MinimumNearClipping << std::endl;
    os << "Default scale : " << _DefaultScale << std::endl;
    os << "Epsilon : " << _epsilon << std::endl;
    os << "Minimum width : " << _minWidth << std::endl;
    os << "Minimum height : " << _minHeight << std::endl;
    os << "Volume ratio : " << _vratio
       << std::endl; // view volume ratio minWidth / minHeight
    os << "Half Depth: " << _HalfDepth << std::endl;
    os << "Unit per pixel : " << _upp << std::endl; // units per pixel

    os << "Scale : " << _scale << std::endl;
    os << "FOV : " << _fov << std::endl;
    os << "Z Shift : " << _ZShift << std::endl;

    os << "View Dir : " << _viewDir.X() << " - " << _viewDir.Y() << " - "
       << _viewDir.Z() << std::endl;
    os << "View Pan : " << _viewPan.X() << " - " << _viewPan.Y() << " - "
       << _viewPan.Z() << std::endl;
    os << "View up : " << _viewUp.X() << " - " << _viewUp.Y() << " - "
       << _viewUp.Z() << std::endl;
    os << "View Left : " << _viewLeft.X() << " - " << _viewLeft.Y() << " - "
       << _viewLeft.Z() << std::endl;
  }

private:
  void _ApplyParallel();
  void _SetParallelProjection();
  void _SetParallelModelview();
  void _ApplyPerspective();
  void _SetPerspectiveProjection();
  void _SetPerspectiveModelview();

  struct Size {
    int w, h;
  } _WindowSize;
  float _sratio; // window size ratio width / height
  Vector3d _viewCenter;

  static const float _MinimumNearClipping;
  static const float _DefaultScale;
  static const float _epsilon;
  float _minWidth, _minHeight;
  float _vratio; // view volume ratio minWidth / minHeight
  float _HalfDepth;
  float _upp; // units per pixel

  float _scale;
  float _fov;
  float _ZShift;

  Vector3d _viewDir, _viewPan;
  Vector3d _viewUp, _viewLeft;
  Vector3d _upRotationAxis; // MC - Sept. 2016 - save the initial _viewUp vector
                            // for proper mouse-based rotation
  Volume _boundingBox;      // MC - save the bounding box
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
