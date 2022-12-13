#ifndef __LSYS_H__
#define __LSYS_H__

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef _WIN32
#define DECLSPEC extern "C" __declspec(dllexport)
#else
#define DECLSPEC extern "C"
#endif

DECLSPEC void __lc_SetExported(const __lc_ExportedFromLpfg *);

DECLSPEC int DerivationLength();
DECLSPEC void Start();
DECLSPEC void StartEach();
DECLSPEC void EndEach();
DECLSPEC void End();
DECLSPEC void Axiom();
DECLSPEC int NumOfProductions(int);
DECLSPEC int NumOfDecompositions(int);
DECLSPEC int NumOfInterpretations(int);
DECLSPEC int NumOfModulePProductions(int iGroup, __lc_ModuleIdType moduleId);
DECLSPEC int NumOfModuleDProductions(int iGroup, __lc_ModuleIdType moduleId);
DECLSPEC int NumOfModuleIProductions(int iGroup, int iVGroup,
                                     __lc_ModuleIdType moduleId);
DECLSPEC const __lc_ProductionPredecessor &
GetProductionPredecessor(int /* group */, int /* id */);
DECLSPEC const __lc_ProductionPredecessor &
GetDecompositionPredecessor(int /* group */, int /* id */);
DECLSPEC const __lc_ProductionPredecessor &
GetInterpretationPredecessor(int /* group */, int /* id */, int /* vgrp */);
DECLSPEC const __lc_ProductionPredecessor &
GetModulePProductionPredecessor(int iGroup, __lc_ModuleIdType moduleId, int id);
DECLSPEC const __lc_ProductionPredecessor &
GetModuleDProductionPredecessor(int iGroup, __lc_ModuleIdType moduleId, int id);
DECLSPEC const __lc_ProductionPredecessor &
GetModuleIProductionPredecessor(int iGroup, int iVGroup,
                                __lc_ModuleIdType moduleId, int id);
DECLSPEC int NumOfConsiderGroups();
DECLSPEC int NumOfConsidered(int);
DECLSPEC int NumOfIgnored(int);
DECLSPEC __lc_ModuleIdType GetConsidered(int, int);
DECLSPEC __lc_ModuleIdType GetIgnored(int, int);
DECLSPEC int NumOfVerify();
DECLSPEC __lc_ModuleIdType GetVerify(int);
DECLSPEC int NumOfModules();
DECLSPEC const __lc_ModuleData *GetModuleData(int);

DECLSPEC bool IsForward();

DECLSPEC int DecompositionMaxDepth();
DECLSPEC int InterpretationMaxDepth();
DECLSPEC int RingLsystem();

DECLSPEC bool IgnoreEnvironment();

DECLSPEC int NumOfTables();
DECLSPEC int CurrentGroup();
DECLSPEC int NumOfViews();

DECLSPEC __lc_GroupType GetDerivationType(int);

pfMessage Printf = 0;
pfFloatIntFloat func = 0;
pfFloatIntFloat tfunc = 0;
pfFloatInt val = 0;
pfFloatIntFloatFloatFloat pfunc = 0;
pfV2fIntFloat funcTangent = 0;
pfFloatIntFloat curveX = 0;
pfFloatIntFloat curveY = 0;
pfFloatIntFloat curveZ = 0;
pfV2fIntFloat curveXY = 0;
pfV3fIntFloat curveXYZ = 0;
pfV3fIntFloat curveNormal = 0;
pfVoidInt curveReset = 0;
pfVoid2Int3Float curveSetPoint = 0;
pfVoidInt curveRecalculate = 0;
pfVoidInt3Float curveScale = 0;
pfVoidString Run = 0;
pfSrfcInt GetSurface = 0;
pvV3fIntInt GetSurfacePoint = 0;
pfVoidInt fUseView = 0;
pfVoidInt CloseView = 0;
pfVoidVoid DisplayFrame = 0;
pfVoidString OutputFrame = 0;
pfFloatInt vvXmin = 0;
pfFloatInt vvYmin = 0;
pfFloatInt vvZmin = 0;
pfFloatInt vvXmax = 0;
pfFloatInt vvYmax = 0;
pfFloatInt vvZmax = 0;
pfFloatInt vvScale = 0;
pfVoidVoid Stop = 0;
pfIntVoid StepNo = 0;
pfDoubleVoid GillespieTime = 0;
pfVoidLong SeedGillespie = 0;
pfVoidVoid ResetGillespie = 0;
pfDoubleDouble ran = 0;
pfVoidLong sran = 0;
pfVoidString LoadString = 0;
pfVoidString OutputString = 0;
pfMStat GetMouseStatus = 0;
pfTStat GetTabletStatus = 0;
pfVoidStringUInt UserMenuItem = 0;
pfVoidVoid UserMenuClear = 0;
pfIntVoid UserMenuChoice = 0;
pfVoidVoid RunSimulation = 0;
pfVoidVoid PauseSimulation = 0;
pfCamPosInt GetCameraPosition = 0;

pfBSSBinaryOp BsurfaceObjSAdd = 0;
pfBSSInterp BsurfaceObjSInterp = 0;
pfBSSGet BsurfaceObjSGet = 0;
pfBSSSet BsurfaceObjSSet = 0;
pfBSSInt GetBsurfaceS = 0;

pfBSMBinaryOp BsurfaceObjMAdd = 0;
pfBSMInterp BsurfaceObjMInterp = 0;
pfBSMGet BsurfaceObjMGet = 0;
pfBSMSet BsurfaceObjMSet = 0;
pfBSMInt GetBsurfaceM = 0;

pfBoolV3fV3f terrainNormalAt = 0;
pfBoolV3fV3f terrainHeightAt = 0;
pfVoidVisibilityMode terrainVisibilityAll = 0;
pfVoidVisibilityModeIntV3f terrainVisibilityPatch = 0;
pfVoidFloat scaleTerrainBy = 0;

pfFloatStringFloat SetOrGetParameterf = 0;
pfIntStringInt SetOrGetParameteri = 0;
pfBoolVoid ParametersNeedUpdating = 0;
pfFloatString GetParameterf = 0;
pfIntString GetParameteri = 0;
pfVoidStringFloat SetParameterf = 0;
pfVoidStringInt SetParameteri = 0;
pfVoidVoid DelayWrite = 0;
pfVoidVoid Write = 0;

pfBoolVoid LContextReset = 0;
pfBoolVoid RContextReset = 0;
pfBoolVoid LNContextReset = 0;
pfBoolVoid RNContextReset = 0;
pfBoolModuleId AcceptContext = 0;
pfBoolVoid AdvanceContext = 0;
pfBoolVoid ConfirmContext = 0;
pfPCharModuleId GetModuleAddr = 0;
pfCharInt GetNextModuleSpot = 0;
pfVoidVoid StartPerformance = 0;
pfVoidVoid StopPerformance = 0;

void __lc_SetExported(const __lc_ExportedFromLpfg *pExp) {
  Printf = pExp->fMessage;
  func = pExp->fFunc;
  tfunc = pExp->fTFunc;
  val = pExp->fVFunc;
  pfunc = pExp->fPFunc;
  funcTangent = pExp->fFuncTangent;
  curveX = pExp->fCurveX;
  curveY = pExp->fCurveY;
  curveZ = pExp->fCurveZ;
  curveReset = pExp->fCurveReset;
  curveSetPoint = pExp->fCurveSetPoint;
  curveRecalculate = pExp->fCurveRecalc;
  curveXY = pExp->fCurveV2fPoint;
  curveXYZ = pExp->fCurveV3fPoint;
  curveNormal = pExp->fCurveV3fNormal;
  curveScale = pExp->fCurveScale;
  Run = pExp->fRun;
  GetSurface = pExp->fGetSurface;
  GetSurfacePoint = pExp->fGetSurfacePoint;
  fUseView = pExp->fUseView;
  CloseView = pExp->fCloseView;
  DisplayFrame = pExp->fDisplayFrame;
  OutputFrame = pExp->fOutputFrame;
  vvXmin = pExp->fvvXmin;
  vvYmin = pExp->fvvYmin;
  vvZmin = pExp->fvvZmin;
  vvXmax = pExp->fvvXmax;
  vvYmax = pExp->fvvYmax;
  vvZmax = pExp->fvvZmax;
  vvScale = pExp->fvvScale;
  Stop = pExp->fStop;
  StepNo = pExp->fStepNo;
  GillespieTime = pExp->fGillespieTime;
  SeedGillespie = pExp->fSeedGillespie;
  ResetGillespie = pExp->fResetGillespie;
  ran = pExp->fRan;
  sran = pExp->fSeedRan;
  LoadString = pExp->fLoadString;
  OutputString = pExp->fOutputString;
  GetMouseStatus = pExp->fGetMouseStatus;
  GetTabletStatus = pExp->fGetTabletStatus;
  UserMenuItem = pExp->fUserMenuItem;
  UserMenuClear = pExp->fUserMenuClear;
  UserMenuChoice = pExp->fUserMenuChoice;
  RunSimulation = pExp->fRunSimulation;
  PauseSimulation = pExp->fPauseSimulation;
  GetCameraPosition = pExp->fGetCameraPosition;

  BsurfaceObjSAdd = pExp->fBsurfaceObjSAdd;
  BsurfaceObjSInterp = pExp->fBsurfaceObjSInterp;
  BsurfaceObjSGet = pExp->fBsurfaceObjSGet;
  BsurfaceObjSSet = pExp->fBsurfaceObjSSet;
  GetBsurfaceS = pExp->fGetBsurfaceS;

  BsurfaceObjMAdd = pExp->fBsurfaceObjMAdd;
  BsurfaceObjMInterp = pExp->fBsurfaceObjMInterp;
  BsurfaceObjMGet = pExp->fBsurfaceObjMGet;
  BsurfaceObjMSet = pExp->fBsurfaceObjMSet;
  GetBsurfaceM = pExp->fGetBsurfaceM;

  terrainHeightAt = pExp->fterrainHeightAt;
  terrainNormalAt = pExp->fterrainNormalAt;
  terrainVisibilityAll = pExp->fterrainVisibilityAll;
  terrainVisibilityPatch = pExp->fterrainVisibilityPatch;
  scaleTerrainBy = pExp->fscaleTerrainBy;

  SetOrGetParameterf = pExp->fSetOrGetParameterf;
  SetOrGetParameteri = pExp->fSetOrGetParameteri;
  ParametersNeedUpdating = pExp->fParametersNeedUpdating;
  GetParameterf = pExp->fGetParameterf;
  GetParameteri = pExp->fGetParameteri;
  SetParameterf = pExp->fSetParameterf;
  SetParameteri = pExp->fSetParameteri;
  DelayWrite = pExp->fDelayWrite;
  Write = pExp->fWrite;

  LContextReset = pExp->fLContextReset;
  RContextReset = pExp->fRContextReset;
  LNContextReset = pExp->fLNContextReset;
  RNContextReset = pExp->fRNContextReset;

  AdvanceContext = pExp->fAdvanceContext;
  AcceptContext = pExp->fAcceptContext;
  ConfirmContext = pExp->fConfirmContext;
  GetModuleAddr = pExp->fGetModuleAddr;

  GetNextModuleSpot = pExp->fGetNextModuleSpot;
  StartPerformance = pExp->fStartPerformance;
  StopPerformance = pExp->fStopPerformance;
}

BsurfaceObjS operator+(const BsurfaceObjS &l, const BsurfaceObjS &r) {
  return BsurfaceObjSAdd(l, r);
}
BsurfaceObjS Interpolate(const BsurfaceObjS &l1, const BsurfaceObjS &l2,
                         float alpha, int r, int c) {
  return BsurfaceObjSInterp(l1, l2, alpha, r, c);
}

// If we want to cast automatically a __lc_V3f
/*
V3f operator*(const float d, const __lc_V3f v){
  return d * (V3f) v;
}
*/

// If we want to cast automatically a __lc_V3f

V3f curveXYZf(const int a, const float f){
  return (V3f) curveXYZ(a,f);
}

V3d curveXYZd(const int a, const float f){
  return (V3d) curveXYZ(a,f);
}
V2f curveXYf(const int a, const float f){
  return (V2f) curveXY(a,f);
}

V2d curveXYd(const int a, const float f){
  return (V2d) curveXY(a,f);
}



V3f Get(const BsurfaceObjS &l1, int i, int j) {
  return BsurfaceObjSGet(l1, i, j);
}

BsurfaceObjS Set(const BsurfaceObjS &l1, int i, int j, V3f p) {
  return BsurfaceObjSSet(l1, i, j, p);
}

BsurfaceObjM operator+(const BsurfaceObjM &l, const BsurfaceObjM &r) {
  return BsurfaceObjMAdd(l, r);
}
BsurfaceObjM Interpolate(const BsurfaceObjM &l1, const BsurfaceObjM &l2,
                         float alpha, int r, int c) {
  return BsurfaceObjMInterp(l1, l2, alpha, r, c);
}

V3f Get(const BsurfaceObjM &l1, int i, int j) {
  return BsurfaceObjMGet(l1, i, j);
}

BsurfaceObjM Set(const BsurfaceObjM &l1, int i, int j, V3f p) {
  return BsurfaceObjMSet(l1, i, j, p);
}

template <typename T> void Produce(T t) { pAdd(&t, sizeof(T)); }

bool __DeriveForward = true;

inline void Forward() { __DeriveForward = true; }
inline void Backward() { __DeriveForward = false; }
bool IsForward() { return __DeriveForward; }

bool __IgnoreEnvironment = false;

inline void Environment() { __IgnoreEnvironment = false; }
inline void NoEnvironment() { __IgnoreEnvironment = true; }

bool IgnoreEnvironment() { return __IgnoreEnvironment; }

int __CurrentTable = 0;

void UseGroup(int i) {
  if (i < 0 || i >= NumOfTables())
    Printf("Invalid group id = %d. Ignored\n", i);
  else
    __CurrentTable = i;
}

int CurrentGroup() { return __CurrentTable; }

void UseView(int i) {
  if (i < 0 || i >= NumOfViews())
    Printf("Invalid vgroup id = %d. Ignored\n", i);
  else
    fUseView(i);
}

#else
#error File already included
#endif
