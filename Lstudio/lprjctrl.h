#ifndef __LPRJCTRL_H__
#define __LPRJCTRL_H__


#include "lsysedit.h"
#include "viewedit.h"
#include "anytextedit.h"
#include "descredit.h"
#include "specifications.h"

class AnimateEdit;
class ColormapEdit;
class MaterialEdit;
class TextureManager;
class SurfaceEdit;
class ContourEdit;
class FuncEdit;
class CurveEdit;
class DiffList;
class LStudioOptions;
class ObjectEdit;
class CommandLineParams;
class LStudioApp;
class PanelEdit;
class SnapIcon;

namespace VLB
{
	class RemoteAccess;
}

//#define SHOW_STDOUT


class LProjectCtrl : public MDIChildCtrl, public PrjNotifySink
{
public:

	LProjectCtrl(HWND, const CREATESTRUCT*);
	~LProjectCtrl();

	enum eColorMode
	{
		cColormap,
		cMaterials
	};

	enum eSurfaceMode
	{
		sSimple,
		sAdvanced
	};

	// Message handlers
	bool Command(int, Window, UINT);
	bool Size(SizeState, int, int);
	bool MDIActivate(bool, HWND);
	bool MouseActivate(LRESULT&);
	LRESULT Notify(int, const NMHDR*);

#ifdef SHOW_STDOUT
	void MoveSplitter(HWND, int);
#endif
	bool Close();
	bool QueryEndSession();

	void Activated(bool);
	bool IsConnected() const;

	static void Register(HINSTANCE);

	static const TCHAR* _ClassName()
	{ return __TEXT("LProjectCtrlClass"); }

	void Load(const TCHAR*);
	void Import(const std::string&, const std::string&);

	bool Modified(DiffList&) const;
	bool AskSave(const DiffList&); // Yes/No --> true, Cancel --> false

	bool RemoteProject() const
	{ return 0 != _pRemoteAccess; }

	// if true -> go ahead, close
	// if false -> wait, the object not saved
	bool BrowserDisconnecting(VLB::RemoteAccess*);
	void UseColormap();
	void UseMaterials();
	void Go()
	{ _Go(); }
	const TCHAR* Name() const;

	static LProjectCtrl* GetActiveProject()
	{ return _pActiveProject; }

	void AdvancedSurfaceMode();
	void SimpleSurfaceMode();
	Specifications::eProjectMode ProjectMode() const
	{ return _mode; }
	eColorMode GetColorMode() const
	{ return _colormode; }

	HWND GetLsystemEditWnd() const;
	HWND GetViewEditWnd() const;

	bool IsUntitled() const
	{ return _Directory.empty(); }

	void SwitchTo(Specifications::eProjectMode);
	void SetEditorFont(const Font&);
	void SetViewOptions(const LStudioOptions&);
	void ReadLSspecifications(const std::string&);
	void ReadSpecifications(const std::string&);
	void ReadLSspecifications();
	void ReadSpecifications();

	// Continuous mode related stuff
	bool ContinuousMode() const
	{ return _ContinuousMode; }
	void ContinuousMode(bool f)
	{ _ContinuousMode = f; }
	bool AutoRun() const
	{ return _specifications.AutoRun(); }


	// the bool parameter specifies wether 
	// L-studio should wait indefinitely
	// to notify cpfg about the change
	bool LsystemModified(bool);
	bool ViewFileModified(bool);
	bool ColormapModified(bool);
	bool MaterialModified(bool);
	bool SurfaceModified(bool);
	bool ContourCurveModified(bool);
	bool ContourModified(bool);
	bool FunctionModified(bool);
	bool CurveXYZModified(bool);
	bool ExternalFileModified(bool);
	bool NewModel(bool);

	static ATOM WndAtom()
	{ return _WndAtom; }

	void AdjustMenu(MenuManipulator&) const;

	const std::string& GetLabTable() const
	{ return _tmpdir; }
	static void NewLabTable(std::string&);

	void LoadObjectToEditors();

	class CreateParams
	{
	public:
		CreateParams(const std::string& Path, const std::string& Name, VLB::RemoteAccess* pAccess) :
		  _Path(Path), _Name(Name), _pRemoteAccess(pAccess)
		{}
		const std::string& Path() const
		{ return _Path; }
		const std::string& Name() const
		{ return _Name; }
		VLB::RemoteAccess* RemoteAccess() const
		{ return _pRemoteAccess; }
	private:
		const std::string& _Path;
		const std::string& _Name;
		VLB::RemoteAccess* _pRemoteAccess;
	};

	HMENU GetProjectHelpMenu() const;

protected:
	HMENU DocumentMenu() const;
	HMENU DocumentWindowMenu() const;


private:
	Specifications::eProjectMode _mode;
	eColorMode _colormode;
	eSurfaceMode _surfacemode;

	Specifications _specifications;

	struct tSimulatorMenu
	{
		enum tEnum
		{
			eCpfg = 0,
			eLpfg = 1,
			eVV = 2,
			eSimulator = 3
		};
	};

	void _NewVersionRemote(const std::string&, bool /* change identity */);
	void _NewVersionLocal(const std::string&, const std::string&, bool /* change identity */);
	void _ExportRemote(const std::string&);
	void _ExportLocal(const std::string&);

	// Commands
	void _Go();
	void _Build();
	void _KillSimulator();
	void _Close();
	void _Export();
	void _ExportTo();
	void _SaveAs();
	void _NewModel();
	void _NewLSystem();
	void _NewHomomorphism();
	void _NewView();
	void _NewProjection();
	void _NewRender();
	void _NewAnimate();
	void _Rerun();
	void _Find();
	void _FindAgain();
	void _ToggleContinuousMode();
	void _SnapIcon();
	void _PositionInTheBrowser();
	void _ShowLabTable() const;
	void _LabTableShell() const;
	void _WriteToLabTable() const;
	void _ReadFromLabTable();

	void LoadSpecifications(std::string& exceptionLog);
	void LoadNewSpecifications(std::string& exceptionLog);
	void LoadOldSpecifications(std::string& exceptionLog);
	void LoadModelFile(const std::string& modelFile, std::string& exceptionLog);
	void LoadDefaultModelFile(std::string& exceptionLog);
	void LoadViewFile(std::string& exceptionLog);
	void LoadAnimateFile(std::string& exceptionLog);
	void LoadColorFile(std::string& exceptionLog);
	void LoadSurfaces(std::string& exceptionLog);
	void LoadContours(std::string& exceptionLog);
	void LoadFunctions(std::string& exceptionLog);
	void LoadPanels(std::string& exceptionLog);
	void LoadDescription(std::string& exceptionLog);
	
	
	void BuildProjectMenu();

	void _DoMagic();
	void _FuncNameFound(std::string&, const char*, FuncEdit*, int, string_buffer&);

	void _DetachFromBrowser();

	void _SimulatorTerminated();
	void _CheckFileHeader(ReadTextFile&);

	void _UseCreateParams(const CreateParams*);

	void _TabSelChanged();
	void _HideActive();
	void _ShowLsystem();
	void _ShowView();
	void _ShowAnimate();
	void _ShowColors();
	void _ShowSurfaces();
	void _ShowContours();
	void _ShowFunctions();
	void _ShowAnyText();
	void _ShowPanels();
	void _ShowDescription();
	void _ErrorExit();

	bool IsEditorConfigurable(Specifications::eProjectMode mode) const;
	const std::string& SimulatorLabel() const;

	void _SnapIconClosing()
	{ _pSnapIcon = 0; }


#ifdef SHOW_STDOUT
	int _StdOutHeight;
#endif

	TabCtrl _tabCtrl;
	Semaphore _LSemaphore;
	Menu _projectMenu;

	LStudioApp* _pApp;
	VLB::RemoteAccess* _pRemoteAccess;
#ifdef SHOW_STDOUT
	Splitter*		_pSplitter;
	StdOut*			_pStdOut;
#endif

	class Editors
	{
	public:
		Editors()
		{
			_pLSysEdit = 0;
			_pFunctionEdit = 0;
			_pViewEdit = 0;
			_pAnimEdit = 0;
			_pClrmpEdit = 0;
			_pMaterialEdit = 0;
			_pSurfaceEdit = 0;
			_pContourEdit = 0;
			_pCurveEdit = 0;
			_pPanelEdit = 0;
			_pDescriptionEdit = 0;
			_pAnyTextEdit = 0;
		}
		void Destroy();
		void SetLSysEdit(HWND hE)
		{
			assert(0 != hE);
			_pLSysEdit = GetWinLong<LSystemEdit*>(hE);
			assert(0 != _pLSysEdit);
		}
		void SetViewEdit(HWND hE)
		{
			assert(0 != hE);
			_pViewEdit = GetWinLong<ViewEdit*>(hE);
			assert(0 != _pViewEdit);
		}
		void SetAnimEdit(HWND hE)
		{
			assert(0 != hE);
			_pAnimEdit = GetWinLong<AnimateEdit*>(hE);
			assert(0 != _pAnimEdit);
		}
		void SetClrmpEdit(HWND hE)
		{
			assert(0 != hE);
			_pClrmpEdit = GetWinLong<ColormapEdit*>(hE);
			assert(0 != _pClrmpEdit);
		}
		void SetMaterialEdit(HWND hE)
		{
			assert(0 != hE);
			_pMaterialEdit = GetWinLong<MaterialEdit*>(hE);
			assert(0 != _pMaterialEdit);
		}
		void SetSurfaceEdit(HWND hE)
		{
			assert(0 != hE);
			_pSurfaceEdit = GetWinLong<SurfaceEdit*>(hE);
			assert(0 != _pSurfaceEdit);
		}
		void SetContourEdit(HWND hE)
		{
			assert(0 != hE);
			_pContourEdit = GetWinLong<ContourEdit*>(hE);
			assert(0 != _pContourEdit);
		}
		void SetFuncEdit(HWND hE)
		{
			assert(0 != hE);
			_pFunctionEdit = GetWinLong<FuncEdit*>(hE);
			assert(0 != _pFunctionEdit);
		}
		void SetCurveEdit(HWND hE)
		{
			assert(0 != hE);
			_pCurveEdit = GetWinLong<CurveEdit*>(hE);
			assert(0 != _pCurveEdit);
		}
		void SetPanelEdit(HWND hE)
		{
			assert(0 != hE);
			_pPanelEdit = GetWinLong<PanelEdit*>(hE);
			assert(0 != _pPanelEdit);
		}
		void SetDescriptionEdit(HWND hE)
		{
			assert(0 != hE);
			_pDescriptionEdit = GetWinLong<DescriptionEdit*>(hE);
			assert(0 != _pDescriptionEdit);
		}
		void SetAnyTextEdit(HWND hE)
		{
			assert(0 != hE);
			_pAnyTextEdit = GetWinLong<AnyTextEdit*>(hE);
			assert(0 != _pAnyTextEdit);
		}
		LSystemEdit* LsysEdit()
		{ return _pLSysEdit; }
		const LSystemEdit* LsysEdit() const
		{ return _pLSysEdit; }
		FuncEdit* FunctionEditor()
		{ return _pFunctionEdit; }
		const FuncEdit* FunctionEditor() const
		{ return _pFunctionEdit; }
		DescriptionEdit* DescriptionEditor() 
		{ return _pDescriptionEdit; }
		const DescriptionEdit* DescriptionEditor() const
		{ return _pDescriptionEdit; }
	private:
		LSystemEdit*		_pLSysEdit;
		FuncEdit*			_pFunctionEdit;
	public:
		ViewEdit*			_pViewEdit;
		AnimateEdit*		_pAnimEdit;
		ColormapEdit*		_pClrmpEdit;
		MaterialEdit*		_pMaterialEdit;
		SurfaceEdit*		_pSurfaceEdit;
		ContourEdit*		_pContourEdit;
		CurveEdit*			_pCurveEdit;
		PanelEdit*			_pPanelEdit;
		DescriptionEdit*	_pDescriptionEdit;
		AnyTextEdit*		_pAnyTextEdit;
	};
	Editors _editors;

	std::string _Directory;
	std::string _tmpdir;

	static int _counter;

	void _RunSimulator();
	void _GenerateLSystemFile() const;
	void _GenerateViewFile() const;
	void _GenerateAnimFile() const;
	void _GenerateColormapFile() const;
	void _GenerateMaterialFile() const;

	void _GenerateAll() const;
	void _ClearAllEditors();

	void _ExportToRA(const char*);
	bool _CompareDirectories(DiffList&) const;
	bool _CompareLocalDirs(DiffList&) const;
	bool _CompareRemoteDirs(DiffList&) const;

	bool _CompareRemoteFile(const char*) const;
	bool _CompareFiles(const TCHAR*, const TCHAR*) const;
	bool _SignificantFile(const TCHAR*) const;

	void _BuildCmndLine(std::string&) const;

	bool _CheckConnection() const;

	// true updated, false simulator is busy
	bool _NotifySimulator(UINT /* msg */, const ObjectEdit* /* pEdit */, bool /* final */);

	bool SimulatorRunning() const
	{
		return _SimProcess.IsRunning();
	}
	Process::Process _SimProcess;
	Window _simulator;

	static LProjectCtrl* _pActiveProject;
	static ATOM _WndAtom;
	static const ObjectEdit* pAllEditors;

	bool _ContinuousMode;

	enum
	{
		eMaxCmndLine = 128
	};

	const CommandLineParams& _clp;

	SnapIcon* _pSnapIcon;
};

#else
	#error File already included
#endif
