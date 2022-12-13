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



#ifndef __LSYSDLL_H__
#define __LSYSDLL_H__

#include <vector>

#include "include/lparams.h"
#include "include/lintrfc.h"
#include "comlineparam.h"
#include "dynlib.h"
#include "asrt.h"

class LsysDll : public DynLibrary {
public:
  LsysDll();
  int DerivationLength() const {
    ASSERT(Connected());
    return _pDerivationLength();
  }
  void Axiom() const {
    ASSERT(Connected());
    _pAxiom();
  }
  void Start() const {
    ASSERT(Connected());
    _pStart();
  }
  void StartEach() const {
    ASSERT(Connected());
    _pStartEach();
  }
  void EndEach() const {
    ASSERT(Connected());
    _pEndEach();
  }
  void End() const {
    ASSERT(Connected());
    _pEnd();
  }
  int NumOfProductions(int grp) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pNumOfProductions(grp);
  }
  int NumOfModulePProductions(int iGroup, __lc_ModuleIdType moduleId) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pNumOfModulePProductions(iGroup, moduleId);
  }
  int NumOfModuleDProductions(int iGroup, __lc_ModuleIdType moduleId) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pNumOfModuleDProductions(iGroup, moduleId);
  }
  int NumOfModuleIProductions(int iGroup, int iVGroup,
                              __lc_ModuleIdType moduleId) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pNumOfModuleIProductions(iGroup, iVGroup, moduleId);
  }
  int NumOfDecompositions(int grp) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pNumOfDecompositions(grp);
  }
  int NumOfInterpretations(int grp) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pNumOfInterpretations(grp);
  }
  const __lc_ProductionPredecessor &GetProductionPredecessor(int grp,
                                                             int i) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pGetProductionPredecessor(grp, i);
  }
  const __lc_ProductionPredecessor &GetDecompositionPredecessor(int grp,
                                                                int i) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pGetDecompositionPredecessor(grp, i);
  }
  const __lc_ProductionPredecessor &
  GetInterpretationPredecessor(int grp, int i, int vgrp) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pGetInterpretationPredecessor(grp, i, vgrp);
  }
  const __lc_ProductionPredecessor &
  GetModulePProductionPredecessor(int iGroup, __lc_ModuleIdType moduleId,
                                  int item) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pGetModulePProductionPredecessor(iGroup, moduleId, item);
  }
  const __lc_ProductionPredecessor &
  GetModuleDProductionPredecessor(int iGroup, __lc_ModuleIdType moduleId,
                                  int item) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pGetModuleDProductionPredecessor(iGroup, moduleId, item);
  }
  const __lc_ProductionPredecessor &
  GetModuleIProductionPredecessor(int iGroup, int iVGroup,
                                  __lc_ModuleIdType moduleId, int item) const {
    ASSERT(Connected());
    ASSERT(grp < NumOfGroups());
    return _pGetModuleIProductionPredecessor(iGroup, iVGroup, moduleId, item);
  }
  int NumOfConsidered(int iConsiderGroup) const {
    ASSERT(Connected());
    return _pNumOfConsidered(iConsiderGroup);
  }
  int NumOfIgnored(int iConsiderGroup) const {
    ASSERT(Connected());
    return _pNumOfIgnored(iConsiderGroup);
  }
  int NumOfVerify() const {
    ASSERT(Connected());
    return _pNumOfVerify();
  }
  __lc_ModuleIdType GetConsidered(int i, int iConsiderGroup) const {
    ASSERT(Connected());
    return _pGetConsidered(i, iConsiderGroup);
  }
  __lc_ModuleIdType GetIgnored(int i, int iConsiderGroup) const {
    ASSERT(Connected());
    return _pGetIgnored(i, iConsiderGroup);
  }
  __lc_ModuleIdType GetVerify(int i) const {
    ASSERT(Connected());
    return _pGetVerify(i);
  }
  int NumOfModules() const {
    ASSERT(Connected());
    return _pNumOfModules();
  }
  const __lc_ModuleData *GetModuleData(int i) const {
    ASSERT(Connected());
    return _pGetModuleData(i);
  }
  bool IsForward() const {
    ASSERT(Connected());
    return _pIsForward();
  }
  int DecompositionMaxDepth() const {
    ASSERT(Connected());
    return _pDecompositionMaxDepth();
  }
  int InterpretationMaxDepth() const {
    ASSERT(Connected());
    return _pInterpretationMaxDepth();
  }
  bool RingLsystem() const {
    ASSERT(Connected());
    return _pRingLsystem() != 0;
  }
  bool IgnoreEnvironment() const {
    ASSERT(Connected());
    return _pIgnoreEnvironment();
  }
  int CurrentGroup() const { return _pCurrentGroup(); }
  int NumOfGroups() const { return _pNumOfGroups(); }
  int NumOfVGroups() const { return _pNumOfVGroups(); }
  int NumOfConsiderGroups() const { return _pNumOfConsiderGroups(); }

  bool IsEnvSensitive() const { return _EnvSensitive; }
  bool HasDecompositions() const { return _HasDecompositions; }

  __lc_GroupType DerivationType(int grp) const {
    return _pGetDerivationType(grp);
  }

  bool IsConsidered(__lc_ModuleIdType moduleId, int iConsiderGroup) const {
    return _considered[ConsiderIndex(moduleId, iConsiderGroup)];
  }

protected:
  bool _Map();
  void _Unmap();

private:
  void DetermineIfESensitive();
  void DetermineIfHasDecompositions();
  bool IsESensitive(const __lc_ProductionPredecessor &) const;
  bool IsESensitive(__lc_ModuleIdType) const;

  void BuildConsiderArray();
  void BuildConsiderGroup(int iConsiderGroup);
  int ConsiderIndex(__lc_ModuleIdType moduleId, int iConsiderGroup) const {
    return iConsiderGroup * NumOfModules() + moduleId;
  }
  bool InConsidered(__lc_ModuleIdType moduleId, int iConsiderGroup) const;
  bool InIgnored(__lc_ModuleIdType moduleId, int iConsiderGroup) const;

  typedef int (*pfIntVoid)();
  typedef void (*pfVoidVoid)();
  typedef const __lc_ProductionPredecessor &(*pfGetProdPred)(int, int);
  typedef const __lc_ProductionPredecessor &(*pfGetModuleProdPred)(
      int, __lc_ModuleIdType, int);
  typedef const __lc_ProductionPredecessor &(*pfGetModuleIProdPred)(
      int, int, __lc_ModuleIdType, int);
  typedef const __lc_ProductionPredecessor &(*pfGetProdPredInterp)(int, int,
                                                                   int);
  typedef __lc_ModuleIdType (*pfModIdInt)(int);
  typedef __lc_ModuleIdType (*pfModIdIntInt)(int, int);
  typedef const __lc_ModuleData *(*pfModuleDataInt)(int);
  typedef __lc_ProdCaller (*pfProdCallerIntInt)(int, int);
  typedef __lc_ProdCaller (*pfProdCaller3Int)(int, int, int);
  typedef bool (*pfBoolVoid)();
  typedef int (*pfIntInt)(int);
  typedef int (*pfIntIntModuleId)(int, __lc_ModuleIdType);
  typedef int (*pfIntIntIntModuleId)(int, int, __lc_ModuleIdType);
  typedef __lc_GroupType (*pfGroupTypeInt)(int);

  pfIntVoid _pDerivationLength;
  pfVoidVoid _pAxiom;
  pfVoidVoid _pStart;
  pfVoidVoid _pStartEach;
  pfVoidVoid _pEndEach;
  pfVoidVoid _pEnd;
  pfIntInt _pNumOfProductions;
  pfIntIntModuleId _pNumOfModulePProductions;
  pfIntIntModuleId _pNumOfModuleDProductions;
  pfIntIntIntModuleId _pNumOfModuleIProductions;
  pfIntInt _pNumOfDecompositions;
  pfIntInt _pNumOfInterpretations;
  pfGetProdPred _pGetProductionPredecessor;
  pfGetProdPred _pGetDecompositionPredecessor;
  pfGetProdPredInterp _pGetInterpretationPredecessor;
  pfGetModuleProdPred _pGetModulePProductionPredecessor;
  pfGetModuleProdPred _pGetModuleDProductionPredecessor;
  pfGetModuleIProdPred _pGetModuleIProductionPredecessor;
  pfIntInt _pNumOfConsidered;
  pfIntInt _pNumOfIgnored;
  pfIntVoid _pNumOfVerify;
  pfModIdIntInt _pGetConsidered;
  pfModIdIntInt _pGetIgnored;
  pfModIdInt _pGetVerify;
  pfIntVoid _pNumOfModules;
  pfModuleDataInt _pGetModuleData;
  pfProdCaller3Int _pGetInterpretation;
  pfProdCallerIntInt _pGetDecomposition;
  pfBoolVoid _pIsForward;
  pfIntVoid _pDecompositionMaxDepth;
  pfIntVoid _pInterpretationMaxDepth;
  pfIntVoid _pRingLsystem;
  pfBoolVoid _pIgnoreEnvironment;
  pfIntVoid _pNumOfGroups;
  pfIntVoid _pCurrentGroup;
  pfIntVoid _pNumOfVGroups;
  pfIntVoid _pNumOfConsiderGroups;
  pfGroupTypeInt _pGetDerivationType;
  pfIntVoid _pGetModuleUnionSize;

  bool _EnvSensitive;
  bool _HasDecompositions;
  std::vector<bool> _considered;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
