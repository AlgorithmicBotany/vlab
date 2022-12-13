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


#define GL_SILENCE_DEPRECATION
#ifdef _WINDOWS
#include <GL/glew.h>
#include <windows.h>
#endif

#ifdef __APPLE__
#include <qgl.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "glenv.h"
#include "surfarr.h"
#include "texturearr.h"
#include "drawparam.h"
#include "utils.h"
#include "exception.h"

GLEnv gl;

GLEnv::GLEnv() { _DefaultLight(); }

GLEnv::~GLEnv() {}

void GLEnv::SetColor(int v) const {
  ASSERT(v >= 0);
  ASSERT(v < LPFGParams::NumOfColors);
  if (_materials.Set()) {
    _materials.Apply(v);
  } else {
    glColor3ubv(_colormap.GetColor(v));
  }
}

void GLEnv::SetSolidColor(int v) const {
  ASSERT(v >= 0);
  ASSERT(v < LPFGParams::NumOfColors);
  ASSERT(_materials.Set());
  _materials.ApplySolidColor(v);
}

bool GLEnv::BindTextures() { return textures.Bind(); }

void GLEnv::DoInit(void *pQ, int viewId) {
  GLUquadricObj *pQObj = reinterpret_cast<GLUquadricObj *>(pQ);
  ClearColor();
  ZBuffer();
  if (_materials.Set()) {
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    for (int i = 0; i < LPFGParams::MaxNumOfLights; ++i)
      _lights[i].Apply(i);
  }
  // Make sure that existing textures are assigned to the surfaces
  for (size_t i = 0; surfaces.ValidId(i); ++i) {
    if (surfaces.IsTextured(i)) {
      size_t txtid = surfaces.TextureId(i);
      if (!textures.IsValid(txtid)) {
        Utils::Message("Invalid texture specified for the surface id = %d\n",
                       i);
        surfaces.DisableTexture(i);
      }
    }
  }

  // these two won't work in a multi-view system.

  // MC - June 2020 - load textures only for the first "view", id==0.
  if (viewId == 0)
    if (!textures.Bind())
      return;

  switch (drawparams.RenderMode()) {
  case DParams::rmWireframe:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glShadeModel(GL_SMOOTH);
    gluQuadricDrawStyle(pQObj, GLU_LINE);
    gluQuadricNormals(pQObj, GLU_FLAT);
    glLineWidth(drawparams.WireframeLineWidth());
    break;
  case DParams::rmFilled:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_FLAT);
    gluQuadricDrawStyle(pQObj, GLU_FILL);
    gluQuadricNormals(pQObj, GLU_FLAT);
    break;
  case DParams::rmShaded:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);
    gluQuadricDrawStyle(pQObj, GLU_FILL);
    gluQuadricNormals(pQObj, GLU_SMOOTH);
    break;
  case DParams::rmShadows: // MC - Oct. 2015 - options for shadows using shaders
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glShadeModel(GL_SMOOTH);
    gluQuadricDrawStyle(pQObj, GLU_FILL);
    gluQuadricNormals(pQObj, GLU_SMOOTH);
  }
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER, 0.1f);
}

void *GLEnv::CreateQuadric() { return gluNewQuadric(); }

void GLEnv::DeleteQuadric(void *pQ) {
  gluDeleteQuadric(reinterpret_cast<GLUquadricObj *>(pQ));
}

void GLEnv::OutputToPOVRay(std::ostream &trg, Vector3d ctr) const {
  // Background color
  {
    float bg[3];
    if (_materials.Set())
      _materials.GetColor(0, bg);
    else
      _colormap.GetColor(0, bg);
    trg << "background { rgb <" << bg[0] << ", " << bg[1] << ", " << bg[2]
        << "> }" << std::endl;
  }
  // Lights
  {
    for (int i = 0; i < LPFGParams::MaxNumOfLights; ++i) {
      LightSource l = _lights[i];
      l.MoveBy(ctr);
      l.OutputToPOVRay(trg);
    }
  }
  // Materials
  {
    for (int i = 0; i < LPFGParams::NumOfColors; ++i) {
      trg << "#declare Material" << i << " = ";
      OutputMatToPOVRay(i, trg);
      trg << std::endl;
    }
  }
}

void GLEnv::OutputMatToPOVRay(int v, std::ostream &trg) const {
  ASSERT(v >= 0);
  ASSERT(v < LPFGParams::NumOfColors);
  if (_materials.Set())
    _materials.OutputToPOVRay(v, trg);
  else {
    float c[3];
    _colormap.GetColor(v, c);
    trg << "pigment { rgb<" << c[0] << ',' << c[1] << ',' << c[2] << "> }";
  }
}

void GLEnv::ClearColor() const {
  // we force to have all the background transparent so we can save we a
  // transparent background
  if (_materials.Set()) {
    const float *cclr = _materials.Emission(0);
    glClearColor(cclr[0], cclr[1], cclr[2], 0.0f);
  } else {
    float c[3];
    _colormap.GetColor(0, c);
    glClearColor(c[0], c[1], c[2], 0.0f);
  }
}

void GLEnv::ClearTransparentColor() const {
  if (_materials.Set()) {
    const float *cclr = _materials.Emission(0);
    glClearColor(cclr[0], cclr[1], cclr[2], 0.0f);
  } else {
    float c[3];
    _colormap.GetColor(0, c);
    glClearColor(c[0], c[1], c[2], 0.0f);
  }
}

Vector3d GLEnv::GetSolidColor(int i) const {
  ASSERT(i >= 0);
  ASSERT(i < LPFGParams::NumOfColors);
  Vector3d res;
  if (_materials.Set()) {
    const float *cclr;
    if (0 == i)
      cclr = _materials.Emission(i);
    else
      cclr = _materials.Diffuse(i);
    res.Set(cclr[0], cclr[1], cclr[2]);
  } else {
    float c[3];
    _colormap.GetColor(i, c);
    res.Set(c[0], c[1], c[2]);
  }
  return res;
}

Vector3d GLEnv::GetTextColor(int i) const {
  ASSERT(i >= 0);
  ASSERT(i < LPFGParams::NumOfColors);
  Vector3d res;
  if (_materials.Set()) {
    const float *cclr;
    cclr = _materials.Emission(i);
    res.Set(cclr[0], cclr[1], cclr[2]);
  } else {
    float c[3];
    _colormap.GetColor(i, c);
    res.Set(c[0], c[1], c[2]);
  }
  return res;
}

void GLEnv::ZBuffer() const {
  if (drawparams.ZBuffer())
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
}

void GLEnv::LoadColormap(const char *mapfile) { _colormap.Load(mapfile); }

void GLEnv::LoadMaterials(const char *matfile) { _materials.Load(matfile); }

void GLEnv::_DefaultLight() {
  _lights[0].Default();
  _lights[0].SetDiffuse(Colors::White);
  _lights[0].SetSpecular(Colors::White);
  _lights[0].TurnOn();
  _lightsCount = 1;
}

void GLEnv::AddLight(const LightSource &light) {
  if (_lightsCount == LPFGParams::MaxNumOfLights)
    throw Exception("Too many lights\n");
  _lights[_lightsCount - 1] = light;
  ++_lightsCount;
}

void GLEnv::ResetLights() {
  LightSource light;
  light.TurnOn();
  _lights[0] = light;
  _lightsCount = 1;
  light.TurnOff();
  for (int i = 1; i < LPFGParams::MaxNumOfLights; ++i)
    _lights[i] = light;
}

// MC - Feb. 2018 - called after viewing transformation to move the light
// sources together with the viewpoint
void GLEnv::ResetLightPositions() {
  for (int i = 0; i < _lightsCount; ++i) {
    float *ls_pos = _lights[i].GetPosition();
    glLightfv(GL_LIGHT0 + i, GL_POSITION, ls_pos);
    if (ls_pos[3] != 0.f) { // update non-directional light source
      float ls_spot[5];
      _lights[i].GetSpotlight(ls_spot);
      glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, ls_spot);
    }
  }
}

/// Returns the LightSource with index id.
LightSource GLEnv::GetLight(int id) {
  if (id < 0 || id >= LPFGParams::MaxNumOfLights)
    throw Exception("Out of range exception in GetLight method.");

  return _lights[id];
}

int GLEnv::GetLightCount() { return _lightsCount; }

void GLEnv::Disk(double inr, double otr, int s, int l, void *pQ) const {
  if (pQ == 0)
    printf("PQ is not intialized\n");

  gluDisk(reinterpret_cast<GLUquadric *>(pQ), inr, otr, s, l);
}

void GLEnv::Sphere(double r, int sl, int st, void *pQ) const {
  gluSphere(reinterpret_cast<GLUquadric *>(pQ), r, sl, st);
}
