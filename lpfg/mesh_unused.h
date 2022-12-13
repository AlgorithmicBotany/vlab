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



#ifndef __MESH_H__
#define __MESH_H__

//#define USE_MESH

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include <list>

#include "vector3d.h"

class Mesh {
public:
  Mesh() { NextRow(); }
  void Clear() {
    _mesh.clear();
    NextRow();
  }
  void NextRow() { _mesh.push_back(_MeshRow()); }
  void AddPoint(Vector3d, int);
  void DrawLines() const { _IteratePoints(false); }
  void DrawPolygons() const { _IteratePoints(true); }

private:
  typedef std::pair<const Vector3d, const int> _MeshPointData;
  typedef std::list<_MeshPointData> _MeshRow;
  typedef std::list<_MeshRow> _Mesh;

  _Mesh _mesh;

  void _IteratePoints(bool filled) const;
  void _PlaceNormal(const Vector3d &, const Vector3d &, const Vector3d &) const;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
