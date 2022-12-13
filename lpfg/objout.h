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



#ifndef __OBJOUTPUT_H__
#define __OBJOUTPUT_H__

#include <vector>
#include <fstream>
#include <utility>

#include "volume.h"
#include "glenv.h"

class ObjOutputStore {
public:
  ObjOutputStore(std::string, GLEnv &glEnv, const Volume &);
  void PrintMaterialUse(int color, int texture);
  size_t Vertex(Vector3d v, Vector3d vt);
  std::pair<size_t, size_t> VertexTexCoord(Vector3d v, Vector3d vt);
  size_t Normal(Vector3d);
  void Triangle(size_t, size_t, size_t, int color, int texture);
  void Triangle(size_t, size_t, size_t, size_t, size_t, size_t, int color,
                int texture);
  void Triangle(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
                size_t, int color, int texture);
  void Quad(size_t, size_t, size_t, size_t, int color, int texture);
  void Quad(size_t, size_t, size_t, size_t, size_t, size_t, size_t, size_t,
            int color, int texture);
  void Quad(size_t v1, size_t n1, size_t t1,
            size_t v2, size_t n2, size_t t2,
            size_t v3, size_t n3, size_t t3,
            size_t v4, size_t n4, size_t t4,
            int color, int texture);
  void Polygon(std::vector<size_t>, int color, int texture);
  void StartLine();
  void EndLine();
  void LinePnt(Vector3d);
  void NewGroup();
  class QuadStripObj {
  public:
    QuadStripObj(ObjOutputStore &);
    void Normal(Vector3d);
    void Vertex(Vector3d, Vector3d, int, int);

  private:
    ObjOutputStore &_trg;
    size_t _curNormal;
    size_t _nrm[4];
    size_t _vrt[4];
    int _counter;
  };
  class TriangleFanObj {
  public:
    TriangleFanObj(ObjOutputStore &);
    void Normal(Vector3d);
    void Vertex(Vector3d, Vector3d, int, int);

  private:
    ObjOutputStore &_trg;
    size_t _curNormal;
    size_t _nrm[3];
    size_t _vrt[3];
    int _counter;
  };

private:
  void PrintMaterial(GLEnv &glEnv, int c, int t);
  size_t _Element(Vector3d, std::vector<Vector3d> &, float, const char *);
  size_t _Find(Vector3d, const std::vector<Vector3d> &, float) const;
  std::ofstream _trg;
  std::ofstream _mtl;
  std::vector<std::pair<int, int>>
      mtPairs; // A list of pairings of materials and textures.
  const Volume &_v;
  const float _precision;
  std::vector<Vector3d> _vertexArr;
  std::vector<Vector3d> _normalArr;
  std::vector<Vector3d> _texCoordArr;
  std::vector<size_t> _lnv;
  int _groupId;
  GLEnv &_glEnv;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
