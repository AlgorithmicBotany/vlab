#include <memory>

#include <fw.h>

#include "menuids.h"
#include "objfgvview.h"
#include "objfgvedit.h"
#include "objfgvgallery.h"
#include "objfgvobject.h"

#include "panelprms.h"
#include "panelsedit.h"
#include "panelvwtsk.h"
#include "pnlitmsel.h"
#include "panelview.h"
#include "panelgallery.h"
#include "paneldesign.h"

#include "resource.h"

INIT_COUNTER(PanelEdit);

PanelEdit::PanelEdit(HWND hwnd, HINSTANCE hInst, PrjNotifySink* pNotifySink) : FormCtrl(hwnd),
_hInst(hInst),
_SelectButton(_hInst, MAKEINTRESOURCE(IDI_SELECT)),
_DeleteButton(_hInst, MAKEINTRESOURCE(IDI_DELETE)),
_SliderButton(_hInst, MAKEINTRESOURCE(IDI_SLIDER)),
_ButtonButton(_hInst, MAKEINTRESOURCE(IDI_BUTTON)),
_GroupButton(_hInst, MAKEINTRESOURCE(IDI_GROUP)),
_LabelButton(_hInst, MAKEINTRESOURCE(IDI_LABEL)),
_AlignLButton(_hInst, MAKEINTRESOURCE(IDI_ALIGNLEFT)),
_AlignLButtonA(_hInst, MAKEINTRESOURCE(IDI_ALIGNLEFTA)),
_AlignRButton(_hInst, MAKEINTRESOURCE(IDI_ALIGNRIGHT)),
_AlignTButton(_hInst, MAKEINTRESOURCE(IDI_ALIGNTOP)),
_AlignBButton(_hInst, MAKEINTRESOURCE(IDI_ALIGNBOTTOM)),
_AlignBButtonA(_hInst, MAKEINTRESOURCE(IDI_ALIGNBOTTOMA)),
_CenterVButton(_hInst, MAKEINTRESOURCE(IDI_CENTERVERT)),
_CenterHButton(_hInst, MAKEINTRESOURCE(IDI_CENTERHORZ)),
_DisrtVButton(_hInst, MAKEINTRESOURCE(IDI_DISTRIBUTEVRT)),
_DisrtHButton(_hInst, MAKEINTRESOURCE(IDI_DISTRIBUTEHRZ)),
_DuplDwnButton(_hInst, MAKEINTRESOURCE(IDI_DUPLDOWN)),
_DuplRgtButton(_hInst, MAKEINTRESOURCE(IDI_DUPLRIGHT))
#ifdef PANELSEDITRESIZEABLE
,
_Gallery(GetDlgItem(IDC_GALLERY)),
_Name(GetDlgItem(IDC_NAME)),
_NameLbl(GetDlgItem(IDC_NAMELBL)),
_DuplDwn(GetDlgItem(IDC_DUPLDOWN)),
_DuplRgt(GetDlgItem(IDC_DUPLRIGHT)),
_AlignLft(GetDlgItem(IDC_ALIGNLEFT)),
_AlignBtm(GetDlgItem(IDC_ALIGNBOTTOM)),
_DistHrz(GetDlgItem(IDC_DISTRIBUTEVRT)),
_DistVrt(GetDlgItem(IDC_DISTRIBUTEHRZ)),
_AlignLbl(GetDlgItem(IDC_ALIGNLBL)),
_SelectBtn(GetDlgItem(IDC_SELECT)),
_DeleteBtn(GetDlgItem(IDC_DELETE)),
_SliderBtn(GetDlgItem(IDC_SLIDER)),
_GroupBtn(GetDlgItem(IDC_GROUP)),
_ButtonBtn(GetDlgItem(IDC_BUTTON)),
_LabelBtn(GetDlgItem(IDC_LABEL)),
_ActionLbl(GetDlgItem(IDC_ACTIONLBL)),
_AssocLbl(GetDlgItem(IDC_ASSOCLBL)),
_AssocLs(GetDlgItem(IDC_LSYSTEM)),
_AssocVw(GetDlgItem(IDC_VIEWFILE)),
_AssocFile(GetDlgItem(IDC_FILE)),
_AssocFlnm(GetDlgItem(IDC_FILENAME)),
_AssocFlBr(GetDlgItem(IDC_BROWSEFILE)),
_ViewWnd(GetDlgItem(IDC_VIEW)),
_TriggerLbl(GetDlgItem(IDC_TRIGGERLBL)),
_Trigger(GetDlgItem(IDC_TRIGGER)),
_ModeGrp(GetDlgItem(IDC_MODEGRP)),
_DesignBtn(GetDlgItem(IDC_DESIGN)),
_ExecuteBtn(GetDlgItem(IDC_EXECUTE)),
#endif
_tooltips(Hwnd(), _hInst)
{
	_curMode = pmExecute;
	_theView = reinterpret_cast<PanelView*>(GetDlgItem(IDC_VIEW).GetPtr());
	_theView->SetProjectNotifySink(pNotifySink);
	_pView = _theView;
	_pView->SetEdit(this);
	_theGallery = reinterpret_cast<PanelGallery*>(GetDlgItem(IDC_GALLERY).GetPtr());
	_theGallery->SetProjectNotifySink(pNotifySink);
	_pGallery = _theGallery;
	_pGallery->SetEdit(this);
	const EditableObject* pDesign = _pGallery->GetObject(0);
	_pView->CopyObject(pDesign);

	_UpdateControls();


	{
		Button button(GetDlgItem(IDC_SLIDER));
		button.SetIcon(_SliderButton);
		_tooltips.Add(button, IDC_SLIDER, _hInst);

		button.Reset(GetDlgItem(IDC_BUTTON));
		button.SetIcon(_ButtonButton);
		_tooltips.Add(button, IDC_BUTTON, _hInst);

		button.Reset(GetDlgItem(IDC_LABEL));
		button.SetIcon(_LabelButton);
		_tooltips.Add(button, IDC_LABEL, _hInst);

		button.Reset(GetDlgItem(IDC_GROUP));
		button.SetIcon(_GroupButton);
		_tooltips.Add(button, IDC_GROUP, _hInst);

		button.Reset(GetDlgItem(IDC_SELECT));
		button.SetIcon(_SelectButton);
		button.SetCheck(true);
		_tooltips.Add(button, IDC_SELECT, _hInst);

		button.Reset(GetDlgItem(IDC_DELETE));
		button.SetIcon(_DeleteButton);
		_tooltips.Add(button, IDC_DELETE, _hInst);

		button.Reset(GetDlgItem(IDC_ALIGNLEFT));
		button.SetIcon(_AlignLButtonA);
		_tooltips.Add(button, IDS_ALIGNHORZ, _hInst);

		button.Reset(GetDlgItem(IDC_ALIGNBOTTOM));
		button.SetIcon(_AlignBButtonA);
		_tooltips.Add(button, IDS_ALIGNVERT, _hInst);

		button.Reset(GetDlgItem(IDC_DISTRIBUTEVRT));
		button.SetIcon(_DisrtVButton);
		_tooltips.Add(button, IDC_DISTRIBUTEVERT, _hInst);

		button.Reset(GetDlgItem(IDC_DISTRIBUTEHRZ));
		button.SetIcon(_DisrtHButton);
		_tooltips.Add(button, IDC_DISTRIBUTEHRZ, _hInst);

		button.Reset(GetDlgItem(IDC_DUPLDOWN));
		button.SetIcon(_DuplDwnButton);
		_tooltips.Add(button, IDC_DUPLDOWN, _hInst);

		button.Reset(GetDlgItem(IDC_DUPLRIGHT));
		button.SetIcon(_DuplRgtButton);
		_tooltips.Add(button, IDC_DUPLRIGHT, _hInst);

		button.Reset(GetDlgItem(IDC_LSYSTEM));
		button.SetCheck(true);
		button.Reset(GetDlgItem(IDC_VIEWFILE));
		button.SetCheck(false);
		button.Reset(GetDlgItem(IDC_FILE));
		button.SetCheck(false);

		{
			EditLine file(GetDlgItem(IDC_FILENAME));
			file.Enable(false);
		}

		button.Reset(GetDlgItem(IDC_BROWSEFILE));
		button.Enable(false);

		button.Reset(GetDlgItem(IDC_DESIGN));
		button.SetCheck(true);
		button.Reset(GetDlgItem(IDC_EXECUTE));
		button.SetCheck(false);
	}
	{

		Window vw(GetDlgItem(IDC_VIEW));
		_theView = reinterpret_cast<PanelView*>(vw.GetPtr());
		assert(0 != _theView);
	}

#ifdef PANELSEDITRESIZEABLE
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

		_DuplDwn.SetRight(dlgrect);
		_DuplDwn.SetTop(dlgrect);
		_DuplDwn.SetWidth();
		_DuplDwn.SetHeight();
		_DuplDwn.SetMinLeft(dlgrect);

		_DuplRgt.SetRight(dlgrect);
		_DuplRgt.SetTop(dlgrect);
		_DuplRgt.SetWidth();
		_DuplRgt.SetHeight();
		_DuplRgt.SetMinLeft(dlgrect);

		_AlignLft.SetRight(dlgrect);
		_AlignLft.SetTop(dlgrect);
		_AlignLft.SetWidth();
		_AlignLft.SetHeight();
		_AlignLft.SetMinLeft(dlgrect);

		_AlignBtm.SetRight(dlgrect);
		_AlignBtm.SetTop(dlgrect);
		_AlignBtm.SetWidth();
		_AlignBtm.SetHeight();
		_AlignBtm.SetMinLeft(dlgrect);

		_DistHrz.SetRight(dlgrect);
		_DistHrz.SetTop(dlgrect);
		_DistHrz.SetWidth();
		_DistHrz.SetHeight();
		_DistHrz.SetMinLeft(dlgrect);

		_DistVrt.SetRight(dlgrect);
		_DistVrt.SetTop(dlgrect);
		_DistVrt.SetWidth();
		_DistVrt.SetHeight();
		_DistVrt.SetMinLeft(dlgrect);

		_AlignLbl.SetRight(dlgrect);
		_AlignLbl.SetTop(dlgrect);
		_AlignLbl.SetWidth();
		_AlignLbl.SetHeight();
		_AlignLbl.SetMinLeft(dlgrect);

		_SelectBtn.SetRight(dlgrect);
		_SelectBtn.SetTop(dlgrect);
		_SelectBtn.SetWidth();
		_SelectBtn.SetHeight();
		_SelectBtn.SetMinLeft(dlgrect);

		_DeleteBtn.SetRight(dlgrect);
		_DeleteBtn.SetTop(dlgrect);
		_DeleteBtn.SetWidth();
		_DeleteBtn.SetHeight();
		_DeleteBtn.SetMinLeft(dlgrect);

		_SliderBtn.SetRight(dlgrect);
		_SliderBtn.SetTop(dlgrect);
		_SliderBtn.SetWidth();
		_SliderBtn.SetHeight();
		_SliderBtn.SetMinLeft(dlgrect);

		_GroupBtn.SetRight(dlgrect);
		_GroupBtn.SetTop(dlgrect);
		_GroupBtn.SetWidth();
		_GroupBtn.SetHeight();
		_GroupBtn.SetMinLeft(dlgrect);

		_ButtonBtn.SetRight(dlgrect);
		_ButtonBtn.SetTop(dlgrect);
		_ButtonBtn.SetWidth();
		_ButtonBtn.SetHeight();
		_ButtonBtn.SetMinLeft(dlgrect);

		_LabelBtn.SetRight(dlgrect);
		_LabelBtn.SetTop(dlgrect);
		_LabelBtn.SetWidth();
		_LabelBtn.SetHeight();
		_LabelBtn.SetMinLeft(dlgrect);

		_ActionLbl.SetRight(dlgrect);
		_ActionLbl.SetTop(dlgrect);
		_ActionLbl.SetWidth();
		_ActionLbl.SetHeight();
		_ActionLbl.SetMinLeft(dlgrect);

		_AssocLbl.SetRight(dlgrect);
		_AssocLbl.SetTop(dlgrect);
		_AssocLbl.SetWidth();
		_AssocLbl.SetHeight();
		_AssocLbl.SetMinLeft(dlgrect);

		_AssocLs.SetRight(dlgrect);
		_AssocLs.SetTop(dlgrect);
		_AssocLs.SetWidth();
		_AssocLs.SetHeight();
		_AssocLs.SetMinLeft(dlgrect);

		_AssocVw.SetRight(dlgrect);
		_AssocVw.SetTop(dlgrect);
		_AssocVw.SetWidth();
		_AssocVw.SetHeight();
		_AssocVw.SetMinLeft(dlgrect);

		_AssocFile.SetRight(dlgrect);
		_AssocFile.SetTop(dlgrect);
		_AssocFile.SetWidth();
		_AssocFile.SetHeight();
		_AssocFile.SetMinLeft(dlgrect);

		_AssocFlnm.SetRight(dlgrect);
		_AssocFlnm.SetTop(dlgrect);
		_AssocFlnm.SetWidth();
		_AssocFlnm.SetHeight();
		_AssocFlnm.SetMinLeft(dlgrect);

		_AssocFlBr.SetRight(dlgrect);
		_AssocFlBr.SetTop(dlgrect);
		_AssocFlBr.SetWidth();
		_AssocFlBr.SetHeight();
		_AssocFlBr.SetMinLeft(dlgrect);

		_TriggerLbl.SetRight(dlgrect);
		_TriggerLbl.SetTop(dlgrect);
		_TriggerLbl.SetWidth();
		_TriggerLbl.SetHeight();
		_TriggerLbl.SetMinLeft(dlgrect);

		_Trigger.SetRight(dlgrect);
		_Trigger.SetTop(dlgrect);
		_Trigger.SetWidth();
		_Trigger.SetHeight();
		_Trigger.SetMinLeft(dlgrect);



		_ModeGrp.SetRight(dlgrect);
		_ModeGrp.SetTop(dlgrect);
		_ModeGrp.SetWidth();
		_ModeGrp.SetHeight();
		_ModeGrp.SetMinLeft(dlgrect);

		_ExecuteBtn.SetRight(dlgrect);
		_ExecuteBtn.SetTop(dlgrect);
		_ExecuteBtn.SetWidth();
		_ExecuteBtn.SetHeight();
		_ExecuteBtn.SetMinLeft(dlgrect);

		_DesignBtn.SetRight(dlgrect);
		_DesignBtn.SetTop(dlgrect);
		_DesignBtn.SetWidth();
		_DesignBtn.SetHeight();
		_DesignBtn.SetMinLeft(dlgrect);

		_ViewWnd.SetLeft(dlgrect);
		_ViewWnd.SetRight(dlgrect);
		_ViewWnd.SetTop(dlgrect);
		_ViewWnd.SetBottom(dlgrect);
		_ViewWnd.SetMinWidth();
		_ViewWnd.SetMinHeight();
	}
#endif

	// temporarily hide trigger stuff
	/*{
		Window w(GetDlgItem(IDC_TRIGGERLBL));
		w.Hide();
		w.Reset(GetDlgItem(IDC_TRIGGER));
		w.Hide();
	}*/

	_Mode(pmDesign);

	INC_COUNTER;
}


PanelEdit::~PanelEdit()
{
	DEC_COUNTER;
}



HWND PanelEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pSink)
{
	class PanelEditCreator : public Creator
	{
	public:
		PanelEditCreator(PrjNotifySink* pNotifySink) : _pNotifySink(pNotifySink)
		{}
		FormCtrl* Create(HWND hDlg)
		{ return new PanelEdit(hDlg, GetWindowInstance(hDlg), _pNotifySink); }
	private:
		PrjNotifySink* _pNotifySink;
	};

	PanelEditCreator creator(pSink);

	return CreateDialogParam
		(
		hInst,
		MAKEINTRESOURCE(IDD_PANELS),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}



bool PanelEdit::Command(int id, Window w, UINT notify)
{
	try
	{
		switch (id)
		{
		case IDC_SELECT :
			_theView->SwitchToSelect();
			break;
		case IDC_DELETE :
			{
				_theView->Delete();
				Button b(GetDlgItem(IDC_DELETE));
				b.SetCheck(false);
				b.Reset(GetDlgItem(IDC_SELECT));
				b.SetCheck(true);
			}
			break;
		case IDC_SLIDER :
			_theView->SwitchToAddSlider();
			break;
		case IDC_BUTTON :
			_theView->SwitchToAddButton();
			break;
		case IDC_LABEL :
			_theView->SwitchToAddLabel();
			break;
		case IDC_GROUP :
			_theView->SwitchToAddGroup();
			break;
		case IDC_ALIGNLEFT :
			_AlignHorz();
			_theView->GrabFocus();
			break;
		case IDC_ALIGNBOTTOM :
			_AlignVert();
			_theView->GrabFocus();
			break;
		case IDC_DISTRIBUTEHRZ :
			_theView->DistributeHorz();
			_theView->GrabFocus();
			break;
		case IDC_DISTRIBUTEVRT :
			_theView->DistributeVert();
			_theView->GrabFocus();
			break;
		case IDC_DUPLDOWN :
			_theView->DuplicateDown();
			_theView->GrabFocus();
			break;
		case IDC_DUPLRIGHT :
			_theView->DuplicateRight();
			_theView->GrabFocus();
			break;
		case IDC_LSYSTEM :
			LsystemSelected();
			break;
		case IDC_VIEWFILE :
			ViewSelected();
			break;
		case IDC_FILE :
			FileSelected();
			break;
		case IDC_BROWSEFILE :
			_BrowseTargetFile();
			break;
		case IDC_DESIGN :
			_Mode(pmDesign);
			break;
		case IDC_EXECUTE :
			_Mode(pmExecute);
			break;
		case ID_PANELALIGNHORZ_ALIGNLEFT :
			_theView->AlignLeft();
			_theView->GrabFocus();
			break;
		case ID_PANELALIGNHORZ_ALIGNRIGHT :
			_theView->AlignRight();
			_theView->GrabFocus();
			break;
		case ID_PANELALIGNHORZ_CENTER :
			_theView->HCenter();
			_theView->GrabFocus();
			break;
		case ID_PANELALIGNVERT_ALIGNTOP :
			_theView->AlignTop();
			_theView->GrabFocus();
			break;
		case ID_PANELALIGNVERT_ALIGNBOTTOM :
			_theView->AlignBottom();
			_theView->GrabFocus();
			break;
		case ID_PANELALIGNVERT_CENTER :
			_theView->VCenter();
			_theView->GrabFocus();
			break;
		case IDC_NAME :
			if (EN_SETFOCUS == notify)
				App::SetModeless(Hwnd());
			else if (EN_KILLFOCUS == notify)
				App::ClearModeless();
			break;
		case IDC_TRIGGER :
			if (CBN_SELCHANGE == notify)
			{
				ComboBox cb(w);
				int id = cb.GetCurSelId();
				if (_theView->IsValidTrigger(id))
					_theView->SetTrigger(static_cast<PanelParameters::TriggerCommand>(id));
			}
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


void PanelEdit::LsystemSelected()
{
	const int LabelMaxSize = 32;
	_theView->SetPanelTarget(PanelParameters::ptLsystem);
	Window w(GetDlgItem(IDC_FILENAME));
	w.Enable(false);
	w.Reset(GetDlgItem(IDC_BROWSEFILE));
	w.Enable(false);
	ComboBox cb(GetDlgItem(IDC_TRIGGER));
	cb.ResetContent();
	ResString lbl(LabelMaxSize, IDS_NEWLSYSTEM);
	cb.AddStringAndId(lbl, PanelParameters::tcNewLsystem);
	lbl.Load(IDS_NEWMODEL);
	cb.AddStringAndId(lbl, PanelParameters::tcNewModel);
	lbl.Load(IDS_NOTHING);
	cb.AddStringAndId(lbl, PanelParameters::tcNothing);
	if (_theView->Trigger() != PanelParameters::tcNewView && _theView->Trigger() != PanelParameters::tcRerun)
		cb.SelectById(_theView->Trigger());
	else
		cb.SelectById(PanelParameters::tcNewLsystem);
}

void PanelEdit::ViewSelected()
{
	const int LabelMaxSize = 32;
	_theView->SetPanelTarget(PanelParameters::ptViewFile);
	Window w(GetDlgItem(IDC_FILENAME));
	w.Enable(false);
	w.Reset(GetDlgItem(IDC_BROWSEFILE));
	w.Enable(false);
	ComboBox cb(GetDlgItem(IDC_TRIGGER));
	cb.ResetContent();
	ResString lbl(LabelMaxSize, IDS_NEWVIEW);
	cb.AddStringAndId(lbl, PanelParameters::tcNewView);
	lbl.Load(IDS_NEWMODEL);
	cb.AddStringAndId(lbl, PanelParameters::tcNewModel);
	lbl.Load(IDS_NOTHING);
	cb.AddStringAndId(lbl, PanelParameters::tcNothing);
	if (_theView->Trigger() != PanelParameters::tcNewLsystem && _theView->Trigger() != PanelParameters::tcRerun)
		cb.SelectById(_theView->Trigger());
	else
		cb.SelectById(PanelParameters::tcNewView);
}

void PanelEdit::FileSelected()
{
	const int LabelMaxSize = 32;
	_theView->SetPanelTarget(PanelParameters::ptFile);
	Window w(GetDlgItem(IDC_FILENAME));
	w.Enable(true);
	w.Reset(GetDlgItem(IDC_BROWSEFILE));
	w.Enable(true);
	ComboBox cb(GetDlgItem(IDC_TRIGGER));
	cb.ResetContent();
	ResString lbl(LabelMaxSize, IDS_NEWLSYSTEM);
	cb.AddStringAndId(lbl, PanelParameters::tcNewLsystem);
	lbl.Load(IDS_NEWVIEW);
	cb.AddStringAndId(lbl, PanelParameters::tcNewView);
	lbl.Load(IDS_NEWMODEL);
	cb.AddStringAndId(lbl, PanelParameters::tcNewModel);
	lbl.Load(IDS_NOTHING);
	cb.AddStringAndId(lbl, PanelParameters::tcNothing);
	lbl.Load(IDS_RERUN);
	cb.AddStringAndId(lbl, PanelParameters::tcRerun);
	cb.SelectById(_theView->Trigger());
}

void PanelEdit::_UpdateView()
{}


void PanelEdit::_UpdateControls()
{
	{
		EditLine name(GetDlgItem(IDC_NAME));
		name.SetText(_theView->GetName());
	}
	{
		Button Ls(GetDlgItem(IDC_LSYSTEM));
		Button Vw(GetDlgItem(IDC_VIEWFILE));
		Button Fl(GetDlgItem(IDC_FILE));
		Button Flnm(GetDlgItem(IDC_FILENAME));
		Button FlBt(GetDlgItem(IDC_BROWSEFILE));

		Ls.SetCheck(false);
		Vw.SetCheck(false);
		Fl.SetCheck(false);
		Flnm.Enable(false);
		FlBt.Enable(false);

		switch (_theView->PanelTarget())
		{
		case PanelParameters::ptLsystem :
			Ls.SetCheck(true);
			LsystemSelected();
			break;
		case PanelParameters::ptViewFile :
			Vw.SetCheck(true);
			ViewSelected();
			break;
		case PanelParameters::ptFile :
			Fl.SetCheck(true);
			FileSelected();
			if (pmDesign == _curMode)
			{
				Flnm.Enable(true);
				FlBt.Enable(true);
			}
			Flnm.SetText(_theView->PanelTargetFilename());
			break;
		}
	}
}


void PanelEdit::_UpdateFromControls()
{
	{
		const int BfSize = 32;
		char bf[BfSize];
		EditLine el(GetDlgItem(IDC_NAME));
		el.GetText(bf, BfSize);
		_theView->SetName(bf);
	}


	Button Ls(GetDlgItem(IDC_LSYSTEM));
	Button Vw(GetDlgItem(IDC_VIEWFILE));
	Button Fl(GetDlgItem(IDC_FILE));
	EditLine Flnm(GetDlgItem(IDC_FILENAME));

	if (Ls.IsChecked())
		_theView->SetPanelTarget(PanelParameters::ptLsystem);
	else if (Vw.IsChecked())
		_theView->SetPanelTarget(PanelParameters::ptViewFile);
	else if (Fl.IsChecked())
	{
		_theView->SetPanelTarget(PanelParameters::ptFile);
		_theView->SetPanelTargetFilename(Flnm);
	}
	ComboBox cb(GetDlgItem(IDC_TRIGGER));
	int id = cb.GetCurSelId();
	if (_theView->IsValidTrigger(id))
		_theView->SetTrigger(static_cast<PanelParameters::TriggerCommand>(id));
}


void PanelEdit::Generate() const
{
	for (int i=0; i<_pGallery->Items(); i++)
	{
		const PanelDesign* pPanel = dynamic_cast<const PanelDesign*>(_pGallery->GetObject(i));
		const int BfSize = 38;
		char fname[BfSize];
		strncpy(fname, pPanel->GetName(), BfSize);
		if (_tcscmp(fname, PanelDesign::DefaultName()))
		{
			fname[BfSize-5] = 0;
			strcat(fname, __TEXT(".pnl"));
			WriteTextFile trg(fname);
			pPanel->Generate(trg);
		}
	}
}


void PanelEdit::Import(const TCHAR* fname)
{
	ReadTextFile src(fname);
	PanelDesign* pNew = new PanelDesign;
	std::unique_ptr<EditableObject> New(pNew);
	const int BfLine = 128;
	char line[BfLine];
	strncpy(line, fname, BfLine);
	line[strlen(line)-4] = 0;
	pNew->SetName(line);
	pNew->Import(src);
	_pGallery->Add(New);
}



#ifdef PANELSEDITRESIZEABLE
bool PanelEdit::Size(SizeState, int w, int h)
{
	_Gallery.Adjust(w, h);
	_Name.Adjust(w, h);
	_NameLbl.Adjust(w, h);
	_DuplDwn.Adjust(w, h);
	_DuplRgt.Adjust(w, h);
	_AlignLft.Adjust(w, h);
	_AlignBtm.Adjust(w, h);
	_DistHrz.Adjust(w, h);
	_DistVrt.Adjust(w, h);
	_AlignLbl.Adjust(w, h);
	_SelectBtn.Adjust(w, h);
	_DeleteBtn.Adjust(w, h);
	_SliderBtn.Adjust(w, h);
	_GroupBtn.Adjust(w, h);
	_ButtonBtn.Adjust(w, h);
	_LabelBtn.Adjust(w, h);
	_ActionLbl.Adjust(w, h);
	_AssocLbl.Adjust(w, h);
	_AssocLs.Adjust(w, h);
	_AssocVw.Adjust(w, h);
	_AssocFile.Adjust(w, h);
	_AssocFlnm.Adjust(w, h);
	_AssocFlBr.Adjust(w, h);
	_TriggerLbl.Adjust(w, h);
	_Trigger.Adjust(w, h);
	_ViewWnd.Adjust(w, h);
	_ModeGrp.Adjust(w, h);
	_DesignBtn.Adjust(w, h);
	_ExecuteBtn.Adjust(w, h);
	return true;
}
#endif



void PanelEdit::_BrowseTargetFile()
{
	OpenFilename ofn(Hwnd(), IDS_ANYFILEFILTER, __TEXT(""));
	CurrentDirectory cd;
	ofn.SetDirectory(cd.c_str());

	Window w(GetDlgItem(IDC_FILENAME));
	std::string fname;
	w.GetText(fname);
	if (!fname.empty())
		ofn.SetDefault(fname);

	if (ofn.Open())
	{
		static TCHAR bf[_MAX_PATH+1];
		_tcscpy(bf, ofn.Filename());
		if (_InCurrentDir(bf))
			w.SetText(bf);
		else if (MessageYesNo(IDS_FILEINDIFFDIR, bf))
			w.SetText(bf);
	}
}



bool PanelEdit::_InCurrentDir(TCHAR* bf) const
{
	TCHAR* bs = _tcsrchr(bf, __TEXT('\\'));
	if (0==bs)
		return true;

	int fnlen = _tcslen(bs+1);
	CurrentDirectory cd;

	*bs = 0;
	if (0 == _tcscmp(bf, cd.c_str()))
	{
		memmove(bf, bs+1, (fnlen+1)*sizeof(TCHAR));
		return true;
	}
	else
		*bs = __TEXT('\\');

	return false;
}


void PanelEdit::ExecuteMode()
{
	if (pmExecute != _curMode)
	{
		if (strcmp(_theView->GetName(), PanelDesign::DefaultName()))
		{
			_Mode(pmExecute);
			Button button(GetDlgItem(IDC_DESIGN));
			button.SetCheck(false);
			button.Reset(GetDlgItem(IDC_EXECUTE));
			button.SetCheck(true);
		}
	}
}

void PanelEdit::TearOffAll()
{
	assert(pmExecute==_curMode);
	_theGallery->TearOffAll();
}


void PanelEdit::_Mode(PanelMode pm)
{
	if (pmDesign == pm)
	{
		if (_theGallery->TearedOffPanels())
		{
			if (MessageYesNo(IDS_CLOSEPANELSBEFOREEDIT))
				_theGallery->CloseTearedOff();
			else
			{
				Button button(GetDlgItem(IDC_DESIGN));
				button.SetCheck(false);
				button.Reset(GetDlgItem(IDC_EXECUTE));
				button.SetCheck(true);
				return;
			}
		}
	}
	else
	{
		ApplyNow();
		const PanelDesign* pDesign = dynamic_cast<const PanelDesign*>(_theView->GetObject());
		if (!strcmp(pDesign->GetName(), PanelDesign::DefaultName()))
		{
			MessageBox(IDERR_UNNAMEDPANELEXECUTE);
			Button button(GetDlgItem(IDC_DESIGN));
			button.SetCheck(true);
			button.Reset(GetDlgItem(IDC_EXECUTE));
			button.SetCheck(false);
			return;
		}
		if (pDesign->PanelTarget() == PanelParameters::ptFile && 0==strlen(pDesign->PanelTargetFilename()))
		{
			MessageBox(IDERR_NOTARGETFILE);
			Button button(GetDlgItem(IDC_DESIGN));
			button.SetCheck(true);
			button.Reset(GetDlgItem(IDC_EXECUTE));
			button.SetCheck(false);
			return;
		}
	}

	bool enable = (pm == pmDesign);
	_curMode = pm;
	if (pmExecute == _curMode)
	{
		_theView->SwitchToExecute();
		_theGallery->SwitchToExecute();
	}
	else
	{
		_theView->SwitchToDesign();
		_theGallery->SwitchToDesign();
	}

	{
		Button b(GetDlgItem(IDC_SELECT));
		b.Enable(enable);
		b.SetCheck(true);
		b.Reset(GetDlgItem(IDC_SLIDER));
		b.Enable(enable);
		b.SetCheck(false);
		b.Reset(GetDlgItem(IDC_GROUP));
		b.Enable(enable);
		b.SetCheck(false);
		b.Reset(GetDlgItem(IDC_DELETE));
		b.Enable(enable);
		b.SetCheck(false);
		b.Reset(GetDlgItem(IDC_BUTTON));
		b.Enable(enable);
		b.SetCheck(false);
		b.Reset(GetDlgItem(IDC_LABEL));
		b.Enable(enable);
		b.SetCheck(false);
	}
	{
		Window w(GetDlgItem(IDC_ALIGNLEFT));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_ALIGNBOTTOM));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_DISTRIBUTEVRT));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_DISTRIBUTEHRZ));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_DUPLDOWN));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_DUPLRIGHT));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_LSYSTEM));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_VIEWFILE));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_FILE));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_APPLY));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_NAME));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_TRIGGER));
		w.Enable(enable);
	}

	if 
		(
		(_curMode == pmExecute) || 
		(_curMode == pmDesign && _theView->PanelTarget()==PanelParameters::ptFile)
		)
	{
		Window w(GetDlgItem(IDC_FILENAME));
		w.Enable(enable);
		w.Reset(GetDlgItem(IDC_BROWSEFILE));
		w.Enable(enable);
	}
}


void PanelEdit::_AlignHorz()
{
	MenuManipulator menu(App::theApp->GetContextMenu(AlignHorzCMenu));
	menu.SetOwnerDraw(ID_PANELALIGNHORZ_ALIGNLEFT);
	menu.SetOwnerDraw(ID_PANELALIGNHORZ_ALIGNRIGHT);
	menu.SetOwnerDraw(ID_PANELALIGNHORZ_CENTER);

	RECT r; 
	Window w(GetDlgItem(IDC_ALIGNLEFT));
	w.GetWindowRect(r);
	TrackPopupMenu(menu.Handle(), TPM_LEFTALIGN | TPM_TOPALIGN, r.left, r.bottom, 0, Hwnd(), 0);
}


void PanelEdit::_AlignVert()
{
	MenuManipulator menu(App::theApp->GetContextMenu(AlignVertCMenu));
	menu.SetOwnerDraw(ID_PANELALIGNVERT_ALIGNTOP);
	menu.SetOwnerDraw(ID_PANELALIGNVERT_ALIGNBOTTOM);
	menu.SetOwnerDraw(ID_PANELALIGNVERT_CENTER);

	RECT r; 
	Window w(GetDlgItem(IDC_ALIGNBOTTOM));
	w.GetWindowRect(r);
	TrackPopupMenu(menu.Handle(), TPM_LEFTALIGN | TPM_TOPALIGN, r.left, r.bottom, 0, Hwnd(), 0);
}


bool PanelEdit::MeasureItem(OwnerDraw::Measure ms)
{
	assert(ms.IsMenu());
	ms.SetWidth(18 + 4 /* shadow margin */ - GetSystemMetrics(SM_CXMENUCHECK));
	ms.SetHeight(18 + 4 /* shadow margin */);
	return true;
}



bool PanelEdit::DrawItem(OwnerDraw::Draw ds)
{
	if (ds.DrawEntire())
	{
		_DrawFace(ds);
		_DrawSelect(ds);
	}
	else if (ds.DrawSelect())
		_DrawSelect(ds);
	return true;
}


void PanelEdit::_DrawFace(OwnerDraw::Draw& ds) const
{
	HICON hI = 0;
	switch (ds.ItemId())
	{
	case ID_PANELALIGNHORZ_ALIGNLEFT :
		hI = _AlignLButton.Handle();
		break; 
	case ID_PANELALIGNHORZ_ALIGNRIGHT :
		hI = _AlignRButton.Handle();
		break;
	case ID_PANELALIGNHORZ_CENTER :
		hI = _CenterHButton.Handle();
		break;
	case ID_PANELALIGNVERT_ALIGNTOP :
		hI = _AlignTButton.Handle();
		break;
	case ID_PANELALIGNVERT_ALIGNBOTTOM :
		hI = _AlignBButton.Handle();
		break;
	case ID_PANELALIGNVERT_CENTER :
		hI = _CenterVButton.Handle();
		break;
	default :
		assert(!"Unknown menu itemID");
	}

	const RECT& r = ds.ItemRect();
	DrawIconEx
		(
		ds.DC(), 
		r.left+2, r.top+3, 
		hI, 0, 0, 0, 0, DI_NORMAL
		);
}


void PanelEdit::_DrawSelect(OwnerDraw::Draw& ds) const
{
	HDC hdc = ds.DC();
	const RECT& r = ds.ItemRect();
	if (ds.IsSelected())
	{
		{
			ObjectHolder sp(hdc, pens3Dset.Hilight());
			MoveToEx(hdc, r.left, r.bottom-1, 0);
			LineTo(hdc, r.left, r.top);
			LineTo(hdc, r.right-1, r.top);
		}
		{
			ObjectHolder sp(hdc, pens3Dset.Light());
			MoveToEx(hdc, r.left+1, r.bottom-2, 0);
			LineTo(hdc, r.left+1, r.top+1);
			LineTo(hdc, r.right-2, r.top+1);
		}
		{
			ObjectHolder sp(hdc, pens3Dset.Shadow());
			MoveToEx(hdc, r.left+1, r.bottom-2, 0);
			LineTo(hdc, r.right-2, r.bottom-2);
			LineTo(hdc, r.right-2, r.top);
		}
		{
			ObjectHolder sp(hdc, pens3Dset.DkShadow());
			MoveToEx(hdc, r.left, r.bottom-1, 0);
			LineTo(hdc, r.right-1, r.bottom-1);
			LineTo(hdc, r.right-1, r.top-1);
		}
	}
	else
	{
		ObjectHolder sp(hdc, pens3Dset.Face());
		MoveToEx(hdc, r.left, r.top, 0);
		LineTo(hdc, r.right-1, r.top);
		LineTo(hdc, r.right-1, r.bottom-1);
		LineTo(hdc, r.left, r.bottom-1);
		LineTo(hdc, r.left, r.top+1);
		LineTo(hdc, r.right-2, r.top+1);
		LineTo(hdc, r.right-2, r.bottom-2);
		LineTo(hdc, r.left+1, r.bottom-2);
		LineTo(hdc, r.left+1, r.top+1);
	}
}
