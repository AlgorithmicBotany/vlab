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

#include "view.h"
#include "lpfg.h"
#include "glenv.h"
#include "glutils.h"
#include "exception.h"
#include "povray.h"
#include "animparam.h"
#include "comlineparam.h"
#include "texturearr.h"
#include "tropismarr.h"
#include "StdModulesStruct.h"

#include "../Lstudio/cmndefs.h"
#include "resource.h"
#include <GL/glu.h>

int View::_counter = 0;

#include "viewWin.imp"

#define CPFG_COMP_UI

void View::Showing(bool show) {
  if (show) {
    _shown = true;
    CurrentContext cc(this);
    _projection.Reset();

    // MC - Sept. 2016 - moved SetModifiers() before SetVolume(), so viewing
    // volume can be computed from view direction
    if (drawparams.ViewModifiersSet(_id))
      _projection.SetModifiers(drawparams.Modifiers(_id));

    if (drawparams.IsBoundingBoxSet(_id))
      _projection.SetVolume(drawparams.BoundingBox(_id), drawparams.Clip());
    else
      _projection.SetVolumeAndPos(_pLpfg->GetLEngine().CalculateVolume(_id),
                                  drawparams.Clip());

    _projection.Apply(drawparams.ProjectionMode());

    if (_pQ == 0)
      _pQ = gl.CreateQuadric();
    ResetOpenGL();
  } else
    _shown = false;
}

void View::HCenter() {
  _projection.HCenter(_pLpfg->GetLEngine().CalculateVolume(_id).first,
                      drawparams.ProjectionMode());
}

void View::Scale() {
  _projection.Scale(_pLpfg->GetLEngine().CalculateVolume(_id).first,
                    drawparams.Clip(), drawparams.ProjectionMode());
}

void View::_DrawAxis() const {

  if (_AxisOn) {
    GLRestore depth(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    GLRestore light(GL_LIGHTING);
    glDisable(GL_LIGHTING);
    _projection.DrawAxis();
  }
}

void View::ResetView() {
  Showing(true);
  SetTitle();
  Repaint();
}

void View::ResetOpenGL() {
  if (drawparams.StationaryLights()) {
    // MC - Nov. 2015 - fix for stationary lights in lpfg for "new render"
    // selection: lpfg should not apply camera transformation to lights.
    // Recall OpenGL applies the modelview transformation to the light position
    // (when glLightf() is called in LightSource.cpp's Apply function),
    // but, if stationary lights are 'on', lpfg assumes the position is fixed.
    // in the case of positional or spot lights, we want to apply a
    // translation...
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // get the original view direction from the camera translation
    Vector3d viewDir = Vector3d(0, 0, -1); // default view direction set in ResetRotation() in projection.cpp
    if (drawparams.ViewModifiersSet(_id)) {
      const WindowParams::ViewModifiers vm = drawparams.Modifiers(_id);
      viewDir = vm.viewDir;
    }
    // apply the translation
    Vector3d viewShift = -_projection.GetLookAt() + _projection.GetPan() +
                         _projection.ZShift() * viewDir;
    glTranslatef(viewShift.X(), viewShift.Y(), viewShift.Z());

    // reset OpenGL settings
    gl.DoInit(_pQ, _id);

    // put the modelview matrix back so the original camera transformation is
    // applied to the model
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  } else {
    // if stationary lights are 'off', the lpfg should apply the
    // view transformation to the position of the light sources
    gl.DoInit(_pQ, _id);
  }
}

void View::SetTitle() {
  std::string title;
  if (drawparams.IsValidViewId(_id)) {
    title = drawparams.ViewName(_id);
  } else {
    std::stringstream ttl;
    ttl << "view " << _id;
    title = ttl.str();
  }
  SetWindowText(_hWnd, title.c_str());
  Repaint();
  // MC - Nov. 2015 - Repaint is not necessary here on Mac or Linux because it
  // is called in ResetView() after SetTitle()
}

void View::Repaint() {
  CurrentContext cc(this);

  // MC - Oct. 2015 - generate shadow map.
  // this could be done only in DrawStep but this way there is a single shadow
  // map for all view windows
  if (drawparams.RenderMode() == DParams::rmShadows) {
    beginShadowMap();
    _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
    endShadowMap();
  }

  if (!animparam.DoubleBuffer()) {
    glDrawBuffer(GL_FRONT);
  }

  if ((animparam.ClearBetweenFrames())
#ifndef WIN32
      || (!_pView->isInitiated())
#endif
  ) {
    gl.ClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  gl.SetColor(1);

  // Blend has not been set yet shouldn't be restore
  // GLRestore blend(GL_BLEND);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Cull has not been set yet shouldn't be restore
  if (drawparams.BackfaceCulling())
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);

  // MC - Oct. 2015 - use shader if shadow mapping selected
  if (drawparams.RenderMode() == DParams::rmShadows)
    beginMainShader();

  _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
  _DrawAxis();

  if (drawparams.RenderMode() == DParams::rmShadows)
    endMainShader();

  if (PrjVar::IsEvalVer())
    _DrawExpired();
  glFlush();
  if (animparam.DoubleBuffer())
    _SwapBuffers();
  else
    glDrawBuffer(GL_BACK);
}

void View::DrawStep() {
  CurrentContext cc(this);
  // MC - Oct. 2015 - generate shadow map.
  if (drawparams.RenderMode() == DParams::rmShadows) {
    beginShadowMap();
    _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
    endShadowMap();
  }

  if (!animparam.DoubleBuffer())
    glDrawBuffer(GL_FRONT);

  if (animparam.ClearBetweenFrames()) {
    gl.ClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  if (animparam.NewViewBetweenFrames())
    ResetView();
  else if (animparam.HCenterBetweenFrames())
    HCenter();
  else if (animparam.ScaleBetweenFrames())
    Scale();

  gl.SetColor(1);

  // MC - Oct. 2015 - use shader if shadow mapping selected
  if (drawparams.RenderMode() == DParams::rmShadows)
    beginMainShader();

  _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
  _DrawAxis();

  if (drawparams.RenderMode() == DParams::rmShadows)
    endMainShader();

  if (PrjVar::IsEvalVer())
    _DrawExpired();
  glFlush();

  if (animparam.DoubleBuffer())
    _SwapBuffers();
  else
    glDrawBuffer(GL_BACK);

  if (_RecordingOn) {
    _SaveFrame(_pLpfg->GetLEngine().StepNo());
  }
}

void View::DrawParamsChanged() {
  CurrentContext cc(this);
  // reset OpenGL settings
  ResetOpenGL();
  _MakeFont();
  resizeShadowMap();
}

void View::RotateBy(int x, int y) {
  _hasMoved = true;
  _projection.RotateBy(0.5f * x, -0.5f * y);
  {
    CurrentContext cc(this);
    _projection.Apply(drawparams.ProjectionMode());
  }
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else {
    Repaint();
  }
}

void View::ZoomBy(int y) {
  _hasMoved = true;
  const float mlt = powf(1.01f, static_cast<float>(y));
  _projection.ZoomBy(mlt);
  {
    CurrentContext cc(this);
    _projection.Apply(drawparams.ProjectionMode());
  }
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else {
    Repaint();
  }
}

void View::FrustumBy(int y) {
  _hasMoved = true;
  ASSERT(DParams::pmPerspective == drawparams.ProjectionMode());
  const float mlt = powf(1.01f, static_cast<float>(y));
  _projection.FrustumBy(mlt);
  {
    CurrentContext cc(this);
    _projection.Apply(drawparams.ProjectionMode());
  }
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else {
    Repaint();
  }
}

void View::RollBy(int x) {
  _hasMoved = true;
  _projection.RollBy(0.5f * x);
  {
    CurrentContext cc(this);
    _projection.Apply(drawparams.ProjectionMode());
  }
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else {
    Repaint();
  }
}

void View::PanBy(int x, int y) {
  _hasMoved = true;
  _projection.PanBy(x, y);
  {
    CurrentContext cc(this);
    _projection.Apply(drawparams.ProjectionMode());
  }
  if (DParams::rovOn == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  else {
    Repaint();
  }
}

void View::StartIdle() {
  // Only regenerate if something has actually moved
  if (_hasMoved && DParams::rovTriggered == drawparams.RedrawOnViewChange())
    _pLpfg->GenerateStatic();
  _hasMoved = false;
  _pTask = &_IdleTask;
}

void View::StartRotating(int x, int y) {
  _pTask = &_RotateTask;
  _pTask->ButtonDown(x, y);
}

void View::StartZooming(int x, int y) {
  _pTask = &_ZoomTask;
  _pTask->ButtonDown(x, y);
}

void View::StartPanning(int x, int y) {
  _pTask = &_PanTask;
  _pTask->ButtonDown(x, y);
}

void View::StartFrustum(int x, int y) {
  _pTask = &_FrustumTask;
  _pTask->ButtonDown(x, y);
}

void View::StartRoll(int x, int y) {
  _pTask = &_RollTask;
  _pTask->ButtonDown(x, y);
}

void View::MouseMove(int x, int y) { _pTask->MouseMove(x, y); }

void View::ResetRotation() {
  {
    CurrentContext cc(this);
    _projection.ResetRotation();
    if (drawparams.ViewModifiersSet(_id)) {
      const WindowParams &wp = drawparams.GetView(_id);
      Vector3d dir = wp.GetDir(), up = wp.GetUp();
      _projection.SetRotation(dir, up);
    }
    _projection.Apply(drawparams.ProjectionMode());
  }
  Repaint();
}

void View::ResetZoom() {
  {
    CurrentContext cc(this);
    _projection.ResetZoom();
    if (drawparams.ViewModifiersSet(_id)) {
      const WindowParams &wp = drawparams.GetView(_id);
      _projection.SetZoom(wp.Scale());
    }
    _projection.Apply(drawparams.ProjectionMode());
  }
  Repaint();
}

void View::ResetPan() {
  {
    CurrentContext cc(this);
    _projection.ResetPan();
    if (drawparams.ViewModifiersSet(_id)) {
      const WindowParams &wp = drawparams.GetView(_id);
      _projection.SetPan(wp.GetPan());
    }
    _projection.Apply(drawparams.ProjectionMode());
  }
  Repaint();
}

void View::ResetFrustum() {}

void View::ResetRoll() {
  {
    CurrentContext cc(this);
    _projection.ResetRoll();
    _projection.Apply(drawparams.ProjectionMode());
  }
  Repaint();
}

void View::ShowAxis(bool show) {
  _AxisOn = !show;
  ShowAxis();
}

void View::ShowAxis() {
  _AxisOn = !_AxisOn;
  Repaint();
}

void View::Recording() { _pLpfg->Recording(); }

void View::RecordingForPovray() { _pLpfg->RecordingForPovray(); }

void View::OutputBmp() { _pLpfg->SaveFrame(-1); }

void View::OutputRGB() const {
  _pLpfg->SaveFrame(0);
}

void View::InsertX(int x, int y) {
  _DoInsertX(x, y);
  if (comlineparam.DebugMode())
    _pLpfg->GetLEngine().DumpString();
}

void View::InsertPosX(int x, int y) { _DoInsertPosX(x, y); }

const float PickWidth = 3.0f, HalfPickWidth = 1;
const float PickHeight = 3.0f, HalfPickHeight = 1;
const int PickRegionSize = 9;

void View::_DoInsertX(int x, int y) {
  CurrentContext cc(this);
  const int SelectionBufferSize = 1024;
  static GLuint buffer[SelectionBufferSize];
  int hitcount = 0;
  {
    glSelectBuffer(SelectionBufferSize, buffer);
    glSelectionMode rm(hitcount);
    glInitNames();
    float projmtrx[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projmtrx);
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      _projection.PickRegion(static_cast<float>(x), static_cast<float>(y),
                             PickWidth, PickHeight,
                             drawparams.ProjectionMode());
      glMatrixMode(GL_MODELVIEW);
      _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
      glFlush();
    }
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projmtrx);
    glMatrixMode(GL_MODELVIEW);
  }
  if (0 == hitcount) {
    Debug("No hits\n");
    return;
  }
  Debug("Hits: %d\n", hitcount);
  int ix = 0;
  unsigned int minx = UINT_MAX;
  unsigned int minxidx = 0;
  for (int i = 0; i < hitcount; ++i) {
    Debug("Hit # %d\n", i);
    unsigned int n = buffer[ix];
    Debug("Name is %u long\n", n);
    ix++;
    Debug("Zmin = %u\tZmax = %u\n", buffer[ix], buffer[ix + 1]);
    if (buffer[ix] < minx) {
      minx = buffer[ix];
      minxidx = ix - 1;
    }
    ix += 2;
    Debug("Name is: ");
    for (unsigned int j = 0; j < n; ++j) {
      Debug("%u ", buffer[ix]);
      ix++;
    }
    Debug("\n");
  }
  Debug("The closest hit is: %u at %u\n", minx, minxidx);
  int insertpos = buffer[minxidx + 3];
  try {
    __MouseIns_ParamsStructArr_ moduleArr;
    moduleArr.arr[0] = MouseIns_id;
    _pLpfg->GetLEngine().Insert<__MouseIns_ParamsStructArr_>(moduleArr,
                                                             insertpos);
  } catch (char *msg) {
    Debug("Cannot insert X: %s\n", msg);
  }
}

void View::_DoInsertPosX(int x, int y) {
  CurrentContext cc(this);
  const int SelectionBufferSize = 1024;
  static GLuint buffer[SelectionBufferSize];
  int hitcount = 0;
  {
    glSelectBuffer(SelectionBufferSize, buffer);
    glSelectionMode rm(hitcount);
    glInitNames();
    float projmtrx[16];
    glGetFloatv(GL_PROJECTION_MATRIX, projmtrx);
    {
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      _projection.PickRegion(static_cast<float>(x), static_cast<float>(y),
                             PickWidth, PickHeight,
                             drawparams.ProjectionMode());
      glMatrixMode(GL_MODELVIEW);
      _pLpfg->GetLEngine().DrawGL(_projection.ViewNormal(), _id, _flist, _pQ);
      glFlush();
    }
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(projmtrx);
    glMatrixMode(GL_MODELVIEW);
  }
  if (0 == hitcount) {
    Debug("No hits\n");
    return;
  }
  Debug("Hits: %d\n", hitcount);
  int ix = 0;
  unsigned int minx = UINT_MAX;
  unsigned int minxidx = 0;
  for (int i = 0; i < hitcount; ++i) {
    Debug("Hit # %d\n", i);
    unsigned int n = buffer[ix];
    Debug("Name is %u long\n", n);
    ix++;
    Debug("Zmin = %u\tZmax = %u\n", buffer[ix], buffer[ix + 1]);
    if (buffer[ix] < minx) {
      minx = buffer[ix];
      minxidx = ix - 1;
    }
    ix += 2;
    Debug("Name is: ");
    for (unsigned int j = 0; j < n; ++j) {
      Debug("%u ", buffer[ix]);
      ix++;
    }
    Debug("\n");
  }
  Debug("The closest hit is: %u at %u\n", minx, minxidx);
  int insertpos = buffer[minxidx + 3];
  try {
    __MouseInsPos_ParamsStruct_ module;
    MouseStatus &ms = module.data.Param0;
    ms.viewNum = -1;
    ms.viewX = x;
    ms.viewY = y;
    FillMouseStatus(ms);
    ms.lbPushed = true;
    ms.selectPoint =
        (ms.selectDepth * ms.atRear) + ((1. - ms.selectDepth) * ms.atFront);
    __MouseInsPos_ParamsStructArr_ *pArr =
        reinterpret_cast<__MouseInsPos_ParamsStructArr_ *>(&module);
    pArr->arr[0] = MouseInsPos_id;
    _pLpfg->GetLEngine().Insert<__MouseInsPos_ParamsStructArr_>(*pArr,
                                                                insertpos);
  } catch (char *msg) {
    Debug("Cannot insert X: %s\n", msg);
  }
}

void View::OutputObj() const {
  Utils::WaitCursor wc;
  std::string fname(comlineparam.Lsystemfile());
  size_t l = fname.length() - 1;
  fname.erase(l);
  fname.append("obj");
  Volume v = _pLpfg->GetLEngine().CalculateVolume(_id).first;
  _pLpfg->GetLEngine().DrawObj(fname.c_str(), gl, v, _id);
}

void View::OutputFBX() const {
  Utils::Message("In OutputFBX in view.cpp\n");
  Utils::Message("********OutputFBX not yet implemented in Windows\n");
  std::string fname(comlineparam.Lsystemfile());
  size_t l = fname.length() - 1;
  fname.erase(l);
  fname.append("fbx");
}

void View::OutputPostscript() {
  Utils::WaitCursor wc;
  std::string fname(comlineparam.Lsystemfile());
  size_t l = fname.length() - 1;
  fname.erase(l);
  fname.append("ps");
  std::ofstream trg(fname.c_str());
  if (!trg.is_open())
    throw Exception("Cannot create %s", fname.c_str());
  // pp is responsible for the postscript
  // prolog and showpage command at the end of output
  Utils::PostscriptPage pp(trg);

  _pLpfg->GetLEngine().DrawPostscript(trg, _id, _projection,
                                      drawparams.ProjectionMode());
}

void View::OutputPovray(const char *FolderName) const {
  char oldDir[400];
  GetCurrentDirectory(400, oldDir);

  // Create a folder for the export
  std::string command("mkdir ");
  command.append(FolderName);
  system(command.c_str());
  SetCurrentDirectory(FolderName);

  system("mkdir Trees");
  SetCurrentDirectory("Trees");

  OutputPovray();

  if (_pLpfg->IsRecordingForPovray()) {
    // Move the textures in to the proper folder
    system("mkdir Textures");
    SetCurrentDirectory("Textures");

    system("xcopy ..\\..\\..\\MyTextures /e");

    SetCurrentDirectory(oldDir);

    // Copy in the camera setup file (user defined)
    command.clear();
    command.append("copy camera.inc ");
    command.append(FolderName);
    system(command.c_str());
  } else {
    // Put a screenshot in the folder
    SetCurrentDirectory(oldDir);

    _pLpfg->SaveFrame(-1);

    std::string fname(comlineparam.Lsystemfile());
    size_t l = fname.length() - 1;
    fname.erase(l);
    fname.append("bmp");

    command.clear();
    command.append("move ");
    command.append(fname);
    command.append(" ");

    command.append(FolderName);
    system(command.c_str());
  }
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
  std::string fname("..\\");
  fname.append(comlineparam.Lsystemfile());
  size_t l = fname.length() - 1;
  fname.erase(l);
  fname.append("pov");
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
void View::OutputRayshade() const {
  //  Set up the filename, with the extension .ray, need to be aware that POSIX
  //    and WINDOWS handle string operations differently.
  std::string fname("..\\");
  fname.append(comlineparam.Lsystemfile());
  size_t length = fname.length() - 1;
  fname.erase(length);
  fname.append("ray");

  std::string filename = comlineparam.Lsystemfile();
  length = filename.length() - 1;
  filename.erase(length);
  filename.append("ray");

  std::ofstream outputStream(fname.c_str());
  if (!outputStream.is_open())
    throw Exception("Failed to create rayshade file.");

  _pLpfg->GetLEngine().DrawRayshade(outputStream, _projection, gl, filename,
                                    _id);

  outputStream.close();
}

void View::PovrayCameraParallel(std::ostream &trg) const {
  trg << "camera\n{\n";
  trg << "orthographic\n";
  Vector3d loc = _projection.GetLookFrom();
  trg << "location ";
  POVRay::Dump(trg, loc);
  trg << std::endl;
  Vector3d ctr = _projection.GetLookAt();
  trg << "look_at ";
  POVRay::Dump(trg, ctr);
  trg << std::endl;
  Vector3d up = _projection.ViewUp();
  trg << "sky ";
  POVRay::Dump(trg, up);
  trg << std::endl;
  trg << "right " << -_projection.Scale() << "*4/3*x" << std::endl;
  trg << "up " << _projection.Scale() << "*y" << std::endl;
  trg << "}" << std::endl;

  gl.OutputToPOVRay(trg, ctr);
}

void View::PovrayCameraPerspective(std::ostream &trg) const {
  trg << "camera\n{\n";
  trg << "perspective\n";
  Vector3d loc = _projection.GetLookFrom();
  trg << "location ";
  POVRay::Dump(trg, loc);
  trg << std::endl;
  Vector3d ctr = _projection.GetLookAt();
  trg << "look_at ";
  POVRay::Dump(trg, ctr);
  trg << std::endl;
  Vector3d up = _projection.ViewUp();
  trg << "sky ";
  POVRay::Dump(trg, up);
  trg << std::endl;
  trg << "right " << -_projection.Scale() << "*4/3*x" << std::endl;
  trg << "up " << _projection.Scale() << "*y" << std::endl;
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

float View::vvXmin() const { return _projection.MinX(); }

float View::vvYmin() const { return _projection.MinY(); }

float View::vvZmin() const { return _projection.MinZ(); }

float View::vvXmax() const { return _projection.MaxX(); }

float View::vvYmax() const { return _projection.MaxY(); }

float View::vvZmax() const { return _projection.MaxZ(); }

float View::vvScale() const { return 1.0f / _projection.Scale(); }

void View::OutputView() const {
  std::stringstream fnm;
#ifndef _WINDOWS
  fnm << _pathToSave << "/" << _imageBaseName;
  if (_numberingImageName) {
    std::stringstream st1, st2;
    st1.fill('0');
    st1.width(4);
    st1 << _id;
    fnm << st1.str();
  }
  fnm << ".view";

#else
  fnm << "view" << _id << ".vw";
#endif

  //std::cerr << "Save view file to : " << fnm.str() << std::endl;
  std::ofstream trg(fnm.str().c_str());
  trg << "view: " << _id << ' ' << "dir: " << _projection.ViewNormal().X()
      << ' ' << _projection.ViewNormal().Y() << ' '
      << _projection.ViewNormal().Z() << ' '
      << "up: " << _projection.ViewUp().X() << ' ' << _projection.ViewUp().Y()
      << ' ' << _projection.ViewUp().Z() << ' '
      << "pan: " << _projection.GetPan().X() << ' ' << _projection.GetPan().Y()
      << ' ' << _projection.GetPan().Z() << ' '
      << "fov: " << _projection.GetFOV() << ' '
      << "scale: " << _projection.Scale() << ' '
      << "shift: " << _projection.ZShift() << std::endl;

  trg << "box: " << _id << ' ';
  _projection.OutputVolume(trg);

  trg << std::endl;
}

void View::FillMouseStatus(MouseStatus &ms) const
// This assumes that ms.view[XY] are the pixel coordinates of the
// mouse in the viewspace, and the mouse is in this view.
{
#ifndef __APPLE__
  makeCurrent();
#endif
  double depth;
  {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Read depth under pointer from the z-buffer
    float winZ[PickRegionSize];
    glReadPixels(static_cast<int>(ms.viewX - HalfPickWidth),
                 static_cast<int>(viewport[3] - ms.viewY - HalfPickHeight),
                 static_cast<int>(PickWidth), static_cast<int>(PickHeight),
                 GL_DEPTH_COMPONENT, GL_FLOAT, winZ);

    depth = winZ[0];
    for (int i = 1; i < PickRegionSize; i++)
      if (depth > winZ[i])
        depth = winZ[i];
  }
  Vector3d prFront = GetProjection().Unproject(ms.viewX, ms.viewY, 0);
  ms.atFront = V3d(prFront.X(), prFront.Y(), prFront.Z());
  Vector3d prRear = GetProjection().Unproject(ms.viewX, ms.viewY, 1);
  ms.atRear = V3d(prRear.X(), prRear.Y(), prRear.Z());
  ms.atMiddle = 0.5 * (ms.atFront + ms.atRear);
  Vector3d prSelect = GetProjection().Unproject(ms.viewX, ms.viewY, depth);
  ms.selectPoint = V3d(prSelect.X(), prSelect.Y(), prSelect.Z());
  {
    V3d sf = ms.selectPoint - ms.atFront, rf = ms.atRear - ms.atFront;
    ms.selectDepth = (sf * rf) / (rf * rf);
  }
  ms.lbDown = _mouseStatus.lbDown;
}

CameraPosition View::GetCameraPosition(void) const {
  CameraPosition pos;
  // Sometimes, having two separate Vector classes sucks.
  Vector3d vec = _projection.GetLookFrom();
  pos.position = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.GetLookAt();
  pos.lookat = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.ViewNormal();
  pos.head = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.ViewLeft();
  pos.left = V3f(vec.X(), vec.Y(), vec.Z());

  vec = _projection.ViewUp();
  pos.up = V3f(vec.X(), vec.Y(), vec.Z());

  pos.scale = vvScale();
  return pos;
}

// MC - July 2016 - added shadow mapping via GLSL shaders
void View::initShadowMap(void) {
  // change default OpenGL texture to white texel instead of black
  // that way whenever a fragment is not textured, it uses this default white
  // texture it may be faster to use a uniform variable in the shader, but this
  // requires changing lpfg's texturing. Because there is no way to check in the
  // shaders if GL_TEXTURE_2D is enabled, everytime glEnable(GL_TEXTURE_2D) is
  // called a uniform variable would need to set in the shaders to TRUE, and
  // when glDisable(GL_TEXTURE_2D) is called the uniform variable would need to
  // be set to FALSE. Using a default white texture that is applied in the
  // shaders requires almost no changes to lpfg's texturing. Just make sure that
  // when glDisable(GL_TEXTURE_2D) is called, glBindTexture(GL_TEXTURE_2D,0) is
  // called!
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  static GLubyte whiteImage[4] = {255, 255, 255, 255};
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               whiteImage);

  // set the width and height of the shadow map texture, depending on if FBO is
  // supported
  GLfloat texWidth = shadowMapResolution;
  GLfloat texHeight = shadowMapResolution;
  if (!shadowMapFBO_supported) {
    texWidth = _projection.getWindowWidth();
    texHeight = _projection.getWindowHeight();
  }

  // create a shadow-map texture
  glActiveTexture(GL_TEXTURE1);
  glGenTextures(1, &shadowMapTexture);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, texWidth, texHeight, 0,
               GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  GLfloat ones[4] = {1.f, 1.f, 1.f, 1.f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ones);

  // if supported, create a framebuffer object for the shadow map, and bind it
  // to the texture
  if (shadowMapFBO_supported) {
    glGenFramebuffers(1, &shadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           shadowMapTexture, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
      Utils::Message("Warning! the 'render mode: shadows' view option will not work.\n");
      Utils::Message("Failed to make complete frame buffer object. Error( %d),error\n");
    }
    // bind render to back buffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  glActiveTexture(GL_TEXTURE0);

  // setup shaders - set path to directory in L-studio
  static char path[_MAX_PATH + 1], vfile[_MAX_PATH + 1], ffile[_MAX_PATH + 1];
  GetModuleFileName(NULL, path, _MAX_PATH); // get path to lpfg.exe
  size_t l = strlen(path);
  // remove 'lpfg.exe' from the path
  while (path[l - 1] != '\\' && l > 0)
    --l;
  path[l - 1] = '\0';
  // add '\'
  strcat(path, "\\");
  ++l;
  strcat(path, "..\\shaders\\");

  // open main shaders
  strcpy(vfile, path);
  strcpy(ffile, path);
  strcat(vfile, "main_vshader.glsl");
  strcat(ffile, "main_fshader.glsl");
  mainShaderProgramID = initShaders(vfile, ffile);

  // open shadow mapping shaders
  strcpy(vfile, path);
  strcpy(ffile, path);
  strcat(vfile, "shadow_vshader.glsl");
  strcat(ffile, "shadow_fshader.glsl");
  shadowShaderProgramID = initShaders(vfile, ffile);

  // set location of uniform variables
  // for the main shaders
  lightMatrixLocation =
      glGetUniformLocation(mainShaderProgramID, "lightMatrix");
  shadowMapLocation = glGetUniformLocation(mainShaderProgramID, "shadowMapTex");
  shadowColorLocation =
      glGetUniformLocation(mainShaderProgramID, "shadowMapColor");
  textureLocation = glGetUniformLocation(mainShaderProgramID, "texture");
  numLightsLocation = glGetUniformLocation(mainShaderProgramID, "numLights");

  // for the shadow mapping shaders
  textureShadowLocation =
      glGetUniformLocation(shadowShaderProgramID, "texture");
}

void View::freeShadowMap(void) {
  if (mainShaderProgramID)
    glDeleteProgram(mainShaderProgramID);
  if (shadowShaderProgramID)
    glDeleteProgram(shadowShaderProgramID);
  if (shadowMapFBO_supported) {
    glDeleteFramebuffers(1, &shadowMapFBO);
  }
  if (shadowMapTexture)
    glDeleteTextures(1, &shadowMapTexture);
}

GLuint View::initShaders(const char *vertex_file_path,
                         const char *fragment_file_path) {
  // Create the shader ids
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if (VertexShaderStream.is_open()) {
    std::string Line = "";
    while (getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  } else {
    Utils::Message("Cannot open vertex shader %s\n", vertex_file_path);
    Utils::Message("'render mode: shadows' will not work.\n");
    return (0);
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if (FragmentShaderStream.is_open()) {
    std::string Line = "";
    while (getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  } else {
    Utils::Message("Cannot open fragment shader %s\n", fragment_file_path);
    Utils::Message("'render mode: shadows' will not work.\n");
    return (0);
  }

  // Compile the shaders
  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  char const *VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char *InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL,
                       &InfoLogErrorMessage[0]);
    Utils::Message("%s\n", InfoLogErrorMessage);
    return (0);
  }

  // Compile Fragment Shader
  char const *FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char *InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL,
                       &InfoLogErrorMessage[0]);
    Utils::Message("%s\n", InfoLogErrorMessage);
    return (0);
  }

  // Link the program
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  if (Result == GL_FALSE) {
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    char *InfoLogErrorMessage = (char *)malloc(InfoLogLength);
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL,
                        &InfoLogErrorMessage[0]);
     Utils::Message("%s\n", InfoLogErrorMessage);
    return (0);
  }

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

void View::resizeShadowMap(void) {

  if (shadowMapResolution == drawparams.ShadowMapSize() ||
      !shadowMapFBO_supported)
    return;

  shadowMapResolution = drawparams.ShadowMapSize();

  // create a shadow-map texture
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapResolution,
               shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE,
                  GL_COMPARE_R_TO_TEXTURE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  GLfloat ones[4] = {1.f, 1.f, 1.f, 1.f};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, ones);

  // bind default texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void View::beginShadowMap(void) {
  if (shadowShaderProgramID == 0)
    return;

  // set OpenGL depth testing for generating the shadow map
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glClearDepth(1.0);

  // render to shadow buffer
  if (shadowMapFBO_supported) {
    glBindFramebuffer(
        GL_FRAMEBUFFER,
        shadowMapFBO); // set to 0 if rendering shadow map to screen
    glViewport(0, 0, shadowMapResolution, shadowMapResolution);
  } else {
    glDrawBuffer(GL_BACK); // FBO not supported; render to back color buffer
                           // ViewPort is same as lpfg's window: glViewport(0,
                           // 0, _projection.getWindowWidth(),
                           // _projection.getWindowHeight());
  }

  glClear(GL_DEPTH_BUFFER_BIT);

  // to address shadow acne, use polygon offset in generating depth map
  glPolygonOffset(drawparams.ShadowMapOffsetFactor(),
                  drawparams.ShadowMapOffsetUnits());
  glEnable(GL_POLYGON_OFFSET_FILL);

  // get the current view matrix and transform the light "view"
  GLfloat gl_modelview[16];
  OpenGLMatrix mvMatrix;
  if (drawparams.StationaryLights()) {
    glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview);
    mvMatrix = OpenGLMatrix(gl_modelview);
    mvMatrix.Transpose(); // transpose because opengl is column-major order?
  }

  // save and reset the projection and modelview matrix before rendering with
  // the shadow shader because they are used to render from the lights point of
  // view
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // get the light source
  LightSource ls = gl.GetLight(0);
  float *ls_pos = ls.GetPosition();

  // set light projection and view matrix
  if (ls_pos[3] == 0.f) { // directional light source
    // compute light ModelView matrix
    Vector3d lightDir(-ls_pos[0], -ls_pos[1], -ls_pos[2]);
    lightDir.Normalize();
    Vector3d lightUp(0.f, 1.f, 0.f);
    // if light directly above, we need a different 'up' vector
    if (ls_pos[0] == 0.f && ls_pos[2] == 0.f) {
      lightUp = Vector3d(0.f, 0.f, 1.f);
    }

    // apply view matrix
    if (drawparams.StationaryLights()) {
      lightDir.Transform(
          mvMatrix); // thanks to the tranpose, this transformation excludes
                     // translation (rotations only)
      lightUp.Transform(mvMatrix);
    }

    // compute the view volume and its half depth given the light source
    // direction
    Volume lightVV;
    _projection.ComputeViewVolume(lightDir, lightVV);
    float lightVVHalfDepth = 0.5f * Distance(lightVV.Max(), lightVV.Min());
    if (0.f == lightVVHalfDepth)
      lightVVHalfDepth = 0.5f;

    // compute light's position and "lookat" vector (remove panning by mouse)
    Vector3d lightLookAt = _projection.GetLookAt() - _projection.GetPan();
    Vector3d lightPos = lightLookAt - lightVVHalfDepth * lightDir;

    // compute light's view matrix, and set gl_ModelViewMatrix in shader
    gluLookAt(lightPos.X(), lightPos.Y(), lightPos.Z(), lightLookAt.X(),
              lightLookAt.Y(), lightLookAt.Z(), lightUp.X(), lightUp.Y(),
              lightUp.Z());

    // set the gl_projection_matrix in the shadow shader
    glMatrixMode(GL_PROJECTION);
    glOrtho(lightVV.MinX(), lightVV.MaxX(), lightVV.MinY(), lightVV.MaxY(),
            lightVV.MinZ(), lightVV.MaxZ());
  } else {
    float ls_spot[5];
    ls.GetSpotlight(ls_spot);

    if (ls_spot[4] == 180.f) {
      Utils::Message("Warning: Lpfg does not support rendering shadows from "
                     "omnidirectional light sources.\n");
    }

    // compute light ModelView matrix
    Vector3d lightDir(ls_spot[0], ls_spot[1], ls_spot[2]);
    lightDir.Normalize();
    Vector3d lightUp(0.f, 1.f, 0.f);
    // if light directly above, we need a different 'up' vector
    if (lightDir.X() == 0.f && lightDir.Z() == 0.f) {
      lightUp = Vector3d(0.f, 0.f, 1.f);
    }

    Vector3d lightPos =
        Vector3d(ls_pos[0], ls_pos[1], ls_pos[2]) + _projection.GetPan();
    Vector3d lightLookAt = lightPos + lightDir;

    // apply view matrix
    if (drawparams.StationaryLights()) {
      lightPos.Transform(mvMatrix);
      lightLookAt.Transform(mvMatrix);
      lightUp.Transform(mvMatrix);
    }

    // compute light's view matrix, and set gl_ModelViewMatrix in shader
    gluLookAt(lightPos.X(), lightPos.Y(), lightPos.Z(), lightLookAt.X(),
              lightLookAt.Y(), lightLookAt.Z(), lightUp.X(), lightUp.Y(),
              lightUp.Z());

    // compute projection matrix, which sets the gl_projection_matrix in the
    // shadow shader
    glMatrixMode(GL_PROJECTION);
    Vector3d d = lightPos - _projection.GetLookAt();
    // using ls_spot[4] is not correct; it should be the spotCosCutoff (like in
    // the main fragment shader) as this requires the vector from surface to
    // light position, use the maximum spotCutoff of 90
    gluPerspective(90., 1., 1., d.Length() + _projection.Depth());
  }

  // bind the shadow shader for rendering
  glUseProgram(shadowShaderProgramID);

  // set location of default texture (a white pixel) to match
  // glActiveTexture(GL_TEXTURE0)
  glUniform1i(textureShadowLocation, 0);

  // save the light's projection * modelview matrix
  glGetFloatv(GL_PROJECTION_MATRIX, gl_modelview);
  lightMatrix = OpenGLMatrix(gl_modelview);
  glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview);
  lightMatrix.Multiply(gl_modelview);

  // ensure OpenGL in ModelView matrix state before rendering
  glMatrixMode(GL_MODELVIEW);
}

void View::endShadowMap(void) {
  if (shadowShaderProgramID == 0)
    return;

  // release the shadow shader after rendering
  glUseProgram(0);

  // render to main frame buffer depending on what is supported
  if (shadowMapFBO_supported) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  } else {
    // copy back buffer to shadow map texture
    glReadBuffer(GL_BACK);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0,
                     _projection.getWindowWidth(),
                     _projection.getWindowHeight(), 0);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
  glViewport(0, 0, _projection.getWindowWidth(), _projection.getWindowHeight());
  glDisable(GL_POLYGON_OFFSET_FILL);

  // put back the projection and modelview matrix before rendering with main
  // shader
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

void View::beginMainShader(void) {
  if (mainShaderProgramID == 0)
    return;

  glUseProgram(mainShaderProgramID);

  // get the inverse of the view matrix before rendering
  // this undoes view transformations so the correct shadow map coordinate is
  // computed the inverse must be calucated here before the turtle changes the
  // modelview matrix in particular, using the builtin inverse matrices
  // (gl_ModelViewInverse or gl_NormalMatrix) in the shader will not work
  GLfloat modelview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview);
  OpenGLMatrix invViewMatrix(modelview);
  invViewMatrix.Inverse();

  // pass light projection-model-view matrix to main shader program
  const float offset[16] = {0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0,
                            0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0};
  OpenGLMatrix lightModelView(offset);
  lightModelView.Multiply(
      lightMatrix.Buf()); // multiply by saved projection-modelview matrix
  lightModelView.Multiply(
      invViewMatrix.Buf()); // multiply by inverse of current view matrix
                            // (before turtle interpretation)
  glUniformMatrix4fv(lightMatrixLocation, 1, GL_FALSE, lightModelView.Buf());

  // pass texture to main shader - value used in glActiveTexture()
  glUniform1i(textureLocation, 0);

  // bind shadow-map texture for reading - value used in glActiveTexture()
  glUniform1i(shadowMapLocation, 1);

  // pass shadow color to main shader program
  glUniform4f(shadowColorLocation, drawparams.ShadowMapRed(),
              drawparams.ShadowMapGreen(), drawparams.ShadowMapBlue(), 1.0);

  // pass the number of enabled lights (ignore the default light)
  int numlights = gl.GetLightCount() > 1 ? gl.GetLightCount() - 1 : 1;
  glUniform1i(numLightsLocation, numlights);
}

void View::endMainShader(void) {
  if (mainShaderProgramID == 0)
    return;
  glUseProgram(0); // release shader from pipeline
}
