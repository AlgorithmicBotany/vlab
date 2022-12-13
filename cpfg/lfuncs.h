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



#ifndef __LFUNCS_H__
#define __LFUNCS_H__

const int MaxIdentLength = 32;

class ReadTextFile;

class Prototype {
public:
  Prototype();
  void operator=(const Prototype &);
  bool Load(ReadTextFile &);
  const char *Name() const { return _Name; }
  void SetAddress(int addr) { _Address = addr; }
  int Address() const { return _Address; }
  int ArgCount() const { return _ArgCount; }
  int ArgType(int i) const {
    assert(i >= 0);
    assert(i < _ArgCount);
    return _Args[i];
  }
  int Type() const { return _Type; }

private:
  int _Type;
  int _ArgCount;
  int _Args[32];
  int _Address;
  char _Name[MaxIdentLength + 1];
};

class PrototypeArray : public DynArray<Prototype> {
public:
  PrototypeArray();
  void Clear();
  void Load(const char *);
  int Find(const char *) const;
  int NumOfParams(int) const;
  bool IsFuncId(int) const;
  void Call(int, double *, int *);
  int ArgType(int id, int aid) const {
    assert(id >= 0);
    assert(id < Count());
    return _arr[id].ArgType(aid);
  }
  int Type(int id) const {
    assert(id >= 0);
    assert(id < Count());
    return _arr[id].Type();
  }
};

extern PrototypeArray LPrototypes;

#else
#error File already included
#endif
