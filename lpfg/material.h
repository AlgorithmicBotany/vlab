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



#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <fstream>

class Material {
public:
  Material();
  void Read(std::istream &);
  void Apply() const;
  void ApplySolid() const;
  const float *Emission() const { return _emission; }
  const float *Diffuse() const { return _diffuse; }
  const float *Ambient() const { return _ambient; }
  const float *Specular() const { return _specular; }
  int Shininess() const { return _shininess; }
  float Transparency() const { return _ambient[3]; }
  void SetTransparency(float t) {
    _diffuse[3] = t;
    _ambient[3] = t;
    _specular[3] = t;
    _emission[3] = t;
  }

  void OutputToPOVRay(std::ostream &) const;

private:
  bool HasSpecular() const {
    return (_specular[0] > 0.0f || _specular[1] > 0.0f || _specular[2] > 0.0f);
  }
  float _ambient[4];
  float _diffuse[4];
  float _specular[4];
  float _emission[4];
  int _shininess;

  // Material editor format
  struct matfiletype {
    unsigned char transparency;
    unsigned char ambient[3];
    unsigned char diffuse[3];
    unsigned char emission[3];
    unsigned char specular[3];
    unsigned char shininess;
  };
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
