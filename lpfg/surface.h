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



#ifndef __SURFACE_H__
#define __SURFACE_H__

#include <string>
#include <vector>

#include "patch.h"
#include "volume.h"

using std::vector;

class ObjOutputStore;
class OpenGLMatrix;

class Surface {
public:
  Surface(const SurfaceObj &);
  Surface(const char *);
  bool Reread();
  void Draw(const UVPrecision &);
  void DrawObj(OpenGLMatrix &, OpenGLMatrix &, ObjOutputStore &, int color,
               int texture) const;
  void GetVolume(Volume &) const;
  void GetVolume(const float[16], Volume &) const;
  bool IsTextured() const { return _TextureId != -1; }
  int TextureId() const { return _TextureId; }
  void SetTextureId(const int id) { _TextureId = id; }

  void DisableTexture() { _TextureId = -1; }
  void OutputToPOVRay(std::ostream &) const;
  void OutputToPOVRay(std::ostream &, const char *, OpenGLMatrix &,
                      OpenGLMatrix &, OpenGLMatrix &) const;
  bool IsValidPatchId(size_t id) const { return id < _patches.size(); }
  const Patch &GetPatch(size_t id) const {
    ASSERT(IsValidPatchId(id));
    return _patches[id];
  }
  void Transform(const OpenGLMatrix &);
  const char *Name() const { return _name.c_str(); }
  void Scale3(float, float, float);

  void GetPatchGeometry(int p, vector<vector<Vector3d>> &pts,
                        vector<vector<Vector3d>> &norms,
                        vector<vector<Vector3d>> &uv) const;
  bool loaded() { return _loaded; }

  float Scale() const { return _scale; }

private:
  bool _LoadSurface(const char *);
  bool _loaded;
  std::string _name;
  std::vector<Patch> _patches;
  typedef std::vector<Patch>::const_iterator citer;
  typedef std::vector<Patch>::iterator iter;
  float _scale;

  UVPrecision _uvPrecision;

  int _TextureId;

  Volume _bbox; // Used for texturing multiple patches
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
