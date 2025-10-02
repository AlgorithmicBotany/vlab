#ifndef __TIMER_H__
#define __TIMER_H__


namespace FW
{

class Timer
{
public:
	Timer(Window*, UINT id, UINT timeout, bool start);
	void Start();
	void Kill();
	~Timer()
	{ Kill(); }
	UINT Id() const
	{ return _id; }
private:
	HWND _hWnd;
	const UINT _id;
	const UINT _timeout; 
	bool _active;
};

}

#endif
