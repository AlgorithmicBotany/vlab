/**************************************************************************

  File:		trackbarctrl.h
  Created:	03-Feb-98


  Declaration of class TrackbarCtrl


**************************************************************************/


#ifndef __TRACKBARCTRL_H__
#define __TRACKBARCTRL_H__

class Trackbar : public Window
{
public:
	Trackbar(Window w) : Window(w) {}
	void SetRange(int Min, int Max)
	{ SendMessage(Hwnd(), TBM_SETRANGE, 0, (LPARAM) MAKELONG(Min, Max)); }
	void SetValue(int v)
	{ SendMessage(Hwnd(), TBM_SETPOS, 1, (LONG) v); }
	int GetValue()
	{ return static_cast<int>(SendMessage(Hwnd(), TBM_GETPOS, 0, 0)); }
};

#endif
