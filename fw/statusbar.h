/**************************************************************************

  File:		statusbar.h
  Created:	25-Nov-97


  Declaration of class StatusBar


**************************************************************************/


#ifndef __STATUSBAR_H__
#define __STATUSBAR_H__


class StatusBar : public Window
{
public:
	StatusBar(HWND);
	int Height() const
	{ return _height; }
	void Size(SizeState, int, int);
private:
	int _height;
	enum { chId = 1 };
};

#endif
