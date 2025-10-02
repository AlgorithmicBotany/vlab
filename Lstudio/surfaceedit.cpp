#include <memory>

#include <fw.h>
#include <glfw.h>



#include "resource.h"

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "glgallery.h"

#include "linethcb.h"
#include "thumbtask.h"
#include "linethumb.h"
#include "surfthumbcb.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"
#include "surfaceedit.h"

#include "patchclrinfo.h"
#include "patch.h"
#include "surface.h"
#include "surfviewtsk.h"
#include "surfaceview.h"
#include "surfgallery.h"


HWND SurfaceEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pNotifySink)
{
	class SurfaceEditCreator : public Creator
	{
	public:
		SurfaceEditCreator(PrjNotifySink* pNotifySink) : _pNotifySink(pNotifySink)
		{}
		FormCtrl* Create(HWND hDlg)
		{ return new SurfaceEdit(hDlg, GetWindowInstance(hDlg), _pNotifySink); }
	private:
		PrjNotifySink* _pNotifySink;
	};

	SurfaceEditCreator creator(pNotifySink);

	return CreateDialogParam
		(
		hInst,
		MAKEINTRESOURCE(IDD_SURFACE),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}


SurfaceEdit::SurfaceEdit(HWND hwnd, HINSTANCE, PrjNotifySink* pNotifySink) : 
ContModeEdit(hwnd, pNotifySink),
_XMoved(this),
_YMoved(this),
_ZMoved(this),
_Gallery(GetDlgItem(IDC_GALLERY)),
_XThumb(GetDlgItem(IDC_XTHUMB)),
_YThumb(GetDlgItem(IDC_YTHUMB)),
_ZThumb(GetDlgItem(IDC_ZTHUMB)),
_CP01(GetDlgItem(IDC_CP1)),
_CP02(GetDlgItem(IDC_CP2)),
_CP03(GetDlgItem(IDC_CP3)),
_CP04(GetDlgItem(IDC_CP4)),
_CP05(GetDlgItem(IDC_CP5)),
_CP06(GetDlgItem(IDC_CP6)),
_CP07(GetDlgItem(IDC_CP7)),
_CP08(GetDlgItem(IDC_CP8)),
_CP09(GetDlgItem(IDC_CP9)),
_CP10(GetDlgItem(IDC_CP10)),
_CP11(GetDlgItem(IDC_CP11)),
_CP12(GetDlgItem(IDC_CP12)),
_CP13(GetDlgItem(IDC_CP13)),
_CP14(GetDlgItem(IDC_CP14)),
_CP15(GetDlgItem(IDC_CP15)),
_CP16(GetDlgItem(IDC_CP16)),
_CTPT(GetDlgItem(IDC_CTPT)),
_PLCM(GetDlgItem(IDC_PLACEMENT)),
_Name(GetDlgItem(IDC_NAME)),
_NameLbl(GetDlgItem(IDC_NMLBL)),
_Preview(GetDlgItem(IDC_VIEW)),
_AdvancedBtn(GetDlgItem(IDC_ADVANCED)),
_SecretBtn(GetDlgItem(IDC_SECRET))
{
	_theView = reinterpret_cast<SurfaceView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_theView->SetEdit(this);
	_theView->SetActivePoint(0);
	_pView = reinterpret_cast<SurfaceView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_pView->SetEdit(this);
	_theGallery = reinterpret_cast<SurfaceGallery*>(GetDlgItem(IDC_GALLERY).GetPtr());
	_pGallery = _theGallery;
	_pGallery->SetEdit(this);
	const Surface* pSurface = dynamic_cast<const Surface*>(_pGallery->GetObject(0));
	_pView->CopyObject(pSurface);

	{
		Button b(GetDlgItem(IDC_CP1));
		b.SetCheck(true);
	}


	// Set thumb callbacks
	{
		const WorldPointf& p = _theView->GetPoint(0);
		Window w(GetDlgItem(IDC_XTHUMB));
		_pXThumb = reinterpret_cast<LineThumb*>(w.GetPtr());
		_pXThumb->SetCallback(&_XMoved);
		_pXThumb->SetX(p.X());
		_pXThumb->AdjustScale();

		w.Reset(GetDlgItem(IDC_YTHUMB));
		_pYThumb = reinterpret_cast<LineThumb*>(w.GetPtr());
		_pYThumb->SetCallback(&_YMoved);
		_pYThumb->SetX(p.Y());
		_pYThumb->AdjustScale();

		w.Reset(GetDlgItem(IDC_ZTHUMB));
		_pZThumb = reinterpret_cast<LineThumb*>(w.GetPtr());
		_pZThumb->SetCallback(&_ZMoved);
		_pZThumb->SetX(p.Z());
		_pZThumb->AdjustScale();
	}

	_UpdateControls();

	_Filename[0] = 0;

	// Set up constrains
	{
		RECT dlgrect;
		GetWindowRect(dlgrect);
		_Gallery.SetLeft(dlgrect);
		_Gallery.SetBottom(dlgrect);
		_Gallery.SetRight(dlgrect);
		_Gallery.SetHeight();
		_Gallery.SetMinWidth();
		_Gallery.SetMinTop(dlgrect);


		
		_XThumb.SetLeft(dlgrect);
		_XThumb.SetPropWidth(dlgrect);
		_XThumb.SetMinWidth();
		_XThumb.SetBottom(dlgrect);
		_XThumb.SetHeight();
		_XThumb.SetMinTop(dlgrect);


		_YThumb.SetPropLeft(dlgrect);
		_YThumb.SetMinLeft(dlgrect);
		_YThumb.SetPropWidth(dlgrect);
		_YThumb.SetMinWidth();
		_YThumb.SetBottom(dlgrect);
		_YThumb.SetHeight();
		_YThumb.SetMinTop(dlgrect);
		

		_ZThumb.SetRight(dlgrect);
		_ZThumb.SetPropWidth(dlgrect);
		_ZThumb.SetMinLeft(dlgrect);
		_ZThumb.SetBottom(dlgrect);
		_ZThumb.SetHeight();
		_ZThumb.SetMinTop(dlgrect);

		_CP01.SetRight(dlgrect);
		_CP01.SetTop(dlgrect);
		_CP01.SetWidth();
		_CP01.SetHeight();
		_CP01.SetMinTop(dlgrect);
		_CP01.SetMinLeft(dlgrect);

		_CP02.SetRight(dlgrect);
		_CP02.SetTop(dlgrect);
		_CP02.SetWidth();
		_CP02.SetHeight();
		_CP02.SetMinTop(dlgrect);
		_CP02.SetMinLeft(dlgrect);

		_CP03.SetRight(dlgrect);
		_CP03.SetTop(dlgrect);
		_CP03.SetWidth();
		_CP03.SetHeight();
		_CP03.SetMinTop(dlgrect);
		_CP03.SetMinLeft(dlgrect);

		_CP04.SetRight(dlgrect);
		_CP04.SetTop(dlgrect);
		_CP04.SetWidth();
		_CP04.SetHeight();
		_CP04.SetMinTop(dlgrect);
		_CP04.SetMinLeft(dlgrect);

		_CP05.SetRight(dlgrect);
		_CP05.SetTop(dlgrect);
		_CP05.SetWidth();
		_CP05.SetHeight();
		_CP05.SetMinTop(dlgrect);
		_CP05.SetMinLeft(dlgrect);

		_CP06.SetRight(dlgrect);
		_CP06.SetTop(dlgrect);
		_CP06.SetWidth();
		_CP06.SetHeight();
		_CP06.SetMinTop(dlgrect);
		_CP06.SetMinLeft(dlgrect);

		_CP07.SetRight(dlgrect);
		_CP07.SetTop(dlgrect);
		_CP07.SetWidth();
		_CP07.SetHeight();
		_CP07.SetMinTop(dlgrect);
		_CP07.SetMinLeft(dlgrect);

		_CP08.SetRight(dlgrect);
		_CP08.SetTop(dlgrect);
		_CP08.SetWidth();
		_CP08.SetHeight();
		_CP08.SetMinTop(dlgrect);
		_CP08.SetMinLeft(dlgrect);

		_CP09.SetRight(dlgrect);
		_CP09.SetTop(dlgrect);
		_CP09.SetWidth();
		_CP09.SetHeight();
		_CP09.SetMinTop(dlgrect);
		_CP09.SetMinLeft(dlgrect);

		_CP10.SetRight(dlgrect);
		_CP10.SetTop(dlgrect);
		_CP10.SetWidth();
		_CP10.SetHeight();
		_CP10.SetMinTop(dlgrect);
		_CP10.SetMinLeft(dlgrect);

		_CP11.SetRight(dlgrect);
		_CP11.SetTop(dlgrect);
		_CP11.SetWidth();
		_CP11.SetHeight();
		_CP11.SetMinTop(dlgrect);
		_CP11.SetMinLeft(dlgrect);

		_CP12.SetRight(dlgrect);
		_CP12.SetTop(dlgrect);
		_CP12.SetWidth();
		_CP12.SetHeight();
		_CP12.SetMinTop(dlgrect);
		_CP12.SetMinLeft(dlgrect);

		_CP13.SetRight(dlgrect);
		_CP13.SetTop(dlgrect);
		_CP13.SetWidth();
		_CP13.SetHeight();
		_CP13.SetMinTop(dlgrect);
		_CP13.SetMinLeft(dlgrect);

		_CP14.SetRight(dlgrect);
		_CP14.SetTop(dlgrect);
		_CP14.SetWidth();
		_CP14.SetHeight();
		_CP14.SetMinTop(dlgrect);
		_CP14.SetMinLeft(dlgrect);

		_CP15.SetRight(dlgrect);
		_CP15.SetTop(dlgrect);
		_CP15.SetWidth();
		_CP15.SetHeight();
		_CP15.SetMinTop(dlgrect);
		_CP15.SetMinLeft(dlgrect);

		_CP16.SetRight(dlgrect);
		_CP16.SetTop(dlgrect);
		_CP16.SetWidth();
		_CP16.SetHeight();
		_CP16.SetMinTop(dlgrect);
		_CP16.SetMinLeft(dlgrect);

		_CTPT.SetRight(dlgrect);
		_CTPT.SetTop(dlgrect);
		_CTPT.SetWidth();
		_CTPT.SetHeight();
		_CTPT.SetMinTop(dlgrect);
		_CTPT.SetMinLeft(dlgrect);

		_PLCM.SetRight(dlgrect);
		_PLCM.SetTop(dlgrect);
		_PLCM.SetWidth();
		_PLCM.SetHeight();
		_PLCM.SetMinTop(dlgrect);
		_PLCM.SetMinLeft(dlgrect);

		_AdvancedBtn.SetRight(dlgrect);
		_AdvancedBtn.SetTop(dlgrect);
		_AdvancedBtn.SetWidth();
		_AdvancedBtn.SetHeight();
		_AdvancedBtn.SetMinTop(dlgrect);
		_AdvancedBtn.SetMinLeft(dlgrect);

		_SecretBtn.SetRight(dlgrect);
		_SecretBtn.SetTop(dlgrect);
		_SecretBtn.SetWidth();
		_SecretBtn.SetHeight();
		_SecretBtn.SetMinTop(dlgrect);
		_SecretBtn.SetMinLeft(dlgrect);

		_Name.SetRight(dlgrect);
		_Name.SetTop(dlgrect);
		_Name.SetWidth();
		_Name.SetHeight();
		_Name.SetMinTop(dlgrect);
		_Name.SetMinLeft(dlgrect);

		_NameLbl.SetRight(dlgrect);
		_NameLbl.SetTop(dlgrect);
		_NameLbl.SetWidth();
		_NameLbl.SetHeight();
		_NameLbl.SetMinTop(dlgrect);
		_NameLbl.SetMinLeft(dlgrect);

		_Preview.SetRight(dlgrect);
		_Preview.SetLeft(dlgrect);
		_Preview.SetTop(dlgrect);
		_Preview.SetBottom(dlgrect);
		_Preview.SetMinWidth();
		_Preview.SetMinHeight();
	}
}

void SurfaceEdit::MoveX(float x, bool final)
{
	_theView->SetActivePointX(x);
	_pView->ForceRepaint();
	Modified(final);
}

void SurfaceEdit::MoveY(float y, bool final)
{
	_theView->SetActivePointY(y);
	_pView->ForceRepaint();
	Modified(final);
}

void SurfaceEdit::MoveZ(float z, bool final)
{
	_theView->SetActivePointZ(z);
	_pView->ForceRepaint();
	Modified(final);
}


void SurfaceEdit::Generate() const
{
	for (int i=0; i<_pGallery->Items(); i++)
	{
		const Surface* pSurface = dynamic_cast<const Surface*>(_pGallery->GetObject(i));
		TCHAR fname[32];
		OemToCharBuff(pSurface->GetName(), fname, 29);
		if (_tcscmp(fname, __TEXT("unnamed")))
		{
			fname[29] = 0;
			_tcscat(fname, __TEXT(".s"));
			WriteTextFile sfile(fname);
			pSurface->Generate(sfile);
		}
	}
}

void SurfaceEdit::Export() const
{
	Generate();
}


bool SurfaceEdit::Command(int id, Window, UINT notify)
{
	try
	{
		switch (id)
		{
		case IDC_CP1 :
			_ActivePointChanged(0);
			break;
		case IDC_CP2 :
			_ActivePointChanged(1);
			break;
		case IDC_CP3 :
			_ActivePointChanged(2);
			break;
		case IDC_CP4 :
			_ActivePointChanged(3);
			break;
		case IDC_CP5 :
			_ActivePointChanged(4);
			break;
		case IDC_CP6 :
			_ActivePointChanged(5);
			break;
		case IDC_CP7 :
			_ActivePointChanged(6);
			break;
		case IDC_CP8 :
			_ActivePointChanged(7);
			break;
		case IDC_CP9 :
			_ActivePointChanged(8);
			break;
		case IDC_CP10 :
			_ActivePointChanged(9);
			break;
		case IDC_CP11 :
			_ActivePointChanged(10);
			break;
		case IDC_CP12 :
			_ActivePointChanged(11);
			break;
		case IDC_CP13 :
			_ActivePointChanged(12);
			break;
		case IDC_CP14 :
			_ActivePointChanged(13);
			break;
		case IDC_CP15 :
			_ActivePointChanged(14);
			break;
		case IDC_CP16 :
			_ActivePointChanged(15);
			break;
		case IDC_PLACEMENT :
			_ActivePointChanged(ContactPointId);
			break;
		case IDC_ADVANCED :
			_Advanced();
			break;
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


bool SurfaceEdit::Size(SizeState, int w, int h)
{
	_Gallery.Adjust(w, h);
	_XThumb.Adjust(w, h);
	_YThumb.Adjust(w, h);
	_ZThumb.Adjust(w, h);
	_CP01.Adjust(w, h);
	_CP02.Adjust(w, h);
	_CP03.Adjust(w, h);
	_CP04.Adjust(w, h);
	_CP05.Adjust(w, h);
	_CP06.Adjust(w, h);
	_CP07.Adjust(w, h);
	_CP08.Adjust(w, h);
	_CP09.Adjust(w, h);
	_CP10.Adjust(w, h);
	_CP11.Adjust(w, h);
	_CP12.Adjust(w, h);
	_CP13.Adjust(w, h);
	_CP14.Adjust(w, h);
	_CP15.Adjust(w, h);
	_CP16.Adjust(w, h);
	_PLCM.Adjust(w, h);
	_CTPT.Adjust(w, h);
	_Name.Adjust(w, h);
	_NameLbl.Adjust(w, h);
	_Preview.Adjust(w, h);
	_AdvancedBtn.Adjust(w, h);
	_SecretBtn.Adjust(w, h);
	return true;
}

void SurfaceEdit::_ActivePointChanged(int a)
{
	assert(a>=0); 
	assert(a<16 || a==ContactPointId);
	const WorldPointf& p = _theView->GetPoint(a);
	_pXThumb->SetX(p.X());
	_pYThumb->SetX(p.Y());
	_pZThumb->SetX(p.Z());
	_pXThumb->AdjustScale();
	_pYThumb->AdjustScale();
	_pZThumb->AdjustScale();
	_theView->SetActivePoint(a);
	_theView->ForceRepaint();
}



void SurfaceEdit::_UpdateView()
{
}


void SurfaceEdit::_UpdateControls()
{
	WorldPointf p = _theView->GetActivePoint();
	_pXThumb->SetX(p.X());
	_pYThumb->SetX(p.Y());
	_pZThumb->SetX(p.Z());
	_pXThumb->AdjustScale();
	_pYThumb->AdjustScale();
	_pZThumb->AdjustScale();
	EditLine name(GetDlgItem(IDC_NAME));
	name.SetText(_theView->GetName());
}

void SurfaceEdit::_UpdateFromControls()
{
	const int BfSize = 32;
	char bf[BfSize];
	EditLine name(GetDlgItem(IDC_NAME));
	name.GetText(bf, BfSize);
	_theView->SetName(bf);
}


void SurfaceEdit::PointSelected(int i)
{
	assert(i>=0);
	assert(i<16 || i==ContactPointId);
	Button button(GetDlgItem(IDC_CP1));
	button.SetCheck(0==i);
	button.Reset(GetDlgItem(IDC_CP2));
	button.SetCheck(1==i);
	button.Reset(GetDlgItem(IDC_CP3));
	button.SetCheck(2==i);
	button.Reset(GetDlgItem(IDC_CP4));
	button.SetCheck(3==i);
	button.Reset(GetDlgItem(IDC_CP5));
	button.SetCheck(4==i);
	button.Reset(GetDlgItem(IDC_CP6));
	button.SetCheck(5==i);
	button.Reset(GetDlgItem(IDC_CP7));
	button.SetCheck(6==i);
	button.Reset(GetDlgItem(IDC_CP8));
	button.SetCheck(7==i);
	button.Reset(GetDlgItem(IDC_CP9));
	button.SetCheck(8==i);
	button.Reset(GetDlgItem(IDC_CP10));
	button.SetCheck(9==i);
	button.Reset(GetDlgItem(IDC_CP11));
	button.SetCheck(10==i);
	button.Reset(GetDlgItem(IDC_CP12));
	button.SetCheck(11==i);
	button.Reset(GetDlgItem(IDC_CP13));
	button.SetCheck(12==i);
	button.Reset(GetDlgItem(IDC_CP14));
	button.SetCheck(13==i);
	button.Reset(GetDlgItem(IDC_CP15));
	button.SetCheck(14==i);
	button.Reset(GetDlgItem(IDC_CP16));
	button.SetCheck(15==i);
	button.Reset(GetDlgItem(IDC_PLACEMENT));
	button.SetCheck(ContactPointId==i);
	_UpdateControls();
}

void SurfaceEdit::PointMoved(bool final)
{
	WorldPointf p = _theView->GetActivePoint();
	_pXThumb->SetX(p.X());
	_pYThumb->SetX(p.Y());
	_pZThumb->SetX(p.Z());
	Modified(final);
}

void SurfaceEdit::EndDrag()
{
	_pXThumb->AdjustScale();
	_pYThumb->AdjustScale();
	_pZThumb->AdjustScale();
	Modified(true);
}




void SurfaceEdit::Import(const TCHAR* fname)
{
	ReadTextFile src(fname);
	Surface* pNew = new Surface;
	std::unique_ptr<EditableObject> New(pNew);
	char line[128];
	CharToOem(fname, line);
	line[strlen(line)-2] = 0;
	pNew->SetName(line);
	pNew->Import(src);
	_pGallery->Add(New);
}


void SurfaceEdit::_Advanced()
{
	_pNotifySink->AdvancedSurfaceMode();
}

bool SurfaceEdit::_ObjectModified(bool final) const
{ 
	return _pNotifySink->SurfaceModified(final);
}



void SurfaceEdit::ApplyNow()
{
	ObjectEdit::ApplyNow();
	_pNotifySink->SurfaceModified(true);
}


bool SurfaceEdit::SelectionChanged(int cur, int newsel) 
{
	const bool res = ObjectEdit::SelectionChanged(cur, newsel);
	if (res)
		_pNotifySink->SurfaceModified(true);
	return res;
}


void SurfaceEdit::ColorschemeModified()
{
	_theView->ColorschemeModified();
	_theGallery->ColorschemeModified();
	InvalidateRect(_theView->Hwnd(), 0, true);
	InvalidateRect(_theGallery->Hwnd(), 0, true);
}
