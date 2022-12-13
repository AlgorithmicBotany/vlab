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



#ifndef __DYNLIB_H__
#define __DYNLIB_H__

#include <string>

#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/types.h>
#endif

class DynLibrary {
public:
  DynLibrary();
  virtual ~DynLibrary();
  void Connect(const std::string &);
  void Disconnect();
  bool Connected() const;
#ifdef _WINDOWS
  typedef FARPROC Proc;
#else
  typedef void (*Proc)();
#endif
  Proc GetProc(const std::string &) const;

protected:
  virtual bool _Map() { return true; }
  virtual void _Unmap() {}

private:
#ifdef _WINDOWS
  HINSTANCE _hDll;
#else
  void *_handle;
  int _tmpId;
  pid_t _pid;
  std::string _tmpName;
#endif
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
