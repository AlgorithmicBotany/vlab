#include <vector>
#include <string>

#include <fw.h>
#include <glfw.h>

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
#include "surfthumbcb.h"
#include "surfaceedit.h"
#include "contouredit.h"
#include "funcedit.h"
#include "curveedit.h"
#include "panelsedit.h"
#include "difflist.h"

#include "params.h"

std::string ExceptionLog;


void LProjectCtrl::_ReadFromLabTable()
{
	_ClearAllEditors();
	LoadObjectToEditors();
}



void LProjectCtrl::Import(const std::string& root, const std::string& directory)
{
	ExceptionLog.erase();
	{
		TmpChangeDir rt(root);
		TmpChangeDir tmp(directory);
		
		{
			FindFile ff(__TEXT("*.*"));
			while (ff.Found())
			{
				if (!(ff.IsDirectory()))
				{
					static TCHAR target[_MAX_PATH];
					_tcscpy(target, _tmpdir.c_str());
					_tcscat(target, __TEXT("\\"));
					_tcscat(target, ff.FileName().c_str());
					CopyFile(ff.FileName().c_str(), target, false);
					DWORD attr = GetFileAttributes(target);
					attr &= ~FILE_ATTRIBUTE_READONLY;
					SetFileAttributes(target, attr);
				}
				ff.FindNext();
			}
		}
		LoadObjectToEditors();
		CurrentDirectory cd(_Directory);
	}
	
	SetText(Name());
	
	if (ExceptionLog.length()>0)
		MessageBox(ExceptionLog);
	
	if (_specifications.AutoRun())
		_Go();
	if (_specifications.PanelsAutoTearOff())
	{
		_editors._pPanelEdit->ExecuteMode();
		_editors._pPanelEdit->TearOffAll();
	}
	
	ExceptionLog.erase();
}


void LProjectCtrl::LoadObjectToEditors()
{
	SwitchTo(Specifications::mLsystem);
	ExceptionLog.erase();

	_specifications.Default();
	LoadNewSpecifications(ExceptionLog);
	if (_specifications.IsLoaded() && _specifications.IsModelFileSpecified())
	{
		LoadModelFile(_specifications.ModelFile(), ExceptionLog);
	}
	else
	{
		LoadDefaultModelFile(ExceptionLog);
	}

	LoadViewFile(ExceptionLog);
	LoadAnimateFile(ExceptionLog);
	LoadColorFile(ExceptionLog);
	LoadSurfaces(ExceptionLog);
	LoadContours(ExceptionLog);
	LoadFunctions(ExceptionLog);
	LoadPanels(ExceptionLog);
	LoadDescription(ExceptionLog);

	if (!_specifications.IsLoaded())
	{
		LoadOldSpecifications(ExceptionLog);
	}

	if (_specifications.ActiveTabSpecified())
		SwitchTo(_specifications.ActiveTab());
	else if (!_editors._pDescriptionEdit->IsEmpty())
		SwitchTo(Specifications::mDescription);

	if (!ExceptionLog.empty())
		throw Exception(ExceptionLog.c_str());
}


void LProjectCtrl::LoadModelFile(const std::string& modelFilename, std::string& exceptionLog)
{
	try
	{
		_editors.LsysEdit()->Import(modelFilename.c_str());
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadDefaultModelFile(std::string& exceptionLog)
{
	// Find lsystem file
	FindFile ff(__TEXT("*.l"));
	if (ff.Found())
	{
		while (ff.StartsWith("PANEL") && ff.Found())
			ff.FindNext();
		if (ff.Found())
			LoadModelFile(ff.FileName(), exceptionLog);
	}
	else
	{
		FindFile ff(__TEXT("*.vvp"));
		if (ff.Found())
			LoadModelFile(ff.FileName(), exceptionLog);
		else
		{
			FindFile ff(__TEXT("*.cpp"));
			if (ff.Found())
				LoadModelFile(ff.FileName(), exceptionLog);
		}
	}
}


void LProjectCtrl::LoadViewFile(std::string& exceptionLog)
{
	try
	{
		// Find view file
		FindFile ff(__TEXT("*.v"));
		if (ff.Found())
		{
			while (ff.StartsWith("PANEL") && ff.Found())
				ff.FindNext();
			if (ff.Found())
				_editors._pViewEdit->Import(ff.FileName().c_str());
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadAnimateFile(std::string& exceptionLog)
{
	try
	{
		// Find animate file
		FindFile ff(__TEXT("*.a"));
		if (ff.Found())
		{
			while (ff.StartsWith("PANEL") && ff.Found())
				ff.FindNext();
			if (ff.Found())
				_editors._pAnimEdit->Import(ff.FileName().c_str());
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadColorFile(std::string& exceptionLog)
{
	try
	{
		// Find colormap file
		FindFile ff(__TEXT("*.map"));
		if (ff.Found())
		{
			while ((ff.StartsWith("PANEL") || !ff.EndsWith("MAP")) && ff.Found())
				ff.FindNext();
			if (ff.Found())
			{
				_editors._pClrmpEdit->Import(ff.FileName().c_str());
				_colormode = cColormap;
			}
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
	

	try
	{
		// Find material file
		FindFile ff(__TEXT("*.mat"));
		if (ff.Found())
		{
			_editors._pMaterialEdit->Import(ff.FileName().c_str());
			_colormode = cMaterials;
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
	
}


void LProjectCtrl::LoadSurfaces(std::string& exceptionLog)
{
	try
	{
		// Find surfaces
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
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadContours(std::string& exceptionLog)
{
	try
	{
		// Find contours
		FindFile ff(__TEXT("*.cset"));
		if (ff.Found())
		{
			_editors._pContourEdit->LoadGallery(ff.FileName().c_str());
		}
		else
		{
			FindFile ff(__TEXT("*.con"));
			while (ff.Found())
			{
				_editors._pContourEdit->Import(ff.FileName().c_str());
				ff.FindNext();
			}
			if (_editors._pContourEdit->Items()>1)
			{
				_editors._pContourEdit->Select(0);
				_editors._pContourEdit->Delete();
			}
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}

void LProjectCtrl::LoadFunctions(std::string& exceptionLog)
{
	try
	{
		// Find functions
		FindFile ff(__TEXT("*.fset"));
		if (ff.Found())
		{
			_editors.FunctionEditor()->LoadGallery(ff.FileName().c_str());
		}
		else
		{
			FindFile ff(__TEXT("*.func"));
			while (ff.Found())
			{
				_editors.FunctionEditor()->Import(ff.FileName().c_str());
				ff.FindNext();
			}
			if (_editors.FunctionEditor()->Items()>1)
			{
				_editors.FunctionEditor()->Select(0);
				_editors.FunctionEditor()->Delete();
			}
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadPanels(std::string& exceptionLog)
{
	try
	{
		// Find panels
		FindFile ff(__TEXT("*.pnl"));
		while (ff.Found())
		{
			_editors._pPanelEdit->Import(ff.FileName().c_str());
			ff.FindNext();
		}
		if (_editors._pPanelEdit->Items()>1)
		{
			_editors._pPanelEdit->Select(0);
			_editors._pPanelEdit->Delete();
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadDescription(std::string& exceptionLog)
{
	try
	{
		// Find description
		FindFile ff(__TEXT(Params::DescriptionFName));
		if (ff.Found())
		{
			_editors._pDescriptionEdit->Import(ff.FileName().c_str());
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadNewSpecifications(std::string& exceptionLog)
{
	try
	{
		FindFile ff(Params::LSspecs);
		if (ff.Found())
		{
			ReadLSspecifications(ff.FileName());
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}


void LProjectCtrl::LoadOldSpecifications(std::string& exceptionLog)
{
	try
	{
		FindFile ff(Params::specs);
		if (ff.Found())
		{
				ReadSpecifications(ff.FileName());
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}

void LProjectCtrl::LoadSpecifications(std::string& exceptionLog)
{
	try
	{
		FindFile ff(Params::LSspecs);
		if (ff.Found())
		{
			ReadLSspecifications(ff.FileName());
		}
		else
		{
			FindFile ff(Params::specs);
			if (ff.Found())
				ReadSpecifications(ff.FileName());
		}
	}
	catch (Exception e)
	{
		exceptionLog.append(e.Msg());
		exceptionLog.append("\n");
	}
}