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



#include "objout.h"
#include "materialset.h"
#include "texturearr.h"

#include <iomanip>
#include <string>

const float epsilon = 0.0001f;

ObjOutputStore::ObjOutputStore(std::string fnm, GLEnv &glEnv, const Volume &v)
    : _v(v), _precision(Distance(_v.Max(), _v.Min()) * epsilon), _groupId(0),
      _glEnv(glEnv) {
  std::string objfile(fnm);
  objfile.append(".obj");
  std::string mtlfile(fnm);
  mtlfile.append(".mtl");
  _trg.open(objfile.c_str());
  _mtl.open(mtlfile.c_str());
  _trg << "# Obj output created by lpfg" << std::endl;
  _trg << "mtllib " << fnm << ".mtl" << std::endl;
  _mtl << "# Mtl output created by lpfg" << std::endl;

  for (int i = 0; i < 256; i++) {
    PrintMaterial(glEnv, i, -1);
  }
}

/// Print the surface data for a material x of the material set, withdrawn from
///   the materialset loaded into LPFG.
///   All of this data goes into a second .mtl file that must be brought around
///   with the .obj file in order to operate.
void ObjOutputStore::PrintMaterial(GLEnv &glEnv, int c, int t) {
  if (t == -1) {
    _mtl << "newmtl s" << std::setfill('0') << std::setw(3) << c << "\n";
    _mtl << std::setfill(' ') << std::setw(0);

    MaterialSet *ms = glEnv.GetMaterialSet();

    //  For Materials.
    if (ms->Set()) {
      _mtl << "Ka " << ms->Ambient(c)[0] * ms->Ambient(c)[3] << " "
           << ms->Ambient(c)[1] * ms->Ambient(c)[3] << " "
           << ms->Ambient(c)[2] * ms->Ambient(c)[3] << "\n";
      _mtl << "Kd " << ms->Diffuse(c)[0] * ms->Diffuse(c)[3] << " "
           << ms->Diffuse(c)[1] * ms->Diffuse(c)[3] << " "
           << ms->Diffuse(c)[2] * ms->Diffuse(c)[3] << "\n";
      _mtl << "Ks " << ms->Specular(c)[0] * ms->Specular(c)[3] << " "
           << ms->Specular(c)[1] * ms->Specular(c)[3] << " "
           << ms->Specular(c)[2] * ms->Specular(c)[3] << "\n";
      _mtl << "Ns " << ms->Shininess(c) << "\n";

      if (ms->Transparency(c) < 1.0f) {
        _mtl << std::setw(13) << " ";
        _mtl << "d " << (1.0f - ms->Transparency(c)) << "\n";
      }
    }
    //  For Colormaps.
    else {
      _mtl << "Ka " << 0 << " " << 0 << " " << 0 << "\n";

      _mtl << std::setfill(' ') << std::setw(13) << " ";
      _mtl << "Kd " << glEnv.GetSolidColor(c).X() << " "
           << glEnv.GetSolidColor(c).Y() << " " << glEnv.GetSolidColor(c).Z()
           << "\n";

      _mtl << std::setw(13) << " ";
      _mtl << "Ks " << 0 << " " << 0 << " " << 0 << "\n";

      _mtl << std::setw(13) << " ";
      _mtl << "Ns " << 1 << "\n";
    }
    _mtl << "illum 1\n";
  } else {
    bool foundExisting = false;
    for (size_t i = 0; i < mtPairs.size(); i++) {
      if (mtPairs[i].first == c && mtPairs[i].second == t) {
        foundExisting = true;
        break;
      }
    }
    if (!foundExisting) {
      mtPairs.push_back(std::make_pair(c, t));

      _mtl << "newmtl s" << std::setfill('0') << std::setw(3)
           << (255 + mtPairs.size()) << "\n";
      _mtl << std::setfill(' ') << std::setw(0);

      MaterialSet *ms = glEnv.GetMaterialSet();

      _mtl << "Ka " << ms->Ambient(c)[0] * ms->Ambient(c)[3] << " "
           << ms->Ambient(c)[1] * ms->Ambient(c)[3] << " "
           << ms->Ambient(c)[2] * ms->Ambient(c)[3] << "\n";
      _mtl << "Kd " << ms->Diffuse(c)[0] * ms->Diffuse(c)[3] << " "
           << ms->Diffuse(c)[1] * ms->Diffuse(c)[3] << " "
           << ms->Diffuse(c)[2] * ms->Diffuse(c)[3] << "\n";
      _mtl << "Ks " << ms->Specular(c)[0] * ms->Specular(c)[3] << " "
           << ms->Specular(c)[1] * ms->Specular(c)[3] << " "
           << ms->Specular(c)[2] * ms->Specular(c)[3] << "\n";
      _mtl << "Ns " << ms->Shininess(c) << "\n";
      _mtl << "map_Kd " << textures.getFilename(t) << "\n";

      if (ms->Transparency(c) < 1.0f) {
        _mtl << std::setw(13) << " ";
        _mtl << "d " << (1.0f - ms->Transparency(c)) << "\n";
      }
    }
  }
}

size_t ObjOutputStore::Vertex(Vector3d v, Vector3d vt) {
  size_t res = _Find(v, _vertexArr, _precision);
  if (res == static_cast<size_t>(-1)) {
    _trg << "v" << ' ' << v.X() << ' ' << v.Y() << ' ' << v.Z() << std::endl;
    _trg << "vt" << ' ' << vt.X() << ' ' << vt.Y() << std::endl;
    _vertexArr.push_back(v);
    _texCoordArr.push_back(vt);
    return _vertexArr.size();
  } else {
    return res;
  }
}

std::pair<size_t, size_t> ObjOutputStore::VertexTexCoord(Vector3d v,
                                                         Vector3d vt) {
  size_t res = _Find(v, _vertexArr, _precision);
  if (res == static_cast<size_t>(-1)) {
    _trg << "v" << ' ' << v.X() << ' ' << v.Y() << ' ' << v.Z() << std::endl;
    _trg << "vt" << ' ' << vt.X() << ' ' << vt.Y() << std::endl;
    _vertexArr.push_back(v);
    _texCoordArr.push_back(vt);
    return std::make_pair(_vertexArr.size(), _texCoordArr.size());
  } else {
    size_t res2 = _Find(vt, _texCoordArr, _precision);
    if (res2 == static_cast<size_t>(-1)) {
      _trg << "vt" << ' ' << vt.X() << ' ' << vt.Y() << std::endl;
      _texCoordArr.push_back(vt);
      return std::make_pair(res, _texCoordArr.size());
    } else {
      return std::make_pair(res, res2);
    }
  }
}

size_t ObjOutputStore::Normal(Vector3d v) {
  return _Element(v, _normalArr, _precision, "vn");
}

size_t ObjOutputStore::_Element(Vector3d v, std::vector<Vector3d> &arr,
                                float precision, const char *lbl) {
  size_t res = _Find(v, arr, precision);
  if (res == static_cast<size_t>(-1)) {
    _trg << lbl << ' ' << v.X() << ' ' << v.Y() << ' ' << v.Z() << std::endl;
    arr.push_back(v);
    return arr.size();
  } else
    return res;
}

size_t ObjOutputStore::_Find(Vector3d v, const std::vector<Vector3d> &arr,
                             float precision) const {
  typedef std::vector<Vector3d>::const_iterator citer;
  const int MaxTries = 128;
  citer b = arr.begin();
  citer it = arr.end();
  if (b == it)
    return static_cast<size_t>(-1);
  --it;
  int tries = 0;
  for (;;) {
    if (Distance(v, *it) < precision)
      return it - b + 1;
    if (b == it)
      break;
    if (tries == MaxTries)
      break;
    ++tries;
    --it;
  }
  return static_cast<size_t>(-1);
}

void ObjOutputStore::Triangle(size_t v1, size_t v2, size_t v3, int color,
                              int texture) {
  PrintMaterialUse(color, texture);
  _trg << "f " << v1 << ' ' << v2 << ' ' << v3 << std::endl;
}

void ObjOutputStore::Triangle(size_t v1, size_t n1, size_t v2, size_t n2,
                              size_t v3, size_t n3, int color, int texture) {
  PrintMaterialUse(color, texture);
  _trg << "f " << v1 << "/" << v1 << "/" << n1 << ' ' << v2 << "/" << v2 << "/"
       << n2 << ' ' << v3 << "/" << v3 << "/" << n3 << std::endl;
}

void ObjOutputStore::Triangle(size_t v1, size_t n1, size_t t1, size_t v2,
                              size_t n2, size_t t2, size_t v3, size_t n3,
                              size_t t3, int color, int texture) {
  PrintMaterialUse(color, texture);
  _trg << "f " << v1 << "/" << t1 << "/" << n1 << ' ' << v2 << "/" << t2 << "/"
       << n2 << ' ' << v3 << "/" << t3 << "/" << n3 << std::endl;
}

void ObjOutputStore::Quad(size_t v1, size_t v2, size_t v3, size_t v4, int color,
                          int texture) {
  PrintMaterialUse(color, texture);
  _trg << "f " << v1 << ' ' << v2 << ' ' << v3 << ' ' << v4 << std::endl;
}

void ObjOutputStore::Quad(size_t v1, size_t n1, size_t v2, size_t n2, size_t v3,
                          size_t n3, size_t v4, size_t n4, int color,
                          int texture) {
  PrintMaterialUse(color, texture);
  _trg << "f " << v1 << "/" << v1 << "/" << n1 << ' ' << v2 << "/" << v2 << "/"
       << n2 << ' ' << v3 << "/" << v3 << "/" << n3 << ' ' << v4 << "/" << v4
       << "/" << n4 << std::endl;
}

void ObjOutputStore::Quad(size_t v1, size_t n1, size_t t1,
                          size_t v2, size_t n2, size_t t2,
                          size_t v3, size_t n3, size_t t3,
                          size_t v4, size_t n4, size_t t4,
                          int color, int texture) {
  PrintMaterialUse(color, texture);

    _trg << "f " << v1 << "/" << t1 << "/" << n1 << ' '
                 << v2 << "/" << t2 << "/" << n2 << ' '
                 << v3 << "/" << t3 << "/" << n3 << ' '
                 << v4 << "/" << t4 << "/" << n4 << std::endl;
}

void ObjOutputStore::Polygon(std::vector<size_t> v, int color, int texture) {
  PrintMaterialUse(color, texture);
  _trg << "f";
  for (size_t i = 0; i < v.size(); i++) {
    _trg << ' ' << v[i];
  }
  _trg << std::endl;
}

void ObjOutputStore::PrintMaterialUse(int color, int texture) {
  // to stop the same "usemtl sxxx" being printed, save last material index
  static int last_color = -1;
  static int last_texture = -1;
  if (color != last_color || texture != last_texture) {
    last_color = color;
    last_texture = texture;

    // if no texture is specified, use existing material
    if (texture == -1) {
      _trg << "usemtl s" << std::setfill('0') << std::setw(3) << color << "\n";
      _trg << std::setfill(' ') << std::setw(0);
    } else {
      // add new material with texture (unless it already exists?!)
      PrintMaterial(_glEnv, color, texture);
      for (size_t i = 0; i < mtPairs.size(); i++) {
        if (mtPairs[i].first == color && mtPairs[i].second == texture) {
          _trg << "usemtl s" << std::setfill('0') << std::setw(3) << 256 + i
               << "\n";
          _trg << std::setfill(' ') << std::setw(0);
          break;
        }
      }
    }
  }
}

void ObjOutputStore::StartLine() { _lnv.clear(); }

void ObjOutputStore::EndLine() {
  _trg << "f ";
  for (std::vector<size_t>::const_iterator it = _lnv.begin(); it != _lnv.end();
       ++it)
    _trg << *it << ' ';
  _trg << std::endl;
}

void ObjOutputStore::LinePnt(Vector3d v) {
  _lnv.push_back(Vertex(v, Vector3d(0, 0, 0)));
}

void ObjOutputStore::NewGroup() {
  _trg << "g group" << _groupId << std::endl;
  ++_groupId;
}

ObjOutputStore::QuadStripObj::QuadStripObj(ObjOutputStore &trg)
    : _trg(trg), _curNormal(1), _counter(0) {}

void ObjOutputStore::QuadStripObj::Normal(Vector3d v) {
  _curNormal = _trg.Normal(v);
}

void ObjOutputStore::QuadStripObj::Vertex(Vector3d v, Vector3d vt, int color,
                                          int texture) {
  _nrm[_counter] = _curNormal;
  _vrt[_counter] = _trg.Vertex(v, vt);
  ++_counter;
  if (4 == _counter) {
    _trg.Quad(_vrt[0], _nrm[0], _vrt[2], _nrm[2], _vrt[3], _nrm[3], _vrt[1],
              _nrm[1], color, texture);
    _counter = 2;
    _vrt[0] = _vrt[2];
    _nrm[0] = _nrm[2];
    _vrt[1] = _vrt[3];
    _nrm[1] = _nrm[3];
  }
}

ObjOutputStore::TriangleFanObj::TriangleFanObj(ObjOutputStore &trg)
    : _trg(trg), _curNormal(1), _counter(0) {}

void ObjOutputStore::TriangleFanObj::Normal(Vector3d v) {
  _curNormal = _trg.Normal(v);
}

void ObjOutputStore::TriangleFanObj::Vertex(Vector3d v, Vector3d vt, int color,
                                            int texture) {
  _nrm[_counter] = _curNormal;
  _vrt[_counter] = _trg.Vertex(v, vt);
  ++_counter;
  if (3 == _counter) {
    _trg.Triangle(_vrt[0], _nrm[0], _vrt[1], _nrm[1], _vrt[2], _nrm[2], color,
                  texture);
    _counter = 2;
    _vrt[1] = _vrt[2];
    _nrm[1] = _nrm[2];
  }
}
