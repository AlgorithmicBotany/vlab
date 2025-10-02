/**************************************************************************

  File:		mdictrl.h
  Created:	24-Nov-97


  Declaration of class MDICtrl


**************************************************************************/


#ifndef __MDICTRL_H__
#define __MDICTRL_H__



class MDICtrl : public AppCtrl<StatusBarOn>
{
public:
	MDICtrl(HWND, const CREATESTRUCT*);
	~MDICtrl();

	const Window* MdiClient() const
	{ return &_MDIClient; }

	void TileHorizontally()
	{ FORWARD_WM_MDITILE(_MDIClient.Hwnd(), MDITILE_HORIZONTAL, SendMessage); }
	void TileVertically()
	{ FORWARD_WM_MDITILE(_MDIClient.Hwnd(), MDITILE_VERTICAL, SendMessage); }
	void Cascade()
	{ FORWARD_WM_MDICASCADE(_MDIClient.Hwnd(), 0, SendMessage); }
	void ArrangeIcons()
	{ FORWARD_WM_MDIICONARRANGE(_MDIClient.Hwnd(), SendMessage); }
	bool CloseAll();

	virtual bool Close()
	{ return false; }
	virtual bool Command(int id, Window, UINT)
	{
		if (_Command(id))
			return true;
		else
		{
			_UnhandledCommand(id);
			return false;
		}
	}
	virtual bool DropFiles(HDROP)
	{ return false; }
	virtual bool InitMenu(MenuManipulator)
	{ return false; }
	virtual bool QueryEndSession()
	{ return false; }
	virtual bool ShowWindow(bool)
	{ return false; }
	virtual bool Size(SizeState, int, int);

protected:

	virtual bool _Command(int)
	{ return false; }
	void _UnhandledCommand(int id)
	{
		HWND hChild = FORWARD_WM_MDIGETACTIVE(_MDIClient.Hwnd(), SendMessage);
		if (IsWindow(hChild))
			FORWARD_WM_COMMAND(hChild, id, 0, 0, SendMessage);
	}

	HWND _NewMDIDocument(const std::string&, const std::string&, const void* lParam = 0);
	template<typename Doc>
		Doc* NewMDIDocument(const std::string& title, const void* lParam)
	{
		HWND hNew = _NewMDIDocument(Doc::ClassName(), title, lParam);
		Doc* pRes = GetWinLong<Doc*>(hNew);
		return pRes;
	}
	virtual void _Exit();

	const MDIClient& GetMDIClient() const
	{ return _MDIClient; }
private:
	HWND _hToolbar;
	MDIClient _MDIClient;
	static BOOL CALLBACK _CloseEnumProc(HWND, LPARAM);
};


#endif
