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

class File
{
protected:
  File(const char* fname, const char* mode);
public:
  ~File();
  bool Valid() const
  { return NULL != _fp; }
  operator FILE*()
  { return _fp; }
  FILE* fp()
  { return _fp; }
  std::string getname()
  { return name;}
protected:
  FILE* _fp;
  std::string name;
};


class ReadTextFile : public File
{
public:
  ReadTextFile(const char* fname) : File(fname, "rt")
  {}
};


class WriteTextFile : public File
{
public:
  WriteTextFile(const char* fname) : File(fname, "wt")
  {}
  void PrintF(const char*, ...);
};


#else
  #error File already included
#endif

