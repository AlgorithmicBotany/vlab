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



#ifndef __FILE_H__
#define __FILE_H__

#ifdef LINUX
#include <QValidator>
#endif // LINUX

#include "msg.h"

class File {
protected:
  File(const char *fname, const char *mode) {
    _fp = fopen(fname, mode);
  }

public:
  ~File();
  bool Valid() const {
			   return NULL != _fp; }

  // [Pascal] keep opening the file until the size is stable
  int waitOpenFile(const char *fname) {
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


  
  operator FILE *() const { return _fp; }
  FILE *fp() const { return _fp; }

protected:
  FILE *_fp;
  
};

class ReadTextFile : public File {
public:
  ReadTextFile(const char *fname) : File(fname, "rt") {
    if (!waitOpenFile(fname)) _fp =  NULL;
  }
};

class WriteTextFile : public File {
public:
  WriteTextFile(const char *fname) : File(fname, "wt") {}
  void PrintF(const char *, ...);
};

#else
#error File already included
#endif
