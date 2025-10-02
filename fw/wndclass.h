/**************************************************************************

  File:		wndclass.h
  Created:	07-Jan-98


  Declaration of classes WndClass


**************************************************************************/


#ifndef __WNDCLASS_H__
#define __WNDCLASS_H__


class WndClass : public WNDCLASS
{
public:
	WndClass(HINSTANCE, const TCHAR*, WNDPROC);
	void SetMenu(WORD id)
	{ lpszMenuName = MAKEINTRESOURCE(id); }
	void SetBackground(HBRUSH hBrush)
	{ hbrBackground = hBrush; }
	ATOM Register()
	{ return RegisterClass(this); }
	void DblClick()
	{ style |= CS_DBLCLKS; }
};


#endif
