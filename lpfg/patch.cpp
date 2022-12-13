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



#include "patch.h"
#include "file.h"
#include "volume.h"
#include "exception.h"
#include "objout.h"
#include "glutils.h"
#include "utils.h"

#include "include/lparams.h"
#include "include/lintrfc.h"

Patch::Patch(ReadTextFile &src) {
  _error = false;
  _divS = _divT = -1;
  const int BfSize = 80;
  char line[BfSize];
  // Skip color info
  src.ReadLine(line, BfSize);
  // Skip three rows of neighboor info
  src.ReadLine(line, BfSize);
  src.ReadLine(line, BfSize);
  src.ReadLine(line, BfSize);

  for (int i = 0; i < NumOfControlPoints; ++i) {
    float x, y, z;
    int res = fscanf(src.Fp(), "%f %f %f", &x, &y, &z);
    if (3 != res) {
      _error = true;
      return;
      throw Exception("Error reading patch info");
    }
    _arr[i].Set(x, y, z);
  }
  // get to the end of file
  src.ReadLine(line, BfSize);
}

Patch::Patch(const SurfaceObj &s) {
  _divS = _divT = -1;
  for (int i = 0; i < NumOfControlPoints; ++i) {
    V3f v(s.Get(i));
    _arr[i].Set(v.x, v.y, v.z);
  }
}

void Patch::Transform(const OpenGLMatrix &mm) {
  for (int i = 0; i < NumOfControlPoints; ++i)
    _arr[i].Transform(mm);
}

void Patch::Translate(Vector3d v) {
  for (int i = 0; i < NumOfControlPoints; ++i)
    _arr[i] += v;
}

void Patch::Scale(float s) {
  for (int i = 0; i < NumOfControlPoints; ++i)
    _arr[i] *= s;
}

void Patch::Rotate(Vector3d l, Vector3d h, Vector3d u) {
  for (int i = 0; i < NumOfControlPoints; ++i)
    _arr[i].RotateBy(h, l, u);
}

void Patch::DrawObj(OpenGLMatrix &vrtx, OpenGLMatrix &nrmx, ObjOutputStore &trg,
                    int sDiv, int tDiv, int color, int texture,
                    TextureMethod tm, const Volume &vol) const {
  if (UVPrecision::eUnspecified == sDiv)
    sDiv = UVPrecision::eUDivDefault;
  if (UVPrecision::eUnspecified == tDiv)
    tDiv = UVPrecision::eUDivDefault;

  for (int i = 0; i < sDiv; ++i) {
    const float u1 = 1.0f * i / sDiv;
    const float u2 = 1.0f * (i + 1) / sDiv;
    ObjOutputStore::QuadStripObj obj(trg);

    for (int j = 0; j <= tDiv; ++j) {
      const float v = 1.0f * j / tDiv;

      Vector3d pnt = -_Normal(u1, v); 
      pnt.Transform(nrmx);
      obj.Normal(pnt);

      pnt = _Point(u1, v);

      //  In order to have textures render the same way in the output
      //    file as they do in LPFG, we must remap the texture
      //    coordinates from (u, v) to (v, 1 - u). This is to be
      //    consistent with how CPFG did textures.
      // MC - Dec. 2020 - if this is a "merged" surface of several patches,
      // modify the texture coordinates. This needs to be redone in all
      // draw calls, because the texture is very specific to Z-X plane. 
      float tv, tu;
      if (tm == tmTexturePerSurface) {
        tv = (pnt.X() - vol.MinX()) / vol.Xrange();
        tu = 1.f - (pnt.Y() - vol.MinY()) / vol.Yrange();
      } else {
        tv = v;
        tu = 1.f - u1;
      }

      pnt.Transform(vrtx); // do transformation after texture coordinate computed
      obj.Vertex(pnt, Vector3d(tv,tu,0.f), color, texture);

      pnt = -_Normal(u2, v);
      pnt.Transform(nrmx);
      obj.Normal(pnt);

      pnt = _Point(u2, v);

      if (tm == tmTexturePerSurface) {
        tv = (pnt.X() - vol.MinX()) / vol.Xrange();
        tu = 1.f - (pnt.Y() - vol.MinY()) / vol.Yrange();
      }  else {
        tv = v;
        tu = 1.f - u2;
      }

      pnt.Transform(vrtx);
      obj.Vertex(pnt, Vector3d(tv,tu,0.f), color, texture);
    }
  }
}

void Patch::Draw(TextureMethod tm, const Volume &vol, int sDiv, int tDiv) {
  if (UVPrecision::eUnspecified == sDiv)
    sDiv = UVPrecision::eUDivDefault;
  if (UVPrecision::eUnspecified == tDiv)
    tDiv = UVPrecision::eUDivDefault;
  if (sDiv != _divS || tDiv != _divT)
    Precompute(sDiv, tDiv);

  // Patch textures are being rendered at 90 degrees from the same model in
  // cpfg; this implies that the internal structure of a patch is different than
  // in cpfg. Rather than alter the internals, the texture coordinates will be
  // modified: (u,v) --> (v,1.-u)
  for (int i = 0; i < _divS; ++i) {
    const float u1 = 1.0f * i / sDiv;
    const float u2 = 1.0f * (i + 1) / sDiv;
    GLprimitive glqs(GL_QUAD_STRIP);
    for (int j = 0; j <= _divT; ++j) {
      int id = PtId(i, j);
      const float v = 1.0f * j / tDiv;

      Vector3d pnt = _vrtx[id];

      switch (tm) {
      case tmNoTexture:
        break;
      case tmTexturePerPatch:
        glTexCoord2f(v, 1.0f - u1);
        break;
      case tmTexturePerSurface: {
        float tv = (pnt.X() - vol.MinX()) / vol.Xrange();
        float tu = (pnt.Y() - vol.MinY()) / vol.Yrange();
        glTexCoord2f(tv, 1.0f - tu);
      } break;
      }

      glNormal3fv(_nrml[id]);
      glVertex3fv(pnt);
      id = PtId(i + 1, j);

      pnt = _vrtx[id];

      switch (tm) {
      case tmNoTexture:
        break;
      case tmTexturePerPatch:
        glTexCoord2f(v, 1.0f - u2);
        break;
      case tmTexturePerSurface: {
        float tv = (pnt.X() - vol.MinX()) / vol.Xrange();
        float tu = (pnt.Y() - vol.MinY()) / vol.Yrange();
        glTexCoord2f(tv, 1.0f - tu);
      } break;
      }

      glNormal3fv(_nrml[id]);
      glVertex3fv(pnt);
    }
  }
}

void Patch::Precompute(int s, int t) {
  size_t newsize = (s + 1) * (t + 1);
  if (_vrtx.size() != newsize) {
    _vrtx.resize(newsize);
    _nrml.resize(newsize);
  }
  _divS = s;
  _divT = t;
  for (int i = 0; i <= _divS; ++i) {
    const float u = 1.0f * i / _divS;
    for (int j = 0; j <= _divT; ++j) {
      const int id = PtId(i, j);
      const float v = 1.0f * j / _divT;
      _vrtx[id] = _Point(u, v);
      _nrml[id] = _Normal(u, v);
    }
  }
}

#define _DumpVector(a) '<' << a.X() << ',' << a.Y() << ',' << a.Z() << '>'
#define _DumpPair(a, b) '<' << a << ',' << b << '>'

void Patch::OutputToPOVRay(std::ostream &_trg, TextureMethod tm,
                           const Volume &vol, int sDiv, int tDiv) const {
  if (UVPrecision::eUnspecified == sDiv)
    sDiv = UVPrecision::eUDivDefault;
  if (UVPrecision::eUnspecified == tDiv)
    tDiv = UVPrecision::eUDivDefault;
  for (int i = 0; i < sDiv; ++i) {
    const float u1 = 1.0f * i / sDiv;
    const float u2 = 1.0f * (i + 1) / sDiv;

    Vector3d pnt1 = _Point(u1, 0);
    Vector3d pnt2 = _Point(u2, 0);
    Vector3d nml1 = _Normal(u1, 0);
    Vector3d nml2 = _Normal(u2, 0);
    float tu1 = 0.0f;
    float tv1 = 0.0f;

    switch (tm) {
    case tmNoTexture:
      break;
    case tmTexturePerPatch:
      tu1 = u1;
      break;
    case tmTexturePerSurface: {
      tu1 = (pnt1.Y() - vol.MinY()) / vol.Yrange();
      tv1 = (pnt1.X() - vol.MinX()) / vol.Xrange();
    } break;
    }

    float tu2 = 0.0f;
    float tv2 = 0.0f;

    switch (tm) {
    case tmNoTexture:
      break;
    case tmTexturePerPatch:
      tu2 = u2;
      break;
    case tmTexturePerSurface: {
      tu2 = (pnt2.Y() - vol.MinY()) / vol.Yrange();
      tv2 = (pnt2.X() - vol.MinX()) / vol.Xrange();
    } break;
    }

    for (int j = 0; j <= tDiv; ++j) {
      const float v = 1.0f * j / tDiv;
      Vector3d pnt3 = _Point(u1, v);
      Vector3d pnt4 = _Point(u2, v);
      Vector3d nml3 = _Normal(u1, v);
      Vector3d nml4 = _Normal(u2, v);

      float tu3 = 0.0f;
      float tv3 = v;

      switch (tm) {
      case tmNoTexture:
        break;
      case tmTexturePerPatch:
        tu3 = u1;
        break;
      case tmTexturePerSurface: {
        tu3 = (pnt3.Y() - vol.MinY()) / vol.Yrange();
        tv3 = (pnt3.X() - vol.MinX()) / vol.Xrange();
      } break;
      }

      float tu4 = 0.0f;
      float tv4 = v;

      switch (tm) {
      case tmNoTexture:
        break;
      case tmTexturePerPatch:
        tu4 = u2;
        break;
      case tmTexturePerSurface: {
        tu2 = (pnt4.Y() - vol.MinY()) / vol.Yrange();
        tv2 = (pnt4.X() - vol.MinX()) / vol.Xrange();
      } break;
      }

      _trg << "\t smooth_triangle { " << _DumpVector(pnt1) << ','
           << _DumpVector(nml1) << ',' << _DumpVector(pnt3) << ','
           << _DumpVector(nml3) << ',' << _DumpVector(pnt2) << ','
           << _DumpVector(nml2);

      if (tm != tmNoTexture)
        _trg << " uv_vectors " << _DumpPair(tu1, tv1) << ','
             << _DumpPair(tu3, tv3) << ',' << _DumpPair(tu2, tv2);

      _trg << '}' << std::endl;

      _trg << "\t smooth_triangle { " << _DumpVector(pnt2) << ','
           << _DumpVector(nml2) << ',' << _DumpVector(pnt3) << ','
           << _DumpVector(nml3) << ',' << _DumpVector(pnt4) << ','
           << _DumpVector(nml4);

      if (tm != tmNoTexture)
        _trg << " uv_vectors " << _DumpPair(tu2, tv2) << ','
             << _DumpPair(tu3, tv3) << ',' << _DumpPair(tu4, tv4);

      _trg << '}' << std::endl;

      pnt1 = pnt3;
      pnt2 = pnt4;
      nml1 = nml3;
      nml2 = nml4;
      tu1 = tu3;
      tu2 = tu4;
      tv1 = tv3;
      tv2 = tv4;
    }
  }
}

void Patch::OutputToPOVRay(std::ostream &_trg, TextureMethod tm,
                           const Volume &vol, int sDiv, int tDiv,
                           const char *, OpenGLMatrix &trans,
                           OpenGLMatrix &rot, OpenGLMatrix &scale) const {
  // Transformed versions of the points for outputing to povray
  OpenGLMatrix mat;
  mat.Multiply(trans.Buf());
  mat.Multiply(rot.Buf());
  mat.Multiply(scale.Buf());
  Vector3d outpnt1, outpnt2, outpnt3, outpnt4, outnml1, outnml2, outnml3,
      outnml4;

  if (UVPrecision::eUnspecified == sDiv)
    sDiv = UVPrecision::eUDivDefault;
  if (UVPrecision::eUnspecified == tDiv)
    tDiv = UVPrecision::eUDivDefault;
  for (int i = 0; i < sDiv; ++i) {
    const float u1 = 1.0f * i / sDiv;
    const float u2 = 1.0f * (i + 1) / sDiv;

    Vector3d pnt1 = _Point(u1, 0);
    Vector3d pnt2 = _Point(u2, 0);
    Vector3d nml1 = _Normal(u1, 0);
    Vector3d nml2 = _Normal(u2, 0);

    float tu1 = 0.0f;
    float tv1 = 0.0f;

    switch (tm) {
    case tmNoTexture:
      break;
    case tmTexturePerPatch:
      tu1 = u1;
      break;
    case tmTexturePerSurface: {
      tu1 = (pnt1.Y() - vol.MinY()) / vol.Yrange();
      tv1 = (pnt1.X() - vol.MinX()) / vol.Xrange();
    } break;
    }

    float tu2 = 0.0f;
    float tv2 = 0.0f;

    switch (tm) {
    case tmNoTexture:
      break;
    case tmTexturePerPatch:
      tu2 = u2;
      break;
    case tmTexturePerSurface: {
      tu2 = (pnt2.Y() - vol.MinY()) / vol.Yrange();
      tv2 = (pnt2.X() - vol.MinX()) / vol.Xrange();
    } break;
    }

    for (int j = 0; j <= tDiv; ++j) {
      const float v = 1.0f * j / tDiv;
      Vector3d pnt3 = _Point(u1, v);
      Vector3d pnt4 = _Point(u2, v);
      Vector3d nml3 = _Normal(u1, v);
      Vector3d nml4 = _Normal(u2, v);

      float tu3 = 0.0f;
      float tv3 = v;

      switch (tm) {
      case tmNoTexture:
        break;
      case tmTexturePerPatch:
        tu3 = u1;
        break;
      case tmTexturePerSurface: {
        tu3 = (pnt3.Y() - vol.MinY()) / vol.Yrange();
        tv3 = (pnt3.X() - vol.MinX()) / vol.Xrange();
      } break;
      }

      float tu4 = 0.0f;
      float tv4 = v;

      switch (tm) {
      case tmNoTexture:
        break;
      case tmTexturePerPatch:
        tu4 = u2;
        break;
      case tmTexturePerSurface: {
        tu2 = (pnt4.Y() - vol.MinY()) / vol.Yrange();
        tv2 = (pnt4.X() - vol.MinX()) / vol.Xrange();
      } break;
      }

      outpnt1 = pnt1;
      outpnt1.Transform(mat);
      outpnt2 = pnt2;
      outpnt2.Transform(mat);
      outpnt3 = pnt3;
      outpnt3.Transform(mat);
      outpnt4 = pnt4;
      outpnt4.Transform(mat);

      outnml1 = nml1;
      outnml1.Transform(rot);
      outnml1.Normalize(); // if (outnml1.Y() < 0) outnml1.Y(-outnml1.Y());
      outnml2 = nml2;
      outnml2.Transform(rot);
      outnml2.Normalize(); // if (outnml2.Y() < 0) outnml2.Y(-outnml2.Y());
      outnml3 = nml3;
      outnml3.Transform(rot);
      outnml3.Normalize(); // if (outnml3.Y() < 0) outnml3.Y(-outnml3.Y());
      outnml4 = nml4;
      outnml4.Transform(rot);
      outnml4.Normalize(); // if (outnml4.Y() < 0) outnml4.Y(-outnml4.Y());

      _trg << "\t smooth_triangle { " << _DumpVector(outpnt1) << ','
           << _DumpVector(outnml1) << ',' << _DumpVector(outpnt3) << ','
           << _DumpVector(outnml3) << ',' << _DumpVector(outpnt2) << ','
           << _DumpVector(outnml2);

      if (tm != tmNoTexture) {
        _trg << " uv_vectors " << _DumpPair(tu1, tv1) << ','
             << _DumpPair(tu3, tv3) << ',' << _DumpPair(tu2, tv2);
      }

      _trg << '}' << std::endl;

      _trg << "\t smooth_triangle { " << _DumpVector(outpnt2) << ','
           << _DumpVector(outnml2) << ',' << _DumpVector(outpnt3) << ','
           << _DumpVector(outnml3) << ',' << _DumpVector(outpnt4) << ','
           << _DumpVector(outnml4);

      if (tm != tmNoTexture) {
        _trg << " uv_vectors " << _DumpPair(tu2, tv2) << ','
             << _DumpPair(tu3, tv3) << ',' << _DumpPair(tu4, tv4);
      }

      _trg << '}' << std::endl;

      pnt1 = pnt3;
      pnt2 = pnt4;
      nml1 = nml3;
      nml2 = nml4;
      tu1 = tu3;
      tu2 = tu4;
      tv1 = tv3;
      tv2 = tv4;
    }
  }
}

Vector3d Patch::_Point(float u, float v) const {
  Vector3d res;
  int ix = 0;
  for (int i = 0; i <= 3; ++i) {
    for (int j = 0; j <= 3; ++j)
      res += _arr[ix++] * (_B(i, u) * _B(j, v));
  }
  return res;
}

Vector3d Patch::_Normal(float u, float v) const {
  Vector3d du;
  int ix = 0;
  int i;
  for (i = 0; i <= 3; ++i) {
    for (int j = 0; j <= 3; ++j)
      du += _arr[ix++] * (_dB(i, u) * _B(j, v));
  }
  Vector3d dv;
  ix = 0;
  for (i = 0; i <= 3; ++i) {
    for (int j = 0; j <= 3; ++j)
      dv += _arr[ix++] * (_B(i, u) * _dB(j, v));
  }
  Vector3d res = du % dv;
  res.Normalize();

  return res;
}

float Patch::_B(int i, float f) {
  switch (i) {
  case 0:
    return _Pow3(1.0f - f);
  case 1:
    return 3.0f * f * _Pow2(1.0f - f);
  case 2:
    return 3.0f * _Pow2(f) * (1.0f - f);
  case 3:
    return _Pow3(f);
  default:
    ASSERT(0);
    return 0;
  }
}

float Patch::_dB(int i, float f) {
  switch (i) {
  case 0:
    return -3.0f * _Pow2(1.0f - f);
  case 1:
    return 3.0f * _Pow2(1.0f - f) - (6.0f * (1.0f - f) * f);
  case 2:
    return 6.0f * (1.0f - f) * f - 3.0f * _Pow2(f);
  case 3:
    return 3.0f * _Pow2(f);
  default:
    ASSERT(0);
    return 0;
  }
}

void Patch::GetVolume(Volume &v) const {
  for (int i = 0; i < NumOfControlPoints; ++i)
    v.Adapt(_arr[i]);
}

void Patch::GetVolume(const float rot[16], Volume &v) const {
  for (int i = 0; i < NumOfControlPoints; ++i) {
    Vector3d p(_arr[i], rot);
    v.Adapt(p);
  }
}

void Patch::GetGeometry(vector<vector<Vector3d>> &pts,
                        vector<vector<Vector3d>> &norms,
                        vector<vector<Vector3d>> &uvs, TextureMethod tm,
                        const Volume &vol, int sDiv, int tDiv) const {
  if (UVPrecision::eUnspecified == sDiv)
    sDiv = UVPrecision::eUDivDefault;
  if (UVPrecision::eUnspecified == tDiv)
    tDiv = UVPrecision::eUDivDefault;

  for (int i = 0; i <= sDiv; ++i) {
    vector<Vector3d> meshPts;
    vector<Vector3d> meshNormals;
    vector<Vector3d> meshUV;
    const float u = 1.0f * i / sDiv;
    for (int j = 0; j <= tDiv; ++j) {
      const float v = 1.0f * j / tDiv;

      Vector3d pnt, norm, uv;

      pnt = _Point(u, v);
      norm = _Normal(u, v);
      switch (tm) {
      case tmNoTexture:
        break;
      case tmTexturePerPatch:
        uv.X(u);
        uv.Y(v);
        break;
      case tmTexturePerSurface: {
        uv.X((pnt.Y() - vol.MinY()) / vol.Yrange());
        uv.Y((pnt.X() - vol.MinX()) / vol.Xrange());
      } break;
      }

      meshPts.push_back(pnt);
      meshNormals.push_back(norm);
      meshUV.push_back(uv);
    }

    pts.push_back(meshPts);
    norms.push_back(meshNormals);
    uvs.push_back(meshUV);
  }
}
