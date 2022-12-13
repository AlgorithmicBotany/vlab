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



#include "surface.h"
#include "exception.h"
#include "file.h"
#include "objout.h"
#include <string.h>
#include <cstdio>

Surface::Surface(const SurfaceObj &so) {
  Patch p(so);
  _patches.push_back(p);
  _scale = 1.0f;
  _TextureId = -1;
  GetVolume(_bbox);
}

Surface::Surface(const char *cmnd) {
  char fnm[80];
  float scale;
  int sDiv, tDiv, txtId;
  int res = sscanf(cmnd, "%79s %f %d %d %d", fnm, &scale, &sDiv, &tDiv, &txtId);
  _scale = 1.0f;
  _TextureId = -1;
  _loaded = true;
  switch (res) {
  case 1: // only filename specified
    break;
  case 2: // filename and scale
    _scale = scale;
    break;
  case 4: // scale, sDiv and tDiv specified
    if (!_uvPrecision.IsValidU(sDiv))
      throw Exception("Invalid s value for surface %s", fnm);
    if (!_uvPrecision.IsValidV(tDiv))
      throw Exception("Invalid t value for surface %d", fnm);
    _scale = scale;
    _uvPrecision.SetU(sDiv);
    _uvPrecision.SetV(tDiv);
    break;
  case 5: // Specified: scake, sDiv, tDiv and txtId
    if (!_uvPrecision.IsValidU(sDiv))
      throw Exception("Invalid s value for surface %s", fnm);
    if (!_uvPrecision.IsValidV(tDiv))
      throw Exception("Invalid t value for surface %d", fnm);
    _scale = scale;
    _uvPrecision.SetU(sDiv);
    _uvPrecision.SetV(tDiv);
    _TextureId = txtId;
    break;
  default:
    throw Exception("Invalid surface specification command\n");
  }

  if (!_LoadSurface(fnm)) {
    _loaded = false;
    return;
  }

  GetVolume(_bbox);
  _name = fnm;
  size_t dot = _name.find('.');
  if (std::string::npos != dot)
    _name.erase(dot);
}

bool Surface::Reread() {
  std::string fname = _name + ".s";
  bool success = false;
#ifndef WIN32
  int cpt = 0;
  while (!success && cpt < 100) {
    success = _LoadSurface(fname.c_str());
    cpt++;
  }
#else
  success = _LoadSurface(fname.c_str());
#endif
  return success;
}

bool Surface::_LoadSurface(const char *fname) {
  _loaded = true;

  const int BfSize = 80;
  char line[BfSize];
#ifndef WIN32
  ReadTextFile src(fname, true);
  if (src.Fp() == NULL) {
    _loaded = false;
    return false;
  }

#else
  ReadTextFile src(fname);
  if (src.Fp() == NULL) {
    _loaded = false;
    return false;
  }
#endif
  // skip bounding box
  src.ReadLine(line, BfSize);

  Vector3d cp;
  src.ReadLine(line, BfSize);
  if (!strncmp(line, "PRECISION", 9))
    src.ReadLine(line, BfSize);

  {
    float x, y, z;
    int res = sscanf(line, "CONTACT POINT  X: %f Y: %f Z: %f", &x, &y, &z);
    if (res != 3) {
      return false;
      throw Exception("Error reading surface file %s", fname);
    }
    cp.Set(x, y, z);
  }

  // skip end point
  src.ReadLine(line, BfSize);

  Vector3d head;
  src.ReadLine(line, BfSize);
  {
    float x, y, z;
    int res = sscanf(line, "HEADING X: %f Y: %f Z: %f", &x, &y, &z);
    if (res != 3) {
      return false;
      throw Exception("Error reading surface file %s", fname);
    }
    head.Set(x, y, z);
    if (head.Length() < 0.001) {
      return false;
      throw Exception("Heading vector in the surface file %s is invalid",
                      fname);
    }
  }
  head.Normalize();

  Vector3d up;
  src.ReadLine(line, BfSize);
  {
    float x, y, z;
    int res = sscanf(line, "UP X: %f Y: %f Z: %f", &x, &y, &z);
    if (res != 3) {
      return false;
      throw Exception("Error reading surface file %s", fname);
    }
    up.Set(x, y, z);
    if (up.Length() < 0.001) {
      return false;
      throw Exception("Up vector in the surface file %s is invalid", fname);
    }
  }
  up.Normalize();

  float size;
  src.ReadLine(line, BfSize);
  {
    int res = sscanf(line, "SIZE: %f", &size);
    if (res != 1) {
      return false;
      throw Exception("Error reading surface file %s", fname);
    }
  }

  Vector3d left = up % head;
  std::vector<Patch> newPatches;
  while (!src.Eof()) {
    int res = fscanf(src.Fp(), "%80s\n", line);
    if (1 == res) {
      Patch patch(src);
      if (patch.error())
        return false;
      patch.Translate(-cp);
      patch.Scale(1.0f / size);
      patch.Rotate(-left, head, up);
      newPatches.push_back(patch);
    }
  }

  _patches.clear();
  for (size_t i = 0; i < newPatches.size(); ++i)
    _patches.push_back(newPatches[i]);

  return true;
}

void Surface::Transform(const OpenGLMatrix &mm) {
  for (iter it = _patches.begin(); it != _patches.end(); ++it)
    it->Transform(mm);
}

void Surface::Scale3(float sx, float sy, float sz) {
  OpenGLMatrix mm;
  mm.Scale(sx, sy, sz);
  Transform(mm);
}

void Surface::Draw(const UVPrecision &p) {
  UVPrecision precision(p);
  if (!precision.IsUSpecified())
    precision.SetU(_uvPrecision.U());
  if (!precision.IsVSpecified())
    precision.SetV(_uvPrecision.V());
  if (_TextureId != -1) {
    if (1 == _patches.size())
      _patches[0].Draw(Patch::tmTexturePerPatch, _bbox, precision.U(),
                       precision.V());
    else {
      for (iter it = _patches.begin(); it != _patches.end(); ++it)
        it->Draw(Patch::tmTexturePerSurface, _bbox, precision.U(),
                 precision.V());
    }
  } else {
    for (iter it = _patches.begin(); it != _patches.end(); ++it)
      it->Draw(Patch::tmNoTexture, _bbox, precision.U(), precision.V());
  }
}

void Surface::GetPatchGeometry(int p, vector<vector<Vector3d>> &pts,
                               vector<vector<Vector3d>> &norms,
                               vector<vector<Vector3d>> &uv) const {
  if (_TextureId != -1) {
    if (1 == _patches.size())
      _patches[p].GetGeometry(pts, norms, uv, Patch::tmTexturePerPatch, _bbox,
                              _uvPrecision.U(), _uvPrecision.V());
    else {
      _patches[p].GetGeometry(pts, norms, uv, Patch::tmTexturePerSurface, _bbox,
                              _uvPrecision.U(), _uvPrecision.V());
    }
  } else {
    _patches[p].GetGeometry(pts, norms, uv, Patch::tmNoTexture, _bbox,
                            _uvPrecision.U(), _uvPrecision.V());
  }
}

void Surface::OutputToPOVRay(std::ostream &stream) const {
  stream << "mesh { " << std::endl;
  if (_TextureId != -1) {
    if (1 == _patches.size())
      _patches[0].OutputToPOVRay(stream, Patch::tmTexturePerPatch, _bbox,
                                 _uvPrecision.U(), _uvPrecision.V());
    else {
      for (citer it = _patches.begin(); it != _patches.end(); ++it)
        it->OutputToPOVRay(stream, Patch::tmTexturePerSurface, _bbox,
                           _uvPrecision.U(), _uvPrecision.V());
    }

  } else {
    for (citer it = _patches.begin(); it != _patches.end(); ++it)
      it->OutputToPOVRay(stream, Patch::tmNoTexture, _bbox, _uvPrecision.U(),
                         _uvPrecision.V());
  }
  stream << "}" << std::endl;
}

void Surface::OutputToPOVRay(std::ostream &stream, const char *currentTexture,
                             OpenGLMatrix &trans, OpenGLMatrix &rot,
                             OpenGLMatrix &scale) const {
  if (_TextureId != -1) {
    if (1 == _patches.size())
      _patches[0].OutputToPOVRay(stream, Patch::tmTexturePerPatch, _bbox,
                                 _uvPrecision.U(), _uvPrecision.V(),
                                 currentTexture, trans, rot, scale);
    else {
      for (citer it = _patches.begin(); it != _patches.end(); ++it)
        it->OutputToPOVRay(stream, Patch::tmTexturePerSurface, _bbox,
                           _uvPrecision.U(), _uvPrecision.V(), currentTexture,
                           trans, rot, scale);
    }
  } else {
    for (citer it = _patches.begin(); it != _patches.end(); ++it)
      it->OutputToPOVRay(stream, Patch::tmNoTexture, _bbox, _uvPrecision.U(),
                         _uvPrecision.V(), currentTexture, trans, rot, scale);
  }
}


void Surface::DrawObj(OpenGLMatrix &vrtx, OpenGLMatrix &nrmx,
                      ObjOutputStore &trg, int color, int texture) const {
  trg.NewGroup();
  // MC - Dec. 2020 - added check for generated tmTexturePerSurface
  if (texture != -1) {
    if (1 == _patches.size())
      _patches[0].DrawObj(vrtx, nrmx, trg,
                          _uvPrecision.U(), _uvPrecision.V(), color,
                          texture, Patch::tmTexturePerPatch, _bbox);
    else {
      for (citer it = _patches.begin(); it != _patches.end(); ++it) {
        it->DrawObj(vrtx, nrmx, trg, _uvPrecision.U(), _uvPrecision.V(), color,
                    texture, Patch::tmTexturePerSurface, _bbox);
      }
    }
  } else {
    for (citer it = _patches.begin(); it != _patches.end(); ++it) {
      it->DrawObj(vrtx, nrmx, trg, _uvPrecision.U(), _uvPrecision.V(), color,
                texture, Patch::tmNoTexture, _bbox);
    }
  }
}


void Surface::GetVolume(const float rot[16], Volume &vol) const {
  for (citer it = _patches.begin(); it != _patches.end(); ++it) {
    Volume v;
    it->GetVolume(rot, v);
    vol.Adapt(v);
  }
}

void Surface::GetVolume(Volume &vol) const {
  static const float rot[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
  GetVolume(rot, vol);
}
