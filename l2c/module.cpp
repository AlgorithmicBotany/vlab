/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <vector>

#include <lparams.h>

#include "module.h"
#include "contextdata.h"
#include "production.h"
#include "file.h"

void l2cerror(const char *fmt, ...);
void GenerateProductionsLookup(char prefix, const ProductionTable &productions);
void GenerateInterpretationsLookup(const ProductionTable &productions);

extern FILE *fOut;
extern int ModuleCounter();
extern bool bModulesOnly;
extern int _considerGroupId;

ModuleTable moduleTable;

void StartGroup(int);

ProductionTable productionTable;
HomomorphismTable interpretationTable;
HomomorphismTable decompositionTable;
std::vector<__lc_GroupType> groups;

static ProductionType productionMode = eProduction;
ProductionType ProductionMode() { return productionMode; }

struct MaxDepthSpecified {
  bool Decomposition;
  bool Interpretation;
};

static MaxDepthSpecified mds = {false, false};
static bool ringlsys = false;

void SwitchToInterpretation() { productionMode = eInterpretation; }

void SwitchToDecomposition() { productionMode = eDecomposition; }

void SwitchToProduction() { productionMode = eProduction; }

static const ModuleDeclaration DclEmpty;

ModuleDeclaration::ModuleDeclaration() { _Ident[0] = 0; }

ModuleDeclaration::ModuleDeclaration(const char *ident,
                                     const ParametersList *pParams, int id) {
  _id = id;
  strcpy(_Ident, ident);
  if (NULL != pParams)
    memcpy(&_Params, pParams, sizeof(ParametersList));
  else
    _Params.count = 0;
}

void ModuleDeclaration::operator=(const ModuleDeclaration &src) {
  if (this != &src) {
    strcpy(_Ident, src._Ident);
    memcpy(&_Params, &(src._Params), sizeof(ParametersList));
    _id = src._id;
  }
}

void ModuleDeclaration::GenerateModId() const {
  fprintf(fOut, "const __lc_ModuleIdType %s_id = %d;", _Ident, _id);
  if (!bModulesOnly) {
    BuildGetModuleFuncPrototype();
    fprintf(fOut, ";");
  }
  BuildModuleParamsStructure();
}

void ModuleDeclaration::BuildGetModuleFuncPrototype() const {
  fprintf(fOut, "bool GetModule%s(", _Ident);
  // references to parameters
  for (int iParam = 0; iParam < _Params.count; ++iParam) {
    fprintf(fOut, "%s& param%d", _Params.Params[iParam], iParam);
    if (iParam < _Params.count - 1)
      fputs(", ", fOut);
  }
  fputs(")", fOut);
}

void ModuleDeclaration::BuildGetModuleFunc() const {
  BuildGetModuleFuncPrototype();
  fputs("\n", fOut);
  fprintf(fOut,
          "{\nconst %s* pParams = reinterpret_cast<const "
          "%s*>(GetModuleAddr(%s_id));\n",
          ModuleParamsStructName(), ModuleParamsStructName(), _Ident);
  fputs("if (0 == pParams)\n"
        "{ return false; }\n",
        fOut);
#ifndef NO_MEMCOPY
  if (_Params.count > 0) {
    fprintf(fOut, "%s lcParams; memcpy(&lcParams, pParams, sizeof(%s));",
            ModuleParamsStructName(), ModuleParamsStructName());
  }
#endif
  for (int iParam = 0; iParam < _Params.count; ++iParam) {
#ifdef NO_MEMCOPY
    fprintf(fOut, "param%d = pParams->data.Param%d;\n", iParam, iParam);
#else
    fprintf(fOut, "param%d = lcParams.data.Param%d;\n", iParam, iParam);
#endif
  }
  fputs("return true;\n"
        "}\n\n",
        fOut);
}

void ModuleDeclaration::BuildModuleParamsStructure() const {
  fprintf(fOut, "struct %s { struct Data {", ModuleParamsStructName());
  fprintf(fOut, " %s %s; ", ModuleIdType(), ModuleIdName());
  for (int iParam = 0; iParam < _Params.count; ++iParam) {
    fprintf(fOut, " %s %s; ", _Params.Params[iParam], ModuleParamName(iParam));
  }
  fprintf(fOut,
          " %s moduleId2; } data; }; const int __%s_ParamsStructSize = %s;",
          ModuleIdType(), _Ident, StructArrSize(ModuleParamsStructName()));
  fprintf(fOut, "struct %s { %s arr[__%s_ParamsStructSize]; };",
          ModuleParamsStructArrName(), ModuleIdType(), _Ident);
}

void ModuleDeclaration::DumpSize(FILE *fp) const {
  fprintf(fp, "{ \"%s\", ", _Ident);
  fprintf(fp, "sizeof(%s) }", ModuleParamsStructArrName());
#ifdef GENERATE_COMMENTS
  fprintf(fp, "/* %s_id = %d */", _Ident, _id);
#endif
}

void ModuleTable::Add(const ModuleDeclaration &Decl) {
  if (Decl.Id() <= _lastAddedId)
    l2cerror("Module ids must be in ascending order\n");
  _lastAddedId = Decl.Id();

  if (_arr.size() <= static_cast<size_t>(_lastArrEntry))
    _arr.push_back(Decl);
  else
    _arr[_lastArrEntry] = Decl;
  _Table[Decl.Ident()] = _lastArrEntry;
  ++_lastArrEntry;
}

const ModuleDeclaration &ModuleTable::Find(const char *ident) const {
  std::map<std::string, int>::const_iterator it;
  it = _Table.find(ident);
  if (it != _Table.end())
    return _arr[it->second];
  else
    return DclEmpty;
}

ModuleTable::ModuleTable() {
  _lastAddedId = -1;
  _lastArrEntry = 0;
}

bool ModuleTable::IsModuleIdent(const char *ident) const {
  const ModuleDeclaration &Res = Find(ident);
  return (strcmp(ident, Res.Ident()) == 0);
}

void ModuleTable::DumpSize(FILE *fp) const {
  for (int i = 0, ix = 0; i < _lastArrEntry; ++i) {
    if (i == _arr[ix].Id()) {
      _arr[ix].DumpSize(fp);
      ix++;
    } else {
      assert(i < _arr[ix].Id());
      fputs("{ \"\", 0 }", fp);
    }
    if (i < _lastAddedId)
      fputs(",", fp);
    if (3 == i % 4)
      fputs("\n", fp);
  }
}

static bool StartDeclared = false;
static bool StartEachDeclared = false;
static bool EndDeclared = false;
static bool EndEachDeclared = false;
static bool VerifyStringDeclared = false;

extern void l2cerror(const char *, ...);

const ModuleDeclaration *pMDecl = NULL;
int curprm = 0;

void StartGenerateProduce(const char *idnt) {
  assert(NULL != fOut);
  assert(NULL == pMDecl);
  pMDecl = &(moduleTable.Find(idnt));
#ifdef NO_MEMCOPY
  fprintf(fOut,
          "{ %s* p%sparams = "
          "reinterpret_cast<%s*>(GetNextModuleSpot(sizeof(%s))); ",
          pMDecl->ModuleParamsStructName(), idnt,
          pMDecl->ModuleParamsStructName(), pMDecl->ModuleParamsStructName());
  fprintf(fOut,
          "p%sparams->idarr[0] = %s_id; "
          "p%sparams->idarr[sizeof(%s)/sizeof(%s)-1] = %s_id; ",
          idnt, idnt, idnt, pMDecl->ModuleParamsStructName(),
          "__lc_ModuleIdType", idnt);
#else
  fprintf(fOut,
          "{"
          "__%s_ParamsStructArr_* p%sparams = "
          "reinterpret_cast<__%s_ParamsStructArr_*>(GetNextModuleSpot(sizeof(__"
          "%s_ParamsStructArr_)));",
          idnt, idnt, idnt, idnt);
  fprintf(fOut, "__%s_ParamsStruct_ %sParams; %sParams.data.moduleId = %s_id; ",
          idnt, idnt, idnt, idnt);
#endif
  curprm = 0;
}

void StartPropensity() {
  fputs("{ ", fOut);
  StartGenerateProduce("Propensity");
#ifdef NO_MEMCOPY
  fputs("pPropensityparams->data.Param0 = ", fOut);
#else
  fputs("PropensityParams.data.Param0 = ", fOut);
#endif
}

void StartGProduce() {
#ifdef NO_MEMCOPY
  fputs("; ", fOut);
#else
  fputs("; memcpy(pPropensityparams, &PropensityParams, "
        "sizeof(__Propensity_ParamsStruct_)); ",
        fOut);
#endif
  fputs("pPropensityparams->arr[0] = "
        "pPropensityparams->arr[__Propensity_ParamsStructSize-1] = "
        "Propensity_id; }",
        fOut);
  pMDecl = NULL;
}

void EndGenerateProduce() {
#ifndef NO_MEMCOPY
  fprintf(fOut, "memcpy(p%sparams, &%sParams, sizeof(__%s_ParamsStruct_));",
          pMDecl->Ident(), pMDecl->Ident(), pMDecl->Ident());
#endif
  fprintf(
      fOut,
      "p%sparams->arr[0] = p%sparams->arr[__%s_ParamsStructSize-1] = %s_id;",
      pMDecl->Ident(), pMDecl->Ident(), pMDecl->Ident(), pMDecl->Ident());
  fprintf(fOut, "} ");
  if (curprm != pMDecl->Params().count)
    l2cerror("Invalid number of parameters in module %s", pMDecl->Ident());
  pMDecl = NULL;
}

void ParameterCast() {
  if (pMDecl->Params().count > 0) {
    if (curprm < pMDecl->Params().count)
#ifdef NO_MEMCOPY
      fprintf(fOut, "p%sparams->data.Param%d = (", pMDecl->Ident(), curprm);
#else
      fprintf(fOut, "%sParams.data.Param%d = (", pMDecl->Ident(), curprm);
#endif
    else
      l2cerror("Too many parameters in module %s", pMDecl->Ident());
  }
}

void EndParameterCast() {
  if (curprm < pMDecl->Params().count) {
    fprintf(fOut, ");");
    curprm++;
  }
}

void StartAxiom() { fputs("void Axiom(){ ", fOut); }

void EndAxiom() { fputs("}", fOut); }

void StartProduce() { fprintf(fOut, "{ __res_ = true; "); }

void EndProduce() { fprintf(fOut, "return;}"); }

void EndGProduce() { fputs("}", fOut); }

void StartNProduce() {
  //  fprintf(fOut, "{ __res_ = true; ");
  // This line has been changed by Pascal, if we use only '{' it causes a bug in
  // Adam's leaf model ...
  fprintf(fOut, "{ ");
}

void EndNProduce() { fprintf(fOut, " }"); }

void ExpandStart() {
  if (StartDeclared)
    l2cerror("Start statement already declared");
  else {
    fprintf(fOut, "void Start()");
    StartDeclared = true;
  }
}

void ExpandStartEach() {
  if (StartEachDeclared)
    l2cerror("StartEach statement already declared");
  else {
    fprintf(fOut, "void StartEach()");
    StartEachDeclared = true;
  }
}

void ExpandEndEach() {
  if (EndEachDeclared)
    l2cerror("EndEach statement already declared");
  else {
    fprintf(fOut, "void EndEach()");
    EndEachDeclared = true;
  }
}

void ExpandEnd() {
  if (EndDeclared)
    l2cerror("End statement already declared");
  else {
    fprintf(fOut, "void End()");
    EndDeclared = true;
  }
}

void BuildCallers(ProductionType prodType) {
  ProductionTableIterator *iter;
  if (prodType == eProduction) {
    iter = new ProductionTableIterator(productionTable);
  }
  if (prodType == eDecomposition) {
    iter = new ProductionTableIterator(decompositionTable);
  }
  if (prodType == eInterpretation) {
    iter = new ProductionTableIterator(interpretationTable);
  }
  while (!(iter->AtEnd())) {
    const ProductionProto *pProto = iter->GetCurrent();
    pProto->GenerateCaller();
    ++(*iter);
  }
  //"Why isn't iter deleted?"
  //  delete iter;
}

int BuildProdProtos(int, ProductionType);
void BuildProdProtos(ProductionType prodType) {
  char tablePrefix;
  const int MaxGrp = groups.size() - 1;
  std::vector<int> prdcnt(MaxGrp + 1);
  int i;
  for (i = 0; i <= MaxGrp; ++i) {
    prdcnt[i] = BuildProdProtos(i, prodType);
  }
  switch (prodType) {
  case eProduction:
    tablePrefix = 'P';
    break;
  case eDecomposition:
    tablePrefix = 'D';
    break;
  case eInterpretation:
    tablePrefix = 'I';
    break;
  }

  fprintf(fOut, "static const __lc_ProductionPredecessor* %ctables[] =\n{\n",
          tablePrefix);
  for (i = 0; i <= MaxGrp; ++i) {
    fprintf(fOut, "%cpred%d", tablePrefix, i);
    if (i < MaxGrp)
      fputs(", ", fOut);
  }
  fputs("\n};\n\n", fOut);

  fprintf(fOut, "static int %cProdCount[] =\n{\n", tablePrefix);
  for (i = 0; i <= MaxGrp; ++i) {
    fprintf(fOut, "%d", prdcnt[i]);
    if (i < MaxGrp)
      fputs(", ", fOut);
  }
  fputs("\n};\n\n", fOut);
}

int BuildProdProtos(int iGroup, ProductionType prodType) {
  ProductionTableIterator *iter = NULL;
  ProductionTable *tableOfType = NULL;
  char tablePrefix;

  switch (prodType) {
  case eProduction:
    tableOfType = &(productionTable);
    iter = new ProductionTableIterator(productionTable);
    tablePrefix = 'P';
    break;
  case eDecomposition:
    tableOfType = &(decompositionTable);
    iter = new ProductionTableIterator(decompositionTable);
    tablePrefix = 'D';
    break;
  case eInterpretation:
    tableOfType = &(interpretationTable);
    iter = new ProductionTableIterator(interpretationTable);
    tablePrefix = 'I';
    break;
  }

  int count = 0;
  fprintf(fOut, "static const __lc_ProductionPredecessor %cpred%d[] =\n{\n",
          tablePrefix, iGroup);
  if (tableOfType->Items() > 0) {
    while (!(iter->AtEnd())) {
      ProductionProto *pProto = iter->AccessCurrent();
      if (pProto->Group() == iGroup) {
        if (count > 0)
          fputs(",\n", fOut);
        pProto->GenerateProtoEntry();
        pProto->SetInArray(count);
        ++count;
      }
      ++(*iter);
    }
  }
  if (count == 0)
    ProductionProto::GenerateEmptyProtoEntry(fOut);
  fputs("\n};\n", fOut);
#pragma message("Why aren't the iterators deleted???")
  //  delete iter;
  //  delete tableOfType;
  return count;
}

void BuildModuleSize() {
  fputs("const __lc_ModuleData moduleData[] =\n{\n", fOut);
  moduleTable.DumpSize(fOut);
  fputs("\n};\n\n", fOut);
}

void BuildConsiderGroups() {
  fprintf(fOut, "int NumOfConsiderGroups() { return %d; }\n\n",
          _considerGroupId + 1);

  fprintf(fOut, "static const __lc_ModuleIdType __ignoreArr0[] = { 0 };\n"
                "static const __lc_ModuleIdType __considerArr0[] = { 0 };\n\n");

  fprintf(fOut, "static const int __ignoreCount[] = { ");
  for (int iConsiderGroup = 0; iConsiderGroup <= _considerGroupId;
       ++iConsiderGroup) {
    fprintf(fOut, "sizeof(__ignoreArr%d)/sizeof(__ignoreArr%d[0])-1, ",
            iConsiderGroup, iConsiderGroup);
  }
  fprintf(fOut, "0 };\n");

  fprintf(fOut, "static const int __considerCount[] = { ");
  for (int iConsiderGroup = 0; iConsiderGroup <= _considerGroupId;
       ++iConsiderGroup) {
    fprintf(fOut, "sizeof(__considerArr%d)/sizeof(__considerArr%d[0])-1, ",
            iConsiderGroup, iConsiderGroup);
  }
  fprintf(fOut, "0 };\n");

  fprintf(fOut, "static const __lc_ModuleIdType* __considerArray[] = {\n");
  for (int iConsiderGroup = 0; iConsiderGroup <= _considerGroupId;
       ++iConsiderGroup) {
    fprintf(fOut, "__considerArr%d, ", iConsiderGroup);
  }
  fprintf(fOut, "0 };\n\n");

  fprintf(fOut, "static const __lc_ModuleIdType* __ignoreArray[] = {\n");
  for (int iConsiderGroup = 0; iConsiderGroup <= _considerGroupId;
       ++iConsiderGroup) {
    fprintf(fOut, "__ignoreArr%d, ", iConsiderGroup);
  }
  fprintf(fOut, "0 };\n\n");

  fputs("int NumOfConsidered(int iConsiderGroup) { return "
        "__considerCount[iConsiderGroup]; }\n\n",
        fOut);
  fputs("int NumOfIgnored(int iConsiderGroup) { return "
        "__ignoreCount[iConsiderGroup]; }\n\n",
        fOut);

  fputs("__lc_ModuleIdType GetConsidered(int iModule, int iConsiderGroup)\n"
        "{ return __considerArray[iConsiderGroup][iModule]; }\n\n",
        fOut);

  fputs("__lc_ModuleIdType GetIgnored(int iModule, int iConsiderGroup)\n"
        "{ return __ignoreArray[iConsiderGroup][iModule]; }\n\n",
        fOut);
}

void GenerateFixed() {
  fputs("\nconst __lc_ModuleData* GetModuleData(int i)\n"
        "{ return &(moduleData[i]); }\n\n",
        fOut);

  fprintf(fOut,
          "\nint NumOfModules()\n"
          "{ return %d; }\n",
          moduleTable.MaxModuleId() + 1);

  if (!StartDeclared)
    fputs("void Start() {}\n", fOut);

  if (!StartEachDeclared)
    fputs("void StartEach() {}\n", fOut);

  if (!EndEachDeclared)
    fputs("void EndEach() {}\n", fOut);

  if (!EndDeclared)
    fputs("void End() {}\n", fOut);

  fputs("\nint NumOfProductions(int grp)\n"
        "{ return PProdCount[grp]; }\n",
        fOut);

  fputs("\nint NumOfDecompositions(int grp)\n"
        "{ return DProdCount[grp]; }\n",
        fOut);

  fputs("\nint NumOfInterpretations(int grp)\n"
        "{ return IProdCount[grp]; }\n",
        fOut);

  fputs("\nconst __lc_ProductionPredecessor& GetProductionPredecessor(int grp, "
        "int i)\n"
        "{ return Ptables[grp][i]; }\n",
        fOut);

  fputs("\nconst __lc_ProductionPredecessor& GetDecompositionPredecessor(int "
        "grp, int i)\n"
        "{ return Dtables[grp][i]; }\n",
        fOut);
  fprintf(
      fOut,
      "\nconst __lc_ProductionPredecessor& GetInterpretationPredecessor(int grp, int i, int vgrp)\n \
		{ \n \
		\treturn Itables[grp][i*(1+vgrp)]; \n \
		}\n");

  fprintf(fOut, "int NumOfViews() { return %d; }\n",
          ProductionProto::MaxVGrp + 1);

  BuildConsiderGroups();

  if (!VerifyStringDeclared) {
    fputs("int NumOfVerify() { return 0; }\n"
          "__lc_ModuleIdType GetVerify(int) { return -1; }\n",
          fOut);
  }

  if (!mds.Decomposition)
    fputs("int DecompositionMaxDepth() { return 1; }\n", fOut);

  if (!mds.Interpretation)
    fputs("int InterpretationMaxDepth() { return 1; }\n", fOut);

  if (!ringlsys)
    fputs("int RingLsystem() { return 0; }\n", fOut);

  const int MaxGrp = groups.size() - 1;
  fprintf(fOut, "int NumOfTables() { return %d; }\n", MaxGrp + 1);

  fprintf(fOut, "__lc_GroupType aGroupType[%lu] =\n{\n", groups.size());

  for (std::vector<__lc_GroupType>::const_iterator it = groups.begin();
       it != groups.end(); ++it) {
    if (it != groups.begin())
      fputs(",", fOut);

    switch (*it) {
    case __lc_gtUnspecified:
      fputs("__lc_gtUnspecified", fOut);
      break;
    case __lc_gtLsystem:
      fputs("__lc_gtLsystem", fOut);
      break;
    case __lc_gtGillespie:
      fputs("__lc_gtGillespie", fOut);
      break;
    }
  }

  fprintf(fOut, "\n};\n__lc_GroupType GetDerivationType(int id)\n"
                "{ return aGroupType[id]; }\n");

  for (int iModule = 0; iModule < moduleTable.Items(); ++iModule) {
    const ModuleDeclaration &module = moduleTable.GetItem(iModule);
    module.BuildGetModuleFunc();
  }

  GenerateProductionsLookup('P', productionTable);
  GenerateProductionsLookup('D', decompositionTable);
  // GenerateProductionsLookup('I', interpretationTable);
  GenerateInterpretationsLookup(interpretationTable);
}

void GenerateProductionsLookup(char prefix,
                               const ProductionTable &productions) {
  ProductionTableIterator itProduction(productions);

  fprintf(fOut, "static int %cProductionModuleCount[] =\n{\n", prefix);
  for (size_t iGroup = 0; iGroup < groups.size(); ++iGroup) {
    for (int iModuleId = 0; iModuleId < moduleTable.MaxModuleId() + 1;
         ++iModuleId) {
      int iCount = 0;
      itProduction.Reset();
      while (!itProduction.AtEnd()) {
        const ProductionProto *pPrototype = itProduction.GetCurrent();
        if ((size_t) pPrototype->Group() == iGroup) {
          if ((pPrototype->GetStrct()->GetHead()->id == iModuleId) ||
              (pPrototype->GetStrct()->CountModules() > 1 &&
               pPrototype->GetStrct()->GetTail()->id == iModuleId))
            ++iCount;
        }
        ++itProduction;
      }
      fprintf(fOut, "%d, ", iCount);
    }
    fprintf(fOut, "\n");
  }
  fprintf(fOut, "};\n\n");

  fprintf(
      fOut,
      "int NumOfModule%cProductions(int iGroup, __lc_ModuleIdType moduleId)\n",
      prefix);
  fprintf(fOut,
          "{ return %cProductionModuleCount[iGroup * NumOfModules() + "
          "moduleId]; }\n",
          prefix);

  fprintf(fOut, "static int Module%cProductions[] = \n{\n", prefix);
  std::vector<int> ModuleGroups((moduleTable.MaxModuleId() + 1) *
                                groups.size());

  int iModuleProductionsEntry = 0;
  int iCurrentModuleGroupsEntry = 0;
  for (size_t iGroup = 0; iGroup < groups.size(); ++iGroup) {
    fprintf(fOut, "\n\n// Group %zu\n", iGroup);
    for (int iModuleId = 0; iModuleId < moduleTable.MaxModuleId() + 1;
         ++iModuleId) {
      ModuleGroups[iCurrentModuleGroupsEntry] = -1;
      itProduction.Reset();
      while (!itProduction.AtEnd()) {
        const ProductionProto *pPrototype = itProduction.GetCurrent();
        if ((size_t)pPrototype->Group() == iGroup) {
          if ((pPrototype->GetStrct()->GetHead()->id == iModuleId) ||
              (pPrototype->GetStrct()->CountModules() > 1 &&
               pPrototype->GetStrct()->GetTail()->id == iModuleId)) {
            fprintf(fOut, "%d, ", pPrototype->GetInArray());
            if (-1 == ModuleGroups[iCurrentModuleGroupsEntry])
              ModuleGroups[iCurrentModuleGroupsEntry] = iModuleProductionsEntry;
            ++iModuleProductionsEntry;
          }
        }

        ++itProduction;
      }
      if (-1 == ModuleGroups[iCurrentModuleGroupsEntry]) {
        fprintf(fOut, "-1, ");
        ModuleGroups[iCurrentModuleGroupsEntry] = iModuleProductionsEntry;
        ++iModuleProductionsEntry;
      }
      fprintf(fOut, "// %s starts at %d",
              moduleTable.GetItem(iModuleId).Ident(),
              ModuleGroups[iCurrentModuleGroupsEntry]);

      fprintf(fOut, "\n");
      ++iCurrentModuleGroupsEntry;
    }
  }

  fprintf(fOut, "};\n\n");

  fprintf(fOut, "static int %cProductionGroupsStart[] =\n", prefix);
  fprintf(fOut, "{\n");

  int iNewLineCounter = 0;
  for (std::vector<int>::const_iterator it = ModuleGroups.begin();
       it != ModuleGroups.end(); ++it) {
    fprintf(fOut, "%d, ", *it);
    ++iNewLineCounter;
    if (16 == iNewLineCounter) {
      fprintf(fOut, "\n");
      iNewLineCounter = 0;
    }
  }
  fprintf(fOut, "\n};\n");

  fprintf(
      fOut,
      "const __lc_ProductionPredecessor& GetModule%cProductionPredecessor(int "
      "iGroup, __lc_ModuleIdType moduleId, int id)\n"
      "{\n"
      "	const __lc_ProductionPredecessor* array = %ctables[iGroup];\n"
      "	int index = Module%cProductions[%cProductionGroupsStart[iGroup * "
      "NumOfModules() + moduleId] + id];\n"
      "	return array[index];\n"
      "}\n",
      prefix, prefix, prefix, prefix);
}

void GenerateInterpretationsLookup(const ProductionTable &productions) {
  ProductionTableIterator itProduction(productions);

  fprintf(fOut, "static int IProductionModuleCount[] = \n{\n");

  for (size_t iGroup = 0; iGroup < groups.size(); ++iGroup) {
    fprintf(fOut, "// Group: %zu\n", iGroup);
    for (int iVGroup = 0; iVGroup < ProductionProto::MaxVGrp + 1; ++iVGroup) {
      fprintf(fOut, "// VGroup: %d\n", iVGroup);
      for (int iModuleId = 0; iModuleId < moduleTable.MaxModuleId() + 1;
           ++iModuleId) {
        int iCount = 0;
        itProduction.Reset();
        while (!itProduction.AtEnd()) {
          const ProductionProto *pPrototype = itProduction.GetCurrent();
          if ((size_t)pPrototype->Group() == iGroup &&
              pPrototype->VGroup() == iVGroup) {
            if ((pPrototype->GetStrct()->GetHead()->id == iModuleId) ||
                (pPrototype->GetStrct()->CountModules() > 1 &&
                 pPrototype->GetStrct()->GetTail()->id == iModuleId))
              ++iCount;
          }
          ++itProduction;
        }
        fprintf(fOut, "%d, ", iCount);
      }
      fprintf(fOut, "\n");
    }
    fprintf(fOut, "\n");
  }

  fprintf(fOut, "};\n\n");
  fprintf(fOut, "int NumOfModuleIProductions(int iGroup, int iVGroup, "
                "__lc_ModuleIdType moduleId)\n");
  fprintf(fOut, "{ return IProductionModuleCount[iGroup * NumOfModules() * "
                "NumOfViews() + iVGroup * NumOfModules() + moduleId]; }\n");

  fprintf(fOut, "static int ModuleIProductions[] = \n{\n");
  std::vector<int> ModuleGroups((moduleTable.MaxModuleId() + 1) *
                                groups.size() * (ProductionProto::MaxVGrp + 1));

  int iModuleProductionsEntry = 0;
  int iCurrentModuleGroupsEntry = 0;
  for (size_t iGroup = 0; iGroup < groups.size(); ++iGroup) {
    fprintf(fOut, "\n// Group %zu\n", iGroup);
    for (int iVGroup = 0; iVGroup < ProductionProto::MaxVGrp + 1; ++iVGroup) {
      fprintf(fOut, "\n\t// VGroup %d'\n", iVGroup);
      for (int iModuleId = 0; iModuleId < moduleTable.MaxModuleId() + 1;
           ++iModuleId) {
        ModuleGroups[iCurrentModuleGroupsEntry] = -1;
        itProduction.Reset();
        while (!itProduction.AtEnd()) {
          const ProductionProto *pPrototype = itProduction.GetCurrent();
          if ((size_t)pPrototype->Group() == iGroup &&
              pPrototype->VGroup() == iVGroup) {
            if (pPrototype->GetStrct()->GetHead()->id == iModuleId ||
                (pPrototype->GetStrct()->CountModules() > 1 &&
                 pPrototype->GetStrct()->GetTail()->id == iModuleId)) {
              fprintf(fOut, "%d, ", pPrototype->GetInArray());
              if (-1 == ModuleGroups[iCurrentModuleGroupsEntry])
                ModuleGroups[iCurrentModuleGroupsEntry] =
                    iModuleProductionsEntry;
              ++iModuleProductionsEntry;
            }
          }
          ++itProduction;
        }
        if (-1 == ModuleGroups[iCurrentModuleGroupsEntry]) {
          fprintf(fOut, "-1, ");
          ModuleGroups[iCurrentModuleGroupsEntry] = iModuleProductionsEntry;
          ++iModuleProductionsEntry;
        }
        fprintf(fOut, "// %s starts at %d",
                moduleTable.GetItem(iModuleId).Ident(),
                ModuleGroups[iCurrentModuleGroupsEntry]);
        fprintf(fOut, "\n");
        ++iCurrentModuleGroupsEntry;
      }
    }
  }

  fprintf(fOut, "};\n\n");
  fprintf(fOut, "static int IProductionGroupsStart[] =\n{\n");

  int iNewLineCounter = 0;
  for (std::vector<int>::const_iterator it = ModuleGroups.begin();
       it != ModuleGroups.end(); ++it) {
    fprintf(fOut, "%d, ", *it);
    ++iNewLineCounter;
    if (16 == iNewLineCounter) {
      fprintf(fOut, "\n");
      iNewLineCounter = 0;
    }
  }
  fprintf(fOut, "\n};\n");

  fprintf(
      fOut,
      "const __lc_ProductionPredecessor& GetModuleIProductionPredecessor(int "
      "iGroup, int iVGroup, __lc_ModuleIdType moduleId, int id)\n"
      "{\n"
      "	const __lc_ProductionPredecessor* array = Itables[iGroup];\n"
      "	int index = ModuleIProductions[IProductionGroupsStart[iGroup * "
      "NumOfModules() * NumOfViews() + iVGroup * NumOfModules() + moduleId] + "
      "id];\n"
      "	return array[index];\n"
      "}\n");
}

FormalModuleDt::FormalModuleDt(const char *idnt,
                               const ParametersList *pParams) {
  strncpy(Ident, idnt, sizeof(Ident));
  // are there any parameters?
  if (pParams != NULL) {
    Params = (*pParams);
  } else {
    Params.count = 0;
  }
  pNext = NULL;
  const ModuleDeclaration &mdecl = moduleTable.Find(idnt);
  id = mdecl.Id();
}

int FormalModuleDt::GenerateForPrototype() const {
  const ModuleDeclaration &decl = moduleTable.Find(Ident);
  const ParametersList &ParamList = decl.Params();
  if (Params.count != ParamList.count)
    l2cerror("Invalid number of parameters for module %s (wanted %i, got %i)",
             Ident, ParamList.count, Params.count);

  for (int i = 0; i < Params.count; i++)
    fprintf(fOut, ", %s %s", ParamList.Params[i], Params.Params[i]);

  return Params.count;
}

FormalModuleDtList::FormalModuleDtList(FormalModuleDt *pFirst) {
  assert(NULL == pFirst->pNext);
  _pFirst = _pLast = pFirst;
  _modules = 1;
}

FormalModuleDtList::~FormalModuleDtList() {
  while (NULL != _pFirst) {
    FormalModuleDt *pNext = _pFirst->pNext;
    delete _pFirst;
    _pFirst = pNext;
  }
}

void FormalModuleDtList::Add(FormalModuleDt *pNext) {
  assert(NULL == pNext->pNext);
  _pLast->pNext = pNext;
  _pLast = pNext;
  _modules++;
}

int FormalModuleDtList::CountParams() const {
  const FormalModuleDt *pCur = _pFirst;
  int res = 0;
  while (NULL != pCur) {
    res += pCur->NumOfParams();
    pCur = pCur->pNext;
  }
  return res;
}

bool FormalModuleDtList::Contains(const char *idnt) const {
  const FormalModuleDt *pCur = _pFirst;
  while (NULL != pCur) {
    if (!(strcmp(pCur->Ident, idnt)))
      return true;
    pCur = pCur->pNext;
  }
  return false;
}

void FormalModuleDtList::GenerateForPrototype() const {
  const FormalModuleDt *pCur = _pFirst;
  while (NULL != pCur) {
    pCur->GenerateForPrototype();
    pCur = pCur->pNext;
  }
}

void FormalModuleDtList::GenerateProtoEntry() const {
  const FormalModuleDt *pCur = _pFirst;
  int count = 0;
  fprintf(fOut, "{ ");
  while (NULL != pCur) {
    fprintf(fOut, "%s_id", pCur->Ident);
    count++;
    pCur = pCur->pNext;
    if (NULL != pCur)
      fprintf(fOut, ", ");
  }
  fprintf(fOut, " }, %d", count);
}

void FormalModuleDtList::Reverse() {
  FormalModuleDt *pCurrent = _pFirst;
  FormalModuleDt *pPrevious = NULL;
  FormalModuleDt *pNext = NULL;
  _pLast = pCurrent;
  while (NULL != pCurrent) {
    pNext = pCurrent->pNext;
    pCurrent->pNext = pPrevious;
    pPrevious = pCurrent;
    pCurrent = pNext;
  }
  _pFirst = pPrevious;
}

void StartConsider() {
  ++_considerGroupId;
  fprintf(fOut, "static const __lc_ModuleIdType __considerArr%d[] = { ",
          _considerGroupId);
}

void StartIgnore() {
  ++_considerGroupId;
  fprintf(fOut, "static const __lc_ModuleIdType __ignoreArr%d[] = { ",
          _considerGroupId);
}

void StartVerify() {
  fputs("static const __lc_ModuleIdType __verifyStringArr[] = { ", fOut);
}

void AppendConIgnModule(const char *midnt) {
  fputs(midnt, fOut);
  fputs("_id,", fOut);
}

void EndConsider() {
  fputs("0 };", fOut);
  fprintf(fOut, " static const __lc_ModuleIdType __ignoreArr%d[] = { 0 }; ",
          _considerGroupId);
}

void EndIgnore() {
  fputs("0 };", fOut);
  fprintf(fOut, " static const __lc_ModuleIdType __considerArr%d[] = { 0 }; ",
          _considerGroupId);
}

void EndVerify() {
  fputs("0 };", fOut);
  fputs("int NumOfVerify()"
        "{ return sizeof(__verifyStringArr)/sizeof(__verifyStringArr[0])-1; }"
        "__lc_ModuleIdType GetVerify(int i)"
        "{ return __verifyStringArr[i]; }",
        fOut);
  VerifyStringDeclared = true;
}

void StartDerivLength() { fputs("int DerivationLength() { return ", fOut); }

void EndDerivLength() { fputs("}", fOut); }

void StartRingLsystem() {
  fputs("int RingLsystem() { return ", fOut);
  ringlsys = true;
}

void EndRingLsystem() { fputs("}", fOut); }

void StartMaxDepth() {
  switch (ProductionMode()) {
  case eInterpretation:
    if (!mds.Interpretation) {
      fputs("int InterpretationMaxDepth() { return ", fOut);
      mds.Interpretation = true;
    } else
      l2cerror("Interpretation max depth defined already");
    break;
  case eDecomposition:
    if (!mds.Decomposition) {
      fputs("int DecompositionMaxDepth() { return ", fOut);
      mds.Decomposition = true;
    } else
      l2cerror("Decomposition max depth defined already");
    break;
  default:
    l2cerror("Max depth statement not allowed here");
  }
}

void EndMaxDepth() { fputs("}", fOut); }

void InitDefault() { StartGroup(0); }
