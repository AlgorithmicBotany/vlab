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



/*
 * $RCSfile: utility.h,v $
 * $Revision: 1.2 $
 * $Date: 2005/08/30 20:32:02 $
 * $Author: radekk $
 * $Locker:  $
 *
 * $Log: utility.h,v $
 * Revision 1.2  2005/08/30 20:32:02  radekk
 * Merge with Windows version
 *
 * Revision 1.2  2001/08/31 23:21:20  federl
 * More changes (bugfixes) performed by Samantha Filkas.
 *
 * Revision 1.3  2001/07/09 19:59:16  filkas
 * Hoping it's right now.
 *
 * Revision 1.1.1.1  2001/06/12 20:14:02  filkas
 * Imported using TkCVS
 *
 * Revision 1.1.1.1.2.1  1993/11/13  00:02:24  jamesm
 * Experimental Environmental interaction version
 *
 * Revision 1.1  1993/10/11  00:15:56  jamesm
 * Initial revision
 *
 *
 */

#ifndef _CPFG_UTILITY_
#define _CPFG_UTILITY_

#ifdef __cplusplus
extern "C" {
#endif

#define VERBOSE                                                                \
  if (clp.verbose)                                                             \
  Message
#define TRACE VERBOSE("%s, %d\n", __FILE__, __LINE__)

FILE *PreprocessFile(const char *name, const char *options);

double nrand(double mean, double variance);
double brand(double beta_a, double beta_b);
double binrand(int n, double p);

char *MakeLabel(const char *label, int parameters, const float *values);
void Do_srand(long int);
double Do_ran(double v);
double Do_nrand(double, double);
double Do_bran(double, double);
double Do_biran(int, double);

float DotProduct(const float[3] /* vec1*/, const float[3] /* vec2 */);
double DDotProduct(const double[3] /* vec1 */, const double[3] /* vec2 */);
double DFDotProduct(const double[3] /* vec1 */, const float[3] /* vec2 */);
double DDistance(const double[3] /* vec1 */, const double[3] /* vec2 */);
void CrossProduct(const float[3] /* v1 */, const float[3] /* v2 */,
                  float[3] /* res */);
void DCrossProduct(const double[3] /* v1 */, const double[3] /* v2 */,
                   double[3] /* res */);
void Normalize(float[3] /* v1 */);
void DNormalize(double[3] /* v1 */);
void Matprt4x4(float /*mat1*/[4][4]);
void DPrint3(const double /*array*/[3]);
void MatMult4x4(const float /*mat1*/[4][4], const float /*mat2*/[4][4],
                float /*mat3*/[4][4]);
void MatMult4x4D(const double /*mat1*/[4][4], const double /*mat2*/[4][4],
                 double /*mat3*/[4][4]);
void VecMatMult(const float vec1[3], const float mat1[4][4], float vec2[3]);
void Vec4Mat4Mult(const float vec1[4], const float mat1[4][4], float vec2[4]);
void PolarTo3d(float theta, float phi, float radius, float p[3]);
void Mat4Vec4Mult(float mat1[4][4], float vec1[4], float vec2[4]);
void IdentityMat4(float mat[4][4]);
void TransposeMat4(float mat[4][4]);
void Inverse(float mat[3][3], float inv[3][3]);
void Transpose(float mat[3][3]);
float Determinant(float mat[3][3]);
void Adjoint(float mat[3][3], float adj[3][3]);
const char *ReadQuotedString(const char *, char *, int);

void changeExtension(char *name, char *ext);
void stripDirectory(char *name);

#ifdef __cplusplus
}
#endif

#endif /* _CPFG_UTILITY_ */
