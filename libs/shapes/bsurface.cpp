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



#include <fstream>

#ifdef _WINDOWS
#include <GL/gl.h>
#include <windows.h>
#else
#include <qgl.h>
#endif
#include <cmath>
#include <iostream>

// for Windows
#ifdef _WINDOWS
#include <float.h>
#define isinf(x) (_finite(x) == 0)
#define isnan _isnan
#ifndef M_PI
#define M_PI 3.14159265358979
#endif
#endif

// for OSX
#ifdef VLAB_MACX
using std::isinf;
using std::isnan;
#endif

#include "bsurface.h"

using std::ifstream;
using std::ofstream;
#define C_SEG 20
#define C_SCALE 0.01
#define d_step 0.01
#define l_step 0.1
#define MRES_OFF 0.01
#define MIN_HEIGHT 10
#define MIN_WIDTH 10

#define err_tol 0.000001
#define max_it 20
#define inner_loop_max_it 100

namespace Shapes {

void BSplineSurface::Load(const char *fnm, double scale, int TextureId,
                          int divU, int divV) {
  _TextureId = TextureId;
  _scale = scale;
  _divU = divU;
  _divV = divV;
  LoadPatch(fnm);
  precomputed = false;
}

void BSplineSurface::Reread() {
  _divU = _divV = -1;
  LoadPatch(_fname.c_str());
  precomputed = false;
}

void BSplineSurface::BoundingBox(double &x1, double &y1, double &z1, double &x2,
                                 double &y2, double &z2) {
  x1 = x2 = Control_points[0][0][0][0];
  y1 = y2 = Control_points[0][0][0][1];
  z1 = z2 = Control_points[0][0][0][2];

  for (unsigned int i = 0; i < Control_points[0].size(); i++)
    for (unsigned int j = 0; j < Control_points[0][0].size(); j++) {

      if (Control_points[0][i][j][0] < x1)
        x1 = Control_points[0][i][j][0];

      if (Control_points[0][i][j][1] < y1)
        y1 = Control_points[0][i][j][1];

      if (Control_points[0][i][j][2] < z1)
        z1 = Control_points[0][i][j][2];

      if (Control_points[0][i][j][0] > x2)
        x2 = Control_points[0][i][j][0];

      if (Control_points[0][i][j][1] > y2)
        y2 = Control_points[0][i][j][1];

      if (Control_points[0][i][j][2] > z2)
        z2 = Control_points[0][i][j][2];
    }
}

void BSplineSurface::Transform(Matrix M, double x, double y, double z) {
  Point3d D;
  D[0] = x;
  D[1] = y;
  D[2] = z;

  for (unsigned int i = 0; i < Control_points[0].size(); i++)
    for (unsigned int j = 0; j < Control_points[0][0].size(); j++) {

      Control_points[0][i][j] = M * (Control_points[0][i][j]);
      Control_points[0][i][j] = Control_points[0][i][j] + D;
    }

  precomputed = false;
}

void BSplineSurface::Draw(int uDiv, int vDiv) {
  if (!precomputed)
    Precompute(uDiv, vDiv);

  for (int i = 0; i < _divU; ++i) {
    const double u1 = 1.0 * i / uDiv;
    const double u2 = 1.0 * (i + 1) / uDiv;

    if (!s_mesh)
      glBegin(GL_TRIANGLES);

    for (int j = 0; j < _divV; ++j) {
      bool flip = false;
      int id1 = PtId(i, j);
      int id2 = PtId(i + 1, j);
      int id3 = PtId(i + 1, j + 1);
      int id4 = PtId(i, j + 1);
      const double v1 = 1.0 * j / vDiv;
      const double v2 = 1.0 * (j + 1) / vDiv;

      if (s_mesh)
        glBegin(GL_LINES);

      if (contraction) {
        if (psurf && _ctract[id4] < 0)
          glColor3f(0.8 * (-_ctract[id4] / (-c_min)) + 0.2, 0.2, 0.2);
        else
          glColor3f(0.2, 0.8 * (_ctract[id4] / (c_max)) + 0.2, 0.2);
      }

      glTexCoord2d(u1, v2);
      glNormal3d(_nrml[id4][0], _nrml[id4][1], _nrml[id4][2]);
      glVertex3d(_vrtx[id4][0], _vrtx[id4][1], _vrtx[id4][2]);

      if (contraction) {
        if (psurf && _ctract[id1] < 0)
          glColor3f(0.8 * (-_ctract[id1] / (-c_min)) + 0.2, 0.2, 0.2);
        else
          glColor3f(0.2, 0.8 * (_ctract[id1] / (c_max)) + 0.2, 0.2);
      }

      glTexCoord2d(u1, v1);
      glNormal3d(_nrml[id1][0], _nrml[id1][1], _nrml[id1][2]);
      glVertex3d(_vrtx[id1][0], _vrtx[id1][1], _vrtx[id1][2]);

      if (_nrml[id4] * _nrml[id2] > _nrml[id3] * _nrml[id1]) {

        if (contraction) {
          if (psurf && _ctract[id2] < 0)
            glColor3f(0.8 * (-_ctract[id2] / (-c_min)) + 0.2, 0.2, 0.2);
          else
            glColor3f(0.2, 0.8 * (_ctract[id2] / (c_max)) + 0.2, 0.2);
        }

        flip = true;
        glTexCoord2d(u2, v1);
        glNormal3d(_nrml[id2][0], _nrml[id2][1], _nrml[id2][2]);
        glVertex3d(_vrtx[id2][0], _vrtx[id2][1], _vrtx[id2][2]);

        if (s_mesh) {
          if (contraction) {
            if (psurf && _ctract[id4] < 0)
              glColor3f(0.8 * (-_ctract[id4] / (-c_min)) + 0.2, 0.2, 0.2);
            else
              glColor3f(0.2, 0.8 * (_ctract[id4] / (c_max)) + 0.2, 0.2);
          }

          glTexCoord2d(u1, v2);
          glNormal3d(_nrml[id4][0], _nrml[id4][1], _nrml[id4][2]);
          glVertex3d(_vrtx[id4][0], _vrtx[id4][1], _vrtx[id4][2]);
        }

        if (contraction) {
          if (psurf && _ctract[id4] < 0)
            glColor3f(0.8 * (-_ctract[id4] / (-c_min)) + 0.2, 0.2, 0.2);
          else
            glColor3f(0.2, 0.8 * (_ctract[id4] / (c_max)) + 0.2, 0.2);
        }

        glTexCoord2d(u1, v2);
        glNormal3d(_nrml[id4][0], _nrml[id4][1], _nrml[id4][2]);
        glVertex3d(_vrtx[id4][0], _vrtx[id4][1], _vrtx[id4][2]);
      } else {
        if (contraction) {
          if (psurf && _ctract[id3] < 0)
            glColor3f(0.8 * (-_ctract[id3] / (-c_min)) + 0.2, 0.2, 0.2);
          else
            glColor3f(0.2, 0.8 * (_ctract[id3] / (c_max)) + 0.2, 0.2);
        }

        glTexCoord2d(u2, v2);
        glNormal3d(_nrml[id3][0], _nrml[id3][1], _nrml[id3][2]);
        glVertex3d(_vrtx[id3][0], _vrtx[id3][1], _vrtx[id3][2]);

        if (s_mesh) {
          if (contraction) {
            if (psurf && _ctract[id4] < 0)
              glColor3f(0.8 * (-_ctract[id4] / (-c_min)) + 0.2, 0.2, 0.2);
            else
              glColor3f(0.2, 0.8 * (_ctract[id4] / (c_max)) + 0.2, 0.2);
          }

          glTexCoord2d(u1, v2);
          glNormal3d(_nrml[id4][0], _nrml[id4][1], _nrml[id4][2]);
          glVertex3d(_vrtx[id4][0], _vrtx[id4][1], _vrtx[id4][2]);
        }

        if (contraction) {
          if (psurf && _ctract[id1] < 0)
            glColor3f(0.8 * (-_ctract[id1] / (-c_min)) + 0.2, 0.2, 0.2);
          else
            glColor3f(0.2, 0.8 * (_ctract[id1] / (c_max)) + 0.2, 0.2);
        }

        glTexCoord2d(u1, v1);
        glNormal3d(_nrml[id1][0], _nrml[id1][1], _nrml[id1][2]);
        glVertex3d(_vrtx[id1][0], _vrtx[id1][1], _vrtx[id1][2]);
      }

      if (contraction) {
        if (psurf && _ctract[id2] < 0)
          glColor3f(0.8 * (-_ctract[id2] / (-c_min)) + 0.2, 0.2, 0.2);
        else
          glColor3f(0.2, 0.8 * (_ctract[id2] / (c_max)) + 0.2, 0.2);
      }

      glTexCoord2d(u2, v1);
      glNormal3d(_nrml[id2][0], _nrml[id2][1], _nrml[id2][2]);
      glVertex3d(_vrtx[id2][0], _vrtx[id2][1], _vrtx[id2][2]);

      if (contraction) {
        if (psurf && _ctract[id3] < 0)
          glColor3f(0.8 * (-_ctract[id3] / (-c_min)) + 0.2, 0.2, 0.2);
        else
          glColor3f(0.2, 0.8 * (_ctract[id3] / (c_max)) + 0.2, 0.2);
      }

      glTexCoord2d(u2, v2);
      glNormal3d(_nrml[id3][0], _nrml[id3][1], _nrml[id3][2]);
      glVertex3d(_vrtx[id3][0], _vrtx[id3][1], _vrtx[id3][2]);

      if (s_mesh) {
        if (flip) {
          if (contraction) {
            if (psurf && _ctract[id4] < 0)
              glColor3f(0.8 * (-_ctract[id4] / (-c_min)) + 0.2, 0.2, 0.2);
            else
              glColor3f(0.2, 0.8 * (_ctract[id4] / (c_max)) + 0.2, 0.2);
          }

          glTexCoord2d(u1, v2);
          glNormal3d(_nrml[id4][0], _nrml[id4][1], _nrml[id4][2]);
          glVertex3d(_vrtx[id4][0], _vrtx[id4][1], _vrtx[id4][2]);
        } else {
          if (contraction) {
            if (psurf && _ctract[id1] < 0)
              glColor3f(0.8 * (-_ctract[id1] / (-c_min)) + 0.2, 0.2, 0.2);
            else
              glColor3f(0.2, 0.8 * (_ctract[id1] / (c_max)) + 0.2, 0.2);
          }

          glTexCoord2d(u1, v1);
          glNormal3d(_nrml[id1][0], _nrml[id1][1], _nrml[id1][2]);
          glVertex3d(_vrtx[id1][0], _vrtx[id1][1], _vrtx[id1][2]);
        }
        glEnd();
      }
    }
    glEnd();
  }

  /*
  // draw normals
  {
  GLprimitive lns(GL_LINES);
  for (int i=0; i<_vrtx.size(); ++i)
  {
  Point3d v = _vrtx[i];
  glVertex3d(v[0],v[1],v[2]);
  v = v+_nrml[i]*0.2;
  glVertex3d(v[0],v[1],v[2]);
  }
  }
  */
}

void BSplineSurface::Rayshade(int uDiv, int vDiv, std::ofstream &target,
                              bool textured, std::string filename) {
  if (!precomputed)
    Precompute(uDiv, vDiv);

  for (int i = 0; i < _divU; i++) {
    const double u1 = 1.0 * i / uDiv;
    const double u2 = 1.0 * (i + 1) / uDiv;

    for (int j = 0; j < _divV; j++) {
      int id1 = PtId(i, j);
      int id2 = PtId(i + 1, j);
      int id3 = PtId(i + 1, j + 1);
      int id4 = PtId(i, j + 1);

      bool flip = (_nrml[id4] * _nrml[id2] > _nrml[id3] * _nrml[id1]);

      const double v1 = 1.0 * j / vDiv;
      const double v2 = 1.0 * (j + 1) / vDiv;

      //  First triangle.
      target << "triangle";
      if (textured)
        target << "uv";
      target << "\n";

      target << _vrtx[id4][0] << " " << _vrtx[id4][1] << " " << _vrtx[id4][2]
             << " " << _nrml[id4][0] << " " << _nrml[id4][1] << " "
             << _nrml[id4][2];
      if (textured)
        target << " " << u1 << " " << v2;
      target << "\n";

      if (flip) {
        target << _vrtx[id2][0] << " " << _vrtx[id2][1] << " " << _vrtx[id2][2]
               << " " << _nrml[id2][0] << " " << _nrml[id2][1] << " "
               << _nrml[id2][2];
        if (textured)
          target << " " << u2 << " " << v1;
        target << "\n";
      } else {
        target << _vrtx[id3][0] << " " << _vrtx[id3][1] << " " << _vrtx[id3][2]
               << " " << _nrml[id3][0] << " " << _nrml[id3][1] << " "
               << _nrml[id3][2];
        if (textured)
          target << " " << u2 << " " << v2;
        target << "\n";
      }

      target << _vrtx[id1][0] << " " << _vrtx[id1][1] << " " << _vrtx[id1][2]
             << " " << _nrml[id1][0] << " " << _nrml[id1][1] << " "
             << _nrml[id1][2];
      if (textured)
        target << " " << u1 << " " << v1;
      target << "\n";

      if (textured)
        target << "texture image " << filename << "\nmap uv\n";

      //  Second triangle.
      target << "triangle";
      if (textured)
        target << "uv";
      target << "\n";

      target << _vrtx[id2][0] << " " << _vrtx[id2][1] << " " << _vrtx[id2][2]
             << " " << _nrml[id2][0] << " " << _nrml[id2][1] << " "
             << _nrml[id2][2];
      if (textured)
        target << " " << u2 << " " << v1;
      target << "\n";

      if (flip) {
        target << _vrtx[id4][0] << " " << _vrtx[id4][1] << " " << _vrtx[id4][2]
               << " " << _nrml[id4][0] << " " << _nrml[id4][1] << " "
               << _nrml[id4][2];
        if (textured)
          target << " " << u1 << " " << v2;
        target << "\n";
      } else {
        target << _vrtx[id1][0] << " " << _vrtx[id1][1] << " " << _vrtx[id1][2]
               << " " << _nrml[id1][0] << " " << _nrml[id1][1] << " "
               << _nrml[id1][2];
        if (textured)
          target << " " << u1 << " " << v1;
        target << "\n";
      }

      target << _vrtx[id3][0] << " " << _vrtx[id3][1] << " " << _vrtx[id3][2]
             << " " << _nrml[id3][0] << " " << _nrml[id3][1] << " "
             << _nrml[id3][2];
      if (textured)
        target << " " << u2 << " " << v2;
      target << "\n";

      if (textured)
        target << "texture image " << filename << "\nmap uv\n";
    }
  }
}

void BSplineSurface::PostScript(int uDiv, int vDiv, std::string &target,
                                Vector3d clr) {
  if (!precomputed)
    Precompute(uDiv, vDiv);

  for (int i = 0; i < _divU; i++) {
    //  Will be needed if texturing.

    for (int j = 0; j < _divV; j++) {
      int id1 = PtId(i, j);
      int id2 = PtId(i + 1, j);
      int id3 = PtId(i + 1, j + 1);
      int id4 = PtId(i, j + 1);

      //  Will be needed if texturing.

      std::ostringstream ostr;

      //  First triangle.
      ostr << _vrtx[id4][0] << " " << _vrtx[id4][1] << " moveto" << std::endl;
      ostr << _vrtx[id2][0] << " " << _vrtx[id2][1] << " lineto" << std::endl;
      ostr << _vrtx[id1][0] << " " << _vrtx[id1][1] << " lineto" << std::endl;
      ostr << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor"
           << std::endl;
      ostr << "1.0 setlinewidth fill" << std::endl;

      //  Second triangle.
      ostr << _vrtx[id2][0] << " " << _vrtx[id2][1] << " moveto" << std::endl;
      ostr << _vrtx[id4][0] << " " << _vrtx[id4][1] << " lineto" << std::endl;
      ostr << _vrtx[id3][0] << " " << _vrtx[id3][1] << " lineto" << std::endl;
      ostr << clr.X() << " " << clr.Y() << " " << clr.Z() << " setrgbcolor"
           << std::endl;
      ostr << "1.0 setlinewidth fill" << std::endl;

      target.append(ostr.str());
    }
  }
}

void BSplineSurface::GetQuad(int i, int j, Vector3d *vlist) {
  int id1 = PtId(i, j);
  int id2 = PtId(i + 1, j);
  int id3 = PtId(i + 1, j + 1);
  int id4 = PtId(i, j + 1);

  Vector3d vct1(_vrtx[id1][0], _vrtx[id1][1], _vrtx[id1][2]);
  Vector3d vct2(_vrtx[id2][0], _vrtx[id2][1], _vrtx[id2][2]);
  Vector3d vct3(_vrtx[id3][0], _vrtx[id3][1], _vrtx[id3][2]);
  Vector3d vct4(_vrtx[id4][0], _vrtx[id4][1], _vrtx[id4][2]);

  vlist[0] = vct1;
  vlist[1] = vct2;
  vlist[2] = vct3;
  vlist[3] = vct4;
}

void BSplineSurface::Precompute(int uDiv, int vDiv) {
  unsigned int newsize = (uDiv + 1) * (vDiv + 1);
  if (_vrtx.size() != newsize) {
    _vrtx.resize(newsize);
    _nrml.resize(newsize);
    // This should only be resized if contraction is being visualized
    _ctract.resize(newsize);
  }

  if (_ctract.size() != newsize) {
    _ctract.resize(newsize);
  }
  _divU = uDiv;
  _divV = vDiv;

  const double du = 1.0 / uDiv * (n_u - degree);
  const double dv = 1.0 / vDiv * (n_v - degree);
  c_min = c_max = 0;
  for (int i = 0; i <= _divU; ++i) {
    const double u = 3.0 + i * du;
    for (int j = 0; j <= _divV; ++j) {
      const int id = PtId(i, j);
      const double v = 3.0 + j * dv;
      _vrtx[id] = Eval(u, v);
      _nrml[id] = NormalEval(u, v);
      if (psurf) {
        _ctract[id] = HasContracted(psurf, u, v);
        if (_ctract[id] < c_min)
          c_min = _ctract[id];
        if (_ctract[id] > c_max)
          c_max = _ctract[id];

      } else
        _ctract[id] = 0;
    }
  }
  if (c_max > fabs(c_min))
    c_min = -c_max;
  else
    c_max = -c_min;
  precomputed = true;
}

BSplineSurface::BSplineSurface() : _TextureId(-1) {
  degree = 3;
  wireframe = true;
  n_u = MIN_HEIGHT;
  n_v = MIN_WIDTH;
  selected[0] = selected[1] = selected_level = 0;
  periodic = false;
  Reset(n_u, n_v);
  lines = true;
  u_prec = l_step;
  v_prec = l_step;

  poly_line[0] = 1.0;
  poly_line[1] = 1.0;

  s_mesh = false;
  disp_normal = false;
  disp_c_mesh = true;
  contraction = false;
  precomputed = false;
  c_min = 0;
  c_max = 0;
  psurf = NULL;
}

BSplineSurface::~BSplineSurface() {}

BSplineSurface::BSplineSurface(const BSplineSurface &B) {
  _TextureId = B._TextureId;
  _scale = B._scale;
  _divU = B._divU;
  _divV = B._divV;
  if (B.precomputed) {
    _vrtx = B._vrtx;
    _nrml = B._nrml;
    precomputed = B.precomputed;
  } else
    precomputed = false;

  wireframe = true;

  degree = 3;
  wireframe = true;
  n_u = B.n_u;
  n_v = B.n_v;
  selected[0] = selected[1] = selected_level = 0;
  periodic = false;
  Reset(n_u, n_v);
  lines = true;
  contraction = B.contraction;
  psurf = B.psurf;

  for (unsigned int i = 0; i < Control_points[0].size(); i++)
    for (unsigned int j = 0; j < Control_points[0][0].size(); j++)
      Control_points[0][i][j] = B.Control_points[0][i][j];
}

bool BSplineSurface::SavePatch(const char *fname) {
  bool saved = false;

  ofstream File;

  File.open(fname);

  File << n_u << " ";

  File << n_v << std::endl;

  File << 3 << std::endl;

  for (int i = 0; i < n_u; i++) {
    for (int j = 0; j < n_v; j++) {

      File << Control_points[0][i][j][0] << " " << Control_points[0][i][j][1]
           << " " << Control_points[0][i][j][2] << std::endl;
    }
  }
  return saved;
}

bool BSplineSurface::LoadPatch(const char *fname) {
  bool loaded = false;
  int degree;
  ifstream File;

  File.open(fname);
  _f_name = fname;

  File >> n_u;
  File >> n_v;
  File >> degree;

  if (degree != 3)
    std::cout << "Warning: " << fname << " is not a cubic patch" << std::endl;

  Reset(n_u, n_v);

  for (int i = 0; i < n_u; i++) {
    for (int j = 0; j < n_v; j++) {

      File >> Control_points[0][i][j][0] >> Control_points[0][i][j][1] >>
          Control_points[0][i][j][2];
    }
  }
  return loaded;
}

void BSplineSurface::Reset(int u, int v) {
  vector<Point3d> *c_points;
  vector<vector<Point3d>> *cc_points;

  n_u = u;
  n_v = v;
  footprint.clear();
  int *cv = new int[4];
  footprint.push_back(cv);

  selected[0] = selected[1] = 0;
  selected_level = 0;
  selected_region.clear();
  // delete existing points
  Control_points.clear();

  Details.clear();

  cc_points = new vector<vector<Point3d>>;

  Point3d p;

  for (int i = 0; i < u; i++) {
    c_points = new vector<Point3d>;
    for (int j = 0; j < v; j++) {

      p[0] = (float)i / u - 0.5 + err_tol;
      p[1] = (float)j / v - 0.5 + err_tol;
      p[2] = 0;

      c_points->push_back(p);
    }
    cc_points->push_back(*c_points);
    delete c_points;
  }
  Control_points.push_back(*cc_points);

  delete cc_points;

}

float BSplineSurface::HasContracted(BSplineSurface *b, float u, float v) {

  Point3d pu = PartialU(u, v);
  Point3d bpu = b->PartialU(u, v);
  Point3d pv = PartialV(u, v);
  Point3d bpv = b->PartialV(u, v);

  pu = pu.Cross(pv);
  bpu = bpu.Cross(bpv);
  float ret = (pu.Length() / bpu.Length());
  ret -= 1;

  return ret;
}

void BSplineSurface::HalveU() {
  vector<Point3d> temp;

  Point3d val;

  vector<vector<Point3d>> c_points;
  vector<vector<Point3d>> d_points;
  int *cr = new int[4];

  if ((Control_points[Control_points.size() - 1].size() + 3) / 2 >=
      MIN_HEIGHT) {

    footprint.push_back(cr);
    val = val * 0;

    for (unsigned int j = 0;
         j < Control_points[Control_points.size() - 1][0].size(); j++)
      temp.push_back(val);

    for (unsigned int i = 0;
         i < (Control_points[Control_points.size() - 1].size() + 3) / 2; i++) {
      c_points.push_back(temp);
      d_points.push_back(temp);
    }

    Control_points.push_back(c_points);
    Details.push_back(d_points);

    for (unsigned int i = 0; i < footprint.size(); i++) {

      footprint[i][1] = footprint[i][3] = 10000000;
      footprint[i][0] = footprint[i][2] = -1;
    }
    footprint[footprint.size() - 2][1] = 0;
    footprint[footprint.size() - 2][0] =
        (int)Control_points[Control_points.size() - 2].size() - 1;
    footprint[footprint.size() - 2][3] = 0;
    footprint[footprint.size() - 2][2] =
        (int)Control_points[Control_points.size() - 2][0].size() - 1;

    footprint[footprint.size() - 1][1] = 0;
    footprint[footprint.size() - 1][0] =
        (int)Control_points[Control_points.size() - 1].size() - 1;
    footprint[footprint.size() - 1][3] = 0;
    footprint[footprint.size() - 1][2] =
        (int)Control_points[Control_points.size() - 1][0].size() - 1;

    UpdateFootprint();
  }
}

void BSplineSurface::DoubleV() {
  vector<Point3d> temp;

  Point3d val;
  int line_size;
  vector<vector<Point3d>> c_points;
  vector<vector<Point3d>> d_points;
  int *cr = new int[4];

  footprint.push_back(cr);

  for (unsigned int k = 0; k < Control_points[0].size(); k++) {

    temp.clear();

    temp.push_back(Control_points[0][k][0]);

    val = Control_points[0][k][0] * (1.0 / 2.0) +
          Control_points[0][k][1] * (1.0 / 2.0);

    temp.push_back(val);

    val = Control_points[0][k][1] * (3.0 / 4.0) +
          Control_points[0][k][2] * (1.0 / 4.0);

    temp.push_back(val);

    val = Control_points[0][k][1] * (3.0 / 16.0) +
          Control_points[0][k][2] * (11.0 / 16.0) +
          Control_points[0][k][3] * (1.0 / 8.0);

    temp.push_back(val);

    val = Control_points[0][k][2] * (1.0 / 2.0) +
          Control_points[0][k][3] * (1.0 / 2.0);

    temp.push_back(val);

    line_size = (int)Control_points[0][k].size() - 1;

    for (int i = 3; i < line_size - 2; i++) {
      val = Control_points[0][k][i - 1] * (1.0 / 8.0) +
            Control_points[0][k][i] * (3.0 / 4.0) +
            Control_points[0][k][i + 1] * (1.0 / 8.0);

      temp.push_back(val);

      // Changed here and in doubleU
      val = Control_points[0][k][i] * (1.0 / 2.0) +
            Control_points[0][k][i + 1] * (1.0 / 2.0);
      temp.push_back(val);

    }

    val = Control_points[0][k][line_size - 1] * (3.0 / 16.0) +
          Control_points[0][k][line_size - 2] * (11.0 / 16.0) +
          Control_points[0][k][line_size - 3] * (1.0 / 8.0);

    temp.push_back(val);

    val = Control_points[0][k][line_size - 2] * (1.0 / 4.0) +
          Control_points[0][k][line_size - 1] * (3.0 / 4.0);

    temp.push_back(val);

    val = Control_points[0][k][line_size] * (1.0 / 2.0) +
          Control_points[0][k][line_size - 1] * (1.0 / 2.0);

    temp.push_back(val);

    temp.push_back(Control_points[0][k][line_size]);

    c_points.push_back(temp);
  }
  Control_points.insert(Control_points.begin(), c_points);
  n_v = (int)Control_points[0][0].size();
  n_u = (int)Control_points[0].size();
  temp.clear();
  val = val * 0;

  for (int i = 0; i < n_v; i++)
    temp.push_back(val);

  for (int i = 0; i < n_u; i++)
    d_points.push_back(temp);

  Details.insert(Details.begin(), d_points);

  for (unsigned int i = 0; i < footprint.size(); i++) {

    footprint[i][1] = footprint[i][3] = 10000000;
    footprint[i][0] = footprint[i][2] = -1;
  }
  CalcPreImage(selected[0], selected[1], selected_level);
}

void BSplineSurface::HalveV() {
  vector<Point3d> temp;

  Point3d val;

  vector<vector<Point3d>> c_points;
  vector<vector<Point3d>> d_points;
  int *cr = new int[4];

  if ((Control_points[Control_points.size() - 1][0].size() + 3) / 2 >=
      MIN_WIDTH) {
    footprint.push_back(cr);
    val = val * 0;

    for (unsigned int i = 0;
         i < (Control_points[Control_points.size() - 1][0].size() + 3) / 2; i++)
      temp.push_back(val);

    for (unsigned int j = 0;
         j < Control_points[Control_points.size() - 1].size(); j++) {
      c_points.push_back(temp);
      d_points.push_back(temp);
    }

    Control_points.push_back(c_points);
    Details.push_back(d_points);

    for (unsigned int i = 0; i < footprint.size(); i++) {
      footprint[i][1] = footprint[i][3] = 10000000;
      footprint[i][0] = footprint[i][2] = -1;
    }

    footprint[footprint.size() - 2][1] = 0;
    footprint[footprint.size() - 2][0] =
        (int)Control_points[Control_points.size() - 2].size() - 1;
    footprint[footprint.size() - 2][3] = 0;
    footprint[footprint.size() - 2][2] =
        (int)Control_points[Control_points.size() - 2][0].size() - 1;

    footprint[footprint.size() - 1][1] = 0;
    footprint[footprint.size() - 1][0] =
        (int)Control_points[Control_points.size() - 1].size() - 1;
    footprint[footprint.size() - 1][3] = 0;
    footprint[footprint.size() - 1][2] =
        (int)Control_points[Control_points.size() - 1][0].size() - 1;

    //  Calc_PreImage(0,0,Control_points.size()-2);
    UpdateFootprint();
  }
}

void BSplineSurface::DoubleU() {
  vector<Point3d> temp;
  vector<vector<Point3d>> temp_grid;
  vector<vector<Point3d>> c_points;
  vector<vector<Point3d>> d_points;
  int *cr = new int[4];

  footprint.push_back(cr);

  Point3d val;
  int line_size;

  for (unsigned int k = 0; k < Control_points[0][0].size(); k++) {

    temp.clear();

    temp.push_back(Control_points[0][0][k]);

    val = Control_points[0][0][k] * (1.0 / 2.0) +
          Control_points[0][1][k] * (1.0 / 2.0);

    temp.push_back(val);

    val = Control_points[0][1][k] * (3.0 / 4.0) +
          Control_points[0][2][k] * (1.0 / 4.0);

    temp.push_back(val);

    val = Control_points[0][1][k] * (3.0 / 16.0) +
          Control_points[0][2][k] * (11.0 / 16.0) +
          Control_points[0][3][k] * (1.0 / 8.0);

    temp.push_back(val);

    //    val=Control_points[0][2][k]*(1.0/2.0)
    //  +Control_points[0][3][k]*(1.0/2.0);

    // temp.push_back(val);

    line_size = (int)Control_points[0].size() - 1;

    for (int i = 2; i < line_size - 3; i++) {
      val = Control_points[0][i][k] * (1.0 / 2.0) +
            Control_points[0][i + 1][k] * (1.0 / 2.0);
      temp.push_back(val);

      val = Control_points[0][i][k] * (1.0 / 8.0) +
            Control_points[0][i + 1][k] * (3.0 / 4.0) +
            Control_points[0][i + 2][k] * (1.0 / 8.0);

      temp.push_back(val);

      //  if((i)<(line_size-2)) {
      //    }
    }
    val = Control_points[0][line_size - 2][k] * (1.0 / 2.0) +
          Control_points[0][line_size - 3][k] * (1.0 / 2.0);

    temp.push_back(val);

    val = Control_points[0][line_size - 1][k] * (3.0 / 16.0) +
          Control_points[0][line_size - 2][k] * (11.0 / 16.0) +
          Control_points[0][line_size - 3][k] * (1.0 / 8.0);

    temp.push_back(val);

    val = Control_points[0][line_size - 2][k] * (1.0 / 4.0) +
          Control_points[0][line_size - 1][k] * (3.0 / 4.0);

    temp.push_back(val);

    val = Control_points[0][line_size][k] * (1.0 / 2.0) +
          Control_points[0][line_size - 1][k] * (1.0 / 2.0);

    temp.push_back(val);

    temp.push_back(Control_points[0][line_size][k]);

    temp_grid.push_back(temp);
    //    Control_points[0][k]=temp;
  }

  // Control_points[0].clear();

  for (unsigned int i = 0; i < temp_grid[0].size(); i++) {
    temp.clear();
    for (unsigned int j = 0; j < temp_grid.size(); j++) {
      temp.push_back(temp_grid[j][i]);
    }
    c_points.push_back(temp);
  }

  Control_points.insert(Control_points.begin(), c_points);

  temp.clear();
  val = val * 0;

  n_v = (int)Control_points[0][0].size();
  n_u = (int)Control_points[0].size();

  for (int i = 0; i < n_v; i++)
    temp.push_back(val);

  for (int i = 0; i < n_u; i++)
    d_points.push_back(temp);

  Details.insert(Details.begin(), d_points);

  for (unsigned int i = 0; i < footprint.size(); i++) {
    footprint[i][1] = footprint[i][3] = 10000000;
    footprint[i][0] = footprint[i][2] = -1;
  }
  CalcPreImage(selected[0], selected[1], selected_level);
}

void BSplineSurface::DisplayControlPoints(Matrix A, Point3d trans) {
  Point3d t;
  glColor3dv(unsel_point);
  int k = selected_level;

  for (unsigned int i = 0; i < Control_points[k].size(); i++) {
    for (unsigned int j = 0; j < Control_points[k][0].size(); j++) {
      if (k == selected_level) {
        t = Control_points[selected_level][i][j];
        t[2] = -t[2] + k * (MRES_OFF);
        t = A * t;
        t[2] = 0;
        t = t + trans;
        if (i != selected[0] || j != selected[1])
          DrawCircle(t[0], t[1], t[2], 0.7);
        // CallBackCircle(t[0],t[1],t[2],0.7);
        else {
          glColor3dv(sel_point);
          ;
          DrawCircle(t[0], t[1], t[2], 1.0);
          // CallBackCircle(t[0],t[1],t[2],1.0);
          glColor3dv(unsel_point);
        }
      } else {
        t = Control_points[k][i][j];
        t[2] = -t[2] + k * (MRES_OFF);
        t = A * t;
        t[2] = 0;
        t = t + trans;
        glColor3d(0.8 * (k / Control_points.size()),
                  0.8 * (k / Control_points.size()),
                  0.8 * (k / Control_points.size()));
        DrawCircle(t[0], t[1], t[2], 0.3);
        // CallBackCircle(t[0],t[1],t[2],0.3);
        glColor3dv(unsel_point);
      }
    }
  }
  //  }

  for (unsigned int i = 0; i < selected_region.size(); i++) {
    t = Control_points[selected_level][selected_region[i][0]]
                      [selected_region[i][1]];
    t[2] = -t[2] + k * (MRES_OFF);
    t = A * t;
    t[2] = 0.01;
    t = t + trans;
    glColor3dv(sel_point);
    ;
    DrawCircle(t[0], t[1], t[2], 1.0);
    // CallBackCircle(t[0],t[1],t[2],0.7);
    glColor3dv(unsel_point);
  }

  if (disp_c_mesh) {

    glBegin(GL_LINES);
    for (unsigned int i = 0; i < Control_points[k].size(); i++) {
      for (unsigned int j = 0; j < Control_points[k][0].size(); j++) {
        if (i != Control_points[k].size() - 1) {
          t = Control_points[k][i][j];
          t[2] = -t[2] + k * (MRES_OFF);
          t = A * t;
          t[2] = 0;
          t = t + trans;
          glVertex3d(t[0], t[1], t[2]);

          t = Control_points[k][i + 1][j];
          t[2] = -t[2] + k * (MRES_OFF);
          t = A * t;
          t[2] = 0;
          t = t + trans;
          glVertex3d(t[0], t[1], t[2]);
        }
        if (j != Control_points[k][0].size() - 1) {
          t = Control_points[k][i][j];
          t[2] = -t[2] + k * (MRES_OFF);
          t = A * t;
          t[2] = 0;
          t = t + trans;
          glVertex3d(t[0], t[1], t[2]);

          t = Control_points[k][i][j + 1];
          t[2] = -t[2] + k * (MRES_OFF);
          t = A * t;
          t[2] = 0;
          t = t + trans;
          glVertex3d(t[0], t[1], t[2]);
        }
      }
    }
    glEnd();
  }
}

void BSplineSurface::DrawCircle(double x, double y, double z, double weight) {
  glBegin(GL_TRIANGLES);
  for (double i = 0; i < C_SEG; i++) {
    glVertex3d(x + sqrt(weight) * C_SCALE * cos(2 * 3.14159 * (i / C_SEG)),
               y + sqrt(weight) * C_SCALE * sin(2 * 3.14159 * (i / C_SEG)), z);
    glVertex3d(
        x + sqrt(weight) * C_SCALE * cos(2 * 3.14159 * ((i + 1) / C_SEG)),
        y + sqrt(weight) * C_SCALE * sin(2 * 3.14159 * ((i + 1) / C_SEG)), z);
    glVertex3d(x, y, z);
  }
  glEnd();
}

/*
void BSplineSurface::CallBackCircle(double x, double y, double z, double weight)
{

  glTranslatef(-x,-y,-z);
  glScalef(weight,weight,weight);

  glCallList(circ);

  glScalef(1/weight,1/weight,1/weight);
 glTranslatef(x,y,z);


}
*/

void BSplineSurface::DisplaySurface() {
  Point3d u_val, u_val2, u_val3, u_val4;
  Point3d n_val, n_val2, n_val3, n_val4;
  glLineWidth(1.0);

  n_u = (int)Control_points[0].size();
  n_v = (int)Control_points[0][0].size();

  if (wireframe) {
    if (n_u > 1) {

      glColor3dv(poly_line);

      for (double i = 3; i <= (n_u + d_step) && n_u > 1;
           i += u_prec * ((double)n_u - (double)degree)) {
        glBegin(GL_LINE_STRIP);
        for (double j = 3; j <= (n_v + d_step) && n_v > 1;
             j += d_step * ((double)n_v - (double)degree)) {
          if (contraction) {
            if (psurf && HasContracted(psurf, i, j))
              glColor3f(0.5, 0.0, 0.0);
            else
              glColor3f(0.0, 0.5, 0.0);
          }

          u_val = Eval(i, j);
          glVertex3d(u_val[0], u_val[1], u_val[2]);
        }
        glEnd();
      }

      for (double i = 3; i <= (n_v + d_step) && n_v > 1;
           i += v_prec * ((double)n_v - (double)degree)) {
        glBegin(GL_LINE_STRIP);
        for (double j = 3; j <= (n_u + d_step) && n_u > 1;
             j += d_step * ((double)n_u - (double)degree)) {
          if (contraction) {
            if (psurf && HasContracted(psurf, i, j))
              glColor3f(0.5, 0.0, 0.0);
            else
              glColor3f(0.0, 0.5, 0.0);
          }

          u_val = Eval(j, i);
          glVertex3d(u_val[0], u_val[1], u_val[2]);
        }
        glEnd();
      }
    }
  } else {
    if (n_u > 1) {
      if (s_mesh)
        glBegin(GL_TRIANGLES);

      for (double i = 3; i <= (n_u) && n_u > 1;
           i += u_prec / 5.0 * ((double)n_u - (double)degree)) {
        for (double j = 3; j <= (n_v) && n_v > 1;
             j += v_prec / 5.0 * ((double)n_v - (double)degree)) {
          u_val = Eval(i, j);
          n_val = NormalEval(i, j);

          u_val2 = Eval(i + u_prec / 5.0 * (n_u - degree), j);
          n_val2 = NormalEval(i + u_prec / 5.0 * (n_u - degree), j);

          n_val3 = NormalEval(i, j + v_prec / 5.0 * (n_v - degree));
          u_val3 = Eval(i, j + v_prec / 5.0 * (n_v - degree));

          u_val4 = Eval(i + u_prec / 5.0 * (n_u - degree),
                        j + v_prec / 5.0 * (n_v - degree));
          n_val4 = NormalEval(i + u_prec / 5.0 * (n_u - degree),
                              j + v_prec / 5.0 * (n_v - degree));

          if (contraction) {
            if (psurf && HasContracted(psurf, i, j))
              glColor3f(0.5, 0.0, 0.0);
            else
              glColor3f(0.0, 0.5, 0.0);
          }

          if (n_val2 * n_val3 > n_val * n_val4) {
            if (!s_mesh)
              glBegin(GL_LINE_STRIP);

            glNormal3d(n_val[0], n_val[1], n_val[2]);
            glVertex3d(u_val[0], u_val[1], u_val[2]);

            glNormal3d(n_val2[0], n_val2[1], n_val2[2]);
            glVertex3d(u_val2[0], u_val2[1], u_val2[2]);

            glNormal3d(n_val3[0], n_val3[1], n_val3[2]);
            glVertex3d(u_val3[0], u_val3[1], u_val3[2]);

            if (!s_mesh) {
              glNormal3d(n_val[0], n_val[1], n_val[2]);
              glVertex3d(u_val[0], u_val[1], u_val[2]);
            }

            if (!s_mesh) {
              glEnd();
              glBegin(GL_LINE_STRIP);
            }
            glNormal3d(n_val4[0], n_val4[1], n_val4[2]);
            glVertex3d(u_val4[0], u_val4[1], u_val4[2]);
	    glNormal3d(n_val3[0], n_val3[1], n_val3[2]);
            glVertex3d(u_val3[0], u_val3[1], u_val3[2]);

	    glNormal3d(n_val2[0], n_val2[1], n_val2[2]);
            glVertex3d(u_val2[0], u_val2[1], u_val2[2]);

            if (!s_mesh) {
              glNormal3d(n_val4[0], n_val4[1], n_val4[2]);
              glVertex3d(u_val4[0], u_val4[1], u_val4[2]);
              glEnd();
            }
          } else {
            if (!s_mesh) {
              glBegin(GL_LINE_STRIP);
            }

            glNormal3d(n_val[0], n_val[1], n_val[2]);
            glVertex3d(u_val[0], u_val[1], u_val[2]);

            glNormal3d(n_val2[0], n_val2[1], n_val2[2]);
            glVertex3d(u_val2[0], u_val2[1], u_val2[2]);

            glNormal3d(n_val4[0], n_val4[1], n_val4[2]);
            glVertex3d(u_val4[0], u_val4[1], u_val4[2]);

            if (!s_mesh) {
              glNormal3d(n_val[0], n_val[1], n_val[2]);
              glVertex3d(u_val[0], u_val[1], u_val[2]);
              glEnd();
              glBegin(GL_LINE_STRIP);
            }

            glNormal3d(n_val4[0], n_val4[1], n_val4[2]);
            glVertex3d(u_val4[0], u_val4[1], u_val4[2]);

            glNormal3d(n_val3[0], n_val3[1], n_val3[2]);
            glVertex3d(u_val3[0], u_val3[1], u_val3[2]);

            glNormal3d(n_val[0], n_val[1], n_val[2]);
            glVertex3d(u_val[0], u_val[1], u_val[2]);

            if (!s_mesh) {
              glNormal3d(n_val4[0], n_val4[1], n_val4[2]);
              glVertex3d(u_val4[0], u_val4[1], u_val4[2]);
              glEnd();
            }
          }
        }
      }
      glEnd();
    }
  }

  glDisable(GL_LIGHTING);
  if (disp_normal) {
    glBegin(GL_LINES);

    for (double i = 1;
         i < (n_u + degree - 1 - u_prec / 2.0 * (n_u + degree)) && n_u > 1;
         i += u_prec / 2.0 * ((double)n_u - (double)degree)) {
      for (double j = 1;
           j < (n_v + degree - 1 - v_prec / 2.0 * (n_v + degree)) && n_v > 1;
           j += v_prec / 2.0 * ((double)n_v + (double)degree)) {

        Point3d pos;
        Point3d nrml;

        pos = Eval(i, j);
        glColor3f(1.0, 0.0, 0.0);
        nrml = NormalEval(i, j);

        if (i == n_u && j == n_v)
          glColor3f(0.0, 1.0, 0.0);
        glVertex3d(pos[0], pos[1], pos[2]);
        pos = pos + nrml * 0.05;

        glVertex3d(pos[0], pos[1], pos[2]);
      }
    }
    glEnd();
  }
  glEnable(GL_LIGHTING);

  glColor3dv(sel_point);
  glLineWidth(3.0);

  glLineWidth(1.0);
  n_u = (int)Control_points[0].size();
  n_v = (int)Control_points[0][0].size();
}

void BSplineSurface::UpdateFootprint() {
  unsigned int line_size;

  for (int i = selected_level - 1; i >= 0; i--) {
    if (Control_points[i].size() > Control_points[i + 1].size()) {
 
      for (unsigned int j = 0; j < Control_points[i][0].size(); j++) {
        for (unsigned int k = 0; k <= Control_points[i].size(); k++) {
          if (k < 5) {
            if (k == 0)
              Control_points[i][0][j] = Control_points[i + 1][0][j];

            if (k == 1) {
              Control_points[i][1][j] =
                  Control_points[i + 1][0][j] * (1.0 / 2.0) +
                  Control_points[i + 1][1][j] * (1.0 / 2.0) + Details[i][0][j];
            }
            if (k == 2)
              Control_points[i][2][j] =
                  Control_points[i + 1][1][j] * (3.0 / 4.0) +
                  Control_points[i + 1][2][j] * (1.0 / 4.0) +
                  Details[i][0][j] * (-2033.0 / 3000.0) +
                  Details[i][1][j] * (-49.0 / 152.0);

            if (k == 3)
              Control_points[i][3][j] =
                  Control_points[i + 1][1][j] * (3.0 / 16.0) +
                  Control_points[i + 1][2][j] * (11.0 / 16.0) +
                  Control_points[i + 1][3][j] * (1.0 / 8.0) +
                  Details[i][0][j] * (2137.0 / 12000.0) +
                  Details[i][1][j] * (-289.0 / 608.0) +
                  Details[i][2][j] * (-23.0 / 208.0);

            if (k == 4) {
              Control_points[i][4][j] =
                  Control_points[i + 1][2][j] * (1.0 / 2.0) +
                  Control_points[i + 1][3][j] * (1.0 / 2.0) +
                  Details[i][0][j] * (139.0 / 500.0) + Details[i][1][j] * 1.0 +
                  Details[i][2][j] * (-23.0 / 52.0);
            }
          } else {
            line_size = (int)Control_points[i].size() - 1;
            unsigned int lsize2 = (int)Control_points[i + 1].size() - 1;
            if (k < line_size - 4) {
              for (; k < line_size - 4; k++) {
                if (k % 2 == 1) {
                  Control_points[i][k][j] =
                      Control_points[i + 1][(k) / 2][j] * (1.0 / 8.0) +
                      Control_points[i + 1][(k) / 2 + 1][j] * (3.0 / 4.0) +
                      Control_points[i + 1][(k) / 2 + 2][j] * (1.0 / 8.0);
                } else {
                  Control_points[i][k][j] =
                      Control_points[i + 1][(k) / 2][j] * (1.0 / 2.0) +
                      Control_points[i + 1][(k) / 2 + 1][j] * (1.0 / 2.0);
                }
                if (k <= 7 || line_size - k <= 7) {
                  if (k == 5) {
                    Control_points[i][k][j] =
                        Control_points[i][k][j] +
                        Details[i][0][j] * (139.0 / 2000.0) +
                        Details[i][1][j] * (-347.0 / 912.0) +
                        Details[i][2][j] * (-63.0 / 208.0) +
                        Details[i][3][j] * (-23.0 / 208.0);

                  } else if (k == 6) {
                    Control_points[i][k][j] =
                        Control_points[i][k][j] +
                        Details[i][1][j] * (-115.0 / 228.0) +
                        Details[i][2][j] * 1.0 +
                        Details[i][3][j] * (-23.0 / 52.0);

                  } else if (k == 7) {
                    Control_points[i][k][j] =
                        Control_points[i][k][j] +
                        Details[i][1][j] * (-115.0 / 912.0) +
                        Details[i][2][j] * (-63.0 / 208.0) +
                        Details[i][3][j] * (-63.0 / 208.0) +
                        Details[i][4][j] * (-23.0 / 208.0);

                  } else if (k == line_size - 7)
                    Control_points[i][k][j] =
                        Control_points[i][k][j] +
                        Details[i][lsize2 - 4][j] * (-115.0 / 912.0) +
                        Details[i][lsize2 - 5][j] * (-63.0 / 208.0) +
                        Details[i][lsize2 - 6][j] * (-63.0 / 208.0) +
                        Details[i][lsize2 - 7][j] * (-23.0 / 208.0);

                  else if (k == line_size - 6)
                    Control_points[i][k][j] =
                        Control_points[i][k][j] +
                        Details[i][lsize2 - 4][j] * (-115.0 / 228.0) +
                        Details[i][lsize2 - 5][j] * 1.0 +
                        Details[i][lsize2 - 6][j] * (-23.0 / 52.0);

                  else if (k == line_size - 5)
                    Control_points[i][k][j] =
                        Control_points[i][k][j] +
                        Details[i][lsize2 - 3][j] * (139.0 / 2000.0) +
                        Details[i][lsize2 - 4][j] * (-347.0 / 912.0) +
                        Details[i][lsize2 - 5][j] * (-63.0 / 208.0) +
                        Details[i][lsize2 - 6][j] * (-23.0 / 208.0);

                } else if (k % 2 == 0)
                  Control_points[i][k][j] =
                      Control_points[i][k][j] +
                      Details[i][(k - 4) / 2][j] * (-23.0 / 52.0) +
                      Details[i][(k - 4) / 2 + 1][j] * 1.0 +
                      Details[i][(k - 4) / 2 + 2][j] * (-23.0 / 52.0);

                else
                  Control_points[i][k][j] =
                      Control_points[i][k][j] +
                      Details[i][(k - 4) / 2][j] * (-23.0 / 208.0) +
                      Details[i][(k - 4) / 2 + 1][j] * (-63.0 / 208.0) +
                      Details[i][(k - 4) / 2 + 2][j] * (-63.0 / 208.0) +
                      Details[i][(k - 4) / 2 + 3][j] * (-23.0 / 208.0);
              }
            }
            if (k >= line_size - 4) {
              if (k == line_size - 4)
                Control_points[i][line_size - 4][j] =
                    Control_points[i + 1][lsize2 - 2][j] * (1.0 / 2.0) +
                    Control_points[i + 1][lsize2 - 3][j] * (1.0 / 2.0) +
                    Details[i][lsize2 - 3][j] * (139.0 / 500.0) +
                    Details[i][lsize2 - 4][j] * 1.0 +
                    Details[i][lsize2 - 5][j] * (-23.0 / 52.0);

              if (k == line_size - 3)
                Control_points[i][line_size - 3][j] =
                    Control_points[i + 1][lsize2 - 1][j] * (3.0 / 16.0) +
                    Control_points[i + 1][lsize2 - 2][j] * (11.0 / 16.0) +
                    Control_points[i + 1][lsize2 - 3][j] * (1.0 / 8.0) +
                    Details[i][lsize2 - 3][j] * (2137.0 / 12000.0) +
                    Details[i][lsize2 - 4][j] * (-289.0 / 608.0) +
                    Details[i][lsize2 - 5][j] * (-23.0 / 208.0);

              if (k == line_size - 2)
                Control_points[i][line_size - 2][j] =
                    Control_points[i + 1][lsize2 - 1][j] * (3.0 / 4.0) +
                    Control_points[i + 1][lsize2 - 2][j] * (1.0 / 4.0) +
                    Details[i][lsize2 - 3][j] * (-2033.0 / 3000.0) +
                    Details[i][lsize2 - 4][j] * (-49.0 / 152.0);

              if (k == line_size - 1)
                Control_points[i][line_size - 1][j] =
                    Control_points[i + 1][lsize2][j] * (1.0 / 2.0) +
                    Control_points[i + 1][lsize2 - 1][j] * (1.0 / 2.0) +
                    Details[i][lsize2 - 3][j];

              if (k == line_size)
                Control_points[i][line_size][j] =
                    Control_points[i + 1][lsize2][j];
            }
          }
        }
      }
    } else {
      for (unsigned int j = 0; j < Control_points[i].size(); j++) {
        for (unsigned int k = 0; k <= Control_points[i][0].size(); k++) {
 
          unsigned int lsize2 = (int)Control_points[i + 1][0].size() - 1;
          if (k < 5) {
            if (k == 0)
              Control_points[i][j][0] = Control_points[i + 1][j][0] * 1.0;

            if (k == 1)
              Control_points[i][j][1] =
                  Control_points[i + 1][j][0] * (1.0 / 2.0) +
                  Control_points[i + 1][j][1] * (1.0 / 2.0) + Details[i][j][0];

            if (k == 2)
              Control_points[i][j][2] =
                  Control_points[i + 1][j][1] * (3.0 / 4.0) +
                  Control_points[i + 1][j][2] * (1.0 / 4.0) +
                  Details[i][j][0] * (-2033.0 / 3000.0) +
                  Details[i][j][1] * (-49.0 / 152.0);

            if (k == 3)
              Control_points[i][j][3] =
                  Control_points[i + 1][j][1] * (3.0 / 16.0) +
                  Control_points[i + 1][j][2] * (11.0 / 16.0) +
                  Control_points[i + 1][j][3] * (1.0 / 8.0) +
                  Details[i][j][0] * (2137.0 / 12000.0) +
                  Details[i][j][1] * (-289.0 / 608.0) +
                  Details[i][j][2] * (-23.0 / 208.0);

            if (k == 4)
              Control_points[i][j][4] =
                  Control_points[i + 1][j][2] * (1.0 / 2.0) +
                  Control_points[i + 1][j][3] * (1.0 / 2.0) +
                  Details[i][j][0] * (139.0 / 500.0) + Details[i][j][1] * 1.0 +
                  Details[i][j][2] * (-23.0 / 52.0);

          } else {

            line_size = (int)Control_points[i][0].size() - 1;

            if (k < line_size - 4) {
              for (; k < line_size - 4; k++) {
                if (k % 2) {
                  Control_points[i][j][k] =
                      Control_points[i + 1][j][(k) / 2] * (1.0 / 8.0) +
                      Control_points[i + 1][j][(k) / 2 + 1] * (3.0 / 4.0) +
                      Control_points[i + 1][j][(k) / 2 + 2] * (1.0 / 8.0);
                } else {
                  Control_points[i][j][k] =
                      Control_points[i + 1][j][(k) / 2] * (1.0 / 2.0) +
                      Control_points[i + 1][j][(k) / 2 + 1] * (1.0 / 2.0);
                }
                if (k <= 7 || k >= line_size - 7) {
                  if (k == 5) {
                    Control_points[i][j][k] =
                        Control_points[i][j][k] +
                        Details[i][j][0] * (139.0 / 2000.0) +
                        Details[i][j][1] * (-347.0 / 912.0) +
                        Details[i][j][2] * (-63.0 / 208.0) +
                        Details[i][j][3] * (-23.0 / 208.0);

                  } else if (k == 6) {
                    Control_points[i][j][k] =
                        Control_points[i][j][k] +
                        Details[i][j][1] * (-115.0 / 228.0) +
                        Details[i][j][2] * 1.0 +
                        Details[i][j][3] * (-23.0 / 52.0);
                  }

                  else if (k == 7) {
                    Control_points[i][j][k] =
                        Control_points[i][j][k] +
                        Details[i][j][1] * (-115.0 / 912.0) +
                        Details[i][j][2] * (-63.0 / 208.0) +
                        Details[i][j][3] * (-63.0 / 208.0) +
                        Details[i][j][4] * (-23.0 / 208.0);

                  } else if (k == line_size - 7)
                    Control_points[i][j][k] =
                        Control_points[i][j][k] +
                        Details[i][j][lsize2 - 4] * (-115.0 / 912.0) +
                        Details[i][j][lsize2 - 5] * (-63.0 / 208.0) +
                        Details[i][j][lsize2 - 6] * (-63.0 / 208.0) +
                        Details[i][j][lsize2 - 7] * (-23.0 / 208.0);

                  else if (k == line_size - 6)
                    Control_points[i][j][k] =
                        Control_points[i][j][k] +
                        Details[i][j][lsize2 - 4] * (-115.0 / 228.0) +
                        Details[i][j][lsize2 - 5] * 1.0 +
                        Details[i][j][lsize2 - 6] * (-23.0 / 52.0);

                  else if (k == line_size - 5)
                    Control_points[i][j][k] =
                        Control_points[i][j][k] +
                        Details[i][j][lsize2 - 3] * (139.0 / 2000.0) +
                        Details[i][j][lsize2 - 4] * (-347.0 / 912.0) +
                        Details[i][j][lsize2 - 5] * (-63.0 / 208.0) +
                        Details[i][j][lsize2 - 6] * (-23.0 / 208.0);
                } else if (k % 2 == 0)
                  Control_points[i][j][k] =
                      Control_points[i][j][k] +
                      Details[i][j][(k - 4) / 2] * (-23.0 / 52.0) +
                      Details[i][j][(k - 4) / 2 + 1] * 1.0 +
                      Details[i][j][(k - 4) / 2 + 2] * (-23.0 / 52.0);
                else
                  Control_points[i][j][k] =
                      Control_points[i][j][k] +
                      Details[i][j][(k - 4) / 2] * (-23.0 / 208.0) +
                      Details[i][j][(k - 4) / 2 + 1] * (-63.0 / 208.0) +
                      Details[i][j][(k - 4) / 2 + 2] * (-63.0 / 208.0) +
                      Details[i][j][(k - 4) / 2 + 3] * (-23.0 / 208.0);
              }
            }
            if (k >= line_size - 4) {
              if (k == line_size - 4)
                Control_points[i][j][line_size - 4] =
                    Control_points[i + 1][j][lsize2 - 2] * (1.0 / 2.0) +
                    Control_points[i + 1][j][lsize2 - 3] * (1.0 / 2.0) +
                    Details[i][j][lsize2 - 3] * (139.0 / 500.0) +
                    Details[i][j][lsize2 - 4] * 1.0 +
                    Details[i][j][lsize2 - 5] * (-23.0 / 52.0);

              if (k == line_size - 3)
                Control_points[i][j][line_size - 3] =
                    Control_points[i + 1][j][lsize2 - 1] * (3.0 / 16.0) +
                    Control_points[i + 1][j][lsize2 - 2] * (11.0 / 16.0) +
                    Control_points[i + 1][j][lsize2 - 3] * (1.0 / 8.0) +
                    Details[i][j][lsize2 - 3] * (2137.0 / 12000.0) +
                    Details[i][j][lsize2 - 4] * (-289.0 / 608.0) +
                    Details[i][j][lsize2 - 5] * (-23.0 / 208.0);

              if (k == line_size - 2)
                Control_points[i][j][line_size - 2] =
                    Control_points[i + 1][j][lsize2 - 1] * (3.0 / 4.0) +
                    Control_points[i + 1][j][lsize2 - 2] * (1.0 / 4.0) +
                    Details[i][j][lsize2 - 3] * (-2033.0 / 3000.0) +
                    Details[i][j][lsize2 - 4] * (-49.0 / 152.0);

              if (k == line_size - 1)
                Control_points[i][j][line_size - 1] =
                    Control_points[i + 1][j][lsize2] * (1.0 / 2.0) +
                    Control_points[i + 1][j][lsize2 - 1] * (1.0 / 2.0) +
                    Details[i][j][lsize2 - 3];

              if (k == line_size)
                Control_points[i][j][line_size] =
                    Control_points[i + 1][j][lsize2];
            }
          }
        }
      }
    }
  }
  for (unsigned int i = selected_level + 1; i < Control_points.size(); i++) {
    if (Control_points[i].size() < Control_points[i - 1].size()) {
      unsigned int line_size = (int)Control_points[i].size() - 1;
      for (unsigned int j = 0; j < Control_points[i][0].size(); j++) {
        for (unsigned int k = 0; k <= Control_points[i].size(); k++) {
          unsigned int l2 = (int)Control_points[i - 1].size() - 1;
          if (k == 0) {
            Control_points[i][0][j] = Control_points[i - 1][0][j];

            Details[i - 1][0][j] =
                Control_points[i - 1][0][j] * (-45.0 / 139.0) +
                Control_points[i - 1][1][j] * (90.0 / 139.0) +
                Control_points[i - 1][2][j] * (-135.0 / 278.0) +
                Control_points[i - 1][3][j] * (30.0 / 139.0) +
                Control_points[i - 1][4][j] * (-15.0 / 278.0);
          }
          if (k == 1) {
            Control_points[i][1][j] =
                Control_points[i - 1][0][j] * (-49.0 / 139.0) +
                Control_points[i - 1][1][j] * (98.0 / 139.0) +
                Control_points[i - 1][2][j] * (135.0 / 139.0) +
                Control_points[i - 1][3][j] * (-60.0 / 139.0) +
                Control_points[i - 1][4][j] * (15.0 / 139.0);

            Details[i - 1][1][j] =
                Control_points[i - 1][2][j] * (57.0 / 490.0) +
                Control_points[i - 1][3][j] * (-114.0 / 245.0) +
                Control_points[i - 1][4][j] * (171.0 / 245.0) +
                Control_points[i - 1][5][j] * (-114.0 / 245.0) +
                Control_points[i - 1][6][j] * (57.0 / 490.0);
          }
          if (k == 2) {
            Control_points[i][2][j] =
                Control_points[i - 1][0][j] * (9.0 / 50.0) +
                Control_points[i - 1][1][j] * (-9.0 / 25.0) +
                Control_points[i - 1][2][j] * (-2.0 / 25.0) +
                Control_points[i - 1][3][j] * (32.0 / 25.0) +
                Control_points[i - 1][4][j] * (43.0 / 100.0) +
                Control_points[i - 1][5][j] * (-3.0 / 5.0) +
                Control_points[i - 1][6][j] * (3.0 / 20.0);

            Details[i - 1][2][j] =
                Control_points[i - 1][4][j] * (13.0 / 98.0) +
                Control_points[i - 1][5][j] * (-26.0 / 49.0) +
                Control_points[i - 1][6][j] * (39.0 / 49.0) +
                Control_points[i - 1][7][j] * (-26.0 / 49.0) +
                Control_points[i - 1][8][j] * (13.0 / 98.0);
          }

          if (k >= 3 && k <= line_size - 3) {
            unsigned int band_start = (k - 3) * 2 + 2;

            Control_points[i][k][j] =
                Control_points[i - 1][band_start][j] * (23.0 / 196.0) +
                Control_points[i - 1][band_start + 1][j] * (-23.0 / 49.0) +
                Control_points[i - 1][band_start + 2][j] * (9.0 / 28.0) +
                Control_points[i - 1][band_start + 3][j] * (52.0 / 49.0) +
                Control_points[i - 1][band_start + 4][j] * (9.0 / 28.0) +
                Control_points[i - 1][band_start + 5][j] * (-23.0 / 49.0) +
                Control_points[i - 1][band_start + 6][j] * (23.0 / 196.0);

            if (2 * k < l2 - 8) {

              Details[i - 1][k][j] =
                  Control_points[i - 1][2 * k][j] * (13.0 / 98.0) +
                  Control_points[i - 1][2 * k + 1][j] * (-26.0 / 49.0) +
                  Control_points[i - 1][2 * k + 2][j] * (39.0 / 49.0) +
                  Control_points[i - 1][2 * k + 3][j] * (-26.0 / 49.0) +
                  Control_points[i - 1][2 * k + 4][j] * (13.0 / 98.0);

            }
          }

          if (k == line_size - 2) {

            Control_points[i][line_size - 2][j] =
                Control_points[i - 1][l2][j] * (9.0 / 50.0) +
                Control_points[i - 1][l2 - 1][j] * (-9.0 / 25.0) +
                Control_points[i - 1][l2 - 2][j] * (-2.0 / 25.0) +
                Control_points[i - 1][l2 - 3][j] * (32.0 / 25.0) +
                Control_points[i - 1][l2 - 4][j] * (43.0 / 100.0) +
                Control_points[i - 1][l2 - 5][j] * (-3.0 / 5.0) +
                Control_points[i - 1][l2 - 6][j] * (3.0 / 20.0);

            Details[i - 1][line_size - 5][j] =
                Control_points[i - 1][l2 - 4][j] * (13.0 / 98.0) +
                Control_points[i - 1][l2 - 5][j] * (-26.0 / 49.0) +
                Control_points[i - 1][l2 - 6][j] * (39.0 / 49.0) +
                Control_points[i - 1][l2 - 7][j] * (-26.0 / 49.0) +
                Control_points[i - 1][l2 - 8][j] * (13.0 / 98.0);
          }
          if (k == line_size - 1) {
            Control_points[i][line_size - 1][j] =
                Control_points[i - 1][l2][j] * (-49.0 / 139.0) +
                Control_points[i - 1][l2 - 1][j] * (98.0 / 139.0) +
                Control_points[i - 1][l2 - 2][j] * (135.0 / 139.0) +
                Control_points[i - 1][l2 - 3][j] * (-60.0 / 139.0) +
                Control_points[i - 1][l2 - 4][j] * (15.0 / 139.0);

            Details[i - 1][line_size - 4][j] =
                Control_points[i - 1][l2 - 2][j] * (57.0 / 490.0) +
                Control_points[i - 1][l2 - 3][j] * (-114.0 / 245.0) +
                Control_points[i - 1][l2 - 4][j] * (171.0 / 245.0) +
                Control_points[i - 1][l2 - 5][j] * (-114.0 / 245.0) +
                Control_points[i - 1][l2 - 6][j] * (57.0 / 490.0);
          }

          if (k == line_size) {
            Control_points[i][line_size][j] = Control_points[i - 1][l2][j];
            Details[i - 1][line_size - 3][j] =
                Control_points[i - 1][l2][j] * (-45.0 / 139.0) +
                Control_points[i - 1][l2 - 1][j] * (90.0 / 139.0) +
                Control_points[i - 1][l2 - 2][j] * (-135.0 / 278.0) +
                Control_points[i - 1][l2 - 3][j] * (30.0 / 139.0) +
                Control_points[i - 1][l2 - 4][j] * (-15.0 / 278.0);
          }
        }
      }
    } else {
      unsigned int line_size = (int)Control_points[i][0].size() - 1;

      for (unsigned int j = 0; j < Control_points[i].size(); j++) {
        for (unsigned int k = 0; k <= Control_points[i][0].size(); k++) {
	  unsigned int l2 = (int)Control_points[i - 1][0].size() - 1;

          if (k == 0) {

            Control_points[i][j][0] = Control_points[i - 1][j][0];

            Details[i - 1][j][0] =
                Control_points[i - 1][j][0] * (-45.0 / 139.0) +
                Control_points[i - 1][j][1] * (90.0 / 139.0) +
                Control_points[i - 1][j][2] * (-135.0 / 278.0) +
                Control_points[i - 1][j][3] * (30.0 / 139.0) +
                Control_points[i - 1][j][4] * (-15.0 / 278.0);
          }
          if (k == 1) {
            Control_points[i][j][1] =
                Control_points[i - 1][j][0] * (-49.0 / 139.0) +
                Control_points[i - 1][j][1] * (98.0 / 139.0) +
                Control_points[i - 1][j][2] * (135.0 / 139.0) +
                Control_points[i - 1][j][3] * (-60.0 / 139.0) +
                Control_points[i - 1][j][4] * (15.0 / 139.0);

            Details[i - 1][j][1] =
                Control_points[i - 1][j][2] * (57.0 / 490.0) +
                Control_points[i - 1][j][3] * (-114.0 / 245.0) +
                Control_points[i - 1][j][4] * (171.0 / 245.0) +
                Control_points[i - 1][j][5] * (-114.0 / 245.0) +
                Control_points[i - 1][j][6] * (57.0 / 490.0);
          }
          if (k == 2) {
            Control_points[i][j][2] =
                Control_points[i - 1][j][0] * (9.0 / 50.0) +
                Control_points[i - 1][j][1] * (-9.0 / 25.0) +
                Control_points[i - 1][j][2] * (-2.0 / 25.0) +
                Control_points[i - 1][j][3] * (32.0 / 25.0) +
                Control_points[i - 1][j][4] * (43.0 / 100.0) +
                Control_points[i - 1][j][5] * (-3.0 / 5.0) +
                Control_points[i - 1][j][6] * (3.0 / 20.0);

            Details[i - 1][j][2] =
                Control_points[i - 1][j][4] * (13.0 / 98.0) +
                Control_points[i - 1][j][5] * (-26.0 / 49.0) +
                Control_points[i - 1][j][6] * (39.0 / 49.0) +
                Control_points[i - 1][j][7] * (-26.0 / 49.0) +
                Control_points[i - 1][j][8] * (13.0 / 98.0);
          }

          if (k >= 3 && k <= line_size - 3) {
            unsigned int band_start = (k - 3) * 2 + 2;

            Control_points[i][j][k] =
                Control_points[i - 1][j][band_start] * (23.0 / 196.0) +
                Control_points[i - 1][j][band_start + 1] * (-23.0 / 49.0) +
                Control_points[i - 1][j][band_start + 2] * (9.0 / 28.0) +
                Control_points[i - 1][j][band_start + 3] * (52.0 / 49.0) +
                Control_points[i - 1][j][band_start + 4] * (9.0 / 28.0) +
                Control_points[i - 1][j][band_start + 5] * (-23.0 / 49.0) +
                Control_points[i - 1][j][band_start + 6] * (23.0 / 196.0);

            if (2 * k < l2 - 8) {
              Details[i - 1][j][k] =
                  Control_points[i - 1][j][2 * k] * (13.0 / 98.0) +
                  Control_points[i - 1][j][2 * k + 1] * (-26.0 / 49.0) +
                  Control_points[i - 1][j][2 * k + 2] * (39.0 / 49.0) +
                  Control_points[i - 1][j][2 * k + 3] * (-26.0 / 49.0) +
                  Control_points[i - 1][j][2 * k + 4] * (13.0 / 98.0);
            }
          }

          if (k == line_size - 2) {
            Control_points[i][j][line_size - 2] =
                Control_points[i - 1][j][l2] * (9.0 / 50.0) +
                Control_points[i - 1][j][l2 - 1] * (-9.0 / 25.0) +
                Control_points[i - 1][j][l2 - 2] * (-2.0 / 25.0) +
                Control_points[i - 1][j][l2 - 3] * (32.0 / 25.0) +
                Control_points[i - 1][j][l2 - 4] * (43.0 / 100.0) +
                Control_points[i - 1][j][l2 - 5] * (-3.0 / 5.0) +
                Control_points[i - 1][j][l2 - 6] * (3.0 / 20.0);

            Details[i - 1][j][line_size - 5] =
                Control_points[i - 1][j][l2 - 4] * (13.0 / 98.0) +
                Control_points[i - 1][j][l2 - 5] * (-26.0 / 49.0) +
                Control_points[i - 1][j][l2 - 6] * (39.0 / 49.0) +
                Control_points[i - 1][j][l2 - 7] * (-26.0 / 49.0) +
                Control_points[i - 1][j][l2 - 8] * (13.0 / 98.0);
          }
          if (k == line_size - 1) {
            Control_points[i][j][line_size - 1] =
                Control_points[i - 1][j][l2] * (-49.0 / 139.0) +
                Control_points[i - 1][j][l2 - 1] * (98.0 / 139.0) +
                Control_points[i - 1][j][l2 - 2] * (135.0 / 139.0) +
                Control_points[i - 1][j][l2 - 3] * (-60.0 / 139.0) +
                Control_points[i - 1][j][l2 - 4] * (15.0 / 139.0);

            Details[i - 1][j][line_size - 4] =
                Control_points[i - 1][j][l2 - 2] * (57.0 / 490.0) +
                Control_points[i - 1][j][l2 - 3] * (-114.0 / 245.0) +
                Control_points[i - 1][j][l2 - 4] * (171.0 / 245.0) +
                Control_points[i - 1][j][l2 - 5] * (-114.0 / 245.0) +
                Control_points[i - 1][j][l2 - 6] * (57.0 / 490.0);
          }

          if (k == line_size) {
            Control_points[i][j][line_size] = Control_points[i - 1][j][l2];

            Details[i - 1][j][line_size - 3] =
                Control_points[i - 1][j][l2] * (-45.0 / 139.0) +
                Control_points[i - 1][j][l2 - 1] * (90.0 / 139.0) +
                Control_points[i - 1][j][l2 - 2] * (-135.0 / 278.0) +
                Control_points[i - 1][j][l2 - 3] * (30.0 / 139.0) +
                Control_points[i - 1][j][l2 - 4] * (-15.0 / 278.0);
          }
        }
      }
    }
  }
}


void BSplineSurface::CalcPreImage(int i, int j, unsigned int level) {
  if (i > footprint[level][0])
    footprint[level][0] = i;
  if (i < footprint[level][1])
    footprint[level][1] = i;
  if (j > footprint[level][2])
    footprint[level][2] = j;
  if (j < footprint[level][3])
    footprint[level][3] = j;

  if (level < Control_points.size() - 1) {
    if (Control_points[level].size() > Control_points[level + 1].size()) {
      int line_size = (int)Control_points[level].size() - 1;
      if (i == 0) {
        CalcPreImage(0, j, level + 1);
        CalcPreImage(2, j, level + 1);
      }

      if (i == 1) {
        CalcPreImage(0, j, level + 1);
        CalcPreImage(2, j, level + 1);
      }

      if (i == 2) {
        CalcPreImage(0, j, level + 1);
        CalcPreImage(3, j, level + 1);
      }

      if (i > 2 && i < line_size - 2) {
        if (i > 6)
          CalcPreImage(i / 2 - (i + 1) % 2, j, level + 1);
        else
          CalcPreImage(0, j, level + 1);
        CalcPreImage(i / 2 + 3, j, level + 1);
      }

      int l2 = (int)Control_points[level + 1].size() - 1;

      if (i == line_size) {
        CalcPreImage(l2, j, level + 1);
        CalcPreImage(l2 - 2, j, level + 1);
      }

      if (i == line_size - 1) {
        CalcPreImage(l2 - 2, j, level + 1);
        CalcPreImage(l2 - 1, j, level + 1);
      }

      if (i == line_size - 2) {
        CalcPreImage(l2 - 3, j, level + 1);
        CalcPreImage(l2 - 1, j, level + 1);
      }
    } else {
      int line_size = (int)Control_points[level][0].size() - 1;

      if (j == 0) {
        CalcPreImage(i, 0, level + 1);
        CalcPreImage(i, 2, level + 1);
      }

      if (j == 1) {
        CalcPreImage(i, 1, level + 1);
        CalcPreImage(i, 2, level + 1);
      }

      if (j == 2) {
        CalcPreImage(i, 1, level + 1);
        CalcPreImage(i, 3, level + 1);
      }

      if (j > 2 && j < line_size - 2) {
        CalcPreImage(i, j / 2 - (j + 1) % 2, level + 1);
        CalcPreImage(i, j / 2 + 2, level + 1);
      }

      int l2 = (int)Control_points[level + 1][0].size() - 1;

      if (j == line_size) {
        CalcPreImage(i, l2 - 2, level + 1);
        CalcPreImage(i, l2, level + 1);
      }

      if (j == line_size - 1) {
        CalcPreImage(i, l2 - 2, level + 1);
        CalcPreImage(i, l2 - 1, level + 1);
      }

      if (j == line_size - 2) {
        CalcPreImage(i, l2 - 3, level + 1);
        CalcPreImage(i, l2 - 1, level + 1);
      }
    }
  }
}

void BSplineSurface::CalcFootprint(unsigned int i, unsigned int j,
                                   unsigned int level) {

  if ((int)i > footprint[level][0])
    footprint[level][0] = (int)i;
  if ((int)i < footprint[level][1])
    footprint[level][1] = (int)i;
  if ((int)j > footprint[level][2])
    footprint[level][2] = (int)j;
  if ((int)j < footprint[level][3])
    footprint[level][3] = (int)j;

  if (level) {
    if (Control_points[level].size() < Control_points[level - 1].size()) {
      if (i == 0) {
        CalcFootprint(0, j, level - 1);
        CalcFootprint(1, j, level - 1);

      } else if (i == 1) {
        CalcFootprint(1, j, level - 1);
        CalcFootprint(3, j, level - 1);

      } else if (i == 2) {
        CalcFootprint(2, j, level - 1);
        CalcFootprint(5, j, level - 1);

      } else if (i >= 3 && i < (Control_points[level].size() - 3)) {
        CalcFootprint(2 * (i - 3) + 3, j, level - 1);
        CalcFootprint(2 * (i - 3) + 7, j, level - 1);

      } else {
        unsigned int l = (int)Control_points[level].size() - 1;
        unsigned int l2 = (int)Control_points[level - 1].size() - 1;

        if (i == l) {
          CalcFootprint(l2, j, level - 1);
          CalcFootprint(l2 - 1, j, level - 1);

        } else if (i == l - 1) {
          CalcFootprint(l2 - 1, j, level - 1);
          CalcFootprint(l2 - 3, j, level - 1);

        } else if (i == l - 2) {
          CalcFootprint(l2 - 2, j, level - 1);
          CalcFootprint(l2 - 5, j, level - 1);
        }
      }
    } else {
      if (j == 0) {
        CalcFootprint(i, 0, level - 1);
        CalcFootprint(i, 1, level - 1);

      } else if (j == 1) {
        CalcFootprint(i, 1, level - 1);
        CalcFootprint(i, 3, level - 1);

      } else if (j == 2) {
        CalcFootprint(i, 2, level - 1);
        CalcFootprint(i, 5, level - 1);

      } else if (j >= 3 && j < (Control_points[level][0].size() - 3)) {
        CalcFootprint(i, 2 * (j - 3) + 3, level - 1);
        CalcFootprint(i, 2 * (j - 3) + 7, level - 1);

      } else {
        unsigned int l = (int)Control_points[level][0].size() - 1;
        unsigned int l2 = (int)Control_points[level - 1][0].size() - 1;

        if (j == l) {
          CalcFootprint(i, l2, level - 1);
          CalcFootprint(i, l2 - 1, level - 1);

        } else if (j == l - 1) {
          CalcFootprint(i, l2 - 1, level - 1);
          CalcFootprint(i, l2 - 3, level - 1);

        } else if (j == l - 2) {
          CalcFootprint(i, l2 - 2, level - 1);
          CalcFootprint(i, l2 - 5, level - 1);
        }
      }
    }
  }
}

void BSplineSurface::RenderFootprint() {

  for (unsigned int i = 0; i < footprint.size(); i++) {
    RenderLine(footprint[i][0], footprint[i][2], i, footprint[i][0],
               footprint[i][3], i);
    RenderLine(footprint[i][0], footprint[i][2], i, footprint[i][1],
               footprint[i][2], i);
    RenderLine(footprint[i][1], footprint[i][2], i, footprint[i][1],
               footprint[i][3], i);
    RenderLine(footprint[i][0], footprint[i][3], i, footprint[i][1],
               footprint[i][3], i);
  }
}

void BSplineSurface::RenderSupport(unsigned int i, unsigned int j,
                                   unsigned int level) {
  if (level) {
    if (Control_points[level].size() < Control_points[level - 1].size()) {
      if (i == 0) {
        RenderLine(i, j, level, 0, j, level - 1);
        RenderSupport(0, j, level - 1);
        RenderLine(i, j, level, 1, j, level - 1);
        RenderSupport(1, j, level - 1);

      } else if (i == 1) {
        RenderLine(i, j, level, 1, j, level - 1);
        RenderSupport(1, j, level - 1);
        RenderLine(i, j, level, 2, j, level - 1);
        RenderSupport(2, j, level - 1);
        RenderLine(i, j, level, 3, j, level - 1);
        RenderSupport(3, j, level - 1);

      } else if (i == 2) {
        RenderLine(i, j, level, 2, j, level - 1);
        RenderSupport(2, j, level - 1);
        RenderLine(i, j, level, 3, j, level - 1);
        RenderSupport(3, j, level - 1);
        RenderLine(i, j, level, 4, j, level - 1);
        RenderSupport(4, j, level - 1);
        RenderLine(i, j, level, 5, j, level - 1);
        RenderSupport(5, j, level - 1);

      } else if (i >= 3 && i < (Control_points[level].size() - 3)) {
        RenderLine(i, j, level, 2 * (i - 3) + 3, j, level - 1);
        RenderSupport(2 * (i - 3) + 3, j, level - 1);
        RenderLine(i, j, level, 2 * (i - 3) + 4, j, level - 1);
        RenderSupport(2 * (i - 3) + 4, j, level - 1);
        RenderLine(i, j, level, 2 * (i - 3) + 5, j, level - 1);
        RenderSupport(2 * (i - 3) + 5, j, level - 1);
        RenderLine(i, j, level, 2 * (i - 3) + 6, j, level - 1);
        RenderSupport(2 * (i - 3) + 6, j, level - 1);
        RenderLine(i, j, level, 2 * (i - 3) + 7, j, level - 1);
        RenderSupport(2 * (i - 3) + 7, j, level - 1);

      } else {
        unsigned int l = (int)Control_points[level].size() - 1;
        unsigned int l2 = (int)Control_points[level - 1].size() - 1;
        if (i == l) {
          RenderLine(i, j, level, l2, j, level - 1);
          RenderSupport(l2, j, level - 1);
          RenderLine(i, j, level, l2 - 1, j, level - 1);
          RenderSupport(l2 - 1, j, level - 1);

        } else if (i == l - 1) {
          RenderLine(i, j, level, l2 - 1, j, level - 1);
          RenderSupport(l2 - 1, j, level - 1);
          RenderLine(i, j, level, l2 - 2, j, level - 1);
          RenderSupport(l2 - 2, j, level - 1);
          RenderLine(i, j, level, l2 - 3, j, level - 1);
          RenderSupport(l2 - 3, j, level - 1);

        } else if (i == l - 2) {
          RenderLine(i, j, level, l2 - 2, j, level - 1);
          RenderSupport(l2 - 2, j, level - 1);
          RenderLine(i, j, level, l2 - 3, j, level - 1);
          RenderSupport(l2 - 3, j, level - 1);
          RenderLine(i, j, level, l2 - 4, j, level - 1);
          RenderSupport(l2 - 4, j, level - 1);
          RenderLine(i, j, level, l2 - 5, j, level - 1);
          RenderSupport(l2 - 5, j, level - 1);
        }
      }
    } else {
      if (j == 0) {
        RenderLine(i, j, level, i, 0, level - 1);
        RenderSupport(i, 0, level - 1);
        RenderLine(i, j, level, i, 1, level - 1);
        RenderSupport(i, 1, level - 1);

      } else if (j == 1) {
        RenderLine(i, j, level, i, 1, level - 1);
        RenderSupport(i, 1, level - 1);
        RenderLine(i, j, level, i, 2, level - 1);
        RenderSupport(i, 2, level - 1);
        RenderLine(i, j, level, i, 3, level - 1);
        RenderSupport(i, 3, level - 1);

      } else if (j == 2) {
        RenderLine(i, j, level, i, 2, level - 1);
        RenderSupport(i, 2, level - 1);
        RenderLine(i, j, level, i, 3, level - 1);
        RenderSupport(i, 3, level - 1);
        RenderLine(i, j, level, i, 4, level - 1);
        RenderSupport(i, 4, level - 1);
        RenderLine(i, j, level, i, 5, level - 1);
        RenderSupport(i, 5, level - 1);

      } else if (j >= 3 && j < (Control_points[level][0].size() - 3)) {
        RenderLine(i, j, level, i, 2 * (j - 3) + 3, level - 1);
        RenderSupport(i, 2 * (j - 3) + 3, level - 1);
        RenderLine(i, j, level, i, 2 * (j - 3) + 4, level - 1);
        RenderSupport(i, 2 * (j - 3) + 4, level - 1);
        RenderLine(i, j, level, i, 2 * (j - 3) + 5, level - 1);
        RenderSupport(i, 2 * (j - 3) + 5, level - 1);
        RenderLine(i, j, level, i, 2 * (j - 3) + 6, level - 1);
        RenderSupport(i, 2 * (j - 3) + 6, level - 1);
        RenderLine(i, j, level, i, 2 * (j - 3) + 7, level - 1);
        RenderSupport(i, 2 * (j - 3) + 7, level - 1);

      } else {
        unsigned int l = (int)Control_points[level][0].size() - 1;
        unsigned int l2 = (int)Control_points[level - 1][0].size() - 1;

        if (j == l) {
          RenderLine(i, j, level, i, l2, level - 1);
          RenderSupport(i, l2, level - 1);
          RenderLine(i, j, level, i, l2 - 1, level - 1);
          RenderSupport(i, l2 - 1, level - 1);

        } else if (j == l - 1) {
          RenderLine(i, j, level, i, l2 - 1, level - 1);
          RenderSupport(i, l2 - 1, level - 1);
          RenderLine(i, j, level, i, l2 - 2, level - 1);
          RenderSupport(i, l2 - 2, level - 1);
          RenderLine(i, j, level, i, l2 - 3, level - 1);
          RenderSupport(i, l2 - 3, level - 1);
        } else if (j == l - 2) {
          RenderLine(i, j, level, i, l2 - 2, level - 1);
          RenderSupport(i, l2 - 2, level - 1);
          RenderLine(i, j, level, i, l2 - 3, level - 1);
          RenderSupport(i, l2 - 3, level - 1);
          RenderLine(i, j, level, i, l2 - 4, level - 1);
          RenderSupport(i, l2 - 4, level - 1);
          RenderLine(i, j, level, i, l2 - 5, level - 1);
          RenderSupport(i, l2 - 5, level - 1);
        }
      }
    }
  }
}

void BSplineSurface::RenderLine(int i1, int j1, int level1, int i2, int j2,
                                int level2) {
  glBegin(GL_LINES);

  glVertex3d(Control_points[level1][i1][j1][0],
             Control_points[level1][i1][j1][1],
             Control_points[level1][i1][j1][2] - (level1) * (MRES_OFF));
  glVertex3d(Control_points[level2][i2][j2][0],
             Control_points[level2][i2][j2][1],
             Control_points[level2][i2][j2][2] - (level2) * (MRES_OFF));

  glEnd();
}

void BSplineSurface::SelectPoint(Point3d n, Matrix A, Point3d trans) {
  Point3d t;

  double dis = 1000;

  for (unsigned int i = 0; i < Control_points[selected_level].size(); i++) {
    for (unsigned int j = 0; j < Control_points[selected_level][0].size();
         j++) {
      t = Control_points[selected_level][i][j];
      t[2] = -t[2] + (selected_level) * (MRES_OFF);
      t = A * t;
      t[2] = 0;
      t = t + trans;
      if ((n - t).Length() < dis) {
        dis = (n - t).Length();
        selected[0] = i;
        selected[1] = j;
      }
    }
  }

  for (unsigned int i = 0; i < footprint.size(); i++) {
    footprint[i][1] = footprint[i][3] = 10000000;
    footprint[i][0] = footprint[i][2] = -1;
    footprint[i][1] = footprint[i][3] = 0;
    footprint[i][0] = (int)Control_points[i].size() - 1;
    footprint[i][2] = (int)Control_points[i][0].size() - 1;
  }
  CalcFootprint(selected[0], selected[1], selected_level);
  CalcPreImage(selected[0], selected[1], selected_level);

  UpdateFootprint();
}

void BSplineSurface::MovePoint(Point3d n, Matrix A, Point3d trans) {
  Point3d t;

  t = Control_points[selected_level][selected[0]][selected[1]];
  t[2] = -t[2] + (selected_level) * (MRES_OFF);
  t = A * t;
  t[0] = n[0] - trans[0];
  t[1] = n[1] - trans[1];
  t = (~A) * t;
  // potential problem
  t[2] = -t[2] + (selected_level) * (MRES_OFF);
  ;
  Control_points[selected_level][selected[0]][selected[1]] = t;

  UpdateFootprint();
}



bool BSplineSurface::EditLevel(unsigned int i) {
  bool valid = true;

  if (i < Control_points.size())
    selected_level = i;
  else
    valid = false;

  selected[0] = 0;
  selected[1] = 0;

  for (unsigned int i = 0; i < footprint.size(); i++) {

    footprint[i][1] = footprint[i][3] = 100000000;
    footprint[i][0] = footprint[i][2] = -1;
  }

  CalcFootprint(selected[0], selected[1], selected_level);
  CalcPreImage(selected[0], selected[1], selected_level);

  selected_region.clear();
  return valid;
}

Point3d BSplineSurface::Eval(double u, double v) {
  Point3d p_v[4];

  if (u > (n_u))
    u = n_u;

  if (u < degree)
    u = degree;

  if (v > (n_v))
    v = n_v;

  if (v < degree)
    v = degree;

  int delta = (int)v;

  if (!periodic) {
    int i = 0;
    for (; i <= degree && (delta - i) <= 0; i++)
      p_v[i] = EvalU(u, 0);

    for (; i <= degree && (delta - i) < n_v; i++)
      p_v[i] = EvalU(u, delta - i);

    int t = n_v - 1;
    for (; i <= degree; i++)
      p_v[i] = EvalU(u, t);

  } else {
    for (int i = 0; i <= degree; i++)
      p_v[i] = EvalU(u, (delta - i + n_v) % (n_v));
  }

  for (int i = degree + 1; i >= 2; i--) {
    double omega;
    double ind = delta;
    int j = 0;

    for (; j <= i - 1 && (i - 1 + ind >= n_v); j++, ind--) {
      if (n_v - ind > 0 && v - ind > 0)
        omega = (v - ind) / (n_v - ind);
      else
        omega = 0;

      p_v[j] = p_v[j] * omega + p_v[j + 1] * (1 - omega);
    }

    for (; j <= i - 1; j++, ind--) {
      if ((i - 1 + ind - (degree)) <= 0 || v - degree <= 0) {
        omega = 0;
      } else if (ind < (degree)) {
        omega = (v - (degree)) / (i - 1 + ind - (degree));
      } else
        omega = (v - ind) / (i - 1);

      p_v[j] = p_v[j] * omega + p_v[j + 1] * (1 - omega);
    }
  }

  return p_v[0];
}

Point3d BSplineSurface::EvalU(double u, int m) {
  Point3d p_u[4];
  int delta = (int)u;

 
  if (!periodic) {
    int i = 0;
    for (; i <= degree && (delta - i) <= 0; i++)
      p_u[i] = Control_points[0][0][m];

    for (; i <= degree && (delta - i) < n_u; i++)
      p_u[i] = Control_points[0][delta - i][m];

    int t = n_u - 1;
    for (; i <= degree; i++)
      p_u[i] = Control_points[0][t][m];

  } else {
    for (int i = 0; i <= degree; i++)
      p_u[i] = Control_points[0][(delta - i + n_u) % (n_u)][m];
  }

  for (int i = degree + 1; i >= 2; i--) {
    double omega;
    double ind = delta;
    int j = 0;

    for (; j <= i - 1 && (i - 1 + ind >= n_u); j++, ind--) {
      if (n_u - ind > 0 && u - ind > 0)
        omega = (u - ind) / (n_u - ind);
      else
        omega = 0;

      p_u[j] = p_u[j] * omega + p_u[j + 1] * (1 - omega);
    }

    for (; j <= i - 1; j++, ind--) {
      if ((i - 1 + ind - (degree)) <= 0 || u - degree <= 0) {
        omega = 0;
      } else if (ind < (degree)) {
        omega = (u - (degree)) / (i - 1 + ind - (degree));
      } else
        omega = (u - ind) / (i - 1);

      p_u[j] = p_u[j] * omega + p_u[j + 1] * (1 - omega);
    }
  }

 
  return p_u[0];
}

void BSplineSurface::SelectRegion(vector<Point3d> s_reg, Matrix A,
                                  Point3d trans) {
  selected_region.clear();

  selected_bb[0] = selected_bb[2] = 0;
  selected_bb[1] = selected_bb[3] = 0;

  if (s_reg.size() > 1) {
    for (unsigned int i = 0; i < Control_points[selected_level].size(); i++) {
      for (unsigned int j = 0; j < Control_points[selected_level][0].size();
           j++) {

        int *index = new int[2];
        index[0] = i;
        index[1] = j;

        Point3d p = Control_points[selected_level][i][j];

        p[2] = -p[2];
        p = A * p;
        p[2] = 0;
        p = p + trans;
        if (InsideContour(s_reg, p))
          selected_region.push_back(index);
      }
    }
  }
}


bool BSplineSurface::InsideContour(vector<Point3d> shape, Point3d p) {

  bool inside = false;
  for (unsigned int i = 0; i < shape.size() - 2; i++) {
    if ((shape[i][1] - p[1]) * (shape[i + 1][1] - p[1]) <= 0 ||
        shape[i + 1][1] == p[1]) {
      if (shape[i][0] <= p[0] && shape[i + 1][0] <= p[0]) {
        inside = !inside;

      } else if ((shape[i][0] - p[0]) * (shape[i + 1][0] - p[0]) <= 0 ||
                 shape[i + 1][0] == p[0]) {
        double t = (p[1] - shape[i][1]) / (shape[i + 1][1] - shape[i][1]);
        if ((t * (shape[i + 1][0] - shape[i][0]) + shape[i][0]) <= p[0]) {
          inside = !inside;
        }
      }
    }
  }
  if ((shape[shape.size() - 1][1] - p[1]) * (shape[0][1] - p[1]) < 0 ||
      shape[0][1] == p[1]) {
    if (shape[shape.size() - 1][0] <= p[0] && shape[0][0] <= p[0]) {
      inside = !inside;

    } else if ((shape[shape.size() - 1][0] - p[0]) * (shape[0][0] - p[0]) < 0 ||
               shape[0][0] == p[0]) {
      double t = (p[1] - shape[shape.size() - 1][1]) /
                 (shape[0][1] - shape[shape.size() - 1][1]);

      if ((t * (shape[0][0] - shape[shape.size() - 1][0]) +
           shape[shape.size() - 1][0]) <= p[0]) {
        inside = !inside;
      }
    }
  }
  return inside;
}

void BSplineSurface::MoveRegion(Point3d dis) {
  for (unsigned int i = 0; i < selected_region.size(); i++)
    Control_points[selected_level][selected_region[i][0]]
                  [selected_region[i][1]] =
                      Control_points[selected_level][selected_region[i][0]]
                                    [selected_region[i][1]] +
                      dis;

  for (unsigned int i = 0; i < footprint.size(); i++) {
    footprint[i][1] = footprint[i][3] = 10000000;
    footprint[i][0] = footprint[i][2] = -1;
    footprint[i][1] = footprint[i][3] = 0;
    footprint[i][0] = (int)Control_points[i].size() - 1;
    footprint[i][2] = (int)Control_points[i][0].size() - 1;
  }
  CalcFootprint(selected_bb[0], selected_bb[1], selected_level);
  CalcPreImage(selected_bb[0], selected_bb[1], selected_level);

  CalcFootprint(selected_bb[0], selected_bb[3], selected_level);
  CalcPreImage(selected_bb[0], selected_bb[3], selected_level);

  CalcFootprint(selected_bb[2], selected_bb[1], selected_level);
  CalcPreImage(selected_bb[2], selected_bb[1], selected_level);

  CalcFootprint(selected_bb[2], selected_bb[3], selected_level);
  CalcPreImage(selected_bb[2], selected_bb[3], selected_level);

  UpdateFootprint();
}

Point3d BSplineSurface::NormalEval(double u, double v) {
  Point3d p_u;
  Point3d p_v;

  if (u > (n_u))
    u = n_u;

  if (u < 3)
    u = 3;

  if (v > (n_v))
    v = n_v;

  if (v < 3)
    v = 3;

  if (u > 5.0)
    p_u = (Eval(u, v) - Eval(u - 0.05, v)) * (1.0 / 0.05);
  else
    p_u = (Eval(u + 0.05, v) - Eval(u, v)) * (1.0 / 0.05);

  if (v > 5.0)
    p_v = (Eval(u, v) - Eval(u, v - 0.05)) * (1.0 / 0.05);
  else
    p_v = (Eval(u, v + 0.05) - Eval(u, v)) * (1.0 / 0.05);

  p_u = p_u * (1 / p_u.Length());
  p_v = p_v * (1 / p_v.Length());

  p_u = p_u.Cross(p_v);

  float l = p_u.Length();
   p_u = p_u * (1 / l);

  return p_u;
}

BSplineSurface &BSplineSurface::operator=(const BSplineSurface &B) {
  _TextureId = B._TextureId;
  _scale = B._scale;
  _divU = B._divU;
  _divV = B._divV;
  _vrtx = B._vrtx;
  _nrml = B._nrml;
  _f_name = B._f_name;

  if (B.precomputed) {
    _vrtx = B._vrtx;
    _nrml = B._nrml;
    precomputed = B.precomputed;
  } else
    precomputed = false;

  Reset(B.n_u, B.n_v);

  for (int i = 0; i < n_u; i++)
    for (int j = 0; j < n_v; j++)
      Control_points[0][i][j] = B.Control_points[0][i][j];

  return *this;
}

BSplineSurface BSplineSurface::operator+(const BSplineSurface &B) {
  BSplineSurface A1 = (*this);
  BSplineSurface A2 = B;
  BSplineSurface P;

  while (A1.n_u < A2.n_u)
    A1.DoubleU();

  while (A2.n_u < A1.n_u)
    A2.DoubleU();

  while (A1.n_v < A2.n_v)
    A1.DoubleV();

  while (A2.n_v < A1.n_v)
    A2.DoubleV();

  P.Reset(A1.n_u, A1.n_v);

  for (int i = 0; i < P.n_u; i++)
    for (int j = 0; j < P.n_v; j++)
      P.Control_points[0][i][j] =
          A1.Control_points[0][i][j] + A2.Control_points[0][i][j];

  return P;
}

BSplineSurface BSplineSurface::operator-(const BSplineSurface &B) {
  BSplineSurface A1 = (*this);
  BSplineSurface A2 = B;
  BSplineSurface P;

  while (A1.n_u < A2.n_u)
    A1.DoubleU();

  while (A2.n_u < A1.n_u)
    A2.DoubleU();

  while (A1.n_v < A2.n_v)
    A1.DoubleV();

  while (A2.n_v < A1.n_v)
    A2.DoubleV();

  P.Reset(A1.n_u, A1.n_v);

  for (int i = 0; i < P.n_u; i++)
    for (int j = 0; j < P.n_v; j++)
      P.Control_points[0][i][j] =
          A1.Control_points[0][i][j] - A2.Control_points[0][i][j];

  return P;
}

BSplineSurface BSplineSurface::operator*(const double c) {
  BSplineSurface P = (*this);

  P.Reset(n_u, n_v);

  for (int i = 0; i < n_u; i++)
    for (int j = 0; j < n_v; j++)
      P.Control_points[0][i][j] = Control_points[0][i][j] * c;

  return P;
}

void BSplineSurface::SetColors(double c1[3], double c2[3], double c3[3]) {
  for (int i = 0; i < 3; i++) {
    sel_point[i] = c1[i];
    unsel_point[i] = c2[i];
    poly_line[i] = c3[i];
  }
}

const Point3d BSplineSurface::PartialU(double u, double v) {
  Point3d p_u, Eu1, Eu2;
  Point2d u1, u2;

  u1[0] = u2[0] = u;
  u1[1] = u2[1] = v;

  u1[0] -= 5 * err_tol;
  u2[0] += 5 * err_tol;
  Eu1 = Eval(u1[0], u1[1]);
  Eu2 = Eval(u2[0], u2[1]);

  if (u1[0] < 3.0)
    u1[0] = 3.0;

  if (u2[0] > (n_u))
    u2[0] = (n_u);

  p_u = (Eu2 - Eu1) * (1.0 / (fabs(u1[0] - u2[0])));

  return p_u;
}

const Point3d BSplineSurface::PartialV(double u, double v) {

  Point3d p_v, Ev1, Ev2;
  Point2d v1, v2;

  v1[0] = v2[0] = u;
  v1[1] = v2[1] = v;

  v1[1] -= 5 * err_tol;
  v2[1] += 5 * err_tol;
  Ev1 = Eval(v1[0], v1[1]);
  Ev2 = Eval(v2[0], v2[1]);

  if (v1[1] < 3.0)
    v1[1] = 3.0;

  if (v2[1] > (n_u))
    v2[1] = (n_u);

  p_v = (Ev2 - Ev1) * (1.0 / (fabs(v1[1] - v2[1])));

  return p_v;
}

bool BSplineSurface::PointInversion(Point2d &surf_p, Point2d p0, Point3d cp) {

  Point2d u1, u2;
  Point3d Eu1, Eu2;
  Point2d v1, v2;
  Point3d Ev1, Ev2;
  Point2d p = p0;
  Point2d prev_step;
  Point3d Esurf_p;

  double dis = 1000;
  double count = 0;
  double du, dv;

  surf_p = p0;
  Esurf_p = Eval(surf_p[0], surf_p[1]);

  while (fabs(dis - (cp - Esurf_p).Length()) > err_tol && count < max_it) {
    count++;
    dis = (cp - Esurf_p).Length();

    if (surf_p[0] < 3)
      surf_p[0] = 3;

    if (surf_p[0] > (n_u))
      surf_p[0] = (n_u);

    if (surf_p[1] < 3)
      surf_p[1] = 3;

    if (surf_p[1] > (n_v))
      surf_p[1] = (n_v);

    u1 = surf_p;
    u2 = surf_p;
    u1[0] -= 5 * err_tol;
    u2[0] += 5 * err_tol;
    Eu1 = Eval(u1[0], u1[1]);
    Eu2 = Eval(u2[0], u2[1]);

    Point3d Fu = Eu2 - Eu1;
    Fu = Fu * (1 / Fu.Length());

    if (u1[0] < 3.0)
      u1[0] = 3.0;

    if (u2[0] > (n_u))
      u2[0] = (n_u);

    v1 = surf_p;
    v2 = surf_p;
    v1[1] -= 5 * err_tol;
    v2[1] += 5 * err_tol;
    Ev1 = Eval(v1[0], v1[1]);
    Ev2 = Eval(v2[0], v2[1]);

    if (v1[1] < 3.0)
      v1[1] = 3.0;

    if (v2[1] > (n_v))
      v2[1] = (n_v);

    Point3d Fv = Ev2 - Ev1;
    Fv = Fv * (1 / Fv.Length());

    float c = 1;

    du = -((cp - Esurf_p - Fu * (fabs(u2[0] - u1[0])) * c).Length() -
           (cp - Esurf_p).Length()) /
         (fabs(u2[0] - u1[0]));
    dv = -((cp - Esurf_p - Fv * (fabs(v2[1] - v1[1])) * c).Length() -
           (cp - Esurf_p).Length()) /
         (fabs(v2[1] - v1[1]));


    double step = 0.01;
    int search_steps = 0;
    while (step > err_tol && search_steps < inner_loop_max_it) {
      search_steps++;

      Point2d temp = surf_p;

      temp[0] += step * du;
      temp[1] += step * dv;

      if (temp[0] < 3)
        temp[0] = 3;

      if (temp[0] > (n_u))
        temp[0] = (n_u);

      if (temp[1] < 3)
        temp[1] = 3;

      if (temp[1] > (n_v))
        temp[1] = (n_v);

      Point3d Etemp = Eval(temp[0], temp[1]);

      if ((Etemp - cp).Length() < (Esurf_p - cp).Length()) {
        step *= 7.1; // 1.5;
        surf_p = temp;

        Esurf_p = Etemp;

      } else
        step /= 6.7; // 1.5;
    }
  }
  if (surf_p[0] < 3)
    surf_p[0] = 3;

  if (surf_p[0] > (n_u))
    surf_p[0] = (n_u);

  if (surf_p[1] < 3)
    surf_p[1] = 3;

  if (surf_p[1] > (n_v))
    surf_p[1] = (n_v);


  if (count >= max_it)
    return false;
  else
    return true;


}

Point3d BSplineSurface::EvalN(double u, double v) {
  return Eval(ConvertU(u), ConvertV(v));
}

Point3d BSplineSurface::NormalEvalN(double u, double v) {
  return NormalEval(ConvertU(u), ConvertV(v));
}

Point3d BSplineSurface::PartialUN(double u, double v) {
  return PartialU(ConvertU(u), ConvertV(v));
}

Point3d BSplineSurface::PartialVN(double u, double v) {
  return PartialU(ConvertU(u), ConvertV(v));
}

Point3d BSplineSurface::ContourEval(double u) {
  Point3d p;
  Point2d k;

  k = ContourInverse(u);
  p = EvalN(k[0], k[1]);

  return p;
}

Point2d BSplineSurface::ContourInverse(double u) {
  Point2d k;
  u = fmod(u, 1.0);

  if (u <= 0.25) {
    k[0] = 0.00;
    k[1] = u / 0.25;
  } else if (u <= 0.5) {
    k[0] = (u - 0.25) / 0.25;
    k[1] = 1.0;
  } else if (u <= 0.75) {
    k[0] = 1.0;
    k[1] = 1.0 - (u - 0.5) / 0.25;
    ;
  } else {
    k[0] = 1.0 - (u - 0.75) / 0.25;
    k[1] = 0.0;
  }
  return k;
}

bool BSplineSurface::PointInversionN(Point2d &surf_p, Point2d p0, Point3d q) {
  bool error;
  surf_p[0] = ConvertU(surf_p[0]);
  surf_p[1] = ConvertV(surf_p[1]);

  p0[0] = ConvertU(p0[0]);
  p0[1] = ConvertV(p0[1]);

  error = PointInversion(surf_p, p0, q);

  surf_p[0] = (surf_p[0] - 3.0) / (n_u - degree);
  surf_p[1] = (surf_p[1] - 3.0) / (n_v - degree);

  return error;
};

BSplineSurface BSplineSurface::Interpolate(const BSplineSurface &BS1,
                                           const BSplineSurface &BS2,
                                           double alpha, int num_rows,
                                           int num_cols) {
  BSplineSurface interpolated = BS1;
  BSplineSurface s1 = BS1;
  BSplineSurface s2 = BS2;

  s1.Resize(num_rows, num_cols);
  s2.Resize(num_rows, num_cols);

  interpolated = s1 * (1 - alpha) + s2 * (alpha);
  return interpolated;
}

unsigned int BSplineSurface::LowestResDim(int i) {
  if (i == 0)
    return (int)Control_points[(int)Control_points.size() - 1].size();

  if (i == 1)
    return (int)Control_points[(int)Control_points.size() - 1][0].size();

  return 0;
}

BSplineSurface BSplineSurface::LowestRes() {
  BSplineSurface l_res = (*this);
  int k = (int)Control_points.size() - 1;
  l_res.Reset(LowestResDim(0), LowestResDim(1));

  for (int i = 0; i < l_res.n_u; i++)
    for (int j = 0; j < l_res.n_v; j++)
      l_res.Control_points[0][i][j] = Control_points[k][i][j];

  return l_res;
}

void BSplineSurface::Resize(unsigned int num_rows, unsigned int num_cols) {
  if (num_rows < LowestResDim(0) || num_cols < LowestResDim(1)) {

    while (num_rows < LowestResDim(0) && num_rows >= MIN_HEIGHT)
      HalveU();
    while (num_cols < LowestResDim(1) && num_cols >= MIN_WIDTH)
      HalveV();

    (*this) = LowestRes();
  }

  if (num_rows > Control_points[0].size() ||
      num_cols > Control_points[0][0].size()) {
    while (num_rows >= 2 * Control_points[0].size() - 3)
      DoubleU();
    while (num_cols >= 2 * Control_points[0][0].size() - 3)
      DoubleV();
  }

}

} // namespace Shapes
