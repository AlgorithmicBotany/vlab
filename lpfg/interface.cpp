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



#ifndef LINUX
#include <rand48/rand.h>
#endif

#include "interface.h"
#include "succstor.h"
#include "lstring.h"
#include "utils.h"
#include "funcs.h"
#include "comlineparam.h"
#include "contourarr.h"
#include "surfarr.h"
#include "bsurfarr.h"
#include "BsurfaceObj.h"
#include "terrain.h"
#include "lpfg.h"
#include "ContextScanner.h"
#include "PerformanceMonitor.h"

#ifdef LINUX
using namespace Qt;
#endif // LINUX

// the one and only successor storage
// used by all kinds of rules:
// productions, interpretation and decomposition.
// Would be nice to somehow hide it.
// Maybe inside LPFG or LEngine?
static SuccessorStorage succstrg;

// Pointer to the (only)
// instance of LPFG.
// Set and reset in LPFG::LPFG and LPFG::~LPFG respectively
LPFG *Interface::pLpfg = 0;

SuccessorStorage &Interface::GetSuccessorStorage() { return succstrg; }

float Interface::VFunc(int id) {
  return functions.GetDefaultValue(id-1);
}

float Interface::PFunc(int id, float v, float lowLimit, float upLimit) {
    // functions indices accessible to the user are 1-based
    int id1 = id-1;
    if (!functions.ValidId(id1)) {
      Utils::Message("Pfunc: Invalid id = %d. Returning 0.\n", id);
      return 0.0f;
    }
    functions.SetLowLimit(id1, lowLimit);
    functions.SetUpLimit(id1, upLimit);
    return TFunc(id, v);
}

float Interface::TFunc(int id, float v) {
    // functions indices accessible to the user are 1-based
    int id1 = id-1;
    if (!functions.ValidId(id1)) {
      Utils::Message("Tfunc: Invalid id = %d. Returning 0.\n", id);
      return 0.0f;
    }
    float v1 = v = (v - functions.LowLimit(id1)) /
                 (functions.UpLimit(id1) - functions.LowLimit(id1));
    return Func(id, v1); 
}

float Interface::Func(int id, float v) {
  // functions indices accessible to the user are 1-based
  --id;
  // if the id is invalid return 0
  if (!functions.ValidId(id)) {
    Utils::Message("func: Invalid id = %d Returning 0.\n", id + 1);
    return 0.0f;
  }
  //[PASCAL] Convert v to the timeMapping
  //[MIK] Moved to TFunc
  //float v1 = v = (v - functions.LowLimit(id)) /
  //               (functions.UpLimit(id) - functions.LowLimit(id));

  // if argument value is invalid
  // assume 0 for values less than 0
  if (v < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("func: Invalid v = %f for id = %d. v = 0.0 assumed\n", v,
                     id + 1);
    v = 0.0f;
  }
  // or 1 for values greater than 1
  else if (v > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("func: Invalid v = %f for id = %d. v = 1.0 assumed\n", v,
                     id + 1);
    v = 1.0f;
  }

  return functions.GetValue(id, v);
}

// SHOULD THERE BE P/T/FuncTangent?

__lc_V2f Interface::FuncTangent(int id, float v) {
  // functions indices accessible to the user are 1-based
  --id;
  // if the id is invalid return 0
  if (!functions.ValidId(id)) {
    Utils::Message("funcTangent: Invalid id = %d. Returning (0,0).\n", id + 1);
    __lc_V2f ans;
    ans.x = ans.y = 0.0f;
    return ans;
  }

  //[PASCAL] Convert v to the timeMapping (could be made in functions.GetTangent
  //...)
  float v1 = v = (v - functions.LowLimit(id)) /
                 (functions.UpLimit(id) - functions.LowLimit(id));

  // if argument value is invalid
  // assume 0 for values less than 0
  if (v1 < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message(
          "funcTangent: Invalid v = %f for id = %d. v = 0.0 assumed\n", v,
          id + 1);
    v1 = 0.0f;
  }
  // or 1 for values greater than 1
  else if (v1 > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message(
          "funcTangent: Invalid v = %f for id = %d. v = 1.0 assumed\n", v,
          id + 1);
    v1 = 1.0f;
  }

  Vector3d tang = functions.GetTangent(id, v1);
  __lc_V2f ans;
  ans.x = tang.X();
  ans.y = tang.Y();
  return ans;
}

float Interface::CurveX(int id, float t) {
  // if the contour id is invalid
  // return 0
  if (!contours.ValidId(id)) {
    Utils::Message("curveX: Invalid curve id = %d. Returning 0.\n", id);
    return 0;
  }
  // for invalid values of t
  // assume 0 for values less than 0
  else if (t < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("curveX: t = %f for id = %d. t = 0.0 assumed\n", t, id);
    t = 0.0f;
  }
  // or 1 for values greater than 1
  else if (t > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("curveX: t = %f for id = %d. t = 1.0 assumed\n", t, id);
    t = 1.0f;
  }

  return contours.Get(id).GetX(t);
}

float Interface::CurveY(int id, float t) {
  // if the contour id is invalid
  // return 0
  if (!contours.ValidId(id)) {
    Utils::Message("curveY: Invalid curve id = %d. Returning 0.\n", id);
    return 0;
  }
  // for invalid values of t
  // assume 0 for values less than 0
  else if (t < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("curveY: t = %f for id = %d. t = 0.0 assumed\n", t, id);
    t = 0.0f;
  }
  // or 1 for values greater than 1
  else if (t > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("curveY: t = %f for id = %d. t = 1.0 assumed\n", t, id);
    t = 1.0f;
  }

  return contours.Get(id).GetY(t);
}

float Interface::CurveZ(int id, float t) {
  // if the contour id is invalid
  // return 0
  if (!contours.ValidId(id)) {
    Utils::Message("curveZ: Invalid curve id = %d. Returning 0.\n", id);
    return 0;
  }
  // for invalid values of t
  // assume 0 for values less than 0
  else if (t < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("curveZ: t = %f for id = %d. t = 0.0 assumed\n", t, id);
    t = 0.0f;
  }
  // or 1 for values greater than 1
  else if (t > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("curveZ: t = %f for id = %d. t = 1.0 assumed\n", t, id);
    t = 1.0f;
  }

  return contours.Get(id).GetZ(t);
}

__lc_V3f Interface::CurveV3fPoint(int id, float t) {
  __lc_V3f ans;
  ans.x = ans.y = ans.z = 0;

  // if the contour id is invalid
  // return (0,0,0)
  if (!contours.ValidId(id)) {
    Utils::Message("CurveV3fPoint: Invalid curve id = %d. Returning (0,0,0).\n",
                   id);
    return ans;
  }
  // for invalid values of t
  // assume 0 for values less than 0
  else if (t < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("CurveV3fPoint: t = %f for id = %d. t = 0.0 assumed\n", t,
                     id);
    t = 0.0f;
  }
  // or 1 for values greater than 1
  else if (t > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("CurveV3fPoint: t = %f for id = %d. t = 1.0 assumed\n", t,
                     id);
    t = 1.0f;
  }

  V3f ansX = contours.Get(id).GetV3fPoint(t);
  ans.x = ansX.x;
  ans.y = ansX.y;
  ans.z = ansX.z;
  return ans;
}

__lc_V3f Interface::CurveV3fNormal(int id, float t) {
  __lc_V3f ans;
  ans.x = ans.y = ans.z = 0;

  // if the contour id is invalid
  // return (0,0,0)
  if (!contours.ValidId(id)) {
    Utils::Message(
        "CurveV3fNormal: Invalid curve id = %d. Returning (0,0,0).\n", id);
    return ans;
  }
  // for invalid values of t
  // assume 0 for values less than 0
  else if (t < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("CurveV3fNormal: t = %f for id = %d. t = 0.0 assumed\n", t,
                     id);
    t = 0.0f;
  }
  // or 1 for values greater than 1
  else if (t > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("CurveV3fNormal: t = %f for id = %d. t = 1.0 assumed\n", t,
                     id);
    t = 1.0f;
  }

  V3f ansX = contours.Get(id).GetV3fNormal(t);
  ans.x = ansX.x;
  ans.y = ansX.y;
  ans.z = ansX.z;
  return ans;
}

__lc_V2f Interface::CurveV2fPoint(int id, float t) {
  __lc_V2f ans;
  ans.x = ans.y = 0;

  // for invalid id return (0,0)
  if (!contours.ValidId(id)) {
    Utils::Message("CurveV2fPoint: Invalid curve id = %d. Returning (0,0).\n",
                   id);
    return ans;
  }
  // for invalid values of t
  // assume 0 for values less than 0
  else if (t < 0.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("CurveV2fPoint: t = %f for id = %d. t = 0.0 assumed\n", t,
                     id);
    t = 0.0f;
  }
  // or 1 for values greater than 1
  else if (t > 1.0f) {
    if (comlineparam.VerboseMode())
      Utils::Message("CurveV2fPoint: t = %f for id = %d. t = 1.0 assumed\n", t,
                     id);
    t = 1.0f;
  }

  V2f ansX = contours.Get(id).GetV2fPoint(t);
  ans.x = ansX.x;
  ans.y = ansX.y;
  return ans;
}

void Interface::CurveReset(int id) {
  // if the contour id is invalid
  // just print the warning
  if (!contours.ValidId(id))
    Utils::Message("CurveReset: invalid id = %d. Function call ignored.\n", id);
  else
    contours.GetAccess(id).Reset();
}

void Interface::CurveSetPoint(int id, int i, float x, float y, float z) {
  // if the contour id is invalid
  // just print the warning
  if (!contours.ValidId(id))
    Utils::Message("CurveSetPoint: invalid id = %d. Function call ignored.\n",
                   id);
  // if the point id is invalid
  // just print the warning
  else if (!contours.Get(id).ValidPointId(i))
    Utils::Message("CurveSetPoint: invalid id = %d for curve id = %d. Function "
                   "call ignored.\n",
                   i, id);
  else
    contours.GetAccess(id).SetPoint(i, x, y, z);
}

void Interface::CurveScale(int id, float x, float y, float z) {
  // if the contour id is invalid
  // just print the warning
  if (!contours.ValidId(id))
    Utils::Message("CurveScale: invalid id = %d. Function call ignored.\n", id);
  else
    contours.GetAccess(id).Scale(x, y, z);
}

void Interface::CurveRecalc(int id) {
  // if the contour id is invalid
  // just print the warning
  if (!contours.ValidId(id))
    Utils::Message("CurveRecalc: invalid id = %d. Function call ignored.\n",
                   id);
  else
    contours.GetAccess(id).Recalculate();
}

float Interface::CurveGAL(int id) {
  // if the contour id is invalid
  // just print the warning
  if (!contours.ValidId(id)) {
    Utils::Message("CurveGAL: invalid id = %d. Function call ignored.\n",
                   id);
    return 0.f;
  }
  else
    return contours.GetAccess(id).GetArcLen();
}

void Interface::RunCmnd(const char *cmnd) { Utils::ExecuteDetached(cmnd); }

SurfaceObj Interface::GetSurface(int id) {
  SurfaceObj res;
  // if the surface id is invalid
  // print the warning
  // and return undefined SurfaceObj
  if (!surfaces.ValidId(id))
    Utils::Message(
        "GetSurface: invalid id = %d. Returning uninitialized object.\n", id);
  else {
    const Surface &s = surfaces.Get(id);
    // res will contain the control points
    // of the 0th patch
    const Patch &p = s.GetPatch(0);
    for (int i = 0; i < 16; ++i)
      res.Set(i, p.Point(i));
  }
  return res;
}

__lc_V3f Interface::GetSurfacePoint(int id, int pt) {
  __lc_V3f res;
  // if the surface id is invalid
  // print the warning
  // and return (0,0,0)
  if (!surfaces.ValidId(id))
    Utils::Message("GetSurfacePoint: invalid id = %d. Returning (0,0,0).\n",
                   id);
  // if invalid patch specified
  // print the warning
  // and return (0,0,0)
  else if (!surfaces.Get(id).IsValidPatchId(pt / 16))
    Utils::Message("GetSurfacePoint: invalid patch = %d. Returning (0,0,0).\n",
                   pt / 16);
  else {
    const Surface &s = surfaces.Get(id);
    const Patch &p = s.GetPatch(pt / 16);
    Vector3d pnt = p.Point(pt % 16);
    res.x = pnt.X();
    res.y = pnt.Y();
    res.z = pnt.Z();
  }
  return res;
}

static unsigned short main_xsubi[3] = {0x330E, 0, 0};

double Interface::Ran(double range) { return erand48(main_xsubi) * range; }

void Interface::SeedRan(long seed) {
  main_xsubi[0] = static_cast<unsigned short>(seed);
  main_xsubi[1] = static_cast<unsigned short>(seed >> 16);
  main_xsubi[2] = 0;
}

MouseStatus Interface::GetMouseStatus(void) { return pLpfg->GetMouseStatus(); }

TabletStatus Interface::GetTabletStatus(void) {
  return pLpfg->GetTabletStatus();
}

void Interface::UserMenuItem(const char *name, unsigned int ref) {
  pLpfg->_userMenu.add(std::string(name), ref);
}

void Interface::UserMenuClear(void) { pLpfg->UserMenuClear(); }

int Interface::UserMenuChoice(void) { return pLpfg->UserMenuChoice(); }

void Interface::RunSimulation(void) { pLpfg->RunSim(); }

void Interface::PauseSimulation(void) { pLpfg->Stop(); }

BsurfaceObjS Interface::GetBsurfaceS(int id) {
  BsurfaceObjS res;
  // if the surface id is invalid
  // print the warning
  // and return undefined BsurfaceObjS
  if (!bsurfaces.ValidId(id))
    Utils::Message(
        "GetBsurfaceS: invalid id = %d. Returning uninitialized object.\n", id);
  else {
    const b_wrapper &s = bsurfaces.Get(id);
    res = ConstructBsurfaceObj(s, res);
  }
  return res;
}

BsurfaceObjM Interface::GetBsurfaceM(int id) {
  BsurfaceObjM res;
  // if the surface id is invalid
  // print the warning
  // and return undefined BsurfaceObjM
  if (!bsurfaces.ValidId(id))
    Utils::Message(
        "GetBsurfaceM: invalid id = %d. Returning uninitialized object.\n", id);
  else {
    const b_wrapper &s = bsurfaces.Get(id);
    res = ConstructBsurfaceObj(s, res);
  }
  return res;
}

bool Interface::terrainHeightAt(V3f world, V3f &surface) {
  if (terrainData != NULL)
    return terrainData->terrainHeightAt(world, surface);
  else {
    Utils::Error("terrainHeightAt: Terrain Not Loaded\n");
    return false;
  }
}

bool Interface::terrainNormalAt(V3f world, V3f &surface) {
  if (terrainData != NULL)
    return terrainData->terrainNormalAt(world, surface);
  else {
    Utils::Error("terrainHeightAt: Terrain Not Loaded\n");
    return false;
  }
}

void Interface::terrainVisibilityAll(VisibilityMode mode) {
  if (terrainData != NULL)
    terrainData->terrainVisibilityAll(mode);
  else
    Utils::Error("terrainVisibilityAll: Terrain Not Loaded\n");
}
void Interface::terrainVisibilityPatch(VisibilityMode mode, int level,
                                       V3f worldSpacePoint) {
  if (terrainData != NULL)
    terrainData->terrainVisibilityPatch(mode, level, worldSpacePoint);
  else
    Utils::Error("terrainVisibilityPatch: Terrain Not Loaded\n");
}
void Interface::scaleTerrainBy(float val) {
  if (terrainData != NULL)
    terrainData->scaleTerrainBy(val);
  else
    Utils::Error("scaleTerrainBy: Terrain Not Loaded\n");
}

float Interface::SetOrGetParameterf(const char *name, float defaultVal) {
  return pLpfg->setOrGetParameterf(name, defaultVal);
}

int Interface::SetOrGetParameteri(const char *name, int defaultVal) {
  return pLpfg->setOrGetParameteri(name, defaultVal);
}

bool Interface::ParametersNeedUpdating() {
  return pLpfg->parametersNeedUpdating();
}

float Interface::GetParameterf(const char *name) {
  return pLpfg->GetParameterf(name);
}

int Interface::GetParameteri(const char *name) {
  return pLpfg->GetParameteri(name);
}

void Interface::SetParameterf(const char *name, float value) {
  pLpfg->SetParameterf(name, value);
}

void Interface::SetParameteri(const char *name, int value) {
  pLpfg->SetParameteri(name, value);
}

void Interface::DelayWrite() { pLpfg->DelayWrite(); }

void Interface::Write() { pLpfg->Write(); }

static ContextScanner *pActiveScanner = NULL;

const char *Interface::GetModuleAddr(__lc_ModuleIdType mid) {
  if (pActiveScanner != NULL) {
    if (pActiveScanner->CurrentModuleId() == mid) {
      return pActiveScanner->Address();
    } else {
      return NULL;
    }
  } else {
    return NULL;
  }
}

bool Interface::ConfirmContext() {
  if (pActiveScanner != NULL) {
    pActiveScanner->Confirm();
    pActiveScanner = NULL;
    return true;
  } else {
    return false;
  }
}

bool Interface::AcceptContext(__lc_ModuleIdType moduleId) {
  if (pActiveScanner != NULL) {
    return pActiveScanner->Accept(moduleId);
  } else {
    return false;
  }
}

bool Interface::LContextReset() {
  pActiveScanner = pgLContextScanner;
  pActiveScanner->Reset();
  return true;
}

bool Interface::RContextReset() {
  pActiveScanner = pgRContextScanner;
  pActiveScanner->Reset();
  return true;
}

bool Interface::AdvanceContext() {
  if (pActiveScanner != NULL) {
    return pActiveScanner->Advance();
  } else {
    return false;
  }
}

bool Interface::LNContextReset() {
  pActiveScanner = pgLNContextScanner;
  if (pActiveScanner != NULL) {
    pActiveScanner->Reset();
    return true;
  } else {
    return false;
  }
}

bool Interface::RNContextReset() {
  pActiveScanner = pgRNContextScanner;
  if (pActiveScanner != NULL) {
    pActiveScanner->Reset();
    return true;
  } else {
    return false;
  }
}

char *Interface::GetNextModuleSpot(int iSize) {
  return succstrg.GetNextChunk(iSize);
}

void Interface::StartPerformance() { PerformanceMonitor::Start(1); }

void Interface::StopPerformance() { PerformanceMonitor::Stop(1); }
