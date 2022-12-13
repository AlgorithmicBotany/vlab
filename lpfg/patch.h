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



#ifndef __PATCH_H__
#define __PATCH_H__

#include <iostream>
#include <vector>

#include "asrt.h"
#include "vector3d.h"
#include "uvprecision.h"

using std::vector;

class Volume;
class ReadTextFile;
class ObjOutputStore;
class SurfaceObj;

class Patch {
public:
  enum TextureMethod { tmNoTexture, tmTexturePerPatch, tmTexturePerSurface };

  Patch(ReadTextFile &);
  Patch(const SurfaceObj &);
  bool error() { return _error; }
  void Translate(Vector3d);
  void Scale(float);
  void Rotate(Vector3d, Vector3d, Vector3d);
  void Draw(TextureMethod, const Volume &, int sDiv, int tDiv);
  void DrawObj(OpenGLMatrix &, OpenGLMatrix &, ObjOutputStore &, int sDiv,
               int tDiv, int color, int texture,
               TextureMethod tm, const Volume &vol) const;
  void OutputToPOVRay(std::ostream &, TextureMethod, const Volume &vol,
                      int sDiv, int tDiv) const;
  void OutputToPOVRay(std::ostream &, TextureMethod, const Volume &vol,
                      int sDiv, int tDiv, const char *currentTexture,
                      OpenGLMatrix &, OpenGLMatrix &, OpenGLMatrix &) const;
  void GetVolume(Volume &) const;
  void GetVolume(const float[16], Volume &) const;
  Vector3d Point(int id) const { return _arr[id]; }
  void Transform(const OpenGLMatrix &);
  int divS() const { return _divS; };
  int divT() const { return _divT; };

  void GetGeometry(vector<vector<Vector3d>> &pts,
                   vector<vector<Vector3d>> &norms,
                   vector<vector<Vector3d>> &uvs, TextureMethod tm,
                   const Volume &vol, int sDiv, int tDiv) const;

  const std::vector<Vector3d> GetPoints() const { return _vrtx; };

private:
  bool _error;
  void Precompute(int, int);
  Vector3d _Point(float, float) const;
  Vector3d _Normal(float, float) const;
  static float _B(int i, float f);
  static float _dB(int i, float f);

  static float _Pow2(float f) { return f * f; }
  static float _Pow3(float f) { return f * f * f; }

  enum { NumOfControlPoints = 16 };
  Vector3d _arr[NumOfControlPoints];

  int PtId(int s, int t) const {
    ASSERT(s >= 0);
    ASSERT(s <= _divS);
    ASSERT(t >= 0);
    ASSERT(t <= _divT);
    return s + (_divS + 1) * t;
  }
  int _divS, _divT;
  std::vector<Vector3d> _vrtx;
  std::vector<Vector3d> _nrml;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
