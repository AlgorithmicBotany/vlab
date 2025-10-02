#include <vector>
#include <sstream>
#include <iomanip>

#include <fw.h>
#include <glfw.h>

#include <browser/remaccess.h>

#include "resource.h"

#include "tedit.h"
#include "params.h"
#include "prjnotifysnk.h"
#include "lprjctrl.h"
#include "animdata.h"
#include "animedit.h"
#include "stdout.h"
#include "colormapedit.h"
#include "matparamcb.h"
#include "lstudioptns.h"
#include "linethcb.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"

#include "contmodedit.h"

#include "materialedit.h"
#include "surfthumbcb.h"
#include "surfaceedit.h"
#include "contouredit.h"
#include "funcedit.h"
#include "curveedit.h"
#include "panelsedit.h"
#include "lstudioapp.h"
#include "difflist.h"
#include "waitsmphrdlg.h"

#include "snapicon.h"
#include "cmndefs.h"


int LProjectCtrl::_counter = 0;
ATOM LProjectCtrl::_WndAtom = 0;
const ObjectEdit* LProjectCtrl::pAllEditors = reinterpret_cast<const ObjectEdit*>(0xFFFFFFFF);
const UINT kSimulatorMenuId = 1;
const UINT kHelpMenuId = 5;
LProjectCtrl* LProjectCtrl::_pActiveProject = 0;


static const UINT Labels[] =
{
	IDS_LSYSTEM,
	IDS_VIEW,
	IDS_ANIMATE,
	IDS_COLORS,
	IDS_SURFACES,
	IDS_CONTOURS,
	IDS_FUNCTIONS,
	IDS_PANELS,
	IDS_DESCRIPTION,
	IDS_ANYTEXT
};

static const int NumOfLabels = CountOf(Labels);

static const MenuManipulator::tMenuBuildData aCpfgSimulatorMenuData[] =
{
	{ "&Go\tCtrl+G",								ID_SIMULATION_GO },
	{ "&Kill",										ID_SIMULATION_STOP },
	{ MenuManipulator::tMenuBuildData::kSeparatorTag, 0 },
	{ Specifications::kContinuousModelingLabel,		Specifications::kContinuousModelingCommandId },
	{ "New &model",									ID_SIMULATION_NEWMODEL },
	{ "New &L-system",								ID_SIMULATION_NEWLSYSTEM },
	{ "New &homomorphism",							ID_SIMULATION_NEWHOMOMORPHISM },
	{ "New &view",									ID_SIMULATION_NEWVIEW },
	{ "New &projection",							ID_SIMULATION_NEWPROJECTION },
	{ "New ren&der",								ID_SIMULATION_NEWRENDER },
	{ "New &animate",								ID_SIMULATION_NEWANIMATE },
	{ "&Rerun",										ID_SIMULATION_RERUN },

	{ MenuManipulator::tMenuBuildData::kLastItemTag, 0 } // terminating item
};

static const MenuManipulator::tMenuBuildData aLpfgSimulatorMenuData[] =
{
	{ "&Go\tCtrl+G",								ID_SIMULATION_GO },
	{ "&Kill",										ID_SIMULATION_STOP },
	{ MenuManipulator::tMenuBuildData::kSeparatorTag, 0 },
	{ Specifications::kContinuousModelingLabel,		Specifications::kContinuousModelingCommandId },
	{ "New &model",									ID_SIMULATION_NEWMODEL },
	{ "New &L-system",								ID_SIMULATION_NEWLSYSTEM },
	{ "New &view",									ID_SIMULATION_NEWVIEW },
	{ "New &projection",							ID_SIMULATION_NEWPROJECTION },
	{ "New &animate",								ID_SIMULATION_NEWANIMATE },
	{ "&Rerun",										ID_SIMULATION_RERUN },

	{ MenuManipulator::tMenuBuildData::kLastItemTag, 0 } // terminating item
};

static const MenuManipulator::tMenuBuildData aVVSimulatorMenuData[] =
{
	{ "&Go\tCtrl+G",								ID_SIMULATION_GO },
	{ "&Build",										ID_SIMULATION_BUILD },
	{ "&Kill",										ID_SIMULATION_STOP },
	{ MenuManipulator::tMenuBuildData::kSeparatorTag, 0 },
	{ Specifications::kContinuousModelingLabel,		Specifications::kContinuousModelingCommandId },
	{ "New &model",									ID_SIMULATION_NEWMODEL },
	{ "New &L-system",								ID_SIMULATION_NEWLSYSTEM },
	{ "New &homomorphism",							ID_SIMULATION_NEWHOMOMORPHISM },
	{ "New &view",									ID_SIMULATION_NEWVIEW },
	{ "New &projection",							ID_SIMULATION_NEWPROJECTION },
	{ "New ren&der",								ID_SIMULATION_NEWRENDER },
	{ "New &animate",								ID_SIMULATION_NEWANIMATE },
	{ "&Rerun",										ID_SIMULATION_RERUN },

	{ MenuManipulator::tMenuBuildData::kLastItemTag, 0 } // terminating item
};

static const MenuManipulator::tMenuBuildData* aSimulatorMenuData[] =
{
	aCpfgSimulatorMenuData,
	aLpfgSimulatorMenuData,
	aVVSimulatorMenuData,
	aVVSimulatorMenuData, // VVE menu is the same as VV menu
};


static const char* ModelFileLabel[] =
{
	"L-system",
	"L-system",
	"Program",
	"Program",
	"Program",
};

static const char* ParameterFileLabel[] =
{
	"View",
	"View",
	"Parameters",
	"Parameters",
	"Parameters",
};

LProjectCtrl::LProjectCtrl(HWND hwnd, const CREATESTRUCT* pCS) :
MDIChildCtrl(hwnd, pCS, 1),
_tabCtrl(this, Labels, NumOfLabels, 1),
// The semaphore that is used to synchronize L-studio and cpfg
// has a name based on the _hWnd of the L-Project.
// This HWND value is known to both: project and cpfg.
// See the assertion below.
_LSemaphore(1, 1, reinterpret_cast<unsigned int>(Hwnd())),
_pApp(dynamic_cast<LStudioApp*>(App::theApp)),
_clp(_pApp->GetCommandLineParams())
{
	assert(sizeof(unsigned int) == sizeof(HWND));

	_pRemoteAccess = 0;

	_tabCtrl.SetImageList(_pApp->GetTabsList().Handle());

	switch (options.GetDisplayOnTabs())
	{
	case Options::dtText :
		_tabCtrl.SetTabText(Labels, NumOfLabels);
		break;
	case Options::dtIcons :
		_tabCtrl.SetTabIcons(NumOfLabels);
		break;
	case Options::dtBoth :
		_tabCtrl.SetTabTextAndIcons(Labels, NumOfLabels);
		break;
	}

	NewLabTable(_tmpdir);

	_colormode = cColormap;
	_surfacemode = sSimple;

#ifdef SHOW_STDOUT
	_StdOutHeight = 80;
#endif

	_tabCtrl.SetFont(Font(reinterpret_cast<HFONT>(GetStockObject(ANSI_VAR_FONT))));

	{
		ResString caption(64, IDS_NEWPROJECTTITLE);
		SetText(caption);
	}

#ifdef SHOW_STDOUT
	// Create StdOut window
	{
		_pStdOut = StdOut::Create(Hwnd(), HInstance(), 2);
		if (0 == _pStdOut)
			throw Exception(IDERR_CREATESTDOUT);
	}
	// Create splitter
	{
		_pSplitter = Splitter::Create(_hWnd, pCS->hInstance, 3);
		if (0 == _pSplitter)
			throw Exception(IDERR_CREATESPLITTER);
	}
#endif
	// Create LSystem editor window
	{
		HWND hLsysEdit = LSystemEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance);
		if (0 == hLsysEdit)
			throw Exception(IDERR_CREATELSYSEDIT);
		_editors.SetLSysEdit(hLsysEdit);
		_editors.LsysEdit()->Hide();
		if (options.ExternalLsysEdit())
			_editors.LsysEdit()->EnableEdit(false);
	}

	// Create view editor window
	{
		HWND hView = ViewEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance);
		if (0 == hView)
			throw Exception(IDERR_CREATEVIEWEDIT);
		_editors.SetViewEdit(hView);
		_editors._pViewEdit->Hide();
	}

	// Create anim editor
	{
		HWND hAnim = AnimateEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance);
		if (0 == hAnim)
			throw Exception(IDERR_CREATEANIMEDIT);
		_editors.SetAnimEdit(hAnim);
		_editors._pAnimEdit->Hide();
	}
	// Create colormap editor window
	{
		HWND hColormap = ColormapEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hColormap)
			throw Exception(IDERR_CREATECOLORMAPEDIT);
		_editors.SetClrmpEdit(hColormap);
		_editors._pClrmpEdit->Hide();
	}

	// Create material editor window
	{
		HWND hMaterial = MaterialEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hMaterial)
			throw Exception(IDERR_CREATEMATERIALEDIT);
		_editors.SetMaterialEdit(hMaterial);
		_editors._pMaterialEdit->Hide();
	}
	

	// Create surface editor window
	{
		HWND hSurface = SurfaceEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hSurface)
			throw Exception(IDERR_CREATESURFACEEDIT);
		_editors.SetSurfaceEdit(hSurface);
		_editors._pSurfaceEdit->Hide();
	}

	// Create curves editor
	{
		HWND hCurves = CurveEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hCurves)
			throw Exception(IDERR_CREATECURVEEDIT);
		_editors.SetCurveEdit(hCurves);
		_editors._pCurveEdit->Hide();
	}

	// Create contour editor window
	{
		HWND hContours = ContourEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hContours)
			throw Exception(IDERR_CREATECONTOUREDIT);
		_editors.SetContourEdit(hContours);
		_editors._pContourEdit->Hide();
	}

	// Create function editor
	{
		HWND hFunctions = FuncEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hFunctions)
			throw Exception(IDERR_CREATEFUNCEDIT);
		_editors.SetFuncEdit(hFunctions);
		_editors.FunctionEditor()->Hide();
	}

	// Create textfile editor window
	{
		HWND hAnyText = AnyTextEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hAnyText)
			throw Exception(IDERR_CREATEANYTEXT);
		_editors.SetAnyTextEdit(hAnyText);
		_editors._pAnyTextEdit->Hide();
	}


	// Create panels editor window
	{
		HWND hPanels = PanelEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance, this);
		if (0 == hPanels)
			throw Exception(IDERR_CREATEPANELS);
		_editors.SetPanelEdit(hPanels);
		_editors._pPanelEdit->Hide();
	}

	// Create description editor
	{
		HWND hDescr = DescriptionEdit::Create(_tabCtrl.Hwnd(), pCS->hInstance);
		if (0 == hDescr)
			throw Exception(IDERR_CREATEDESCRIPTION);
		_editors.SetDescriptionEdit(hDescr);
		_editors._pDescriptionEdit->Hide();
	}

	_ShowLsystem();

	_counter++;

	_ContinuousMode = false;
	
	LProjectCtrl* pPrevAct = _pActiveProject;
	_pActiveProject = this;

	BuildProjectMenu();

	if (1==_counter)
		FORWARD_WM_MDIMAXIMIZE(GetParent(), Hwnd(), PostMessage);

	_Directory.erase();

	_pActiveProject = pPrevAct;

	const MDICREATESTRUCT* pMCS = reinterpret_cast<const MDICREATESTRUCT*>(pCS->lpCreateParams);
	if (0 != pMCS)
	{
		const CreateParams* pCP = reinterpret_cast<const CreateParams*>(pMCS->lParam);
		if (0 != pCP)
			_UseCreateParams(pCP);
	}
	_pSnapIcon = 0;

	if (0 != _pRemoteAccess)
		_pRemoteAccess->AddRef();
}


void LProjectCtrl::_UseCreateParams(const CreateParams* pCP)
{
	_pRemoteAccess = pCP->RemoteAccess();
	_Directory = pCP->Path();
	SetText(pCP->Name());
}

bool LProjectCtrl::BrowserDisconnecting(VLB::RemoteAccess* pRA)
{
	if (_pRemoteAccess == pRA)
	{
		_specifications.DemoMode(true);
		FORWARD_WM_CLOSE(Hwnd(), SendMessage);
	}
	return true;
}

LProjectCtrl::~LProjectCtrl()
{
	assert(!_tmpdir.empty());

	if (0 != _pSnapIcon)
	{
		_pSnapIcon->PostClose();
		_pSnapIcon = 0;
	}

	_KillSimulator();

	try
	{
		TmpChangeDir tmpdir(_tmpdir);
		FindFile ff(__TEXT("*.*"));
		while (ff.Found())
		{
			::DeleteFile(ff.FileName().c_str());
			ff.FindNext();
		}
		::SetCurrentDirectory(__TEXT(".."));
		::RemoveDirectory(_tmpdir.c_str());
	}
	catch (Exception)
	{}

	_DetachFromBrowser();

	_editors.Destroy();

	_counter--;
}

void LProjectCtrl::Editors::Destroy()
{
	::DestroyWindow(_pLSysEdit->Hwnd()); _pLSysEdit = 0;
	::DestroyWindow(_pFunctionEdit->Hwnd()); _pFunctionEdit = 0;
	::DestroyWindow(_pViewEdit->Hwnd()); _pViewEdit = 0;
	::DestroyWindow(_pAnimEdit->Hwnd()); _pAnimEdit = 0;
	::DestroyWindow(_pClrmpEdit->Hwnd()); _pClrmpEdit = 0;
	::DestroyWindow(_pMaterialEdit->Hwnd()); _pMaterialEdit = 0;
	::DestroyWindow(_pSurfaceEdit->Hwnd()); _pSurfaceEdit = 0;
	::DestroyWindow(_pContourEdit->Hwnd()); _pContourEdit = 0;
	::DestroyWindow(_pCurveEdit->Hwnd()); _pCurveEdit = 0;
	::DestroyWindow(_pPanelEdit->Hwnd()); _pPanelEdit = 0;
	::DestroyWindow(_pDescriptionEdit->Hwnd()); _pDescriptionEdit = 0;
	::DestroyWindow(_pAnyTextEdit->Hwnd()); _pAnyTextEdit = 0;
}


void LProjectCtrl::_DetachFromBrowser()
{
	if (0 != _pRemoteAccess)
	{
		_pRemoteAccess->Release();
		_pRemoteAccess = 0;
	}
}


void LProjectCtrl::NewLabTable(std::string& path)
{
	TmpChangeDir tmp;
	for (int i=0; i<1000; i++)
	{
		std::stringstream name;
		name << "lst" << std::setw(3) << std::setfill('0') << i << ".tmp";
		if (::CreateDirectory(name.str().c_str(), 0))
		{
			::SetCurrentDirectory(name.str().c_str());
			CurrentDirectory cd(path);
			break;
		}
		else
			path.erase();
	}
	if (path.empty())
		throw Exception(IDERR_CANNOTCREATETMPDIR);
}




void LProjectCtrl::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), MDIChWnd<LProjectCtrl>::Proc);
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_PROJECT));
	_WndAtom = wc.Register();
}


bool LProjectCtrl::MouseActivate(LRESULT& res)
{
	SendMessage(GetParent(), WM_MDIACTIVATE, (WPARAM) Hwnd(), 0);
	res = MA_ACTIVATE;
	return true;
}


#ifdef SHOW_STDOUT
void LProjectCtrl::MoveSplitter(HWND, int v)
{
	_StdOutHeight = _height - v - SplitterSize + 2;
	if (_StdOutHeight<-1)
		_StdOutHeight = -1;
	Size(0, 0, _width, _height);
}
#endif



bool LProjectCtrl::Size(SizeState state, int w, int h)
{
	if (state.Minimized())
		return true;

#ifdef SHOW_STDOUT
	_tabCtrl.MoveWindow(0, 0, w, h-_StdOutHeight-SplitterSize);
#else
	_tabCtrl.MoveWindow(0, 0, w, h);
#endif

	// Adjust size of all Editors nested in the tab control
	{
		RECT r;
		{
			r.left = 0;
			r.right = w;
			r.top = 0;
#ifdef SHOW_STDOUT
			r.bottom = h-_StdOutHeight-SplitterSize;
#else
			r.bottom = h;
#endif
		}
		_tabCtrl.AdjustRect(false, &r);
		r.bottom -= r.top;
		r.right -= r.left;

		_editors.LsysEdit()->MoveWindow(r);
		_editors._pViewEdit->MoveWindow(r);
		_editors._pAnimEdit->MoveWindow(r);
		_editors._pClrmpEdit->MoveWindow(r);
		_editors._pMaterialEdit->MoveWindow(r);
		_editors._pSurfaceEdit->MoveWindow(r);
		_editors._pCurveEdit->MoveWindow(r);
		_editors._pContourEdit->MoveWindow(r);
		_editors.FunctionEditor()->MoveWindow(r);
		_editors._pAnyTextEdit->MoveWindow(r);
		_editors._pPanelEdit->MoveWindow(r);
		_editors._pDescriptionEdit->MoveWindow(r);
	}

	
#ifdef SHOW_STDOUT
	// Move the splitter
	_pSplitter->MoveWindow(0, h-_StdOutHeight-SplitterSize, w, SplitterSize);
	// Move the stdout window
	_pStdOut->MoveWindow(0, h-_StdOutHeight, w, _StdOutHeight);
#endif

	return true;
}


bool LProjectCtrl::MDIActivate(bool activate, HWND hActivate)
{
	if (activate)
	{
		switch (_mode)
		{
		case Specifications::mLsystem :
			_editors.LsysEdit()->SetFocus(0);
			break;
		case Specifications::mView :
			_editors._pViewEdit->SetFocus(0);
			break;
		case Specifications::mAnyText :
			_editors._pAnyTextEdit->SetFocus(0);
			break;
		case Specifications::mDescription :
			_editors._pDescriptionEdit->SetFocus(0);
			break;
		}
		assert(!_tmpdir.empty());
		::SetCurrentDirectory(_tmpdir.c_str());
		_pActiveProject = this;

		Activated(true);
	}
	else
	{
		_pActiveProject = 0;
		Activated(false);
	}

	return MDIChildCtrl::MDIActivate(activate, hActivate);
}


HMENU LProjectCtrl::DocumentMenu() const
{
	return _projectMenu.Handle();
}

HMENU LProjectCtrl::DocumentWindowMenu() const
{
	return _projectMenu.SubMenu(4);
}

HMENU LProjectCtrl::GetProjectHelpMenu() const
{
	int index = 5;
	if (Maximized())
	{
		++index;
	}

	return _projectMenu.SubMenu(index);
}

void LProjectCtrl::Activated(bool active)
{
	if (_mode==Specifications::mColors && _colormode==cMaterials)
		_editors._pMaterialEdit->Activated(active);
}

LRESULT LProjectCtrl::Notify(int, const NMHDR* pH)
{
	switch (pH->code)
	{
	case TCN_SELCHANGE :
		_TabSelChanged();
		return false;
	}

	return false;
}



void LProjectCtrl::SwitchTo(Specifications::eProjectMode mode)
{
	_tabCtrl.SetCurSel(mode);
	_TabSelChanged();
}


void LProjectCtrl::_TabSelChanged()
{
	_HideActive();
	::SetFocus(0);
	switch (_tabCtrl.GetCurSel())
	{
	case Specifications::mLsystem :
		_ShowLsystem();
		break;
	case Specifications::mView :
		_ShowView();
		break;
	case Specifications::mAnimate :
		_ShowAnimate();
		break;
	case Specifications::mColors :
		_ShowColors();
		break;
	case Specifications::mSurface :
		_ShowSurfaces();
		break;
	case Specifications::mContours :
		_ShowContours();
		break;
	case Specifications::mFunctions :
		_ShowFunctions();
		break;
	case Specifications::mAnyText :
		_ShowAnyText();
		break;
	case Specifications::mPanels :
		_ShowPanels();
		break;
	case Specifications::mDescription :
		_ShowDescription();
		break;
	default :
		assert(!"Unidentified Flying Tab");
	}
}


void LProjectCtrl::_HideActive()
{
	switch (_mode)
	{
	case Specifications::mLsystem :
		_editors.LsysEdit()->Hide();
		break;
	case Specifications::mView :
		_editors._pViewEdit->Hide();
		break;
	case Specifications::mAnimate :
		_editors._pAnimEdit->Hide();
		break;
	case Specifications::mColors :
		switch (_colormode)
		{
		case cColormap :
			_editors._pClrmpEdit->Hide();
			break;
		case cMaterials :
			_editors._pMaterialEdit->Hide();
			_editors._pMaterialEdit->Activated(false);
			break;
		}
		break;
	case Specifications::mSurface :
		switch (_surfacemode)
		{
		case sSimple :
			_editors._pSurfaceEdit->Hide();
			break;
		case sAdvanced :
			_editors._pCurveEdit->Hide();
			break;
		}
		break;
	case Specifications::mContours :
		_editors._pContourEdit->Hide();
		break;
	case Specifications::mFunctions :
		_editors.FunctionEditor()->Hide();
		break;
	case Specifications::mAnyText :
		_editors._pAnyTextEdit->Hide();
		break;
	case Specifications::mPanels :
		_editors._pPanelEdit->Hide();
		break;
	case Specifications::mDescription :
		_editors._pDescriptionEdit->Hide();
		break;
	case Specifications::mUnspecified :
		break;
	default :
		assert(!"Invalid mode");
	}
}


void LProjectCtrl::_ShowLsystem()
{
	_editors.LsysEdit()->Show();
	_mode = Specifications::mLsystem;
}

void LProjectCtrl::_ShowView()
{
	_editors._pViewEdit->Show();
	_mode = Specifications::mView;
}

void LProjectCtrl::_ShowAnimate()
{
	_editors._pAnimEdit->Show();
	_mode = Specifications::mAnimate;
}


void LProjectCtrl::_ShowColors()
{
	switch (_colormode)
	{
	case cColormap :
		_editors._pClrmpEdit->Show();
		break;
	case cMaterials :
		_editors._pMaterialEdit->Show();
		_editors._pMaterialEdit->Activated(true);
		break;
	}
	_mode = Specifications::mColors;
}

void LProjectCtrl::_ShowSurfaces()
{
	switch (_surfacemode)
	{
	case sSimple :
		_editors._pSurfaceEdit->Show();
		break;
	case sAdvanced :
		_editors._pCurveEdit->Show();
		break;
	}
	_mode = Specifications::mSurface;
}


void LProjectCtrl::_ShowContours()
{
	_editors._pContourEdit->Show();
	_mode = Specifications::mContours;
}

void LProjectCtrl::_ShowFunctions()
{
	_editors.FunctionEditor()->Show();
	_mode = Specifications::mFunctions;
}


void LProjectCtrl::_ShowAnyText()
{
	_editors._pAnyTextEdit->Show();
	_mode = Specifications::mAnyText;
}


void LProjectCtrl::_ShowPanels()
{
	_editors._pPanelEdit->Show();
	_mode = Specifications::mPanels;
}

void LProjectCtrl::_ShowDescription()
{
	_editors._pDescriptionEdit->Show();
	_mode = Specifications::mDescription;
}


bool LProjectCtrl::Command(int id, Window, UINT)
{
	try
	{
		if (static_cast<size_t>(id)>=Specifications::SimulatorCommandBaseId())
		{
			size_t commandId = id - Specifications::SimulatorCommandBaseId();
			if (commandId < _specifications.GetSimulatorCommandsCount())
			{
				const std::string& command = _specifications.GetSimulatorCommandAction(commandId);
				if (_specifications.WriteBeforeCommand(commandId))
				{
					_WriteToLabTable();
				}
				_pApp->ExecuteCommand(command);
			}
		}
		else
		{
			switch (id)
			{
			case ID_SIMULATION_GO :
				_Go();
				break;
			case ID_SIMULATION_BUILD :
				_Build();
				break;
			case ID_SIMULATION_STOP :
				_KillSimulator();
				break;
			case ID_SIMULATION_NEWMODEL :
				if (_simulator.IsSet())
					_NewModel();
				break;
			case ID_SIMULATION_NEWLSYSTEM :
				if (_simulator.IsSet())
					_NewLSystem();
				break;
			case ID_SIMULATION_NEWHOMOMORPHISM :
				if (_simulator.IsSet())
					_NewHomomorphism();
				break;
			case ID_SIMULATION_NEWVIEW :
				if (_simulator.IsSet())
					_NewView();
				break;
			case ID_SIMULATION_RERUN :
				if (_simulator.IsSet())
					_Rerun();
				break;
			case ID_SIMULATION_NEWPROJECTION :
				if (_simulator.IsSet())
					_NewProjection();
				break;
			case ID_SIMULATION_NEWRENDER :
				if (_simulator.IsSet())
					_NewRender();
				break;
			case ID_SIMULATION_NEWANIMATE :
				if (_simulator.IsSet())
					_NewAnimate();
				break;
			case PrjVar::ccQuitting :
				_SimulatorTerminated();
				break;
			case ID_OBJECT_SAVE :
				if (IsUntitled())
					_SaveAs();
				else
					_Export();
				break;
			case ID_OBJECT_SAVEAS :
				_SaveAs();
				break;
			case ID_OBJECT_MAKE_EXTENSION :
				_ExportTo();
				break;
			case ID_OBJECT_CLOSE :
				_Close();
				break;
			case PrjVar::ccPanicExit :
				_ErrorExit();
				break;
			case ID_HIDDEN_FIND :
				_Find();
				break;
			case ID_HIDDEN_FINDAGAIN :
				_FindAgain();
				break;
			case ID_SIMULATION_CONTINUOUSMODELING :
				_ToggleContinuousMode();
				break;
			case ID_HIDDEN_EDITCOMPONENT_LSYSTEM :
				SwitchTo(Specifications::mLsystem);
				break;
			case ID_HIDDEN_EDITCOMPONENT_VIEW :
				SwitchTo(Specifications::mView);
				break;
			case ID_HIDDEN_EDITCOMPONENT_ANIMATE :
				SwitchTo(Specifications::mAnimate);
				break;
			case ID_HIDDEN_EDITCOMPONENT_COLORS :
				SwitchTo(Specifications::mColors);
				break;
			case ID_HIDDEN_EDITCOMPONENT_SURFACES :
				SwitchTo(Specifications::mSurface);
				break;
			case ID_HIDDEN_EDITCOMPONENT_CONTOURS :
				SwitchTo(Specifications::mContours);
				break;
			case ID_HIDDEN_EDITCOMPONENT_FUNCTIONS :
				SwitchTo(Specifications::mFunctions);
				break;
			case ID_HIDDEN_EDITCOMPONENT_PANELS :
				SwitchTo(Specifications::mPanels);
				break;
			case ID_HIDDEN_EDITCOMPONENT_DESCRIPTION :
				SwitchTo(Specifications::mDescription);
				break;
			case ID_HIDDEN_EDITCOMPONENT_TEXTFILE :
				SwitchTo(Specifications::mAnyText);
				break;
			case ID_HIDDEN_DOMAGIC :
				_DoMagic();
				break;
			case ID_TOOLS_SNAPICON :
				_SnapIcon();
				break;
			case ID_OBJECT_POSITIONINBROWSER :
				_PositionInTheBrowser();
				break;
			case ID_OBJECT_SHOWLABTABLE :
				_ShowLabTable();
				break;
			case ID_OBJECT_WRITETOLABTABLE :
				_WriteToLabTable();
				break;
			case ID_OBJECT_READFROMLABTABLE :
				_ReadFromLabTable();
				break;
			case ID_OBJECT_LABTABLESHELL :
				_LabTableShell();
				break;
			case ID_UTILS_SNAPICONCLOSING :
				_SnapIconClosing();
				break;
			}
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void LProjectCtrl::SetViewOptions(const LStudioOptions& options)
{
	_editors._pSurfaceEdit->ColorschemeModified();
	_editors._pContourEdit->ColorschemeModified();
	_editors.FunctionEditor()->ColorschemeModified();

	switch (options.GetDisplayOnTabs())
	{
	case Options::dtText :
		_tabCtrl.SetTabText(Labels, NumOfLabels);
		break;
	case Options::dtIcons :
		_tabCtrl.SetTabIcons(NumOfLabels);
		break;
	case Options::dtBoth :
		_tabCtrl.SetTabTextAndIcons(Labels, NumOfLabels);
		break;
	}
	Invalidate();
}


const char* LProjectCtrl::Name() const
{
	std::string::size_type bs = _Directory.find_last_of('\\');
	if (std::string::npos == bs)
		bs = _Directory.find_last_of('/');
	if (std::string::npos == bs)
		return "untitled";
	else
		return _Directory.c_str()+bs+1;
}




void LProjectCtrl::_Close()
{
	FORWARD_WM_CLOSE(Hwnd(), PostMessage);
}


void LProjectCtrl::UseColormap()
{
	_colormode = cColormap;
	if (Specifications::mColors == _mode)
	{
		_editors._pMaterialEdit->Hide();
		_editors._pMaterialEdit->Activated(false);
		_editors._pClrmpEdit->Show();
	}
}


void LProjectCtrl::UseMaterials()
{
	_colormode = cMaterials;
	if (Specifications::mColors == _mode)
	{
		_editors._pClrmpEdit->Hide();
		_editors._pMaterialEdit->Show();
		_editors._pMaterialEdit->Activated(true);
	}
}

#define USE_CURVES

#ifdef USE_CURVES

void LProjectCtrl::AdvancedSurfaceMode()
{
	_editors._pSurfaceEdit->Generate();
	_editors._pSurfaceEdit->Hide();
	while (_editors._pCurveEdit->Items()>1)
	{
		_editors._pCurveEdit->Select(0);
		_editors._pCurveEdit->Delete();
	}
	{
		{
			// Find all surfaces (.s)
			FindFile ff(__TEXT("*.s"));
			while (ff.Found())
			{
				_editors._pCurveEdit->Import(ff.FileName().c_str());
				ff.FindNext();
			}
		}
		{
			// Find all surfaces (.srf)
			FindFile fs(__TEXT("*.srf"));
			while (fs.Found())
			{
				_editors._pCurveEdit->Import(fs.FileName().c_str());
				fs.FindNext();
			}
		}
		{
			// Find all curves
			FindFile fc(__TEXT("*.crv"));
			while (fc.Found())
			{
				_editors._pCurveEdit->Import(fc.FileName().c_str());
				fc.FindNext();
			}
		}

		if (_editors._pCurveEdit->Items()>1)
		{
			_editors._pCurveEdit->Select(0);
			_editors._pCurveEdit->Delete();
		}
	}
	_editors._pCurveEdit->Show();
	_surfacemode = sAdvanced;
}

#else
void LProjectCtrl::AdvancedSurfaceMode()
{
	_editors._pSurfaceEdit->Generate();
	_editors._pSurfaceEdit->Hide();
	while (_editors._pCurveEdit->Items()>1)
	{
		_editors._pCurveEdit->Select(0);
		_editors._pCurveEdit->Delete();
	}
	{
		// Find all surfaces
		FindFile ff(__TEXT("*.s"));
		while (ff.Found())
		{
			_editors._pCurveEdit->Import(ff.FileName());
			ff.FindNext();
		}
		if (_editors._pCurveEdit->Items()>1)
		{
			_editors._pCurveEdit->Select(0);
			_editors._pCurveEdit->Delete();
		}
	}
	_editors._pCurveEdit->Show();
	_surfacemode = sAdvanced;
}
#endif

void LProjectCtrl::SimpleSurfaceMode()
{
	_editors._pCurveEdit->Generate();
	_editors._pCurveEdit->Hide();
	_editors._pSurfaceEdit->Clear();
	{
		// Find all surfaces
		FindFile ff(__TEXT("*.s"));
		while (ff.Found())
		{
			_editors._pSurfaceEdit->Import(ff.FileName().c_str());
			ff.FindNext();
		}
		if (_editors._pSurfaceEdit->Items()>1)
		{
			_editors._pSurfaceEdit->Select(0);
			_editors._pSurfaceEdit->Delete();
		}
	}
	_editors._pSurfaceEdit->Show();
	_surfacemode = sSimple;
}



void LProjectCtrl::_Go()
{
	_GenerateAll();

	_RunSimulator();

	if (_simulator.IsSet())
	{
		_editors._pPanelEdit->ExecuteMode();
	}
}

void LProjectCtrl::_Build()
{
  // still no error checking
	if(_specifications.HasBuildLine())
	{
		_GenerateAll();
		NewProcess buildProc(_specifications.BuildLine(), IDERR_BUILDCOMMAND);
	}
}


void LProjectCtrl::_RunSimulator()
{
	if (_simulator.IsSet())
		_simulator.ResetHwnd(0);
	if (!_SimProcess.IsNull())
		_SimProcess.Close();
	std::string cmndln;
	if (_specifications.HasCmndLine())
		cmndln = _specifications.CmndLine();
	else
		_BuildCmndLine(cmndln);

	if (_specifications.ModelType() == Specifications::mtLpfg ||
	    _specifications.ModelType() == Specifications::mtCpfg)
	// launch the simulator and connect to it
	{
		TmpFile Xwnds
			(
			".\\xwnds",
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			CREATE_NEW
			);

		{
			HWND hW = Hwnd();
			Xwnds.Write(&hW, sizeof(HWND));
		}

		Semaphore waitforsimulator(0, 1, __TEXT("CpfgStarted"));

		NewProcess simulator(cmndln, IDERR_LAUNCHCPFG);

		if (!waitforsimulator.Wait(5000))
		{
			_simulator.ResetHwnd(0);
			throw Exception(IDERR_CPFGWAITTIMEOUT);
		}
		else
		{
			HWND hSimulator;
			Xwnds.Read(&hSimulator, sizeof(HWND));
			_simulator.ResetHwnd(hSimulator);
			_SimProcess.Reset(simulator.ReleaseProcess());
			{
				static TCHAR wndttl[256];
				GetWindowText(Hwnd(), wndttl, 256);
				_simulator.SetText(wndttl);
			}
		}
	}
	else
		NewProcess simulator(cmndln, IDERR_LAUNCHCPFG);
}


void LProjectCtrl::_KillSimulator()
{
	if (_SimProcess.IsRunning())
	{
		assert(_simulator.IsSet());
		_simulator.PostClose();
		if (!_SimProcess.Wait(3000))
			_SimProcess.Terminate();
		_SimProcess.Close();
		_simulator.ResetHwnd(0);
		_SimulatorTerminated();
	}
}



void LProjectCtrl::_NewModel()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
	{
		_GenerateLSystemFile();
		_GenerateViewFile();
		_GenerateAnimFile();

		switch (_colormode)
		{
		case cColormap :
			_GenerateColormapFile();
			break;
		case cMaterials :
			_GenerateMaterialFile();
			break;
		}

		switch (_surfacemode)
		{
		case sSimple :
			_editors._pSurfaceEdit->Generate();
			break;
		case sAdvanced :
			_editors._pCurveEdit->Generate();
			break;
		}

		_editors._pContourEdit->Generate();
		_editors.FunctionEditor()->Generate();
		_simulator.PostCommand(PrjVar::ccNewModel);
	}
}


void LProjectCtrl::_NewLSystem()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
	{
		_GenerateLSystemFile();
		_simulator.PostCommand(PrjVar::ccNewLsystem);
	}
}

void LProjectCtrl::_NewHomomorphism()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
	{
		_GenerateLSystemFile();
		_simulator.PostCommand(PrjVar::ccNewHomomorphism);
	}
}


void LProjectCtrl::_NewView()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
	{
		_GenerateViewFile();

		switch (_colormode)
		{
		case cColormap :
			_GenerateColormapFile();
			break;
		case cMaterials :
			_GenerateMaterialFile();
			break;
		}

		switch (_surfacemode)
		{
		case sSimple :
			_editors._pSurfaceEdit->Generate();
			break;
		case sAdvanced :
			_editors._pCurveEdit->Generate();
			break;
		}

		_editors._pContourEdit->Generate();
		_editors.FunctionEditor()->Generate();

		_simulator.PostCommand(PrjVar::ccNewView);
	}
}


void LProjectCtrl::_Rerun()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
		_simulator.PostCommand(PrjVar::ccRerun);
}

void LProjectCtrl::_NewProjection()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
	{
		_GenerateViewFile();
		_simulator.PostCommand(PrjVar::ccNewProjection);
	}
}



void LProjectCtrl::_NewRender()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
	{
		_GenerateViewFile();
		_simulator.PostCommand(PrjVar::ccNewRender);
	}
}


void LProjectCtrl::_NewAnimate()
{
	assert(_simulator.IsSet());
	if (_simulator.IsValid())
	{
		_GenerateAnimFile();
		_simulator.PostCommand(PrjVar::ccNewAnimate);
	}
}



void LProjectCtrl::_SimulatorTerminated()
{
	_simulator.ResetHwnd(0);
	if (!_SimProcess.IsNull())
		_SimProcess.Close();
	_LSemaphore.Release();
	if (_specifications.AutoExit())
	{
		_pApp->PostQuit();
	}
}

void LProjectCtrl::AdjustMenu(MenuManipulator& menu) const
{
	menu.Enable(ID_PREFERENCES_EDIT_CURRENTEDITOR, IsEditorConfigurable(_mode));
	menu.SetCheck(ID_SIMULATION_CONTINUOUSMODELING, _ContinuousMode);

	if (_SimProcess.IsRunning())
	{
		menu.Disable(ID_SIMULATION_GO); // Go
		menu.Disable(ID_SIMULATION_BUILD);
		menu.Enable(ID_SIMULATION_CONTINUOUSMODELING);
		menu.Enable(ID_SIMULATION_STOP);
		menu.Enable(ID_SIMULATION_NEWMODEL);         // New model
		menu.Enable(ID_SIMULATION_NEWLSYSTEM);         // New L-system
		menu.Enable(ID_SIMULATION_NEWVIEW);         // New view
		menu.Enable(ID_SIMULATION_NEWHOMOMORPHISM);
		menu.Enable(ID_SIMULATION_NEWPROJECTION);
		menu.Enable(ID_SIMULATION_NEWRENDER);
		menu.Enable(ID_SIMULATION_RERUN);
		menu.Enable(ID_SIMULATION_NEWANIMATE);
	}
	else if (_specifications.ModelType() == Specifications::mtLpfg ||
		 _specifications.ModelType() == Specifications::mtCpfg)
	{
		menu.Enable(ID_SIMULATION_GO);   // Go
		menu.Disable(ID_SIMULATION_BUILD); // yes, we disable Build for *pfg even if
						   // there's a build line in the LSspecifications
		if (!options.ExternalLsysEdit() && _editors.LsysEdit()->IsEmpty())
			menu.Disable(ID_SIMULATION_GO);
		menu.Enable(ID_SIMULATION_CONTINUOUSMODELING);
		menu.Disable(ID_SIMULATION_STOP);
		menu.Disable(ID_SIMULATION_NEWMODEL);         // New model
		menu.Disable(ID_SIMULATION_NEWLSYSTEM);         // New L-system
		menu.Disable(ID_SIMULATION_NEWVIEW);         // New view
		menu.Disable(ID_SIMULATION_NEWHOMOMORPHISM);
		menu.Disable(ID_SIMULATION_NEWPROJECTION);
		menu.Disable(ID_SIMULATION_NEWRENDER);
		menu.Disable(ID_SIMULATION_RERUN);
		menu.Disable(ID_SIMULATION_NEWANIMATE);
	}
	else if (_specifications.ModelType() == Specifications::mtVVE)
	{
		menu.Enable(ID_SIMULATION_GO);   // Go
		if(_specifications.HasBuildLine())
			menu.Enable(ID_SIMULATION_BUILD);
		else
			menu.Disable(ID_SIMULATION_BUILD);
		menu.Enable(ID_SIMULATION_CONTINUOUSMODELING);
		menu.Disable(ID_SIMULATION_STOP);
		menu.Disable(ID_SIMULATION_NEWMODEL);         // New model
		menu.Disable(ID_SIMULATION_NEWLSYSTEM);         // New L-system
		menu.Disable(ID_SIMULATION_NEWVIEW);         // New view
		menu.Disable(ID_SIMULATION_NEWHOMOMORPHISM);
		menu.Disable(ID_SIMULATION_NEWPROJECTION);
		menu.Disable(ID_SIMULATION_NEWRENDER);
		menu.Disable(ID_SIMULATION_RERUN);
		menu.Disable(ID_SIMULATION_NEWANIMATE);
	}
	else
	{
		menu.Enable(ID_SIMULATION_GO);
		if(_specifications.HasBuildLine())
			menu.Enable(ID_SIMULATION_BUILD);
		else
			menu.Disable(ID_SIMULATION_BUILD);
		menu.Disable(ID_SIMULATION_CONTINUOUSMODELING);
		menu.Enable(ID_SIMULATION_CONTINUOUSMODELING);
		menu.Disable(ID_SIMULATION_STOP);
		menu.Disable(ID_SIMULATION_NEWMODEL);         // New model
		menu.Disable(ID_SIMULATION_NEWLSYSTEM);         // New L-system
		menu.Disable(ID_SIMULATION_NEWVIEW);         // New view
		menu.Disable(ID_SIMULATION_NEWHOMOMORPHISM);
		menu.Disable(ID_SIMULATION_NEWPROJECTION);
		menu.Disable(ID_SIMULATION_NEWRENDER);
		menu.Disable(ID_SIMULATION_RERUN);
		menu.Disable(ID_SIMULATION_NEWANIMATE);
	}

	if (0 != _pSnapIcon)
		menu.Check(ID_TOOLS_SNAPICON);
	else
		menu.Uncheck(ID_TOOLS_SNAPICON);

	if (IsConnected())
	{
		menu.Enable(ID_OBJECT_POSITIONINBROWSER);
		menu.Enable(ID_OBJECT_MAKE_EXTENSION); // Make extension
	}
	else
	{
		menu.Disable(ID_OBJECT_POSITIONINBROWSER);
		menu.Disable(ID_OBJECT_MAKE_EXTENSION); // Make extension
	}

	::DrawMenuBar(Hwnd());
}



void LProjectCtrl::_ErrorExit()
{
	UINT err = 0;
	switch (_specifications.ModelType())
	{
	case Specifications::mtLpfg:
		err = IDERR_LPFGCRASHED;
		break;
	case Specifications::mtCpfg:
		err = IDERR_CPFGERROREXIT;
		break;
	case Specifications::mtOther:
	case Specifications::mtVV:
	case Specifications::mtVVE:
	default:
		err = IDERR_SIMULATORCRASHED;
		break;
	}

	ErrorBox(256, err);
	MenuManipulator mm(_pApp->GetProjectMenu());
	_simulator.ResetHwnd(0);
	if (!_SimProcess.IsNull())
		_SimProcess.Close();
	// If the simulator failed to release the sempahore before crashing
	// it must be done here
	_LSemaphore.Release();
}


HWND LProjectCtrl::GetLsystemEditWnd() const
{
	return _editors.LsysEdit()->HEdit().Hwnd();
}

HWND LProjectCtrl::GetViewEditWnd() const
{
	return _editors._pViewEdit->HEdit().Hwnd();
}



void LProjectCtrl::SetEditorFont(const Font& font)
{
	_editors.LsysEdit()->SetEditorFont(font);
	_editors._pViewEdit->SetEditorFont(font);
	_editors._pDescriptionEdit->SetEditorFont(font);
	_editors._pAnyTextEdit->SetEditorFont(font);
}


void LProjectCtrl::_Find()
{
	switch (_mode)
	{
	case Specifications::mLsystem :
		_editors.LsysEdit()->Find();
		break;
	case Specifications::mView :
		_editors._pViewEdit->Find();
		break;
	case Specifications::mDescription:
		_editors._pDescriptionEdit->Find();
		break;
	case Specifications::mAnyText:
		_editors._pAnyTextEdit->Find();
		break;
	}
}


void LProjectCtrl::_FindAgain()
{
	switch (_mode)
	{
	case Specifications::mLsystem :
		_editors.LsysEdit()->FindAgain();
		break;
	case Specifications::mView :
		_editors._pViewEdit->FindAgain();
		break;
	case Specifications::mDescription:
		_editors._pDescriptionEdit->FindAgain();
		break;
	case Specifications::mAnyText:
		_editors._pAnyTextEdit->FindAgain();
		break;
	}
}


void LProjectCtrl::_ToggleContinuousMode()
{
	_ContinuousMode = !_ContinuousMode;
}


bool LProjectCtrl::LsystemModified(bool final)
{
	if (_simulator.IsSet())
	{
		if (final)
		{
			if (_LSemaphore.Wait(eDefaultWaitForFinal*1000))
			{
				_GenerateLSystemFile();
				_simulator.PostCommand(PrjVar::ccNewLsystem);
				return true;
			}
			else
			{
				WaitForSemaphoreDlg dlg(_LSemaphore, IDS_WAITINGSIMULATOR);
				if (IDCANCEL != dlg.DoModal(*this))
				{
					_GenerateLSystemFile();
					_simulator.PostCommand(PrjVar::ccNewLsystem);
					return true;
				}
			}
		}
		else
		{
			if (_LSemaphore.Wait(0))
			{
				_GenerateLSystemFile();
				_simulator.PostCommand(PrjVar::ccNewLsystem);
				return true;
			}
		}
	}
	else
		return true;
	return false;
}


bool LProjectCtrl::ViewFileModified(bool final)
{
	if (_simulator.IsSet())
	{
		if (final)
		{
			if (_LSemaphore.Wait(eDefaultWaitForFinal*1000))
			{
				_GenerateViewFile();
				_simulator.PostCommand(PrjVar::ccNewView);
				return true;
			}
			else
			{
				WaitForSemaphoreDlg dlg(_LSemaphore, IDS_WAITINGSIMULATOR);
				if (IDCANCEL != dlg.DoModal(*this))
				{
					_GenerateViewFile();
					_simulator.PostCommand(PrjVar::ccNewView);
					return true;
				}
			}
		}
		else
		{
			if (_LSemaphore.Wait(0))
			{
				_GenerateViewFile();
				_simulator.PostCommand(PrjVar::ccNewView);
				return true;
			}
		}
	}
	else
		return true;
	return false;
}



bool LProjectCtrl::ColormapModified(bool final)
{
	if (_simulator.IsSet())
	{
		if (final)
		{
			if (_LSemaphore.Wait(eDefaultWaitForFinal*1000))
			{
				_GenerateColormapFile();
				_simulator.PostCommand(PrjVar::ccRereadColors);
				return true;
			}
			else
			{
				WaitForSemaphoreDlg dlg(_LSemaphore, IDS_WAITINGSIMULATOR);
				if (IDCANCEL != dlg.DoModal(*this))
				{
					_GenerateColormapFile();
					_simulator.PostCommand(PrjVar::ccRereadColors);
					return true;
				}
			}
		}
		else
		{
			if (_LSemaphore.Wait(0))
			{
				_GenerateColormapFile();
				_simulator.PostCommand(PrjVar::ccRereadColors);
				return true;
			}
		}
	}
	else
	{
		_GenerateColormapFile();
		return true;
	}
	return false;
}


bool LProjectCtrl::MaterialModified(bool final)
{
	return _NotifySimulator(PrjVar::ccRereadColors, _editors._pMaterialEdit, final);
}


bool LProjectCtrl::SurfaceModified(bool final)
{
	const ObjectEdit* pEdit = 0;
	switch (_surfacemode)
	{
	case sSimple :
		pEdit = _editors._pSurfaceEdit;
		break;
	case sAdvanced :
		pEdit = _editors._pCurveEdit;
		break;
	}
	return _NotifySimulator(PrjVar::ccRereadSurfaces, pEdit, final);
}


bool LProjectCtrl::ContourCurveModified(bool final)
{
	return _NotifySimulator(PrjVar::ccRereadCurvesRerun, _editors._pContourEdit, final);
}


bool LProjectCtrl::ContourModified(bool final)
{
	return _NotifySimulator(PrjVar::ccRereadContours, _editors._pContourEdit, final);
}

bool LProjectCtrl::NewModel(bool final)
{
	return _NotifySimulator(PrjVar::ccNewModel, pAllEditors, final);
}

bool LProjectCtrl::FunctionModified(bool final)
{
	return _NotifySimulator(PrjVar::ccRereadFunctionsRerun, _editors.FunctionEditor(), final);
}

bool LProjectCtrl::CurveXYZModified(bool final)
{
	return _NotifySimulator(PrjVar::ccRereadCurvesRerun, _editors._pCurveEdit, final);
}

bool LProjectCtrl::ExternalFileModified(bool final)
{
	return _NotifySimulator(PrjVar::ccRerun, 0, final);
}


bool LProjectCtrl::_NotifySimulator(UINT msg, const ObjectEdit* pEdit, bool final)
{
	if (_simulator.IsSet())
	{
		if (final)
		{
			if (_LSemaphore.Wait(eDefaultWaitForFinal*1000))
			{
				if (pAllEditors==pEdit)
					_GenerateAll();
				else if (0 != pEdit)
					pEdit->Generate();
				_simulator.PostCommand(msg);
				return true;
			}
			else
			{
				WaitForSemaphoreDlg dlg(_LSemaphore, IDS_WAITINGSIMULATOR);
				if (IDCANCEL != dlg.DoModal(*this))
				{
					if (pAllEditors==pEdit)
						_GenerateAll();
					else if (0 != pEdit)
						pEdit->Generate();
					_simulator.PostCommand(msg);
					return true;
				}
			}
		}
		else
		{
			if (_LSemaphore.Wait(0))
			{
				if (pAllEditors==pEdit)
					_GenerateAll();
				else if (0 != pEdit)
					pEdit->Generate();
				_simulator.PostCommand(msg);
				return true;
			}
		}
	}
	else
	// VVE uses a File Alteration Monitor to implement Continuous mode,
	// so we only have to save the files
	{
		if (pAllEditors==pEdit)
			_GenerateAll();
		else if (0 != pEdit)
			pEdit->Generate();
		return true;
	}
	return false;
}


void LProjectCtrl::BuildProjectMenu()
{
	Menu newMenu(IDR_PROJECT);

	MenuManipulator projectMenu(newMenu.Handle());
	projectMenu.SetTextBP(kSimulatorMenuId, SimulatorLabel());
	MenuManipulator simulatorMenu(projectMenu.GetSubMenu(kSimulatorMenuId));

	if (_specifications.GetSimulatorCommandsCount()>0)
	{
		simulatorMenu.BuildMenu(_specifications.SimulatorCommands());
	}
	else if (_specifications.ModelType() != Specifications::mtOther)
	{
		simulatorMenu.BuildMenu(aSimulatorMenuData[_specifications.ModelType()]);
	}
	else
	{
		simulatorMenu.BuildMenu(aSimulatorMenuData[Specifications::mtCpfg]);
	}

	MenuManipulator helpMenu(projectMenu.GetSubMenu(kHelpMenuId));
	_pApp->UpdateHelpMenu(helpMenu, options);

	_projectMenu.Swap(newMenu);
}

void LProjectCtrl::ReadLSspecifications()
{
	ReadLSspecifications(Params::LSspecs);
}

void LProjectCtrl::ReadLSspecifications(const std::string& fname)
{
	std::string dir;
	_specifications.ReadNew(fname, _editors._pClrmpEdit->Name());

	_ContinuousMode = _specifications.ContinuousMode();
	BuildProjectMenu();
	SetDocumentMenu();
	const int BfSize = 32;
	ResString progCaption(BfSize, IDS_EMPTY), parmCaption(BfSize, IDS_EMPTY);

	if (options.GetDisplayOnTabs() == Options::dtText)
	{
		_tabCtrl.SetTabText(Specifications::mLsystem, ModelFileLabel[_specifications.ModelType()]);
		_tabCtrl.SetTabText(Specifications::mView, ParameterFileLabel[_specifications.ModelType()]);
	}
	else if (options.GetDisplayOnTabs() == Options::dtBoth)
	{
		_tabCtrl.SetTabTextAndIcon(Specifications::mLsystem, ModelFileLabel[_specifications.ModelType()]); 
		_tabCtrl.SetTabTextAndIcon(Specifications::mView, ParameterFileLabel[_specifications.ModelType()]); 
	}
}

void LProjectCtrl::ReadSpecifications()
{
	ReadSpecifications(Params::specs);
}

void LProjectCtrl::ReadSpecifications(const std::string& fname)
{
	_specifications.ReadOld(fname, _editors._pClrmpEdit->Name());
}


void LProjectCtrl::_BuildCmndLine(std::string& cmndln) const
{
	cmndln = "cpfg ";
	if (cColormap==_colormode)
	{
		cmndln.append("-m ");
		cmndln.append(_editors._pClrmpEdit->Name());
		cmndln.append(" ");
	}
	else
	{
		cmndln.append("-M ");
		cmndln.append(_editors._pMaterialEdit->Name());
		cmndln.append(" ");
	}

	// If an *.e file present then include it in the command line 
	{
		FindFile ff(__TEXT("*.e"));
		if (ff.Found())
		{
			cmndln.append("-e ");
			cmndln.append(ff.FileName());
			cmndln.append(" ");
		}
	}

	cmndln.append(_editors.LsysEdit()->Name());
	cmndln.append(" ");

	cmndln.append(_editors._pViewEdit->Name());
	cmndln.append(" ");

	cmndln.append(_editors._pAnimEdit->Name());
}


bool LProjectCtrl::Close()
{
	if (_clp.DemoMode() || _specifications.DemoMode())
		return true;
	try
	{
		_GenerateAll();
		if (IsUntitled())
		{
			if (MessageYesNo(IDS_SAVENEWPROJECT))
				_SaveAs();
		}
		else
		{
			DiffList dl;
			_CompareDirectories(dl);
			if (!dl.Identical())
			{
				if (!AskSave(dl))
					return false;
			}
		}
		return true;
	}
	catch (Exception e)
	{
		ErrorBox(e);
		return true;
	}
	return true;
}

bool LProjectCtrl::QueryEndSession()
{ 
	return Close();
}




void LProjectCtrl::_SnapIcon()
{
	if (0 != _pSnapIcon)
	{
		_pSnapIcon->PostClose();
		_pSnapIcon = 0;
	}
	else
	{
		POINT p;
		if (SimulatorRunning())
			_simulator.GetCenter(p);
		else
			GetCenter(p);
		_pSnapIcon = SnapIcon::Create(Hwnd(), p);
	}
}


bool LProjectCtrl::IsConnected() const
{
	if (0 == _pRemoteAccess)
		return false;
	else if (!_pRemoteAccess->Connected())
		return false;
	else
		return true;
}

bool LProjectCtrl::_CheckConnection() const
{
	if (0 == _pRemoteAccess)
	{
		MessageBox(IDERR_BROWSERNOTRUNNING);
		return false;
	}
	else if (!_pRemoteAccess->Connected())
	{
		return false;
		MessageBox(IDERR_CONNECTFIRST);
	}
	return true;
}



void LProjectCtrl::_PositionInTheBrowser()
{
	if (!_CheckConnection())
	{
		MessageBox(IDERR_BROWSERNOTRUNNING);
		return;
	}
	_pRemoteAccess->PositionObject(_Directory);
	_pRemoteAccess->ShowInFront();
}


void LProjectCtrl::_ShowLabTable() const
{
	ShellExecute(0, "open", _tmpdir.c_str(), 0, 0, SW_SHOWNORMAL);
}

void LProjectCtrl::_LabTableShell() const
{
	// szComspec will hold the name of the shell command
	// (command.com or cmd.exe or whatever)
	TCHAR szComspec[MAX_PATH];
	if(0 != GetEnvironmentVariable("COMSPEC",szComspec,MAX_PATH))
		ShellExecute(0,"open",szComspec,_tmpdir.c_str(),0,SW_SHOWNORMAL);
}



void LProjectCtrl::_ClearAllEditors()
{
	_editors.LsysEdit()->Clear();
	_editors._pViewEdit->Clear();
	_editors._pAnimEdit->Clear();
	_editors._pClrmpEdit->Clear();
	_editors._pMaterialEdit->Clear();
	_editors._pSurfaceEdit->Clear();
	_editors._pContourEdit->Clear();
	_editors.FunctionEditor()->Clear();
	_editors._pCurveEdit->Clear();
	_editors._pPanelEdit->Clear();
	_editors._pDescriptionEdit->Clear();
	_editors._pAnimEdit->Clear();
}


bool LProjectCtrl::IsEditorConfigurable(Specifications::eProjectMode mode) const
{
	switch (mode)
	{
	case Specifications::mLsystem:
	case Specifications::mView:
	case Specifications::mContours:
	case Specifications::mFunctions:
	case Specifications::mDescription:
	case Specifications::mAnyText:
		return true;
	case Specifications::mAnimate:
	case Specifications::mColors:
	case Specifications::mPanels:
	case Specifications::mSurface:
		return false;
	case Specifications::mUnspecified:
		return false;
	}
	return false;
}


const std::string& LProjectCtrl::SimulatorLabel() const
{
	static const std::string cpfgLabel = "&Cpfg";
	static const std::string lpfgLabel = "&Lpfg";
	static const std::string vvLabel = "&VV";
	static const std::string simulatorLabel = "&Simulator";
	if (_specifications.IsSimulatorNameSpecified())
		return _specifications.SimulatorName();
	else
	{
		switch (_specifications.ModelType())
		{
		case Specifications::mtCpfg :
			return cpfgLabel;
		case Specifications::mtLpfg :
			return lpfgLabel;
		case Specifications::mtVV :
		case Specifications::mtVVE :
			return vvLabel;
		case Specifications::mtOther :
			return simulatorLabel;
		}
	}

	return simulatorLabel;
}

