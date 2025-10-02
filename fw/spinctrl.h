#ifndef __SPINCTRL_H__
#define __SPINCTRL_H__


class SpinCtrl : public Window
{
public:
	SpinCtrl(HWND hwnd) : Window(hwnd)
	{}

	void SetRange(int, int);
	void SetPos(int);
};


#endif
