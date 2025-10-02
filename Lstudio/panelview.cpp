#include <fw.h>

#include "menuids.h"

#include "objfgvview.h"
#include "objfgvobject.h"

#include "panelvwtsk.h"

#include "panelprms.h"
#include "pnlitmsel.h"
#include "panelview.h"
#include "paneldesign.h"
#include "panelitem.h"

#include "resource.h"




void PanelView::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<PanelView>::Proc);
	wc.style |= CS_DBLCLKS;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.Register();
}



PanelView::PanelView(HWND hWnd, const CREATESTRUCT* pCS) : 
Scrollable(hWnd, pCS),
_SelectTask(this),
_AddSliderTask(this),
_AddButtonTask(this),
_AddLabelTask(this),
_AddGroupTask(this),
_DragItemTask(this),
_ExecuteTask(this),
_DragPen(RGB(64, 64, 64), Pen::psDot),
_MarkPen(RGB(0, 0, 255), Pen::psDot),
_font((HFONT)PanelParameters::PanelFont.HObj()),
_timer(this, eTimerId, eTimerTimeout, false)
{
	_pSink = 0;
	_pTask = &_SelectTask;
	_pObj = new PanelDesign;
	_dragging = false;
	if (-1 == PanelParameters::_AveCharWidth)
	{
		UpdateCanvas cnv(Hwnd());
		ObjectHolder sf(cnv, _font);
		TEXTMETRIC tm;
		GetTextMetrics(cnv, &tm);
		PanelParameters::_AveCharWidth = tm.tmAveCharWidth;
		PanelParameters::_CharHeight = tm.tmHeight;
	}
}


PanelView::~PanelView()
{}


void PanelView::CopyObject(const EditableObject* pObj)
{
	_selection.Clear();
	SetScroll(0, 0);

	ObjectView::CopyObject(pObj);
	assert(0 != _pSink);
	_GetDesign()->SetNotifySink(_pSink);
}

void PanelView::DoPaint(Canvas& cnv, const RECT&)
{
	ObjectHolder sf(cnv, _font);

	{
		const PanelDesign* pObj = _GetDesign();
		pObj->Draw(cnv);
	}

	if (!_IsExecuteMode())
		_selection.Draw(cnv);
}


void PanelView::SwitchToExecute()
{
	_pTask->Reset();
	_pTask = &_ExecuteTask;
	Ctrl::Invalidate();
}

void PanelView::SwitchToDesign()
{
	_GetDesign()->ResetValues();
	SwitchToSelect();
	Ctrl::Invalidate();
}

void PanelView::SwitchToSelect()
{
	_pTask = &_SelectTask;
	_pTask->Reset();
}


void PanelView::SwitchToAddSlider()
{
	_pTask->Reset();
	_pTask = &_AddSliderTask;
	_selection.Clear();
	Invalidate();
}

void PanelView::SwitchToAddButton()
{
	_pTask->Reset();
	_pTask = &_AddButtonTask;
	_selection.Clear();
	Invalidate();
}


void PanelView::SwitchToAddLabel()
{
	_pTask->Reset();
	_pTask = &_AddLabelTask;
	_selection.Clear();
	Invalidate();
}


void PanelView::SwitchToAddGroup()
{
	_pTask->Reset();
	_pTask = &_AddGroupTask;
	_selection.Clear();
	Invalidate();
}



void PanelView::StartDragging(POINT p1, POINT p2)
{
	assert(!_selection.IsEmpty());
	_pTask->Reset();
	RECT r; _selection.GetRect(r);
	_DragItemTask.StartDragging(p1, p2, r);
	_pTask = &_DragItemTask;
}


bool PanelView::MouseMove(KeyState ks, int x, int y)
{
	_pTask->MouseMove(ks, x+ScrollX(), y+ScrollY());
	return true;
}


bool PanelView::LButtonDown(KeyState ks, int x, int y)
{
	_dragging = true;
	GrabFocus();
	SetCapture(Hwnd());
	_pTask->LButtonDown(ks, x+ScrollX(), y+ScrollY());
	return true;
}

bool PanelView::LBDblClick(KeyState ks, int x, int y)
{
	_dragging = true;
	GrabFocus();
	SetCapture(Hwnd());
	_pTask->LBDblClick(ks, x+ScrollX(), y+ScrollY());
	return true;
}

bool PanelView::LButtonUp(KeyState ks, int x, int y)
{
	if (_dragging)
	{
		ReleaseCapture();
		_dragging = false;
	}
	_pTask->LButtonUp(ks, x+ScrollX(), y+ScrollY());
	_timer.Kill();
	return true;
}

bool PanelView::Timer(UINT id)
{
	if (id == _timer.Id())
	{
		POINT p = _ExecuteTask.LastPoint();
		UpdateCanvas cnv(Hwnd());
		cnv.SetOrigin(-ScrollX(), -ScrollY());
		ObjectHolder sf(cnv, _font);
		if (_GetDesign()->ExecuteDrag(cnv, p.x, p.y, false))
			_timer.Kill();
	}
	return true;
}

bool PanelView::CaptureChanged()
{
	_dragging = false;
	_timer.Kill();
	return true;
}

bool PanelView::KillFocus(HWND)
{
	_timer.Kill();
	_dragging = false;
	ClipCursor(0);
	return true;
}


void PanelView::AddSlider(int x, int y)
{
	PanelDesign* pObj = _GetDesign();
	if (pObj->CanAddItems())
	{
		pObj->AddSlider(x, y);
		_GetDesign()->UpdateRect();
		Invalidate();
	}
	else
		MessageBox(IDERR_PANELFULL);
}


void PanelView::AddButton(int x, int y)
{
	PanelDesign* pObj = _GetDesign();
	if (pObj->CanAddItems())
	{
		pObj->AddButton(x, y);
		_GetDesign()->UpdateRect();
		Invalidate();
	}
	else
		MessageBox(IDERR_PANELFULL);
}


void PanelView::AddLabel(int x, int y)
{
	PanelDesign* pObj = _GetDesign();
	if (pObj->CanAddItems())
	{
		pObj->AddLabel(x, y);
		_GetDesign()->UpdateRect();
		Invalidate();
	}
	else
		MessageBox(IDERR_PANELFULL);
}


void PanelView::AddGroup(POINT p1, POINT p2)
{
	PanelDesign* pObj = _GetDesign();
	try
	{
		if (pObj->CanAddItems())
		{
			pObj->AddGroup(p1, p2);
			_GetDesign()->UpdateRect();
			Invalidate();
		}
		else
			MessageBox(IDERR_PANELFULL);
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
}


void PanelView::ContextMenu(HWND, UINT x, UINT y)
{
	if (_IsExecuteMode())
	{
		MenuManipulator mm(App::theApp->GetContextMenu(PanelCtrlCMenu));
		_AdaptExecuteCMenu(mm);
		TrackPopupMenu(
			mm.Handle(),
			TPM_LEFTALIGN | TPM_TOPALIGN,
			x, y,
			0,
			Hwnd(),
			0);
	}
	else
	{
		HMENU hMenu = App::theApp->GetContextMenu(PanelDesignCMenu);
		TrackPopupMenu(
			hMenu,
			TPM_LEFTALIGN | TPM_TOPALIGN,
			x, y,
			0,
			Hwnd(),
			0);
	}
}



void PanelView::_AdaptExecuteCMenu(MenuManipulator& mm) const
{
	switch (_GetDesign()->PanelTarget())
	{
	case PanelParameters::ptLsystem :
		mm.Enable(ID_PANELCNTXT_NEWMODEL);
		mm.Enable(ID_PANELCNTXT_NEWLSYSTEM);
		mm.Disable(ID_PANELCNTXT_NEWVIEW);
		mm.Disable(ID_PANELCNTXT_RERUN);
		break;
	case PanelParameters::ptViewFile :
		mm.Enable(ID_PANELCNTXT_NEWMODEL);
		mm.Disable(ID_PANELCNTXT_NEWLSYSTEM);
		mm.Enable(ID_PANELCNTXT_NEWVIEW);
		mm.Disable(ID_PANELCNTXT_RERUN);
		break;
	case PanelParameters::ptFile :
		mm.Enable(ID_PANELCNTXT_NEWMODEL);
		mm.Enable(ID_PANELCNTXT_NEWLSYSTEM);
		mm.Enable(ID_PANELCNTXT_NEWVIEW);
		mm.Enable(ID_PANELCNTXT_RERUN);
		break;
	}
	mm.Disable(ID_PANELCNTXT_CLOSE);
}




bool PanelView::Command(int id, Window, UINT)
{
	switch (id)
	{
	// Design mode commands
	case ID_PANELDESIGN_APPLY :
		_ApplyNow();
		break;
	case ID_PANELDESIGN_RETRIEVEFROMGALLERY :
		_Retrieve();
		break;
	case ID_PANELDESIGN_ADDASNEW :
		_AddAsNew();
		break;
	// Execute mode commands
	case ID_PANELCNTXT_NEWMODEL :
		_NewModel();
		break;
	case ID_PANELCNTXT_NEWLSYSTEM :
		_NewLsystem();
		break;
	case ID_PANELCNTXT_NEWVIEW :
		_NewView();
		break;
	case ID_PANELCNTXT_RESET :
		_ResetPanel();
		break;
	case ID_PANELCNTXT_RERUN :
		_Rerun();
		break;
	}
	return true;
}


void PanelView::SetName(const char* nm)
{
	_GetDesign()->SetName(nm);
}

const char* PanelView::GetName() const
{
	return _GetDesign()->GetName();
}


bool PanelView::SelectAt(int x, int y)
{
	PanelItem* pSelection = _GetItemAt(x, y);
	if (0 != pSelection)
	{
		_selection.Clear();
		_selection.Add(pSelection);
		return true;
	}
	else
		return false;
}


bool PanelView::SelectAdd(int x, int y)
{
	PanelItem* pSelection = _GetItemAt(x, y);
	if (0 != pSelection)
	{
		_selection.Add(pSelection);
		return true;
	}
	else
		return false;
}

PanelItem* PanelView::_GetItemAt(int x, int y)
{
	return _GetDesign()->GetItemAt(x, y);
}



void PanelView::DrawDragRect(const RECT& r) const
{
	UpdateCanvas cnv(Hwnd());
	cnv.SetOrigin(-ScrollX(), -ScrollY());
	ObjectHolder sdp(cnv, _DragPen);
	SetROP2(cnv, R2_NOTXORPEN);
	cnv.Rectangle(r);
}


void PanelView::DrawDragRect(RECT& r, int dx, int dy) const
{
	UpdateCanvas cnv(Hwnd());
	cnv.SetOrigin(-ScrollX(), -ScrollY());
	ObjectHolder sdp(cnv, _DragPen);
	SetROP2(cnv, R2_NOTXORPEN);
	cnv.Rectangle(r);
	r.left += dx; r.right += dx;
	r.top += dy; r.bottom += dy;
	cnv.Rectangle(r);
}


void PanelView::DrawMark(POINT p1, POINT p2) const
{
	UpdateCanvas cnv(Hwnd());
	cnv.SetOrigin(-ScrollX(), -ScrollY());
	ObjectHolder smp(cnv, _MarkPen);
	SetROP2(cnv, R2_NOTXORPEN);
	cnv.Rectangle(p1, p2);
}

void PanelView::DrawMark(POINT p1, POINT p2, POINT p3) const
{
	UpdateCanvas cnv(Hwnd());
	cnv.SetOrigin(-ScrollX(), -ScrollY());
	ObjectHolder smp(cnv, _MarkPen);
	SetROP2(cnv, R2_NOTXORPEN);
	cnv.Rectangle(p1, p2);
	cnv.Rectangle(p1, p3);
}

void PanelView::SelectArea(POINT p1, POINT p2)
{
	if (p1.x>p2.x)
	{
		LONG x = p2.x;
		p2.x = p1.x;
		p1.x = x;
	}
	if (p1.y>p2.y)
	{
		LONG y = p2.y;
		p2.y = p1.y;
		p1.y = y;
	}

	_selection.Clear();
	RECT reg = { p1.x, p1.y, p2.x, p2.y };
	_GetDesign()->SelectArea(reg, _selection);
	Invalidate();
}



void PanelView::AlignLeft() 
{
	if (!_selection.IsEmpty())
	{
		_selection.AlignLeft();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::AlignRight() 
{
	if (!_selection.IsEmpty())
	{
		_selection.AlignRight();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::AlignTop() 
{
	if (!_selection.IsEmpty())
	{
		_selection.AlignTop();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::AlignBottom() 
{
	if (!_selection.IsEmpty())
	{
		_selection.AlignBottom();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::HCenter()
{
	if (!_selection.IsEmpty())
	{
		_selection.HCenter();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::VCenter()
{
	if (!_selection.IsEmpty())
	{
		_selection.VCenter();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::DistributeHorz()
{
	if (!_selection.IsEmpty())
	{
		_selection.DistributeHorz();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::DistributeVert()
{
	if (!_selection.IsEmpty())
	{
		_selection.DistributeVert();
		_selection.UpdateRect();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}

void PanelView::ItemProperties()
{
	if (!_selection.IsEmpty())
	{
		PanelItem* pItem = _selection.GetSelected(0);
		pItem->Properties(*this);

		// label may change size if renamed
		if (PanelParameters::pitLabel == pItem->Type())
		{
			_selection.UpdateRect();
			_GetDesign()->UpdateRect();
		}
		Invalidate();
	}
}


bool PanelView::SelectionEmpty() const
{
	return _selection.IsEmpty();
}


bool PanelView::InSelection(POINT pt) const
{
	if (_selection.IsEmpty())
		return false;
	return _selection.InSelection(pt);
}


void PanelView::DuplicateDown()
{
	if (_selection.IsEmpty())
		MessageBox(IDERR_NOTHINGSELECTEDTODUPL);
	else
	{
		PanelDesign* pDesign = _GetDesign();
		int cnt = _selection.Items();
		if (pDesign->CanAddItems(cnt))
		{
			PanelItemsSelection tmpsel;
			RECT r;
			_selection.GetRect(r);
			int dy = r.bottom - r.top + 24;
			for (int i=0; i<cnt; i++)
			{
				const PanelItem* pItm = _selection.GetSelected(i);
				if (PanelParameters::pitGroup!=pItm->Type())
					tmpsel.Add(pDesign->Duplicate(pItm, 0, dy));
			}
			_selection.Clear();
			_selection.TransferSelection(tmpsel);
			_GetDesign()->UpdateRect();
			Invalidate();
		}
		else
		{
			MessageBox(IDERR_TOOMANYITEMSTODUPL);
		}
	}
}

void PanelView::DuplicateRight()
{
	if (_selection.IsEmpty())
		MessageBox(IDERR_NOTHINGSELECTEDTODUPL);
	else
	{
		PanelDesign* pDesign = _GetDesign();
		int cnt = _selection.Items();
		if (pDesign->CanAddItems(cnt))
		{
			PanelItemsSelection tmpsel;
			RECT r;
			_selection.GetRect(r);
			int dx = r.right - r.left + 24;
			for (int i=0; i<cnt; i++)
			{
				const PanelItem* pItm = _selection.GetSelected(i);
				if (PanelParameters::pitGroup!=pItm->Type())
					tmpsel.Add(pDesign->Duplicate(pItm, dx, 0));
			}
			_selection.Clear();
			_selection.TransferSelection(tmpsel);
			_GetDesign()->UpdateRect();
			Invalidate();
		}
		else
		{
			MessageBox(IDERR_TOOMANYITEMSTODUPL);
		}
	}
}



void PanelView::Delete()
{
	if (_selection.IsEmpty())
		MessageBox(IDERR_NOTHINGSELECTEDTODELETE);
	else
	{
		PanelDesign* pDesign = _GetDesign();
		for (int i=0; i<_selection.Items(); i++)
		{
			PanelItem* pItm = _selection.GetSelected(i);
			int id = pDesign->FindItem(pItm);
			assert(-1 != id);
			pDesign->DeleteItem(id);
		}
		_selection.Clear();
		_GetDesign()->UpdateRect();
		Invalidate();
	}
}


void PanelView::MoveBy(int x, int y)
{
	assert(!_selection.IsEmpty());
	_selection.MoveBy(x, y);
	_GetDesign()->UpdateRect();
}


PanelParameters::PanelTarget PanelView::PanelTarget() const
{
	return _GetDesign()->PanelTarget();
}

const char* PanelView::PanelTargetFilename() const
{
	return _GetDesign()->PanelTargetFilename();
}

void PanelView::SetPanelTarget(PanelParameters::PanelTarget target)
{
	_GetDesign()->SetPanelTarget(target);
}

void PanelView::SetPanelTargetFilename(const Window& w)
{
	std::string fnm;
	w.GetText(fnm);
	_GetDesign()->SetPanelTargetFilename(fnm.c_str());
}



int PanelView::MaxScrollX() const
{
	return max(0, _GetDesign()->GetWidth()+eMargin-Width());
}


int PanelView::MaxScrollY() const
{
	return max(0, _GetDesign()->GetHeight()+eMargin-Height());
}


int PanelView::MaxX() const
{
	return _GetDesign()->GetWidth()+eMargin;
}


int PanelView::MaxY() const
{
	return _GetDesign()->GetHeight()+eMargin;
}

PanelParameters::TriggerCommand PanelView::Trigger() const
{
	return _GetDesign()->Trigger();
}

void PanelView::SetTrigger(PanelParameters::TriggerCommand trigger)
{
	_GetDesign()->SetTrigger(trigger);
}

bool PanelView::IsValidTrigger(int id) const
{
	if (id == PanelParameters::tcNewLsystem)
		return true;
	if (id == PanelParameters::tcNewModel)
		return true;
	if (id == PanelParameters::tcNewView)
		return true;
	if (id == PanelParameters::tcRerun)
		return true;
	if (id == PanelParameters::tcNothing)
		return true;
	return false;
}

RECT PanelView::ExecuteHit(int x, int y)
{
	RECT r = { 0, 0, 0, 0 };
	try
	{
		UpdateCanvas cnv(Hwnd());
		cnv.SetOrigin(-ScrollX(), -ScrollY());
		ObjectHolder sf(cnv, _font);
		r = _GetDesign()->ExecuteHit(cnv, x, y);
		r.left -= ScrollX();
		r.right -= ScrollX();
		r.top -= ScrollY();
		r.bottom -= ScrollY();
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return r;
}

void PanelView::ExecuteDrag(int x, int y, bool final)
{
	try
	{
		_timer.Kill();

		UpdateCanvas cnv(Hwnd());
		cnv.SetOrigin(-ScrollX(), -ScrollY());
		ObjectHolder sf(cnv, _font);

		if (!_GetDesign()->ExecuteDrag(cnv, x, y, final))
			_timer.Start();
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
}



int PanelView::GetDlgCode()
{
	return DLGC_WANTARROWS;
}

bool PanelView::KeyDown(UINT vk)
{
	if (!_IsExecuteMode() && !_selection.IsEmpty())
	{
		SHORT state = GetKeyState(VK_CONTROL);
		int step = 1;
		if (0x8000 & state)
			step = 10;
		switch (vk)
		{
		case VK_UP :
			MoveBy(0, -step);
			Invalidate();
			break;
		case VK_DOWN :
			MoveBy(0, step);
			Invalidate();
			break;
		case VK_LEFT :
			MoveBy(-step, 0);
			Invalidate();
			break;
		case VK_RIGHT :
			MoveBy(step, 0);
			Invalidate();
			break;
		case VK_NEXT :
			MoveBy(0, 10);
			Invalidate();
			break;
		case VK_PRIOR :
			MoveBy(0, -10);
			Invalidate();
			break;
		}
	}
	return true;
}


void PanelView::_ResetPanel()
{
	UpdateCanvas cnv(Hwnd());
	cnv.SetOrigin(-ScrollX(), -ScrollY());
	ObjectHolder sf(cnv, _font);
	_GetDesign()->DefaultValues(cnv);
}


void PanelView::_NewLsystem()
{
	_GetDesign()->NewLsystem(true);
}

void PanelView::_NewView()
{
	_GetDesign()->NewView(true);
}

void PanelView::_NewModel()
{
	_GetDesign()->NewModel(true);
}

void PanelView::_Rerun()
{
	_GetDesign()->Rerun(true);
}


PanelDesign* PanelView::_GetDesign()
{ return dynamic_cast<PanelDesign*>(_pObj); }


const PanelDesign* PanelView::_GetDesign() const
{ return dynamic_cast<const PanelDesign*>(_pObj); }

