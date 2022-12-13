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




#ifndef __SET_H__
#define __SET_H__

#include <string>
#include <list>
#include "delete_recursive.h"

class Gallery;

class Set {
 public:
  Set(Gallery* pgal, std::string fileaname);
  virtual ~Set();

  virtual void load()=0;
  virtual void saveCurrentSet() {}
  virtual void cleanup()=0;

  std::string getFileName() {return filename;}
  void setFileName(std::string name) {filename = name;}

  class FileReadExc {
  public:
    FileReadExc(std::string msg) : msg(msg) {}
    std::string getMessage() {return msg;}
  private:
    std::string msg;
  };
  class FileWriteExc {
  public:
    FileWriteExc(std::string msg) : msg(msg) {}
    std::string getMessage() {return msg;}
  private:
    std::string msg;
  };

 protected:
  Gallery* pGal;
  std::string filename;
  std::list<std::string> filenames;
};

#endif
