#ifndef __SUBCLASS_H__
#define __SUBCLASS_H__


template <typename T>
class Subclass
{
public:
	typedef LRESULT (T::*tWProc)(HWND, UINT, WPARAM, LPARAM);
	Subclass(HWND hWnd, tWProc WProc, T* pT) :
		_hWnd(hWnd), _WProc(WProc), _pT(pT)
	{
		_WndProc = reinterpret_cast<WNDPROC> 
			(SetWindowLongPtr(_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR> (_SWProc)));
		_lUD = SetWindowLongPtr(_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}
	~Subclass()
	{
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, _lUD);
		SetWindowLongPtr(_hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(_WndProc));
	}
	WNDPROC WndProc() const
	{ return _WndProc; }

private:

	static LRESULT CALLBACK _SWProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		Subclass<T>* pSelf = reinterpret_cast<Subclass<T>*>
			(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		return ((pSelf->_pT)->*(pSelf->_WProc))(hWnd, msg, wParam, lParam);
	}

	HWND _hWnd;
	tWProc _WProc;
	T* _pT;
	WNDPROC _WndProc;
	LONG_PTR _lUD;
};




#else
	#error File already included
#endif
