#include <memory>

#include <iostream>
#include <fw.h>
#include <glfw.h>

#include "menuids.h"
#include "lstudioptns.h"
#include "resource.h"

#include "objfgvgallery.h"
#include "objfgvobject.h"
#include "glgallery.h"

#include "curvegallery.h"
#include "curveXYZv.h"


void CurveGallery::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<CurveGallery>::Proc);
	wc.Register();
}


CurveGallery::CurveGallery(HWND hwnd, const CREATESTRUCT* pCS) : GLGallery(hwnd, pCS, 2.2f, 8)
{
	std::unique_ptr<EditableObject> pNew(new CurveXYZView);
	Add(pNew);
}


EditableObject* CurveGallery::_NewObject() const
{
	return new CurveXYZView;
}


bool CurveGallery::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_CURVEGALLERY_NEW : 
			_New();
			break;
		case ID_CURVEGALLERY_DELETE :
			if (!options.WarnGalleryDelete() || MessageYesNo(IDS_DELETEFROMGALLERY))
				_Delete();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void CurveGallery::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(CurveGalleryCMenu);
	MenuManipulator mm(hMenu);
	ResString lbl(64, IDS_DELETE);
	if (Items()==1)
		lbl.Load(IDS_RESET);
	mm.SetText(ID_CURVEGALLERY_DELETE, lbl.c_str());
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
