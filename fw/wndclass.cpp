/**************************************************************************

  File:		wndclass.cpp
  Created:	07-Jan-98


  Implementation of classes WndClass


**************************************************************************/


#include <windows.h>

#include "warningset.h"

#include "wndclass.h"

WndClass::WndClass(HINSTANCE hInst, const TCHAR* classname, WNDPROC WndProc)
{
	style = CS_HREDRAW | CS_VREDRAW;
	lpfnWndProc = WndProc;
	cbClsExtra = 0;
	cbWndExtra = 0;
	hInstance = hInst;
	hIcon = 0;
	hCursor = LoadCursor(0, IDC_ARROW);
	hbrBackground = 0;
	lpszMenuName = 0;
	lpszClassName = classname;
}
