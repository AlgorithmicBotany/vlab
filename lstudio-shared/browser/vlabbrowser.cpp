#include <string>
#include <vector>
#include <fstream>
#include <strstream>

#include <fw.h>

#include <General/onoff.h>
#include <shstrng.h>

#include <RAconsts.h>
#include "racomm.h"
#include "socket.h"
#include "remaccess.h"
#include "labtbl.h"

#include "vlabbrowser.h"
#include "vlboptns.h"
#include "localcnct.h"
#include "remotecnct.h"
#include "params.h"
#include "rdngobjsdlg.h"
#include "rdngicnsdlg.h"
#include "renameobjdlg.h"


#include "resids.h"
#include "../../Lstudio/resource.h"

void VLB::VlabBrowser::Register(HINSTANCE hInst, WORD iconid)
{
	WndClass wc(hInst, _ClassName(), Wnd<VlabBrowser>::Proc);
	wc.style = CS_DBLCLKS;
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(iconid));
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_BROWSER);

	wc.Register();	
}


VLB::VlabBrowser::VlabBrowser(HWND hWnd, const CREATESTRUCT* pCS) : Scrollable(hWnd, pCS),
//_RASink(this),
_font(reinterpret_cast<CreateData*>(pCS->lpCreateParams)->_fntsize, reinterpret_cast<CreateData*>(pCS->lpCreateParams)->_fntName),
_ignored(reinterpret_cast<CreateData*>(pCS->lpCreateParams)->_ignored)
{
	CreateData* pCD = reinterpret_cast<CreateData*>(pCS->lpCreateParams);
	SetBgColor(pCD->_bgColor);
	_RootPos.x = 0;
	_RootPos.y = pCD->_fntsize*2;

	_iconWidth = Parameters::Params[Parameters::pObjIconX];
	_LockDrawing = false;
	_pActive = 0;
	_pNotifySink = pCD->_pSink;
	_silently = false;
	assert(0 != _pNotifySink);
	if (0 == pCD->_host)
		_ConnectLocal(pCD->_oofs);
	else
		_ConnectRemote(pCD);
	if (_pActive->HasExtensions())
		_ShowExtensions();
}


VLB::VlabBrowser::~VlabBrowser()
{
	try
	{
		_StorePlacement();
	}
	catch (Exception)
	{}
}

VLB::VlabBrowser* VLB::VlabBrowser::Create
(HINSTANCE hInst, NotifySink* pSink, const char* oofs, const string_buffer& ignored, Window* pParent, int chid)
{
	WINDOWPLACEMENT wp;
	WinMaker maker(_ClassName(), hInst);
	if (_LoadWindowPlacement(wp))
	{
		maker.Origin(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
		wp.rcNormalPosition.right -= wp.rcNormalPosition.left;
		wp.rcNormalPosition.bottom -= wp.rcNormalPosition.top;
		maker.Size(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);
	}
	if (0 != pParent)
		maker.MakeChild(chid, pParent->Hwnd());
	else
		maker.MakeOverlapped();
	maker.MakeHVScroll();
	maker.Name(oofs);
	string_buffer crlf;
	CreateData cd(crlf, ignored, pSink, oofs);
	maker.lpData(&cd);
	Window w(maker.Create());
	if (w.IsSet())
	{
		w.Show();
		return reinterpret_cast<VlabBrowser*>(w.GetPtr());
	}
	else
		return 0;
}


VLB::VlabBrowser* VLB::VlabBrowser::Create
(
 HINSTANCE hInst, 
 NotifySink* pSink, 
 const char* host,
 const char* user,
 const char* pswd,
 const char* oofs,
 const string_buffer& crlf,
 const string_buffer& ignored,
 Window* pParent,
 int chid
)
{
	WINDOWPLACEMENT wp;
	WinMaker maker(_ClassName(), hInst);
	if (_LoadWindowPlacement(wp))
	{
		maker.Origin(wp.rcNormalPosition.left, wp.rcNormalPosition.top);
		wp.rcNormalPosition.right -= wp.rcNormalPosition.left;
		wp.rcNormalPosition.bottom -= wp.rcNormalPosition.top;
		maker.Size(wp.rcNormalPosition.right, wp.rcNormalPosition.bottom);
	}
	if (pParent != 0)
		maker.MakeChild(chid, pParent->Hwnd());
	else
		maker.MakeOverlapped();
	maker.MakeHVScroll();
	std::string name(host);
	name += ":";
	name += oofs;
	maker.Name(name.c_str());
	CreateData cd(crlf, ignored, pSink, oofs, host, user, pswd);
	maker.lpData(&cd);
	Window w(maker.Create());
	
	if (w.IsSet())
	{
		w.Show();
		return reinterpret_cast<VlabBrowser*>(w.GetPtr());
	}
	else
		return 0;
}


void VLB::VlabBrowser::QuitSilently()
{
	_silently = true;
	PostMessage(Hwnd(), WM_CLOSE, 0, 0);
}

bool VLB::VlabBrowser::Close()
{
	if (_silently)
		return true;
	if (!CanClose())
	{
		if (!MessageYesNo(SharedStr::GetLibString(SharedStr::strObjectsStillOpen)))
			return false;
	}
	_pNotifySink->BrowserDisconnecting(this);
	return CanClose();
}

void VLB::VlabBrowser::_Close()
{
	PostClose();
}

void VLB::VlabBrowser::ShowInFront()
{
	if (IsIconic(Hwnd()))
		OpenIcon(Hwnd());
	else
		BringWindowToTop(Hwnd());
}


bool VLB::VlabBrowser::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_BROWSER_CLOSE :
			_Close();
			break;
		case ID_OBJECT_OPEN :
			_OpenObject();
			break;
		case ID_OBJECT_DELETE :
			_DeleteObject();
			break;
		case ID_VIEW_SHOWEXTENSIONS :
			_ToggleExtensions();
			break;
		case ID_VIEW_SHOWALLEXTENSIONS :
			_ShowAllExtensions();
			break;
		case ID_VIEW_SHOWICON :
			_ToggleIcon();
			break;
		case ID_VIEW_SHOWALLICONS :
			_ShowIcons(true, true);
			break;
		case ID_VIEW_HIDEALLICONS :
			_ShowIcons(true, false);
			break;
		case ID_OBJECT_COPYOBJECT :
			CopyObject(false, false);
			break;
		case ID_OBJECT_HYPERCOPYOBJECT :
			CopyObject(false, true);
			break;
		case ID_OBJECT_COPYSUBTREE :
			CopyObject(true, false);
			break;
		case ID_OBJECT_PASTEOBJECTS :
			_PasteObjects();
			break;
		case ID_OBJECT_RENAME :
			_RenameObject();
			break;
		case ID_OBJECT_FIND :
			FindObject();
			break;
		case ID_OBJECT_SHOWHYPERLINKTARGET:
			ShowHyperlinkTarget();
			break;
		case ID_OBJECT_EXTENDOBJECT:
			ExtendObject();
			break;
		case ID_BROWSER_RECONCILE_GUIDS:
			ReconcileGuids();
			break;
		case ID_BROWSER_FIXOOFS:
			FixOofs();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


class FindObjectDlg : public Dialog
{
public:
	FindObjectDlg(const VLB::ObjectFindInfo& fi, VLB::Connection* pConnect, VLB::VlabBrowser* pBrowser) : 
	  Dialog(IDD_FINDOBJECT), 
		  finfo(fi),
		  _pConnect(pConnect),
		  _pBrowser(pBrowser)
	{}
	void UpdateData(bool what)
	{
		DX(finfo.Name(), IDC_NAME, what);
		DXButton(finfo.MatchCase(), IDC_MATCHCASE, what);
		DXButton(finfo.WholeName(), IDC_WHOLENAME, what);
	}
	const VLB::ObjectFindInfo& FindInfo() const
	{ return finfo; }
	bool Command(int id, Window, UINT)
	{
		switch (id)
		{
		case IDC_FINDOBJ :
			UpdateData(true);
			DoFind();
			return true;
		}
		return false;
	}
private:
	void DoFind()
	{
		WaitCursor wc;
		std::string path;
		if (_pConnect->FindObject(_pBrowser->OofsRoot(), _pBrowser->CurrentObjectPath(), finfo, path))
			_pBrowser->PositionObject(path);
		else
		{
			Window w(Hdlg());
			w.MessageBox(SharedStr::GetLibString(SharedStr::strErrFindObject), finfo.Name().c_str());
		}
	}
	bool _Check()
	{
		if (finfo.Name().empty())
		{
			_CheckFailed(IDERR_EMPTYNAME, IDC_NAME);
			return false;
		}
		return true;
	}
	VLB::ObjectFindInfo finfo;
	VLB::Connection* _pConnect;
	VLB::VlabBrowser* _pBrowser;
};

void VLB::VlabBrowser::FindObject()
{
	FindObjectDlg dlg(lastFind, _pConnect.get(), this);
	dlg.DoModal(*this);
	lastFind = dlg.FindInfo();
}


void VLB::VlabBrowser::_ConnectLocal(const char* pOofs)
{
	{
		std::unique_ptr<Connection> pNew(new LocalConnection);
		_pConnect = std::move(pNew);
		_pConnect->LoadUUIDBase(pOofs);
	}
	int type = 0;
	std::string oofs(pOofs);
	if (_pConnect->HasExtensions(oofs))
		type |= Connection::tRAHasExtensions;
	std::string name;
	_pConnect->GetObjectName(pOofs,pOofs,name);
	{
		std::unique_ptr<Node> pNew(new Node(name.c_str(), oofs.c_str(), type, 0, oofs.c_str()));
		_pNode = std::move(pNew);
	}
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);
	_pNode->Measure(cnv);
	_pActive = _pNode.get();
	_pNotifySink->BrowserConnecting(this);
}


void VLB::VlabBrowser::_ConnectRemote(const CreateData* pCD)
{
	{
		std::unique_ptr<Connection> pNew
			(
			new RemoteConnection
			(pCD->_host, pCD->_user, pCD->_paswd, pCD->_crlf)
			);
		_pConnect = std::move(pNew);
		_pConnect->LoadUUIDBase(pCD->_oofs);
	}
	int type = 0;
	std::string oofs(pCD->_oofs);
	if (_pConnect->HasExtensions(oofs))
		type |= Connection::tRAHasExtensions;
	std::string name;
	_pConnect->GetObjectName( oofs, oofs, name);
	{
		std::unique_ptr<Node> pNew(new Node(name.c_str(), pCD->_oofs, type, 0, pCD->_oofs));
		_pNode = std::move(pNew);
	}
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);
	_pNode->Measure(cnv);
	_pActive = _pNode.get();
	_pNotifySink->BrowserConnecting(this);
}


void VLB::VlabBrowser::_Disconnect()
{
	if (0 != _pConnect.get())
	{
		_pNotifySink->BrowserDisconnecting(this);
		if (CanClose())
		{
			delete _pNode.release();
			{
				std::unique_ptr<Node> nl;
				_pNode = std::move(nl);
			}
			delete _pConnect.release();
			{
				std::unique_ptr<Connection> nc;
				_pConnect = std::move(nc);
			}
		}
	}
	PostClose();
}


void VLB::VlabBrowser::DoPaint(Canvas& cnv, const RECT& r)
{
	if (!_LockDrawing)
	{
		ObjectHolder sf(cnv, _font);
		MemoryCanvas mcnv(cnv);
		TextAlignment ta(cnv, TextAlignment::Bottom);
		Pen pen(Parameters::Colors[Parameters::Line]);
		ObjectHolder ph(cnv, pen);
		cnv.BkMode(Canvas::bkTransparent);
		cnv.TextColor(Parameters::Colors[Parameters::Text]);
		cnv.BkColor(Parameters::Colors[Parameters::SelectedTextBg]);
		if (0 != _pNode.get())
		{
			const POINT p = { _RootPos.x, _RootPos.y };
			_pNode->Paint(cnv, mcnv, p, &r, _pActive);
		}
	}
}


RECT VLB::VlabBrowser::_GetNodeRect(const Node* pNode) const
{
	RECT r;
	_pNode->GetRect(pNode, _RootPos, r);
	--(r.right);
	--(r.bottom);
	return r;
}


bool VLB::VlabBrowser::LButtonDown(KeyState ks, int x, int y)
{
	x += ScrollX(); 
	y += ScrollY();
	if (0 != _pNode.get())
	{
		Node::eNodePart part = Node::pNone;
		Node* pClicked = _pNode->Contains(x, y, _RootPos.x, _RootPos.y, part);
		if (0 != pClicked)
		{
			if (Node::pExtension == part)
			{
				_pActive = pClicked;
				if (ks.IsShift())
					_ShowAllExtensions();
				else 
					_ToggleExtensions();
			}
			else
			{
				UpdateCanvas cnv(Hwnd());
				ObjectHolder sf(cnv, _font);
				_SelChanged(cnv, _pActive, pClicked);
				_pActive = pClicked;
			}
		}
	}
	return true;
}



int VLB::VlabBrowser::MaxScrollX() const
{ return max(0, _pNode->TotalWidth()-Width()+ _RootPos.x); }

int VLB::VlabBrowser::MaxScrollY() const
{ return max(0, _pNode->TotalHeight()-Height()+_RootPos.y); }

int VLB::VlabBrowser::MaxX() const
{ 
	if (0 != _pNode.get())
		return _pNode->TotalWidth() + _RootPos.x;
	else
		return 0;
}

int VLB::VlabBrowser::MaxY() const
{
	if (0 != _pNode.get())
		return _pNode->TotalHeight() + _RootPos.y;
	else
		return 0;
}



void VLB::VlabBrowser::_ToggleExpand(Node* pParent, bool recursive)
{
	try
	{
		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		if (pParent->Expanded() || !recursive)
			pParent->ToggleExpand(cnv, _pConnect.get(), recursive, 0, Window(0));
		else
		{
			OnOff ld(_LockDrawing);
			LockSink ls(_pNotifySink);
			ReadingObjsDlg dlg(cnv, _pConnect.get(), pParent, IDD_READINGOBJECTS, IDC_PATH);
			dlg.DoModal(*this);
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
}


void VLB::VlabBrowser::_Expand(Node* pParent, bool recursive)
{
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);
	if (recursive)
	{
		OnOff ld(_LockDrawing);
		LockSink ls(_pNotifySink);
		ReadingObjsDlg dlg(cnv, _pConnect.get(), pParent, IDD_READINGOBJECTS, IDC_PATH);
		dlg.DoModal(*this);
	}
	else if (!pParent->Expanded())
		pParent->Expand(cnv, _pConnect.get());
}

void VLB::VlabBrowser::_ShowExtensions()
{
	if (0 != _pActive)
	{
		if (!_pActive->Expanded())
		{
			_Expand(_pActive, false);
			_pNode->RecalcTotalSize();
			{
				int maxscrx = max(0, MaxScrollX());
				if (ScrollX() > maxscrx)
					SetScrollX(maxscrx);
			}
			{
				int maxscry = max(0, MaxScrollY());
				if (ScrollY() > maxscry)
					SetScrollY(maxscry);
			}
			UpdateScrollbars();
			Invalidate();
		}
	}
	else
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
	}
}

void VLB::VlabBrowser::_ToggleExtensions()
{
	if (0 != _pActive)
	{
		_pConnect->ReloadUUIDBase();
		_ToggleExpand(_pActive, false);
		_pNode->RecalcTotalSize();
		{
			int maxscrx = max(0, MaxScrollX());
			if (ScrollX() > maxscrx)
				SetScrollX(maxscrx);
		}
		{
			int maxscry = max(0, MaxScrollY());
			if (ScrollY() > maxscry)
				SetScrollY(maxscry);
		}
		UpdateScrollbars();
		Invalidate();
		if (_pActive->Expanded())
			_SelectFirstChild();
	}
	else
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
	}
}

void VLB::VlabBrowser::_ShowAllExtensions()
{
	if (0 != _pActive)
	{
		_pConnect->ReloadUUIDBase();
		if (_pActive->Expanded())
			_ToggleExpand(_pActive, false);
		_Expand(_pActive, true);
		_pNode->RecalcTotalSize();
		{
			int maxscrx = max(0, MaxScrollX());
			if (ScrollX() > maxscrx)
				SetScrollX(maxscrx);
		}
		{
			int maxscry = max(0, MaxScrollY());
			if (ScrollY() > maxscry)
				SetScrollY(maxscry);
		}
		UpdateScrollbars();
		Invalidate();
	}
	else
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
	}
}


bool VLB::VlabBrowser::GetExtensions(const char* pth, string_buffer& res) const
{
	string_buffer Names;
	std::vector<int> Types;
	return _pConnect->GetExtensions(Names, res, Types, pth);	
}

bool VLB::VlabBrowser::GetFileList(const char* pth, string_buffer& res, bool filesonly) const
{
	return _pConnect->GetFileList(res, pth, filesonly);
}

const char* VLB::VlabBrowser::OofsRoot() const
{
	return _pNode->Path();
}

bool VLB::VlabBrowser::SupportsTar() const
{
	return _pConnect->SupportsTar();
}

bool VLB::VlabBrowser::Archive(const char* path, TmpFile& tarfile, bool recursive)
{
	return _pConnect->Archive(tarfile, _pNode->Path(), path, recursive);
}

bool VLB::VlabBrowser::LBDblClick(KeyState, int x, int y)
{
	x += ScrollX(); 
	y += ScrollY();
	if (0 != _pNode.get())
	{
		Node::eNodePart part = Node::pNone;
		Node* pClicked = _pNode->Contains(x, y, _RootPos.x, _RootPos.y, part);
		if (0 != pClicked && pClicked == _pActive)
			_OpenObject();
	}
	return true;
}

bool VLB::VlabBrowser::_LooksLikeAProject(const string_buffer& flist) const
{
	for (string_buffer::const_iterator it(flist); !it.at_end(); it.advance())
	{
		std::string file(it.str());
		if(file == std::string("LSspecifications") ||
			file == std::string("specifications"))
			return true;
		if (file.length()>2 && file.substr(file.length()-2, 2) == std::string(".l"))
			return true;
		if (file.length()>4 && file.substr(file.length()-4, 4) == std::string(".vvp"))
			return true;
	}
	return false;
}


void VLB::VlabBrowser::NativeOpenObject()
{
	std::string sourcePath;
	if(_pActive->IsHyperobject())
	{
		if(!_pConnect->LookupPath(_pActive->UUId(),OofsRoot(),sourcePath))
		{
			MessageBox(SharedStr::GetLibString(SharedStr::strErrDeadHyperlink),
				_pActive->Path());
			return;
		}
		if (sourcePath.back() == _pConnect->PathSeparator())
		{
			sourcePath.erase(sourcePath.length()-1);
		}
	}
	else // normal object
	{
		sourcePath = std::string(_pActive->Path());
	}

#ifdef FETCH_OBJECT
	WaitCursor wc;
	HWND hProject = _pNotifySink->CreateProject(sourcePath.c_str(),
		_pActive->Name(),
		&_RASink);
	_pConnect->FetchObject(sourcePath.c_str());
#else
	string_buffer FileList;
	bool res = _pConnect->GetFileList(FileList, sourcePath.c_str(), true);
	if (!res)
	{
		throw Exception(SharedStr::GetLibString(SharedStr::strErrOpenObject), sourcePath.c_str());
	}

	if (!_LooksLikeAProject(FileList))
	{
		if (!MessageYesNo(SharedStr::GetLibString(SharedStr::strYesNoNotAProject), _pActive->Name()))
			return;
	}

	WaitCursor wc;
	Window wPrj(_pNotifySink->CreateProject(sourcePath.c_str(),
		_pActive->Name(), this));
	size_t pos = FileList.begin();
	std::string fnm(sourcePath.c_str());
	while (pos != string_buffer::npos)
	{
		if ('.' != FileList.string(pos)[0] && !_ignored.contains(FileList.string(pos)))
		{
			_pConnect->AppendFName(sourcePath.c_str(),
				FileList.string(pos), fnm);
			_pConnect->GetFile(fnm.c_str());
		}
		pos = FileList.find_next(pos);
	}
#endif
	_pNotifySink->ObjectFetched(wPrj);
}

void VLB::VlabBrowser::_OpenObject()
{
	try
	{
		NativeOpenObject();
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
}


bool VLB::VlabBrowser::FetchObject(const std::string& src, const std::string& lbtbl)
{
	TmpChangeDir tcd(lbtbl);
	string_buffer FileList;
	bool res = _pConnect->GetFileList(FileList, src.c_str(), true);
	if (!res)
		return false;
	size_t pos = FileList.begin();
	std::string fnm;
	while (pos != string_buffer::npos)
	{
		if ('.' != FileList.string(pos)[0])
		{
			_pConnect->AppendFName(src.c_str(), FileList.string(pos), fnm);
			_pConnect->GetFile(fnm.c_str());
		}
		pos = FileList.find_next(pos);
	}
	return true;
}

bool VLB::VlabBrowser::PutObject(const string_buffer& flist, const std::string& lbtbl, const std::string& path)
{
	return _pConnect->PutObject(flist, lbtbl, path);
}

void VLB::VlabBrowser::_StorePlacement()
{
	RegKey key(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sWrite);
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(Hwnd(), &wp);
	key.StoreBinary(__TEXT("VLBPlcmnt"), &wp, sizeof(WINDOWPLACEMENT));
}


bool VLB::VlabBrowser::_LoadWindowPlacement(WINDOWPLACEMENT& wp)
{
	try
	{
		RegKey key(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sQueryValue);
		key.LoadBinary(__TEXT("VLBPlcmnt"), &wp, sizeof(WINDOWPLACEMENT));
	}
	catch (Exception)
	{ return false; }
	return true;
}


bool VLB::VlabBrowser::PutFile(const char* src, const char* trg)
{
	return _pConnect->PutFile(src, trg);
}

bool VLB::VlabBrowser::PutFile(const char* trgpth, const char* name, const char* rmnm)
{
	return _pConnect->PutFile(trgpth, name, rmnm);
}

bool VLB::VlabBrowser::PrototypeObject(const char* path)
{
	return _pConnect->PrototypeObject(path);
}


char VLB::VlabBrowser::PathSeparator() const
{ 
	return _pConnect->PathSeparator();
}



bool VLB::VlabBrowser::MakeExtension(std::string& location, const char* name, const char* labtbl, const string_buffer& ignored)
{
	std::string trgdir;
	bool res = MakeExtension(location.c_str(), name, labtbl, &trgdir, true, ignored);
	if (res)
	{
		location = trgdir;
		return true;
	}
	else
		return false;
}


bool VLB::VlabBrowser::MakeExtension
(const char* location, const char* name, const char* labtbl, std::string* pExtPath, bool redraw, const string_buffer& ignored)
{
	const char* lctn = location;
	if (0 == location)
		lctn = _pActive->Path();
	std::string dirname;
	if (!_pConnect->MakeExt(lctn, name, dirname))
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCreateExtension), lctn, name);
	TmpChangeDir td(labtbl);
	FindFile ff("*.*");
	while (ff.Found())
	{
		if (!ff.IsDirectory() && '.' != ff.FileName()[0] && !ignored.contains(ff.FileName().c_str()))
		{
			if (!PutFile(dirname.c_str(), ff.FileName().c_str(), ff.FileName().c_str()))
				return false;
		}
		ff.FindNext();
	}
	PrototypeObject(dirname.c_str());
	if (redraw)
	{
		{
			Node* pExtended = _pNode->Find(lctn);
			UpdateCanvas cnv(Hwnd());
			ObjectHolder sf(cnv, _font);

			if (pExtended->Expanded())
				pExtended->AddChild(name, cnv, _pConnect.get());
			else
				pExtended->ForceExpand(cnv, _pConnect.get());

			_pNode->RecalcTotalSize();
			Invalidate();
		}
		{
			Node* pNew = _pNode->Find(dirname.c_str());
			assert(pNew != 0);

			RECT r = _GetNodeRect(pNew);

			if (!IsVisible(r))
				ScrollToShow(r);

			UpdateCanvas cnv(Hwnd());
			ObjectHolder sf(cnv, _font);
			_SelChanged(cnv, _pActive, pNew);
			_pActive = pNew;
		}
	}
	if (0 != pExtPath)
		pExtPath->assign(dirname);

	return true;
}


bool VLB::VlabBrowser::CompareFiles(const char* fname, const char* path) const
{
	return _pConnect->CompareFiles(fname, path);
}



bool VLB::VlabBrowser::DeleteFile(const char* directory, const char* name)
{
	return _pConnect->DeleteFile(directory, name);
}

bool VLB::VlabBrowser::RButtonDown(KeyState ks, int x, int y)
{
	x += ScrollX(); 
	y += ScrollY();
	if (0 != _pNode.get())
	{
		Node::eNodePart part = Node::pNone;
		Node* pClicked = _pNode->Contains(x, y, _RootPos.x, _RootPos.y, part);
		if (0 != pClicked)
		{
			WaitCursor wc;
			_pActive = pClicked;
			if (_pActive->HasIcon())
				_ShowIcons(ks.IsShift(), false);
			else
				_ShowIcons(ks.IsShift(), true);
		}
	}
	return true;
}


void VLB::VlabBrowser::_ToggleIcon()
{
	if (0==_pActive)
		return;
	if (_pActive->HasIcon())
		_ShowIcons(false, false);
	else
		_ShowIcons(false, true);
}

void VLB::VlabBrowser::_ShowIcons(bool recursive, bool show)
{
	if (0==_pActive)
		return;

	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);

	if (recursive && show)
	{
		OnOff ld(_LockDrawing);
		LockSink ls(_pNotifySink);
		ReadingIcnsDlg dlg(cnv, _pConnect.get(), _pActive, _iconWidth, IDD_READINGICONS, IDC_PROGRESS);
		dlg.DoModal(*this);
	}
	else if (recursive && !show)
	{
		_pActive->ShowIcon(false, 0, cnv, true, 0, Window(0), _iconWidth);
	}
	else
		_pActive->ShowIcon(show, _pConnect.get(), cnv, false, 0, Window(0), _iconWidth);

	_pNode->RecalcTotalSize();
	{
		int maxscrx = max(0, MaxScrollX());
		if (ScrollX() > maxscrx)
			SetScrollX(maxscrx);
	}
	{
		int maxscry = max(0, MaxScrollY());
		if (ScrollY() > maxscry)
			SetScrollY(maxscry);
	}
	UpdateScrollbars();
	Invalidate();
}

void VLB::VlabBrowser::_SelChanged(Canvas& cnv, const Node* pDeact, const Node* pAct)
{
	TextAlignment ta(cnv, TextAlignment::Bottom);
	cnv.BkMode(Canvas::bkOpaque);
	cnv.TextColor(Parameters::Colors[Parameters::Text]);
	cnv.BkColor(BgColor());
	_pNode->DrawLabel(cnv, pDeact, _RootPos.x-ScrollX(), _RootPos.y-ScrollY(), false);
	cnv.BkColor(Parameters::Colors[Parameters::SelectedTextBg]);
	_pNode->DrawLabel(cnv, pAct, _RootPos.x-ScrollX(), _RootPos.y-ScrollY(), true);
}


void VLB::VlabBrowser::_DeleteObject()
{
	if (0 != _pActive)
	{
		if (_pActive == _pNode.get())
			throw Exception(SharedStr::GetLibString(SharedStr::strErrDelOofsRoot));

		int msgid;
		if(_pActive->IsHyperobject())
		{
			if(_pActive->HasExtensions())
				msgid = SharedStr::strDeleteHyperobjectAndChildren;
			else
				msgid = SharedStr::strDeleteHyperobject;
		}
		else // regular object
		{
			if(_pActive->HasExtensions())
				msgid = SharedStr::strDeleteObjectAndChildren;
			else
				msgid = SharedStr::strDeleteObject;
		}

		if (MessageYesNo(SharedStr::GetLibString(msgid), _pActive->Name()))
		{
			WaitCursor wc;
			const Node* pParent = _pActive->GetParent();
			if (_pConnect->DelTree(_pActive->Path()))
			{
				_pNode->DelSubtree(_pActive);
				_pNode->RecalcTotalSize();
				_pActive = const_cast<Node*>(pParent);
				Invalidate();
			}
			else
				throw Exception(SharedStr::GetLibString(SharedStr::strErrDelTree), _pActive->Path());
		}
	}
}


void VLB::VlabBrowser::CopyObject(bool bRecursive, bool bHypercopy)
{
	assert(!(bRecursive && bHypercopy)); // cannot hypercopy recursively
	if (!Connected())
	{
		MessageBeep(0xFFFFFFFF);
		return;
	}
	if (bHypercopy && !_pConnect->SupportsHyperCopy())
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strHypercopyNotSupported));
		return;
	}

	if (!_pConnect->SupportsCopyPaste())
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strCopyPasteNotSupported));
		return;
	}

	_pNotifySink->SetPasteSource(this, _pActive->Path(), bRecursive, bHypercopy);
}


void VLB::VlabBrowser::DoHyperPaste()
{
	assert(_pConnect->SupportsHyperCopy());
	RemoteAccess* pRA = NULL;
	std::string sourcePath;
	bool bRecursive = false;
	_pNotifySink->GetPasteSource(pRA, sourcePath, bRecursive);
	RemoteAccess* pSelf = this;
	if (pRA != pSelf)
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrHyperPasteAcrossBrowsers));
		return;
	}

	tGUID guid; 
	std::string name;

	if (_pConnect->IsHyperobject(sourcePath))
	{
		std::string realPath;
		if (!_pConnect->GetHyperobjectInfo(OofsRoot(), sourcePath, guid, realPath, name))
		{
			MessageBox(SharedStr::GetLibString(SharedStr::strErrHyperobjectLookupPath), sourcePath.c_str());
			return;
		}
	}
	else
	{
		name = sourcePath;
		size_t slash = name.find_last_of(_pConnect->PathSeparator());
		if (slash != std::string::npos)
		{
			name.erase(0, slash+1);
		}
		// Retrieve or create new GUID 
		bool bResult = _pConnect->GetUUID(OofsRoot(), sourcePath.c_str(), guid);
		if (!bResult)
		{
			MessageBox(SharedStr::GetLibString(SharedStr::strCannotGetUUIDForObject), sourcePath.c_str());
			return;
		}
	}

	// Create unique name for the extension directory
	std::string directoryName;
	bool bResult = CreateUniqueExtension(_pActive->Path(), name, directoryName);
	if (!bResult)
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrCouldNotCreateDestinationDirectory));
		return;
	}
	std::string extensionPath;
	bResult = _pConnect->MakeExt(_pActive->Path(), directoryName.c_str(), extensionPath);
	if (!bResult)
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrCouldNotCreateDestinationDirectory));
		return;
	}
	bResult = _pConnect->MakeHyperobject(extensionPath.c_str(), name.c_str(), guid);
	if (!bResult)
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrCouldNotCreateHyperobject));
		return;
	}

	{
		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		if (_pActive->Expanded())
		{
			_pActive->AddChild(directoryName.c_str(), cnv, _pConnect.get());
		}
		else
		{
			_pActive->ForceExpand(cnv, _pConnect.get());
		}
	}

}


bool VLB::VlabBrowser::CreateUniqueExtension(const char* szPath, const std::string& prefix, std::string& extensionName)
{
	string_buffer names, paths;
	std::vector<int> types;
	bool bResult = _pConnect->GetExtensions(names, paths, types, szPath);

	const int kBufferSize = 512;
	char buffer[kBufferSize];
	if (!bResult)
	{
		sprintf(buffer, "%s_h_000", prefix.c_str());
		extensionName = buffer;
		return true;
	}

	const int kMaxTries = 1000;
	extensionName.clear();
	for (int iCount=0; iCount<kMaxTries; ++iCount)
	{
		sprintf(buffer, "%s_h_%03d", prefix.c_str(), iCount);
		if (!names.contains(buffer))
		{
			extensionName = buffer;
			break;
		}
	}

	// success if created an extension name
	// failure otherwise
	return !extensionName.empty();
}

void VLB::VlabBrowser::DoRegularPaste()
{
	if(_pActive->IsHyperobject())
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrPastingObjectUnderHyperobject));
		return;
	}

	RemoteAccess* pRA = NULL;
	std::string srcpath;
	bool recursive = false;
	_pNotifySink->GetPasteSource(pRA, srcpath, recursive);
	WaitCursor wc;
	RemoteAccess* pSelf = this;
	if (pRA==pSelf)
	{
		if (!_pConnect->Paste(_pNode->Path(), srcpath.c_str(), _pActive->Path(), recursive))
		{
			if (recursive)
				MessageBox(SharedStr::GetLibString(SharedStr::strErrPasteObjects));
			else
				MessageBox(SharedStr::GetLibString(SharedStr::strErrPasteObject));
			return;
		}
	}
	else if (pRA->SupportsTar() && _pConnect->SupportsTar())
	{
		TmpFile tarfile;
		pRA->Archive(srcpath.c_str(), tarfile, recursive);
		_pConnect->Dearchive(tarfile, _pNode->Path(), srcpath.c_str(), _pActive->Path());
	}
	else
	{
		LabTable lt;
		_TransferObjects(pRA, srcpath.c_str(), _pActive->Path(), lt, recursive);
	}

	{
		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		if (_pActive->Expanded())
		{
			std::string name(srcpath);
			size_t pos = name.find_last_of(pRA->PathSeparator());
			if (pos != std::string::npos)
				name = name.substr(pos+1);
			_pActive->AddChild(name.c_str(), cnv, _pConnect.get());
		}
		else
			_pActive->ForceExpand(cnv, _pConnect.get());
	}

}


bool VLB::VlabBrowser::CanPasteNow() const
{
	if (!Connected())
		return false;
	if (!_pConnect->SupportsCopyPaste())
		return false;
	if (!_pNotifySink->IsPasteSourceSet())
		return false;

	return true;
}

void VLB::VlabBrowser::_PasteObjects()
{
	if (!CanPasteNow())
		return;

	if (_pNotifySink->IsHyperCopy())
	{
		DoHyperPaste();
	}
	else
	{
		DoRegularPaste();
	}

	_pNode->RecalcTotalSize();
	{
		int maxscrx = max(0, MaxScrollX());
		if (ScrollX() > maxscrx)
			SetScrollX(maxscrx);
	}
	{
		int maxscry = max(0, MaxScrollY());
		if (ScrollY() > maxscry)
			SetScrollY(maxscry);
	}

	UpdateScrollbars();

	Invalidate();
}


void VLB::VlabBrowser::_TransferObjects(RemoteAccess* pRA, const char* rempath, const char* lclpth, LabTable& ltbl, bool recursive)
{
	pRA->FetchObject(rempath, ltbl.Path());
	const char* name = strrchr(rempath, '/');
	if (0 == name)
		name = strrchr(rempath, '\\');
	++name;
	std::string extpth;
	string_buffer dummy_ignored;
	MakeExtension(lclpth, name, ltbl.Path(), &extpth, false, dummy_ignored);
	if (recursive)
	{
		ltbl.Clean();
		string_buffer extns;
		pRA->GetExtensions(rempath, extns);
		string_buffer::const_iterator it(extns);
		while (!it.at_end())
		{
			_TransferObjects(pRA, it.str(), extpth.c_str(), ltbl, recursive);
			it.advance();
		}
	}
}

bool VLB::VlabBrowser::KeyDown(UINT vk)
{
	if (0 == _pActive)
		return true;

	try
	{
		HandleKeyDown(vk);
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void VLB::VlabBrowser::HandleKeyDown(UINT vk)
{		
	switch (vk)
	{
	case VK_ADD :
		if (GetKeyState(VK_SHIFT)<0)
			_ShowAllExtensions();
		else
			_ToggleExtensions();
		break;
	case VK_RIGHT :
		_SelectFirstChild();
		break;
	case VK_LEFT :
		_SelectParent();
		break;
	case VK_DOWN :
		if(GetKeyState(VK_CONTROL) < 0)
			_MoveObjectDown();
		else
			_SelectNextChild();
		break;
	case VK_UP :
		if(GetKeyState(VK_CONTROL) < 0)
			_MoveObjectUp();
		else
			_SelectPrevChild();
		break;
	case VK_MULTIPLY :
		if (_pActive != 0)
		{
			if (GetKeyState(VK_SHIFT)<0)
				_ShowIcons(true, true);
			else
				_ToggleIcon();
		}
		break;
	case VK_RETURN :
		_OpenObject();
		break;
	case VK_DELETE :
		_DeleteObject();
		break;
	}
}


bool VLB::VlabBrowser::Char(char c)
{
	if (0 != _pActive)
	{
		const Node* pParent = _pActive->GetParent();
		if (0 != pParent)
		{
			Node* pChld = pParent->FindFirstChild(c);
			if (0 != pChld)
			{
				UpdateCanvas cnv(Hwnd());
				ObjectHolder sf(cnv, _font);
				_SelChanged(cnv, _pActive, pChld);
				_pActive = pChld;
			}
		}
	}
	return true;
}

void VLB::VlabBrowser::_SelectFirstChild()
{
	assert(0 != _pActive);
	Node* pA = _pActive->GetFirstChildNode();
	if (0 != pA)
	{
		RECT r = _GetNodeRect(pA);

		if (!IsVisible(r))
			ScrollToShow(r);

		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		_SelChanged(cnv, _pActive, pA);
		_pActive = pA;
	}
}


void VLB::VlabBrowser::_SelectParent()
{
	assert(0 != _pActive);
	Node* pA = const_cast<Node*>(_pActive->GetParent());
	if (0 != pA)
	{
		RECT r = _GetNodeRect(pA);

		if (!IsVisible(r))
			ScrollToShow(r);

		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		_SelChanged(cnv, _pActive, pA);
		_pActive = pA;
	}
}


void VLB::VlabBrowser::_SelectNextChild()
{
	assert(0 != _pActive);
	const Node* pParent = _pActive->GetParent();
	if (0 != pParent)
	{
		Node* pA = pParent->GetNextChild(_pActive);
		if (0 != pA)
		{
			RECT r = _GetNodeRect(pA);

			if (!IsVisible(r))
				ScrollToShow(r);

			UpdateCanvas cnv(Hwnd());
			ObjectHolder sf(cnv, _font);
			_SelChanged(cnv, _pActive, pA);
			_pActive = pA;
		}
	}
}


void VLB::VlabBrowser::_SelectPrevChild()
{
	assert(0 != _pActive);
	const Node* pParent = _pActive->GetParent();
	if (0 != pParent)
	{
		Node* pA = pParent->GetPrevChild(_pActive);
		if (0 != pA)
		{
			RECT r = _GetNodeRect(pA);

			if (!IsVisible(r))
				ScrollToShow(r);

			UpdateCanvas cnv(Hwnd());
			ObjectHolder sf(cnv, _font);
			_SelChanged(cnv, _pActive, pA);
			_pActive = pA;
		}
	}
}


void VLB::VlabBrowser::WriteOrdering(const VLB::Node* pParent, VLB::Connection* pConnect) const
{
	assert(pParent != 0);
	std::vector<std::string> names;
	for(const VLB::Node* n = pParent->GetFirstChildNode() ; n != NULL ;
		n = pParent->GetNextChild(n))
	{
		std::string path(n->Path());
		size_t pos = path.find_last_of(pConnect->PathSeparator());
		if(pos != std::string::npos)
			names.push_back(path.substr(pos+1));
		else
			names.push_back(path);
	}
	pConnect->WriteOrdering(pParent->Path(),names);
}

void VLB::VlabBrowser::_MoveObjectUp()
{
	if(_pActive == 0)
		return;

	Node* pParent = const_cast<Node*>(_pActive->GetParent());
	if(pParent == 0)
		return;

	Node* pUp = pParent->GetPrevChild(_pActive);
	if(pUp == 0)
		return;

	pParent->SwapChildren(_pActive,pUp);

	WriteOrdering(pParent,_pConnect.get());

	pParent->RecalcTotalSize();
	{
		int maxscrx = max(0, MaxScrollX());
		if (ScrollX() > maxscrx)
			SetScrollX(maxscrx);
	}
	{
		int maxscry = max(0, MaxScrollY());
		if (ScrollY() > maxscry)
			SetScrollY(maxscry);
	}

	UpdateScrollbars();

	Invalidate();
}


void VLB::VlabBrowser::_MoveObjectDown()
{
	if(_pActive == 0)
		return;

	Node* pParent = const_cast<Node*>(_pActive->GetParent());
	if(pParent == 0)
		return;

	Node* pDown = pParent->GetNextChild(_pActive);
	if(pDown == 0)
		return;

	pParent->SwapChildren(_pActive,pDown);

	WriteOrdering(pParent,_pConnect.get());

	pParent->RecalcTotalSize();
	{
		int maxscrx = max(0, MaxScrollX());
		if (ScrollX() > maxscrx)
			SetScrollX(maxscrx);
	}
	{
		int maxscry = max(0, MaxScrollY());
		if (ScrollY() > maxscry)
			SetScrollY(maxscry);
	}

	UpdateScrollbars();

	Invalidate();
}


const char* VLB::VlabBrowser::CurrentObjectPath() const
{
	assert(Connected());
	assert(0 != _pActive);
	return _pActive->Path();
}

void VLB::VlabBrowser::ApplyOptions(const Options& options)
{
	Font f(options.FontSize(), options.FontName());
	_RootPos.x = 0; _RootPos.y = options.FontSize()*2;
	_iconWidth = options.IconWidth();
	ExchangeGDIobjects(f, _font);
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);
	_pNode->RereadIcons(_pConnect.get(), cnv, options.IconWidth());
	_pNode->Measure(cnv, true);
	_pNode->RecalcTotalSize();
	SetBgColor(options.BgColor());

	{
		int maxscrx = max(0, MaxScrollX());
		if (ScrollX() > maxscrx)
			SetScrollX(maxscrx);
	}
	{
		int maxscry = max(0, MaxScrollY());
		if (ScrollY() > maxscry)
			SetScrollY(maxscry);
	}

	UpdateScrollbars();
	Invalidate();
}


void VLB::VlabBrowser::PositionObject(const std::string& path)
{
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);
	Node* pNode = _ForceLocate(path, cnv);

	_pNode->RecalcTotalSize();
	{
		int maxscrx = max(0, MaxScrollX());
		if (ScrollX() > maxscrx)
			SetScrollX(maxscrx);
	}
	{
		int maxscry = max(0, MaxScrollY());
		if (ScrollY() > maxscry)
			SetScrollY(maxscry);
	}

	UpdateScrollbars();

	if (0 != pNode)
	{
		_pActive = pNode;
		{
			RECT r = _GetNodeRect(pNode);

			if (!IsVisible(r))
				ScrollToShow(r);
		}

		Invalidate();
	}
	else
	{
		Invalidate();
		MessageBox(SharedStr::GetLibString(SharedStr::strErrLocateObject), path.c_str());
	}
}

VLB::Node* VLB::VlabBrowser::_ForceLocate(const std::string& pth, Canvas& cnv)
{
	return _pNode->ForceLocate(pth, cnv, _pConnect.get());
}


void VLB::VlabBrowser::_RenameObject()
{
	if (0 != _pActive)
	{
		RenameObjDlg dlg(IDD_RENAME, IDC_NEWNAME, _pActive->Name());
		if (IDOK == dlg.DoModal(*this))
		{
			if (_pActive->Rename(_pConnect.get(), _pNode->Path(), dlg.Name()))
			{
				UpdateCanvas cnv(Hwnd());
				ObjectHolder sf(cnv, _font);
				_pActive->Measure(cnv);
				_pNode->RecalcTotalSize();
				Invalidate();
			}
			else
			{
				MessageBox(SharedStr::GetLibString(SharedStr::strErrCannotRenameObj), _pActive->Name());
			}
		}
	}
	else
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
	}
}

void VLB::VlabBrowser::ShowHyperlinkTarget()
{
	if (NULL != _pActive && _pActive->IsHyperobject())
	{
		std::string targetPath = _pActive->GetTargetPath();
		if (targetPath.empty())
		{
			MessageBox(SharedStr::GetLibString(SharedStr::strErrHyperobjectLookupPath), _pActive->Name());
		}
		else
		{
			PositionObject(targetPath);
		}
	}
}

bool VLB::VlabBrowser::InitMenu(MenuManipulator mm)
{
	if (_pActive != 0)
	{
		if (_pActive->Expanded())
			mm.SetText(ID_VIEW_SHOWEXTENSIONS, SharedStr::GetLibString(SharedStr::strHideExtensions));
		else
			mm.SetText(ID_VIEW_SHOWEXTENSIONS, SharedStr::GetLibString(SharedStr::strShowExtensions));

		if (_pActive->HasIcon())
			mm.SetText(ID_VIEW_SHOWICON, SharedStr::GetLibString(SharedStr::strHideIcon));
		else
			mm.SetText(ID_VIEW_SHOWICON, SharedStr::GetLibString(SharedStr::strShowIcon));

		if (_pActive->HasExtensions())
		{
			mm.Enable(ID_VIEW_SHOWEXTENSIONS);
			mm.Enable(ID_VIEW_SHOWALLEXTENSIONS);
		}
		else
		{
			mm.Disable(ID_VIEW_SHOWEXTENSIONS);
			mm.Disable(ID_VIEW_SHOWALLEXTENSIONS);
		}

		if (_pActive->IsHyperobject())
		{
			mm.Enable(ID_OBJECT_SHOWHYPERLINKTARGET);
		}
		else
		{
			mm.Disable(ID_OBJECT_SHOWHYPERLINKTARGET);
		}
	}

	mm.Enable(ID_OBJECT_PASTEOBJECTS, CanPasteNow());
	return true;
}


void VLB::VlabBrowser::ExtendObject()
{
	if (0 != _pActive)
	{
		RenameObjDlg dlg(IDD_EXTENDOBJECT, IDC_NEWNAME, _pActive->Name());
		if (IDOK == dlg.DoModal(*this))
		{
			std::string extensionPath;
			if (_pConnect->MakeExt(_pActive->Path(), dlg.Name(), extensionPath))
			{
				UpdateCanvas cnv(Hwnd());
				ObjectHolder sf(cnv, _font);

				if (_pActive->Expanded())
				{
					_pActive->AddChild(dlg.Name(), cnv, _pConnect.get());
				}
				else
					_pActive->ForceExpand(cnv, _pConnect.get());

				_pActive->Measure(cnv);
				_pNode->RecalcTotalSize();
				Invalidate();
			}
			else
			{
				MessageBox(SharedStr::GetLibString(SharedStr::strErrCannotCreateObj), _pActive->Name());
			}
		}
	}
	else
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
	}
}


void VLB::VlabBrowser::ReconcileGuids()
{
	if (!_pConnect->ReconcileGuids(OofsRoot(), OofsRoot()))
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strFailedOrNotSupported));
	}
}

void VLB::VlabBrowser::FixOofs()
{
	if (!_pConnect->FixOofs())
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strFailedOrNotSupported));
	}
}
