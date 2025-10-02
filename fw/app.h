/**************************************************************************

  File:		app.h
  Created:	24-Nov-97


  Declaration of class App


**************************************************************************/


#ifndef __APP_H__
#define __APP_H__


class Window;
class Exception;

class App
{
public:
	App(HINSTANCE, UINT = 0);
	~App();

	static HINSTANCE GetInstance() 
	{ 
		if (!IsDll())
			return theApp->_hInst; 
		else
			return _hDllInst;
	}

	int Execute();

	static void SetModeless(const Window&);
	static void SetModeless(HWND hwnd)
	{
		if (0 != theApp)
			theApp->_hModeless = hwnd;
	}
	static void ClearModeless()
	{
		if (0 != theApp)
			theApp->_hModeless = 0;
	}
	HWND GetModeless() const
	{ return _hModeless; }

	Window* WMain() const
	{ return _pMain; }

	const OSVERSIONINFO& GetVersion()
	{ return _os; }

	void LoadAccelerators(const char* acc)
	{ _hAccel = ::LoadAccelerators(GetInstance(), acc); }
	void ErrorBox(const Exception&) const;

	HMENU GetContextMenu(int which) const
	{ return _contextMenus.SubMenu(which); }

	void PostQuit() const
	{ _pMain->PostClose(); }

	static void IsDll(HINSTANCE hInst)
	{ _hDllInst = hInst; }
	static bool IsDll()
	{ return 0 != _hDllInst; }
	static App* theApp;
private:
	Window* _pMain;
protected:

	virtual void _Show(Window::sw);
	void SetMain(Window* pMain)
	{ _pMain = pMain; }
	HACCEL _Accelerator() const
	{ return _hAccel; }
	void _TranslateAndDispatch(MSG&);
private:
	const HINSTANCE _hInst;
	static HINSTANCE _hDllInst;
	HACCEL _hAccel;
	HWND _hModeless;
	OSVERSIONINFO _os;
	Menu _contextMenus;
};


class MDIApp : public App
{
public:
	MDIApp(HINSTANCE, const MDIMenus::InitData*, int, UINT = 0);

	int Execute();
protected:
	void _PostCreate();
	void _Show(Window::sw);
	const MDIMenus& _GetMDIMenus() const
	{ return _Menus; }
	void _TranslateAndDispatch(MSG&);
private:
	const Window* _pMDIClient;
	MDIMenus _Menus;
};




#endif
