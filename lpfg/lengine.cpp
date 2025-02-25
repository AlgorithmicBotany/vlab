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



#include <stack>
#include <iomanip>

#include <sstream>
#include <unistd.h>

#include "lengine.h"
#include "comlineparam.h"
#include "animparam.h"
#include "envparams.h"
#include "exception.h"
#include "environment.h"
#include "funcs.h"
#include "surfarr.h"
#include "contourarr.h"
#include "turtle.h"
#include "succstor.h"
#include "BsurfaceObj.h"
#include "terrain.h"

const __lc_ModuleData *aModuleData = 0;
int LEngine::_sDrawCount = 0;

size_t GetSizeOfParams(int mid) {
  // return the size of parameters
  // in bytes
  return aModuleData[mid].size;
}

const char *GetNameOf(int mid) {
  // return the string
  // representing the module's name
  return aModuleData[mid].Name;
}

LEngine::LEngine()
    : _lstring(eLstringInitSize), _derivedstring(eLstringInitSize) {
  _step = 0;

  InitGillespieXSubi();
  _GillespieTime = 0.0;
  _stopFlag = false;
}

LEngine::~LEngine() {
  if (comlineparam.TimedMode()) {
    std::ofstream fpstrg("fps.txt");
    fpstrg << "FPS: " << _sDrawCount / PerformanceMonitor::TimeGroup(3)
           << std::endl;
  }
}

void LEngine::StartEnvironment() {
  ASSERT(_pEnvironment.get() == 0);
  if (comlineparam.EnvFileSpecified()) {
    try {
      std::unique_ptr<Environment> newenv(new Environment(envparams));
      _pEnvironment = std::move(newenv);
    } catch (Exception e) {
      Utils::Error(e.Msg());
    }
  }
}

bool LEngine::ConnectToLsys() {
  ASSERT(!_dll.Connected());
  // if dll specified at the command line
  if (comlineparam.UseDll())
    // use that name
    _dll.Connect(comlineparam.Dll());
  // otherwise
  else
    // use the default name
    _dll.Connect(LPFGParams::CompiledLsys);

  bool success = _dll.Connected();
  // if successfully connected
  // set pointers to the functions
  // exported from lpfg to the dll
  if (success) {
    {
      typedef void (*pfSetExp)(const __lc_ExportedFromLpfg *);
      pfSetExp fSetExp = 0;
      fSetExp = (pfSetExp)_dll.GetProc("__lc_SetExported");
      if (0 == fSetExp) {
        Utils::Message("Missing __lc_SetExported\n");
        success = false;
      } else {
        __lc_ExportedFromLpfg exported;
        exported.fMessage = Utils::Message;
        exported.fFunc = Interface::Func;
        exported.fPFunc = Interface::PFunc;
        exported.fTFunc = Interface::TFunc;
        exported.fVFunc = Interface::VFunc;
        exported.fFuncTangent = Interface::FuncTangent;
        exported.fCurveX = Interface::CurveX;
        exported.fCurveY = Interface::CurveY;
        exported.fCurveZ = Interface::CurveZ;
        exported.fCurveReset = Interface::CurveReset;
        exported.fCurveSetPoint = Interface::CurveSetPoint;
        exported.fCurveRecalc = Interface::CurveRecalc;
        exported.fCurveV2fPoint = Interface::CurveV2fPoint;
        exported.fCurveV3fPoint = Interface::CurveV3fPoint;
        exported.fCurveV3fNormal = Interface::CurveV3fNormal;
        exported.fCurveScale = Interface::CurveScale;
        exported.fRun = Interface::RunCmnd;
        exported.fGetSurface = Interface::GetSurface;
        exported.fGetSurfacePoint = Interface::GetSurfacePoint;
        exported.fUseView = Interface::UseView;
        exported.fCloseView = Interface::CloseView;
        exported.fDisplayFrame = Interface::DisplayFrame;
        exported.fOutputFrame     = Interface::OutputFrame;
        exported.fvvScale = Interface::vvScale;
        exported.fvvXmax = Interface::vvXmax;
        exported.fvvXmin = Interface::vvXmin;
        exported.fvvYmax = Interface::vvYmax;
        exported.fvvYmin = Interface::vvYmin;
        exported.fvvZmax = Interface::vvZmax;
        exported.fvvZmin = Interface::vvZmin;
        exported.fStop = Interface::Stop;
        exported.fStepNo = Interface::StepNo;
        exported.fGillespieTime = Interface::GillespieTime;
        exported.fSeedGillespie = Interface::SeedGillespie;
        exported.fResetGillespie = Interface::ResetGillespie;
        exported.fRan = Interface::Ran;
        exported.fSeedRan = Interface::SeedRan;
        exported.fOutputString = Interface::OutputString;
        exported.fLoadString = Interface::LoadString;
        exported.fGetMouseStatus = Interface::GetMouseStatus;
        exported.fGetTabletStatus = Interface::GetTabletStatus;
        exported.fUserMenuItem = Interface::UserMenuItem;
        exported.fUserMenuClear = Interface::UserMenuClear;
        exported.fUserMenuChoice = Interface::UserMenuChoice;
        exported.fRunSimulation = Interface::RunSimulation;
        exported.fPauseSimulation = Interface::PauseSimulation;
        exported.fGetCameraPosition = Interface::GetCameraPosition;

        exported.fBsurfaceObjSAdd = BsurfaceObjSAdd;
        exported.fBsurfaceObjSInterp = BsurfaceObjSInterp;
        exported.fBsurfaceObjSGet = BsurfaceObjSGet;
        exported.fBsurfaceObjSSet = BsurfaceObjSSet;
        exported.fGetBsurfaceS = Interface::GetBsurfaceS;

        exported.fBsurfaceObjMAdd = BsurfaceObjMAdd;
        exported.fBsurfaceObjMInterp = BsurfaceObjMInterp;
        exported.fBsurfaceObjMGet = BsurfaceObjMGet;
        exported.fBsurfaceObjMSet = BsurfaceObjMSet;
        exported.fGetBsurfaceM = Interface::GetBsurfaceM;

        exported.fterrainHeightAt = Interface::terrainHeightAt;
        exported.fterrainNormalAt = Interface::terrainNormalAt;
        exported.fterrainVisibilityAll = Interface::terrainVisibilityAll;
        exported.fterrainVisibilityPatch = Interface::terrainVisibilityPatch;
        exported.fscaleTerrainBy = Interface::scaleTerrainBy;

        exported.fSetOrGetParameterf = Interface::SetOrGetParameterf;
        exported.fSetOrGetParameteri = Interface::SetOrGetParameteri;
        exported.fParametersNeedUpdating = Interface::ParametersNeedUpdating;
        exported.fGetParameterf = Interface::GetParameterf;
        exported.fGetParameteri = Interface::GetParameteri;
        exported.fSetParameterf = Interface::SetParameterf;
        exported.fSetParameteri = Interface::SetParameteri;
        exported.fDelayWrite = Interface::DelayWrite;
        exported.fWrite = Interface::Write;

        exported.fLContextReset = Interface::LContextReset;
        exported.fRContextReset = Interface::RContextReset;
        exported.fLNContextReset = Interface::LNContextReset;
        exported.fRNContextReset = Interface::RNContextReset;
        exported.fAdvanceContext = Interface::AdvanceContext;
        exported.fAcceptContext = Interface::AcceptContext;
        exported.fConfirmContext = Interface::ConfirmContext;
        exported.fGetModuleAddr = Interface::GetModuleAddr;

        exported.fGetNextModuleSpot = Interface::GetNextModuleSpot;
        exported.fStartPerformance = Interface::StartPerformance;
        exported.fStopPerformance = Interface::StopPerformance;

        fSetExp(&exported);
      }
    }
  }

  // initialize aModulData array
  if (success)
    aModuleData = _dll.GetModuleData(0);

  // if there were any errors
  if (!success) {
    // print message
    Utils::Message("Cannot connect to dll\n");
    // and disconnect
    DisconnectLsys();
  }

  return success;
}

void LEngine::DisconnectLsys() {
  // disconnect from the dll
  if (_dll.Connected())
    _dll.Disconnect();
  // reset aModuleData array
  aModuleData = 0;
  // empty the string
  _lstring.Clear();
}

int LEngine::LastAnimFrame() const {
  if (ValidLsystem()) {
    // specified in the animation parameters
    if (animparam.LastFrame() != -1)
      return animparam.LastFrame();
    // or derivation length in L-system
    else
      return _dll.DerivationLength();
  }
  // unspecified
  return -1;
}

std::pair<Volume, ViewPos> LEngine::CalculateVolume(int vgrp) const {
  Volume vv;
  ViewPos vp;
  if (ValidLsystem()) {
    CheckNumeric(vgrp);
    ViewTurtle turtle(vv, vp);
    std::stack<ViewTurtle> stack;
    // interpret the string using ViewTurtle
    InterpretString(turtle, stack, _lstring, _dll.InterpretationMaxDepth(),
                    _dll.CurrentGroup(), vgrp);
    if (!stack.empty())
      Utils::Message("Some branches are not terminated\n");
  }
  // if not connected to L-system
  else {
    // assume default viewing volume
    Vector3d v(1.0f, 1.0f, 1.0f);
    vv.Adapt(v);
    v.Set(-1.0f, -1.0f, -1.0f);
    vv.Adapt(v);
  }
  return std::pair<Volume, ViewPos>(vv, vp);
}

bool LEngine::NewLsystem(const char *lfile) {
  // first disconnect
  DisconnectLsys();
  // Stop environment
  if (_pEnvironment.get() != 0 && _pEnvironment->IsRunning())
    _pEnvironment->Stop();
  // preprocess the file
  PreprocessLsystem(lfile, PreprocessedFile());

  // try to translate
  if (!Translate(PreprocessedFile(), TranslatedFile())) {
    Utils::Mark("Error translating L++ to C++");
    return false;
  }

  // try to compile
  CompileLsys(TranslatedFile());
  // try to connect
  bool res = ConnectToLsys();
  // if connected and environmental program present
  if (res && _pEnvironment.get() != 0 && !_pEnvironment->IsRunning()) {
    try {
      // start environment
      _pEnvironment->Start();
    } catch (Exception e) {
      Utils::Error("Cannot start environment. The reason:\n");
      Utils::Message(e.Msg());
    } catch (...) {
      Utils::Error("Cannot start environment\n");
    }
  }
  return res;
}

void LEngine::Rewind() {
  if (ValidLsystem()) {
    // reset step counter
    _step = 0;
    _GillespieTime = 0.0;
    // clear the string
    _lstring.Clear();
    // execute Start statement
    _dll.Start();
    // initialize the string
    Axiom();
    try {
      // derive to the first frame or until Stop called
      while (_step < animparam.FirstFrame() && !StopRequested())
        Derive();
    } catch (const char *msg) {
      Utils::Error(msg);
    }
  }
}

void LEngine::Stop() {
  // execute End statement
  if (!StopRequested())
    _dll.End();
}

void LEngine::PreprocessLsystem(const char *src, const char *trg) const {
  // only if no .dll specified in command line
  if (!comlineparam.UseDll()) {
    Utils::RemoveFile(trg);
    // build the file containing the preprocessor
    // directives that define
    // surface, function, contour identifiers
    const char clopt[] = "clopt";
    BuildDefFile(clopt);
    char bf[256];

    sprintf(bf, "%s %s %s", LPFGParams::PreprocScript, src, trg);
    const int EnvSize = 4096;
    // run the preprocessor
    Utils::Execute(bf, EnvSize);
    // delete the definitions file
    Utils::RemoveFile(clopt);
  }
}

void LEngine::BuildDefFile(const char *dfname) const {
  const char define[] =
#ifdef LINUX
      "-D";
#else
      "/D";
#endif
  std::ofstream trg(dfname);
  {
    // output names of the functions with their identifiers
    for (size_t i = 0; functions.ValidId(i); ++i)
      trg << define << functions.GetName(i) << '='
          << static_cast<unsigned int>(i + 1) << ' ';
  }
  {
    // output names of the contours with their identifiers
    for (size_t i = 0; contours.ValidId(i); ++i)
      trg << define << contours.GetName(i) << '='
          << static_cast<unsigned int>(i) << ' ';
  }
  {
    // output names of the views with their identifiers
    for (size_t i = 0; drawparams.IsValidViewId(i); ++i) {
      if (!drawparams.ViewName(i).empty())
        trg << define << drawparams.ViewName(i) << '='
            << static_cast<unsigned int>(i) << ' ';
    }
  }
  {
    // output names of the surfaces with their identifiers
    for (size_t i = 0; surfaces.ValidId(i); ++i)
      trg << define << surfaces.Get(i).Name() << '='
          << static_cast<unsigned int>(i) << ' ';
  }
}

bool LEngine::Translate(const char *src, const char *trg) const {
  // only if no .dll specified at command line
  if (!comlineparam.UseDll()) {
    Utils::RemoveFile(trg);
    char bf[256];
    // build the command line
    sprintf(bf, "l2c %s %s", src, trg);
    // execute the translator
    int status = Utils::Execute(bf);
    return (status == 0);
  } else
    return true;
}

void LEngine::CompileLsys(const char *) const {
  // only if no .dll specified at command line
  if (!comlineparam.UseDll()) {
    // remove existing dll
    Utils::RemoveFile(LPFGParams::CompiledOLsys);
    Utils::RemoveFile(LPFGParams::CompiledLsys);
    const int EnvSize = 4096;
    // execute the batch file
    Utils::Execute(LPFGParams::CompileScript, EnvSize);
  }
}

void LEngine::DumpString(const Lstring &str) const {
  // output the string
  // with indentation on branches
  LstringIterator iter(str);

  // track the total line length
  size_t llength = 0;
  // and current indentation
  size_t indent = 0;
  const size_t MaxLineLength = 78;
  const size_t IndentSize = 2;
  // iterate the string
  while (!iter.AtEnd()) {
    switch (iter.GetModuleId()) {
    case SB_id: {
      // start new branch in a new line
      Utils::Message("\n");
      size_t i;
      for (i = 0; i < indent; ++i)
        Utils::Message(" ");
      // print SB
      Utils::Message("SB\n");
      // increase indentation
      indent += IndentSize;
      llength = indent;
      for (i = 0; i < indent; ++i)
        Utils::Message(" ");
    } break;
    case EB_id: {
      // end branch in a new line
      Utils::Message("\n");
      // decrease indentation
      if (indent > 0)
        indent -= IndentSize;
      size_t i;
      for (i = 0; i < indent; ++i)
        Utils::Message(" ");
      // print EB
      Utils::Message("EB\n");
      for (i = 0; i < indent; ++i)
        Utils::Message(" ");
      llength = indent;
    } break;
    default: {
      // get name of the module
      const char *lbl = iter.GetModuleName();
      // check to see if it will fit in the current line
      llength += strlen(lbl) + 1;
      if (llength > MaxLineLength) {
        // if not start a new line
        Utils::Message("\n");
        for (size_t i = 0; i < indent; ++i)
          Utils::Message(" ");
        llength = indent;
      }
      // and print the module name
      Utils::Message("%s ", lbl);
    } break;
    }
    ++iter;
  }
  Utils::Message("\n");
}

void LEngine::DumpString(std::ostream &trg) const {
  // dump the string to a text file
  // only module names are output without spaces
  LstringIterator iter(_lstring);
  while (!iter.AtEnd()) {
    trg << iter.GetModuleName();
    ++iter;
  }
}

bool LEngine::InsertReadData(const EnvironmentReply &reply) {
  LstringIterator iter(_lstring);
  if (!iter.ValidPosition(reply.Position(), _dll.NumOfModules() - 1))
    return false;
  iter.MoveTo(reply.Position(), _dll.NumOfModules() - 1);

  switch (iter.GetModuleId()) {
  case E1_id:
    if (reply.Size() != 1)
      return false;
    else {
      __E1_ParamsStruct_ *pParams = reinterpret_cast<__E1_ParamsStruct_ *>(
          const_cast<char *>(iter.Ptr()));
#ifdef NO_MEMCOPY
      pParams->data.Param0 = reply.GetParameter(0);
#else
      __E1_ParamsStruct_ params;
      memcpy(&params, pParams, sizeof(__E1_ParamsStruct_));
      params.data.Param0 = reply.GetParameter(0);
      memcpy(pParams, &params, sizeof(__E1_ParamsStruct_));
#endif
    }
    break;
  case E2_id:
    if (reply.Size() != 2)
      return false;
    else {
      __E2_ParamsStruct_ *pParams = reinterpret_cast<__E2_ParamsStruct_ *>(
          const_cast<char *>(iter.Ptr()));
#ifdef NO_MEMCOPY
      pParams->data.Param0 = reply.GetParameter(0);
      pParams->data.Param1 = reply.GetParameter(1);
#else
      __E2_ParamsStruct_ params;
      memcpy(&params, pParams, sizeof(__E2_ParamsStruct_));
      params.data.Param0 = reply.GetParameter(0);
      params.data.Param1 = reply.GetParameter(1);
      memcpy(pParams, &params, sizeof(__E2_ParamsStruct_));
#endif
    }
    break;
  case EA20_id:
    if (reply.Size() > 20)
      return false;
    else {
      __EA20_ParamsStruct_ *pParams = reinterpret_cast<__EA20_ParamsStruct_ *>(
          const_cast<char *>(iter.Ptr()));
#ifdef NO_MEMCOPY
      for (size_t parameter = 0; parameter < reply.Size(); ++parameter) {
        pParams->data.Param0[parameter] = reply.GetParameter(parameter);
      }
      if (comlineparam.CleanEA20()) {
        for (int parameter = reply.Size();
             parameter < pParams->data.Param0.Length(); ++parameter) {
          pParams->data.Param0[parameter] = 0;
        }
      }
#else
      __EA20_ParamsStruct_ params;
      memcpy(&params, pParams, sizeof(__EA20_ParamsStruct_));
      for (size_t parameter = 0; parameter < reply.Size(); ++parameter) {
        params.data.Param0[parameter] = reply.GetParameter(parameter);
      }
      if (comlineparam.CleanEA20()) {
        for (int parameter = reply.Size();
             parameter < pParams->data.Param0.Length(); ++parameter) {
          params.data.Param0[parameter] = 0;
        }
      }
      // MC 02/2013 - copy the params values was missing:
      memcpy(pParams, &params, sizeof(__EA20_ParamsStruct_));
#endif
    }
    break;
  default:
    return false;
  }

  return true;
}

void LEngine::ReadAnswer() {
  ASSERT(0 != _pEnvironment.get());
  if (_pEnvironment->IsRunning()) {
    _pEnvironment->WaitForReply();
    EnvironmentReply reply;
    _pEnvironment->GetReply(reply);
    while (reply.DataPresent()) {
      if (!InsertReadData(reply))
        Utils::Message(
            "Invalid response received from environment at position %d\n",
            reply.Position());
      _pEnvironment->GetReply(reply);
    }
  }
}

bool LEngine::Verify(const char *szLsystemName) const {
  if (_dll.NumOfVerify() > 0) {
    bool bResult = true;
    LstringIterator iterator(_lstring);
    int iModule = 0;

    for (;;) {
      if (iterator.AtEnd() && iModule == _dll.NumOfVerify())
        break;

      __lc_ModuleIdType expected = -1;
      __lc_ModuleIdType actual = -1;

      if (iModule < _dll.NumOfVerify()) {
        expected = _dll.GetVerify(iModule);
      } else {
        bResult = false;
      }

      if (!iterator.AtEnd()) {
        actual = iterator.GetModuleId();
      } else {
        bResult = false;
      }

      if (expected != actual)
        bResult = false;

      if (iModule < _dll.NumOfVerify()) {
        ++iModule;
      }

      if (!iterator.AtEnd()) {
        ++iterator;
      }
    }

    Utils::Message("Verify: %s\n", bResult ? "Success" : "Fail");

    if (!bResult) {
      const int kFileNameLength = 128;
      char buffer[kFileNameLength];
      {
        sprintf(buffer, "Verify_%s_expected.txt", szLsystemName);
        std::ofstream expectedFile(buffer);
        for (int iModule = 0; iModule < _dll.NumOfVerify(); ++iModule) {
          expectedFile << GetNameOf(_dll.GetVerify(iModule)) << std::endl;
        }
      }

      {
        sprintf(buffer, "Verify_%s_actual.txt", szLsystemName);
        std::ofstream actualFile(buffer);
        LstringIterator iterator(_lstring);
        while (!iterator.AtEnd()) {
          actualFile << iterator.GetModuleName() << std::endl;
          ++iterator;
        }
      }
    }

    return bResult;
  } else {
    return true;
  }
}

void LEngine::OutputString(std::ostream &trg) const {
  LstringIterator iter(_lstring);
  trg.write(iter.Ptr(), static_cast<unsigned int>(_lstring.BytesUsed()));
}

void LEngine::LoadString(std::istream &trg) {
  _lstring.Clear();
  __lc_ModuleIdType mid;
  trg.read(reinterpret_cast<char *>(&mid), sizeof(__lc_ModuleIdType));
  if (mid < 0) {
    Utils::Error("Error loading string : No positive Module ...");
    return;
  }
  if (mid >= _dll.NumOfModules()) {
    Utils::Error("Error loading string : Too many Modules ...");
    return;
  }
  SuccessorStorage ss;
  const int eInitBufSize = 256;
  Utils::RawMemory rm(eInitBufSize);
  while (!trg.eof()) {

    ss.Clear();
    ss.AddOneModule(reinterpret_cast<char *>(&mid), sizeof(__lc_ModuleIdType));
    const size_t psize = GetSizeOfParams(mid) - 2 * sizeof(__lc_ModuleIdType);
    if (psize > 0) {
      rm.SetSizeAtLeast(psize);
      trg.read(rm, static_cast<unsigned int>(psize));
      ss.AddOneModule(rm, psize);
      __lc_ModuleIdType emid;
      trg.read(reinterpret_cast<char *>(&emid), sizeof(__lc_ModuleIdType));

      if (emid != mid) {
        Utils::Error("Error loading string");
        return;
      }

      ss.AddOneModule(reinterpret_cast<char *>(&emid),
                      sizeof(__lc_ModuleIdType));
    }
    _lstring.Add(ss);
    trg.read(reinterpret_cast<char *>(&mid), sizeof(__lc_ModuleIdType));
  }
}

const char *LEngine::TranslatedFile() const {
  return LPFGParams::TranslatedFileName;
}

void LEngine::BuildOutputFilename(std::string &result, int iStep) const {
  result = comlineparam.Lsystemfile();
  size_t dotL = result.find_last_of(".l");
  if (std::string::npos != dotL) {
    result.erase(dotL);
    result.append("str");
  }
  if (iStep != -1) // include step no in the filename
  {
    std::stringstream number;
    number << std::setw(8) << std::setfill('0') << iStep;
    result.insert(dotL - 1, number.str());
  }
}
