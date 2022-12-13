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



#ifdef _WINDOWS
#include <windows.h>
#include <GL/gl.h>
#else
#include <qgl.h>
#endif

#include "lightsrc.h"
#include "asrt.h"
#include "lpfgparams.h"
#include "utils.h"
#include "povray.h"
#include "exception.h"

LightSource::LightSource() { Default(); }

LightSource::LightSource(const char *line) {
  Default();
  _Load(line);
}

void LightSource::Default() {
  _on = false;
  _SpotExponent = 0.0f;
  _SpotCutoff = 180.0f;
  _Attenuation[aConstant] = 1.0f;
  _Attenuation[aLinear] = 0.0f;
  _Attenuation[aQuadratic] = 0.0f;

  _Ambient[0] = 1.0f;
  _Ambient[1] = 1.0f;
  _Ambient[2] = 1.0f;
  _Ambient[3] = 1.0f;

  _Diffuse[0] = 1.0f;
  _Diffuse[1] = 1.0f;
  _Diffuse[2] = 1.0f;
  _Diffuse[3] = 1.0f;

  _Specular[0] = 1.0f;
  _Specular[1] = 1.0f;
  _Specular[2] = 1.0f;
  _Specular[3] = 1.0f;

  _Position[0] = 0.0f;
  _Position[1] = 0.0f;
  _Position[2] = 1.0f;
  _Position[3] = 0.0f;

  _SpotDirection[0] = 0.0f;
  _SpotDirection[1] = 0.0f;
  _SpotDirection[2] = -1.0f;
  _SpotDirection[3] = -1.0f;
}

void LightSource::Apply(int id) const {
  assert(id >= 0);
  assert(id < LPFGParams::MaxNumOfLights);
  GLenum l = GL_LIGHT0 + id;
  if (_on) {

    glLightf(l, GL_SPOT_EXPONENT, _SpotExponent);
    glLightf(l, GL_SPOT_CUTOFF, _SpotCutoff);
    glLightf(l, GL_CONSTANT_ATTENUATION, _Attenuation[aConstant]);
    glLightf(l, GL_LINEAR_ATTENUATION, _Attenuation[aLinear]);
    glLightf(l, GL_QUADRATIC_ATTENUATION, _Attenuation[aQuadratic]);
    glLightfv(l, GL_AMBIENT, _Ambient);
    glLightfv(l, GL_DIFFUSE, _Diffuse);
    glLightfv(l, GL_SPECULAR, _Specular);

    glLightfv(l, GL_POSITION, _Position);

    glLightfv(l, GL_SPOT_DIRECTION, _SpotDirection);
    glEnable(l);
  } else
    glDisable(l);
}

void LightSource::MoveBy(const float *arr) {
  if (!_IsDirectional()) {
    _Position[0] += arr[0];
    _Position[1] += arr[1];
    _Position[2] += arr[2];
  }
}

void LightSource::SetAmbient(const float *arr) {
  _Ambient[0] = arr[0];
  _Ambient[1] = arr[1];
  _Ambient[2] = arr[2];
  _Ambient[3] = arr[3];
}

float *LightSource::GetAmbient() { return _Ambient; }

void LightSource::SetDiffuse(const float *arr) {
  _Diffuse[0] = arr[0];
  _Diffuse[1] = arr[1];
  _Diffuse[2] = arr[2];
  _Diffuse[3] = arr[3];
}

float *LightSource::GetDiffuse() { return _Diffuse; }

void LightSource::SetSpecular(const float *arr) {
  _Specular[0] = arr[0];
  _Specular[1] = arr[1];
  _Specular[2] = arr[2];
  _Specular[3] = arr[3];
}

float *LightSource::GetSpecular() { return _Specular; }

void LightSource::SetPosition(const float *arr) {
  _Position[0] = arr[0];
  _Position[1] = arr[1];
  _Position[2] = arr[2];
  _Position[3] = 1.0f;
}

float *LightSource::GetPosition() { return _Position; }

void LightSource::SetDirectional(const float *arr) {
  _Position[0] = arr[0];
  _Position[1] = arr[1];
  _Position[2] = arr[2];
  _Position[3] = 0.0f;
}

void LightSource::SetSpotlight(const float *arr) {
  _SpotDirection[0] = arr[0];
  _SpotDirection[1] = arr[1];
  _SpotDirection[2] = arr[2];
  _SpotExponent = arr[3];
  _SpotCutoff = arr[4];
}
void LightSource::GetSpotlight(float *target) {
  target[0] = _SpotDirection[0];
  target[1] = _SpotDirection[1];
  target[2] = _SpotDirection[2];
  target[3] = _SpotExponent;
  target[4] = _SpotCutoff;
}

void LightSource::SetAttenuation(const float *arr) {
  _Attenuation[0] = arr[0];
  _Attenuation[1] = arr[1];
  _Attenuation[2] = arr[2];
}

void LightSource::_Load(const char *line) {
  const char *token = Utils::SkipBlanks(line);
  float arr[5];
  while (0 != token) {
    switch (*token) {
    case 'O':
      token = Utils::ReadFloats(token + 2, arr, 3);
      SetPosition(arr);
      break;
    case 'V':
      token = Utils::ReadFloats(token + 2, arr, 3);
      SetDirectional(arr);
      break;
    case 'A':
      token = Utils::ReadFloats(token + 2, arr, 3);
      SetAmbient(arr);
      break;
    case 'D':
      token = Utils::ReadFloats(token + 2, arr, 3);
      SetDiffuse(arr);
      break;
    case 'S':
      token = Utils::ReadFloats(token + 2, arr, 3);
      SetSpecular(arr);
      break;
    case 'P':
      token = Utils::ReadFloats(token + 2, arr, 5);
      SetSpotlight(arr);
      break;
    case 'T':
      token = Utils::ReadFloats(token + 2, arr, 3);
      SetAttenuation(arr);
      break;
    default:
      throw Exception("Error reading light description\n");
    }
  }
}

void LightSource::OutputToPOVRay(std::ostream &trg) const {
  if (_on) {
    if (_IsAmbient()) {
      trg << "global_settings { ambient_light rgb<" << _Ambient[0] << ", "
          << _Ambient[1] << ", " << _Ambient[2] << "> } " << std::endl;
    }
    if (_IsDirectional()) {
      Vector3d v(100.0f, 100.0f, 100.0f);
      Vector3d dir(_Position[0], _Position[1], _Position[2]);
      v = 100.0f * dir;
      trg << "light_source \n{\n";
      POVRay::Dump(trg, v);
      trg << ',' << "rgb ";
      Vector3d clr(_Diffuse[0], _Diffuse[1], _Diffuse[2]);
      POVRay::Dump(trg, clr);
      trg << std::endl;
      trg << "parallel" << std::endl;
      trg << "point_at ";
      POVRay::Dump(trg, dir);
      trg << std::endl;
      trg << '}' << std::endl;
    }
    {}
  }
}
