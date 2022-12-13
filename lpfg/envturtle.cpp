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



#include <iostream>
#include "turtle.h"
#include "lstriter.h"
#include "lstring.h"
#include "lpfgparams.h"
#include "environment.h"

#include "StdModulesStruct.h"

EnvironmentTurtle::EnvironmentTurtle(Lstring &,
                                     Environment *pEnv)
  :
      _pEnvironment(pEnv), _InterpretedE(false) {
}

void EnvironmentTurtle::Query(const LstringIterator &iter) const {
  const __lc_BasicParameterStruct *pBasicParams =
      reinterpret_cast<const __lc_BasicParameterStruct *>(iter.Ptr());

#ifdef NO_MEMCOPY
#define PARAMS(moduleName)                                                     \
  const __##moduleName##_ParamsStruct_ *pCParams =                             \
      reinterpret_cast<const __##moduleName##_ParamsStruct_ *>(pBasicParams);  \
  __##moduleName##_ParamsStruct_ *pParams =                                    \
      const_cast<__##moduleName##_ParamsStruct_ *>(pCParams);                  \
  __##moduleName##_ParamsStruct_::Data &params = pParams->data;
#else
#define PARAMS(moduleName)                                                     \
  const __##moduleName##_ParamsStruct_ *pCParams =                             \
      reinterpret_cast<const __##moduleName##_ParamsStruct_ *>(pBasicParams);  \
  __##moduleName##_ParamsStruct_ Params;                                       \
  memcpy(&Params, pCParams, sizeof(__##moduleName##_ParamsStruct_));           \
  __##moduleName##_ParamsStruct_::Data &params = Params.data;
#endif

  switch (iter.GetModuleId()) {
  case GetPos_id: {
    PARAMS(GetPos);
    params.Param0 = _position.X();
    params.Param1 = _position.Y();
    params.Param2 = _position.Z();
#ifndef NO_MEMCOPY
    memcpy(const_cast<__lc_BasicParameterStruct *>(pBasicParams), &Params,
           sizeof(Params));
#endif
  } break;
  case GetHead_id: {
    PARAMS(GetHead);

    params.Param0 = _heading.X();
    params.Param1 = _heading.Y();
    params.Param2 = _heading.Z();
#ifndef NO_MEMCOPY
    memcpy(const_cast<__lc_BasicParameterStruct *>(pBasicParams), &Params,
           sizeof(Params));
#endif
  } break;
  case GetLeft_id: {
    PARAMS(GetLeft);
    params.Param0 = _left.X();
    params.Param1 = _left.Y();
    params.Param2 = _left.Z();
#ifndef NO_MEMCOPY
    memcpy(const_cast<__lc_BasicParameterStruct *>(pBasicParams), &Params,
           sizeof(Params));
#endif
  } break;
  case GetUp_id: {
    PARAMS(GetUp);
    params.Param0 = _up.X();
    params.Param1 = _up.Y();
    params.Param2 = _up.Z();
#ifndef NO_MEMCOPY
    memcpy(const_cast<__lc_BasicParameterStruct *>(pBasicParams), &Params,
           sizeof(Params));
#endif
  } break;
  }
}

// from mesh_unused.h/.cpp
//void EnvironmentTurtle::MeshPoint() {
//}

void EnvironmentTurtle::E1(float v, const LstringIterator &iter) {
  _InterpretedE = true;
  if (_pEnvironment != 0)
    _pEnvironment->SendData(v, iter, this);
}

void EnvironmentTurtle::E2(float v1, float v2, const LstringIterator &iter) {
  _InterpretedE = true;
  if (_pEnvironment != 0)
    _pEnvironment->SendData(v1, v2, iter, this);
}

void EnvironmentTurtle::EA(int numParams, const float *vs,
                           const LstringIterator &iter) {
  _InterpretedE = true;
  if (_pEnvironment != 0)
    _pEnvironment->SendData(numParams, vs, iter, this);
}
