#include <vector>
#include <fstream>

#include <fw.h>
#include <glfw.h>

#include <scramble.h>

#include <browser/remaccess.h>

#include "resource.h"

#include "tedit.h"
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
#include "newextdlg.h"

#include "surfthumbcb.h"
#include "surfaceedit.h"
#include "contouredit.h"
#include "funcedit.h"
#include "curveedit.h"
#include "panelsedit.h"
#include "difflist.h"
#include "savechngdlg.h"

#include "params.h"




void LProjectCtrl::_WriteToLabTable() const
{
	_GenerateAll();
	if (_editors._pAnyTextEdit->IsNamed())
		_editors._pAnyTextEdit->Generate();
}



void LProjectCtrl::_ExportTo()
{
	assert(RemoteProject());
	_GenerateAll();
	NewExtensionDlg dlg(_Directory, _pRemoteAccess->PathSeparator(), false);
	if (IDOK == dlg.DoModal(*this))
	{
		if (dlg.StoreLocally())
			_NewVersionLocal(dlg.Path(), dlg.Name(), false);
		else
		{
			_NewVersionRemote(dlg.Name(), dlg.ChangeIdentity());
			_pRemoteAccess->PositionObject(_Directory);
		}
	}
}


class SaveAsDlg : public Dialog
{
public:
	SaveAsDlg(const std::string& dir) : Dialog(IDD_SAVEAS), _directory(dir) {}
	void UpdateData(bool what)
	{
		DX(_directory, IDC_OOFS, what);
	}
	bool Command(int id, Window, UINT)
	{
		if (IDC_BROWSE==id)
		{
			_Browse();
			return true;
		}
		return false;
	}
	const char* Path() const
	{ return _directory.c_str(); }
	bool _Check()
	{
		if (_directory.empty())
		{
			_CheckFailed(IDERR_NEWEXTNMEMPTY, IDC_NAME);
			return false;
		}
		if (std::string::npos == _directory.find_last_of('\\'))
		{
			_CheckFailed(IDERR_NAMENOTDIR, IDC_NAME);
			return false;
		}
		return true;
	}

private:
	bool DoInit()
	{
		EditLine dir(GetDlgItem(IDC_OOFS));
		dir.GrabFocus();
		dir.EditAtEnd();
		return false;
	}
	std::string _directory;
	void _Browse()
	{
		UpdateData(true);
		FolderBrowser fb(Hdlg(), IDS_BROWSENEWVERDIR, IDS_EMPTY);
		fb.AllowNewFolder(true);
		if (fb.Browse(_directory))
		{
			UpdateData(false);
		}
	}
};


void LProjectCtrl::_SaveAs()
{
	_GenerateAll();
	std::string newDir;
	if (IsUntitled() || RemoteProject())
	{
		if (options.OofsRoot()[0] == 0)
		{
			SpecialFolder sf(CSIDL_DESKTOP);
			sf.GetPath(newDir);
		}
		else
			newDir = options.OofsRoot();
		if (RemoteProject())
		{
			newDir.append("\\");
			newDir.append(Name());
		}
		else
			newDir.append("\\NewObject");
	}
	else // named local object
		newDir = _Directory;

	SaveAsDlg dlg(newDir);


	if (IDOK == dlg.DoModal(*this))
	{
		std::string path(dlg.Path());
		size_t sl = path.find_last_of('\\');
		std::string name(path.substr(sl+1));
		path.erase(sl);
		_NewVersionLocal(path.c_str(), name.c_str(), true);
		_Directory = dlg.Path();
	}
}


void LProjectCtrl::_NewVersionRemote(const std::string& name, bool ChangeIdentity)
{
	if (!_CheckConnection())
		return;

	string_buffer ignored;
	{
		for (string_buffer::const_iterator it(options.IgnoredFiles()); !it.at_end(); it.advance())
			ignored.add(it.str());
	}
	{
		for (string_buffer::const_iterator it(_specifications.Ignored()); !it.at_end(); it.advance())
		{
			if (!ignored.contains(it.str()))
				ignored.add(it.str());
		}
	}

	std::string Dir(_Directory);
	if (!_pRemoteAccess->MakeExtension(Dir, name.c_str(), _tmpdir.c_str(), ignored))
		throw Exception(IDERR_CREATENEWVER, name.c_str());

	if (ChangeIdentity)
	{
		_Directory = Dir;
		SetText(name);
		if (SimulatorRunning())
			_simulator.SetText(name);
	}
}



void LProjectCtrl::_NewVersionLocal(const std::string& directory, const std::string& name, bool ChangeIdentity)
{
	std::string trgdir = directory;
	trgdir.append("\\");
	trgdir.append(name);
	if (!CreateDirectory(trgdir.c_str(), 0))
	{
		DWORD err = GetLastError();
		switch (err)
		{
		case ERROR_ALREADY_EXISTS :
			if (!MessageYesNo(IDS_OVERWRITEPROJECT, trgdir.c_str()))
				return;
			break;
		case ERROR_ACCESS_DENIED :
			throw Exception(IDERR_CANNOTCREATEDIR, trgdir.c_str());
		default :
			throw Exception(IDERR_CANNOTCREATEDIR, trgdir.c_str());
		}
	}
	_ExportLocal(trgdir.c_str());
	if (ChangeIdentity)
	{
		_Directory = trgdir;
		SetText(name);
		if (SimulatorRunning())
			_simulator.SetText(name);
		_DetachFromBrowser();
	}
}

void LProjectCtrl::_GenerateLSystemFile() const
{
	if (!options.ExternalLsysEdit())
		_editors.LsysEdit()->Generate();
}


void LProjectCtrl::_GenerateViewFile() const
{
	_editors._pViewEdit->Generate();
}


void LProjectCtrl::_GenerateAnimFile() const
{
	_editors._pAnimEdit->Generate();
}


void LProjectCtrl::_GenerateColormapFile() const
{
	_editors._pClrmpEdit->Generate();
}

void LProjectCtrl::_GenerateMaterialFile() const
{
	_editors._pMaterialEdit->Generate();
}


void LProjectCtrl::_Export()
{
	assert(!_Directory.empty());

	_GenerateAll();

	if (_editors._pAnyTextEdit->ShouldSave())
	{
		_tabCtrl.SetCurSel(Specifications::mAnyText);
		_HideActive();
		_ShowAnyText();
		_editors._pAnyTextEdit->Save();
	}

	if (RemoteProject())
		_ExportRemote(_Directory);
	else
		_ExportLocal(_Directory);
}


void LProjectCtrl::_ExportLocal(const std::string& trgdir)
{
	// First clean the storage
	{
		TmpChangeDir tcd(trgdir);
		FindFile ff("*.*");
		while (ff.Found())
		{
			if (!(ff.IsDirectory()) && '.' != ff.FileName()[0])
				::DeleteFile(ff.FileName().c_str());
			ff.FindNext();
		}
	}

	FindFile ff(__TEXT("*.*"));

	// Copy significant files
	while (ff.Found())
	{
		if (!(ff.IsDirectory()))
		{
			if (_SignificantFile(ff.FileName().c_str()) || _specifications.AlwaysSave().contains(ff.FileName().c_str()))
			{
				std::string trg(trgdir);
				trg.append("\\");
				trg.append(ff.FileName());
				if (0==CopyFile(ff.FileName().c_str(), trg.c_str(), FALSE))
				{
					const int BfSize = 64;
					static char Bf[BfSize+1];
					FormatMessage
						(
						FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						0,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						Bf, BfSize+1, 0
						);
					throw Exception(IDERR_CANNOTSAVEFILE, trg.c_str(), Bf);
				}
			}
		}
		ff.FindNext();
	}
}


bool LProjectCtrl::AskSave(const DiffList& dl) 
{
	int res = -1;

	if (!_Directory.empty())
	{
		std::string obj;
		GetText(obj);
		SaveChangesDlg dlg(obj.c_str(), dl);
		MessageBeep(MB_ICONQUESTION);
		res = dlg.DoModal(*this);
	}
	else
	{
		std::string title;
		GetText(title);
		ResString text(64, IDS_SAVENEWPROJECT);
		res = ::MessageBox(Hwnd(), text.c_str(), title.c_str(), MB_YESNOCANCEL | MB_ICONQUESTION);
	}

	switch (res)
	{
	case IDNO :
		return true;
	case IDYES :
		if (RemoteProject())
			_ExportRemote(_Directory);
		else
		{
			try
			{
				if (!_Directory.empty())
					_Export();
				else
					_SaveAs();
			}
			catch (Exception e)
			{
				ErrorBox(e);
				return false;
			}
		}
		return true;
	case IDCANCEL :
		return false;
	}

	return true;
}

void LProjectCtrl::_GenerateAll() const
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
	_editors._pPanelEdit->Generate();
	_editors._pDescriptionEdit->Generate();
}


bool LProjectCtrl::Modified(DiffList& dl) const
{
	if (_Directory.empty())
		return true;
	_GenerateAll();

	if (_editors._pAnyTextEdit->ShouldSave())
	{
		_editors._pAnyTextEdit->Save();
	}

	if (!(_CompareDirectories(dl)))
		return true;
	else
		return false;
}


bool LProjectCtrl::_CompareDirectories(DiffList& dl) const
{
	if (RemoteProject())
		return _CompareRemoteDirs(dl);
	else
		return _CompareLocalDirs(dl);
}

bool LProjectCtrl::_CompareRemoteDirs(DiffList& dl) const
{
	{
		FindFile ff(__TEXT("*.*"));
		while (ff.Found())
		{
			if (!(ff.IsDirectory()))
			{
				if (_SignificantFile(ff.FileName().c_str()))
					dl.AddSrc1(ff.FileName().c_str());
			}
			ff.FindNext();
		}
	}
	string_buffer remfiles;
	_pRemoteAccess->GetFileList(_Directory.c_str(), remfiles, true);
	const string_buffer& always_save = _specifications.AlwaysSave();
	for (string_buffer::const_iterator it(remfiles); !it.at_end(); it.advance())
	{
		if (it.str()[0] != '.'  && !always_save.contains(it.str()))
			dl.AddSrc2(it.str());
	}
	dl.Compare(_pRemoteAccess, _tmpdir.c_str(), _Directory.c_str());
	return dl.Identical();
}

bool LProjectCtrl::_CompareLocalDirs(DiffList& dl) const
{
	{
		FindFile ff(__TEXT("*.*"));
		while (ff.Found())
		{
			if (!(ff.IsDirectory()))
			{
				if (_SignificantFile(ff.FileName().c_str()))
					dl.AddSrc1(ff.FileName().c_str());
			}
			ff.FindNext();
		}
	}
	{
		TmpChangeDir tcd(_Directory);
		FindFile ff(__TEXT("*.*"));
		const string_buffer& always_save = _specifications.AlwaysSave();
		while (ff.Found())
		{
			if (!ff.IsDirectory() && !always_save.contains(ff.FileName().c_str()))
				dl.AddSrc2(ff.FileName().c_str());
			ff.FindNext();
		}
	}
	dl.Compare(_Directory.c_str());
	return dl.Identical();
}



bool LProjectCtrl::_CompareFiles(const TCHAR* f1, const TCHAR* f2) const
{
	try
	{
		ReadBinFile F1(f1);
		ReadBinFile F2(f2);
		long sz1 = F1.Size();
		long sz2 = F2.Size();
		if (sz1 != sz2)
			return false;
		for (long p = 0; p<sz1; p++)
		{
			unsigned char b1, b2;
			F1.Read(&b1, sizeof(char));
			F2.Read(&b2, sizeof(char));
			if (b1 != b2)
				return false;
		}
		return true;
	}
	catch (Exception)
	{
		return false;
	}
}


bool LProjectCtrl::_SignificantFile(const TCHAR* fn) const
{
	{
		const string_buffer& global = options.IgnoredFiles();
		if (global.contains(fn))
			return false;
	}

	{
		const string_buffer& always_save = _specifications.AlwaysSave();
		if (always_save.contains(fn))
			return false;
	}

	static const TCHAR* InsignificantFiles[] =
	{
		__TEXT("cpfg.log"),
		__TEXT("lpfg.log")
	};

	for (int i=0; i<sizeof(InsignificantFiles)/sizeof(InsignificantFiles[0]); ++i)
	{
		if (!(_tcsicmp(InsignificantFiles[i], fn)))
			return false;
	}
	if (!(_tcsnicmp(__TEXT(".from_field"), fn, 11)))
		return false;
	if (!(_tcsnicmp(__TEXT(".to_field"), fn, 9)))
		return false;
	return !_specifications.Ignored(fn);
}

bool LProjectCtrl::_CompareRemoteFile(const char* fname) const
{
	if (!_CheckConnection())
		return false;
	return _pRemoteAccess->CompareFiles(fname, _Directory.c_str());
}

//#define PUTOBJ

void LProjectCtrl::_ExportRemote(const std::string& trgdir)
{
	if (!_CheckConnection())
		return;

	// First clean the storage
	if (options.CleanStorageBeforeWrite())
	{
		string_buffer fl;
		_pRemoteAccess->GetFileList(trgdir.c_str(), fl, true);
		for (string_buffer::const_iterator it(fl); !it.at_end(); it.advance())
		{
			if ('.' != it.str()[0])
				_pRemoteAccess->DeleteFile(trgdir.c_str(), it.str());
		}
	}

#ifdef PUTOBJ
	string_buffer flist;
	flist.add(trgdir);
	for (FindFile ff(__TEXT("*.*")); ff.Found(); ff.FindNext())
	{
		if (!(FILE_ATTRIBUTE_DIRECTORY & ff.Attributes()))
		{
			if (_SignificantFile(ff.FileName()))
			{
				flist.add(ff.FileName());
			}
		}
	}
	assert(0 != _pRemoteAccess);
	_pRemoteAccess->PutObject(flist);
#else
	for (FindFile ff(__TEXT("*.*")); ff.Found(); ff.FindNext())
	{
		if (!(ff.IsDirectory()))
		{
			if (_SignificantFile(ff.FileName().c_str()) || _specifications.AlwaysSave().contains(ff.FileName().c_str()))
			{
				std::string trg(trgdir);
				trg.append(1, _pRemoteAccess->PathSeparator());
				trg.append(ff.FileName());
				assert(0 != _pRemoteAccess);
				if (!_pRemoteAccess->PutFile(ff.FileName().c_str(), trg.c_str()))
					throw Exception(IDERR_SAVINGOBJECT);
			}
		}
	}

	_pRemoteAccess->PrototypeObject(trgdir.c_str());	
#endif
}

