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



#ifndef __PSOUT_H__
#define __PSOUT_H__

#include <vector>
#include <fstream>
#include <string>
#include <iostream>

/// The PsOutputStore class is meant to be used with Postscript output.
///   Data is written to the store by creating a new node with AddNew(),
///   writing the node with the << operator, and then setting the visual depth
///   of this new information. When all of the information is written, the
///   store then needs to sort the information by its visual depth, so that
///   farther objects draw first, and closer objects then draw over top.
class PsOutputStore {
public:
  PsOutputStore();
  void PrintToStream(std::ostream &trg);
  PsOutputStore &operator<<(const std::string);
  PsOutputStore &operator<<(const char *);
  PsOutputStore &operator<<(const char);
  PsOutputStore &operator<<(const float);
  PsOutputStore &operator<<(const int);
  void SetDepth(int);
  void AddNew();
  void Reset();
  void Sort();

private:
  class Data {
  public:
    std::string command;
    float depth;
    Data() {
      command = "";
      depth = 0;
    }
    ~Data() {}
  };
  struct DataComparator {
    bool operator()(const Data *a, const Data *b) const {
      return a->depth < b->depth;
    }
  };
  std::vector<Data *> data;
};

#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
