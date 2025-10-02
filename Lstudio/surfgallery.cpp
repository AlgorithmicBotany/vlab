#include <memory>

#include <fw.h>
#include <glfw.h>


#include "resource.h"

#include "objfgvobject.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"
#include "glgallery.h"

#include "patchclrinfo.h"
#include "patch.h"
#include "surface.h"
#include "surfgallery.h"
#include "linethcb.h"
#include "surfthumbcb.h"
#include "surfaceedit.h"
#include "menuids.h"
#include "lstudioptns.h"


UINT SurfaceGallery::_ClipboardFormat = 0;

void SurfaceGallery::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<SurfaceGallery>::Proc);
	wc.style = 0;
	wc.Register();
	_ClipboardFormat = RegisterClipboardFormat(_ClipboardFormatName());
}



SurfaceGallery::SurfaceGallery(HWND hwnd, const CREATESTRUCT* pCS) : GLGallery(hwnd, pCS, 4.0, 8, true)
{
	std::unique_ptr<EditableObject> pNew(new Surface);
	Add(pNew);
}


SurfaceGallery::~SurfaceGallery()
{
}



EditableObject* SurfaceGallery::_NewObject() const
{
	return new Surface;
}


bool SurfaceGallery::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_SURFACEGALLERY_NEW : 
			_New();
			break;
		case ID_SURFACEGALLERY_CUT :
			_Cut();
			break;
		case ID_SURFACEGALLERY_DELETE :
			if (!options.WarnGalleryDelete() || MessageYesNo(IDS_DELETEFROMGALLERY))
				_Delete();
			break;
		case ID_SURFACEGALLERY_COPY :
			_Copy();
			break;
		case ID_SURFACEGALLERY_PASTE_BEFORE :
			_PasteBefore();
			break;
		case ID_SURFACEGALLERY_PASTE_AFTER :
			_PasteAfter();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void SurfaceGallery::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(SurfaceGalleryCMenu);
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


void SurfaceGallery::_AdaptMenu(HMENU hMenu) const
{
	ResString caption(64, (1==_items) ? IDS_RESET : IDS_DELETE);
	MenuManipulator mm(hMenu);
	mm.SetText(ID_SURFACEGALLERY_DELETE, caption);
	if (IsClipboardFormatAvailable(_ClipboardFormat))
	{
		mm.Enable(ID_SURFACEGALLERY_PASTE_BEFORE);
		mm.Enable(ID_SURFACEGALLERY_PASTE_AFTER);
	}
	else
	{
		mm.Disable(ID_SURFACEGALLERY_PASTE_BEFORE);
		mm.Disable(ID_SURFACEGALLERY_PASTE_AFTER);
	}
}




const TCHAR* SurfaceGallery::_GetObjectName(int i) const
{
	assert(i>=0);
	assert(i<Items());
	const Surface* pFunc = dynamic_cast<const Surface*>(GetObject(i));
	static TCHAR res[80];
	OemToCharBuff(pFunc->GetName(), res, 79);
	res[79] = 0;
	return res;
}


void SurfaceGallery::Add(std::unique_ptr<EditableObject>& pNew)
{
	Surface* pS = dynamic_cast<Surface*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::Add(pNew);
}

void SurfaceGallery::_UniqueName(Surface* pS) const
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


void SurfaceGallery::_Insert(std::unique_ptr<EditableObject>& pNew, int p)
{
	Surface* pS = dynamic_cast<Surface*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::_Insert(pNew, p);
}


bool SurfaceGallery::Contains(const char* nm) const
{
	for (int i=0; i<_items; ++i)
	{
		const Surface* pS = dynamic_cast<const Surface*>(_GetObject(i));
		if (pS->IsNamed(nm))
			return true;
	}
	return false;
}

