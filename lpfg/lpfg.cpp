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

#include "lpfg.h"
#include "comlineparam.h"
#include "animparam.h"
#ifdef WIN32
#include "view.h"
#else
#include "viewLnx.h"
#endif
#include "funcs.h"
#include "contourarr.h"
#include "glenv.h"
#include "envparams.h"
#include "exception.h"
#include "delay.h"
#include "patch.h"
#include "surface.h"
#include "surfarr.h"
#include "bsurfarr.h"
#include "texturearr.h"

#ifdef _WINDOWS
#include "../Lstudio/cmndefs.h"
#include "lpfgWin.imp"
#endif

#ifdef LINUX
#include "lpfgLnx.imp"
#endif

// interface from the lpfg predefined function to the LPFG object
void Interface::UseView(int vid) { pLpfg->UseView(vid); }

// interface from the lpfg predefined function to the LPFG object
void Interface::CloseView(int vid) { pLpfg->CloseView(vid); }

// interface from the lpfg predefined function to the LPFG object
void Interface::DisplayFrame() { pLpfg->DisplayFrame(); }

void Interface::OutputFrame(const char *name) { pLpfg->SetOutputFrame(name); }

// interface from the lpfg predefined function to the LPFG object
int Interface::StepNo() { return pLpfg->GetLEngine().StepNo(); }

double Interface::GillespieTime() { return pLpfg->GillespieTime(); }

void Interface::SeedGillespie(long seed) { pLpfg->SeedGillespie(seed); }

void Interface::ResetGillespie() { pLpfg->ResetGillespie(); }

LPFG::~LPFG() {
  // reset the global LPFG pointer
  Interface::pLpfg = 0;
#ifdef _WINDOWS
  DestroyIcon(_hDocIcon);
  DestroyMenu(_hWinMenu);
  disposeTablet(_tablet);
#endif
}

void LPFG::Repaint() const {
  // iterate the existing views
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      // if the view is not visible
      if (!pV->IsVisible()){
        // show it
        pV->Show();
      }
        // repaint
#ifdef WIN32
      pV->Repaint();
#else
      pV->update();
#endif
    }
  }
}

void LPFG::UseView(int vid) {
  // ignored in batch mode
  if (!_BatchMode) {
    // just warn if the view already exists
    if (ViewExists(vid)) {
      if (comlineparam.VerboseMode())
        Utils::Message("UseView: view %d already in use\n", vid + 1);
    }
    // otherwise
    else {
      // create it
      _CreateView(vid);
    }
  }
}

void LPFG::CloseView(int vid) {
  // ignored in batch mode
  if (!_BatchMode) {
    // just warn if the view does not exist
    if (!ViewExists(vid)) {
      if (comlineparam.VerboseMode())
        Utils::Message("CloseView: view %d is not in use\n", vid + 1);
    }
    // otherwise
    else {
      // close it
      _CloseView(vid);
      // and reset the pointer
      _aView[vid] = 0;
    }
  }
}

void LPFG::Timer() {
  // delay from the current moment
  Delay delay;
  // if continuing animation
  if (_lengine.StepNo() < _lengine.LastAnimFrame() &&
      !_lengine.StopRequested()) {
    // perform one step
    _Step();
    // and wait for the remainder of the animation timeout period
    delay.Wait(animparam.Timeout());
  }
  // if animating forever
  else if (_mAMode == amForever) {
    // stop
    _lengine.Stop();
    // rewind
    _Rewind();
    // and wait for the remainder of the animation timeout period
    delay.Wait(animparam.Timeout());
  }
  // otherwise (end of animation)
  else {
    // stop
    _lengine.Stop();
    _Stop();
  }
}

void LPFG::_SyncMaster() const {
#ifdef _WINDOWS
  static bool first = true;
  if (first) {
    first = false;
    Utils::Log("     \n");
  }
  if (NULL != _hLSemaphore)
    ReleaseSemaphore(_hLSemaphore, 1, NULL);
#endif
}

int LPFG::CompileOnly() {
  if (_lengine.NewLsystem(comlineparam.Lsystemfile()))
    return 0;
  else
    return 1;
}

void LPFG::_NewLsystem() {
  // clear the temporary successor string which is not empty when
  // creating a new model or rewinding ortherwise.
  Interface::GetSuccessorStorage().Clear();
  _displayFrame = false;
  _outputFrame = false;

  PView pV;
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {

    if (0 != *it) {
      pV = *it;
#ifdef WIN32
      pV->makeCurrent();
#else
      pV->makeCurrent();
#endif
      pV->Clear();
    }
  }

  bool res = _lengine.NewLsystem(comlineparam.Lsystemfile());

  _SyncMaster();
  if (res) {
    if (comlineparam.StartInAnimMode()) {
      _lengine.Rewind();
    } else {
      _lengine.DeriveString();
    }
  }

  _Scale();

  Repaint();
}

void LPFG::_ResetViewNoSync() {
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      pV->ResetView();
    }
  }
}

void LPFG::_Scale() {
  if (animparam.ScaleBetweenFrames()) {

    for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
      if (0 != *it) {
        PView pV = *it;
#ifdef WIN32
        pV->makeCurrent();
#else
        pV->makeCurrent();
#endif

#ifdef WIN32
        if (!animparam.DoubleBuffer())
          glDrawBuffer(GL_FRONT);
#endif
        if (animparam.ClearBetweenFrames()) {
          pV->Clear();
        }
        pV->Scale();
      }
    }
  }
}

void LPFG::_ToggleShowAxis() {
  _show = !_show;
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      pV->ShowAxis(_show);
    }
  }
}

void LPFG::_ResetRotation() {
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      pV->ResetRotation();
    }
  }
}

void LPFG::_ResetPan() {
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      pV->ResetPan();
    }
  }
}

void LPFG::_ResetZoom() {
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      pV->ResetZoom();
    }
  }
}

void LPFG::_ResetRoll() {
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      pV->ResetRoll();
    }
  }
}

void LPFG::_ResetView(bool sync) {
  _RereadColors(false);
  _RereadDrawParams(true);
  if (sync)
    _SyncMaster();
  _ResetViewNoSync();
}

void LPFG::_NewModel() {
  // clear the temporary successor string which is not empty when
  // creating a new model or rewinding ortherwise.
  Interface::GetSuccessorStorage().Clear();
  _displayFrame = false;
  _outputFrame = false;
  Utils::ResetLog();
  functions.Clear();
  bool success = true;
  if (comlineparam.FunctionsSpecified()) {
    success &= functions.Load(comlineparam.FunctionsFile());
  } 
  if (comlineparam.TFunctionsSpecified()){
    success &= functions.Load(comlineparam.TFunctionsFile());
  }
  if (comlineparam.VFunctionsSpecified()){
    success &= functions.Load(comlineparam.VFunctionsFile());
  }
  if (comlineparam.IndividualFunctionsSpecified()) {
    success &= functions.LoadIndividualFunctions(comlineparam.IndividualFunctionsFile());
  }
  if(!success){
    Utils::Message("Warning can't load functions\n");
    return;
  }

  //Loading contours
 
  if (comlineparam.ContoursSpecified()) {
    contours.Clear();
    success &= contours.Load(comlineparam.ContoursFile());
  }
  if (comlineparam.IndividualContoursSpecified()) {
    if (!comlineparam.ContoursSpecified())
      contours.Clear();
    success &= contours.LoadIndividualContours(comlineparam.IndividualContoursFile());
  }

    if(!success){
      Utils::Message("Warning can't load contours\n");
    return;
  }


  surfaces.Reread();
  bsurfaces.Reread();
  _RereadAnimParams();
  _RereadEnvironment(false);

  if (_lengine.NewLsystem(comlineparam.Lsystemfile())) {

    if (comlineparam.StartInAnimMode()) {
      _lengine.Rewind();
    } else {
      _lengine.DeriveString();
    }
  }

  _ResetView(false);
  _SyncMaster();
}

void LPFG::_Step() {
  for (;;) {
    _displayFrame = false;
    _outputFrame = false;
    _lengine.Derive();
    if (animparam.DisplayOnRequest()) {
      if (_displayFrame)
        break;
    } else if (0 ==
               (_lengine.StepNo() - animparam.FirstFrame()) % animparam.Step())
      break;
    // in case that DisplayFrame is never called, or step is too big
    if (_lengine.StepNo() >= _lengine.LastAnimFrame() ||
        _lengine.StopRequested())
      break;
  }
  int i = 0;
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    ++i;
    if (0 != *it) {
      PView pV = *it;
      if (!pV->IsVisible())
        pV->Show();
      pV->DrawStep();
    }
  }
  if (_outputFrame) {
    OutputFrame();
  }
  if (IsRecordingForPovray()) {
    for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
      if (0 != *it) {
        PView pV = *it;
        if (animparam.FrameNumbers() == AnimParam::nfConsecutive) {
          pV->OutputPovray(_frameNo);
        } else
          pV->OutputPovray(_lengine.StepNo());
      }
    }
  }

  if (IsRecording()) {
    if (animparam.FrameNumbers() == AnimParam::nfConsecutive) {
      SaveFrame(_frameNo);
    } else
      SaveFrame(_lengine.StepNo());
  }

  if (IsRecording() || IsRecordingForPovray()) {
    ++_frameNo;
  }
}

bool LPFG::ViewExists(int id) const {
  if (_aView.empty() || _aView.size() < static_cast<size_t>(id + 1))
    return false;
  else if (_aView[id] == 0)
    return false;
  else if (!_aView[id]->isCreated())
    return false;
  else
    return true;
}

bool LPFG::ViewExist() const {
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it)
      return true;
  }
  return false;
}

#ifdef _WINDOWS
void LPFG::ViewClosing(View *pView, int id) {
  if (ViewExists(id))
    Utils::Message("Closing View : %d exists\n",id);
  else
    Utils::Message("Closing View : %d doesn't exist\n",id);

  assert(ViewExists(id));
  assert(pView == _aView[id]);
  _aView[id] = 0;
  if (View::IsClean())
    PostMessage(_hWnd, WM_CLOSE, 0, 0);
#else
void LPFG::ViewClosing(View *, int ) {

  {
    if (_mainWindow) {

      _mainWindow->close();
      delete _mainWindow;
      _mainWindow = 0;
    }
  }

#endif
}

void LPFG::_CloseView(int id) {
  assert(ViewExists(id));
  _aView[id]->Destroy();
}

void LPFG::_Rewind() {
  // clear the temporary successor string which is not empty when
  // creating a new model or rewinding ortherwise.
  Interface::GetSuccessorStorage().Clear();

  // empty the user menu
  UserMenuClear();
  // reset the mouse status
  GetMouseStatus();

  _lengine.Rewind();
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {

    if (0 != *it) {
      PView pV = *it;
      if (!pV->IsVisible())
        pV->Show();

      pV->Clear();
      pV->DrawStep();
    }
  }
  if (IsRecording()) {
    if (animparam.FrameNumbers() == AnimParam::nfConsecutive) {
      SaveFrame(_frameNo);
      ++_frameNo;
    } else
      SaveFrame(_lengine.StepNo());
  }
}

void LPFG::_NewAnimate() {
  _RereadAnimParams();
  _SyncMaster();
}

void LPFG::_NewRender() { _SyncMaster(); }

void LPFG::_NewProjection() { _SyncMaster(); }

void LPFG::_NewHomomorphism() { _SyncMaster(); }

void LPFG::_Rerun() {
  _SyncMaster();
  _Scale();

  switch (_mAMode) {
  case amStill:
    _GenerateStatic();
    break;
  case amRun:
  case amForever:
    _Rewind();
    break;
  }
}

void LPFG::_RereadFunctionsRerun() {
  bool success = true;
  Functions tmpfuns;
  if (comlineparam.FunctionsSpecified()) {
    success &= tmpfuns.Load(comlineparam.FunctionsFile());
  } 
  if (comlineparam.TFunctionsSpecified()){
    success &= tmpfuns.Load(comlineparam.TFunctionsFile());
  }
  if (comlineparam.VFunctionsSpecified()){
    success &= tmpfuns.Load(comlineparam.VFunctionsFile());
  }
  if (comlineparam.IndividualFunctionsSpecified()) {
    success &= tmpfuns.LoadIndividualFunctions(comlineparam.IndividualFunctionsFile());
  }
  
  _SyncMaster();
  if (success){
    functions.reInitialize(tmpfuns);
    _Rerun();
  }
  else
    Utils::Message("Warning error while reading functions\n");
}

void LPFG::_RereadFunctions() {
  Functions tmpfuns;
  bool success = true;
  if (comlineparam.FunctionsSpecified()) {
    success &= tmpfuns.Load(comlineparam.FunctionsFile());
  }
  if (comlineparam.TFunctionsSpecified()){
    success &= tmpfuns.Load(comlineparam.TFunctionsFile());
  }
  if (comlineparam.VFunctionsSpecified()){
    success &= tmpfuns.Load(comlineparam.VFunctionsFile());
  }
  if (comlineparam.IndividualFunctionsSpecified()) {
    success &= tmpfuns.LoadIndividualFunctions(comlineparam.IndividualFunctionsFile());
  }
  if (success){
    functions.reInitialize(tmpfuns);
  }

  _SyncMaster();
}

void LPFG::_RereadContours() {
  bool success = true;
  if (comlineparam.ContoursSpecified()) {
    contours.Clear();
    success = contours.Load(comlineparam.ContoursFile());
  }
  if (comlineparam.IndividualContoursSpecified()) {
    if (!comlineparam.ContoursSpecified())
      contours.Clear();
    success =
        contours.LoadIndividualContours(comlineparam.IndividualContoursFile());
  }

  _SyncMaster();
  if (success)
    _Rerun();
}

void LPFG::_RereadContoursNoRerun() {
  bool success = true;
  if (comlineparam.ContoursSpecified()) {
    contours.Clear();
    success = contours.Load(comlineparam.ContoursFile());
  }
  if (comlineparam.IndividualContoursSpecified()) {
    if (!comlineparam.ContoursSpecified())
      contours.Clear();
    success =
        contours.LoadIndividualContours(comlineparam.IndividualContoursFile());
  }

  _SyncMaster();
}

void LPFG::_RereadCurveXYZRerun() { _SyncMaster(); }

void LPFG::_RereadSurfaces() {

  bool success = surfaces.Reread();
  bsurfaces.Reread();
  if (surfaces.Count() > 0 || bsurfaces.Count() > 0)
    if (success)
      Repaint();
  _SyncMaster();
}

void LPFG::_RereadSurfacesNorepaint() {

  surfaces.Reread();
  bsurfaces.Reread();
  _SyncMaster();
}

void LPFG::_RereadTexturesNorepaint() {
  textures.Reread();
  _SyncMaster();
}

 
void LPFG::_GenerateStatic() {
  _Stop();
  _lengine.DeriveString();
  Repaint();
}

void LPFG::GenerateString() { _lengine.DeriveString(); }

void LPFG::_RereadColors(bool sync) {
  if (comlineparam.ColormapMode()) {
    if (comlineparam.ColormapfileSpecified()) {
      gl.LoadColormap(comlineparam.Colormapfile());
    }
  } else{
    gl.LoadMaterials(comlineparam.Materialfile());
  }

  if (sync) {
    _SyncMaster();
  }
}

bool LPFG::_RereadDrawParams(bool sync) {
  drawparams.Default(false);
  if (comlineparam.DrawparamFileSpecified()) {
    if (!drawparams.Load(comlineparam.DrawparamFile()))
      return false;
    for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
      if ((*it) != nullptr) {
        PView pV = *it;
	(*it)->Showing(false);
	pV->DrawParamsChanged();
      }
    }
  }
  if (sync)
    _SyncMaster();
  return true;
}

void LPFG::_RereadAnimParams(bool sync) {
  animparam.Default();

  if (comlineparam.AnimparamFileSpecified())
    animparam.Load(comlineparam.AnimparamFile());
  if (sync)
    _SyncMaster();
}

void LPFG::_RereadEnvironment(bool sync) {
  envparams.Default();
  if (comlineparam.EnvFileSpecified())
    envparams.Load(comlineparam.EnvironmentFile());
  if (sync)
    _SyncMaster();
}

int LPFG::RunBatchMode() {
  _lengine.StartEnvironment();
  _BatchMode = true;
  if (_lengine.NewLsystem(comlineparam.Lsystemfile())) {
    if (comlineparam.StartInAnimMode()) {
    } else {
      _lengine.DeriveString();

      if (comlineparam.OutputfileSpecified())
        switch (comlineparam.OutputType()) {
        case oText:
        case oUnknown:
          DumpString(comlineparam.Outputfile());
          break;
        case oBinary:
          OutputString(comlineparam.Outputfile());
          break;
        default:
	  Utils::Message("Not implemented in batch mode, try to remove -b option from command line\n");
          break;
        }
      if (!_lengine.Verify(comlineparam.Lsystemfile()))
        return -1;
    }
  }
  return 0;
}

void LPFG::DumpString(const char *outfile) const {
  try {
    std::ofstream trg(outfile);
    if (!trg.is_open())
      throw Exception("Cannot create %s", outfile);
    _lengine.DumpString(trg);
  } catch (Exception e) {
    Utils::Error(e.Msg());
  }
}

void LPFG::Recording() {
  if (IsRecording())
    _recording = false;
  else {
    _recording = true;
    if (animparam.FrameNumbers() == AnimParam::nfConsecutive)
      _frameNo = 1;
  }
}

void LPFG::RecordingForPovray() {
  if (IsRecordingForPovray())
    _recordingForPovray = false;
  else {
    _recordingForPovray = true;
    if (animparam.FrameNumbers() == AnimParam::nfConsecutive)
      _frameNo = 1;
  }
}

void LPFG::OutputString(void) const {
  std::string fname(comlineparam.Lsystemfile());
  size_t dotl = fname.find_last_of('.');
  if (std::string::npos != dotl) {
    ++dotl;
    fname.erase(dotl);
  }
  fname.append("strb");
  return OutputString(fname);
}

void LPFG::OutputString(const std::string &fname) const {
  std::ofstream trg(fname.c_str(),
                    std::ios::out | std::ios::binary | std::ios::trunc);
  if (trg.good())
    OutputString(trg);
  trg.close();
}

void LPFG::LoadString(void) {
  std::string fname(comlineparam.Lsystemfile());
  size_t dotl = fname.find_last_of('.');
  if (std::string::npos != dotl) {
    ++dotl;
    fname.erase(dotl);
  }
  fname.append("strb");
  return LoadString(fname);
}

void LPFG::LoadString(const std::string &fname) {
  std::ifstream src(fname.c_str(), std::ios::in | std::ios::binary);
  if (src.is_open())
    LoadString(src);
  src.close();
}

void LPFG::SaveViewArrangement() {
  const char *ViewArrName = "winparams.cfg";
  std::ofstream trg(ViewArrName);
#ifdef _WINDOWS
  RECT r;
  GetClientRect(_hMDIClient, &r);
#else
  QRect r = _mainWindow->rect();
#endif
  size_t n = 0;
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      float sz[4];
      Rect rc(r);
      (*it)->GetRelSize(rc, sz);
      trg << "window: ";
      if (drawparams.IsValidViewId(n))
        trg << drawparams.ViewName(n);
      else
        trg << "v" << n;
      trg << ' ' << sz[0] << ' ' << sz[1] << ' ' << sz[2] << ' ' << sz[3]
          << std::endl;
      ++n;
    }
  }
}

void LPFG::Step() {
  if (Running())
    _Stop();
  if (_lengine.StepNo() < _lengine.LastAnimFrame() && !_lengine.StopRequested())
    _Step();
}

void LPFG::Rewind() {
  if (Running())
    _Stop();
  _Rewind();
}

void LPFG::Clear() {
  for (Viter it = _aView.begin(); it != _aView.end(); ++it) {
    if (0 != *it) {
      PView pV = *it;
      pV->Clear();
    }
  }
}

float LPFG::vvXmin(int vid) const {
  assert(ViewExists(vid));
  return _aView[vid]->vvXmin();
}

float LPFG::vvYmin(int vid) const {
  assert(ViewExists(vid));
  return _aView[vid]->vvYmin();
}

float LPFG::vvZmin(int vid) const {
  assert(ViewExists(vid));
  return _aView[vid]->vvZmin();
}

float LPFG::vvXmax(int vid) const {
  assert(ViewExists(vid));
  return _aView[vid]->vvXmax();
}

float LPFG::vvYmax(int vid) const {
  assert(ViewExists(vid));
  return _aView[vid]->vvYmax();
}

float LPFG::vvZmax(int vid) const {
  assert(ViewExists(vid));
  return _aView[vid]->vvZmax();
}

float LPFG::vvScale(int vid) const {
  assert(ViewExists(vid));
  return _aView[vid]->vvScale();
}

void LPFG::StopFunction() { _lengine.StopFunction(); }

float Interface::vvXmin(int vid) {
  if (pLpfg->ViewExists(vid))
    return pLpfg->vvXmin(vid);
  else {
    Utils::Message("vvXmin: invalid vid = %d. Returning 0.\n", vid);
    return 0.0f;
  }
}

float Interface::vvYmin(int vid) {
  if (pLpfg->ViewExists(vid))
    return pLpfg->vvYmin(vid);
  else {
    Utils::Message("vvYmin: invalid vid = %d. Returning 0.\n", vid);
    return 0.0f;
  }
}

float Interface::vvZmin(int vid) {
  if (pLpfg->ViewExists(vid))
    return pLpfg->vvZmin(vid);
  else {
    Utils::Message("vvZmin: invalid vid = %d. Returning 0.\n", vid);
    return 0.0f;
  }
}

float Interface::vvXmax(int vid) {
  if (pLpfg->ViewExists(vid))
    return pLpfg->vvXmax(vid);
  else {
    Utils::Message("vvXmax: invalid vid = %d. Returning 0.\n", vid);
    return 0.0f;
  }
}

float Interface::vvYmax(int vid) {
  if (pLpfg->ViewExists(vid))
    return pLpfg->vvYmax(vid);
  else {
    Utils::Message("vvYmax: invalid vid = %d. Returning 0.\n", vid);
    return 0.0f;
  }
}

float Interface::vvZmax(int vid) {
  if (pLpfg->ViewExists(vid))
    return pLpfg->vvZmax(vid);
  else {
    Utils::Message("vvZmax: invalid vid = %d. Returning 0.\n", vid);
    return 0.0f;
  }
}

float Interface::vvScale(int vid) {
  if (pLpfg->ViewExists(vid))
    return pLpfg->vvScale(vid);
  else {
    Utils::Message("vvScale: invalid vid = %d. Returning 1.\n", vid);
    return 1.0f;
  }
}

CameraPosition Interface::GetCameraPosition(int vid) {
  if (!(pLpfg->ViewExists(vid))) {
    Utils::Message("CameraPosition: invalid vid = %d.\n", vid);
    return CameraPosition();
  }
  return pLpfg->GetView(vid)->GetCameraPosition();
}

void Interface::Stop() { pLpfg->StopFunction(); }

void Interface::OutputString(const char *filename) {
  pLpfg->OutputString(std::string(filename));
}

void Interface::LoadString(const char *filename) {
  pLpfg->LoadString(std::string(filename));
}

void LPFG::_Run() {
  _mAMode = amRun;
  StartTimer();
}

void LPFG::_Stop() {
  _mAMode = amStill;
  StopTimer();
}

void LPFG::_Forever() {
  _mAMode = amForever;
  StartTimer();
}

bool LPFG::parametersNeedUpdating() {
  return false;
}

float LPFG::setOrGetParameterf(const char *, float ) {
  return 0.;
}

int LPFG::setOrGetParameteri(const char *, int ) {
  return 0;
}

float LPFG::GetParameterf(const char *) {
  return 0.;
}

int LPFG::GetParameteri(const char *) {
  return 0;
}

void LPFG::SetParameterf(const char *, float ) {
}

void LPFG::SetParameteri(const char *, int ) {
}

void LPFG::DelayWrite() {
}

void LPFG::Write() {
}

void LPFG::OutputFrame()
{
    _outputFrame = false;
    
#ifdef _WINDOWS
    Utils::Message("OutputFrame has not been tested on Windows!\n");
    SaveFrame(Interface::StepNo());
#else
    // get rid of the extension
    size_t lastindex = _outputFrameName.find_last_of("."); 
    string fileNameWithoutExtension = _outputFrameName.substr(0, lastindex);
    string extension = _outputFrameName.substr(lastindex+1);
    _mainWindow->setImageName(fileNameWithoutExtension);
    PixFormat pix_format;

    if (!extension.compare("obj")) {
      OutputObj();
      return;
    }
    else if (!extension.compare("ps")) {
      _mainWindow->OutputPostscript();
      return;
    }
    else if (!extension.compare("bmp")) {
      pix_format = BMP;
    }
    else if (!extension.compare("jpg")) {
      pix_format = JPG;
    }
    else if (!extension.compare("jpeg")) {
      pix_format = JPG;
    }
    else if (!extension.compare("png")) {
      pix_format = PNG;
    }
    else if (!extension.compare("gif")) {
      pix_format = GIF;
    }
    else if (!extension.compare("tiff")) {
      pix_format = TIFF;
    }
    else if (!extension.compare("pdf")) {
      pix_format = PDF;
    }
    else {
      Utils::Message ("Error in OutputFrame: unknown extension %s\n", extension.c_str());
      return;
    }
   
    _mainWindow->setOutputFormat(0);
    _mainWindow->setFormat(pix_format);
    
    // in case, some process is waiting for the output image file, just...
    // write the image to a temporary file first
    // don't use: tmpnam(NULL); it isn't safe? and doesn't work the same on all systems
    string tempName = "tmp" + std::to_string(::getpid()) +
	    fileNameWithoutExtension + std::to_string(Interface::StepNo()) + extension;
    _mainWindow->SaveFrame(tempName.c_str(),_mainWindow->getFilenameId());
    // when finished writing image file, rename to the correct name
    if (rename(tempName.c_str(), _outputFrameName.c_str()) != 0) {
      Utils::Message("Error renaming temp output image file\n");
    }
#endif
}

