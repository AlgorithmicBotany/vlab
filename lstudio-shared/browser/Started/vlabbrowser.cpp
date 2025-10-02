#include <string>
#include <vector>
#include <fstream>
#include <strstream>

#include <fw.h>

#include <General/autovector.h>
#include <General/onoff.h>
#include <shstrng.h>

#include "ra.h"
#include "racomm.h"
#include "socket.h"
#include "remaccess.h"
#include "labtbl.h"
#include "connection.h"
#include "localcnct.h"
#include "remotecnct.h"
#include "params.h"
#include "node.h"
#include "rdngobjsdlg.h"
#include "rdngicnsdlg.h"
#include "renameobjdlg.h"

#include "vlabbrowser.h"
#include "vlboptns.h"

#include "resids.h"


void VLB::VlabBrowser::Register(HINSTANCE hInst, WORD iconid)
{
	WndClass wc(hInst, _ClassName(), Wnd<VlabBrowser>::Proc);
	wc.style = CS_DBLCLKS;
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(iconid));
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_BROWSER);

	wc.Register();	
}


VLB::VlabBrowser::VlabBrowser(HWND hWnd, const CREATESTRUCT* pCS) : Scrollable(hWnd, pCS),
_RASink(this),
_font(reinterpret_cast<CreateData*>(pCS->lpCreateParams)->_fntsize, reinterpret_cast<CreateData*>(pCS->lpCreateParams)->_fntName),
_ignored(reinterpret_cast<CreateData*>(pCS->lpCreateParams)->_ignored)
{
	CreateData* pCD = reinterpret_cast<CreateData*>(pCS->lpCreateParams);
	_bgColor = pCD->_bgColor;
	_RootPos.x = 0;
	_RootPos.y = pCD->_fntsize*2;

	_iconWidth = Parameters::Params[Parameters::pObjIconX];
	_LockDrawing = false;
	_pActive = 0;
	_pNotifySink = pCD->_pSink;
	_silently = false;
	assert(0 != _pNotifySink);
	_pRedirector = 0;
	if (0 == pCD->_host)
		_ConnectLocal(pCD->_oofs);
	else
		_ConnectRemote(pCD);//->_host, pCD->_user, pCD->_paswd, pCD->_oofs);
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
	_pNotifySink->BrowserDisconnecting(&_RASink);
	return _RASink.CanClose();
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
			_CopySubtree(false);
			break;
		case ID_OBJECT_COPYSUBTREE :
			_CopySubtree(true);
			break;
		case ID_OBJECT_PASTEOBJECTS :
			_PasteObjects();
			break;
		case ID_OBJECT_RENAME :
			_RenameObject();
			break;

		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}



void VLB::VlabBrowser::_ConnectLocal(const char* pOofs)
{
	{
		std::auto_ptr<Connection> pNew(new LocalConnection);
		_pConnect = pNew;
	}
	int type = 0;
	std::string oofs(pOofs);
	if (_pConnect->HasExtensions(oofs.c_str()))
		type = 2;
	std::string::size_type pos = oofs.find_last_of(_pConnect->PathSeparator());
	std::string name(pos != std::string::npos ? oofs.substr(pos+1) : oofs);
	{
		std::auto_ptr<Node> pNew(new Node(name.c_str(), oofs.c_str(), type, 0));
		_pNode = pNew;
	}
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);
	_pNode->Measure(cnv);
	_pActive = _pNode.get();
	_pNotifySink->BrowserConnecting(RASink());
}


void VLB::VlabBrowser::_ConnectRemote(const CreateData* pCD)
{
	{
		std::auto_ptr<Connection> pNew
			(
			new RemoteConnection
				(pCD->_host, pCD->_user, pCD->_paswd, pCD->_crlf)
			);
		_pConnect = pNew;
	}
	int type = 0;
	if (_pConnect->HasExtensions(pCD->_oofs))
		type = 2;
	std::string oofs(pCD->_oofs);
	std::string::size_type pos = oofs.find_last_of(_pConnect->PathSeparator());
	std::string name(pos != std::string::npos ? oofs.substr(pos+1) : oofs);
	{
		std::auto_ptr<Node> pNew(new Node(name.c_str(), oofs.c_str(), type, 0));
		_pNode = pNew;
	}
	UpdateCanvas cnv(Hwnd());
	ObjectHolder sf(cnv, _font);
	_pNode->Measure(cnv);
	_pActive = _pNode.get();
	_pNotifySink->BrowserConnecting(RASink());
}


void VLB::VlabBrowser::_Disconnect()
{
	if (0 != _pConnect.get())
	{
		_pNotifySink->BrowserDisconnecting(&_RASink);
		if (_RASink.CanClose())
		{
			delete _pNode.release();
			{
				std::auto_ptr<Node> nl;
				_pNode = nl;
			}
			delete _pConnect.release();
			{
				std::auto_ptr<Connection> nc;
				_pConnect = nc;
			}
		}
	}
	PostClose();
}


bool VLB::VlabBrowser::Paint()
{
	PaintCanvas cnv(Hwnd(), _bgColor);
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
			const POINT p = { _RootPos.x-ScrollX(), _RootPos.y-ScrollY() };
			const RECT r = { 0+p.x, 0+p.y, Width()+p.x, Height()+p.y };
			_pNode->Paint(cnv, mcnv, p, &r, _pActive);
		}
	}
	return true;
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
{ return _pNode->TotalWidth()-Width()+ _RootPos.x; }

int VLB::VlabBrowser::MaxScrollY() const
{ return _pNode->TotalHeight()-Height()+_RootPos.y; }

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
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
}

void VLB::VlabBrowser::_ToggleExtensions()
{
	if (0 != _pActive)
	{
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
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
}

void VLB::VlabBrowser::_ShowAllExtensions()
{
	if (0 != _pActive)
	{
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
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
}


bool VLB::VlabBrowser::GetExtensions(const char* pth, string_buffer& res)
{
	string_buffer Names;
	std::vector<int> Types;
	return _pConnect->GetExtensions(Names, res, Types, pth);	
}

bool VLB::VlabBrowser::GetFileList(const char* pth, string_buffer& res, bool filesonly)
{
	return _pConnect->GetFileList(res, pth, filesonly);
}

bool VLB::VlabBrowser::LBDblClick(KeyState, int x, int y)
{
	x += ScrollX(); 
	y += ScrollY();
	if (0 != _pNode.get())
	{
		Node::eNodePart part = Node::pNone;
		_pNode->Contains(x, y, _RootPos.x, _RootPos.y, part);
		if (_pActive != 0 && part != Node::pExtension)
			_OpenObject();
	}
	return true;
}

bool VLB::VlabBrowser::_LooksLikeAProject(const string_buffer& flist) const
{
	for (string_buffer::const_iterator it(flist); !it.at_end(); it.advance())
	{
		std::string file(it.str());
		if (file.length()>2 && file.substr(file.length()-2, 2) == std::string(".l"))
				return true;
	}
	return false;
}


//#define FETCH_OBJECT

void VLB::VlabBrowser::NativeOpenObject()
{
#ifdef FETCH_OBJECT
	WaitCursor wc;
	HWND hProject = _pNotifySink->CreateProject(_pActive->Path(), _pActive->Name(), &_RASink);
	_pConnect->FetchObject(_pActive->Path());
#else
	string_buffer FileList;
	bool res = _pConnect->GetFileList(FileList, _pActive->Path(), true);
	if (!res)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrOpenObject), _pActive->Path());
	if (!_LooksLikeAProject(FileList))
	{
		if (!MessageYesNo(SharedStr::GetLibString(SharedStr::strYesNoNotAProject), _pActive->Name()))
			return;
	}

	WaitCursor wc;
	Window wPrj(_pNotifySink->CreateProject(_pActive->Path(), _pActive->Name(), &_RASink));
	size_t pos = FileList.begin();
	std::string fnm(_pActive->Path());
	while (pos != string_buffer::npos)
	{
		if ('.' != FileList.string(pos)[0] && !_ignored.contains(FileList.string(pos)))
		{
			_pConnect->AppendFName(_pActive->Path(), FileList.string(pos), fnm);
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
		if (_pRedirector != 0)
			_pRedirector->ObjectOpen(_pActive->Path(), _pActive->Name());
		else
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
	std::string lctn; 
	if (0 == location)
		lctn = _pActive->Path();
	else
		lctn = location;
	std::string dirname;
	if (!_pConnect->MakeExt(lctn, name, dirname))
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCreateExtension), lctn, name);
	TmpChangeDir td(labtbl);
	for (FindFile ff("*.*"); ff.Found(); ff.FindNext())
	{
		if (!ff.IsDirectory() && '.' != ff.FileName()[0] && !ignored.contains(ff.FileName()))
		{
			if (!PutFile(dirname, ff.FileName(), ff.FileName()))
				return false;
		}
	}
	if (_pConnect->RequiresIds())
	{
		_pConnect->AddId(_pNode->Path(), dirname.c_str());
	}
	PrototypeObject(dirname);
	if (redraw)
	{
		{
			Node* pExtended = _pNode->Find(lctn);
			UpdateCanvas cnv(Hwnd());
			ObjectHolder sf(cnv, _font);

			if (pExtended->Expanded())
				pExtended->AddChild(name, dirname, cnv);
			else
				pExtended->ForceExpand(cnv, _pConnect.get());

			_pNode->RecalcTotalSize();
			Invalidate();
		}
		{
			Node* pNew = _pNode->Find(dirname);
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


bool VLB::VlabBrowser::CompareFiles(const char* fname, const char* path)
{
	return _pConnect->CompareFiles(fname, path);
}

/*bool VLB::VlabBrowser::Compare(const std::string& file, const std::string& lbtbl, const std::string& path)
{
	std::string lcl(lbtbl);
	lcl.append(1, '\\');
	lcl.append(file);
	MessageBox("Comparing %s and %s", lcl.c_str(), path.c_str());
	return _pConnect->CompareFiles(lcl.c_str(), path.c_str());
}*/


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
	cnv.BkColor(_bgColor);
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
		int msgid = SharedStr::strDeleteObject;
		if (_pActive->HasExtensions())
			msgid = SharedStr::strDeleteObjectAndChildren;
		if (MessageYesNo(SharedStr::GetLibString(msgid), _pActive->Name()))
		{
			WaitCursor wc;
			const Node* pParent = _pActive->GetParent();
			if (_pConnect->DelTree(_pActive->Path()))
			{
				if (_pConnect->RequiresIds())
				{
					_pConnect->DeleteIds(_pNode->Path(), _pActive->Path());
				}
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


void VLB::VlabBrowser::_CopySubtree(bool recursive)
{
	if (!Connected())
	{
		MessageBeep(0xFFFFFFFF);
		return;
	}
	if (_pConnect->SupportsCopyPaste())
		_pNotifySink->SetPasteSource(&_RASink, _pActive->Path(), recursive);
	else
		MessageBox(SharedStr::GetLibString(SharedStr::strCopyPasteNotSupported));
}


void VLB::VlabBrowser::_PasteObjects()
{
	if (!Connected())
	{
		MessageBeep(0xFFFFFFFF);
		return;
	}
	if (!_pConnect->SupportsCopyPaste())
	{
		MessageBox(SharedStr::GetLibString(SharedStr::strCopyPasteNotSupported));
		return;
	}
	if (_pNotifySink->IsPasteSourceSet())
	{
		RemoteAccess* pRA;
		std::string path;
		bool recursive;
		_pNotifySink->GetPasteSource(pRA, path, recursive);
		WaitCursor wc;
		if (pRA==&_RASink)
		{
			if (!_pConnect->Paste(_pNode->Path(), path.c_str(), _pActive->Path(), recursive))
				MessageBox(SharedStr::GetLibString(SharedStr::strErrPasteObject));
		}
		/*else if (recursive)
		{
			OnOff ld(_LockDrawing);
			LockSink<NotifySink> ls(_pNotifySink);
			TransferObjsDlg dlg(pRA, _RASink, path, _pNode->Path());
			dlg.DoModal(*this);
		}*/
		else
		{
			LabTable lt;
			_TransferObjects(pRA, path.c_str(), _pActive->Path(), lt, recursive);
		}
		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		_pActive->ForceExpand(cnv, _pConnect.get());

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
		MessageBox(SharedStr::GetLibString(SharedStr::strErrPasteSrcNotSet));
		return;
	}
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
		_SelectNextChild();
		break;
	case VK_UP :
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

	return true;
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


const std::string& VLB::VlabBrowser::CurrentObjectPath() const
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
		MessageBox(SharedStr::GetLibString(SharedStr::strErrLocateObject), path);
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
				MessageBox(SharedStr::GetLibString(SharedStr::strErrCannotRenameObj), _pActive->Name());
		}
	}
	else
		MessageBox(SharedStr::GetLibString(SharedStr::strErrNoObjectSelect));
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
	}
	return true;
}
