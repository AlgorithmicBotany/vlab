#ifndef __WAITFORSEMAPHOREDLG_H__
#define __WAITFORSEMAPHOREDLG_H__

enum
{ eDefaultWaitForFinal = 3 };

class WaitForSemaphoreDlg : public Dialog
{
public:
	WaitForSemaphoreDlg(Semaphore&, UINT);
	bool DoInit();
	bool HandleMsg(HWND, UINT, WPARAM, LPARAM);
	void Timer(HWND, UINT);
protected:
	Semaphore& _semaphore;
	ResString _caption;
	int _time;
	UINT _timerId;
};



#endif
