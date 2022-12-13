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



#include <sstream>
#include <time.h>

#include "viewLnx.h"
#include "lpfg.h"
#include "glenv.h"
#include "glutils.h"
#include "exception.h"
#include "povray.h"
#include "animparam.h"
#include "comlineparam.h"
#include "texturearr.h"
#include "StdModulesStruct.h"

#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

// NOT USED: #ifdef LINUX
// NOT USED: extern QApplication *pApp; // QQQ IS THIS EVEN USED ANYWHERE????
// NOT USED: #endif

int View::_counter = 0;

#include "mainwindow.cpp"

#define CPFG_COMP_UI

void View::ResetView() {
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->ResetView();
  }

}

void View::SetTitle() {
  char *vlab_obj_name = getenv("VLAB_OBJECT_NAME");
  if (vlab_obj_name != NULL) {
    // Note from Pascal (july 2016): Przemek asked for not displaying the name
    // of the lpfg file to make it consistent with cpfg
    _title = QString("Lpfg: %1").arg(vlab_obj_name);
  } else {
    _title = comlineparam.Lsystemfile();
  }
  if (drawparams.IsValidViewId(_id)) {
    // PASCAL with new multiwindowing we don't give the name of the window
    // anymore
  }
  setWindowTitle(_title);
  // MC - Nov. 2015 - Repaint is not necessary here on Mac or Linux because it
  // is called in ResetView() after SetTitle()
}

void View::DrawParamsChanged() {
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->DrawParamsChanged();
  }
}

void View::ResetRotation() {
   for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->ResetRotation();
  }
}

void View::ResetZoom() {
   for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->ResetZoom();
   }
}

void View::ResetPan() {
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->ResetPan();
  }
}

void View::ResetFrustum() {
}

void View::ResetRoll() {
  for (QVector<GLWidget *>::Iterator it = m_glWidgets.begin(); it != m_glWidgets.end(); ++it){
    if ((*it) != nullptr)
      (*it)->ResetRoll();
   }
}

void View::ShowAxis(bool show) {
  _AxisOn = !show;
  ShowAxis();
}

void View::ShowAxis() {
  _AxisOn = !_AxisOn;
  update();
}

void View::Recording() { _pLpfg->Recording(); }

void View::RecordingForPovray() { _pLpfg->RecordingForPovray(); }

void View::OutputBmp() { _pLpfg->SaveFrame(-1); }

void View::OutputRGB() const {
  _pLpfg->SaveFrame(0);
}

void View::OutputObj() {
  Utils::WaitCursor wc;
  
  // replace local creation of file name (code commented out below)
  // with the use of View object's private member: _filename
  // The below was not correctly numbering the file anyway...
  //std::string fname = _pathToSave + "/" + _imageBaseName;
  //if (_numberingImageName) {
  //  std::stringstream st1, st2;
  //  st1.fill('0');
  //  st1.width(4);
  //  st1 << _id;
  //  fname = fname + st1.str();
  //}
  // BUT _filename contains the .obj extension, which must be removed
  // because the obj output function appends .obj and .mtl
  size_t index = _filename.find_last_of(".");
  std::string fname = _filename.substr(0,index); 

  Volume v = _pLpfg->GetLEngine().CalculateVolume(_id).first;
  _pLpfg->GetLEngine().DrawObj(fname.c_str(), gl, v, _id);
}

void View::OutputPostscript() {
  Utils::WaitCursor wc;

  // replace local creation of file name (code commented out below)
  // with the use of View object's private member: _filename
  // The below was not correctly numbering the file anyway...
  //std::string fname = _pathToSave + "/" + _imageBaseName;
  //if (_numberingImageName) {
  //  std::stringstream st1, st2;
  //  st1.fill('0');
  //  st1.width(4);
  //  st1 << _id;
  //  fname = fname + st1.str();
  //}
  //fname.append(".ps");
  std::string fname = _filename;

  std::ofstream trg(fname.c_str());
  if (!trg.is_open())
    throw Exception("Cannot create %s", fname.c_str());
  // pp is responsible for the postscript
  // prolog and showpage command at the end of output
  Utils::PostscriptPage pp(trg);
  if (_glWidgetClicked == -1)
    _glWidgetClicked = 0;
  Projection *projection = m_glWidgets[_glWidgetClicked]->GetProjection();
  _pLpfg->GetLEngine().DrawPostscript(trg, _id, *projection,
                                      drawparams.ProjectionMode());
  _glWidgetClicked = 0;
}

void View::OutputPovray(const char *FolderName) const {
  (void)FolderName; // prevent unused argument warning
}

// Used for movie mode
void View::OutputPovray(int frameNumber) const {
  size_t FRAME_PADDING_LENGTH = 8;

  char tmp[100];
  sprintf(tmp, "%d", frameNumber);
  std::string folder(tmp);

  if (folder.size() < FRAME_PADDING_LENGTH) {
    std::string temp("");
    for (size_t i = 0; i < FRAME_PADDING_LENGTH - folder.size(); ++i) {
      temp.append("0");
    }
    temp.append(folder);

    folder = temp;
  } else if (folder.size() > FRAME_PADDING_LENGTH) {
    throw Exception("FRAME PAD LENGTH TOO SMALL. ln 693 view.cpp");
  }

  OutputPovray(folder.c_str());
}

void View::OutputPovray() const {
  Utils::Message("*** Starting Export ***\n");

  Utils::WaitCursor wc;
  // replace local creation of file name (code commented out below)
  // with the use of View object's private member: _filename
  // The below was not correctly numbering the file anyway...
  //std::string fname = _pathToSave + "/" + _imageBaseName;
  //if (_numberingImageName) {
  //  std::stringstream st1, st2;
  //  st1.fill('0');
  //  st1.width(4);
  //  st1 << _id;
  //  fname = fname + st1.str();
  //}
  //fname.append(".pov");
  std::string fname = _filename;

  srand(static_cast<unsigned int>(time(NULL)));

  std::ofstream trg(fname.c_str());
  if (!trg.is_open())
    throw Exception("Cannot create output.pov");

  trg << "// Redeclare these in camera.inc if you want different values or "
         "change them here\n";

  trg << "#declare USE_ALPHA_MAPPED_TEXTURES=0;\n";
  trg << "#declare DOUBLE_ILLUMINATE_SURFACES=1;\n";
  trg << "#declare SCALE_AND_POSITION_TREES=0;\n";

  trg << "/*\n";
  switch (drawparams.ProjectionMode()) {
  case DParams::pmParallel:
    PovrayCameraParallel(trg);
    break;
  case DParams::pmPerspective:
    PovrayCameraPerspective(trg);
    break;
  }
  trg << "*/\n";

  if (_pLpfg->IsRecordingForPovray()) {
    trg << "#declare MOVIE_MODE=1;\n";
    trg << "#include \"camera.inc\"\n";
  } else
    trg << "#declare MOVIE_MODE=0;\n";

  std::ofstream scene_trg("POV_Scene.inc");
  if (!scene_trg.is_open())
    throw Exception("Cannot create POV_Scene.inc");

  std::ofstream *surface_trgs = NULL;
  if (textures.NumTextures() > 0)
    surface_trgs = new std::ofstream[textures.NumTextures()];

  std::string baseSurfaceFn("POV_Surfaces");
  for (int i = 0; i < textures.NumTextures(); ++i) {
    std::string surfaceFn(baseSurfaceFn);
    char tmp[10];
    sprintf(tmp, "%d", i);
    surfaceFn.append(tmp);
    surfaceFn.append(".inc");

    surface_trgs[i].open(surfaceFn.c_str());
    if (!surface_trgs[i].is_open()) {
      throw Exception("Cannot create Surface output files");
    }
  }

  std::ofstream terrain_trg("POV_Terrain.inc");
  if (!terrain_trg.is_open())
    throw Exception("Cannot create POV_Terrain.inc");

  std::ofstream layout("Scene.layout");
  if (!layout.is_open())
    throw Exception("Cannot create layout output file!");

  // Interpret String
  _pLpfg->GetLEngine().DrawPOVRay(scene_trg, surface_trgs, terrain_trg, layout,
                                  _id);

  trg << "#include \"Trees\\POV_Terrain.inc\"" << std::endl;
  trg << "#include \"Trees\\POV_Scene.inc\"" << std::endl;

  for (int i = 0; i < textures.NumTextures(); ++i) {
    std::string surfaceFn(baseSurfaceFn);
    char tmp[10];
    sprintf(tmp, "%d", i);
    surfaceFn.append(tmp);
    surfaceFn.append(".inc");
    trg << "#include \"Trees\\" << surfaceFn << "\"" << std::endl;
  }

  trg.close();

  for (int i = 0; i < textures.NumTextures(); ++i)
    surface_trgs[i].close();
  delete[] surface_trgs;

  scene_trg.close();
  terrain_trg.close();
  layout.close();

  //	del is only a valid command on Windows, not Linux or OSX
  //	todo: enumerate the files that need to be deleted and
  //	remove them (without using system())
  //	system("del END_CAP*.inc");

  Utils::Message("*** Export Complete ***\n");
}

/// Standard .ray output format:
///   SURFACES - Describes the materials to be used.
///   HEADER - Describes the view parameters to be used.
///   BBOX - Describes the bounding box of the scene.
///   DATA - Describes the object and its components.
///   TRAILER - Describes transforms to be applied to the scene.
void View::OutputRayshade() {
  if (_glWidgetClicked == -1)
    _glWidgetClicked = 0;
  
  // replaced local creation of file name with the View object's
  // private member: _filename, which tracks image numbering
  // The code below was not correctly numbering the file anyway
  //std::string fname = _pathToSave + "/" + _imageBaseName;
  //if (_numberingImageName) {
  //  std::stringstream st1;
  //  st1.fill('0');
  //  st1.width(4);
  //  st1 << _id;
  //  fname = fname + st1.str();
  //}
  //fname.append(".ray");

  std::string filename = _imageBaseName;
  filename.append(".ray");

  //std::ofstream outputStream(fname.c_str());
  std::ofstream outputStream(_filename);//fname.c_str());
  if (!outputStream.is_open())
    throw Exception("Failed to create rayshade file.");
  Projection *projection = m_glWidgets[_glWidgetClicked]->GetProjection();
  int id = m_glWidgets[_glWidgetClicked]->getId();
  _pLpfg->GetLEngine().DrawRayshade(outputStream, *projection, gl, filename,
                                    id);

  outputStream.close();
  _glWidgetClicked = 0;
}

void View::PovrayCameraParallel(std::ostream &trg) const {
  int glWidgetClicked = _glWidgetClicked;
  if (glWidgetClicked == -1)
    glWidgetClicked = 0;
  Projection *projection = m_glWidgets[glWidgetClicked]->GetProjection();

  trg << "camera\n{\n";
  trg << "orthographic\n";
  Vector3d loc = projection->GetLookFrom();
  trg << "location ";
  POVRay::Dump(trg, loc);
  trg << std::endl;
  Vector3d ctr = projection->GetLookAt();
  trg << "look_at ";
  POVRay::Dump(trg, ctr);
  trg << std::endl;
  Vector3d up = projection->ViewUp();
  trg << "sky ";
  POVRay::Dump(trg, up);
  trg << std::endl;
  trg << "right " << -projection->Scale() << "*4/3*x" << std::endl;
  trg << "up " << projection->Scale() << "*y" << std::endl;
  trg << "}" << std::endl;


  gl.OutputToPOVRay(trg, ctr);
}

void View::PovrayCameraPerspective(std::ostream &trg) const {
  int glWidgetClicked = _glWidgetClicked;
  if (glWidgetClicked == -1)
    glWidgetClicked = 0;
  Projection *projection = m_glWidgets[glWidgetClicked]->GetProjection();

  trg << "camera\n{\n";
  trg << "perspective\n";
  Vector3d loc = projection->GetLookFrom();
  trg << "location ";
  POVRay::Dump(trg, loc);
  trg << std::endl;
  Vector3d ctr = projection->GetLookAt();
  trg << "look_at ";
  POVRay::Dump(trg, ctr);
  trg << std::endl;
  Vector3d up = projection->ViewUp();
  trg << "sky ";
  POVRay::Dump(trg, up);
  trg << std::endl;
  trg << "right " << -projection->Scale() << "*4/3*x" << std::endl;
  trg << "up " << projection->Scale() << "*y" << std::endl;
  trg << "}" << std::endl;

  gl.OutputToPOVRay(trg, ctr);
}

void View::_DrawExpired() const {
  if (!comlineparam.Expired())
    return;
  float mview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, mview);
  float proj[16];
  glGetFloatv(GL_PROJECTION_MATRIX, proj);

  GLRestore depth(GL_DEPTH_TEST);

  glDisable(GL_DEPTH_TEST);

  GLRestore light(GL_LIGHTING);

  glDisable(GL_LIGHTING);

  GLRestore blend(GL_BLEND);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glLineWidth(3.0f);
  Vector3d bg = gl.GetBgColor();
  if (fabs(bg.X() - 0.5f) < 0.1f && fabs(bg.Y() - 0.5f) < 0.1f &&
      fabs(bg.Z() - 0.5f) < 0.1f)
    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
  else
    glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
  glTranslatef(-0.9f, 0.8f, 0.0f);
  const float scale = 0.25f;
  glScalef(scale, scale, scale);
  glRotatef(-45.0f, 0.0f, 0.0f, 1.0f);
  glListBase(_oflist);
  glCallLists(20, GL_UNSIGNED_BYTE, "Expired demo version");

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(mview);
}

void View::OutputView() {
  if (_glWidgetClicked == -1)
    _glWidgetClicked = 0;

  std::stringstream fnm;
  fnm << _pathToSave << "/" << _imageBaseName;
  if (_numberingImageName) {
    std::stringstream st1, st2;
    st1.fill('0');
    st1.width(4);
    st1 << _id;
    fnm << st1.str();
  }
  fnm << "view";

  m_glWidgets[_glWidgetClicked]->OutputView(fnm.str());

  _glWidgetClicked = 0;
}
