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



#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "include/lparams.h"
#include "include/lintrfc.h"

class SuccessorStorage;
class LPFG;

namespace Interface {
float Func(int, float);
float VFunc(int);
float TFunc(int, float);
float PFunc(int, float, float, float);
__lc_V2f FuncTangent(int, float);
float CurveX(int, float);
float CurveY(int, float);
float CurveZ(int, float);
void CurveReset(int);
void CurveSetPoint(int, int, float, float, float);
void CurveRecalc(int);
void CurveScale(int, float, float, float);
__lc_V2f CurveV2fPoint(int, float);
__lc_V3f CurveV3fPoint(int, float);
__lc_V3f CurveV3fNormal(int, float);
float CurveGAL(int);
SuccessorStorage &GetSuccessorStorage();
SurfaceObj GetSurface(int);
__lc_V3f GetSurfacePoint(int, int);
void RunCmnd(const char *);
void UseView(int);
void CloseView(int);
void DisplayFrame();
void OutputFrame(const char*);
float vvXmin(int vid);
float vvXmax(int vid);
float vvYmin(int vid);
float vvYmax(int vid);
float vvZmin(int vid);
float vvZmax(int vid);
float vvScale(int vid);
void Stop();
int StepNo();
double GillespieTime();
void SeedGillespie(long);
void ResetGillespie();
double Ran(double);
void SeedRan(long);
void LoadString(const char *);
void OutputString(const char *);
MouseStatus GetMouseStatus(void);
TabletStatus GetTabletStatus(void);
void UserMenuItem(const char *, unsigned int);
void UserMenuClear(void);
int UserMenuChoice(void);
void RunSimulation(void);
void PauseSimulation(void);
CameraPosition GetCameraPosition(int vid);

BsurfaceObjS GetBsurfaceS(int);
BsurfaceObjM GetBsurfaceM(int);

bool terrainHeightAt(V3f, V3f &);
bool terrainNormalAt(V3f, V3f &);
void terrainVisibilityAll(VisibilityMode);
void terrainVisibilityPatch(VisibilityMode, int, V3f);
void scaleTerrainBy(float);

float SetOrGetParameterf(const char *, float);
int SetOrGetParameteri(const char *, int);
bool ParametersNeedUpdating();
float GetParameterf(const char *);
int GetParameteri(const char *);
void SetParameterf(const char *, float);
void SetParameteri(const char *, int);
void DelayWrite();
void Write();

bool LContextReset();
bool RContextReset();
bool LNContextReset();
bool RNContextReset();
bool AcceptContext(__lc_ModuleIdType moduleId);
bool AdvanceContext();
bool ConfirmContext();

const char *GetModuleAddr(__lc_ModuleIdType);

char *GetNextModuleSpot(int iSize);
void StartPerformance();
void StopPerformance();

extern LPFG *pLpfg;

} // namespace Interface

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
