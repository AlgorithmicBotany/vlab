#ifndef __VLABBROWSER_H__
#define __VLABBROWSER_H__


#include "connection.h"
#include "node.h"

namespace VLB 
{

class VlabBrowser : public Scrollable, public RemoteAccess
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
	void DoPaint(Canvas&, const RECT&);
	bool RButtonDown(KeyState, int, int);
	bool LButtonDown(KeyState, int, int);
	bool LBDblClick(KeyState, int, int);
	bool Close();

	// RA operations
	bool Connected() const
	{ return 0 != _pConnect.get(); }
	bool PutFile(const char* /* src full path */, const char* /* trg full path */);
	bool PutFile(const char* /*remote path*/, const char* /* local name */, const char* /* remote name */);
	bool MakeExtension(const char*, const char*, const char*, std::string*, bool, const string_buffer&);
	bool MakeExtension(std::string&, const char*, const char*, const string_buffer&);
	bool PrototypeObject(const char*);
	// Current directory must be the lab table (location of the local file)
	bool CompareFiles(const char*, const char*) const;
	const char* CurrentObjectPath() const;
	const char* OofsRoot() const;
	bool FetchObject(const std::string&, const std::string&);
	bool PutObject(const string_buffer&, const std::string& lbtbl, const std::string& path);
	bool GetExtensions(const char*, string_buffer&) const;
	bool GetFileList(const char*, string_buffer&, bool) const;
	bool DeleteFile(const char*, const char*);
	char PathSeparator() const;
	bool SupportsTar() const;
	bool Archive(const char*, TmpFile&, bool);
	void PositionObject(const std::string&);
	bool CanPasteNow() const;

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
		virtual void SetPasteSource(RemoteAccess*, const std::string&, bool bRecursive, bool bHyperCopy) = 0;
		virtual bool IsPasteSourceSet() const = 0;
		virtual bool IsHyperCopy() const = 0;
		virtual void GetPasteSource(RemoteAccess*&, std::string&, bool&) const = 0;
		virtual void ClearPasteSource() = 0;
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


	static VlabBrowser* Create
		(HINSTANCE, NotifySink*, const char*, const string_buffer& ignored, Window* = 0, int = ChWndId);
	static VlabBrowser* Create
		(
		HINSTANCE, NotifySink*, 
		const char*, const char*, const char*, const char*, 
		const string_buffer& crlf, const string_buffer& ignored,
		Window* = 0, int = ChWndId
		);
private:

	
	RECT _GetNodeRect(const Node*) const;

	bool _LockDrawing;
	bool _silently;

	enum { ChWndId = 2 };

	void _StorePlacement();
	static bool _LoadWindowPlacement(WINDOWPLACEMENT&);
	NotifySink* _pNotifySink;

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

	void HandleKeyDown(UINT);

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
	void ExtendObject();

	void _TransferObjects(RemoteAccess*, const char*, const char*, LabTable&, bool);
	void _SelectFirstChild();
	void _SelectParent();
	void _SelectNextChild();
	void _SelectPrevChild();

	void _MoveObjectUp();
	void _MoveObjectDown();

	bool _LooksLikeAProject(const string_buffer&) const;

	// will try to find the child, expanding nodes if necessary
	Node* _ForceLocate(const std::string&, Canvas&);

	void _ToggleIcon();
	void _ShowIcons(bool /*recursive*/, bool /*on|off*/);
	void _OpenObject();
	void NativeOpenObject();
	void ReconcileGuids();
	void FixOofs();

	void CopyObject(bool bRecursive, bool bHypercopy);
	void _DeleteObject();
	void _PasteObjects();
	void _SelChanged(Canvas&, const Node*, const Node*);
	void _LoadOptions(const TCHAR*);
	void _SaveOptions(const TCHAR*) const;
	void _RenameObject();
	void FindObject();
	void ShowHyperlinkTarget();
	void DoRegularPaste();
	void DoHyperPaste();
	bool CreateUniqueExtension(const char* szPath, const std::string& prefix, std::string& extensionName);
	void WriteOrdering(const Node* pNode, VLB::Connection* pConnection) const;

	int MaxScrollX() const;
	int MaxScrollY() const;
	int MaxX() const;
	int MaxY() const;

	std::unique_ptr<Connection> _pConnect;
	std::unique_ptr<Node> _pNode;
	Node* _pActive;
	Font _font;
	string_buffer _ignored;
	POINT _RootPos;
	int _iconWidth;
	ObjectFindInfo lastFind;
};


}

#else
	#error File already included
#endif
