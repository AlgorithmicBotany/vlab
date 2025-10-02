#ifndef __CLIPBOARD_H__
#define __CLIPBOARD_H__


class UseClipboard
{
public:
	UseClipboard(HWND hwnd);
	~UseClipboard()
	{
		CloseClipboard();
	}
	void Empty()
	{ EmptyClipboard(); }
};


class ClipboardMemory
{
public:
	ClipboardMemory(DWORD size);
	~ClipboardMemory();
	HGLOBAL Release()
	{
		HGLOBAL hRet = _hMem;
		_hMem = 0;
		return hRet;
	}
	HGLOBAL Handle()
	{ return _hMem; }
private:
	HGLOBAL _hMem;
};




class MemoryLock
{
public:
	MemoryLock(HGLOBAL hMem)
	{
		_hMem = hMem;
		_pVoid = GlobalLock(_hMem);
	}
	~MemoryLock()
	{
		GlobalUnlock(_hMem);
	}
	void* Ptr()
	{ return _pVoid; }
private:
	HGLOBAL _hMem;
	void* _pVoid;
};



template<typename T>
void ToClipboard(const T& v, char*& pCur)
{
	T* pP = reinterpret_cast<T*>(pCur);
	*pP = v;
	pCur += sizeof(T);
}

template<typename T>
void FromClipboard(T& v, const char*& pCur)
{
	const T* pP = reinterpret_cast<const T*>(pCur);
	v = *pP;
	pCur += sizeof(T);
}



#endif
