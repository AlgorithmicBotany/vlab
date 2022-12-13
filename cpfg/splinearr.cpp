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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef LINUX
#include <limits.h>
#endif

#include "dynarray.h"
#include "maxpth.h"

#include "splinefun.h"
#include "splinearr.h"
#include "file.h"
#include "log.h"

#ifdef LINUX
#include <iostream>
#endif
#include "comlineparam.h"

extern COMLINEPARAM clp;

bool SplineFunctionArray::Read(const char *fname, int samples) {
  SplineFunction New(fname, samples);
  Add(New);
  return true;
}

void SplineFunctionArray::Reset() {
  _items = 0;
  _galleryname[0] = 0;
}

bool SplineFunctionArray::Reread() {
  if (0 == _galleryname[0]) {
    for (int i = 0; i < _items; i++) {
      if (!_arr[i].Reread())
        return false;
    }
  } else if (!_RereadGallery())
    return false;
  return true;
}

bool SplineFunctionArray::ReadGallery(const char *fname) {
  strcpy(_galleryname, fname);
  ReadTextFile src(fname);
  if (!src.Valid()) {
    Message("Cannot open %s file for reading\n", fname);
    return false;
  }
  int vmaj, vmin;
  int res = fscanf(src, "funcgalleryver %d %d\n", &vmaj, &vmin);
  if (2 != res) {
    Message("Error reading function set file %s\n", fname);
    return false;
  }
  int ver = 100 * vmaj + vmin;
  switch (ver) {
  case 101:
    return _Read0101(src);
    break;
  default:
    Message("Cannot read function set version %d.%02d from file %s\n", vmaj,
            vmin, fname);
    return false;
  }
  return true;
}
// [Pascal] keep opening the file until the size is stable
int SplineFunctionArray::waitOpenFile(const char *fname) {
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  FILE *fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((fp == NULL) && (counter < 10000)) {
    fp = fopen(fname, "r");
    counter++;
  }
  if (counter == 10000) {
    fprintf(stderr, "WARNING: Can't open file %s.\n", fname);
    return 0;
  } else {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp);       // get current file pointer
    counter = 0;
    while (((size == 0) || (current_size != size)) && (counter < 10000)) {
      current_size = size;
      fclose(fp);

      fp = NULL;
      while (fp == NULL) {
        fp = fopen(fname, "r");
        counter++;
      }
      fseek(fp, 0, SEEK_END); // seek to end of file
      size = ftell(fp);       // get current file pointer
    }
  }
  fseek(fp, 0L, SEEK_SET);
  fclose(fp);
  if (counter >= 1000) {
    Message("we can't reload the file : %d\n",counter);
    return 0;
  }

  return 1;
}



bool SplineFunctionArray::_RereadGallery() {
  if (!waitOpenFile(_galleryname))
    return false;
 
  ReadTextFile src(_galleryname);
  if (!src.Valid()) {
    Message("Cannot open %s file for reading\n", _galleryname);
    return false;
  }
  int vmaj, vmin;
  int res = fscanf(src, "funcgalleryver %d %d\n", &vmaj, &vmin);
  if (2 != res) {
    Message("Error reading function set file %s\n", _galleryname);
    return false;
  }
  int ver = 100 * vmaj + vmin;
  switch (ver) {
  case 101:
    return _Reread0101(src);
    break;
  default:
    Message("Cannot read function set version %d.%02d from file %s\n", vmaj,
            vmin, _galleryname);
    return false;
  }
  return true;
}

bool SplineFunctionArray::_Read0101(ReadTextFile &src) {
  int items;
  int res = fscanf(src, "items: %d\n", &items);
  if (res != 1) {
    Message("Error reading function set file\n");
    return false;
  }
  for (int i = 0; i < items; i++) {
    SplineFunction New(src);
    Add(New);
  }
  return true;
}

bool SplineFunctionArray::_Reread0101(ReadTextFile &src) {
  int items;
  int res = fscanf(src, "items: %d\n", &items);
  if (res != 1) {
    Message("Error reading function set file\n");
    return false;
  }
  if (items != Count()) {
    _items = 0;
    for (int i = 0; i < items; i++) {
      SplineFunction New(src);
      Add(src);
    }
  } else {
    for (int i = 0; i < items; i++)
      if (!_arr[i].Reread(src))
        return false;
  }
  return true;
}

const char *SplineFunctionArray::GetName(int i) const {
  assert(i >= 0);
  assert(i < Count());
  return _arr[i].GetName();
}
