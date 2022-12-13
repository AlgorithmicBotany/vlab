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

#include <string>
#include <cstdio>

class ReadTextFile {
public:
  ReadTextFile(const char *);
  ReadTextFile(const char *, const bool);
  ~ReadTextFile() {
    if (_fp != NULL)
      fclose(_fp);
  }
  void ReadLine(char *, int);
  bool Eof() const { return 0 != feof(_fp); }
  int Line() const { return _line; }
  const char *Filename() const { return _filename.c_str(); }
  FILE *Fp() const { return _fp; }
  bool good(){return _good;}

private:
  void ProcessCPP(char *);
  int waitOpenFile(const char *fname);
  std::string _filename;
  FILE *_fp;
  int _line;
  bool _good;

#ifdef _WINDOWS
  bool _readSuccess;
#endif
};

class WriteTextFile {
public:
  WriteTextFile(const char *filename);
  ~WriteTextFile();
  FILE *Fp() const { return _fp; }

private:
  FILE *_fp;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
