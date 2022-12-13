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



#ifndef __PRODUCTION_H__
#define __PRODUCTION_H__

#include "contextdata.h"
#include "module.h"

enum ProductionType { eProduction, eDecomposition, eInterpretation };

class ProductionProto {
public:
  ProductionProto(ContextData *, FormalModuleDtList *, ContextData *);
  ~ProductionProto();
  void Generate() const;
  void GenerateCaller() const;
  void GenerateProtoEntry() const;
  static void GenerateEmptyProtoEntry(FILE *);
  const FormalModuleDtList *GetStrct() const { return _pStrct; }
  bool HasNewContext() const {
    return _LCntxt.HasNewContext() || _RCntxt.HasNewContext();
  }
  bool IsContextFree() const;
  int Id() const { return _id; }
  int Group() const { return _group; }
  int VGroup() const { return _vgroup; }
  static int Compare(const void *, const void *);
  static int MaxVGrp;
  ProductionType GetType() const { return _type; }
  void SetInArray(int iInArray) { _iInArray = iInArray; }
  int GetInArray() const { return _iInArray; }

private:
  ContextData _LCntxt;
  FormalModuleDtList *_pStrct;
  ContextData _RCntxt;
  const int _id;
  const int _group;
  const int _vgroup;
  const int _considerGroup;
  ProductionType _type;
  int _iInArray;
  static int _counter;
};

typedef ProductionProto *pProductionProto;

class ProductionTable {
public:
  ProductionTable();
  virtual ~ProductionTable();
  virtual void Add(ProductionProto *);
  int Items() const { return _items; }
  const ProductionProto *GetProto(int i) const {
    assert(i >= 0);
    assert(i < _items);
    return _arr[i];
  }

protected:
  void _Grow();
  enum { eInitSize = 64 };
  pProductionProto *_arr;
  int _items;
  int _size;
};

class HomomorphismTable : public ProductionTable {
public:
  void Add(ProductionProto *);
  int Find(int mid, int grp, int vgrp) const;
};

class ProductionTableIterator {
public:
  ProductionTableIterator(const ProductionTable &tbl) : _tbl(tbl) {
    _current = 0;
  }
  void operator++() {
    assert(!AtEnd());
    _current++;
  }
  bool AtEnd() const {
    assert(_current <= _tbl.Items());
    return _tbl.Items() == _current;
  }
  const ProductionProto *GetCurrent() const {
    assert(!AtEnd());
    return _tbl.GetProto(_current);
  }
  ProductionProto *AccessCurrent() {
    assert(!AtEnd());
    return const_cast<ProductionProto *>(_tbl.GetProto(_current));
  }
  int CurrentPosition() const { return _current; }
  void Reset() { _current = 0; }

private:
  const ProductionTable &_tbl;
  int _current;
};

#endif
