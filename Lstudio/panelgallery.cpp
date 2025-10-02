#include <memory>

#include <fw.h>

#include "objfgvgallery.h"
#include "objfgvobject.h"
#include "objfgvedit.h"

#include "panelprms.h"
#include "panelgallery.h"
#include "paneldesign.h"

#include "menuids.h"
#include "lstudioptns.h"

#include "resource.h"

UINT PanelGallery::_ClipboardFormat = 0;

void PanelGallery::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<PanelGallery>::Proc);
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.style |= CS_DBLCLKS;
	wc.Register();
}





PanelGallery::PanelGallery(HWND hWnd, const CREATESTRUCT* pCS) : Ctrl(hWnd, pCS)
{
	_pNotifySink = 0;
	_designmode = false;
	_FirstVisible = 0;
	std::unique_ptr<EditableObject> pNew(new PanelDesign);
	Add(pNew);
}


PanelGallery::~PanelGallery()
{}


bool PanelGallery::Size(SizeState, int, int)
{
	_UpdateScrollbar();
	return true;
}

void PanelGallery::_UpdateScrollbar()
{
	if (Height()>0)
	{
		SCROLLINFO si;
		{
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
			si.nMin = 0;
			si.nMax = _items-1;
			si.nPage = Width() / Height();
			si.nPos = _FirstVisible;
		}
		SetScrollInfo(Hwnd(), SB_HORZ, &si, true);
	}
}



int PanelGallery::_DetermineSelection(int x, int) const
{
	const int ItemWidth = Height();
	int res = x / ItemWidth + _FirstVisible;
	if (res>=Items())
		res = -1;
	return res;
}


bool PanelGallery::Paint()
{
	PaintCanvas pc(Hwnd());
	int x = 0;
	int n = _FirstVisible;
	const int ItemWidth = Height();
	while ((x<Width()) && (n<_items))
	{
		pc.SetOrigin(x, 0);
		pc.ClipRect(1, 1, ItemWidth-2, Height()-2);
		const PanelDesign* pDsgn = reinterpret_cast<const PanelDesign*>(_arr[n].get());
		pDsgn->DrawInGallery(ItemWidth, Height(), pc);
		pc.ClearClip();
		if (_current == n)
		{
			Pen wp(RGB(255, 255, 255), 2);
			ObjectHolder swp(pc, wp);
			pc.Rectangle(1, 1, ItemWidth-1, Height()-2);
		}
		else if ((n>=_FirstSelected) && (n<=_LastSelected))
		{
			Pen gp(RGB(153, 153, 153), 2);
			ObjectHolder sgp(pc, gp);
			pc.Rectangle(1, 1, ItemWidth-1, Height()-2);
		}
		x += ItemWidth;
		n++;
	}
	return true;
}


bool PanelGallery::LButtonDown(KeyState ks, int x, int y)
{
	assert(0 != _pEdit);
	int newselection = _DetermineSelection(x, y);
	if (-1 != newselection)
	{
		if (ks.IsShift())
		{
			if (newselection<_current)
			{
				_FirstSelected = newselection;
				_LastSelected = _current;
				ForceRepaint();
			}
			else if (newselection>_current)
			{
				_FirstSelected = _current;
				_LastSelected = newselection;
				ForceRepaint();
			}
			else
			{
				_FirstSelected = _LastSelected = _current;
			}
		}
		else
		{
			_FirstSelected = _LastSelected = newselection;
			if (newselection == _current)
				_pEdit->Retrieve();
			else
				_pEdit->SelectionChanged(_current, newselection);
		}
	}
	return true;
}

bool PanelGallery::LBDblClick(KeyState, int x, int y)
{
	assert(0 != _pEdit);
	int newselection = _DetermineSelection(x, y);
	if (-1 != newselection)
	{
		if (!_designmode)
			_Execute();
	}
	return true;
}



EditableObject* PanelGallery::_NewObject() const
{ 
	return new PanelDesign;
}



bool PanelGallery::HScroll(HScrollCode code, int pos, HWND)
{
	switch (code)
	{
	case hscLineLeft :
		if (_FirstVisible>0)
		{
			_FirstVisible--;
			_UpdateScrollbar();
			ForceRepaint();
		}
		break;
	case hscLineRight :
		if (_FirstVisible<_items - Width()/Height())
		{
			_FirstVisible++;
			_UpdateScrollbar();
			ForceRepaint();
		}
		break;
	case hscThumbPosition :
	case hscThumbTrack :
		_FirstVisible = pos;
		_UpdateScrollbar();
		ForceRepaint();
		break;
	case hscPageLeft :
		_FirstVisible -= Width()/Height();
		if (_FirstVisible<0)
			_FirstVisible = 0;
		_UpdateScrollbar();
		ForceRepaint();
		break;
	case hscPageRight :
		_FirstVisible += Width()/Height();
		if (_FirstVisible>_items - Width()/Height())
			_FirstVisible = _items - Width()/Height();
		_UpdateScrollbar();
		ForceRepaint();
		break;
	case hscLeft :
		if (_FirstVisible != 0)
		{
			_FirstVisible = 0;
			_UpdateScrollbar();
			ForceRepaint();
		}
		break;
	case hscRight :
		_FirstVisible = _items - Width()/Height();
		_UpdateScrollbar();
		ForceRepaint();
		break;
	}
	return true;
}


void PanelGallery::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(PanelGalleryCMenu);
	_AdaptMenu(hMenu);
	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}


void PanelGallery::_AdaptMenu(HMENU hMenu)
{
	// Delete/reset item
	ResString caption(64, (1==_items) ? IDS_RESET : IDS_DELETE);
	MenuManipulator mm(hMenu);
	mm.SetText(ID_PANELGALLERY_DELETE, caption);
	if (_designmode)
	{
		mm.Enable(ID_PANELGALLERY_NEW);
		mm.Enable(ID_PANELGALLERY_DELETE);
		mm.Disable(ID_PANELGALLERY_EXECUTE);
		mm.Disable(ID_PANELGALLERY_EXECUTEALL);
	}
	else
	{
		mm.Disable(ID_PANELGALLERY_NEW);
		mm.Disable(ID_PANELGALLERY_DELETE);
		mm.Enable(ID_PANELGALLERY_EXECUTE);
		mm.Enable(ID_PANELGALLERY_EXECUTEALL);
	}
}

void PanelGallery::TearOffAll()
{
	_ExecuteAll();
}

bool PanelGallery::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_PANELGALLERY_NEW :
			_New();
			break;
		case ID_PANELGALLERY_DELETE :
			if (!options.WarnGalleryDelete() || MessageYesNo(IDS_DELETEFROMGALLERY))
				_Delete();
			break;
		case ID_PANELGALLERY_EXECUTE :
			_Execute();
			break;
		case ID_PANELGALLERY_EXECUTEALL :
			_ExecuteAll();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}



void PanelGallery::_Execute()
{
	EditableObject* pObj = _arr[_current].get();
	PanelDesign* pDsgn = dynamic_cast<PanelDesign*>(pObj);
	assert(0 != _pNotifySink);
	pDsgn->SetNotifySink(_pNotifySink);
	pDsgn->Execute(Hwnd());
}


void PanelGallery::_ExecuteAll()
{
	for (int i=0; i<Items(); i++)
	{
		EditableObject* pObj = _arr[i].get();
		PanelDesign* pDsgn = dynamic_cast<PanelDesign*>(pObj);
		assert(0 != _pNotifySink);
		pDsgn->SetNotifySink(_pNotifySink);
		pDsgn->Execute(Hwnd());
	}
}


bool PanelGallery::TearedOffPanels() const
{
	for (int i=0; i<Items(); i++)
	{
		const EditableObject* pObj = _arr[i].get();
		const PanelDesign* pDsgn = dynamic_cast<const PanelDesign*>(pObj);
		if (pDsgn->IsTearedOff())
			return true;
	}
	return false;
}


void PanelGallery::CloseTearedOff()
{
	for (int i=0; i<Items(); i++)
	{
		EditableObject* pObj = _arr[i].get();
		PanelDesign* pDsgn = dynamic_cast<PanelDesign*>(pObj);
		pDsgn->CloseTearedOff();
	}
}


void PanelGallery::Add(std::unique_ptr<EditableObject>& pNew)
{
	PanelDesign* pS = dynamic_cast<PanelDesign*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::Add(pNew);
}

void PanelGallery::_UniqueName(PanelDesign* pS) const
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


void PanelGallery::_Insert(std::unique_ptr<EditableObject>& pNew, int p)
{
	PanelDesign* pS = dynamic_cast<PanelDesign*>(pNew.get());
	_UniqueName(pS);
	ObjectGallery::_Insert(pNew, p);
}


bool PanelGallery::Contains(const char* nm) const
{
	for (int i=0; i<_items; ++i)
	{
		const PanelDesign* pS = dynamic_cast<const PanelDesign*>(_GetObject(i));
		if (pS->IsNamed(nm))
			return true;
	}
	return false;
}
