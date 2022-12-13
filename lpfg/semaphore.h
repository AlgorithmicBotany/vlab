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



#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#ifdef WIN32
#include <windows.h>
#endif

class SemaphorePair {
public:
  SemaphorePair();
  ~SemaphorePair();
  void Create(int, bool, bool);
  void Release(int);
  void Wait(int);

private:
#ifdef WIN32
  class Semaphore {
  public:
    Semaphore();
    ~Semaphore();
    void Create(const std::string &, int, int);
    void Release();
    void Wait();

  private:
    HANDLE _hSemaphore;
  };
  Semaphore _sem0, _sem1;
#endif
  int _semid;
};

#else
#error File already included
#endif
