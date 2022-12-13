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



#ifndef __LFUNCSC_H__
#define __LFUNCSC_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
const int LFuncBaseId = 100000;
*/
#define LFuncBaseId 100000

void ClearLFuncs(void);
void PreprocessForL(const char *, char *);
int GenerateLCode(const char *, char *);
void ReadLFuncs(char *);

/*
   The return value is LFuncBaseId + address of the function in LPU code.
   if the function is not defined it is 0
*/
int FindLFunc(const char *);

/*
   The argument to this function is the id of the function
   in LPU code. Which is: FindLFunc(name)-LFuncBaseId
*/
int NumOfParamsForLFunc(int);

void CallLFunction(int /* address of the function in the LPU code*/,
                   double * /* the stack */, int * /* current stack offset */
);

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
