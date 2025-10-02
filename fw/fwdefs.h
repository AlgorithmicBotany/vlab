#ifndef __FWDEFS_H__
#define __FWDEFS_H__


#ifdef _DEBUG
#define DECLARE_COUNTER \
	public : \
	static bool IsClean() \
	{ return 0==_counter; } \
	private: \
	static int _counter
#define INIT_COUNTER(clssnm) \
	int clssnm::_counter = 0
#define INC_COUNTER _counter++
#define DEC_COUNTER _counter--
#else
#define DECLARE_COUNTER
#define INIT_COUNTER(clssnm)
#define INC_COUNTER
#define DEC_COUNTER
#endif

#define CountOf(array) sizeof(array)/sizeof(array[0])


#define MSG_MOVESPLITTER (WM_APP + 1)

#define FORWARD_MSG_MOVESPLITTER(hwnd, val, fn) \
	(fn)((hwnd), MSG_MOVESPLITTER, (WPARAM)(val), (LPARAM) 0)

#define HANDLE_MSG_MOVESPLITTER(hwnd, wParam, lParam, fn) \
	((fn)((hwnd), (int)(wParam)), 0)

#endif
