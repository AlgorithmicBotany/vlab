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



#include <sstream>

#include "file.h"
#include "exception.h"
#include "utils.h"
#include <string.h>

ReadTextFile::ReadTextFile(const char *fnm) : _filename(fnm) {
  _good = true;
  _fp = fopen(fnm, "rt");
  if (0 == _fp) {
    // the file doesn't exist
    printf("WARNING: Cannot open file %s for reading\n", fnm);
    _good = false;
    return;
  }
  _line = 0;
}

ReadTextFile::ReadTextFile(const char *fnm, const bool wait) : _filename(fnm) {
  _good = true;

  if (!wait) {
    _fp = fopen(fnm, "rt");
    if (0 == _fp) {
      _good = false;
      return;
      throw Exception("Cannot open file %s for reading", fnm);
    }
    _line = 0;
  } else {
    bool success = waitOpenFile(fnm);
    if (!success){
      _good = false;
      return;
    }

    _line = 0;
  }
}

void ReadTextFile::ReadLine(char *ln, int length) {
  ++_line;
  const char *res = fgets(ln, length, _fp);
  if (0 == res) {
    if (!feof(_fp)) {
      printf("Error reading file %s at line %d\n", Filename(), Line());
      return;
      throw Exception("Error reading file %s at line %d\n", Filename(), Line());
    } else
      ln[0] = 0;
  } else {
    size_t l = strlen(ln);
    if (ln[l - 1] == '\n')
      ln[l - 1] = 0;
  }
  if ('#' == ln[0])
    ProcessCPP(ln);
}

void ReadTextFile::ProcessCPP(char *ln) {
  const char *scs = Utils::SkipNonBlanks(ln);
  scs = Utils::SkipBlanks(scs);
  _line = atoi(scs);
  scs = Utils::SkipNonBlanks(scs);
  scs = Utils::SkipBlanks(scs);
  _filename = scs;
  --_line;
  ln[0] = 0;
}

// [Pascal] keep opening the file until the size is stable
int ReadTextFile::waitOpenFile(const char *fname) {
#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  _fp = fopen(fname, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((_fp == NULL) && (counter < 10000)) {
    _fp = fopen(fname, "r");
    counter++;
  }
  if (counter == 10000) {
    fprintf(stderr, "WARNING: Can't open file %s - using defaults.\n", fname);
    return 0;
  } else {
    fseek(_fp, 0, SEEK_END); // seek to end of file
    size = ftell(_fp);       // get current file pointer

    while ((size == 0) || (current_size != size)) {
      current_size = size;
      fclose(_fp);

      _fp = NULL;
      while (_fp == NULL) {
        _fp = fopen(fname, "r");
        counter++;
      }
      fseek(_fp, 0, SEEK_END); // seek to end of file
      size = ftell(_fp);       // get current file pointer
    }
  }
  fseek(_fp, 0L, SEEK_SET);
  if (counter >= 1000)
    Utils::Message("we can't reload the file : %d",counter);

#else
  if ((_fp = fopen(fname, "rt")) == NULL) {
    printf("Unable to open file %s \n", fname);
    return 0;
  }

#endif
  return 1;
}

WriteTextFile::WriteTextFile(const char *filename) {
  _fp = fopen(filename, "wt");
}

WriteTextFile::~WriteTextFile() {
  if (_fp != NULL)
    fclose(_fp);
}
