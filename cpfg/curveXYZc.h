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



#ifndef __CURVEXYZC_H__
#define __CURVEXYZC_H__

#ifdef __cplusplus
extern "C" {
#endif

int CurveXYZCount();
const char *CurveXYZName(int i);

void ReadCurveXYZ(const char *filename, int samples);
int RereadCurveXYZ();
void FreeCurveXYZ(void);

double CurveCXT(int id, int i, double t, double s, double r);
double CurveCYT(int id, int i, double t, double s, double r);
double CurveCZT(int id, int i, double t, double s, double r);
double CurveCXS(int id, int i, double t, double s, double r);
double CurveCYS(int id, int i, double t, double s, double r);
double CurveCZS(int id, int i, double t, double s, double r);
double CurveCXX(int id, int i, double t, double s, double r);
double CurveCYX(int id, int i, double t, double s, double r);
double CurveCZX(int id, int i, double t, double s, double r);
int CurveGCN(int id);
double CurveGCX(int id, int num);
double CurveGCY(int id, int num);
double CurveGCZ(int id, int num);
void CurveSCX(int id, int num, double x);
void CurveSCY(int id, int num, double y);
void CurveSCZ(int id, int num, double z);
double CurveGCM(int id, int num);
void CurveSCM(int id, int num, int m);
double CurveGCT(int id);
void CurveSCT(int id, int type);
void CurveACL(int id, int num, double x, double y, double z);
void CurveACR(int id, int num, double x, double y, double z);
void CurveDCP(int id, int num);
double CurveGCU(int id, int num);
double CurveGCV(int id, int num);
double CurveGCW(int id, int num);
void CurveSCU(int id, int num, double u);
void CurveSCV(int id, int num, double v);
void CurveSCW(int id, int num, double w);
void CurveCCV(int id);
void CurveCPP(int id);
int CurveGPN(int id);
double CurveGPX(int id, int num);
double CurveGPY(int id, int num);
double CurveGPZ(int id, int num);
void CurveSPX(int id, int num, double x);
void CurveSPY(int id, int num, double y);
void CurveSPZ(int id, int num, double z);
void CurveTPP(int id, double tx, double ty, double tz);
void CurveSPP(int id, double sx, double sy, double sz);
void CurveRPP(int id, double alpha, double vx, double vy, double vz);
void CurveRPF(int id, double t1x, double t1y, double t1z, double n1x,
              double n1y, double n1z, double t2x, double t2y, double t2z,
              double n2x, double n2y, double n2z);
void CurveCAP(int id, int sample);
double CurveGAL(int id);

#ifdef __cplusplus
}
#endif

#endif
