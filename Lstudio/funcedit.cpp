#include <vector>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "glgallery.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "resource.h"
#include "funcedit.h"
#include "gridviewtask.h"
#include "gridview.h"
#include "functionview.h"
#include "funcgallery.h"
#include "funcpts.h"
#include "function.h"


HWND FuncEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pNotifySink)
{
	class FuncEditCreator : public Creator
	{
	public:
		FuncEditCreator(PrjNotifySink* pNotifySink) : _pNotifySink(pNotifySink)
		{}
		FormCtrl* Create(HWND hDlg)
		{ return new FuncEdit(hDlg, GetWindowInstance(hDlg), _pNotifySink); }
	private:
		PrjNotifySink* _pNotifySink;
	};

	FuncEditCreator creator(pNotifySink);

	return CreateDialogParam
		(
		hInst,
		MAKEINTRESOURCE(IDD_FUNCEDIT),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}



FuncEdit::FuncEdit(HWND hwnd, HINSTANCE hInst, PrjNotifySink* pNotifySink) :
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
#ifdef FUNCTIONEDITRESIZABLE
,
_Gallery(GetDlgItem(IDC_GALLERY)),
_Name(GetDlgItem(IDC_NAME)),
_NameLbl(GetDlgItem(IDC_NAMELBL)),
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
_FlipBtn(GetDlgItem(IDC_FLIP)),
_SmplsLbl(GetDlgItem(IDC_SMPLSLBL)),
_SmplsEdt(GetDlgItem(IDC_SAMPLES)),
_ViewWnd(GetDlgItem(IDC_VIEW)),
#endif
_tooltips(Hwnd(), hInst)
{
	_theView = reinterpret_cast<FunctionView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_theView->SetFunctionEdit(this);
	_pView = reinterpret_cast<FunctionView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_pView->SetEdit(this);
	_theGallery = reinterpret_cast<FuncGallery*>(GetDlgItem(IDC_GALLERY).GetPtr());
	_pGallery = _theGallery;
	_pGallery->SetEdit(this);
	const EditableObject* pFunc = _pGallery->GetObject(0);
	_pView->CopyObject(pFunc);

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

#ifdef FUNCTIONEDITRESIZABLE
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

		_FlipBtn.SetRight(dlgrect);
		_FlipBtn.SetTop(dlgrect);
		_FlipBtn.SetWidth();
		_FlipBtn.SetHeight();
		_FlipBtn.SetMinLeft(dlgrect);

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
	}
#endif
}


void FuncEdit::_UpdateControls()
{
	{
		EditLine name(GetDlgItem(IDC_NAME));
		name.SetText(_theView->GetName());
	}
	{
		Button flip(GetDlgItem(IDC_FLIP));
		flip.SetCheck(_theView->Flip());
	}
	{
		EditLine samples(GetDlgItem(IDC_SAMPLES));
		if (_theView->ImplementsSamples())
		{
			samples.Enable(true);
			samples.SetInt(_theView->GetSamples());
		}
		else
		{
			samples.Enable(false);
			samples.SetText("");
		}
	}
}

void FuncEdit::_UpdateFromControls()
{
	const int BfSize = 32;
	char bf[BfSize+1];
	EditLine name(GetDlgItem(IDC_NAME));
	name.GetText(bf, BfSize);
	_theView->SetName(bf);
	if (_theView->ImplementsSamples())
	{
		EditLine ESamples(GetDlgItem(IDC_SAMPLES));
		int samples = ESamples.GetInt();
		if (samples<3)
			samples = 0;
		_theView->SetSamples(samples);
	}
}


void FuncEdit::_UpdateView()
{}


bool FuncEdit::Command(int id, Window ctl, UINT notify)
{
	try
	{
		switch (id)
		{
		case IDC_POINTS :
			{
				Button points(ctl);
				if (points.IsChecked())
					_theView->SetDrawPoints(true);
				else
					_theView->SetDrawPoints(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_SEGMENTS :
			{
				Button segments(ctl);
				if (segments.IsChecked())
					_theView->SetDrawSegments(true);
				else
					_theView->SetDrawSegments(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_CURVE :
			{
				Button Curve(ctl);
				if (Curve.IsChecked())
					_theView->SetDrawCurve(true);
				else
					_theView->SetDrawCurve(false);
				_pView->ForceRepaint();
			}
			break;
		case IDC_AXIS :
			{
				Button axis(ctl);
				if (axis.IsChecked())
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
					Button labels(GetDlgItem(IDC_LABELS));
					labels.SetCheck(false);
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
		case IDC_FLIP :
			{
				Button Flip(ctl);
				if (Flip.IsChecked())
					_theView->Flip(true);
				else
					_theView->Flip(false);
			}
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
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


void FuncEdit::LoadGallery(const TCHAR* fname)
{
	_theGallery->LoadGallery(fname);
	Select(0);
}

void FuncEdit::Generate() const
{
	_theGallery->Generate();
}


void FuncEdit::Import(const TCHAR* fname)
{
	_theGallery->Unbind();
	ReadTextFile src(fname);
	Function* pNew = new Function;
	std::unique_ptr<EditableObject> New(pNew);
	char line[128];
	CharToOem(fname, line);
	line[strlen(line)-5] = 0;
	pNew->SetName(line);
	pNew->Import(src);
	_pGallery->Add(New);
}



#ifdef FUNCTIONEDITRESIZABLE
bool FuncEdit::Size(SizeState, int w, int h)
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
	_FlipBtn.Adjust(w, h);
	_SmplsLbl.Adjust(w, h);
	_SmplsEdt.Adjust(w, h);
	return true;
}
#endif



bool FuncEdit::_ObjectModified(bool final) const
{ 
	return _pNotifySink->FunctionModified(final);
}


void FuncEdit::ApplyNow()
{
	ObjectEdit::ApplyNow();
	_pNotifySink->FunctionModified(true);
}

bool FuncEdit::SelectionChanged(int cur, int newsel)
{
	const bool res = ObjectEdit::SelectionChanged(cur, newsel);
	if (res)
		_pNotifySink->FunctionModified(true);
	return res;
}


void FuncEdit::ColorschemeModified()
{
	_theView->ColorschemeModified();
	_theGallery->ColorschemeModified();
	InvalidateRect(_theView->Hwnd(), 0, true);
	InvalidateRect(_theGallery->Hwnd(), 0, true);
}

void FuncEdit::Flipped(bool on)
{
	Button Flip(GetDlgItem(IDC_FLIP));
	Flip.SetCheck(on);
}


const TCHAR* FuncEdit::GalleryName() const
{
	if (_theGallery->Bound())
		return _theGallery->GalleryName();
	else
		return __TEXT("");
}


int FuncEdit::Count() const
{
	return _theGallery->Count();
}


const TCHAR* FuncEdit::GetObjectName(int n) const
{
	return _theGallery->GetObjectName(n);
}

void FuncEdit::Duplicate(int i, const char* nm)
{
	_theGallery->Duplicate(i, nm);
}
