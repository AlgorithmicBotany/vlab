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



#include "lsysdll.h"
#include "utils.h"
#include "lpfgparams.h"
#include "comlineparam.h"

#include "StdModulesStruct.h"

LsysDll::LsysDll() { _Unmap(); }

bool LsysDll::_Map() {
  bool res = true;
  _pDerivationLength = (pfIntVoid)GetProc("DerivationLength");
  if (0 == _pDerivationLength) {
    Utils::Message("Missing DerivationLength\n");
    res = false;
  }
  _pAxiom = (pfVoidVoid)GetProc("Axiom");
  if (0 == _pAxiom) {
    Utils::Message("Missing Axiom\n");
    res = false;
  }

  _pStart = (pfVoidVoid)GetProc("Start");
  if (0 == _pStart) {
    Utils::Message("Missing Start\n");
    res = false;
  }

  _pStartEach = (pfVoidVoid)GetProc("StartEach");
  if (0 == _pStartEach) {
    Utils::Message("Missing StartEach\n");
    res = false;
  }

  _pEndEach = (pfVoidVoid)GetProc("EndEach");
  if (0 == _pEndEach) {
    Utils::Message("Missing EndEach\n");
    res = false;
  }

  _pEnd = (pfVoidVoid)GetProc("End");
  if (0 == _pEnd) {
    Utils::Message("Missing End\n");
    res = false;
  }

  _pNumOfProductions = (pfIntInt)GetProc("NumOfProductions");
  if (0 == _pNumOfProductions) {
    Utils::Message("Missing NumOfProductions\n");
    res = false;
  }

  _pNumOfInterpretations = (pfIntInt)GetProc("NumOfInterpretations");
  if (0 == _pNumOfInterpretations) {
    Utils::Message("Missing NumOfInterpretations\n");
    res = false;
  }

  _pNumOfDecompositions = (pfIntInt)GetProc("NumOfDecompositions");
  if (0 == _pNumOfDecompositions) {
    Utils::Message("Missing NumOfDecompositions\n");
    res = false;
  }

  _pNumOfModulePProductions =
      (pfIntIntModuleId)GetProc("NumOfModulePProductions");
  if (0 == _pNumOfModulePProductions) {
    Utils::Message("Missing NumOfModulePProductions\n");
    res = false;
  }

  _pNumOfModuleDProductions =
      (pfIntIntModuleId)GetProc("NumOfModuleDProductions");
  if (0 == _pNumOfModuleDProductions) {
    Utils::Message("Missing NumOfModuleDProductions\n");
    res = false;
  }

  _pNumOfModuleIProductions =
      (pfIntIntIntModuleId)GetProc("NumOfModuleIProductions");
  if (0 == _pNumOfModuleIProductions) {
    Utils::Message("Missing NumOfModulePProductions\n");
    res = false;
  }

  _pGetProductionPredecessor =
      (pfGetProdPred)GetProc("GetProductionPredecessor");
  if (0 == _pGetProductionPredecessor) {
    Utils::Message("Missing GetProductionPredecessor\n");
    res = false;
  }
  _pGetInterpretationPredecessor =
      (pfGetProdPredInterp)GetProc("GetInterpretationPredecessor");
  if (0 == _pGetInterpretationPredecessor) {
    Utils::Message("Missing GetInterpretationPredecessor\n");
    res = false;
  }
  _pGetDecompositionPredecessor =
      (pfGetProdPred)GetProc("GetDecompositionPredecessor");
  if (0 == _pGetDecompositionPredecessor) {
    Utils::Message("Missing GetDecompositionPredecessor\n");
    res = false;
  }

  _pGetModulePProductionPredecessor =
      (pfGetModuleProdPred)GetProc("GetModulePProductionPredecessor");
  if (0 == _pGetModulePProductionPredecessor) {
    Utils::Message("Missing GetModulePProductionPredecessor\n");
    res = false;
  }

  _pGetModuleDProductionPredecessor =
      (pfGetModuleProdPred)GetProc("GetModuleDProductionPredecessor");
  if (0 == _pGetModuleDProductionPredecessor) {
    Utils::Message("Missing GetModuleDProductionPredecessor\n");
    res = false;
  }

  _pGetModuleIProductionPredecessor =
      (pfGetModuleIProdPred)GetProc("GetModuleIProductionPredecessor");
  if (0 == _pGetModuleIProductionPredecessor) {
    Utils::Message("Missing GetModuleIProductionPredecessor\n");
    res = false;
  }

  _pNumOfConsidered = (pfIntInt)GetProc("NumOfConsidered");
  if (0 == _pNumOfConsidered) {
    Utils::Message("Missing NumOfConsidered\n");
    res = false;
  }

  _pNumOfIgnored = (pfIntInt)GetProc("NumOfIgnored");
  if (0 == _pNumOfIgnored) {
    Utils::Message("Missing NumOfIgnored\n");
    res = false;
  }

  _pNumOfVerify = (pfIntVoid)GetProc("NumOfVerify");
  if (0 == _pNumOfVerify) {
    Utils::Message("Missing NumOfVerify\n");
    res = false;
  }

  _pGetConsidered = (pfModIdIntInt)GetProc("GetConsidered");
  if (0 == _pGetConsidered) {
    Utils::Message("Missing GetConsidered\n");
    res = false;
  }

  _pGetIgnored = (pfModIdIntInt)GetProc("GetIgnored");
  if (0 == _pGetIgnored) {
    Utils::Message("Missing GetIgnored\n");
    res = false;
  }

  _pGetVerify = (pfModIdInt)GetProc("GetVerify");
  if (0 == _pGetVerify) {
    Utils::Message("Missing GetVerify\n");
    res = false;
  }

  _pNumOfModules = (pfIntVoid)GetProc("NumOfModules");
  if (0 == _pNumOfModules) {
    Utils::Message("Missing NumOfModules\n");
    res = false;
  }

  _pGetModuleData = (pfModuleDataInt)GetProc("GetModuleData");
  if (0 == _pGetModuleData) {
    Utils::Message("Missing GetModuleData\n");
    res = false;
  }

  _pIsForward = (pfBoolVoid)GetProc("IsForward");
  if (0 == _pIsForward) {
    Utils::Message("Missing IsForward\n");
    res = false;
  }

  _pDecompositionMaxDepth = (pfIntVoid)GetProc("DecompositionMaxDepth");
  if (0 == _pDecompositionMaxDepth) {
    Utils::Message("Missing DecompositionMaxDepth\n");
    res = false;
  }

  _pInterpretationMaxDepth = (pfIntVoid)GetProc("InterpretationMaxDepth");
  if (0 == _pInterpretationMaxDepth) {
    Utils::Message("Missing InterpretationMaxDepth\n");
    res = false;
  }

  _pRingLsystem = (pfIntVoid)GetProc("RingLsystem");
  if (0 == _pRingLsystem) {
    Utils::Message("Missing RingLsystem\n");
    res = false;
  }

  _pIgnoreEnvironment = (pfBoolVoid)GetProc("IgnoreEnvironment");
  if (0 == _pIgnoreEnvironment) {
    Utils::Message("Missing IgnoreEnvironment\n");
    res = false;
  }

  _pNumOfGroups = (pfIntVoid)GetProc("NumOfTables");
  if (0 == _pNumOfGroups) {
    Utils::Message("Missing NumOfTables\n");
    res = false;
  }

  _pCurrentGroup = (pfIntVoid)GetProc("CurrentGroup");
  if (0 == _pCurrentGroup) {
    Utils::Message("Missing CurrentGroup\n");
    res = false;
  }

  _pNumOfVGroups = (pfIntVoid)GetProc("NumOfViews");
  if (0 == _pNumOfVGroups) {
    Utils::Message("Missing NumOfViews\n");
    res = false;
  }

  _pNumOfConsiderGroups = (pfIntVoid)GetProc("NumOfConsiderGroups");
  if (0 == _pNumOfConsiderGroups) {
    Utils::Message("Missing NumOfConsiderGroups\n");
    res = false;
  }

  _pGetDerivationType = (pfGroupTypeInt)GetProc("GetDerivationType");
  if (0 == _pGetDerivationType) {
    Utils::Message("Missing GetDerivationType\n");
    res = false;
  }

  if (res) {
    DetermineIfESensitive();
    DetermineIfHasDecompositions();
    BuildConsiderArray();
  }

  return res;
}

void LsysDll::_Unmap() {
  _pDerivationLength = 0;
  _pAxiom = 0;
  _pStart = 0;
  _pStartEach = 0;
  _pEndEach = 0;
  _pEnd = 0;
  _pNumOfProductions = 0;
  _pNumOfModulePProductions = 0;
  _pNumOfModuleDProductions = 0;
  _pNumOfModuleIProductions = 0;
  _pNumOfInterpretations = 0;
  _pNumOfDecompositions = 0;
  _pGetInterpretation = 0;
  _pGetDecomposition = 0;
  _pGetProductionPredecessor = 0;
  _pGetDecompositionPredecessor = 0;
  _pGetInterpretationPredecessor = 0;
  _pGetModulePProductionPredecessor = 0;
  _pGetModuleDProductionPredecessor = 0;
  _pGetModuleIProductionPredecessor = 0;
  _pNumOfConsidered = 0;
  _pNumOfIgnored = 0;
  _pNumOfVerify = 0;
  _pGetConsidered = 0;
  _pGetIgnored = 0;
  _pGetVerify = 0;
  _pNumOfModules = 0;
  _pGetModuleData = 0;
  _pIsForward = 0;
  _pDecompositionMaxDepth = 0;
  _pInterpretationMaxDepth = 0;
  _pRingLsystem = 0;
  _pIgnoreEnvironment = 0;
  _pNumOfGroups = 0;
  _pCurrentGroup = 0;
  _pNumOfVGroups = 0;
  _pGetDerivationType = 0;
  _pGetModuleUnionSize = 0;
  _pNumOfConsiderGroups = 0;
}

void LsysDll::DetermineIfESensitive() {
  _EnvSensitive = false;
  for (int tbl = 0; tbl < NumOfGroups(); ++tbl) {
    // Check if any rule contains one of the environmental modules in the
    // predecessor
    for (int i = 0; i < NumOfProductions(tbl); ++i) {
      const __lc_ProductionPredecessor &pred = GetProductionPredecessor(tbl, i);
      if (IsESensitive(pred)) {
        _EnvSensitive = true;
        return;
      }
    }

    for (int i = 0; i < NumOfDecompositions(tbl); ++i) {
      const __lc_ProductionPredecessor &pred =
          GetDecompositionPredecessor(tbl, i);
      if (IsESensitive(pred)) {
        _EnvSensitive = true;
        return;
      }
    }

    for (int i = 0; i < NumOfInterpretations(tbl); ++i) {
      const __lc_ProductionPredecessor &pred =
          GetInterpretationPredecessor(tbl, i, 0);
      if (IsESensitive(pred)) {
        _EnvSensitive = true;
        return;
      }
    }
  }
}

bool LsysDll::IsESensitive(const __lc_ProductionPredecessor &pred) const {
  int i;
  for (i = 0; i < pred.RCntxt.Cntxt.count; ++i) {
    if (IsESensitive(pred.RCntxt.Cntxt.arr[i]))
      return true;
  }
  for (i = 0; i < pred.Strct.count; ++i) {
    if (IsESensitive(pred.Strct.arr[i]))
      return true;
  }
  for (i = 0; i < pred.LCntxt.Cntxt.count; ++i) {
    if (IsESensitive(pred.LCntxt.Cntxt.arr[i]))
      return true;
  }
  return false;
}

bool LsysDll::IsESensitive(__lc_ModuleIdType id) const {
  switch (id) {
  case GetPos_id:
  case GetHead_id:
  case GetLeft_id:
  case GetUp_id:
  //case MeshPoint_id: // from mesh_unused.h/.cpp
    return true;
  default:
    return false;
  }
}

void LsysDll::DetermineIfHasDecompositions() {
  _HasDecompositions = false;
  for (int tbl = 0; tbl < NumOfGroups(); ++tbl) {
    if (NumOfDecompositions(tbl) > 0) {
      _HasDecompositions = true;
      break;
    }
  }
}

void LsysDll::BuildConsiderArray() {
  _considered.resize(NumOfModules() * NumOfConsiderGroups());

  for (int iConsiderGroup = 0; iConsiderGroup < NumOfConsiderGroups();
       ++iConsiderGroup) {
    BuildConsiderGroup(iConsiderGroup);
  }
}

void LsysDll::BuildConsiderGroup(int iConsiderGroup) {
  if (NumOfConsidered(iConsiderGroup) > 0) {
    for (__lc_ModuleIdType moduleId = 0; moduleId < NumOfModules();
         ++moduleId) {
      _considered[ConsiderIndex(moduleId, iConsiderGroup)] =
          InConsidered(moduleId, iConsiderGroup);
    }
  } else if (NumOfIgnored(iConsiderGroup) > 0) {
    for (__lc_ModuleIdType moduleId = 0; moduleId < NumOfModules();
         ++moduleId) {
      _considered[ConsiderIndex(moduleId, iConsiderGroup)] =
          !InIgnored(moduleId, iConsiderGroup);
    }
  } else {
    for (__lc_ModuleIdType moduleId = 0; moduleId < NumOfModules();
         ++moduleId) {
      _considered[ConsiderIndex(moduleId, iConsiderGroup)] = true;
    }
  }
  _considered[ConsiderIndex(SB_id, iConsiderGroup)] = true;
  _considered[ConsiderIndex(EB_id, iConsiderGroup)] = true;
}

bool LsysDll::InConsidered(__lc_ModuleIdType moduleId,
                           int iConsiderGroup) const {
  ASSERT(NumOfConsidered(iConsiderGroup) > 0);
  for (int iConsidered = 0; iConsidered < NumOfConsidered(iConsiderGroup);
       ++iConsidered) {
    if (GetConsidered(iConsidered, iConsiderGroup) == moduleId)
      return true;
  }
  return false;
}

bool LsysDll::InIgnored(__lc_ModuleIdType moduleId, int iConsiderGroup) const {
  ASSERT(NumOfIgnored() > 0);
  for (int iIgnored = 0; iIgnored < NumOfIgnored(iConsiderGroup); ++iIgnored) {
    if (GetIgnored(iIgnored, iConsiderGroup) == moduleId)
      return true;
  }
  return false;
}
