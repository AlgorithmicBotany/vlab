#ifndef __COMTMPLTS_H__
#define __COMTMPLTS_H__



template<class T>
class CoInst
{
public:
	CoInst(REFCLSID rclsid, IUnknown* pUnk, DWORD cntxt, REFIID riid, UINT ermsg)
	{
		HRESULT hRes = CoCreateInstance
			(rclsid, pUnk, cntxt, riid, (void**)&_pI);
		if (!(SUCCEEDED(hRes)))
			throw Exception(ermsg);
	}
	~CoInst()
	{
		if (0 != _pI)
			_pI->Release();
	}
	operator T*()
	{ return _pI; }
	T* operator->()
	{ return _pI; }
protected:
	CoInst() : _pI(0) {}
	T* _pI;
};


template<class T>
class Interface
{
public:
	Interface() : _pI(0) {}
	Interface(IUnknown* pUnk, REFIID iid, UINT ermsg)
	{
		HRESULT hRes = pUnk->QueryInterface(iid, (void**)&_pI);
		if (!(SUCCEEDED(hRes)))
			throw Exception(ermsg);
	}
	~Interface()
	{
		if (0 != _pI)
			_pI->Release();
	}
	T* operator->()
	{ return _pI; }
	bool IsSet() const
	{ return (0 != _pI); }
	void Set(IUnknown* pUnk, REFIID iid, UINT ermsg)
	{
		assert(!IsSet());
		HRESULT hRes = pUnk->QueryInterface(iid, (void**)&_pI);
		if (!(SUCCEEDED(hRes)))
			throw Exception(ermsg);
	}
protected:
	T* _pI;
};


template <class T>
class SShellPtr
{
public:
    ~SShellPtr ()
    {
        _Free ();
        _malloc->Release ();
    }
    T * operator->() 
	{
		assert(0 != _p);
		return _p; 
	}
    T const * operator->() const 
	{
		assert(0 != _p);
		return _p; 
	}
protected:
    SShellPtr () : _p (0) 
    {
        // Obtain malloc here, rather than
        // in the destructor. 
        // Destructor must be fail-proof.
        // Revisit: Would static IMalloc * _shellMalloc work?
        if (SHGetMalloc (& _malloc) == E_FAIL)
            throw Exception(__TEXT("Hmmm ...")); 
    }
    void _Free ()
    {
        if (_p != 0)
            _malloc->Free (_p);
        _p = 0;
    }

    T * _p;
    IMalloc *  _malloc;
private:
    SShellPtr (const SShellPtr&) { assert(0); }
    void operator = (const SShellPtr&) { assert(0); }
};




class ComMalloc : public CoInst<IMalloc>
{
public:
	ComMalloc()
	{
		HRESULT hRes = CoGetMalloc(1, &_pI);
		if (hRes != S_OK)
			throw Exception("Cannot get IMalloc");
	}
};

#endif
