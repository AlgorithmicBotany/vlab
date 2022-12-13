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



#ifndef __GLENV_H__
#define __GLENV_H__

#include <iostream>
#include "colormap.h"
#include "materialset.h"
#include "lightsrc.h"
#include "vector3d.h"

namespace Colors {
const float White[4] = {1.0f, 1.0f, 1.0f, 1.0f};
};

class GLEnv {
public:
  GLEnv();
  ~GLEnv();

  void DoInit(void *, int);

  void SetColor(int) const;
  void SetSolidColor(int) const;
  void ZBuffer() const;
  void ClearColor() const;
  void ClearTransparentColor() const;
  void LoadColormap(const char *);
  void LoadMaterials(const char *);
  Vector3d GetBgColor() const { return GetSolidColor(0); }
  Vector3d GetSolidColor(int) const;
  Vector3d GetTextColor(int) const;

  void ResetLights();
  void ResetLightPositions();
  void AddLight(const LightSource &);

  void OutputToPOVRay(std::ostream &, Vector3d) const;
  void OutputMatToPOVRay(int, std::ostream &) const;

  void Disk(double, double, int, int, void *) const;
  void Sphere(double, int, int, void *) const;
  void *CreateQuadric();
  void DeleteQuadric(void *);

  MaterialSet *GetMaterialSet() { return &_materials; };

  LightSource GetLight(int id);
  int GetLightCount();
  bool BindTextures();

  const unsigned char *GetColor(int v) { return _colormap.GetColor(v); }

private:
  void _DefaultColormap();
  void _DefaultLight();

  Colormap _colormap;
  MaterialSet _materials;
  LightSource _lights[LPFGParams::MaxNumOfLights];
  int _lightsCount;
};

extern GLEnv gl;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
