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
#include <string.h>
#include <stdio.h>

#include <file.h>
#include <sarray.h>
#include <dynarray.h>
#include <lpuparams.h>
#include <labels.h>
#include <lpu.h>

#include "lfuncsC.h"
#include "lfuncs.h"
#include "control.h"

LPU lpu;

PrototypeArray LPrototypes;

void ClearLFuncs() {
  LPrototypes.Clear();
  lpu.Clear();
}

void PreprocessForL(const char *src, char *output) {
#ifdef WIN32
#error Not implemented yet
#else
  static char cmnd[1024];
  strcpy(output, "/tmp/lfunc.XXXXXX");
  mktemp(output);
  sprintf(cmnd, "%s -DLCODE %s > %s", clp.preprocessor, src, output);
  system(cmnd);
#endif
}

int GenerateLCode(const char *src, char *lcode) {
#ifdef WIN32
#error Not implemented yet
#else
  static char cmnd[1024];
  strcpy(lcode, "/tmp/lcode.XXXXXX");
  mktemp(lcode);
  sprintf(cmnd, "/home/jungle/radekk/Projects/LPU/GenFun1.0/lgen %s %s", src,
          lcode);
  return system(cmnd);
#endif
}

void ReadLFuncs(char *src) {
  strcat(src, ".prot");
  LPrototypes.Load(src);
  {
    char *dot = strrchr(src, '.');
    *dot = 0;
  }
  if (LPrototypes.Count() > 0) {
    ReadTextFile ld(src);
    lpu.Load(ld);
    for (int i = 0; i < LPrototypes.Count(); i++) {
      int addr = lpu.FindFunc(LPrototypes[i].Name());
      LPrototypes[i].SetAddress(addr);
    }
  }
}

int FindLFunc(const char *nm) {
  int res = LPrototypes.Find(nm);
  if (-1 == res)
    return 0;
  else
    return res + LFuncBaseId;
}

int NumOfParamsForLFunc(int id) {
  assert(LPrototypes.IsFuncId(id));
  return LPrototypes.NumOfParams(id);
}

void CallLFunction(int id, double *stack, int *pOffset) {
  LPrototypes.Call(id, stack, pOffset);
}

Prototype::Prototype() {
  _Type = -1;
  _ArgCount = -1;
  _Name[0] = 0;
}

void Prototype::operator=(const Prototype &src) {
  _Type = src._Type;
  _ArgCount = src._ArgCount;
  for (int i = 0; i < _ArgCount; i++)
    _Args[i] = src._Args[i];
  strcpy(_Name, src._Name);
}

bool Prototype::Load(ReadTextFile &src) {
  int res = fscanf(src, "%d %s %d", &_Type, _Name, &_ArgCount);
  if (3 != res)
    return false;
  for (int i = 0; i < _ArgCount; i++)
    if (1 != fscanf(src, "%d", &(_Args[i])))
      return false;
  return true;
}

PrototypeArray::PrototypeArray() : DynArray<Prototype>(32) {}

void PrototypeArray::Clear() { _items = 0; }

void PrototypeArray::Load(const char *nm) {
  ReadTextFile src(nm);
  if (!(src.Valid()))
    return;
  Prototype p;
  while (p.Load(src)) {
    Add(p);
  }
}

int PrototypeArray::Find(const char *nm) const {
  for (int i = 0; i < Count(); i++) {
    if (!(strcmp(nm, _arr[i].Name())))
      return i;
  }
  return -1;
}

bool PrototypeArray::IsFuncId(int id) const {
  if (id < 0)
    return false;
  if (id >= Count())
    return false;
  return true;
}

int PrototypeArray::NumOfParams(int id) const {
  assert(id >= 0);
  assert(id < Count());
  return _arr[id].ArgCount();
}

void PrototypeArray::Call(int id, double *stack, int *pOffset) {
  int ac = NumOfParams(id);
  int offset = *pOffset;
  FloatType farg;
  int iarg;
  for (int i = ac - 1; i >= 0; i--) {
    switch (ArgType(id, i)) {
    case 0: // Int
    {
      iarg = int(stack[offset]);
      offset--;
      lpu.Push(iarg);
    } break;
    case 1: // Float
    {
      farg = stack[offset];
      offset--;
      lpu.Push(farg);
    } break;
    }
  }
  lpu.Push(0); // Reseve space for return value
  lpu.Push(-1);
  lpu.Call(_arr[id].Address());
  switch (Type(id)) {
  case 0: // Int
  {
    iarg = lpu.PopInt();
    offset++;
    stack[offset] = iarg;
  } break;
  case 1: // Float
  {
    farg = lpu.PopFloat();
    offset++;
    stack[offset] = farg;
  } break;
  }
  // Clean stack from arguments
  lpu.ClearStack();
  *pOffset = offset;
}
