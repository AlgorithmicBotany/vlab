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



#ifndef __CONTEXTDATA_H__
#define __CONTEXTDATA_H__

class FormalModuleDtList;

class ContextData {
public:
  bool HasNewContext() const { return NULL != _pNewCntxt; }
  void SetContext(FormalModuleDtList *pCntxt) { _pCntxt = pCntxt; }
  void SetNewContext(FormalModuleDtList *pNewCntxt) { _pNewCntxt = pNewCntxt; }
  bool Contains(const char *ident) const;
  void GenerateForPrototype() const;
  int CountParams() const;
  int GenerateForCaller(FILE *, int, char) const;
  int GenerateForProductionCall(FILE *fileOut, int parameterIndex,
                                char prefix) const;
  void GenerateProtoEntry(FILE *) const;
  void Clear();
  void Copy(const ContextData *);
  bool IsClear() const;

private:
  FormalModuleDtList *_pCntxt;
  FormalModuleDtList *_pNewCntxt;
};

#endif
