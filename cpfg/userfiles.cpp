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



#ifdef WIN32
#include "warningset.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "userfiles.h"
#include "log.h"

#define MAXFILES 25
#define MAXFNAME 128

static FILE *fps[MAXFILES] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL};

static char fnames[MAXFILES][MAXFNAME] = {"", "", "", "", "", "", "", "", "",
                                          "", "", "", "", "", "", "", "", "",
                                          "", "", "", "", "", "", ""};

static int fix[MAXFILES] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
                            13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};

static int firstfreefix = 24;

static int GetFreeFpIndex(void) {
  if (-1 == firstfreefix)
    return -1;
  return fix[firstfreefix--];
}

static void ReleaseFpIndex(int i) {
  assert(firstfreefix < MAXFILES);
  assert(i >= 0);
  assert(i < MAXFILES);
  fix[++firstfreefix] = i;
}

void FreeFileDefSpace(void) {
  int i;
  for (i = 0; i < MAXFILES; ++i) {
    if (NULL != fps[i]) {
      Message("File %s was never closed\n", fnames[i]);
      fclose(fps[i]);
      fps[i] = NULL;
      fnames[i][0] = 0;
    }
    fix[i] = i;
  }
}

void FreeFps(void) { FreeFileDefSpace(); }

int Getnewfp(const char *fname, const char *mode) {
  int res;
  res = GetFreeFpIndex();
  if (-1 == res) {
    Message("Out of free file handles\n");
    return -1;
  }
  fps[res] = fopen(fname, mode);
  if (NULL == fps[res]) {
    ReleaseFpIndex(res);
    return -1;
  }
  strncpy(fnames[res], fname, MAXFNAME);
  fnames[res][MAXFNAME - 1] = 0;
  return res;
}

int IsValidFpIndex(int i) {
  if (i < 0)
    return 0;
  if (i >= MAXFILES)
    return 0;
  return 1;
}

FILE *Getfp(int i) {
  assert(IsValidFpIndex(i));
  return (fps[i]);
}

const char *GetFname(int i) {
  assert(IsValidFpIndex(i));
  return fnames[i];
}

int Fclosefp(int i) {
  assert(IsValidFpIndex(i));
  if (NULL == fps[i])
    return 0;
  fclose(fps[i]);
  fps[i] = NULL;
  ReleaseFpIndex(i);
  return 1;
}
