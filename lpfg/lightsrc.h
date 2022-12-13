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



#ifndef __LIGHTSRC_H__
#define __LIGHTSRC_H__

#include <iostream>

enum Attenuation { aConstant = 0, aLinear, aQuadratic };

class LightSource {
public:
  LightSource();
  LightSource(const char *);
  void Default();
  void MoveBy(const float *);
  void Apply(int) const;
  void SetAmbient(const float *);
  float *GetAmbient();

  void SetDiffuse(const float *);
  float *GetDiffuse();

  void SetSpecular(const float *);
  float *GetSpecular();

  // This one implies point source
  void SetPosition(const float *);
  float *GetPosition();

  // This one implies directional source
  void SetDirectional(const float *);

  void SetSpotlight(const float *);
  void GetSpotlight(float *);

  void SetAttenuation(const float *);
  void TurnOn() { _on = true; }
  void TurnOff() { _on = false; }
  void OutputToPOVRay(std::ostream &) const;

private:
  void _Load(const char *);

  bool _on;

  bool _IsAmbient() const {
    return _Ambient[0] > 0.0f || _Ambient[1] > 0.0f || _Ambient[2] > 0.0f;
  }
  bool _IsDirectional() const { return 0.0f == _Position[3]; }

  float _SpotExponent;
  float _SpotCutoff;
  float _Attenuation[3];
  float _Ambient[4];
  float _Diffuse[4];
  float _Specular[4];
  float _Position[4];
  float _SpotDirection[4];
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
