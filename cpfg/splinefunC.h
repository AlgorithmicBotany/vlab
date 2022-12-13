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



#ifndef __SPLINEFUNCTIONC_H__
#define __SPLINEFUNCTIONC_H__

#ifdef __cplusplus
extern "C" {
#endif

int ReadSplineFunction(const char *, int);
double SplineFuncRange(int, double);
void FreeSplineFunctions(void);
double SplineFuncValue(int, double);
int RereadFunctions(void);
int ReadFunctionSet(const char *);
int SplineFuncCount(void);
const char *SplineFuncName(int);
void SetFunctionsFromGallery(int);
int FunctionsFromGallery(void);
const char *GetGalleryName(void);

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
