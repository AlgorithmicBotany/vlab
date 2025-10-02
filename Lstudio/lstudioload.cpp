#include <warningset.h>

#include <memory>
#include <fw.h>


#include "lstudioload.h"


class LstudioClassFactory : public IClassFactory
{
public:
	LstudioClassFactory() { _refCount = 0; }

	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)();
	STDMETHOD_(ULONG, Release)();

	STDMETHOD(CreateInstance)(IUnknown* pUnknown, REFIID riid, void** ppvObj);
	STDMETHOD(LockServer)(THIS_ BOOL flock);
	bool CanUnload() const
	{ return 0==_refCount; }
	static LstudioClassFactory* theFactory();
private:
	ULONG _refCount;
};


static LstudioClassFactory factory;

LstudioClassFactory* LstudioClassFactory::theFactory()
{ return &factory; }

class LoadObject : public ILoadObject
{
public:
	LoadObject();
	~LoadObject();

	STDMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	STDMETHOD_(bool, Load)(THIS_ const char*);
	static void Lock(BOOL lock);
	static bool IsClean()
	{ return 0==_counter; }
private:
	ULONG _refCount;
	static ULONG _counter;
};


STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void** ppV)
{
	*ppV = 0;
	if (clsid != CLSID_Lstudio)
		return CLASS_E_CLASSNOTAVAILABLE;
	if (riid != IID_IUnknown && riid != IID_IClassFactory)
		return E_INVALIDARG;

	*ppV = LstudioClassFactory::theFactory();
	LstudioClassFactory::theFactory()->AddRef();
	return NOERROR;
}


STDAPI DllCanUnloadNow()
{
	if (LoadObject::IsClean() && LstudioClassFactory::theFactory()->CanUnload())
		return S_OK;
	else
		return S_FALSE;
}


void DoRegisterServer();

STDAPI DllRegisterServer()
{
	try
	{
		DoRegisterServer();
	}
	catch (...)
	{
		return S_FALSE;
	}
	return S_OK;
}

void DoRegisterServer()
{
	TCHAR ClassId[GUID_SIZE+16];
	_stprintf
		(
		ClassId, "CLSID\\{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
		CLSID_Lstudio.Data1, CLSID_Lstudio.Data2, 
		CLSID_Lstudio.Data3, 
		CLSID_Lstudio.Data4[0],
		CLSID_Lstudio.Data4[1],
		CLSID_Lstudio.Data4[2],
		CLSID_Lstudio.Data4[3],
		CLSID_Lstudio.Data4[4],
		CLSID_Lstudio.Data4[5],
		CLSID_Lstudio.Data4[6],
		CLSID_Lstudio.Data4[7]
		);
	{
		HINSTANCE hInst = GetModuleHandle("Lstudio.exe");
		TCHAR Module[MAX_PATH];
		
		// Obtain the path to this module's executable file for later use.
		GetModuleFileName(hInst, Module, MAX_PATH);
		RegKey key(HKEY_CLASSES_ROOT, ClassId, KEY_WRITE);
		key.StoreString(0, "Lstudio");
		RegKey inproc(key, "InprocServer32", KEY_WRITE);
		inproc.StoreString(0, Module);
	}
	{
		RegKey key(HKEY_CLASSES_ROOT, "Lstudio", KEY_WRITE);
		key.StoreString(0, "L-studio");
		RegKey clsid(key, "CLSID", KEY_WRITE);
		clsid.StoreString(0, ClassId+6);
		RegKey curver(key, "CurVer", KEY_WRITE);
		curver.StoreString(0, "L-studio.3.1");
	}
}


STDAPI DllUnregisterServer()
{
	TCHAR ClassId[GUID_SIZE+16];
	_stprintf
		(
		ClassId, "CLSID\\{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
		CLSID_Lstudio.Data1, CLSID_Lstudio.Data2, 
		CLSID_Lstudio.Data3, 
		CLSID_Lstudio.Data4[0],
		CLSID_Lstudio.Data4[1],
		CLSID_Lstudio.Data4[2],
		CLSID_Lstudio.Data4[3],
		CLSID_Lstudio.Data4[4],
		CLSID_Lstudio.Data4[5],
		CLSID_Lstudio.Data4[6],
		CLSID_Lstudio.Data4[7]
		);
	RegDeleteKey(HKEY_CLASSES_ROOT, ClassId);

	return S_OK;
}




STDMETHODIMP LstudioClassFactory::QueryInterface(REFIID riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_IClassFactory)
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
}


STDMETHODIMP_(ULONG) LstudioClassFactory::AddRef()
{
	return ++_refCount;
}


STDMETHODIMP_(ULONG) LstudioClassFactory::Release()
{
	return --_refCount;
}


STDMETHODIMP LstudioClassFactory::CreateInstance(IUnknown* pUnknown, REFIID riid, void** ppvObj)
{
	if (0 != pUnknown)
		return CLASS_E_NOAGGREGATION;

	std::auto_ptr<LoadObject> pRes;
	try
	{
		std::auto_ptr<LoadObject> pNew(new LoadObject);
		pRes = pNew;
	}
	catch (...)
	{
		return E_OUTOFMEMORY;
	}
	HRESULT hErr = pRes->QueryInterface(riid, ppvObj);
	if (FAILED(hErr))
	{
		return hErr;
	}
	else
		pRes.release();
	return NOERROR;
}


STDMETHODIMP LstudioClassFactory::LockServer(BOOL lock)
{
	LoadObject::Lock(lock);
	return NOERROR;
}


ULONG LoadObject::_counter = 0;

LoadObject::LoadObject()
{
	_refCount = 0;
	++_counter;
}

LoadObject::~LoadObject()
{
	--_counter;
}


void LoadObject::Lock(BOOL lock)
{
	if (lock)
		++_counter;
	else
		--_counter;
}


STDMETHODIMP LoadObject::QueryInterface(REFIID riid, void** ppvObj)
{
	if (riid == IID_IUnknown || riid == IID_ILoadObject)
	{
		*ppvObj = this;
		AddRef();
		return S_OK;
	}
	else
	{
		*ppvObj = 0;
		return E_NOINTERFACE;
	}
}

STDMETHODIMP_(ULONG) LoadObject::AddRef()
{
	return ++_refCount;
}

STDMETHODIMP_(ULONG) LoadObject::Release()
{
	if (--_refCount==0)
	{
		delete this;
		return 0;
	}
	else
		return _refCount;
}


STDMETHODIMP_(bool) LoadObject::Load(const char* obj)
{
	MessageBox(App::theApp->HMain(), obj, "Lstudio as server", MB_ICONEXCLAMATION);
	return true;
}
