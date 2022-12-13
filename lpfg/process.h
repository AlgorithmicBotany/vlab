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



#ifndef __PROCESS_H__
#define __PROCESS_H__

#ifdef WIN32
#include <windows.h>

#include <string>
#else
#include <sys/types.h>
#endif

class Process {
#ifdef WIN32

  class StartupInfo : private STARTUPINFO {
  public:
    StartupInfo();
    STARTUPINFO *Data() { return this; }
  };

  class ProcessMaker {
  public:
    ProcessMaker(const char *);
    HANDLE Create();

  private:
    std::string _cmndLn;
    SECURITY_ATTRIBUTES _ProcessSecurity;
    SECURITY_ATTRIBUTES _ThreadSecurity;
    BOOL _inherit;
    DWORD _flags;
    void *_pEnv;
    std::string _Directory;
    StartupInfo _si;
    PROCESS_INFORMATION _pi;
  };

#endif

public:
  Process();
  ~Process();

  void Start(const char *);

#ifdef WIN32
  void WaitAndClose(DWORD);
#else
  void WaitAndClose(int);
#endif
  bool IsRunning() const;

private:
#ifdef WIN32
  HANDLE _hProcess;
#else
  pid_t _pid;
#endif
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
