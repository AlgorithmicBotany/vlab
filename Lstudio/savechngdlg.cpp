#include <vector>

#include <fw.h>

#include "difflist.h"
#include "savechngdlg.h"
#include "resource.h"


SaveChangesDlg::SaveChangesDlg(const char* obj, const DiffList& dl) : 
Dialog(IDD_SAVECHANGES), _obj(obj), _dl(dl), _Diff(Window(0))
{}


bool SaveChangesDlg::Command(int id, Window, UINT)
{
	switch (id)
	{
	case IDYES :
	case IDNO :
		EndDialog(id);
		break;
	case IDC_DETAILS :
		_ShowDetails();
		break;
	default :
		return false;
	}
	return true;
}


void SaveChangesDlg::_ShowDetails()
{
	if (_collapsed)
		_Expand();
	else
		_Collapse();
}


bool SaveChangesDlg::DoInit()
{
	{
		EditLine Name(GetDlgItem(IDC_NAME));
		Name.SetText(_obj);
	}

	_Diff.Reset(GetDlgItem(IDC_DIFF));

	// Set up columns
	{
		RECT r; _Diff.GetRect(r);
		ResString clmn(32, IDS_FILE);
		int sw = GetSystemMetrics(SM_CXVSCROLL);
		int cw = (r.right-sw-1)/2;

		_Diff.AddColumn(0, clmn, cw);
		clmn.Load(IDS_STATUS);
		_Diff.AddColumn(1, clmn, cw);
	}

	// Add entries
	{
		ResString stts(32, IDS_ADDED);

		for (int i=0; i<_dl.Items(); i++)
		{
			if (DiffList::stIdentical != _dl.Status(i))
			{
				int ix = _Diff.AddItem(i, _dl.Name(i));
				switch (_dl.Status(i))
				{
				case DiffList::stAdded:
					stts.Load(IDS_ADDED);
					break;
				case DiffList::stDeleted:
					stts.Load(IDS_DELETED);
					break;
				case DiffList::stModified:
					stts.Load(IDS_MODIFIED);
					break;
				}
				_Diff.SetItem(ix, 1, stts);
			}
		}
	}
	
	_Yes.Reset(GetDlgItem(IDYES));
	_No.Reset(GetDlgItem(IDNO));
	_Cancel.Reset(GetDlgItem(IDCANCEL));
	_Details.Reset(GetDlgItem(IDC_DETAILS));
	_DiffLbl.Reset(GetDlgItem(IDC_DIFFLBL));

	GetWindowRect(Hdlg(), &_DlgER);
	_DlgER.right = _DlgER.right - _DlgER.left;
	_DlgER.bottom = _DlgER.bottom - _DlgER.top;


	_Yes.GetWindowRect(_YesER);
	ScreenToClient(_YesER);
	_YesER.right = _YesER.right - _YesER.left;
	_YesER.bottom = _YesER.bottom - _YesER.top;

	_No.GetWindowRect(_NoER);
	ScreenToClient(_NoER);
	_NoER.right = _NoER.right - _NoER.left;
	_NoER.bottom = _NoER.bottom - _NoER.top;

	_Cancel.GetWindowRect(_CancelER);
	ScreenToClient(_CancelER);
	_CancelER.right = _CancelER.right - _CancelER.left;
	_CancelER.bottom = _CancelER.bottom - _CancelER.top;

	_Details.GetWindowRect(_DetailsER);
	ScreenToClient(_DetailsER);
	_DetailsER.right = _DetailsER.right - _DetailsER.left;
	_DetailsER.bottom = _DetailsER.bottom - _DetailsER.top;
	{
		RECT r;
		_DiffLbl.GetWindowRect(r);
		ScreenToClient(r);
		_Shift = _YesER.top-r.top;
	}

	_Collapse();

	return true;
}

void SaveChangesDlg::_Collapse()
{
	_Diff.Hide();
	_DiffLbl.Hide();

	RECT r = _DlgER;
	r.bottom -= _Shift;
	MoveWindow(Hdlg(), r.left, r.top, r.right, r.bottom, true);
	r = _YesER;
	r.top -= _Shift;
	_Yes.MoveWindow(r);
	r = _NoER;
	r.top -= _Shift;
	_No.MoveWindow(r);
	r = _CancelER;
	r.top -= _Shift;
	_Cancel.MoveWindow(r);
	r = _DetailsER;
	r.top -= _Shift;
	_Details.MoveWindow(r);

	r = _DlgER;
	r.bottom -= _Shift;

	ResString lbl(64, IDS_DETAILSSHOW);
	_Details.SetText(lbl);
	_collapsed = true;
}


void SaveChangesDlg::_Expand()
{
	MoveWindow(Hdlg(), _DlgER.left, _DlgER.top, _DlgER.right, _DlgER.bottom, true);
	_Yes.MoveWindow(_YesER);
	_No.MoveWindow(_NoER);
	_Cancel.MoveWindow(_CancelER);
	_Details.MoveWindow(_DetailsER);
	_Diff.Show();
	_DiffLbl.Show();
	ResString lbl(64, IDS_DETAILSHIDE);
	_Details.SetText(lbl);
	_collapsed = false;
}

