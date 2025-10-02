#include <memory>
#include <vector>


#include <fw.h>
#include <glfw.h>


#include "resource.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"
#include "glgallery.h"

#include "contourgallery.h"
#include "contour.h"
#include "contouredit.h"
#include "menuids.h"
#include "lstudioptns.h"
#include "knot.h"

UINT ContourGallery::_ClipboardFormat = 0;

void ContourGallery::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<ContourGallery>::Proc);
	wc.style = 0;
	wc.Register();
	_ClipboardFormat = RegisterClipboardFormat(_ClipboardFormatName());
}



ContourGallery::ContourGallery(HWND hwnd, const CREATESTRUCT* pCS) : GLGallery(hwnd, pCS, 2.0, 8, true)
{
	std::unique_ptr<EditableObject> pNew(new Contour);
	Add(pNew);
	_version = 0;
}


ContourGallery::~ContourGallery()
{
}



EditableObject* ContourGallery::_NewObject() const
{
	return new Contour;
}


bool ContourGallery::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_CONTOURGALLERY_NEW : 
			_New();
			break;
		case ID_CONTOURGALLERY_CUT :
			_Cut();
			break;
		case ID_CONTOURGALLERY_DELETE :
			if (!options.WarnGalleryDelete() || MessageYesNo(IDS_DELETEFROMGALLERY))
				_Delete();
			break;
		case ID_CONTOURGALLERY_COPY :
			_Copy();
			break;
		case ID_CONTOURGALLERY_PASTE_BEFORE :
			_PasteBefore();
			break;
		case ID_CONTOURGALLERY_PASTE_AFTER :
			_PasteAfter();
			break;
		case ID_CONTOURGALLERY_BIND :
			if (Bound())
				_Unbind();
			else
				_Bind();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void ContourGallery::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(ContourGalleryCMenu);
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


void ContourGallery::_AdaptMenu(HMENU hMenu) const
{
	ResString caption(64, (1==_items) ? IDS_RESET : IDS_DELETE);
	MenuManipulator mm(hMenu);
	mm.SetText(ID_CONTOURGALLERY_DELETE, caption);
	if (IsClipboardFormatAvailable(_ClipboardFormat))
	{
		mm.Enable(ID_CONTOURGALLERY_PASTE_BEFORE);
		mm.Enable(ID_CONTOURGALLERY_PASTE_AFTER);
	}
	else
	{
		mm.Disable(ID_CONTOURGALLERY_PASTE_BEFORE);
		mm.Disable(ID_CONTOURGALLERY_PASTE_AFTER);
	}
	if (Bound())
		mm.Check(ID_CONTOURGALLERY_BIND);
	else
		mm.Uncheck(ID_CONTOURGALLERY_BIND);
}



const TCHAR* ContourGallery::_GetObjectName(int i) const
{
	assert(i>=0);
	assert(i<Items());
	const Contour* pCnt = dynamic_cast<const Contour*>(GetObject(i));
	static TCHAR res[80];
	OemToCharBuff(pCnt->GetName(), res, 79);
	res[79] = 0;
	return res;
}


void ContourGallery::Add(std::unique_ptr<EditableObject>& pNew)
{
	Contour* pS = dynamic_cast<Contour*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::Add(pNew);
}

void ContourGallery::_UniqueName(Contour* pS) const
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


void ContourGallery::_Insert(std::unique_ptr<EditableObject>& pNew, int p)
{
	Contour* pS = dynamic_cast<Contour*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::_Insert(pNew, p);
}


bool ContourGallery::Contains(const char* nm) const
{
	for (int i=0; i<_items; ++i)
	{
		const Contour* pS = dynamic_cast<const Contour*>(_GetObject(i));
		if (pS->IsNamed(nm))
			return true;
	}
	return false;
}



void ContourGallery::_SetObjectsVersion(int v)
{
	if (Bound() && v<101)
	{
		MessageBox(IDERR_INVVERFORBOUNDCONTOURS);
	}
	else
	{
		for (int i=_FirstSelected; i<=_LastSelected; i++)
		{
			Contour* pCont = dynamic_cast<Contour*>(_GetObject(i));
			pCont->SetVersion(v);
		}
		_pEdit->Retrieve();
	}
}

void ContourGallery::_Bind()
{
	_version = 101;
	// make sure that all functions are version 1.01 at least
	for (int i=0; i<Items(); i++)
	{
		Contour* pContour = dynamic_cast<Contour*>(_GetObject(i));
		if (pContour->GetVersion()<101)
			pContour->SetVersion(101);
	}
	_pEdit->Retrieve();
}


void ContourGallery::_PostPaste(int pos)
{
	if (Bound())
	{
		Contour* pContour = dynamic_cast<Contour*>(_GetObject(pos));
		if (pContour->GetVersion()<101)
			pContour->SetVersion(101);
	}
}


void ContourGallery::_Unbind()
{
	_version = 0;
}

void ContourGallery::_CurveFormat()
{
	const Contour* pContour = dynamic_cast<const Contour*>(_GetObject(_current));
	int fver = pContour->GetVersion();
	if (0==fver)
		_SetObjectsVersion(10);
	else
		_SetObjectsVersion(0);
}

void ContourGallery::Generate() const
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


void ContourGallery::_Generate0000() const
{
	assert(!"Not implemented yet");
}


void ContourGallery::_Generate0101() const
{
	assert(Bound());
	if (Empty())
		return;
	WriteTextFile trg(__TEXT("contours.cset"));
	trg.WriteLn(__TEXT("contourgalleryver 1 1"));
	trg.PrintF(__TEXT("items: %d\n"), Items());
	for (int i=0; i<Items(); i++)
	{
		const Contour* pContour = dynamic_cast<const Contour*>(_GetObject(i));
		pContour->Generate(trg);
	}
}


bool ContourGallery::Empty() const
{
	for (int i=0; i<Items(); i++)
	{
		const Contour* pContour = dynamic_cast<const Contour*>(_GetObject(i));
		if (pContour->IsNamed())
			return false;
	}
	return true;
}



void ContourGallery::LoadGallery(const TCHAR* fname)
{
	ReadTextFile src(fname);
	int vmaj, vmin;
	std::string line;
	src.Read(line);
	if (2 != sscanf(line.c_str(), "contourgalleryver %d %d", &vmaj, &vmin))
		throw Exception(IDERR_CONTOURGALLERY, fname);
	_version = 100*vmaj+vmin;
	switch (_version)
	{
	case 101 :
		_Load0101(src);
		break;
	default :
		throw Exception(IDERR_CONTOURGALLERYVER, vmaj, vmin, fname);
	}
}


void ContourGallery::_Load0101(ReadTextFile& src)
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
			Contour* pNew = new Contour;
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
