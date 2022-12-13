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



#include <cstdio>

#include <vector>

#include <lparams.h>

#include "module.h"
#include "contextdata.h"

extern FILE *fOut;

void ContextData::Copy(const ContextData *pSrc) {
  _pCntxt = pSrc->_pCntxt;
  _pNewCntxt = pSrc->_pNewCntxt;
}

void ContextData::Clear() {
  _pCntxt = NULL;
  _pNewCntxt = NULL;
}

bool ContextData::IsClear() const {
  return (NULL == _pCntxt && NULL == _pNewCntxt);
}

bool ContextData::Contains(const char *ident) const {
  return (NULL != _pCntxt && _pCntxt->Contains(ident)) ||
         (NULL != _pNewCntxt && _pNewCntxt->Contains(ident));
}

int ContextData::CountParams() const {
  int res = 0;
  if (_pCntxt != NULL)
    res += _pCntxt->CountParams();
  if (_pNewCntxt != NULL)
    res += _pNewCntxt->CountParams();
  return res;
}

void ContextData::GenerateForPrototype() const {
  if (_pCntxt != NULL)
    _pCntxt->GenerateForPrototype();
  if (_pNewCntxt != NULL)
    _pNewCntxt->GenerateForPrototype();
}

int ContextData::GenerateForCaller(FILE *fOut, int prmcnt, char prefix) const {
  const FormalModuleDt *pCurrentModule = NULL;
  if (NULL != _pCntxt)
    pCurrentModule = _pCntxt->GetHead();
  if (NULL != _pNewCntxt)
    pCurrentModule = _pNewCntxt->GetHead();
  int mdlcnt = 0;
  while (NULL != pCurrentModule) {
    const ModuleDeclaration &decl = moduleTable.Find(pCurrentModule->Ident);
    if (pCurrentModule->Params.count > 0) {
      fprintf(fOut,
              "const %s* p%cCntxt%dParams = reinterpret_cast<const "
              "%s*>(pCD->%cCntxt().pArg(%d));\n",
              decl.ModuleParamsStructName(), prefix, mdlcnt,
              decl.ModuleParamsStructName(), prefix, mdlcnt);
#ifndef NO_MEMCOPY
      fprintf(fOut,
              "%s %cCntxt%dParams; memcpy(&%cCntxt%dParams, p%cCntxt%dParams, "
              "sizeof(%s));\n",
              decl.ModuleParamsStructName(), prefix, mdlcnt, prefix, mdlcnt,
              prefix, mdlcnt, decl.ModuleParamsStructName());
#endif
    }
    pCurrentModule = pCurrentModule->pNext;
    ++mdlcnt;
  }
  return prmcnt;
}

int ContextData::GenerateForProductionCall(FILE *, int parameterIndex,
                                           char prefix) const {
  const FormalModuleDt *pCurrentModule = NULL;
  if (NULL != _pCntxt)
    pCurrentModule = _pCntxt->GetHead();
  if (NULL != _pNewCntxt)
    pCurrentModule = _pNewCntxt->GetHead();

  int moduleCount = 0;
  while (NULL != pCurrentModule) {
    for (int iParameter = 0; iParameter < pCurrentModule->Params.count;
         ++iParameter) {
#ifdef NO_MEMCOPY
      fprintf(fOut, ", p%cCntxt%dParams->data.Param%d", prefix, moduleCount,
              iParameter);
#else
      fprintf(fOut, ", %cCntxt%dParams.data.Param%d", prefix, moduleCount,
              iParameter);
#endif
    }
    ++moduleCount;
    pCurrentModule = pCurrentModule->pNext;
  }
  return moduleCount + parameterIndex;
}

void ContextData::GenerateProtoEntry(FILE *fOut) const {
  fprintf(fOut, "{ ");
  if (NULL != _pCntxt)
    _pCntxt->GenerateProtoEntry();
  else
    fprintf(fOut, "{ 0 }, 0");
  fprintf(fOut, " }, { ");
  if (NULL != _pNewCntxt)
    _pNewCntxt->GenerateProtoEntry();
  else
    fprintf(fOut, "{ 0 }, 0");
  fprintf(fOut, " }");
}

void GenerateInContext(const FormalModuleDtList *pContextModules) {
  const FormalModuleDt *pModule = pContextModules->GetHead();
  while (pModule != NULL) {
    fprintf(fOut, "AcceptContext(%s_id) && ", pModule->Ident);
    fprintf(fOut, "GetModule%s(", pModule->Ident);
    for (int iParameter = 0; iParameter < pModule->NumOfParams();
         ++iParameter) {
      fprintf(fOut, "%s", pModule->Params.Params[iParameter]);
      if (iParameter < pModule->NumOfParams() - 1)
        fprintf(fOut, ",");
    }
    fprintf(fOut, ") && AdvanceContext() &&");
    pModule = pModule->pNext;
  }

  fprintf(fOut, " ConfirmContext() )");
}

void GenerateInRightContext(const FormalModuleDtList *pContextModules) {
  fprintf(fOut, "( RContextReset() && ");
  GenerateInContext(pContextModules);
}

void GenerateInLeftContext(FormalModuleDtList *pContextModules) {
  pContextModules->Reverse();
  fprintf(fOut, "( LContextReset() && ");
  GenerateInContext(pContextModules);
}

void GenerateInNewRightContext(const FormalModuleDtList *pContextModules) {
  fprintf(fOut, "( RNContextReset() && ");
  GenerateInContext(pContextModules);
}

void GenerateInNewLeftContext(FormalModuleDtList *pContextModules) {
  pContextModules->Reverse();
  fprintf(fOut, "( LNContextReset() && ");
  GenerateInContext(pContextModules);
}
