#ifndef __WINENUMERATOR_H__
#define __WINENUMERATOR_H__


template <typename T, typename U=char>
class ChildEnumerator
{
public:
	typedef bool (T::*tEnumProc)(HWND, U*);
	ChildEnumerator(tEnumProc EProc, T* pT, U* pU=0) : _EProc(EProc), _pT(pT), _pU(pU) {}
	void Enumerate(HWND hWnd)
	{ ::EnumChildWindows(hWnd, _EnumProc, reinterpret_cast<LPARAM>(this)); }
private:
	static BOOL CALLBACK _EnumProc(HWND hWnd, LPARAM lParam)
	{
		ChildEnumerator<T,U>* pSelf = reinterpret_cast<ChildEnumerator<T,U>*>(lParam);
		return ((pSelf->_pT)->*(pSelf->_EProc))(hWnd, (pSelf->_pU)) ? TRUE : FALSE;
	}
	tEnumProc _EProc;
	T* _pT;
	U* _pU;
};

template <typename T, typename U=char>
class ChildCEnumerator
{
public:
	typedef bool (T::*tEnumProc)(HWND, const U*);
	ChildCEnumerator(tEnumProc EProc, T* pT, const U* pU=0) : _EProc(EProc), _pT(pT), _pU(pU) {}
	void Enumerate(HWND hWnd)
	{ ::EnumChildWindows(hWnd, _EnumProc, reinterpret_cast<LPARAM>(this)); }
private:
	static BOOL CALLBACK _EnumProc(HWND hWnd, LPARAM lParam)
	{
		ChildCEnumerator<T,U>* pSelf = reinterpret_cast<ChildCEnumerator<T,U>*>(lParam);
		return ((pSelf->_pT)->*(pSelf->_EProc))(hWnd, (pSelf->_pU)) ? TRUE : FALSE;
	}
	tEnumProc _EProc;
	T* _pT;
	const U* _pU;
};



#endif
