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



#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <vector>

#include <lparams.h>

#include "contextdata.h"
#include "production.h"
#include "module.h"

extern FILE *fOut;
extern bool verbose;
extern bool bProfileProductions;

int ProductionProto::_counter = 1;
int _GrpId = 0;
int _VGrpId = 0;
int _considerGroupId = 0;

void StartGroup(int);
int ProductionProto::MaxVGrp = 0;
extern std::vector<__lc_GroupType> groups;
extern ModuleTable moduleTable;
extern ProductionType ProductionMode();
extern void l2cerror(const char *, ...);

ProductionProto::ProductionProto(ContextData *pLCntxt,
                                 FormalModuleDtList *pStrct,
                                 ContextData *pRCntxt)
    : _pStrct(pStrct), _id(_counter++), _group(_GrpId), _vgroup(_VGrpId),
      _considerGroup(_considerGroupId) {
  if (NULL != pLCntxt) {
    if (pLCntxt->Contains(StartBranchIdent) ||
        pLCntxt->Contains(EndBranchIdent))
      l2cerror("Branch modules not allowed in left context");
  }
  if (NULL == pLCntxt) {
    _LCntxt.Clear();
  } else {
    _LCntxt.Copy(pLCntxt);
  }
  if (NULL == pRCntxt) {
    _RCntxt.Clear();
  } else {
    _RCntxt.Copy(pRCntxt);
  }

  assert(NULL != _pStrct);
  _type = ProductionMode();
}

ProductionProto::~ProductionProto() { delete _pStrct; }

bool ProductionProto::IsContextFree() const {
  return (_LCntxt.IsClear() && _RCntxt.IsClear());
}

void ProductionProto::Generate() const {
  assert(NULL != fOut);
  switch (_type) {
  case eProduction:
    fprintf(fOut, "void _P%d(bool& __res_", _id);
    break;
  case eInterpretation:
    fprintf(fOut, "void _I%d(bool& __res_", _id);
    break;
  case eDecomposition:
    fprintf(fOut, "void _D%d(bool& __res_", _id);
    break;
  }
  _LCntxt.GenerateForPrototype();
  _pStrct->GenerateForPrototype();
  _RCntxt.GenerateForPrototype();
  fprintf(fOut, ")");
}

int ProductionProto::Compare(const void *ppLV, const void *ppRV) {
  const ProductionProto **ppL = (const ProductionProto **)(ppLV);
  const ProductionProto **ppR = (const ProductionProto **)(ppRV);
  int l = (*ppL)->_pStrct->GetHead()->id;
  int r = (*ppR)->_pStrct->GetHead()->id;
  if (l == r)
    return (*ppL)->VGroup() - (*ppR)->VGroup();
  else
    return l - r;
}

void ProductionProto::GenerateCaller() const {
  switch (_type) {
  case eProduction:
    fprintf(fOut, "\nbool _PC%d(const __lc_CallerData* pCD)\n", _id);
    break;
  case eInterpretation:
    fprintf(fOut, "\nbool _IC%d(const __lc_CallerData* pCD)\n", _id);
    break;
  case eDecomposition:
    fprintf(fOut, "\nbool _DC%d(const __lc_CallerData* pCD)\n", _id);
    break;
  }

  fprintf(fOut, "{\nbool res = false;\n");
  int prmcnt = 0;
  prmcnt += _LCntxt.CountParams();
  prmcnt += _pStrct->CountParams();
  prmcnt += _RCntxt.CountParams();
  prmcnt = 0;

  prmcnt = _LCntxt.GenerateForCaller(fOut, prmcnt, 'L');

  // Strict predecessor
  {
    const FormalModuleDt *pCurrentModule = _pStrct->GetHead();
    int mdlcnt = 0;
    while (NULL != pCurrentModule) {
      const ModuleDeclaration &decl = moduleTable.Find(pCurrentModule->Ident);
      if (pCurrentModule->Params.count > 0) {
        fprintf(fOut,
                "const %s* pStrct%dParams = reinterpret_cast<const "
                "%s*>(pCD->Strct().pArg(%d));\n",
                decl.ModuleParamsStructName(), mdlcnt,
                decl.ModuleParamsStructName(), mdlcnt);
#ifndef NO_MEMCOPY
        fprintf(fOut,
                "%s Strct%dParams; memcpy(&Strct%dParams, pStrct%dParams, "
                "sizeof(%s));\n",
                decl.ModuleParamsStructName(), mdlcnt, mdlcnt, mdlcnt,
                decl.ModuleParamsStructName());
#endif
      }
      pCurrentModule = pCurrentModule->pNext;
      mdlcnt++;
    }
  }

  prmcnt = _RCntxt.GenerateForCaller(fOut, prmcnt, 'R');

  if (bProfileProductions)
    fputs("StartPerformance();\n", fOut);

  switch (_type) {
  case eProduction:
    fprintf(fOut, "_P%d(res", _id);
    break;
  case eInterpretation:
    fprintf(fOut, "_I%d(res", _id);
    break;
  case eDecomposition:
    fprintf(fOut, "_D%d(res", _id);
    break;
  }

  prmcnt = 0;
  prmcnt += _LCntxt.GenerateForProductionCall(fOut, prmcnt, 'L');

  {
    const FormalModuleDt *pCurrentModule = _pStrct->GetHead();
    int moduleCount = 0;
    while (pCurrentModule != NULL) {
      for (int iParameter = 0; iParameter < pCurrentModule->Params.count;
           ++iParameter) {
#ifdef NO_MEMCOPY
        fprintf(fOut, ", pStrct%dParams->data.Param%d", moduleCount,
                iParameter);
#else
        fprintf(fOut, ", Strct%dParams.data.Param%d", moduleCount, iParameter);
#endif
      }
      ++moduleCount;
      pCurrentModule = pCurrentModule->pNext;
    }
  }

  prmcnt += _RCntxt.GenerateForProductionCall(fOut, prmcnt, 'R');

  fprintf(fOut, ");\n");
  if (bProfileProductions)
    fputs("StopPerformance();\n", fOut);
  fprintf(fOut, "return res;\n");
  fprintf(fOut, "}\n\n");
}

void ProductionProto::GenerateEmptyProtoEntry(FILE *fOut) {
  fputs("\t{\n"
        "\t\t{\n"
        "\t\t\t{ { 0 }, 0 }, { { 0 }, 0 }\n"
        "\t\t},\n"
        "\t\t{\n"
        "\t\t\t{ 0 }, 0\n"
        "\t\t},\n"
        "\t\t{\n"
        "\t\t\t{ { 0 }, 0 }, { { 0 }, 0 }\n"
        "\t\t},\n"
        "\t\t0, 0\n"
        "\t}\n",
        fOut);
}

void ProductionProto::GenerateProtoEntry() const {
  fprintf(fOut, "\t{\n");

  fprintf(fOut, "\t\t{\n");
  fprintf(fOut, "\t\t\t");

  _LCntxt.GenerateProtoEntry(fOut);

  fprintf(fOut, "\n\t\t},");

  fprintf(fOut, "\n\t\t{\n");
  fprintf(fOut, "\t\t\t");

  _pStrct->GenerateProtoEntry();
  fprintf(fOut, "\n\t\t},");

  fprintf(fOut, "\n\t\t{\n");
  fprintf(fOut, "\t\t\t");

  _RCntxt.GenerateProtoEntry(fOut);

  fprintf(fOut, "\n\t\t},\n\t\t");
  switch (_type) {
  case eProduction:
    fprintf(fOut, "_PC%d, ", _id);
    break;
  case eInterpretation:
    fprintf(fOut, "_IC%d, ", _id);
    break;
  case eDecomposition:
    fprintf(fOut, "_DC%d, ", _id);
    break;
  }

  fprintf(fOut, "%d \n\t}", _considerGroup);
}

ProductionTable::ProductionTable() {
  _arr = new pProductionProto[eInitSize];
  _items = 0;
  _size = eInitSize;
}

ProductionTable::~ProductionTable() {
  for (int i = 0; i < _items; i++)
    delete _arr[i];
  delete[] _arr;
}

void ProductionTable::Add(ProductionProto *pProto) {
  if (groups[_GrpId] == __lc_gtGillespie && !pProto->IsContextFree()) {
    if (pProto->HasNewContext())
      l2cerror("Gillespie productions cannot have new context");
  }
  if (_items == _size)
    _Grow();
  _arr[_items] = pProto;
  _items++;
}

void ProductionTable::_Grow() {
  pProductionProto *aNew = new pProductionProto[_size * 2];
  for (int i = 0; i < _items; ++i)
    aNew[i] = _arr[i];
  delete[] _arr;
  _arr = aNew;
  _size *= 2;
}

void HomomorphismTable::Add(ProductionProto *pProto) {
  ProductionTable::Add(pProto);
}

int HomomorphismTable::Find(int id, int grp, int vgrp) const {
  for (int i = 0; i < _items; i++) {
    if (_arr[i]->GetStrct()->GetHead()->id == id && _arr[i]->Group() == grp) {
      if (_arr[i]->GetType() == eInterpretation) {
        if (_arr[i]->VGroup() == vgrp)
          return i;
      } else
        return i;
    }
  }
  return -1;
}

void StartGroup(int grpid) {
  if (groups.size() < static_cast<size_t>(grpid) + 1)
    groups.resize(grpid + 1, __lc_gtUnspecified);
  else if (groups[grpid] != __lc_gtLsystem &&
           groups[grpid] != __lc_gtUnspecified)
    l2cerror("%d is not L-system production group", grpid);

  groups[grpid] = __lc_gtLsystem;

  _GrpId = grpid;
}

void StartGGroup(int grpid) {
  if (groups.size() < static_cast<size_t>(grpid) + 1)
    groups.resize(grpid + 1, __lc_gtUnspecified);
  else if (groups[grpid] != __lc_gtGillespie &&
           groups[grpid] != __lc_gtUnspecified)
    l2cerror("%d is not Gillespie production group", grpid);

  groups[grpid] = __lc_gtGillespie;

  _GrpId = grpid;
}

void EndGroup() { _GrpId = 0; }

void StartVGroup(int grpid) {
  _VGrpId = grpid;
  if (ProductionProto::MaxVGrp < grpid)
    ProductionProto::MaxVGrp = grpid;
}

void EndVGroup() { _VGrpId = 0; }
