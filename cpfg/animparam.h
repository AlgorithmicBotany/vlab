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



#ifndef __ANIMPARAM_H__
#define __ANIMPARAM_H__

#ifdef __cplusplus
extern "C" {
#endif

struct ANIMINTERVAL {
  int from;
  int to;
  int step;
  unsigned char function; /* so far: 0 nothing, 1 rotate */
  float data[4];          /* function params */
};

#define MAXANIMINTERVALS 1000

struct ANIMPARAM {
  int first;
  int last;
  int swap_interval;
  unsigned int flags;
  int step;
  struct ANIMINTERVAL intervals[MAXANIMINTERVALS];
};

typedef struct ANIMPARAM ANIMPARAM;

void InitializeAnimparam(ANIMPARAM *);

int DoubleBuffer(const ANIMPARAM *);
void SetDoubleBuffer(ANIMPARAM *, int);
int NewViewBetweenFrames(const ANIMPARAM *);
void SetNewViewBetweenFrames(ANIMPARAM *, int);
int ScaleBetweenFrames(const ANIMPARAM *);
void SetScaleBetweenFrames(ANIMPARAM *, int);
int ClearBetweenFrames(const ANIMPARAM *);
void SetClearBetweenFrames(ANIMPARAM *, int);
int CenterBetweenFrames(const ANIMPARAM *);
void SetCenterBetweenFrames(ANIMPARAM *, int);
int HCenterBetweenFrames(const ANIMPARAM *);
void SetHCenterBetweenFrames(ANIMPARAM *, int);

void ReadAnimData(const char *, ANIMPARAM *);

#ifdef __cplusplus
}
#endif

#else
#error File already included
#endif
