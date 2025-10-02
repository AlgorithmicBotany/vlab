#include <vector>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "glgallery.h"

#include "resource.h"

#include "funcgallery.h"
#include "funcpts.h"
#include "function.h"
#include "menuids.h"
#include "lstudioptns.h"


UINT FuncGallery::_ClipboardFormat = 0;

void FuncGallery::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<FuncGallery>::Proc);
	wc.style = 0;
	wc.Register();
	_ClipboardFormat = RegisterClipboardFormat(_ClipboardFormatName());
}


FuncGallery::FuncGallery(HWND hwnd, const CREATESTRUCT* pCS) : GLGallery(hwnd, pCS, 2.2f, 8, true)
{
	_version = 101;
	std::unique_ptr<EditableObject> pNew(new Function);
	Add(pNew);
}


FuncGallery::~FuncGallery()
{}


EditableObject* FuncGallery::_NewObject() const
{
	return new Function;
};


bool FuncGallery::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_FUNCTIONGALLERY_NEW : 
			_New();
			break;
		case ID_FUNCTIONGALLERY_DELETE :
			if (!options.WarnGalleryDelete() || MessageYesNo(IDS_DELETEFROMGALLERY))
				_Delete();
			break;
		case ID_FUNCTIONGALLERY_CUT :
			_Cut();
			break;
		case ID_FUNCTIONGALLERY_COPY :
			_Copy();
			break;
		case ID_FUNCTIONGALLERY_BIND :
			_Bind();
			break;
		case ID_FUNCTIONGALLERY_PASTE_AFTER :
			_PasteAfter();
			break;
		case ID_FUNCTIONGALLERY_PASTE_BEFORE :
			_PasteBefore();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void FuncGallery::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(FuncGalleryCMenu);
	_AdaptMenu(hMenu);
	TrackPopupMenu
		(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0
		);
}


void FuncGallery::_AdaptMenu(HMENU hMenu) const
{
	{
		ResString caption(64, (1==_items) ? IDS_RESET : IDS_DELETE);
		MenuManipulator mm(hMenu);
		mm.SetText(ID_FUNCTIONGALLERY_DELETE, caption);
	}

	if (IsClipboardFormatAvailable(_ClipboardFormat))
	{
		EnableMenuItem(hMenu, ID_FUNCTIONGALLERY_PASTE_AFTER, MF_ENABLED);
		EnableMenuItem(hMenu, ID_FUNCTIONGALLERY_PASTE_BEFORE, MF_ENABLED);
	}
	else
	{
		EnableMenuItem(hMenu, ID_FUNCTIONGALLERY_PASTE_AFTER, MF_GRAYED);
		EnableMenuItem(hMenu, ID_FUNCTIONGALLERY_PASTE_BEFORE, MF_GRAYED);
	}

	if (Bound())
		CheckMenuItem(hMenu, ID_FUNCTIONGALLERY_BIND, MF_CHECKED);
	else
		CheckMenuItem(hMenu, ID_FUNCTIONGALLERY_BIND, MF_UNCHECKED);
}



const TCHAR* FuncGallery::_GetObjectName(int i) const
{
	assert(i>=0);
	assert(i<Items());
	const Function* pFunc = dynamic_cast<const Function*>(GetObject(i));
	static TCHAR res[80];
	OemToCharBuff(pFunc->GetName(), res, 79);
	res[79] = 0;
	return res;
}


void FuncGallery::_SetVersion(int v)
{
	if (Bound() && v<101)
	{
		MessageBox(IDERR_INVVERFORBOUNDFUNCS);
	}
	else
	{
		for (int i=_FirstSelected; i<=_LastSelected; i++)
		{
			Function* pFunc = dynamic_cast<Function*>(_GetObject(i));
			pFunc->SetVersion(v);
		}
		_pEdit->Retrieve();
	}
}


void FuncGallery::_ToggleNewFormat()
{
	const Function* pFunc = dynamic_cast<Function*>(_GetObject(_current));
	bool upgrade = (0==pFunc->GetVersion());
	int trgver = -1;
	if (upgrade)
	{
		trgver = 101;
	}
	else if (Bound())
			MessageBox(IDERR_INVVERFORBOUNDFUNCS);
	else
		trgver = 0;
	if (trgver != -1)
	{
		for (int i=_FirstSelected; i<=_LastSelected; i++)
		{
			Function* pFunc = dynamic_cast<Function*>(_GetObject(i));
			pFunc->SetVersion(trgver);
		}
		_pEdit->Retrieve();
	}
}

void FuncGallery::_Bind()
{
	if (Bound())
		_Unbind();
	else
	{
		_version = 101;
		// make sure that all functions are version 1.01 at least
		for (int i=0; i<Items(); i++)
		{
			Function* pFunc = dynamic_cast<Function*>(_GetObject(i));
			if (pFunc->GetVersion()<101)
				pFunc->SetVersion(101);
		}
		_pEdit->Retrieve();
	}
	Generate();
}

void FuncGallery::_PostPaste(int pos)
{
	if (Bound())
	{
		Function* pFunc = static_cast<Function*>(_GetObject(pos));
		if (pFunc->GetVersion()<101)
			pFunc->SetVersion(101);
	}
}

void FuncGallery::_Unbind()
{
	_version = 0;
}


void FuncGallery::Generate() const
{
	switch (_version)
	{
	case 0 :
		_Generate0000();
		break;
	case 101 :
		_Generate0101();
		break;
	}
}


void FuncGallery::_Generate0101() const
{
	assert(Bound());
	if (Empty())
		return;
	WriteTextFile trg(__TEXT("functions.fset"));
	trg.WriteLn(__TEXT("funcgalleryver 1 1"));
	trg.PrintF(__TEXT("items: %d\n"), Items());
	for (int i=0; i<Items(); i++)
	{
		const Function* pFunc = dynamic_cast<const Function*>(GetObject(i));
		pFunc->Generate(trg);
	}
}

void FuncGallery::_Generate0000() const
{
	for (int i=0; i<Items(); i++)
	{
		const Function* pFunc = dynamic_cast<const Function*>(GetObject(i));
		TCHAR fname[37];
		OemToCharBuff(pFunc->GetName(), fname, 32);
		if (_tcscmp(fname, __TEXT("unnamed")))
		{
			fname[32] = 0;
			_tcscat(fname, __TEXT(".func"));
			WriteTextFile ffile(fname);
			pFunc->Generate(ffile);
		}
	}
}

void FuncGallery::LoadGallery(const TCHAR* fname)
{
	ReadTextFile src(fname);
	int vmaj, vmin;
	std::string line;
	src.Read(line);
	if (2 != sscanf(line.c_str(), "funcgalleryver %d %d", &vmaj, &vmin))
		throw Exception(IDERR_FUNCGALLERY, fname);
	_version = 100*vmaj+vmin;
	switch (_version)
	{
	case 101 :
		_Load0101(src);
		break;
	default :
		throw Exception(IDERR_FUNCGALLERYVER, vmaj, vmin, fname);
	}
}


void FuncGallery::_Load0101(ReadTextFile& src)
{
	std::string line;
	try
	{
		int n;
		src.Read(line);
		if (1 != sscanf(line.c_str(), "items: %d", &n))
			throw Exception(IDERR_FUNCGALLERY, src.Filename());
		if (n<1)
			throw Exception(IDERR_FUNCGALLERY, src.Filename());
		for (int i=0; i<n; i++)
		{
			Function* pNew = new Function;
			std::unique_ptr<EditableObject> New(pNew);
			pNew->Import(src);
			Add(New);
		}
		SetCurrent(0);
		Delete();
	}
	catch (Exception)
	{
		if (Items()>0)
		{
			SetCurrent(0);
			Delete();
		}
		throw;
	}
}

void FuncGallery::Unbind()
{
	_version = 0;
}


int FuncGallery::Count() const
{
	int res = 0;
	for (int i=0; i<Items(); i++)
	{
		const Function* pFunc = dynamic_cast<const Function*>(GetObject(i));
		if (pFunc->IsNamed())
			res++;
	}
	return res;
}


const TCHAR* FuncGallery::GetObjectName(int n) const
{
	assert(n>=0);
	assert(n<Items());
	const Function* pFunc = dynamic_cast<const Function*>(GetObject(n));
	return pFunc->GetName();
}



void FuncGallery::Add(std::unique_ptr<EditableObject>& pNew)
{
	Function* pS = dynamic_cast<Function*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::Add(pNew);
}

void FuncGallery::_UniqueName(Function* pS) const
{
	int id = 1;
	while (Contains(pS->GetName()))
	{
		if (0==strncmp("Copy", pS->GetName(), 4) && strlen(pS->GetName())>6)
		{
			const int BfSize = 64;
			char bf[BfSize+1];
			sprintf(bf, "Copy%02d", id);
			strcat(bf, pS->GetName()+6);
			pS->SetName(bf);
			++id;
		}
		else
		{
			std::string newname("CopyOf");
			newname.append(pS->GetName());
			pS->SetName(newname.c_str());
		}
	}
}


void FuncGallery::_Insert(std::unique_ptr<EditableObject>& pNew, int p)
{
	Function* pS = dynamic_cast<Function*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::_Insert(pNew, p);
}


bool FuncGallery::Contains(const char* nm) const
{
	for (int i=0; i<_items; ++i)
	{
		const Function* pS = dynamic_cast<const Function*>(_GetObject(i));
		if (pS->IsNamed(nm))
			return true;
	}
	return false;
}


void FuncGallery::Duplicate(int i, const char* nm)
{
	const Function* pF = dynamic_cast<const Function*>(GetObject(i));
	std::unique_ptr<EditableObject> pNew(pF->Clone());
	Function* pN = dynamic_cast<Function*>(pNew.get());
	pN->SetName(nm);
	Add(pNew);
}
