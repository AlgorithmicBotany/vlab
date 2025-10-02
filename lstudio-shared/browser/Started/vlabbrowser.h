#ifndef __VLABBROWSER_H__
#define __VLABBROWSER_H__


namespace VLB 
{

class Connection;
class Node;


class VlabBrowser : public Scrollable
{
public:
	VlabBrowser(HWND, const CREATESTRUCT*);
	~VlabBrowser();

	// Windowing
	void ShowInFront();
	void QuitSilently();


	// Message handlers
	bool Char(char);
	bool Command(int, Window, UINT);
	bool InitMenu(MenuManipulator);
	bool KeyDown(UINT);
	bool Paint();
	bool RButtonDown(KeyState, int, int);
	bool LButtonDown(KeyState, int, int);
	bool LBDblClick(KeyState, int, int);
	bool Close();

	// RA operations
	bool Connected() const
	{ return 0 != _pConnect.get(); }
	bool PutFile(const std::string& /* src full path */, const std::string& /* trg full path */);
	bool PutFile(const std::string& /*remote path*/, const std::string& /* local name */, const std::string& /* remote name */);
	bool MakeExtension(const std::string&, const std::string&, const std::string&, std::string*, bool, const string_buffer&);
	bool MakeExtension(std::string&, const std::string&, const std::string&, const string_buffer&);
	bool PrototypeObject(const std::string&);
	// Current directory must be the lab table (location of the local file)
	bool CompareFiles(const std::string&, const std::string&);
	const std::string& CurrentObjectPath() const;
	bool FetchObject(const std::string&, const std::string&);
	bool PutObject(const string_buffer&, const std::string& lbtbl, const std::string& path);
	bool GetExtensions(const std::string&, string_buffer&);
	bool GetFileList(const std::string&, string_buffer&, bool);
	bool DeleteFile(const std::string&, const std::string&);
	char PathSeparator() const;
	void PositionObject(const std::string&);

	void ApplyOptions(const Options&);


	static void Register(HINSTANCE, WORD);
	static const TCHAR* _ClassName()
	{ return __TEXT("VlabBrowser"); }

	class NotifySink
	{
	public:
		virtual void BrowserConnecting(RemoteAccess*) = 0;
		virtual bool BrowserDisconnecting(RemoteAccess*) = 0;
		virtual Window CreateProject(const std::string&, const std::string&, RemoteAccess*) = 0;
		virtual void ObjectFetched(Window) = 0;
		virtual void Lock() = 0;
		virtual void Unlock() = 0;
		virtual void SetPasteSource(RemoteAccess*, const std::string&, bool) = 0;
		virtual bool IsPasteSourceSet() const = 0;
		virtual void GetPasteSource(RemoteAccess*&, std::string&, bool&) const = 0;
	};

	class LockSink
	{
	public:
		LockSink(NotifySink* pSink) : _pSink(pSink)
		{ _pSink->Lock(); }
		~LockSink()
		{ _pSink->Unlock(); }
	private:
		NotifySink* _pSink;
	};

	class CommandRedirector
	{
	public:
		virtual void ObjectOpen(const std::string&, const std::string&) = 0;
	};

	template <class Target>
	class CommandRedir : public CommandRedirector
	{
	public:
		CommandRedir(Target* pTarget) : _pTarget(pTarget) {}
		void ObjectOpen(const std::string& path, const std::string& name)
		{ _pTarget->ObjectOpen(path, name); }
	private:
		Target* _pTarget;
	};

	void SetRedirector(CommandRedirector* pRedirector)
	{ _pRedirector = pRedirector; }

	static VlabBrowser* Create
		(HINSTANCE, NotifySink*, const char*, const string_buffer& ignored, Window* = 0, int = ChWndId);
	static VlabBrowser* Create
		(
		HINSTANCE, NotifySink*, 
		const char*, const char*, const char*, const char*, 
		const string_buffer& crlf, const string_buffer& ignored,
		Window* = 0, int = ChWndId
		);
	RemoteAccess* RASink() 
	{ return &_RASink; }
private:

	
	RECT _GetNodeRect(const Node*) const;

	bool _LockDrawing;
	bool _silently;

	enum { ChWndId = 2 };

	void _StorePlacement();
	static bool _LoadWindowPlacement(WINDOWPLACEMENT&);
	NotifySink* _pNotifySink;
	CommandRedirector* _pRedirector;

	struct CreateData
	{
		CreateData(const string_buffer& crlf, const string_buffer& ignored, NotifySink* pSink, const char* oofs) : 
			_crlf(crlf), _ignored(ignored),
			_pSink(pSink), _oofs(oofs),
			_host(0), _user(0), _paswd(0)
		{
			_fntsize = 16;
			_fntName = "Arial";
			_bgColor = RGB(0, 64, 0);
		}
		CreateData
			(const string_buffer& crlf, const string_buffer& ignored, 
			NotifySink* pSink, const char* oofs, const char* host, const char* user, const char* paswd) : 
			_crlf(crlf), _ignored(ignored),
			_pSink(pSink), _oofs(oofs), 
			_host(host), _user(user), _paswd(paswd)
		{
			_fntsize = 16;
			_fntName = "Arial";
			_bgColor = RGB(0, 64, 0);
		}
		const string_buffer& _crlf;
		const string_buffer& _ignored;
		NotifySink* _pSink;
		const char* _oofs;
		const char* _host;
		const char* _user;
		const char* _paswd;
		int _fntsize;
		const char* _fntName;
		COLORREF _bgColor;
	};


	void _Connect();
	void _ConnectLocal(const char*);
	void _ConnectRemote(const CreateData*);//char* host, const char* user, const char* pswd, const char* oofs);
	void _Disconnect();
	void _Close();
	void _ToggleExtensions();
	void _ShowExtensions();
	void _ShowAllExtensions();
	void _ToggleExpand(Node*, bool);
	void _Expand(Node*, bool);

	void _TransferObjects(RemoteAccess*, const char*, const char*, LabTable&, bool);
	void _SelectFirstChild();
	void _SelectParent();
	void _SelectNextChild();
	void _SelectPrevChild();

	bool _LooksLikeAProject(const string_buffer&) const;

	// will try to find the child, expanding nodes if necessary
	Node* _ForceLocate(const std::string&, Canvas&);

	void _ToggleIcon();
	void _ShowIcons(bool /*recursive*/, bool /*on|off*/);
	void _OpenObject();
	void NativeOpenObject();

	void _DeleteObject();
	void _CopySubtree(bool);
	void _PasteObjects();
	void _SelChanged(Canvas&, const Node*, const Node*);
	void _LoadOptions(const TCHAR*);
	void _SaveOptions(const TCHAR*) const;
	void _RenameObject();

	int MaxScrollX() const;
	int MaxScrollY() const;
	int MaxX() const;
	int MaxY() const;

	std::auto_ptr<Connection> _pConnect;
	std::auto_ptr<Node> _pNode;
	Node* _pActive;
	TargetAccess<VlabBrowser> _RASink;
	Font _font;
	string_buffer _ignored;
	POINT _RootPos;
	COLORREF _bgColor;
	int _iconWidth;

};

template<class Target>
class RANotifySink : public VlabBrowser::NotifySink
{
public:
	RANotifySink(Target* pTarget) : _pTarget(pTarget) {}
	void BrowserConnecting(RemoteAccess* pRA) 
	{ _pTarget->BrowserConnecting(pRA); }
	bool BrowserDisconnecting(RemoteAccess* pRA) 
	{ return _pTarget->BrowserDisconnecting(pRA); }
	Window CreateProject(const std::string& path, const std::string& name, RemoteAccess* pRA) 
	{ return _pTarget->CreateProject(path, name, pRA); }
	void ObjectFetched(Window w) 
	{ _pTarget->ObjectFetched(w); }
	void Lock() 
	{ _pTarget->Lock(); }
	void Unlock() 
	{ _pTarget->Unlock(); }
	void SetPasteSource(RemoteAccess* pRA, const std::string& path, bool recursive) 
	{ _pTarget->SetPasteSource(pRA, path, recursive); }
	bool IsPasteSourceSet() const 
	{ return _pTarget->IsPasteSourceSet(); }
	void GetPasteSource(RemoteAccess*& pRA, std::string& path, bool& recursive) const 
	{ _pTarget->GetPasteSource(pRA, path, recursive); }
private:
	Target* _pTarget;
};


}

#else
	#error File already included
#endif
