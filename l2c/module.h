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



#ifndef __MODULE_H__
#define __MODULE_H__

#ifdef WIN32
#pragma warning(disable : 4786)
#endif

#include <map>
#include <string>
#include <vector>

struct ParametersList {
  char Params[__lc_eMaxIdentifierLength + 1][__lc_eMaxParams];
  int count;
};

class ModuleDeclaration {
public:
  ModuleDeclaration();
  ModuleDeclaration(const char *, const ParametersList *, int);
  const char *Ident() const { return _Ident; }
  void operator=(const ModuleDeclaration &);
  const ParametersList &Params() const { return _Params; }
  int Id() const { return _id; }
  void GenerateModId() const;
  void DumpSize(FILE *) const;
  const char *ModuleParamsStructName() const {
    const int BfSize = 128;
    static char bf[BfSize];
    sprintf(bf, "__%s_ParamsStruct_", _Ident);
    return bf;
  }

  const char *ModuleParamsStructArrName() const {
    const int BfSize = 128;
    static char bf[BfSize];
    sprintf(bf, "__%s_ParamsStructArr_", _Ident);
    return bf;
  }

  const char *StructArrSize(const char *szStructName) const {
    const int BfSize = 512;
    static char bf[BfSize];
    sprintf(bf,
            "sizeof(%s) %% sizeof(%s) == 0 ? "
            "sizeof(%s) / sizeof(%s) : sizeof(%s) / sizeof(%s) + 1",
            szStructName, ModuleIdType(), szStructName, ModuleIdType(),
            szStructName, ModuleIdType());
    return bf;
  }

  const char *ModuleUnionItemName() const {
    const int BfSize = 128;
    static char bf[BfSize];
    sprintf(bf, "module_%s_Item", _Ident);
    return bf;
  }

  void BuildGetModuleFunc() const;

private:
  const char *ModuleParamName(int iParam) const {
    const int BfSize = 128;
    static char bf[BfSize];
    sprintf(bf, "Param%d", iParam);
    return bf;
  }
  const char *ModuleIdType() const { return "__lc_ModuleIdType"; }
  const char *ModuleIdName() const { return "moduleId"; }
  void BuildGetModuleFuncPrototype() const;
  void BuildModuleParamsStructure() const;
  char _Ident[__lc_eMaxIdentifierLength + 1];
  ParametersList _Params;
  int _id;
};

class ModuleTable {
public:
  ModuleTable();
  void Add(const ModuleDeclaration &);
  const ModuleDeclaration &Find(const char *) const;
  bool IsModuleIdent(const char *) const;
  void DumpSize(FILE *) const;
  int Items() const { return _arr.size(); }
  int MaxModuleId() const { return _lastAddedId; }
  const ModuleDeclaration &GetItem(int iItem) const { return _arr[iItem]; }

private:
  std::vector<ModuleDeclaration> _arr;
  typedef std::map<std::string, int> AssocTableOfModuleDeclaration;
  AssocTableOfModuleDeclaration _Table;
  int _lastAddedId;
  int _lastArrEntry;
};

class FormalModuleDt {
public:
  FormalModuleDt(const char *, const ParametersList *);
  int NumOfParams() const { return Params.count; }
  int GenerateForPrototype() const;
  char Ident[__lc_eMaxIdentifierLength + 1];
  ParametersList Params;
  FormalModuleDt *pNext;
  int id;

private:
  void operator=(const FormalModuleDt &);
};

class FormalModuleDtList {
public:
  FormalModuleDtList(FormalModuleDt *);
  ~FormalModuleDtList();
  void Add(FormalModuleDt *);
  FormalModuleDt *GetHead() const { return _pFirst; }
  FormalModuleDt *GetTail() const { return _pLast; }
  int CountParams() const;
  void GenerateForPrototype() const;
  void GenerateProtoEntry() const;
  bool Contains(const char *) const;
  int CountModules() const { return _modules; }
  void Reverse();

private:
  FormalModuleDt *_pFirst;
  FormalModuleDt *_pLast;
  int _modules;
  void operator=(const FormalModuleDtList &);
};

extern ModuleTable moduleTable;

#endif
