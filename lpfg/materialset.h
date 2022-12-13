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



#ifndef __MATERIALSET_H__
#define __MATERIALSET_H__

#include "material.h"
#include "lpfgparams.h"
#include "asrt.h"

class MaterialSet {
public:
  MaterialSet();
  void Apply(int id) const {
    ASSERT(ValidId(id));
    _material[id].Apply();
  }
  void ApplySolidColor(int id) const {
    ASSERT(ValidId(id));
    _material[id].ApplySolid();
  }
  void Load(const char *);
  bool Set() const { return _set; }
  void GetColor(int id, float arr[3]) const {
    ASSERT(ValidId(id));
    arr[0] = _material[id].Emission()[0];
    arr[1] = _material[id].Emission()[1];
    arr[2] = _material[id].Emission()[2];
  }

  const float *Emission(int id) const {
    ASSERT(ValidId(id));
    return _material[id].Emission();
  }
  const float *Diffuse(int id) const {
    ASSERT(ValidId(id));
    return _material[id].Diffuse();
  }
  const float *Ambient(int id) const {
    ASSERT(ValidId(id));
    return _material[id].Ambient();
  }
  const float *Specular(int id) const {
    ASSERT(ValidId(id));
    return _material[id].Specular();
  }
  int Shininess(int id) const {
    ASSERT(ValidId(id));
    return _material[id].Shininess();
  }
  float Transparency(int id) const {
    ASSERT(ValidId(id));
    return _material[id].Transparency();
  }

  void SetTransparency(int id, float t) {
    ASSERT(ValidId(id));
    _material[id].SetTransparency(t);
  }

  void OutputToPOVRay(int id, std::ostream &out) const {
    ASSERT(ValidId(id));
    _material[id].OutputToPOVRay(out);
  }

private:
  bool ValidId(int id) const {
    return (id >= 0) && (id < LPFGParams::NumOfColors);
  }
  bool _set;
  Material _material[LPFGParams::NumOfColors];
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
