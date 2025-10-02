#include <memory>
#include <vector>

#include <fw.h>
#include <glfw.h>

#include "resource.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "glgallery.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "contour.h"
#include "contouredit.h"

#include "gridviewtask.h"
#include "gridview.h"
#include "contourvwtsk.h"
#include "contourview.h"
#include "contourgallery.h"



HWND ContourEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pNotifySink)
{
	class ContourEditCreator : public Creator
	{
	public:
		ContourEditCreator(PrjNotifySink* pNotifySink) : _pNotifySink(pNotifySink)
		{}
		FormCtrl* Create(HWND hDlg)
		{ return new ContourEdit(hDlg, GetWindowInstance(hDlg), _pNotifySink); }
	private:
		PrjNotifySink* _pNotifySink;
	};

	ContourEditCreator creator(pNotifySink);
	return CreateDialogParam
		(
		hInst,
		MAKEINTRESOURCE(IDD_CONTOUR),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}


ContourEdit::ContourEdit(HWND hwnd, HINSTANCE hInst, PrjNotifySink* pNotifySink) : 
ContModeEdit(hwnd, pNotifySink),
_moveButton(hInst, MAKEINTRESOURCE(IDI_MOVEPOINT)),
_addButton(hInst, MAKEINTRESOURCE(IDI_ADDPOINT)),
_deleteButton(hInst, MAKEINTRESOURCE(IDI_DELETEPOINT)),
_setcoordButton(hInst, MAKEINTRESOURCE(IDI_ICON3)),
_pointsButton(hInst, MAKEINTRESOURCE(IDI_POINTS)),
_linesButton(hInst, MAKEINTRESOURCE(IDI_SEGMENTS)),
_curveButton(hInst, MAKEINTRESOURCE(IDI_CURVE)),
_axisButton(hInst, MAKEINTRESOURCE(IDI_AXIS)),
_gridButton(hInst, MAKEINTRESOURCE(IDI_GRID)),
_labelsButton(hInst, MAKEINTRESOURCE(IDI_LABELS))
#ifdef CONTOUREDITRESIZABLE
,
_Gallery(GetDlgItem(IDC_GALLERY)),
_Name(GetDlgItem(IDC_NAME)),
_NameLbl(GetDlgItem(IDC_NAMELBL)),
_ViewWnd(GetDlgItem(IDC_VIEW)),
_EditLbl(GetDlgItem(IDC_EDITLBL)),
_MoveBtn(GetDlgItem(IDC_MOVE)),
_AddBtn(GetDlgItem(IDC_ADD)),
_RemoveBtn(GetDlgItem(IDC_REMOVE)),
_InputPBtn(GetDlgItem(IDC_EDITCOORD)),
_DispLbl(GetDlgItem(IDC_DISPLBL)),
_PointsBtn(GetDlgItem(IDC_POINTS)),
_SegmentsBtn(GetDlgItem(IDC_SEGMENTS)),
_CurveBtn(GetDlgItem(IDC_CURVE)),
_AxisBtn(GetDlgItem(IDC_AXIS)),
_GridBtn(GetDlgItem(IDC_GRID)),
_LabelsBtn(GetDlgItem(IDC_LABELS)),
_ClosedChk(GetDlgItem(IDC_CLOSED)),
_SmplsLbl(GetDlgItem(IDC_SMPLSLBL)),
_SmplsEdt(GetDlgItem(IDC_SAMPLES)),
_BTypeLbl(GetDlgItem(IDC_TYPELBL)),
_BType(GetDlgItem(IDC_TYPECB)),
#endif
_tooltips(Hwnd(), hInst)
{
	_theView = reinterpret_cast<ContourView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_pView = reinterpret_cast<ContourView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_pView->SetEdit(this);
	_theView->SetContourEdit(this);
	_theGallery = reinterpret_cast<ContourGallery*>(GetDlgItem(IDC_GALLERY).GetPtr());
	_pGallery = _theGallery;
	_pGallery->SetEdit(this);
	const Contour* pContour = dynamic_cast<const Contour*>(_pGallery->GetObject(0));
	_pView->CopyObject(pContour);

	_UpdateControls();

	{
		Button button(GetDlgItem(IDC_MOVE));
		button.SetIcon(_moveButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDC_MOVE, hInst);

		button.Reset(GetDlgItem(IDC_ADD));
		button.SetIcon(_addButton);
		button.SetCheck(false);
		_tooltips.Add(button, IDC_ADD, hInst);

		button.Reset(GetDlgItem(IDC_REMOVE));
		button.SetIcon(_deleteButton);
		button.SetCheck(false);
		_tooltips.Add(button, IDC_REMOVE, hInst);

		button.Reset(GetDlgItem(IDC_EDITCOORD));
		button.SetIcon(_setcoordButton);
		button.SetCheck(false);
		_tooltips.Add(button, IDC_EDITCOORD, hInst);

		_theView->SwitchMovePointMode();

		
		button.Reset(GetDlgItem(IDC_POINTS));
		button.SetIcon(_pointsButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDC_POINTS, hInst);
		_theView->SetDrawPoints();
		
		button.Reset(GetDlgItem(IDC_SEGMENTS));
		button.SetIcon(_linesButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDC_SEGMENTS, hInst);
		_theView->SetDrawSegments();
		
		button.Reset(GetDlgItem(IDC_CURVE));
		button.SetIcon(_curveButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDC_CURVE, hInst);
		_theView->SetDrawCurve();
		
		button.Reset(GetDlgItem(IDC_AXIS));
		button.SetIcon(_axisButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDC_AXIS, hInst);
		_theView->SetDrawAxis();

		button.Reset(GetDlgItem(IDC_GRID));
		button.SetIcon(_gridButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDC_GRID, hInst);
		_theView->SetDrawGrid();

		button.Reset(GetDlgItem(IDC_LABELS));
		button.SetIcon(_labelsButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDS_LABELSTT, hInst);
		_theView->SetDrawLabels();
	}

	{
		ComboBox type(GetDlgItem(IDC_TYPECB));
		ResString lbl(32, IDS_SPLREGULAR);
		type.AddString(lbl);
		lbl.Load(IDS_SPLENDPTINT);
		type.AddString(lbl);
		type.SetCurSel(0);
	}

	_Filename[0] = 0;

#ifdef CONTOUREDITRESIZABLE
	{
		RECT dlgrect;
		GetWindowRect(dlgrect);
		_Gallery.SetLeft(dlgrect);
		_Gallery.SetBottom(dlgrect);
		_Gallery.SetRight(dlgrect);
		_Gallery.SetHeight();
		_Gallery.SetMinWidth();
		_Gallery.SetMinTop(dlgrect);

		_Name.SetRight(dlgrect);
		_Name.SetBottom(dlgrect);
		_Name.SetWidth();
		_Name.SetHeight();
		_Name.SetMinTop(dlgrect);
		_Name.SetMinLeft(dlgrect);

		_NameLbl.SetRight(dlgrect);
		_NameLbl.SetBottom(dlgrect);
		_NameLbl.SetWidth();
		_NameLbl.SetHeight();
		_NameLbl.SetMinTop(dlgrect);
		_NameLbl.SetMinLeft(dlgrect);

		_EditLbl.SetRight(dlgrect);
		_EditLbl.SetTop(dlgrect);
		_EditLbl.SetWidth();
		_EditLbl.SetHeight();
		_EditLbl.SetMinLeft(dlgrect);

		_MoveBtn.SetRight(dlgrect);
		_MoveBtn.SetTop(dlgrect);
		_MoveBtn.SetWidth();
		_MoveBtn.SetHeight();
		_MoveBtn.SetMinLeft(dlgrect);

		_AddBtn.SetRight(dlgrect);
		_AddBtn.SetTop(dlgrect);
		_AddBtn.SetWidth();
		_AddBtn.SetHeight();
		_AddBtn.SetMinLeft(dlgrect);

		_RemoveBtn.SetRight(dlgrect);
		_RemoveBtn.SetTop(dlgrect);
		_RemoveBtn.SetWidth();
		_RemoveBtn.SetHeight();
		_RemoveBtn.SetMinLeft(dlgrect);

		_InputPBtn.SetRight(dlgrect);
		_InputPBtn.SetTop(dlgrect);
		_InputPBtn.SetWidth();
		_InputPBtn.SetHeight();
		_InputPBtn.SetMinLeft(dlgrect);

		_DispLbl.SetRight(dlgrect);
		_DispLbl.SetTop(dlgrect);
		_DispLbl.SetWidth();
		_DispLbl.SetHeight();
		_DispLbl.SetMinLeft(dlgrect);

		_PointsBtn.SetRight(dlgrect);
		_PointsBtn.SetTop(dlgrect);
		_PointsBtn.SetWidth();
		_PointsBtn.SetHeight();
		_PointsBtn.SetMinLeft(dlgrect);

		_SegmentsBtn.SetRight(dlgrect);
		_SegmentsBtn.SetTop(dlgrect);
		_SegmentsBtn.SetWidth();
		_SegmentsBtn.SetHeight();
		_SegmentsBtn.SetMinLeft(dlgrect);

		_CurveBtn.SetRight(dlgrect);
		_CurveBtn.SetTop(dlgrect);
		_CurveBtn.SetWidth();
		_CurveBtn.SetHeight();
		_CurveBtn.SetMinLeft(dlgrect);

		_AxisBtn.SetRight(dlgrect);
		_AxisBtn.SetTop(dlgrect);
		_AxisBtn.SetWidth();
		_AxisBtn.SetHeight();
		_AxisBtn.SetMinLeft(dlgrect);

		_GridBtn.SetRight(dlgrect);
		_GridBtn.SetTop(dlgrect);
		_GridBtn.SetWidth();
		_GridBtn.SetHeight();
		_GridBtn.SetMinLeft(dlgrect);

		_LabelsBtn.SetRight(dlgrect);
		_LabelsBtn.SetTop(dlgrect);
		_LabelsBtn.SetWidth();
		_LabelsBtn.SetHeight();
		_LabelsBtn.SetMinLeft(dlgrect);

		_ClosedChk.SetRight(dlgrect);
		_ClosedChk.SetTop(dlgrect);
		_ClosedChk.SetWidth();
		_ClosedChk.SetHeight();
		_ClosedChk.SetMinLeft(dlgrect);

		_SmplsLbl.SetRight(dlgrect);
		_SmplsLbl.SetTop(dlgrect);
		_SmplsLbl.SetWidth();
		_SmplsLbl.SetHeight();
		_SmplsLbl.SetMinLeft(dlgrect);

		_SmplsEdt.SetRight(dlgrect);
		_SmplsEdt.SetTop(dlgrect);
		_SmplsEdt.SetWidth();
		_SmplsEdt.SetHeight();
		_SmplsEdt.SetMinLeft(dlgrect);

		_ViewWnd.SetLeft(dlgrect);
		_ViewWnd.SetRight(dlgrect);
		_ViewWnd.SetTop(dlgrect);
		_ViewWnd.SetBottom(dlgrect);
		_ViewWnd.SetMinWidth();
		_ViewWnd.SetMinHeight();

		_BTypeLbl.SetRight(dlgrect);
		_BTypeLbl.SetTop(dlgrect);
		_BTypeLbl.SetWidth();
		_BTypeLbl.SetHeight();
		_BTypeLbl.SetMinLeft(dlgrect);

		_BType.SetRight(dlgrect);
		_BType.SetTop(dlgrect);
		_BType.SetWidth();
		_BType.SetHeight();
		_BType.SetMinLeft(dlgrect);
	}
#endif
}


void ContourEdit::_UpdateControls()
{
	const Contour* pCntr = dynamic_cast<const Contour*>(_theView->GetObject());
	Button closed(GetDlgItem(IDC_CLOSED));
	closed.SetCheck(pCntr->IsClosed());
	EditLine name(GetDlgItem(IDC_NAME));
	name.SetText(pCntr->GetName());
	EditLine samples(GetDlgItem(IDC_SAMPLES));
	samples.SetInt(pCntr->GetSamples());

	ComboBox type(GetDlgItem(IDC_TYPECB));
	if (pCntr->Type() == Contour::btRegular)
		type.SetCurSel(0);
	else
		type.SetCurSel(1);

	if (pCntr->GetVersion()>=102)
		type.Enable(true);
	else
		type.Enable(false);
}


void ContourEdit::_UpdateFromControls()
{
	const int BfSize = 32;
	char bf[BfSize+1];
	Contour* pCntr = dynamic_cast<Contour*>(_theView->GetObject());
	EditLine Name(GetDlgItem(IDC_NAME));
	Name.GetText(bf, BfSize);
	pCntr->SetName(bf);
	EditLine samples(GetDlgItem(IDC_SAMPLES));
	int smpl = samples.GetInt();
	if (pCntr->IsValidSamples(smpl))
		pCntr->SetSamples(smpl);
	else
		samples.SetInt(pCntr->GetSamples());
}


bool ContourEdit::Command(int id, Window ctl, UINT notify)
{
	try
	{
		switch (id)
		{
		case IDC_POINTS :
			{
				Button btn(ctl);
				if (btn.IsChecked())
					_theView->SetDrawPoints(true);
				else
					_theView->SetDrawPoints(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_SEGMENTS :
			{
				Button b(ctl);
				if (b.IsChecked())
					_theView->SetDrawSegments(true);
				else
					_theView->SetDrawSegments(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_CURVE :
			{
				Button b(ctl);
				if (b.IsChecked())
					_theView->SetDrawCurve(true);
				else
					_theView->SetDrawCurve(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_AXIS :
			{
				Button b(ctl);
				if (b.IsChecked())
					_theView->SetDrawAxis(true);
				else
					_theView->SetDrawAxis(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_GRID :
			{
				Button Grid(ctl);
				if (Grid.IsChecked())
					_theView->SetDrawGrid(true);
				else
				{
					Button Labels(GetDlgItem(IDC_LABELS));
					Labels.SetCheck(false);
					_theView->SetDrawGrid(false);
					_theView->SetDrawLabels(false);
				}
				_pView->ForceRepaint();
			}
			break;
		case IDC_LABELS :
			{
				Button Labels(ctl);
				if (Labels.IsChecked())
				{
					Button Grid(GetDlgItem(IDC_GRID));
					Grid.SetCheck(true);
					_theView->SetDrawLabels(true);
					_theView->SetDrawGrid(true);
				}
				else
					_theView->SetDrawLabels(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_CLOSED :
			{
				assert(GetDlgItem(IDC_CLOSED).Is(ctl));
				Button closed(ctl);
				if (closed.IsChecked())
					_theView->SetClosed(true);
				else
					_theView->SetClosed(false);
				_pView->ForceRepaint();
				Modified(true);
			}
			break;
			
		case IDC_MOVE :
			_theView->SwitchMovePointMode();
			break;
		case IDC_ADD :
			_theView->SwitchAddPointMode();
			break;
		case IDC_REMOVE :
			_theView->SwitchDeletePointMode();
			break;
		case IDC_EDITCOORD :
			_theView->SwitchSetPointMode();
			break;
		case IDC_SAMPLES :
		case IDC_NAME :
			if (EN_SETFOCUS == notify)
				App::SetModeless(Hwnd());
			else if (EN_KILLFOCUS == notify)
				App::ClearModeless();
			break;
		case IDC_APPLY :
			ApplyNow();
			break;
		case IDC_TYPECB :
			if (CBN_SELCHANGE==notify)
			{
				ComboBox cb(ctl);
				switch (cb.GetCurSel())
				{
				case 0 :
					_BsplineRegular();
					break;
				case 1 :
					_BsplineEndPoint();
					break;
				}
			}
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void ContourEdit::_UpdateView()
{
}

void ContourEdit::LoadGallery(const TCHAR* fname)
{
	_theGallery->LoadGallery(fname);
	Select(0);
}


void ContourEdit::Generate() const
{
	if (_theGallery->Bound())
	{
		_theGallery->Generate();
	}
	else
	{
		for (int i=0; i<_pGallery->Items(); i++)
		{
			const Contour* pContour = dynamic_cast<const Contour*>(_pGallery->GetObject(i));
			TCHAR fname[32];
			OemToCharBuff(pContour->GetName(), fname, 27);
			if (_tcscmp(fname, __TEXT("unnamed")))
			{
				fname[27] = 0;
				_tcscat(fname, __TEXT(".con"));
				WriteTextFile cfile(fname);
				pContour->Generate(cfile);
			}
		}
	}
}

void ContourEdit::Export() const
{
	Generate();
}


void ContourEdit::Import(const TCHAR* fname)
{
	ReadTextFile src(fname);
	Contour* pNew = new Contour;
	std::unique_ptr<EditableObject> New(pNew);
	char line[128];
	CharToOem(fname, line);
	line[strlen(line)-4] = 0;
	pNew->SetName(line);
	pNew->Import(src);
	_pGallery->Add(New);
}



#ifdef CONTOUREDITRESIZABLE
bool ContourEdit::Size(SizeState, int w, int h)
{
	_Gallery.Adjust(w, h);
	_Name.Adjust(w, h);
	_NameLbl.Adjust(w, h);
	_ViewWnd.Adjust(w, h);
	_EditLbl.Adjust(w, h);
	_MoveBtn.Adjust(w, h);
	_AddBtn.Adjust(w, h);
	_RemoveBtn.Adjust(w, h);
	_InputPBtn.Adjust(w, h);

	_DispLbl.Adjust(w, h);
	_PointsBtn.Adjust(w, h);
	_SegmentsBtn.Adjust(w, h);
	_CurveBtn.Adjust(w, h);
	_AxisBtn.Adjust(w, h);
	_GridBtn.Adjust(w, h);
	_LabelsBtn.Adjust(w, h);
	_ClosedChk.Adjust(w, h);
	_BTypeLbl.Adjust(w, h);
	_BType.Adjust(w, h);
	_SmplsLbl.Adjust(w, h);
	_SmplsEdt.Adjust(w, h);
	return true;
}
#endif



bool ContourEdit::_ObjectModified(bool final) const
{
	const Contour* pContour = dynamic_cast<const Contour*>(_pGallery->GetObject(_pGallery->GetCurrent()));
	if (pContour->IsCurve())
		return _pNotifySink->ContourCurveModified(final);
	else
		return _pNotifySink->ContourModified(final);
}


void ContourEdit::ApplyNow()
{
	ObjectEdit::ApplyNow();
	const Contour* pContour = dynamic_cast<const Contour*>(_pGallery->GetObject(_pGallery->GetCurrent()));
	if (pContour->IsCurve())
		_pNotifySink->ContourCurveModified(true);
	else
		_pNotifySink->ContourModified(true);
}


bool ContourEdit::SelectionChanged(int cur, int newsel)
{
	const bool res = ObjectEdit::SelectionChanged(cur, newsel);
	if (res)
		_pNotifySink->ContourModified(true);
	return res;
}


void ContourEdit::ColorschemeModified()
{
	_theView->ColorschemeModified();
	_theGallery->ColorschemeModified();
	InvalidateRect(_theView->Hwnd(), 0, true);
	InvalidateRect(_theGallery->Hwnd(), 0, true);
}


void ContourEdit::_BsplineRegular()
{
	Contour* pCntr = dynamic_cast<Contour*>(_theView->GetObject());
	pCntr->SetType(Contour::btRegular);
	_pView->ForceRepaint();
	Modified(true);
}

void ContourEdit::_BsplineEndPoint()
{
	Contour* pCntr = dynamic_cast<Contour*>(_theView->GetObject());
	pCntr->SetType(Contour::btEndPoint);
	_pView->ForceRepaint();
	Modified(true);
}

