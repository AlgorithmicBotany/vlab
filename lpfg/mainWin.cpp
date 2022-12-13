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



#include <cassert>
#include <cmath>
#include <stack>
#include <iostream>
#include <vector>

#ifdef _WINDOWS
#include <io.h>
#include <fcntl.h>
#endif

#ifdef _WINDOWS
#include <GL/glew.h>
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>

#ifdef _WINDOWS
//#include <warningset.h>
#endif

#include "vector3d.h"

#include "include/lparams.h"
#include "include/lintrfc.h"

#include "asrt.h"
#include "lpfgparams.h"
#include "maxpath.h"
#include "rect.h"

#include "lstring.h"
#include "glutils.h"
#include "gencyldata.h"
#include "turtle.h"
#include "pipepair.h"
#include "mesh.h"
#include "dynlib.h"
#include "lsysdll.h"
#include "lengine.h"
#include "configfile.h"
#include "drawparam.h"
#include "projection.h"
#include "lpfg.h"
#include "comlineparam.h"
#include "utils.h"
#include "resource.h"
#include "view.h"
#include "../Lstudio/cmndefs.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int main(int, char **argv) {
  return WinMain(GetModuleHandle(NULL), NULL, argv[0], SW_SHOWDEFAULT);
}

class OptionalConsole {
public:
  OptionalConsole(bool hasconsole) : _has(hasconsole) {
    if (_has) {
      AllocConsole();
      int hCrt =
          _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
      FILE *hf = _fdopen(hCrt, "w");
      *stdout = *hf;
      setvbuf(stdout, NULL, _IONBF, 0);
      hCrt = _open_osfhandle((long)GetStdHandle(STD_ERROR_HANDLE), _O_TEXT);
      hf = _fdopen(hCrt, "w");
      *stderr = *hf;
      setvbuf(stderr, NULL, _IONBF, 0);
    }
  }
  ~OptionalConsole() {
    if (_has)
      FreeConsole();
  }

private:
  const bool _has;
};

LPFG *pLpfg = NULL;

BOOL WINAPI ConsoleClosing(DWORD type) {
  switch (type) {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
  case CTRL_CLOSE_EVENT:
    if (NULL != pLpfg) {
      Utils::Mark("Console Closing");
      pLpfg->ConsoleClosed();
      Utils::Quitting();
    }
    break;
  }
  return FALSE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
  if (PrjVar::IsEvalVer()) {
    SYSTEMTIME stToday, stExpire;
    {
      stExpire.wYear = PrjVar::EvalYear;
      stExpire.wMonth = PrjVar::EvalMnth;
      stExpire.wDayOfWeek = 2;
      stExpire.wDay = PrjVar::EvalDay;
      stExpire.wHour = 0;
      stExpire.wMinute = 0;
      stExpire.wSecond = 0;
      stExpire.wMilliseconds = 0;
    }
    GetLocalTime(&stToday);

    {
      FILETIME ftToday, ftExpire;
      SystemTimeToFileTime(&stToday, &ftToday);
      SystemTimeToFileTime(&stExpire, &ftExpire);
      if (1 == CompareFileTime(&ftToday, &ftExpire)) {
        comlineparam.SetExpired();
      }
    }
  }

  LPFG lpfg(hInst);
  pLpfg = &lpfg;

  try {
    comlineparam.Parse(__argc, __argv);
    OptionalConsole console(!comlineparam.BatchMode() &&
                            comlineparam.HasConsole());
    if (PrjVar::IsEvalVer())
      Utils::Log("Demo version\n");
    Utils::SetEnv(comlineparam.Path());
    if (comlineparam.HasConsole())
      SetConsoleCtrlHandler(ConsoleClosing, TRUE);
    comlineparam.Apply();
    if (comlineparam.BatchMode())
      return lpfg.RunBatchMode();
    else if (comlineparam.CompileOnly())
      return lpfg.CompileOnly();

    lpfg.Register();
    View::Register(hInst);

    lpfg.Create();

    return lpfg.Loop();
  } catch (...) {
    lpfg.PanicExit();
    Utils::Quitting();
    return 0;
  }
}
