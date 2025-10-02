#ifndef __LSTUDIOCTRL_H__
#define __LSTUDIOCTRL_H__


class ComUser
{
public:
	ComUser()
	{ CoInitialize(0); }
	~ComUser()
	{ CoUninitialize(); }
};

class LProjectCtrl;
class CommandLineParams;

namespace VLB
{
class RemoteAccess;
}

class LStudioCtrl : public MDICtrl, public VLB::VlabBrowser::NotifySink
{
public:
	LStudioCtrl(HWND, const CREATESTRUCT*);
	~LStudioCtrl();
	static Window* Create(HINSTANCE, const MDIMenus&);
	static void Register(HINSTANCE);

	bool Close();
	bool DropFiles(HDROP);
	bool InitMenu(MenuManipulator);
	bool QueryEndSession();
	bool ShowWindow(bool);
	bool Timer(UINT);

	void BrowserConnecting(VLB::RemoteAccess*);
	bool BrowserDisconnecting(VLB::RemoteAccess*);
	Window CreateProject(const std::string&, const std::string&, VLB::RemoteAccess*);
	void ObjectFetched(Window);
	void AdjustMenu(MenuManipulator&);

	void Lock()
	{ Enable(false); }
	void Unlock()
	{ Enable(true); }
	void SetPasteSource(VLB::RemoteAccess*, const std::string&, bool bRecusrive, bool bHyperCopy);
	bool IsPasteSourceSet() const;
	void GetPasteSource(VLB::RemoteAccess*&, std::string&, bool&) const;
	void ClearPasteSource() { _pasteSrc.Clear(); }
	bool IsHyperCopy() const { return _pasteSrc.IsHyperCopy(); }
	
private:

	bool _Command(int);
	void _New();
	void _Help();
	void _LpfgManual();
	void _VVManual();
	void _CpfgUsersManual();
	void _CpfgEnvironmentalPrograms();
	void _CpfgGraphicsExtensions();
	void _About();
	void _Open();
	void _Open(const TCHAR*);
	void _OpenGallery();
	void _Import();
	void _Import(const std::string&);
	void _ImportLink(const std::string&);
	void _ViewOptions();
	void _SelectEditorFont();
	void _GridViewOptions();
	void _Settings();
	void _SaveOptions();
	void _LoadOptions();
	void _SaveOptions(const std::string&);
	void _LoadOptions(const std::string&);
	bool _TryLastConfig();
	void _TryDefaultConfig();
	void _LoadArgv1();
	void _StartBrowser();
	void GetLastConfig(std::string&) const;
	void GetDefaultConfig(std::string&) const;
	void UpdateHelpMenu(const LStudioOptions& options);
	void UpdateHelpMenu(MenuManipulator& helpMenu, const LStudioOptions& options);
	void PostProjectCreate(Window& window);

	void OpenBrowserLocal();
	void OpenBrowserLocal(const std::string&);
	void _OpenBrowserRemote();

	void _ShellOpen(const TCHAR*);
	static void _SetGridViewOptApplyClbck(const COLORREF*, const float*, void*);
	void _SetGridViewOptApply(const COLORREF*, const float*);
	bool _SetFontProc(HWND, char*);
	bool _SetViewOptionsProc(HWND, char*);
	bool _VLBrowserDisconnecting(HWND, VLB::RemoteAccess*);

	bool UpdateHelpMenuProc(HWND hWnd, const LStudioOptions* pOptions);

	static void _SetEditorFontApplyClbck(const LOGFONT&, COLORREF, COLORREF, void*);
	void _DoSetEditorFontApply(const LOGFONT&, COLORREF, COLORREF);


	bool LooksLikeOofs(const FindFile&) const;
	bool _LooksLikeAProject(const std::string&) const;

	void _LoadWindowPlacement(WINDOWPLACEMENT&);
	void _StoreWindowPlacement();

	static const TCHAR* _ClassName()
	{ return __TEXT("LStudioCtrlClass"); }

	enum Params
	{
		eExpiredTimerId = 102,
		eExpiredTimerTimeout = 200
	};
	int _titlePos;

	ComUser _comUser;

	std::string _LastFolderOpened;
	TCHAR _HelpFolder[MAX_PATH+1];
	std::string _StartupPath;

	LStudioApp* _pApp;
	const CommandLineParams& _clp;

	class PasteSource
	{
	public:
		PasteSource() : _pRA(0), _path(""), _recursive(false), _bHyperCopy(false) {}
		void Clear()
		{ 
			_pRA = 0;
			_path = "";
			_bHyperCopy = false;
		}
		VLB::RemoteAccess* RA() const
		{ return _pRA; }
		bool IsSet() const
		{ return 0 != _pRA; }
		void Set(VLB::RemoteAccess* pRA, const std::string& path, bool bRecursive, bool bHyperCopy)
		{
			_pRA = pRA;
			_path = path;
			_recursive = bRecursive;
			_bHyperCopy = bHyperCopy;
		}
		void Get(VLB::RemoteAccess*& pRA, std::string& path, bool& recursive) const
		{
			pRA = _pRA;
			path = _path;
			recursive = _recursive;
		}
		bool IsHyperCopy() const { return _bHyperCopy; }
	private:
		VLB::RemoteAccess* _pRA;
		std::string _path;
		bool _recursive;
		bool _bHyperCopy;
	};

	PasteSource _pasteSrc;

	std::vector<VLB::RemoteAccess*> _aBrowsers;
	//VLB::RANotifySink<LStudioCtrl> _browserSink;
};


#else
	#error File already included
#endif
