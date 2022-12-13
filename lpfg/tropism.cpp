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



#include "tropism.h"
#include "utils.h"
#include "exception.h"

Tropism::Tropism(const char *line, trType type) : _type(type) {
  _Default();
  _Load(line);
}

void Tropism::_Default() {
  _direction.Set(0.0f, -1.0f, 0.0f);
  _gamma = 0.0f;
  _elasticity = 0.0f;
  _step = 0.0f;
}

void Tropism::_Load(const char *line) {
  const char *token = Utils::SkipBlanks(line);
  float arr[3];
  float len = 1.0f;
  while (0 != token) {
    switch (*token) {
    case 'T': {
      token = Utils::ReadFloats(token + 2, arr, 3);
      Vector3d v;
      v.Set(arr);
      if (v.IsNull())
        throw Exception("Tropism vector is null\n");
      SetDirection(arr);
    } break;
    case 'A': {
      if (trTorque == _type)
        throw Exception("A command not allowed for torque\n");

      token = Utils::ReadFloats(token + 2, arr, 1);
      if (arr[0] > 180.0f)
        throw Exception("Tropism angle > 180.0\n");
      else if (arr[0] < -180.0f)
        throw Exception("Tropism angle < -180.0\n");
      arr[0] = Deg2Rad(arr[0]);
      SetAngle(arr[0]);
    } break;
    case 'I': {
      token = Utils::ReadFloats(token + 2, arr, 1);
      if (0.0f == arr[0])
        throw Exception("Tropism intensity is 0\n");
      len = arr[0];
    } break;
    case 'E':
      token = Utils::ReadFloats(token + 2, arr, 1);
      _elasticity = arr[0];
      break;
    case 'S':
      token = Utils::ReadFloats(token + 2, arr, 1);
      _step = arr[0];
      break;
    default:
      throw Exception("Error reading tropism/torque description\n");
    }
  }
  _direction *= len;
  _Update();
}

void Tropism::_Update() {
  _singamma = sinf(_gamma);
  _cosgamma = cosf(_gamma);
  _esingamma = _elasticity * _singamma;
}

void Tropism::SetElasticity(float e) {
  _elasticity = e;
  _esingamma = _elasticity * _singamma;
}
