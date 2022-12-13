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



#include "turtle.h"
#include "contourarr.h"
#include "texturearr.h"
#include "surfarr.h"
#include "utils.h"
#include "terrain.h"

#include <string>
#include <stdio.h>
#include <stdlib.h>

POVRayTurtle::POVRayTurtle(std::ofstream &trg, std::ofstream *surface_trg_arr,
                           std::ofstream &layout)
    : _CurrentContour(0), _ContourId2(0), _blender(0.0f), _allowBranchGC(false), 
      _trg(trg), _surface_trg_arr(surface_trg_arr), _layoutTrg(layout) {
  _textureV = 0.0f;
  _textureVCoeff = 1.0f;
  fileAppend = new char[100];
  basePosSet = false;
  basePos = Vector3d(0, 0, 0);
  if (terrainData != NULL)
    modelScaleFactor = terrainData->getScale();
  else
    modelScaleFactor = 1;
  oldFn = std::string("");
  currentBB.Set(0, 0, 0, 0, 0, 0);

  _surface_trg_is_used = new bool[textures.NumTextures()];
  for (int i = 0; i < textures.NumTextures(); ++i)
    _surface_trg_is_used[i] = false;
}

void POVRayTurtle::operator=(const POVRayTurtle &src) {
  Turtle::operator=(src);
  _CurrentContour = src._CurrentContour;
  _ContourId2 = src._ContourId2;
  _blender = src._blender;
  _gc = src._gc;
  _CurrentTexture = src._CurrentTexture;
  _textureV = src._textureV;
  _textureVCoeff = src._textureVCoeff;
}

// Utility functions for safly converting values to strings platform independent
void POVRayTurtle::itos(char *s, int s_len, int val) const{
#ifdef WIN32
  sprintf_s(s, s_len, "%d", val);
#else
  snprintf(s, s_len, "%d", val);
#endif
}
void POVRayTurtle::ftos(char *s, int s_len, float val) {
#ifdef WIN32
  sprintf_s(s, s_len, "%f", val);
#else
  snprintf(s, s_len, "%f", val);
#endif
}

void POVRayTurtle::textureDeclaration(std::ofstream &trg) const {
  trg << std::endl;
  textures.OutputToPOVRay(trg, fileAppend);
  trg << std::endl;
}
void POVRayTurtle::surfaceDeclaration(std::ofstream &trg) const {
  trg << std::endl;
  surfaces.OutputToPOVRay(trg, fileAppend);
  trg << std::endl;
}
void POVRayTurtle::terrainDeclaration(std::ofstream &trg) const {
  if (terrainData != NULL) {
    trg << std::endl;
    terrainData->OutputToPOVRay(trg);
    trg << std::endl;
  }
}

void POVRayTurtle::PovRayStart(const char *fn, POVRayMeshMode mode)  {
  meshMode = mode;
  if (oldFn.compare("") != 0) // dont do this on the first occurance
  {
    // Add the record to the layout file
    Vector3d scaledBasePos = basePos * (1.0f / modelScaleFactor);
    _layoutTrg << "[TREE]\n"
               << scaledBasePos.X() << " " << scaledBasePos.Y() << " "
               << scaledBasePos.Z() << "\n"
               << "0 0 0\n1 1 1\n" /* rotation and scale */ << oldFn
               << fileAppend << ".inc"
               << "\n";

    if (mesh != NULL && mesh->Empty() == false)
      delete mesh;

    // Close the previous surfaces union
    for (int i = 0; i < textures.NumTextures(); ++i) {
      std::string surfaceFn(oldFn);
      surfaceFn.append("Surfaces");
      surfaceFn.append(fileAppend);
      char tmp[10];
      itos(tmp, 10, i);
      surfaceFn.append(tmp);
      surfaceFn.append(".inc");

      // If we wrote to it then close it and include it in file, otherwise
      // remove it
      if (_surface_trg_is_used[i]) {
        _surface_trg_arr[i] << "}";

        if (_surface_trg_arr[i].is_open())
          _surface_trg_arr[i].close();

        _trg << "#include \"Trees\\" << surfaceFn << "\"\n";
      } else {
        if (_surface_trg_arr[i].is_open())
          _surface_trg_arr[i].close();

        Utils::Message("Removing Unused File: %s", surfaceFn.c_str());
        bool error = (-1 == remove(surfaceFn.c_str()));

        if (error)
          Utils::Message("   , Error Removing!\n");
        else
          Utils::Message("   Successful\n");
      }
    }

    _trg << "#declare " << oldFn << fileAppend << " = union { \n object {"
         << oldFn << "Branches" << fileAppend << "}\n";

    for (int i = 0; i < textures.NumTextures(); ++i) {
      if (_surface_trg_is_used[i]) {
        _trg << "#if (DOUBLE_ILLUMINATE_SURFACES=1)\n";
        _trg << "object {" << oldFn << "Surfaces" << fileAppend << i
             << " double_illuminate texture { Texture_" << i << fileAppend
             << "} }\n";
        _trg << "#else\n";
        _trg << "object {" << oldFn << "Surfaces" << fileAppend << i
             << " texture { Texture_" << i << fileAppend << "} }\n";
        _trg << "#end\n";
      }
    }

    _trg << "#if (SCALE_AND_POSITION_TREES=1)\n";
    _trg << "translate <" << -basePos.X() << "," << -basePos.Y() << ","
         << -basePos.Z() << ">\n"
         << "scale <" << 1.0 / modelScaleFactor << "," << 1.0 / modelScaleFactor
         << "," << 1.0 / modelScaleFactor << ">\n";
    _trg << "#end\n"
         << "}\n";

    // If we are recording we want the object to make an instance of them selves
    _trg << "#if (MOVIE_MODE=1)\n";
    _trg << "object { " << oldFn << fileAppend
         << "\n#if (SCALE_AND_POSITION_TREES=1)\n translate <"
         << scaledBasePos.X() << " " << scaledBasePos.Y() << " "
         << scaledBasePos.Z() << "> \n#end\n }\n";
    _trg << "#end\n";

    // output the bounding box of the mesh
    Vector3d Max = currentBB.Max();
    Vector3d Min = currentBB.Min();
    Max = Max - basePos;
    Min = Min - basePos;
    Max *= 1.0f / modelScaleFactor;
    Min *= 1.0f / modelScaleFactor;
    _trg << "\n/*[BB]\n"
         << Max.X() << " " << Max.Y() << " " << Max.Z() << "\n"
         << Min.X() << " " << Min.Y() << " " << Min.Z() << "\n*/";
    // output the object name, this allows renaming of the original file
    _trg << "\n/*[OBJECT NAME]\n" << oldFn << fileAppend << "\n*/";

    // get a new "unique number ID"
    itos(fileAppend, 100, rand());
    _trg << "\n//Tree Done, Changing to " << fn << fileAppend << ".inc"
         << std::endl;

    // If we are recording we want to include the next file here so that povray
    // will render all the files in the scene
    if (strcmp(fn, "END_CAP") != 0) {
      _trg << "#if (MOVIE_MODE=1)\n";
      _trg << "#include \"Trees\\" << fn << fileAppend << ".inc\"" << std::endl;
      _trg << "#end\n";
    }

  } else {
    // get a new "unique number ID"
    itos(fileAppend, 100, rand());
    // ADDED FOR MOVIE MODE
    _trg << "#include \"Trees\\" << fn << fileAppend << ".inc\"\n";
  }

  // Create the new filename
  std::string newFn(fn);
  newFn.append(fileAppend);
  newFn.append(".inc");

  _trg.close();

  _trg.open(newFn.c_str());
  if (!_trg.is_open())
    throw Exception("Cannot create povray output file!");

  textureDeclaration(_trg);
  if (mode == Single) {
    surfaceDeclaration(_trg);
  }

  std::string baseSurfaceFn(fn);
  baseSurfaceFn.append("Surfaces");
  baseSurfaceFn.append(fileAppend);

  for (int i = 0; i < textures.NumTextures(); ++i) {
    if (_surface_trg_arr[i].is_open())
      _surface_trg_arr[i].close();

    std::string surfaceFn(baseSurfaceFn);
    char tmp[10];
    itos(tmp, 10, i);
    surfaceFn.append(tmp);
    surfaceFn.append(".inc");

    _surface_trg_arr[i].open(surfaceFn.c_str());
    if (!_surface_trg_arr[i].is_open()) {
      throw Exception("Cannot create Surface output files");
    }

    _surface_trg_is_used[i] = false;
  }

  _trg << "#declare " << fn << "Branches" << fileAppend << "= ";

  for (int i = 0; i < textures.NumTextures(); ++i) {
    if (mode == Instance)
      _surface_trg_arr[i] << "#declare " << fn << "Surfaces" << fileAppend << i
                          << "= mesh {\n";
    else if (mode == Single)
      _surface_trg_arr[i] << "#declare " << fn << "Surfaces" << fileAppend << i
                          << "= union {\n";
  }

  basePosSet = false;
  mesh = new POVRay::Mesh(_trg);
  oldFn = std::string(fn);
}

void POVRayTurtle::_SetColor() const {
  _trg << " texture { Material" << _color << " } ";
}

void POVRayTurtle::Sphere(float radius) const {
  _trg << "sphere { <" << _position.X() << ", " << _position.Y() << ", "
       << _position.Z() << ">, " << radius << std::endl;
  _SetColor();
  _trg << " } " << std::endl;
}

void POVRayTurtle::F(float v) {
  if (v > epsilon) {
    if (_gc.On()) {
      _GCF(v);
      _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
              _ContourId2, _blender);
    } else
      _NormalF(v);
  }
}

void POVRayTurtle::_NormalF(float v) {
  std::string currentTexture;
  if (_CurrentContour == 0) {
    const Vector3d oldpos = _position;
    Turtle::F(v);
    _trg << "cylinder\n"
            "{\n"
            "<"
         << oldpos.X() << ',' << oldpos.Y() << ',' << oldpos.Z()
         << ">,\n"
            "<"
         << _position.X() << ',' << _position.Y() << ',' << _position.Z()
         << ">,\n"
         << 0.5 * Width() << '\n'
         << "open\n";
    _SetColor();
    _trg << "\n}\n";
    return;
  }

  const Vector3d oldpos = _position;

  const float rot[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  Turtle::F(v);

  OpenGLMatrix bgn;
  OpenGLMatrix end;

  {
    bgn.Translate(oldpos.X(), oldpos.Y(), oldpos.Z());
    bgn.Multiply(rot);
    bgn.Scale(0.5f * Width(), 0.5f * Width(), 1.0f);

    end.Translate(_position.X(), _position.Y(), _position.Z());
    end.Multiply(rot);
    end.Scale(0.5f * Width(), 0.5f * Width(), 1.0f);
  }

  const float nrm[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                         _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                         _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                         0.0f,         0.0f,         0.0f,         1.0f};

  const float endtv = _textureV + v * _textureVCoeff;

  {
    const Contour &contour = contours.Get(_CurrentContour);
    Vector3d vtx1;
    vtx1.Transform(contour.Vertex(0), bgn.Buf());
    Vector3d nrm1;
    nrm1.Transform(contour.Normal(0), nrm);
    Vector3d vtx2;
    vtx2.Transform(contour.Vertex(0), end.Buf());
    for (size_t i = 1; i < contour.Divisions(); ++i) {
      Vector3d vtx3;
      vtx3.Transform(contour.Vertex(i), bgn.Buf());
      Vector3d nrm3;
      nrm3.Transform(contour.Normal(i), nrm);

      Vector3d vtx4;
      vtx4.Transform(contour.Vertex(i), end.Buf());

      POVRay::Triangle trngl;
      trngl.SetVertices(vtx1, vtx3, vtx2);
      trngl.SetNormals(nrm1, nrm3, nrm1);

      if (_TextureOn()) {
        Vector3d uv1(((i - 1) * 1.0f / (contour.Divisions() - 1)), _textureV,
                     0.0f);
        Vector3d uv2((i * 1.0f / (contour.Divisions() - 1)), _textureV, 0.0f);
        Vector3d uv3(((i - 1) * 1.0f / (contour.Divisions() - 1)), endtv, 0.0f);
        trngl.SetUV(uv1, uv2, uv3);
      }

      if (!trngl.Empty()) {
        if (_TextureOn()) {
          currentTexture = std::string("Texture_");
          char *temp = new char[20];
          itos(temp, 20, _CurrentTexture);
          currentTexture.append(temp);
          currentTexture.append(fileAppend);
          delete[] temp;
        } else {
          currentTexture = std::string("Material");
          char *temp = new char[20];
          itos(temp, 20, _color);
          currentTexture.append(temp);
          delete[] temp;
        }
        strcpy(trngl.currentTexture, currentTexture.c_str());
        mesh->AddTriangle(trngl);
      }

      trngl.Clear();
      trngl.SetVertices(vtx2, vtx3, vtx4);
      trngl.SetNormals(nrm1, nrm3, nrm3);

      if (_TextureOn()) {
        Vector3d uv1(((i - 1) * 1.0f / (contour.Divisions() - 1)), endtv, 0.0f);
        Vector3d uv2((i * 1.0f / (contour.Divisions() - 1)), _textureV, 0.0f);
        Vector3d uv3((i * 1.0f / (contour.Divisions() - 1)), endtv, 0.0f);
        trngl.SetUV(uv1, uv2, uv3);
      }

      if (!trngl.Empty()) {
        if (_TextureOn()) {
          currentTexture = std::string("Texture_");
          char *temp = new char[20];
          itos(temp, 20, _CurrentTexture);
          currentTexture.append(temp);
          currentTexture.append(fileAppend);
          delete[] temp;
        } else {
          currentTexture = std::string("Material");
          char *temp = new char[20];
          itos(temp, 20, _color);
          currentTexture.append(temp);
          delete[] temp;
        }
        strcpy(trngl.currentTexture, currentTexture.c_str());
        mesh->AddTriangle(trngl);
      }

      vtx1 = vtx3;
      nrm1 = nrm3;
      vtx2 = vtx4;
    }
  }

  if (_TextureOn()) {
    _textureV = endtv;
    if (_textureV > 1.0f)
      _textureV -= 1.0f;
  }
}

void POVRayTurtle::_GCF(float v) {
  ASSERT(_gc.On());
  std::string currentTexture;

  Turtle::F(v);
  OpenGLMatrix bgn;
  OpenGLMatrix end;
  {
    bgn.Translate(_gc.Position().X(), _gc.Position().Y(), _gc.Position().Z());
    {
      const float rot[16] = {_gc.Left().X(),
                             _gc.Left().Y(),
                             _gc.Left().Z(),
                             0.0f,
                             _gc.Up().X(),
                             _gc.Up().Y(),
                             _gc.Up().Z(),
                             0.0f,
                             _gc.Heading().X(),
                             _gc.Heading().Y(),
                             _gc.Heading().Z(),
                             0.0f,
                             0.0f,
                             0.0f,
                             0.0f,
                             1.0f};
      bgn.Multiply(rot);
    }
    bgn.Scale(0.5f * _gc.ScaleP(), 0.5f * _gc.ScaleQ(), 1.0f);
    end.Translate(_position.X(), _position.Y(), _position.Z());
    {
      const float rot[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                             _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                             _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                             0.0f,         0.0f,         0.0f,         1.0f};
      end.Multiply(rot);
    }
    end.Scale(0.5f * _ScaleP(), 0.5f * _ScaleQ(), 1.0f);
  }

  const float bgnnrm[16] = {_gc.Left().X(),
                            _gc.Left().Y(),
                            _gc.Left().Z(),
                            0.0f,
                            _gc.Up().X(),
                            _gc.Up().Y(),
                            _gc.Up().Z(),
                            0.0f,
                            _gc.Heading().X(),
                            _gc.Heading().Y(),
                            _gc.Heading().Z(),
                            0.0f,
                            0.0f,
                            0.0f,
                            0.0f,
                            1.0f};
  const float endnrm[16] = {_left.X(),    _left.Y(),    _left.Z(),    0.0f,
                            _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                            _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                            0.0f,         0.0f,         0.0f,         1.0f};

  float endtv = _textureV + v * _textureVCoeff;
  if (v == 0.f)
    endtv =  _textureV + _textureVCoeff;

  {
    const Contour &bgncntr =
        contours.Get1(_gc.ContourId(), _gc.ContourId2(), _gc.Blender());
    const Contour &endcntr =
        contours.Get2(_CurrentContour, _ContourId2, _blender);
    Vector3d vtx1;
    vtx1.Transform(bgncntr.Vertex(0), bgn.Buf());
    Vector3d nrm1;
    nrm1.Transform(bgncntr.Normal(0), bgnnrm);
    Vector3d vtx2;
    vtx2.Transform(endcntr.Vertex(0), end.Buf());
    Vector3d nrm2;
    nrm2.Transform(endcntr.Normal(0), endnrm);
    for (size_t i = 1; i < bgncntr.Divisions(); ++i) {
      Vector3d vtx3;
      vtx3.Transform(bgncntr.Vertex(i), bgn.Buf());
      Vector3d nrm3;
      nrm3.Transform(bgncntr.Normal(i), bgnnrm);

      Vector3d vtx4;
      vtx4.Transform(endcntr.Vertex(i), end.Buf());
      Vector3d nrm4;
      nrm4.Transform(endcntr.Normal(i), endnrm);

      POVRay::Triangle trngl;
      trngl.SetVertices(vtx1, vtx3, vtx2);
      trngl.SetNormals(nrm1, nrm3, nrm2);

      if (basePosSet == false) {
        basePos = Vector3d(vtx1.X(), vtx1.Y(), vtx1.Z());
        currentBB.Set(vtx1.X(), vtx1.X(), vtx1.Y(), vtx1.Y(), vtx1.Z(),
                      vtx1.Z());
        basePosSet = true;
      } else {
        currentBB.Adapt(vtx1);
        currentBB.Adapt(vtx2);
      }

      if (_TextureOn()) {
        Vector3d uv1(((i - 1) * 1.0f / (bgncntr.Divisions() - 1)), _textureV,
                     0.0f);
        Vector3d uv2((i * 1.0f / (bgncntr.Divisions() - 1)), _textureV, 0.0f);
        Vector3d uv3(((i - 1) * 1.0f / (bgncntr.Divisions() - 1)), endtv, 0.0f);
        trngl.SetUV(uv1, uv2, uv3);
      }

      if (!trngl.Empty()) {
        if (_TextureOn()) {
          currentTexture = std::string("Texture_");
          char *temp = new char[20];
          itos(temp, 20, _CurrentTexture);
          currentTexture.append(temp);
          currentTexture.append(fileAppend);
          delete[] temp;
        } else {
          currentTexture = std::string("Material");
          char *temp = new char[20];
          itos(temp, 20, _color);
          currentTexture.append(temp);
          delete[] temp;
        }
        strcpy(trngl.currentTexture, currentTexture.c_str());
        mesh->AddTriangle(trngl);
      }

      trngl.Clear();
      trngl.SetVertices(vtx2, vtx3, vtx4);
      trngl.SetNormals(nrm2, nrm3, nrm4);

      if (_TextureOn()) {
        Vector3d uv1(((i - 1) * 1.0f / (bgncntr.Divisions() - 1)), endtv, 0.0f);
        Vector3d uv2((i * 1.0f / (bgncntr.Divisions() - 1)), _textureV, 0.0f);
        Vector3d uv3((i * 1.0f / (bgncntr.Divisions() - 1)), endtv, 0.0f);
        trngl.SetUV(uv1, uv2, uv3);
      }

      if (!trngl.Empty()) {
        if (_TextureOn()) {
          currentTexture = std::string("Texture_");
          char *temp = new char[20];
          itos(temp, 20, _CurrentTexture);
          currentTexture.append(temp);
          currentTexture.append(fileAppend);
          delete[] temp;
        } else {
          currentTexture = std::string("Material");
          char *temp = new char[20];
          itos(temp, 20, _color);
          currentTexture.append(temp);
          delete[] temp;
        }
        strcpy(trngl.currentTexture, currentTexture.c_str());
        mesh->AddTriangle(trngl);
      }

      vtx1 = vtx3;
      nrm1 = nrm3;
      vtx2 = vtx4;
      nrm2 = nrm4;
    }
  }


  if (_TextureOn()) {
    _textureV = endtv;
    if (_textureV > 1.0f)
      _textureV -= 1.0f;
  }
}

void POVRayTurtle::StartGC() {
  if (_allowBranchGC) {
    _gc.End();
    _allowBranchGC = false;
  }
  if (!_gc.On()) {
    _gc.Start();
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
    _trg << "// StartGC" << std::endl;
  } else
    Utils::Message("StartGC: Generalized cylinder already started\n");
}

void POVRayTurtle::PointGC() {
  if (_gc.On()) {
    _gc.Set(_Scale, _position, _up, _left, _heading, _CurrentContour,
            _ContourId2, _blender);
  } else
    Utils::Message(
        "PointGC: Generalized cylinder not started, module ignored\n");
}

void POVRayTurtle::EndGC() {
  if (_gc.On()) {
    _gc.End();
    _trg << "// EndGC" << std::endl;
  } else
    Utils::Message("EndGC: Generalized cylinder not started\n");
}

void POVRayTurtle::StartBranch() {
  if (_gc.On() && _allowBranchGC == false) {
    _allowBranchGC = true;
  }
}

void POVRayTurtle::BlendContours(int id1, int id2, float b) {
  if (!contours.ValidId(id1)) {
    Utils::Message("Invalid contour id = %d\n", id1);
    return;
  }
  if (!contours.ValidId(id2)) {
    Utils::Message("Invalid contour id = %d\n", id2);
    return;
  }
  if (b < 0.0f) {
    Utils::Message("Contours blending factor %f < 0.0f\n", b);
    b = 0.0f;
  } else if (b > 1.0f) {
    Utils::Message("Contours blending factor %f > 1.0f\n", b);
    b = 1.0f;
  }
  _CurrentContour = id1;
  _ContourId2 = id2;
  _blender = b;
}

void POVRayTurtle::CurrentContour(int id) {
  if (!contours.ValidId(id)) {
    Utils::Message("Invalid contour id = %d\n", id);
    return;
  }
  _CurrentContour = id;
}

void POVRayTurtle::SetWidth(float w) { ScaleContour(w, w); }

void POVRayTurtle::ScaleContour(float p, float q) {
  Turtle::ScaleContour(p, q);
  _Scale.p = p;
  _Scale.q = q;
}

void POVRayTurtle::Surface(int id, float sx, float sy, float sz) const {
  if (!surfaces.ValidId(id)) {
    Utils::Message("Invalid surface id == %d\n", id);
    return;
  }

  if (meshMode == Instance) {
    OpenGLMatrix rotMat, transMat, scaleMat;

    transMat.Translate(_position);

    const float rot[16] = {-_left.X(),   -_left.Y(),   -_left.Z(),   0.0f,
                           _heading.X(), _heading.Y(), _heading.Z(), 0.0f,
                           _up.X(),      _up.Y(),      _up.Z(),      0.0f,
                           0.0f,         0.0f,         0.0f,         1.0f};

    rotMat.Multiply(rot);

    scaleMat.Scale(sx, sy, sz);

    std::string currentTexture;
    if (_TextureOn()) {
      if (_CurrentTexture != -1)
        surfaces.SetTextureId(id, _CurrentTexture);
      else
        surfaces.DisableTexture(id);
    }
    if (surfaces.IsTextured(id)) {
      currentTexture = std::string("Texture_");
      char *temp = new char[20];
      itos(temp, 20, surfaces.TextureId(id));
      currentTexture.append(temp);
      currentTexture.append(fileAppend);
      delete[] temp;
    } else {
      currentTexture = std::string("Material");
      char *temp = new char[20];
      itos(temp, 20, _color);
      currentTexture.append(temp);
      delete[] temp;
    }

    surfaces.OutputToPOVRay(id, _surface_trg_arr[surfaces.TextureId(id)],
                            currentTexture.c_str(), transMat, rotMat, scaleMat);
  } else if (meshMode == Single) {
    if (surfaces.IsTextured(id)) {
      int texID = surfaces.TextureId(id);
      _surface_trg_arr[texID] << "object { " << std::endl;
      _surface_trg_arr[texID] << "\tobject { " << std::endl;
      _surface_trg_arr[texID] << "\t\tobject { " << std::endl;

      _surface_trg_arr[texID]
          << "\t\t\tSurface_" << id << fileAppend
          << "\n"; //"\t texture { Texture_" << texID << fileAppend <<" }\n";

      _surface_trg_arr[texID] << "\t\t\tscale <" << sx << ',' << sy << ',' << sz
                              << '>' << std::endl;
      _surface_trg_arr[texID] << "\t\t}" << std::endl;
      _surface_trg_arr[texID]
          << "\t\tmatrix <" << -_left.X() << ',' << -_left.Y() << ','
          << -_left.Z() << ',' << _heading.X() << ',' << _heading.Y() << ','
          << _heading.Z() << ',' << _up.X() << ',' << _up.Y() << ',' << _up.Z()
          << ",0.0,0.0,0.0>" << std::endl;

      _surface_trg_arr[texID] << "\t}" << std::endl;
      _surface_trg_arr[texID] << "\ttranslate <" << _position.X() << ','
                              << _position.Y() << ',' << _position.Z() << '>'
                              << std::endl;
      _surface_trg_arr[texID] << "}" << std::endl;
    }

  }

  _surface_trg_is_used[surfaces.TextureId(id)] = true;
}

#define _DumpVector(a) '<' << a.X() << ',' << a.Y() << ',' << a.Z() << '>'
#define _DumpPair(a, b) '<' << a << ',' << b << '>'

void POVRayTurtle::Rhombus(float a, float b) const {
  _trg << "mesh { " << std::endl;
  Vector3d v1(_position);
  Vector3d v2(_position + _heading * a);

  Vector3d v(_position + _heading * a * 0.5f);
  v += _left * b * 0.5f;

  _trg << "\ttriangle { " << _DumpVector(v1) << ',';
  _trg << _DumpVector(v) << ',';
  _trg << _DumpVector(v2) << " }" << std::endl;

  v = _position + _heading * a * 0.5f - _left * b * 0.5f;

  _trg << "\ttriangle { " << _DumpVector(v1) << ',';
  _trg << _DumpVector(v2) << ',';
  _trg << _DumpVector(v) << " }" << std::endl;

  _trg << "\t";
  _SetColor();
  _trg << std::endl << "}" << std::endl;
}

void POVRayTurtle::Triangle(float a, float b) const {
  _trg << "mesh { " << std::endl;

  Vector3d v(_position + _left * a * 0.5f);
  _trg << "\ttriangle { " << _DumpVector(v) << ',';
  v = _position + _heading * b;
  _trg << _DumpVector(v) << ',';
  v = _position - _left * a * 0.5;
  _trg << _DumpVector(v) << " }" << std::endl;

  _trg << "\t";
  _SetColor();
  _trg << std::endl << "}" << std::endl;
}

void POVRayTurtle::Label(const char *t) const {
  _trg << "text {" << std::endl;
  _trg << "ttf \"timrom.ttf\" \"" << t << "\" 1 0" << std::endl;
  _trg << "\t";
  _SetColor();
  _trg << "\ttranslate " << _DumpVector(_position) << std::endl;
  _trg << std::endl << "}" << std::endl;
}

void POVRayTurtle::Terrain(CameraPosition) const{
  if (terrainData == NULL) {
    Utils::Message("Terrain Not Loaded!");
    return;
  }

  _trg << "object { " << std::endl;
  _trg << "\tunion{ " << std::endl;

  terrainData->OutputUnionToPOVRay(_trg);
  if (terrainData->IsTextured()) {
    int texID = terrainData->TextureId();
    _trg << "\t texture { Texture_" << texID << fileAppend << " }";
  } else {
    _trg << "\t\t";
    _SetColor();
    _trg << std::endl;
  }
  _trg << "\t\tmatrix <" << -_left.X() << ',' << -_left.Y() << ',' << -_left.Z()
       << ',' << _heading.X() << ',' << _heading.Y() << ',' << _heading.Z()
       << ',' << _up.X() << ',' << _up.Y() << ',' << _up.Z() << ",0.0,0.0,0.0>"
       << std::endl;

  _trg << "\ttranslate <" << _position.X() << ',' << _position.Y() << ','
       << _position.Z() << '>' << std::endl;

  _trg << "}" << std::endl;
  _trg << "}" << std::endl;
}
