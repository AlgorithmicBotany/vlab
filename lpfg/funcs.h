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



#ifndef __FUNCS_H__
#define __FUNCS_H__

#include "function.h"
#include "file.h"

class ReadTextFile;

class Functions : private std::vector<Function> {
public:
  Functions();
  ~Functions();
  void reInitialize(const Functions &funs){
    if (funs.Size() != size())
      return;

    clear();
    for (unsigned int i = 0; i < funs.size(); ++i)
      push_back(funs[i]);
  }

    
  void Clear() { clear(); }
  bool ValidId(size_t id) const { return id < size(); }
  void SetLowLimit(size_t id, float lowLimit) {
    operator[](id).SetLowLimit(lowLimit);
  }
  void SetUpLimit(size_t id, float upLimit) {
    operator[](id).SetUpLimit(upLimit);
  }
  size_t Size() const { return size();}

  float LowLimit(size_t id) const { return operator[](id).LowLimit(); }
  float UpLimit(size_t id) const { return operator[](id).UpLimit(); }
  float GetValue(size_t id, float v) const {
    ASSERT(id < size());
    ASSERT(v >= 0.0f);
    ASSERT(v <= 1.0f);
    return operator[](id).GetValue(v);
  }

  float GetDefaultValue(size_t id) const {
    return operator[](id).GetDefaultValue();
  }
    
  Vector3d GetTangent(size_t id, float v) const {
    ASSERT(id < size());
    ASSERT(v >= 0.0f);
    ASSERT(v <= 1.0f);
    return operator[](id).GetTangent(v);
  }
  const char *GetName(size_t id) const {
    ASSERT(id < size());
    return operator[](id).GetName();
  }
  bool Load(const char *);
  bool LoadIndividualFunctions(const std::vector<std::string>);

  size_t Count() { return size(); }

private:
  bool _Read0101(ReadTextFile &);
  bool _Read0102(ReadTextFile &);
  bool _Read0103(const char*);

};

extern Functions functions;

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
