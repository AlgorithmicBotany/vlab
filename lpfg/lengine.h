/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#ifndef __LENGINE_H__
#define __LENGINE_H__

/************

        LEngine performs the string derivation.

************/

#include <fstream>
#include <memory>
#include <algorithm>
#include <stack>

#include <stdlib.h>

#include <rand48/rand.h>

#include "include/lparams.h"
#include "include/lintrfc.h"
#include "volume.h"
#include "viewpos.h"
#include "projection.h"
#include "lsysdll.h"
#include "lstring.h"
#include "lstriter.h"
#include "utils.h"
#include "interface.h"
#include "comlineparam.h"
#ifdef WIN32
#include "view.h"
#else
#include "viewLnx.h"
#endif
#include "glenv.h"

#include "StdModulesStruct.h"
#include "PerformanceMonitor.h"

class Environment;
class EnvironmentReply;
class GillespieSuccessor;

class LEngine {
public:
  LEngine();
  ~LEngine();

  void StartEnvironment();
  bool NewLsystem(const char *);
  std::pair<Volume, ViewPos> CalculateVolume(int vgrp) const;

  void DeriveString();
  void Derive();
  void Rewind();
  void Stop();
  int StepNo() const { return _step; }
  int LastAnimFrame() const;
  void StopFunction() { _stopFlag = true; }
  bool StopRequested() const { return _stopFlag; }

  void ClearLog();
  void DrawGL(Vector3d, int vgroup, unsigned int glbase, void *) const;
  void DrawForSelection() const;
  void DrawPOVRay(std::ofstream &, std::ofstream *, std::ofstream &,
                  std::ofstream &, int vgrp) const;
  void DrawRayshade(std::ofstream &, const Projection &currentProjection,
                    GLEnv &glEnv, std::string fname, int vgrp) const;
  void InterpretToFile(const std::string &targetFilename) const;
  void DrawObj(std::string, GLEnv &glEnv, const Volume &, int vgrp) const;
  void DrawPostscript(std::ostream &, int vgrp,
                      const Projection &currentProjection,
                      DParams::ProjectionMode mode) const;
  bool IsESensitive() const { return _dll.IsEnvSensitive(); }
  bool HasDecompositions() const { return _dll.HasDecompositions(); }
  void DumpString(std::ostream &) const;
  void DumpString() const { DumpString(_lstring); }
  bool Verify(const char *szLsystemName) const;

  int NumOfVGroups() const { return _dll.NumOfVGroups(); }

  // This method does some checking if the location is correct
  // but this is not a 100% bulletproof method.
  // ModuleStruct& module parameter needs to have the leading module_id
  // initialized as well as the parameters. It does not need to have
  // the trailing module_id set (last item in module.idarr).
  // It will be set by this method
  template <typename ModuleStructArr>
  void Insert(ModuleStructArr &module, int location) {
#ifndef NDEBUG
    {
      LstringIterator iter(_lstring);
      ASSERT(iter.ValidPosition(location, _dll.NumOfModules() - 1));
    }
#endif
    module.arr[sizeof(ModuleStructArr) / sizeof(__lc_ModuleIdType) - 1] =
        module.arr[0];
    _lstring.Insert(&module, sizeof(ModuleStructArr), location);
  }

  void OutputString(std::ostream &) const;
  void LoadString(std::istream &);
  bool ValidLsystem() const { return _dll.Connected(); }
  double GetGillespieTime() const { return _GillespieTime; }
  void ResetGillespie() { _GillespieTime = 0.0; }
  void SGRand(long seed) {
    gillespie_xsubi[0] = static_cast<unsigned short>(seed);
    gillespie_xsubi[1] = static_cast<unsigned short>(seed >> 16);
    gillespie_xsubi[2] = 0;
  }

  bool Considered(__lc_ModuleIdType, int iConsiderGroup) const;

#ifdef LINUX
  // MC June 2014 - changed so there is a pointer to QGLWidget for each View
  std::vector<GLWidget *> _glview;

#endif // LINUX

  static int _sDrawCount;

private:
  void PreprocessLsystem(const char *, const char *) const;
  void BuildDefFile(const char *) const;
  bool Translate(const char *, const char *) const;
  void CompileLsys(const char *) const;
  bool ConnectToLsys();
  void DisconnectLsys();

  const char *PreprocessedFile() const { return "input.i"; }
  const char *TranslatedFile() const;

  // represents the L+C program translated
  // into a dll
  LsysDll _dll;

  enum { eLstringInitSize = 10240 };

  double GRand() const { return erand48(gillespie_xsubi); }
  mutable unsigned short gillespie_xsubi[3];
  void InitGillespieXSubi() { SGRand(0x330E); }
  double _GillespieTime;
  // step counter
  int _step;
  // set to true by the Stop function in L-system
  bool _stopFlag;
  // the current string
  Lstring _lstring;
  // the new string
  Lstring _derivedstring;
  // memory pool used by Lstring's
  // in recursive decompositions and interpretations
  mutable MemoryPool _pool;

  std::unique_ptr<Environment> _pEnvironment;

  void Axiom();
  void DeriveForward(int);
  bool TryForwardGroup(int, LstringIterator &, __lc_CallerData &, int, bool);
  void DeriveBackward(int);
  bool TryBackwardGroup(int, LstringIterator &, __lc_CallerData &, int, bool);

  // Gillespie
  void DeriveGForward(int);
  void DeriveGBackward(int);
  void ApplyForwardGGroup(int, const LstringIterator &, __lc_CallerData &, int,
                          std::vector<GillespieSuccessor> &, bool);

  void AdvanceGillespieTime(double);

  void DecomposeString(const Lstring &src, Lstring &trg, int depth,
                       int grp) const;
  void InterpretForEnvironment(int);
  void ReadAnswer();
  bool InsertReadData(const EnvironmentReply &);

  struct ProductionMatchIteratorSet {
    ProductionMatchIteratorSet(Lstring &oldString, Lstring &newString)
        : rightContext(oldString), leftContext(oldString),
          rightNewContext(newString), leftNewContext(newString) {}

    LstringIterator rightContext;
    LstringIterator leftContext;
    LstringIterator rightNewContext;
    LstringIterator leftNewContext;
  };

  __lc_ProdCaller TryMatchForward(const LstringIterator &, __lc_CallerData &,
                                  const __lc_ProductionPredecessor &,
                                  ProductionMatchIteratorSet &iteratorSet,
                                  bool bRing) const;
  __lc_ProdCaller TryMatchBackward(const LstringIterator &, __lc_CallerData &,
                                   const __lc_ProductionPredecessor &,
                                   ProductionMatchIteratorSet &iteratorSet,
                                   bool bRing) const;

  bool CheckStrictPredecessor(LstringIterator &, __lc_CallerData &,
                              const __lc_ProductionModules &, bool bRing) const;
  bool CheckLeftContext(LstringIterator &, __lc_CallerData &,
                        const __lc_ProductionModules &, bool bRing,
                        int iConsiderGroup) const;
  bool CheckRightContext(LstringIterator &, __lc_CallerData &,
                         const __lc_ProductionModules &, bool bRing,
                         int iConsiderGroup) const;

  class MatchParameters {
  protected:
    MatchParameters(const LsysDll &dll)
        : _dll(dll), _bIsRing(_dll.RingLsystem()) {}

  public:
    virtual int NumOfModuleProductions(__lc_ModuleIdType moduleId) const = 0;
    virtual const __lc_ProductionPredecessor &
    GetModuleProductionPredecessor(__lc_ModuleIdType moduleId,
                                   int id) const = 0;
    bool IsRing() const { return _bIsRing; }

  protected:
    const LsysDll &_dll;

  private:
    bool _bIsRing;
  };

  class MatchInterpretationParameters : public MatchParameters {
  public:
    MatchInterpretationParameters(int iGroup, int iVGroup, const LsysDll &dll)
        : MatchParameters(dll), _iGroup(iGroup), _iVGroup(iVGroup) {}
    virtual int NumOfModuleProductions(__lc_ModuleIdType moduleId) const {
      return _dll.NumOfModuleIProductions(_iGroup, _iVGroup, moduleId);
    }
    virtual const __lc_ProductionPredecessor &
    GetModuleProductionPredecessor(__lc_ModuleIdType moduleId, int id) const {
      return _dll.GetModuleIProductionPredecessor(_iGroup, _iVGroup, moduleId,
                                                  id);
    }

  private:
    int _iGroup;
    int _iVGroup;
  };

  class MatchDecompositionParameters : public MatchParameters {
  public:
    MatchDecompositionParameters(int iGroup, const LsysDll &dll)
        : MatchParameters(dll), _iGroup(iGroup) {}

    virtual int NumOfModuleProductions(__lc_ModuleIdType moduleId) const {
      return _dll.NumOfModuleDProductions(_iGroup, moduleId);
    }
    virtual const __lc_ProductionPredecessor &
    GetModuleProductionPredecessor(__lc_ModuleIdType moduleId, int id) const {
      return _dll.GetModuleDProductionPredecessor(_iGroup, moduleId, id);
    }

  private:
    int _iGroup;
  };

  bool FindMatch(const LstringIterator &citer, __lc_CallerData &cd,
                 const MatchParameters &matchParameters) const;
  bool TryDecompose(const LstringIterator &citer, int iGroup) const;
  bool TryInterpret(const LstringIterator &citer, int iGroup,
                    int iVGroup) const;

  void CheckNumeric(int vgrp) const;

  void DumpString(const Lstring &) const;
  void BuildOutputFilename(std::string &result, int iStep) const;

  // Interpret string applying interpretation rules
  // recursively as required
  template <class Turtle, class Stack>
  void InterpretString(Turtle &turtle, Stack &stack, const Lstring &str,
                       int depth, int tbl, int vgrp) const {
    ASSERT(ValidLsystem());

    LstringIterator iter(str);

    // This one is used for OpenGL selection.
    // Only ScreenTurtle (and its descendants)
    // specialization has a useful implementation
    Utils::PPName<Turtle> nm;

    while (!iter.AtEnd()) {
      Debug("--interpreting module  %s\n", GetNameOf(iter.GetModuleId()));
      nm.Set(iter.Position());

      // assume no production applied yet
      bool applied = false;

      // if the current group is not default (0)
      // first try the current group

      if (tbl != 0) {
        applied = TryInterpret(iter, tbl, vgrp);
      }

      // if interpretation didn't apply
      // or was not found try default group
      if (!applied) {
        applied = TryInterpret(iter, 0, vgrp);
      }

      // if no matching interpretation rule
      // can be found
      if (!applied)
        // just interpret the current module
        InterpretModule(turtle, stack, iter, vgrp);
      // otherwise
      else {
        // if maximum interpretation depth not reached
        if (depth > 0) {
          // create a new (temporary) string
          Lstring lstr(&_pool);
          // fill it with the contents
          // of the interpretation rule's
          // successor
          lstr.Add(Interface::GetSuccessorStorage());
          // and interpret the successor recursively
          InterpretString(turtle, stack, lstr, depth - 1, tbl, vgrp);
        }
        // otherwise (max depth exceeded)
        else {
          // just interpret the current module
          InterpretModule(turtle, stack, iter, vgrp);
          // but also print the warning message
          Utils::Message("Maximum interpretation depth reached for module %s\n",
                         iter.GetModuleName());
        }
      }
      // advance to the next module in the string
      ++iter;
    }
  }

  template <class Turtle, class Stack>
  void InterpretModule(Turtle &turtle, Stack &stack,
                       const LstringIterator &iter, int vgrp) const {
    // if the current module is one of the
    // standard ones, execute the appropriate action
    const __lc_BasicParameterStruct *pBasicParams =
        reinterpret_cast<const __lc_BasicParameterStruct *>(iter.Ptr());

    Debug("Interpreting module %s\n", iter.GetModuleName());
#ifdef NO_MEMCOPY
#define PARAMS(moduleName)                                                     \
  const __##moduleName##_ParamsStruct_ *pParams =                              \
      reinterpret_cast<const __##moduleName##_ParamsStruct_ *>(pBasicParams);  \
  const __##moduleName##_ParamsStruct_::Data &params = pParams->data;
#else
#define PARAMS(moduleName)                                                     \
  const __##moduleName##_ParamsStruct_ *pParams =                              \
      reinterpret_cast<const __##moduleName##_ParamsStruct_ *>(pBasicParams);  \
  __##moduleName##_ParamsStruct_ lcParams;                                     \
  memcpy(&lcParams, pParams, sizeof(__##moduleName##_ParamsStruct_));          \
  const __##moduleName##_ParamsStruct_::Data &params = lcParams.data;
#endif

    switch (iter.GetModuleId()) {
    case SB_id:
      stack.push(turtle);
      // indicate to the turtle that a branch was started.
      // this is used for starting new generalized
      // cylinders that are in a branch (encolsed between SB ... EB)
      turtle.StartBranch();
      break;
    case EB_id:
      if (!stack.empty()) {
        turtle = stack.top();
        turtle.UpdateTropisms(); // Tropisms is a global variable, we have to
                                 // reset to the one that corresponds to the
                                 // current turtle
        stack.pop();
      } else
        Utils::Message("EB: unmatched end-of-branch module. Ignored.\n");
      break;
    case F_id: {
      PARAMS(F);
      turtle.F(params.Param0);
    } break;
    case f_id: {
      PARAMS(f);
      turtle.f(params.Param0);
    } break;
    case G_id: {
      PARAMS(G);
      turtle.G(params.Param0);
    } break;
    case g_id: {
      PARAMS(g);
      turtle.g(params.Param0);
    } break;
    case Left_id: {
      PARAMS(Left);
      turtle.Left(params.Param0);
    } break;
    case Right_id: {
      PARAMS(Right);
      turtle.Right(params.Param0);
    } break;
    case Down_id: {
      PARAMS(Down);
      turtle.Down(params.Param0);
    } break;
    case Up_id: {
      PARAMS(Up);
      turtle.Up(params.Param0);
    } break;
    case RollR_id: {
      PARAMS(RollR);
      turtle.RollR(params.Param0);
    } break;
    case RollL_id: {
      PARAMS(RollL);
      turtle.RollL(params.Param0);
    } break;
    case IncColor_id:
      turtle.IncColor();
      break;
    case DecColor_id:
      turtle.DecColor();
      break;
    case SetColor_id: {
      PARAMS(SetColor);
      turtle.SetColor(params.Param0);
    } break;
    case SetWidth_id: {
      PARAMS(SetWidth);
      turtle.SetWidth(params.Param0);
    } break;
    case Label_id:
      //[PASCAL] This if for text display we will have to take care of this
      // later
      {
#ifdef LINUX
        turtle._glview = _glview[vgrp];
#endif // LINUX
        PARAMS(Label);
        turtle.Label(params.Param0);
      }
      break;
    case TurnAround_id:
      turtle.Left(180.0);
      break;
    case GetPos_id:
    case GetHead_id:
    case GetLeft_id:
    case GetUp_id:
      turtle.Query(iter);
      break;
    case Circle_id: {
      PARAMS(Circle);
      turtle.Circle(params.Param0);
    } break;
    case CircleB_id: {
      PARAMS(Circle);
      turtle.CircleB(params.Param0);
    }
    case MoveTo_id: {
      PARAMS(MoveTo);
      turtle.MoveTo(params.Param0, params.Param1, params.Param2);
    } break;
    case Sphere_id: {
      PARAMS(Sphere);
      turtle.Sphere(params.Param0);
    } break;
    case SetHead_id: {
      PARAMS(SetHead);
      float arr[6] = {params.Param0, params.Param1, params.Param2,
                      params.Param3, params.Param4, params.Param5};
      turtle.SetHead(arr);
    } break;
    case SetHead3f_id: {
      PARAMS(SetHead3f);
      turtle.SetHeadOnly(params.Param0);
    } break;
    case Sphere0_id:
      turtle.Sphere0();
      break;
    case Circle0_id:
      turtle.Circle0();
      break;
    case Line2f_id: {
      PARAMS(Line2f);
      turtle.Line(V3f(params.Param0), V3f(params.Param1));
    } break;
    case Line2d_id: {
      PARAMS(Line2d);
      turtle.Line(V3f(params.Param0), V3f(params.Param1));
    } break;
    case Line3f_id: {
      PARAMS(Line3f);
      turtle.Line(params.Param0, params.Param1);
    } break;
    case Line3d_id: {
      PARAMS(Line3d);
      turtle.Line(params.Param0, params.Param1);
    } break;
    case LineTo2f_id: {
      PARAMS(LineTo2f);
      turtle.LineTo(V3f(params.Param0));
    } break;
    case LineTo2d_id: {
      PARAMS(LineTo2d);
      turtle.LineTo(V3f(params.Param0));
    } break;
    case LineTo3f_id: {
      PARAMS(LineTo3f);
      turtle.LineTo(params.Param0);
    } break;
    case LineTo3d_id: {
      PARAMS(LineTo3d);
      turtle.LineTo(V3f(params.Param0));
    } break;
    case LineRel2f_id: {
      PARAMS(LineRel2f);
      turtle.LineRel(V3f(params.Param0));
    } break;
    case LineRel2d_id: {
      PARAMS(LineRel2d);
      turtle.LineRel(V3f(params.Param0));
    } break;
    case LineRel3f_id: {
      PARAMS(LineRel3f);
      turtle.LineRel(params.Param0);
    } break;
    case LineRel3d_id: {
      PARAMS(LineRel3d);
      turtle.LineRel(V3f(params.Param0));
    } break;
    case Surface_id: {
      PARAMS(Surface);
      float s = params.Param1;
      turtle.Surface(params.Param0, s, s, s);
    } break;
    case Surface3_id: {
      PARAMS(Surface3);
      turtle.Surface(params.Param0, params.Param1, params.Param2,
                     params.Param3);
    } break;
    case RollToVert_id:
      turtle.RollToVert();
      break;
    case SetElasticity_id: {
      PARAMS(SetElasticity);
      turtle.SetElasticity(params.Param0, params.Param1);
    } break;
    case IncElasticity_id: {
      PARAMS(IncElasticity);
      turtle.IncElasticity(params.Param0);
    } break;
    case DecElasticity_id: {
      PARAMS(DecElasticity);
      turtle.DecElasticity(params.Param0);
    } break;
    case CurrentContour_id: {
      PARAMS(CurrentContour);
      turtle.CurrentContour(params.Param0);
    } break;
    case StartGC_id:
      turtle.StartGC();
      break;
    case PointGC_id:
      turtle.PointGC();
      break;
    case EndGC_id:
      turtle.EndGC();
      break;
    //case MeshPoint_id:  // from mesh_unused.h/.cpp
    //  turtle.MeshPoint();
    //  break;
    case Mesh_id: {
      PARAMS(Mesh);
      float s = params.Param1;
      turtle.Mesh(params.Param0, s, s, s);
    } break;
    case Mesh3_id: {
      PARAMS(Mesh3);
      turtle.Mesh(params.Param0, params.Param1, params.Param2,
                  params.Param3);
    } break;
    case E1_id: {
      PARAMS(E1);
      turtle.E1(params.Param0, iter);
    } break;
    case E2_id: {
      PARAMS(E2);
      turtle.E2(params.Param0, params.Param1, iter);
    } break;
    case MoveTo2f_id: {
      PARAMS(MoveTo2f);
      turtle.MoveTo3f(V3f(params.Param0));
    } break;
    case MoveTo2d_id: {
      PARAMS(MoveTo2d);
      turtle.MoveTo3f(V3f(params.Param0));
    } break;
    case MoveTo3f_id: {
      PARAMS(MoveTo3f);
      turtle.MoveTo3f(params.Param0);
    } break;
    case MoveTo3d_id: {
      PARAMS(MoveTo3d);
      turtle.MoveTo3f(V3f(params.Param0));
    } break;
    case SP_id:
      turtle.StartPolygon();
      break;
    case EP_id:
      turtle.EndPolygon();
      break;
    case PP_id:
      turtle.PolygonPoint();
      break;
    case Rhombus_id: {
      PARAMS(Rhombus);
      turtle.Rhombus(params.Param0, params.Param1);
    } break;
    case Triangle_id: {
      PARAMS(Triangle);
      turtle.Triangle(params.Param0, params.Param1);
    } break;
    case BlendedContour_id: {
      PARAMS(BlendedContour);
      turtle.BlendContours(params.Param0, params.Param1, params.Param2);
    } break;
    case CurrentTexture_id: {
      PARAMS(CurrentTexture);
      turtle.CurrentTexture(params.Param0);
    } break;
    case TextureVCoeff_id: {
      PARAMS(TextureVCoeff);
      turtle.TextureVCoeff(params.Param0);
    } break;
    case Orient_id:
      turtle.Orient();
      break;
    case ScaleContour_id: {
      PARAMS(ScaleContour);
      turtle.ScaleContour(params.Param0, params.Param1);
    } break;
    case Elasticity_id: {
      PARAMS(Elasticity);
      turtle.Elasticity(params.Param0);
    } break;
    case SetTropismDirection3f_id: {
      PARAMS(SetTropismDirection3f);
      turtle.STropismDirection(params.Param0);
    } break;
    case DSurface_id: {
      PARAMS(DSurface);
      turtle.DSurface(params.Param0, true);
    } break;
    case ContourSides_id: {
      PARAMS(ContourSides);
      turtle.ContourSides(params.Param0);
    } break;
    case ContourNormal_id: {
      PARAMS(ContourNormal);
      turtle.ContourNormal(params.Param0);
    } break;
    case InitSurface_id: {
      PARAMS(InitSurface);
      turtle.InitSurface(params.Param0);
    } break;
    case SurfacePoint_id: {
      PARAMS(SurfacePoint);
      turtle.SurfacePoint(params.Param0, params.Param1, params.Param2);
    } break;
    case DrawSurface_id: {
      PARAMS(DrawSurface);
      turtle.DrawSurface(params.Param0);
    } break;
    case CircleFront_id: {
      PARAMS(CircleFront);
      turtle.CircleFront(params.Param0);
    } break;
    case CircleFrontB_id: {
      PARAMS(CircleFront);
      turtle.CircleFrontB(params.Param0);
    } break;
    case CircleFront0_id:
      turtle.CircleFront0();
      break;
    case SetUPrecision_id: {
      PARAMS(SetUPrecision);
      turtle.SetUPrecision(params.Param0);
    } break;
    case SetVPrecision_id: {
      PARAMS(SetVPrecision);
      turtle.SetVPrecision(params.Param0);
    } break;
    case LineTo_id: {
      PARAMS(LineTo);
      turtle.LineTo(V3f(params.Param0, params.Param1, params.Param2));
    } break;
    case BSurface_id: {
      PARAMS(BSurface);
      float s = params.Param1;
      turtle.BSurface(params.Param0, s, s, s);
    } break;
    case MoveRel2f_id: {
      PARAMS(MoveRel2f);
      turtle.MoveRel3f(V3f(params.Param0));
    } break;
    case MoveRel2d_id: {
      PARAMS(MoveRel2d);
      turtle.MoveRel3f(V3f(params.Param0));
    } break;
    case MoveRel3f_id: {
      PARAMS(MoveRel3f);
      turtle.MoveRel3f(params.Param0);
    } break;
    case MoveRel3d_id: {
      PARAMS(MoveRel3d);
      turtle.MoveRel3f(V3f(params.Param0));
    } break;
    case DBSurfaceS_id: {
      PARAMS(DBSurfaceS);
      turtle.DBSurfaceS(params.Param0);
    } break;
    case DBSurfaceM_id: {
      PARAMS(DBSurfaceM);
      turtle.DBSurfaceM(params.Param0);
    } break;
    case Camera_id: {
      turtle.Camera();
    } break;
    case EA20_id: {
      PARAMS(EA20);
      turtle.EA(params.Param0.Length(), params.Param0.Array(), iter);
    } break;
    case Rotate_id: {
      PARAMS(Rotate);
      turtle.RotateXYZ(params.Param0, params.Param1);
    } break;
    case RotateHLU_id: {
      PARAMS(RotateHLU);
      turtle.RotateHLU(params.Param0, params.Param1);
    } break;
    case RotateXYZ_id: {
      PARAMS(RotateXYZ);
      turtle.RotateXYZ(params.Param0, params.Param1);
    } break;
    case Terrain_id: {
      PARAMS(Terrain);
      turtle.Terrain(params.Param0);
    } break;
    case PovRayStart_id: {
      PARAMS(PovRayStart);
      turtle.PovRayStart(params.Param0, params.Param1);
    } break;
    case SetCoordinateSystem_id: {
      PARAMS(SetCoordinateSystem);
      turtle.SetCoordinateSystem(params.Param0);
    } break;
    }
#undef PARAMS
  }
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
