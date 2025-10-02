/**************************************************************************

  File:		appctrl.h
  Created:	11-Dec-97


  Declaration of class AppCtrl


**************************************************************************/


#ifndef __APPCTRL_H__
#define __APPCTRL_H__



template <class QuitPolicy, class StatusBarPolicy>
class TopCtrl : public Ctrl
{
public:
	TopCtrl(HWND hWnd, const CREATESTRUCT* pCS) : Ctrl(hWnd, pCS), _StatusBar(Hwnd())
	{}
	~TopCtrl()
	{ QuitPolicy::Execute(); }
public:
	bool Size(SizeState sst, int w, int h)
	{
		_StatusBar.Size(sst, w, h);
		_waheight = h-_StatusBar.Height();
		return true;
	}
	bool MenuSelect(int id, UINT flags)
	{
		if (!(flags & MF_POPUP))
		{
			_StatusBar.ShowPrompt(id);
		}
		return true;
	}
	bool ExitMenuLoop()
	{
		_StatusBar.ClearPrompt();
		return true;
	}
	int WorkAreaHeight() const
	{ return _waheight; }
	void SetStatusText(const std::string& str)
	{ _StatusBar.SetText(str); }
private:
	StatusBarPolicy _StatusBar;
	int _waheight;
};


class PostQuit
{
public:
	static void Execute()
	{ PostQuitMessage(0); }
};

class DoNothing
{
public:
	static void Execute()
	{}
};

template <class StatusBarPolicy>
class AppCtrl : public TopCtrl<PostQuit, StatusBarPolicy>
{
protected:
	AppCtrl(HWND hWnd, const CREATESTRUCT* pCS) : TopCtrl<PostQuit, StatusBarPolicy>(hWnd, pCS)
	{}
};


class StatusBarOff
{
public:
	StatusBarOff(HWND) {}
	void Size(SizeState, int, int) {}
	int Height() const
	{ return 0; }
	void ShowPrompt(int) {}
	void ClearPrompt() {}
	void SetText(const std::string&) {}
};


class StatusBarOn
{
public:
	StatusBarOn(HWND hWnd) : _statusBar(hWnd) {}
	void Size(SizeState sst, int w, int h)
	{ _statusBar.Size(sst, w, h); }
	int Height() const
	{ return _statusBar.Height(); }
	void ShowPrompt(int id);
	void ClearPrompt()
	{ _statusBar.SetText(""); }
	void SetText(const std::string& txt)
	{ _statusBar.SetText(txt); }
private:
	StatusBar _statusBar;
};

#endif
