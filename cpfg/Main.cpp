/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#include "warningset.h"

#pragma warning(disable : 4189)

#include <assert.h>
#include <stdio.h>
#include <windows.h>

#include "drawparam.h"
#include "wininterface.h"
#include "app.h"
#include "../Lstudio/cmndefs.h"

App app;

#define ID_FROMCPFG_ERROREXIT 40077

extern "C" HWND hLProject;

extern "C" void MyExit(int status);
extern "C" FILE *fpLog;

extern "C" int expired;

int PASCAL WinMain(HINSTANCE hInst, HINSTANCE, LPSTR cmd, int) {
  if (PrjVar::IsEvalVer()) {
    SYSTEMTIME stToday, stExpire;
    {
      stExpire.wYear = PrjVar::EvalYear;
      stExpire.wMonth = PrjVar::EvalMnth;
      stExpire.wDayOfWeek = 1;
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
        expired = 1;
      }
    }
  }

  try {
    app.Init(hInst);

    app.Run(cmd);
  } catch (const char *msg) {
    MessageBox(NULL, msg, "Error", MB_ICONHAND);
  } catch (...) {
    if (hLProject != NULL) {
      PostMessage(hLProject, WM_COMMAND,
                  MAKEWPARAM((UINT)(ID_FROMCPFG_ERROREXIT), (UINT)(0)), 0);
      hLProject = NULL;
    } else
      MessageBox(NULL, "Unknown error!", "Sorry :(", MB_ICONHAND);
    if (NULL != fpLog) {
      fclose(fpLog);
      fpLog = NULL;
    }
  }

  MyExit(0);

  return 0;
}
