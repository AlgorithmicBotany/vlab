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



#ifndef __POLYGON_H__
#define __POLYGON_H__
#define GL_SILENCE_DEPRECATION

#include <vector>

#include "vector3d.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#ifdef WIN32
#include <GL/glew.h>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#else
#include <GL/glut.h>
#endif
#endif

namespace GLDraw {

class Polygon {
public:
  Polygon();
  ~Polygon();
  void Start();
  void End();
  bool Started() const { return _started; }
  void AddVertex(Vector3d, Vector3d, int);
  bool Full() const { return false; }
  void Suspend() { _started = false; }
  void Resume() { _started = true; }

private:
  Vector3d calculateAutomaticNormals();
  std::vector<Vector3d> _p, _n;
  std::vector<int> _c;
  int _count;
  bool _started;
  GLUtesselator *tess;
};

} // namespace GLDraw

// wrapper for GLUtesselator to output vertices instead of rendering
namespace GLTesselate {

class Polygon {

public:
  Polygon();
  ~Polygon();
  const std::vector<Vector3d> & tesselate(std::vector<Vector3d> &vertices);

private:
  GLUtesselator *tess;
};

}

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
