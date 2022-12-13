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



#ifndef __WINPARAMS_H__
#define __WINPARAMS_H__

#include <string>

#include "vector3d.h"
#include "volume.h"

class WindowBorderParams {
public:
  WindowBorderParams();
  WindowBorderParams(const WindowBorderParams &wbp) {
    _width = wbp.width();
    _red = wbp.red();
    _green = wbp.green();
    _blue = wbp.blue();
  }

  bool Read(const char *);

  int width() const { return _width; }
  int red() const { return _red; }
  int green() const { return _green; }
  int blue() const { return _blue; }

private:
  int _width;
  int _red;
  int _green;
  int _blue;
};

class WindowParams {
public:
  WindowParams();
  bool Read(const char *);
  const std::string &Name() const { return _name; }
  float Left() const { return _left; }
  float Top() const { return _top; }
  float Width() const { return _width; }
  float Height() const { return _height; }

  void SetModifierDir(const Vector3d &dir) {
    if (!_modifiers.set)
      _modifiers.SetDefaults();
    _modifiers.viewDir = dir;
  }
  void SetModifierUp(const Vector3d &up) {
    if (!_modifiers.set)
      _modifiers.SetDefaults();
    _modifiers.viewUp = up;
  }
  void SetModifierPan(const Vector3d &pan) {
    if (!_modifiers.set)
      _modifiers.SetDefaults();
    _modifiers.viewPan = pan;
  }
  void SetModifierZShift(float zshift) {
    if (!_modifiers.set)
      _modifiers.SetDefaults();
    _modifiers.ZShift = zshift;
  }
  void SetModifierScale(float scale) {
    if (!_modifiers.set)
      _modifiers.SetDefaults();
    _modifiers.scale = scale;
  }
  void SetModifierFOV(float fov) {
    if (!_modifiers.set)
      _modifiers.SetDefaults();
    _modifiers.fov = fov;
  }

  void SetBoundingBox(const Volume &bbox) { _bbox = bbox; }
  bool IsBoundingBoxSet() const { return _bbox.IsInitialized(); }
  const Volume &GetBoundingBox() const { return _bbox; }
  float Scale() const { return _modifiers.scale; }
  float FOV() const { return _modifiers.fov; }
  float ZShift() const { return _modifiers.ZShift; }
  const Vector3d &GetDir() const { return _modifiers.viewDir; }
  const Vector3d &GetUp() const { return _modifiers.viewUp; }
  const Vector3d &GetPan() const { return _modifiers.viewPan; }
  bool ModifiersSet() const { return _modifiers.set; }
  struct ViewModifiers {
    ViewModifiers() : set(false) {}

    void SetDefaults(void) {
      set = true;
      viewDir = Vector3d(0, 0, -1);
      viewUp = Vector3d(0, 1, 0);
      viewPan = Vector3d(0, 0, 0);
      // ZShift = 1.0;
      // MC - August 2016 - Old-style view command doesn't set ZShift;
      // set to zero to flag Projection::SetModifiers to recalculate the
      // z-shift.
      ZShift = 0.0;
      fov = 45.0;
      scale = 0.0;
    }

    Vector3d viewDir, viewUp, viewPan;
    float ZShift, fov, scale;

    bool set;
  };
  const ViewModifiers &Modifiers() const { return _modifiers; }

private:
  float _left;
  float _top;
  float _width;
  float _height;
  std::string _name;

  ViewModifiers _modifiers;
  Volume _bbox;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
