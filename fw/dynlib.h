#ifndef __DYNLIB_H__
#define __DYNLIB_H__


class DynLib
{
public:
	DynLib(const std::string& fnm)
	{
		_hDll = LoadLibrary(fnm.c_str());
	}
	~DynLib()
	{
		if (0 != _hDll)
			FreeLibrary(_hDll);
	}
	bool IsOK() const
	{ return (0 != _hDll); }
	FARPROC GetProc(const std::string& nm)
	{ 
		assert(IsOK());
		return GetProcAddress(_hDll, nm.c_str());
	}
private:
	HINSTANCE _hDll;
};


#endif

