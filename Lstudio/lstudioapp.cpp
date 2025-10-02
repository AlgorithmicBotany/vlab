#include <memory>
#include <vector>

#include <fw.h>
#include <glfw.h>

#include "resource.h"


#include <browser/racomm.h>
#include <browser/socket.h>
#include <browser/remaccess.h>
#include <browser/labtbl.h>

#include <browser/vlabbrowser.h>

#include "lstudioapp.h"
#include "lstudioptns.h"
#include "lstudioctrl.h"
#include "banner.h"

#include "tedit.h"
#include "params.h"
#include "prjnotifysnk.h"
#include "lprjctrl.h"
#include "stdout.h"
#include "colormapedit.h"
#include "colormap.h"
#include "colormapwnd.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "glgallery.h"

#include "matpreview.h"
#include "matgallery.h"
#include "patchclrinfo.h"
#include "patch.h"
#include "surface.h"
#include "surfviewtsk.h"
#include "surfaceview.h"
#include "curveview.h"
#include "curvegallery.h"
#include "thumbtask.h"
#include "linethcb.h"
#include "linethumb.h"
#include "surfgallery.h"
#include "contour.h"
#include "contourgallery.h"
#include "gridviewtask.h"
#include "gridview.h"
#include "contourvwtsk.h"
#include "contourview.h"
#include "traditional.h"
#include "funcgallery.h"
#include "functionview.h"
#include "funcpts.h"
#include "function.h"

#include "evalverdlg.h"

#include "panelprms.h"
#include "panelgallery.h"
#include "panelvwtsk.h"
#include "pnlitmsel.h"
#include "panelview.h"
#include "panelitem.h"
#include "panelctrl.h"
#include "gridpreview.h"
#include "drpdwnpalette.h"
#include "snapicon.h"
#include "cmndefs.h"


static const UINT MenuIds[] =
{
	IDR_DEFAULT,
		IDR_PROJECT
};

static const int WinMenuIdx[] =
{
	0,
		4
};

static const int MenuCount = CountOf(MenuIds);

static const UINT bmps[] =
{
	IDB_LSYSTEM, IDB_LSYSTEMMASK, 
	IDB_VIEW, IDB_VIEWMASK,
	IDB_ANIM, IDB_ANIMMASK,
	IDB_COLORS, IDB_COLORSMASK,
	IDB_SURFACE, IDB_SURFACEMASK,
	IDB_CONTOUR, IDB_CONTOURMASK,
	IDB_FUNCTION, IDB_FUNCTIONMASK,
	IDB_PANEL, IDB_PANELMASK,
	IDB_DESCRIPTION, IDB_DESCRIPTIONMASK,
	IDB_TEXTFILE, IDB_TEXTFILEMASK
};

const MDIMenus::InitData lapp[] =
{
	{ IDR_DEFAULT, 0, 3 },
	{ IDR_PROJECT, 4, 5 }
};

static const UINT kHelpMenuItemBaseID = 45000;

LStudioApp::LStudioApp(HINSTANCE hInst) :
MDIApp(hInst, lapp, CountOf(lapp), IDR_CONTEXTMENUS),
_TabsList(16, 16, ILC_COLOR4 | ILC_MASK, 10, 10),
_RichEdDll("Riched20.dll")
{
	_TabsList.Build(GetInstance(), bmps, CountOf(bmps));
	if (PrjVar::IsEvalVer())
		_CheckEval();

	LoadAccelerators(MAKEINTRESOURCE(IDR_ACCELERATOR1));
	_RegisterClasses();

	// Add Lstudio.bin and Enviro.bin to PATH

	static TCHAR path[32768];
	static TCHAR shrtpth[256];
	static TCHAR argv0[_MAX_PATH+1];
	DWORD res = GetEnvironmentVariable(__TEXT("PATH"), path, 32768);
	path[res] = 0;

	GetModuleFileName(GetInstance(), argv0, _MAX_PATH+1);
	GetShortPathName(argv0, shrtpth, 256);
	_tcscpy(argv0, shrtpth);

	TCHAR* backslash = _tcsrchr(argv0, __TEXT('\\'));
	if (0 != backslash)
	{
		*backslash = 0;
		_tcscat(path, __TEXT(";"));
		_tcscat(path, argv0);
		_tcscat(argv0, __TEXT("\\Enviro.bin"));
		_tcscat(path, __TEXT(";"));
		_tcscat(path, argv0);
		SetEnvironmentVariable(__TEXT("PATH"), path);
		*backslash = 0;
		backslash = _tcsrchr(argv0, __TEXT('\\'));
		if (0 != backslash)
		{
			backslash++;
			_tcscpy(backslash, __TEXT("lpfg"));
			SetEnvironmentVariable(__TEXT("LPFGPATH"), argv0);
		}
	}

	_simulatorMenus.Load(IDR_SIMULATORMENU);

	try
	{
		_Clp.Parse();
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
}


LStudioApp::~LStudioApp()
{
	_CleanTmp();
#if TEST_ISCLEAN
	assert(LSystemEdit::IsClean());
	assert(ViewEdit::IsClean());
	assert(StdOut::IsClean());
	assert(ColormapEdit::IsClean());
	assert(ColormapWnd::IsClean());
	assert(MaterialPreview::IsClean());
	assert(MaterialGallery::IsClean());
	assert(EditableObject::IsClean());
	assert(TraditionalSurface::IsClean());
	assert(Contour::IsClean());
	assert(ObjectView::IsClean());
	assert(PanelItem::IsClean());
	assert(PanelItemCtrl::IsClean());
	assert(PickColorModeless::IsClean());
	assert(Function::IsClean());
#endif /* TEST_ISCLEAN */
	assert(TraditionalPatch::IsClean());
}


void LStudioApp::Create()
{
	if (_Clp.InitialProjectSpecified() || _Clp.SkipSplash() && !options.Expired())
	{
		SetMain(LStudioCtrl::Create(GetInstance(), _GetMDIMenus()));
		_PostCreate();
		LaunchMainWindow();
		if (_Clp.InitialProjectSpecified())
			WMain()->PostCommand(ID_HIDDEN_LOADARGV1);
	}
	else
	{
		SetMain(LStudioCtrl::Create(GetInstance(), _GetMDIMenus()));
		Banner::Create(GetInstance(), WMain()->Hwnd());
		_PostCreate();
	}
}


void LStudioApp::LaunchMainWindow()
{
	_Show(Window::swShowNormal);
}


void LStudioApp::_RegisterClasses()
{
	LStudioCtrl::Register(GetInstance());
	LProjectCtrl::Register(GetInstance());
	ColormapWnd::Register(GetInstance());
	MaterialPreview::Register(GetInstance());
	MaterialGallery::Register(GetInstance());
	SurfaceView::Register(GetInstance());
	LineThumb::Register(GetInstance());
	SurfaceGallery::Register(GetInstance());
	ContourView::Register(GetInstance());
	ContourGallery::Register(GetInstance());
	Banner::Register(GetInstance());
	FuncGallery::Register(GetInstance());
	FunctionView::Register(GetInstance());
	CurveView::Register(GetInstance());
	CurveGallery::Register(GetInstance());
	PanelGallery::Register(GetInstance());
	PanelView::Register(GetInstance());
	PanelCtrl::Register(GetInstance());
	GridPreview::Register(GetInstance());
	DropDownPalette::Register(GetInstance());
	VLB::VlabBrowser::Register(GetInstance(), IDI_BROWSER);
}


void LStudioApp::_CleanTmp()
{
	try
	{
		TmpChangeDir tmp;
		{
			FindFile ff(__TEXT("cpfg.*"));
			while (ff.Found())
			{
				if (!(ff.IsDirectory()))
				{
					if (ff.FileName().compare("cpfg.exe"))
						::DeleteFile(ff.FileName().c_str());
				}
				ff.FindNext();
			}
		}
	}
	catch (...)
	{}
}


void LStudioApp::_CheckEval()
{
	SYSTEMTIME stToday, stExpire;
	{
		stExpire.wYear = PrjVar::EvalYear;
		stExpire.wMonth = PrjVar::EvalMnth;
		stExpire.wDayOfWeek = 0;
		stExpire.wDay = PrjVar::EvalDay;
		stExpire.wHour = 1;
		stExpire.wMinute = 0;
		stExpire.wSecond = 0;
		stExpire.wMilliseconds = 0;
	}
	GetLocalTime(&stToday);

	{
		FILETIME ftToday, ftExpire;
		SystemTimeToFileTime(&stToday, &ftToday);
		SystemTimeToFileTime(&stExpire, &ftExpire);
		if (1 == CompareFileTime(&ftToday, &ftExpire))
		{
			options.Expired(true);
			MessageBox(0, "This evaluation version of L-studio has expired", "L-studio demo", MB_ICONWARNING);
			return;
		}
	}

	EvalVerDlg evaldlg(stExpire);
	evaldlg.DoModal();
}


int LStudioApp::GetDefaultHelpMenuPosition() const
{
	return lapp[eDefaultMenu].helpMenuPosition;
}

int LStudioApp::GetProjectHelpMenuPosition() const
{
	return lapp[eProjectMenu].helpMenuPosition;
}

UINT LStudioApp::HelpMenuItemBaseID()
{ 
	return kHelpMenuItemBaseID;
}


void LStudioApp::UpdateHelpMenu(MenuManipulator& helpMenu, const LStudioOptions& options)
{
	int iCurrentItem = 0;
	bool deleted = false;
	while (iCurrentItem<helpMenu.ItemCount())
	{
		MenuManipulator::MIInfo mii;
		mii.SetMask(MIIM_ID | MIIM_FTYPE | MIIM_SUBMENU);
		helpMenu.GetMenuItemInfo(iCurrentItem, mii);
		if (mii.IsCommand() && mii.ID() >= LStudioApp::HelpMenuItemBaseID())
		{
			helpMenu.DeleteItem(iCurrentItem);
			deleted = true;
		}
		else
		{
			++iCurrentItem;
		}
	}

	UINT iMenuItemPosition = helpMenu.FindItem(ID_HELP_ABOUT);
	if (iMenuItemPosition == MenuManipulator::kInvalidId)
		iMenuItemPosition = helpMenu.ItemCount()-1;
	for (int iCommand=0; iCommand<options.GetHelpCommandCount(); ++iCommand)
	{
		helpMenu.InsertCommand(iMenuItemPosition+iCommand, LStudioApp::HelpMenuItemBaseID() + iCommand, options.GetHelpCommand(iCommand).mLabel);
	}
}

bool LStudioApp::ExecuteShellCommand(const std::string& command) const
{
	const int kBufferSize = 1024;
	TCHAR expandedString[kBufferSize];
	::ExpandEnvironmentStrings(command.c_str(), expandedString, kBufferSize);
	HINSTANCE hInst = ::ShellExecute(NULL, NULL, expandedString, NULL, NULL, SW_SHOWDEFAULT);
	int iResult = reinterpret_cast<int>(hInst);
	DWORD err = ::GetLastError();
	return (iResult>32) && (0==err);
}

bool LStudioApp::ExecuteCommand(const std::string& command) const
{
	const int kBufferSize = 1024;
	TCHAR expandedString[kBufferSize];
	::ExpandEnvironmentStrings(command.c_str(), expandedString, kBufferSize);
	NewProcess process(expandedString, IDERR_SIMULATORCOMMAND);
	return true;
}

