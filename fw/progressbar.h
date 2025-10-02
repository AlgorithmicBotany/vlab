#ifndef __PROGRESSBARCTRL_H__
#define __PROGRESSBARCTRL_H__


class ProgressBar : public Window
{
public:
	ProgressBar() : Window(0) {}
	ProgressBar(Window w) : Window(w) {}
	void SetRange(int nmin, int nmax)
	{ SendMessage(Hwnd(), PBM_SETRANGE, 0, MAKELPARAM(nmin, nmax)); }
	void SetStep(int step)
	{ SendMessage(Hwnd(), PBM_SETSTEP, step, 0); }
	void Advance()
	{ PostMessage(Hwnd(), PBM_STEPIT, 0, 0); }
};

#endif
