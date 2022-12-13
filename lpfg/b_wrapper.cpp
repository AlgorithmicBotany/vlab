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



#include "b_wrapper.h"

using Shapes::Matrix;

b_wrapper::b_wrapper(const char *line) {
  char fnm[80];
  float scale = -1.0;
  int sDiv, tDiv, txtId;
  float _scale = 1.0f;
  int _TextureId = -1;
  int _divU = -1;
  int _divV = -1;
  std::string _name;
  int res = sscanf(line, "%79s %f %d %d %d", fnm, &scale, &sDiv, &tDiv, &txtId);

  switch (res) {
  case 1: // only filename specified
    break;
  case 2: // filename and scale
    if (scale <= 0.0f)
      throw Exception("Invalid scale value for surface %s", fnm);

    break;
  case 4: // scale, sDiv and tDiv specified
    if (scale <= 0.0f)
      throw Exception("Invalid scale value for surface %s", fnm);
    if (!_uvPrecision.IsValidU(sDiv))
      throw Exception("Invalid s value for surface %s", fnm);
    if (!_uvPrecision.IsValidV(tDiv))
      throw Exception("Invalid t value for surface %d", fnm);
    _uvPrecision.SetU(sDiv);
    _uvPrecision.SetV(tDiv);
    break;
  case 5: // Specified: scake, sDiv, tDiv and txtId
    if (scale <= 0.0f)
      throw Exception("Invalid scale value for surface %s", fnm);
    if (!_uvPrecision.IsValidU(sDiv))
      throw Exception("Invalid s value for surface %s", fnm);
    if (!_uvPrecision.IsValidV(tDiv))
      throw Exception("Invalid t value for surface %d", fnm);
    _uvPrecision.SetU(sDiv);
    _uvPrecision.SetV(tDiv);
    _TextureId = txtId;
    break;
  default:
    throw Exception("Invalid surface specification command\n");
  }

  if (scale <= 0)
    scale = _scale;

  surf.Load(fnm, scale, _TextureId, _divU, _divV);

  // what is _name used for?
  _name = fnm;
  size_t dot = _name.find('.');
  if (std::string::npos != dot)
    _name.erase(dot);
}

b_wrapper::b_wrapper(V3f **v, int N, int M) {

  surf.Reset(N, M);

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++) {
      surf.SetControlMesh(i, j, v[i][j].x, v[i][j].y, v[i][j].z);
    }
}

void b_wrapper::Draw(const UVPrecision &uvprecision) {

  int uDiv = UVPrecision::eUDivDefault;
  if (uvprecision.IsUSpecified())
    uDiv = uvprecision.U();
  else if (_uvPrecision.IsUSpecified())
    uDiv = _uvPrecision.U();

  int vDiv = UVPrecision::eVDivDefault;
  if (uvprecision.IsVSpecified())
    vDiv = uvprecision.V();
  else if (_uvPrecision.IsVSpecified())
    vDiv = _uvPrecision.V();

  surf.Draw(uDiv, vDiv);
}

void b_wrapper::Rayshade(const UVPrecision &uvprecision, std::ofstream &target,
                         bool textured, std::string filename) {
  int uDiv = UVPrecision::eUDivDefault;
  if (uvprecision.IsUSpecified())
    uDiv = uvprecision.U();
  else if (_uvPrecision.IsUSpecified())
    uDiv = _uvPrecision.U();

  int vDiv = UVPrecision::eVDivDefault;
  if (uvprecision.IsVSpecified())
    vDiv = uvprecision.V();
  else if (_uvPrecision.IsVSpecified())
    vDiv = _uvPrecision.V();

  surf.Rayshade(uDiv, vDiv, target, textured, filename);
}
void b_wrapper::PostScript(const UVPrecision &uvprecision, std::string &target,
                           Vector3d clr) {
  int uDiv = UVPrecision::eUDivDefault;
  if (uvprecision.IsUSpecified())
    uDiv = uvprecision.U();
  else if (_uvPrecision.IsUSpecified())
    uDiv = _uvPrecision.U();

  int vDiv = UVPrecision::eVDivDefault;
  if (uvprecision.IsVSpecified())
    vDiv = uvprecision.V();
  else if (_uvPrecision.IsVSpecified())
    vDiv = _uvPrecision.V();

  surf.PostScript(uDiv, vDiv, target, clr);
}
void b_wrapper::DrawObj(const UVPrecision &uvprecision, ObjOutputStore &target,
                        int color, int texture) {
  int uDiv = UVPrecision::eUDivDefault;
  if (uvprecision.IsUSpecified())
    uDiv = uvprecision.U();
  else if (_uvPrecision.IsUSpecified())
    uDiv = _uvPrecision.U();

  int vDiv = UVPrecision::eVDivDefault;
  if (uvprecision.IsVSpecified())
    vDiv = uvprecision.V();
  else if (_uvPrecision.IsVSpecified())
    vDiv = _uvPrecision.V();

  if (!surf.Precomputed())
    surf.Precompute(uDiv, vDiv);

  for (int i = 0; i < surf.DivU(); i++) {
    for (int j = 0; j < surf.DivV(); j++) {
      Vector3d vlist[4];
      surf.GetQuad(i, j, &vlist[0]);
      /*
       * j+1 v4--v3
       *     | \  |
       *     |  \ |
       *   j v1--v2
       *     i    i+1
       */
      size_t nrml =
          target.Normal((vlist[1] - vlist[0]) % (vlist[2] - vlist[1]));
      size_t vtx1 =
          target.Vertex(vlist[0], Vector3d((float)i / surf.DivU(),
                                           (float)j / surf.DivV(), 0));
      size_t vtx2 =
          target.Vertex(vlist[1], Vector3d((float)(i + 1) / surf.DivU(),
                                           (float)j / surf.DivV(), 0));
      size_t vtx3 =
          target.Vertex(vlist[2], Vector3d((float)(i + 1) / surf.DivU(),
                                           (float)(j + 1) / surf.DivV(), 0));
      size_t vtx4 =
          target.Vertex(vlist[3], Vector3d((float)i / surf.DivU(),
                                           (float)(j + 1) / surf.DivV(), 0));
      target.Triangle(vtx4, nrml, vtx2, nrml, vtx1, nrml, color, texture);
      target.Triangle(vtx2, nrml, vtx4, nrml, vtx3, nrml, color, texture);
    }
  }
}

void b_wrapper::GetVolume(Volume &v) {

  double x1, y1, z1;
  double x2, y2, z2;
  Vector3d p;

  surf.BoundingBox(x1, y1, z1, x2, y2, z2);
  p.Set(static_cast<float>(x1), static_cast<float>(y1), static_cast<float>(z1));
  v.Adapt(p);
  p.Set(static_cast<float>(x2), static_cast<float>(y2), static_cast<float>(z2));
  v.Adapt(p);
}

void b_wrapper::Transform(const OpenGLMatrix &mm) {

  Matrix M(3, 3);

  for (int i = 0; i < 3; i++)
    for (int j = 0; j < 3; j++)
      M[i][j] = mm.Get(i + 4 * j);

  surf.Transform(M, mm.Get(3), mm.Get(7), mm.Get(11));
}

V3f b_wrapper::GetControlPoint(unsigned int i, unsigned int j) {

  double x, y, z;
  V3f v;

  if (i > (unsigned int)surf.DimU()) {
    std::cout
        << "Error: invalid choice of i value for static b-spline surface\n";
    i = surf.DimU();
  }
  if (j > (unsigned int)surf.DimV()) {
    std::cout
        << "Error: invalid choice of i value for static b-spline surface\n";
    j = surf.DimV();
  }

  surf.GetControlMesh(i, j, x, y, z);
  v.Set(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));

  return v;
}

void b_wrapper::SetControlPoint(unsigned int i, unsigned int j, V3f v) {

  if (i > (unsigned int)surf.DimU()) {
    std::cout
        << "Error: invalid choice of i value for static b-spline surface\n";
    i = surf.DimU();
  }
  if (j > (unsigned int)surf.DimV()) {
    std::cout
        << "Error: invalid choice of i value for static b-spline surface\n";
    j = surf.DimV();
  }

  surf.SetControlMesh(i, j, v.x, v.y, v.z);
}

void b_wrapper::GetControlNet(V3f **v, int &N, int &M) {

  if (N > surf.DimU())
    N = surf.DimU();
  if (M > surf.DimV())
    M = surf.DimV();

  if (N != surf.DimU() || M != surf.DimV())
    surf.Resize(N, M);

  N = surf.DimU();
  M = surf.DimV();

  for (int i = 0; i < N; i++)
    for (int j = 0; j < M; j++) {
      double x, y, z;
      surf.GetControlMesh(i, j, x, y, z);
      v[i][j].Set(static_cast<float>(x), static_cast<float>(y),
                  static_cast<float>(z));
    }
}

b_wrapper b_wrapper::Interpolate(b_wrapper b1, b_wrapper b2, float alpha, int r,
                                 int c) {

  b_wrapper t;

  t.surf = t.surf.Interpolate(b1.surf, b2.surf, alpha, r, c);

  t._uvPrecision = b1._uvPrecision;

  return t;
}
