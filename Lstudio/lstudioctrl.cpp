#include <vector>
#include <strstream>
#include <fstream>

#include <fw.h>
#include <glfw.h>

#include <browser/racomm.h>
#include <browser/socket.h>
#include <browser/remaccess.h>
#include <browser/labtbl.h>
#include <browser/vlabbrowser.h>
#include <browser/vlboptns.h>

#include "resource.h"

#include "lstudioapp.h"
#include "lstudioptns.h"
#include "lstudioctrl.h"
#include "params.h"
#include "prjnotifysnk.h"
#include "tedit.h"
#include "lprjctrl.h"
#include "aboutdlg.h"
#include "optionsdlg.h"
#include "viewoptnsdlg.h"
#include "choosetextfmt.h"
#include "openbrwsrdlg.h"

#include "cmndefs.h"


Window* LStudioCtrl::Create(HINSTANCE hInst, const MDIMenus& menus)
{
	LongString caption(64, IDS_PROGRAMCAPTION);
	if (options.Expired())
	{
		LongString expired(64, IDS_EXPIREDVER);
		caption = expired;
	}
	else if (PrjVar::IsEvalVer())
	{
		LongString evalv(64, IDS_EVALVER);
		caption += __TEXT(" ");
		caption += evalv;
	}

	WinMaker wm(_ClassName(), hInst);
	wm.AcceptFiles();
	wm.SetTitle(caption);
	wm.MakeOverlapped();
	wm.SetMenu(menus.GetDefaultMenu());
	Window w(wm.Create());
	if (w.IsSet())
	{
		LStudioCtrl* pCtrl = reinterpret_cast<LStudioCtrl*>(w.GetPtr());
		return pCtrl;
	}
	else
		return 0;
}

void LStudioCtrl::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), MDIWnd<LStudioCtrl>::Proc);
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_LSTUDIO));
	wc.Register();
}



LStudioCtrl::LStudioCtrl(HWND hwnd, const CREATESTRUCT* pCS) : MDICtrl(hwnd, pCS),
_pApp(dynamic_cast<LStudioApp*>(App::theApp)),
_clp(_pApp->GetCommandLineParams())
{
	if (options.Expired())
		SetTimer(eExpiredTimerId, eExpiredTimerTimeout);
	_titlePos = 0;
	_LastFolderOpened.erase();
	CurrentDirectory cd(_StartupPath);
	// Determine location of the help folder
	_tcscpy(_HelpFolder, __targv[0]);
	{
		TCHAR* bs = _tcsrchr(_HelpFolder, __TEXT('\\'));
		if (0 == bs)
			_HelpFolder[0] = 0;
		else
		{
			*bs = 0;
			bs = _tcsrchr(_HelpFolder, __TEXT('\\'));
			if (0 != bs)
			{
				++bs;
				*bs = 0;
				_tcscat(_HelpFolder, __TEXT("Help\\"));
			}
			else
				_HelpFolder[0] = 0;
		}
	}

	if (!_TryLastConfig())
		_TryDefaultConfig();
	if (PrjVar::IsExperimental())
	{
		MenuManipulator m(_pApp->GetProjectMenu());
		m.AppendCommand(1, ID_HIDDEN_DOMAGIC, "Do magic...");
	}
}


bool LStudioCtrl::ShowWindow(bool show)
{
	if (show)
	{
		static bool recursive = false;
		if (!recursive)
		{
			try
			{
				recursive = true;
				WINDOWPLACEMENT wp;
				_LoadWindowPlacement(wp);
				if (_clp.InitRect().left != CW_USEDEFAULT || _clp.InitRect().top != CW_USEDEFAULT)
					wp.rcNormalPosition = _clp.InitRect();
				if (_clp.ShowMinimized())
					wp.showCmd = SW_SHOWMINIMIZED;
				SetWindowPlacement(Hwnd(), &wp);
				recursive = false;
			}
			catch (...)
			{}
		}
	}
	return true;
}


LStudioCtrl::~LStudioCtrl()
{
	try
	{
		if (!_clp.ShowMinimized())
			_StoreWindowPlacement();
	}
	catch (Exception)
	{}
}


bool LStudioCtrl::_Command(int id)
{
	try
	{
		if (id >= static_cast<int>(LStudioApp::HelpMenuItemBaseID()))
		{
			int iHelpCommand = id - LStudioApp::HelpMenuItemBaseID();
			if (iHelpCommand >= 0 && iHelpCommand < options.GetHelpCommandCount())
			{
				_pApp->ExecuteShellCommand(options.GetHelpCommand(iHelpCommand).mCommand);
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			switch (id)
			{
			case ID_OBJECT_NEW :
				_New();
				break;
			case ID_OBJECT_EXIT :
				FORWARD_WM_CLOSE(Hwnd(), SendMessage);
				break;
			case ID_HELP_LSTUDIOMANUAL :
				_Help();
				break;
			case ID_WINDOW_TILEHORIZONTALLY :
				TileHorizontally();
				break;
			case ID_WINDOW_TILEVERTICALLY :
				TileVertically();
				break;
			case ID_WINDOWS_CASCADE :
				Cascade();
				break;
			case ID_WINDOW_CLOSEALL :
				CloseAll();
				break;
			case ID_HELP_ABOUT :
				_About();
				break;
			case ID_OBJECT_LOAD :
				_Import();
				break;
			case ID_PREFERENCES_EDIT_GENERAL :
				_Settings();
				break;
			case ID_PREFERENCES_EDIT_CURRENTEDITOR :
				_ViewOptions();
				break;
			case ID_HELP_LPFGMANUAL :
				_LpfgManual();
				break;
			case ID_HELP_CPFGMANUALS_USERSMANUAL :
				_CpfgUsersManual();
				break;
			case ID_HELP_CPFGMANUALS_ENVIRONMENTAL :
				_CpfgEnvironmentalPrograms();
				break;
			case ID_HELP_CPFGMANUALS_GRAPHICSEXT :
				_CpfgGraphicsExtensions();
				break;
			case ID_PREFERENCES_SAVETO :
				_SaveOptions();
				break;
			case ID_PREFERENCES_LOADFROM :
				_LoadOptions();
				break;
			case ID_HIDDEN_TESTARCBALL :
				// test things here
				break;
			case ID_HIDDEN_LOADARGV1 :
				_LoadArgv1();
				break;
			case ID_TOOLS_BROWSER_OPENLOCAL :
				OpenBrowserLocal();
				break;
			case ID_TOOLS_BROWSER_OPENREMOTE :
				_OpenBrowserRemote();
				break;
			default : 
				return false;
			}
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
		return false;
	}
	return true;
}



void LStudioCtrl::_New()
{
	Window wnew(_NewMDIDocument(LProjectCtrl::_ClassName(), ""));
	if (wnew.IsSet())
	{
		PostProjectCreate(wnew);
	}
	else
	{
		MessageBox(IDERR_NEWPROJECT);
	}
}



void LStudioCtrl::_About()
{
	AboutDlg about;
	about.DoModal(*this);
}


void LStudioCtrl::_StoreWindowPlacement()
{
	RegKey rg(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sWrite);
	WINDOWPLACEMENT wp;
	wp.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(Hwnd(), &wp);
	rg.StoreBinary(__TEXT("WindowPlacement"), &wp, sizeof(WINDOWPLACEMENT));
}

void LStudioCtrl::_LoadWindowPlacement(WINDOWPLACEMENT& wp)
{
	RegKey rg(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sRead);
	rg.LoadBinary(__TEXT("WindowPlacement"), &wp, sizeof(WINDOWPLACEMENT));
}


void LStudioCtrl::_Open()
{}

void LStudioCtrl::_Open(const TCHAR*)
{}


void LStudioCtrl::_OpenGallery()
{}



void LStudioCtrl::_Import()
{
	std::string path;
	if (!_LastFolderOpened.empty())
		path = _LastFolderOpened;
	else
		path = options.OofsRoot(); 
	FolderBrowser fb(Hwnd(), IDS_IMPORTBROWSETITLE, IDS_BROWSEOBJECT);
	if (fb.Browse(path))
		_Import(path);
}

void LStudioCtrl::_LoadArgv1()
{
	assert(_clp.InitialProjectSpecified());
	FindFile ff(_clp.InitialProject());
	if (LooksLikeOofs(ff))
		OpenBrowserLocal(_clp.InitialProject());
	else
		_Import(_clp.InitialProject());
}

void LStudioCtrl::_Import(const std::string& path)
{
	::SetForegroundWindow(Hwnd());
	::SetCurrentDirectory(_StartupPath.c_str());
	if (!(_LooksLikeAProject(path)))
	{
		std::string::size_type nm = path.find_last_of('\\');
		std::string name;
		if (nm == std::string::npos)
			name = path;
		else
			name = path.substr(nm+1);

		if (!MessageYesNo(IDYESNO_NOTAPROJECT, name.c_str()))
			return;
	}
	Window wnew(_NewMDIDocument(LProjectCtrl::_ClassName(), ""));
	if (!wnew.IsSet())
	{
		MessageBox(IDERR_NEWPROJECT);
		return;
	}
	LProjectCtrl* pCtrl = reinterpret_cast<LProjectCtrl*>(wnew.GetPtr());

	try
	{
		PostProjectCreate(wnew);
	}
	catch (Exception e)
	{
		MessageBox(e.Msg());
	}
	catch (...)
	{
		MessageBox("Error setting editor font");
	}

	try
	{
		pCtrl->Import(_StartupPath, path);
	}
	catch (Exception e)
	{
		MessageBox(e.Msg());
	}
	catch (...)
	{
		MessageBox("Error importing");
	}

	_LastFolderOpened = path;
}

bool LStudioCtrl::_LooksLikeAProject(const std::string& path) const
{
	TmpChangeDir tdir(path);
	{
		FindFile ff(__TEXT("specifications"));
		if (ff.Found())
			return true;
	}
	{
		FindFile ff(__TEXT("*.l"));
		if (ff.Found())
			return true;
	}
	{
		FindFile ff(__TEXT("*.vvp"));
		if (ff.Found())
			return true;
	}
	return false;
}

bool LStudioCtrl::LooksLikeOofs(const FindFile& ff) const
{
	const std::string oofs="oofs";
	if (ff.IsDirectory() && ff.FilenameIs(oofs))
		return true;
	else
		return false;
}

bool LStudioCtrl::DropFiles(HDROP hDrop)
{
	try
	{
		DroppedFiles drop(hDrop);
		std::string dropped;
		const UINT nod = drop.NumOfFiles();
		for (UINT i=0; i<nod; i++)
		{
			drop.GetFilename(i, dropped);
			FindFile ff(dropped);
			if (LooksLikeOofs(ff))
				OpenBrowserLocal(dropped);
			else if (ff.IsDirectory())
				_Import(dropped);
			else
				_ImportLink(dropped);
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void LStudioCtrl::_Help()
{
	LongString cntnts(_HelpFolder);
	cntnts += __TEXT("index.html");

	_ShellOpen(cntnts);
}


void LStudioCtrl::_ShellOpen(const TCHAR* doc)
{
	HINSTANCE hInst = 
	ShellExecute
		(
		0,
		__TEXT("open"),
		doc,
		0,
		__TEXT(".\\"),
		SW_SHOWDEFAULT
		);

	UINT res = reinterpret_cast<UINT>(hInst);
	switch (res)
	{
	case 0 :
		throw Exception(IDERR_OUTOFMEMORY);
	case ERROR_PATH_NOT_FOUND :
	case ERROR_FILE_NOT_FOUND :
		throw Exception(IDERR_FILE_NOT_FOUND, doc);
	case SE_ERR_ACCESSDENIED :
		throw Exception(IDERR_ACCESS_DENIED, doc);
	case SE_ERR_NOASSOC :
		throw Exception(IDERR_NOASSOC, doc);
	}
}


void LStudioCtrl::_CpfgUsersManual()
{
	LongString cntnts(_HelpFolder);
	cntnts += __TEXT("CPFGman.pdf");
	_ShellOpen(cntnts);
}

void LStudioCtrl::_LpfgManual()
{
	LongString cntnts(_HelpFolder);
	cntnts += __TEXT("LPFGman.pdf");

	_ShellOpen(cntnts);
}

void LStudioCtrl::_CpfgEnvironmentalPrograms()
{
	LongString cntnts(_HelpFolder);
	cntnts += __TEXT("enviro.pdf");

	_ShellOpen(cntnts);
}


void LStudioCtrl::_CpfgGraphicsExtensions()
{
	LongString cntnts(_HelpFolder);
	cntnts += __TEXT("graph.pdf");
	_ShellOpen(cntnts);
}



bool LStudioCtrl::QueryEndSession()
{ return Close(); }

bool LStudioCtrl::Close()
{
	if (!_clp.DemoMode() && options.ConfirmExit())
	{
		if (!MessageYesNo(IDS_EXITLSTUDIO))
			return false;
	}
	if (!(CloseAll()))
		return false;
	typedef std::vector<VLB::RemoteAccess*>::iterator iter;
	for (iter it = _aBrowsers.begin(); it != _aBrowsers.end(); ++it)
		(*it)->QuitSilently();
	return true;
}


void LStudioCtrl::_ImportLink(const std::string& fname)
{
	Shortcut shortcut(fname);
	std::string targetFilename;
	bool bResolved = shortcut.Resolve(targetFilename, IDERR_RESSHORTCUT);

	if (!bResolved)
	{
		throw Exception(IDERR_DONTDROPFILES);
	}

	{
		FindFile ff(targetFilename);
		if (ff.Found())
		{
			if (!(ff.IsDirectory()))
				throw Exception(IDERR_DONTDROPFILES);
		}
		else
			throw Exception(IDERR_SRTCTMSNGFILE, targetFilename);
	}
	_Import(targetFilename);
}


void LStudioCtrl::_SelectEditorFont()
{
	LOGFONT origlf = options.GetLogFont();
	COLORREF origbg = options.GetTextEditBgColor();
	COLORREF origfg = options.GetTextEditTxtColor();

	ChooseTextFormat ctf
		(
		options.GetLogFont(), 
		options.GetTextEditBgColor(), 
		options.GetTextEditTxtColor()
		);
	ctf.SetApplyCallback(LStudioCtrl::_SetEditorFontApplyClbck, this);

	if (IDOK == ctf.DoModal(*this))
	{
		options.SetEditorFont(ctf.GetLogFont());
		options.SetTextEditBgColor(ctf.GetBgColor());
		options.SetTextEditTxtColor(ctf.GetFgColor());
	}
	else
	{
		options.SetEditorFont(origlf);
		options.SetTextEditBgColor(origbg);
		options.SetTextEditTxtColor(origfg);
	}

	ChildEnumerator<LStudioCtrl> ce(&LStudioCtrl::_SetFontProc, this);
	ce.Enumerate(GetMDIClient().Hwnd());
}

bool LStudioCtrl::_SetFontProc(HWND hWnd, char*)
{
	Window w(hWnd);
	if (w.WindowClass() == LProjectCtrl::WndAtom())
	{
		LProjectCtrl* pCtrl = GetWinLong<LProjectCtrl*>(hWnd);
		pCtrl->SetEditorFont(options.EditorFont());
	}
	return true;
}


bool LStudioCtrl::_VLBrowserDisconnecting(HWND hWnd, VLB::RemoteAccess* pRA)
{
	Window w(hWnd);
	if (w.WindowClass()==LProjectCtrl::WndAtom())
	{
		LProjectCtrl* pCtrl = GetWinLong<LProjectCtrl*>(hWnd);
		if (!pCtrl->BrowserDisconnecting(pRA))
			return false;
	}
	return true;
}





void LStudioCtrl::_SetEditorFontApplyClbck(const LOGFONT& lf, COLORREF fg, COLORREF bg, void* pV)
{
	LStudioCtrl* pCtrl = reinterpret_cast<LStudioCtrl*>(pV);
	pCtrl->_DoSetEditorFontApply(lf, fg, bg);
}


void LStudioCtrl::_DoSetEditorFontApply(const LOGFONT& lf, COLORREF fg, COLORREF bg)
{
	options.SetEditorFont(lf);
	options.SetTextEditTxtColor(fg);
	options.SetTextEditBgColor(bg);

	ChildEnumerator<LStudioCtrl> ec(&LStudioCtrl::_SetFontProc, this);
	ec.Enumerate(GetMDIClient().Hwnd());
}



void LStudioCtrl::_ViewOptions()
{
	const LProjectCtrl* pProject = LProjectCtrl::GetActiveProject();
	if (0 != pProject)
	{
		switch (pProject->ProjectMode())
		{
		case Specifications::mLsystem :
		case Specifications::mView :
		case Specifications::mDescription :
		case Specifications::mAnyText :
			_SelectEditorFont();
			break;
		case Specifications::mSurface:
		case Specifications::mContours:
		case Specifications::mFunctions:
			_GridViewOptions();
			break;
		case Specifications::mPanels:
		case Specifications::mColors:
		case Specifications::mAnimate:
			MessageBox(IDERR_NOTHINGTOCUSTOMIZE);
		}
	}
	else
		MessageBox(IDERR_NOACTIVEPROJECT);
}


bool LStudioCtrl::InitMenu(MenuManipulator mm)
{
	const LProjectCtrl* pProject = LProjectCtrl::GetActiveProject();
	if (0 != pProject)
		pProject->AdjustMenu(mm);
	AdjustMenu(mm);
	return true;
}


void LStudioCtrl::AdjustMenu(MenuManipulator&)
{
	/*if (options.HideEmptyTabs())
		mm.Check(ID_PREFERENCES_HIDEUNUSEDTABS);
	else
		mm.Uncheck(ID_PREFERENCES_HIDEUNUSEDTABS);*/
}


void LStudioCtrl::_GridViewOptions()
{
	COLORREF gc[Options::eGridViewEntryCount];
	for (int i=0; i<Options::eGridViewEntryCount; i++)
		gc[i] = options.GetGridColors()[i];

	float gw[Options::wGridWidthEntryCount];
	for (int i=0; i<Options::wGridWidthEntryCount; i++)
		gw[i] = options.GetGridWidths()[i];

	ViewOptionsDlg dlg(options);
	dlg.SetApplyCallback(LStudioCtrl::_SetGridViewOptApplyClbck, this);
	if (IDOK==dlg.DoModal(*this))
	{
		options.SetGridColors(dlg.GetGridColors());
		options.SetGridWidths(dlg.GetGridWidths());
	}
	else
	{
		options.SetGridColors(gc);
		options.SetGridWidths(gw);
	}

	ChildEnumerator<LStudioCtrl> ce(&LStudioCtrl::_SetViewOptionsProc, this);
	ce.Enumerate(GetMDIClient().Hwnd());
}

void LStudioCtrl::_SetGridViewOptApplyClbck(const COLORREF* clr, const float* aW, void* pV)
{
	LStudioCtrl* pCtrl = reinterpret_cast<LStudioCtrl*>(pV);
	pCtrl->_SetGridViewOptApply(clr, aW);
}


void LStudioCtrl::_SetGridViewOptApply(const COLORREF* clr, const float* aW)
{
	options.SetGridColors(clr);
	options.SetGridWidths(aW);
	ChildEnumerator<LStudioCtrl> ce(&LStudioCtrl::_SetViewOptionsProc, this);
	ce.Enumerate(GetMDIClient().Hwnd());
}

bool LStudioCtrl::_SetViewOptionsProc(HWND hWnd, char*)
{
	ATOM atom = static_cast<ATOM>(GetClassLong(hWnd, GCW_ATOM));
	if (atom == LProjectCtrl::WndAtom())
	{
		LProjectCtrl* pCtrl = GetWinLong<LProjectCtrl*>(hWnd);
		pCtrl->SetViewOptions(options);
	}
	return true;
}


void LStudioCtrl::_Settings()
{
	OptionsDlg dlg(options);
	if (IDOK==dlg.DoModal(*this))
	{
		dlg.UpdateOptions(options);

		ChildEnumerator<LStudioCtrl> ce(&LStudioCtrl::_SetViewOptionsProc, this);
		ce.Enumerate(GetMDIClient().Hwnd());
	}
}

void LStudioCtrl::_SaveOptions() 
{
	std::string fname;
	OpenFilename ofn(Hwnd(), IDS_CONFIGFILTER, __TEXT("cfg"));

	try
	{
		RegKey key(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sQueryValue);
		key.SetIgnore(true);
		if (key.LoadString(__TEXT("LastConfig"), fname))
			ofn.SetDefault(fname);
	}
	catch (...) {}

	ofn.SaveSecurity();
	if (ofn.Save())
		_SaveOptions(ofn.Filename());
}

void LStudioCtrl::_SaveOptions(const std::string& fname)
{
	std::ofstream trg(fname);
	if (!trg.is_open())
		throw Exception(IDERR_WRITECONFIG, fname.c_str());
	options.Save(trg);
	try
	{
		RegKey key(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sSetValue);
		key.SetIgnore(true);
		key.StoreString(__TEXT("LastConfig"), fname);
	}
	catch (...) {}
}


void LStudioCtrl::_LoadOptions()
{
	std::string fname;
	OpenFilename ofn(Hwnd(), IDS_CONFIGFILTER, __TEXT("cfg"));

	try
	{
		RegKey key(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sQueryValue);
		key.SetIgnore(true);
		if (key.LoadString(__TEXT("LastConfig"), fname))
			ofn.SetDefault(fname);
	}
	catch (...) {}

	ofn.OpenSecurity();
	if (ofn.Open())
		_LoadOptions(ofn.Filename());
}


void LStudioCtrl::_LoadOptions(const std::string& fname)
{
	WaitCursor wc;
	ReadTextFile src(fname);
	options.Load(src);

	try
	{
		RegKey key(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sSetValue);
		key.SetIgnore(true);
		key.StoreString(__TEXT("LastConfig"), fname);
	}
	catch (...) {}

	{
		ChildEnumerator<LStudioCtrl> ce(&LStudioCtrl::_SetViewOptionsProc, this);
		ce.Enumerate(GetMDIClient().Hwnd());
	}
	{
		ChildEnumerator<LStudioCtrl> ce(&LStudioCtrl::_SetFontProc, this);
		ce.Enumerate(GetMDIClient().Hwnd());
	}

	{
		VLB::Options voptions(options.BrowserFontSize(), options.GetBrowserFontName(), options.IconWidth());
		typedef std::vector<VLB::RemoteAccess*>::iterator iter;
		for (iter i = _aBrowsers.begin(); i != _aBrowsers.end(); ++i)
			(*i)->ApplyOptions(voptions);
	}

	UpdateHelpMenu(options);
}

void LStudioCtrl::GetLastConfig(std::string& fname) const
{
	RegKey key(HKEY_CURRENT_USER, __TEXT("Software\\RadekSoftware\\LStudio"), RegKey::sQueryValue);
	key.SetIgnore(true);
	key.LoadString(__TEXT("LastConfig"), fname);
}

void LStudioCtrl::GetDefaultConfig(std::string& fname) const
{
	const int BfSize = 1024;
	std::string flnm; flnm.resize(BfSize);
	int sz = GetModuleFileName(0, &(flnm[0]), BfSize);
	flnm.resize(sz);
	std::string::size_type bsl = flnm.find_last_of('\\');
	if (std::string::npos != bsl)
	{
		flnm.erase(bsl);
		bsl = flnm.find_last_of('\\');
		flnm.erase(bsl+1);
		flnm.append("lstudio.cfg");
	}
	else
		fname.erase();
}

bool LStudioCtrl::_TryLastConfig()
{
	std::string fname;
	GetLastConfig(fname);
	if (!fname.empty())
	{
		try
		{			
			_LoadOptions(fname);
			return true;
		}
		catch (...)
		{}
	}
	return false;
}

void LStudioCtrl::_TryDefaultConfig()
{
	std::string fname;
	GetDefaultConfig(fname);
	if (!fname.empty())
	{
		try
		{
			_LoadOptions(fname);
		}
		catch (...)
		{}
	}
}


void LStudioCtrl::OpenBrowserLocal()
{
	std::string fname;
	GetLastConfig(fname);
	if (fname.empty())
		GetDefaultConfig(fname);

	OpenBrowserLocalDlg dlg(options, fname);
	if (IDOK == dlg.DoModal(*this))
	{
		OpenBrowserLocal(dlg.Oofs());
	}
}

void LStudioCtrl::OpenBrowserLocal(const std::string& oofs)
{
	options.SetOofsRoot(oofs);
	WaitCursor wc;
	VLB::VlabBrowser* pBrowser = VLB::VlabBrowser::Create
		(
		_pApp->GetInstance(), this, options.OofsRoot(),
		options.IgnoredFiles()
		);
	VLB::Options voptions(options.BrowserFontSize(), options.GetBrowserFontName(), options.IconWidth());
	pBrowser->ApplyOptions(voptions);
}




void LStudioCtrl::_OpenBrowserRemote()
{
	std::string fname;
	GetLastConfig(fname);
	if (fname.empty())
		GetDefaultConfig(fname);

	OpenBrowserRemoteDlg dlg(options, fname);
	if (IDOK == dlg.DoModal(*this))
	{
		dlg.SetDefaults(options);
		WaitCursor wc;
		VLB::VlabBrowser* pBrowser = VLB::VlabBrowser::Create
			(
			_pApp->GetInstance(), this, 
			options.Host(),
			options.User(),
			options.Password(),
			options.Oofs(),
			options.ConvertCRLF(),
			options.IgnoredFiles()
			);
		if (0 != pBrowser)
		{
			VLB::Options voptions(options.BrowserFontSize(), options.GetBrowserFontName(), options.IconWidth());
			pBrowser->ApplyOptions(voptions);
		}
	}
}


void LStudioCtrl::BrowserConnecting(VLB::RemoteAccess* pRA)
{
	_aBrowsers.push_back(pRA);
}

bool LStudioCtrl::BrowserDisconnecting(VLB::RemoteAccess* pRA)
{
	if (pRA == _pasteSrc.RA())
		ClearPasteSource();
	ChildEnumerator<LStudioCtrl, VLB::RemoteAccess> ce(&LStudioCtrl::_VLBrowserDisconnecting, this, pRA);
	ce.Enumerate(GetMDIClient().Hwnd());
	if (pRA->CanClose())
	{
		std::vector<VLB::RemoteAccess*>::iterator it;
		for (it = _aBrowsers.begin(); it != _aBrowsers.end() && *it != pRA; ++it)
			;

		if (it != _aBrowsers.end())
			_aBrowsers.erase(it);
		return true;
	}
	else
		return false;
}

Window LStudioCtrl::CreateProject(const std::string& path, const std::string& name, VLB::RemoteAccess* pAccess)
{
	LProjectCtrl::CreateParams cp(path, name, pAccess);
	Window wnew(_NewMDIDocument(LProjectCtrl::_ClassName(), "", &cp));
	if (!wnew.IsSet())
	{
		throw Exception(IDERR_NEWPROJECT);
	}
	else
	{
		PostProjectCreate(wnew);
	}
	return wnew;
}


void LStudioCtrl::PostProjectCreate(Window& window)
{
	LProjectCtrl* pCtrl = reinterpret_cast<LProjectCtrl*>(window.GetPtr());
	pCtrl->SetEditorFont(options.EditorFont());
	MenuManipulator projectHelpMenu(pCtrl->GetProjectHelpMenu());
	_pApp->UpdateHelpMenu(projectHelpMenu, options);
}

void LStudioCtrl::ObjectFetched(Window project)
{
	LProjectCtrl* pCtrl = reinterpret_cast<LProjectCtrl*>(project.GetPtr());
	pCtrl->LoadObjectToEditors();
	if (pCtrl->AutoRun())
		pCtrl->Go();
}



void LStudioCtrl::SetPasteSource(VLB::RemoteAccess* pRA, const std::string& path, bool bRecursive, bool bHyperCopy)
{
	_pasteSrc.Set(pRA, path, bRecursive, bHyperCopy);
}

bool LStudioCtrl::IsPasteSourceSet() const
{
	return _pasteSrc.IsSet();
}

void LStudioCtrl::GetPasteSource(VLB::RemoteAccess*& pRA, std::string& path, bool& recursive) const
{
	_pasteSrc.Get(pRA, path, recursive);
}


const char* Ttl =
{
	"This is an evaluation copy of L-studio. "
	"Its evaluation period has expired. "
	"Please support our work by purchasing "
	"the full version of this software..."
};


bool LStudioCtrl::Timer(UINT)
{
	if (Ttl[_titlePos] == 0)
		_titlePos = 0;
	const int BfSize = 36;
	static char bf[BfSize+1];
	strncpy(bf, Ttl+_titlePos, BfSize);
	bf[BfSize] = 0;
	SetStatusText(bf);
	++_titlePos;
	return true;
}


void LStudioCtrl::UpdateHelpMenu(const LStudioOptions& options)
{
	MenuManipulator defaultMenu(_pApp->GetDefaultMenu());
	MenuManipulator defaultHelpMenu(defaultMenu.GetSubMenu(_pApp->GetDefaultHelpMenuPosition()));
	_pApp->UpdateHelpMenu(defaultHelpMenu, options);

	ChildCEnumerator<LStudioCtrl, LStudioOptions> ce(&LStudioCtrl::UpdateHelpMenuProc, this, &options);
	ce.Enumerate(GetMDIClient().Hwnd());
}

bool LStudioCtrl::UpdateHelpMenuProc(HWND hWnd, const LStudioOptions* pOptions)
{
	Window w(hWnd);
	if (w.WindowClass() == LProjectCtrl::WndAtom())
	{
		LProjectCtrl* pCtrl = GetWinLong<LProjectCtrl*>(hWnd);
		MenuManipulator projectHelpMenu(pCtrl->GetProjectHelpMenu());
		_pApp->UpdateHelpMenu(projectHelpMenu, *pOptions);
	}
	return true;
}


