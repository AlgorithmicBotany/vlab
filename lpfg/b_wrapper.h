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



#ifndef B_WRAPPER_H
#define B_WRAPPER_H

#ifdef LINUX
#include <qiodevice.h>
#endif // LINUX

#include "bsurface.h"
#include "exception.h"
#include "vector3d.h"
#include "volume.h"
#include "utils.h"
#include "glutils.h"
#include "uvprecision.h"
#include "include/lparams.h"
#include "include/lintrfc.h"
#include "objout.h"

class b_wrapper {

public:
  b_wrapper(){};
  b_wrapper(const char *line);
  b_wrapper(const b_wrapper &B) {
    surf = B.surf;
    _uvPrecision = B._uvPrecision;
  }
  b_wrapper(V3f **, int N, int M);

  void Load(const char *fname) { surf.LoadPatch(fname); }

  void Reread() { return surf.Reread(); }

  void GetControlNet(V3f **v, int &N, int &M);

  V3f GetControlPoint(unsigned int i, unsigned int j);
  void SetControlPoint(unsigned int i, unsigned int j, V3f p);

  void GetVolume(Volume &);

  void Transform(const OpenGLMatrix &);

  bool IsTextured() const { return surf.IsTextured(); }
  int TextureId() const { return surf.TextureId(); }

  b_wrapper operator+(const b_wrapper &B) {
    b_wrapper r;
    r.surf = surf + B.surf;
    r._uvPrecision = _uvPrecision;
    return r;
  }
  b_wrapper operator-(const b_wrapper &B) {
    b_wrapper r;
    r.surf = surf - B.surf;
    r._uvPrecision = _uvPrecision;
    return r;
  }
  b_wrapper operator*(const float c) {
    b_wrapper r;
    r.surf = surf * c;
    r._uvPrecision = _uvPrecision;
    return r;
  }

  void Draw(const UVPrecision &);
  void Rayshade(const UVPrecision &, std::ofstream &, bool, std::string);
  void PostScript(const UVPrecision &, std::string &, Vector3d);
  void DrawObj(const UVPrecision &, ObjOutputStore &, int, int);
  b_wrapper Interpolate(const b_wrapper b1, const b_wrapper b2, float alpha,
                        int r, int c);

private:
  BSplineSurface surf;
  UVPrecision _uvPrecision;
};

#endif
