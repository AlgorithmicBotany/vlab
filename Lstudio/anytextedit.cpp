#include <string>

#include <fw.h>

#include "resource.h"

#include "lstudioptns.h"
#include "tedit.h"
#include "prjnotifysnk.h"
#include "anytextedit.h"
#include "params.h"


AnyTextEdit::AnyTextEdit(HWND hwnd, PrjNotifySink* pNotifySink) : 
TextEdit<EmptyName, IDD_ANYTEXT>(hwnd),
_pNotifySink(pNotifySink),
_iOpen(App::GetInstance(), MAKEINTRESOURCE(IDI_CRV_OPEN)),
_iClose(App::GetInstance(), MAKEINTRESOURCE(IDI_CLOSE)),
_iSave(App::GetInstance(), MAKEINTRESOURCE(IDI_CRV_SAVE)),
_iSaveAs(App::GetInstance(), MAKEINTRESOURCE(IDI_SAVEAS))
{
	HINSTANCE hInst = App::GetInstance();

	{
		Button button(GetDlgItem(IDC_OPEN));
		button.SetIcon(_iOpen);
		_tooltips.Add(button, IDS_OPEN, hInst);

		button.Reset(GetDlgItem(IDC_CLOSE));
		button.SetIcon(_iClose);
		_tooltips.Add(button, IDS_CLOSE, hInst);

		button.Reset(GetDlgItem(IDC_SAVE));
		button.SetIcon(_iSave);
		_tooltips.Add(button, IDS_SAVE, hInst);

		button.Reset(GetDlgItem(IDC_SAVEAS));
		button.SetIcon(_iSaveAs);
		_tooltips.Add(button, IDS_SAVEAS, hInst);
	}
}


AnyTextEdit::~AnyTextEdit()
{
}


HWND AnyTextEdit::Create(HWND hParent, HINSTANCE hInst, PrjNotifySink* pNotifySink)
{
	class AnyTextEditCreator : public Creator
	{
	public:
		AnyTextEditCreator(PrjNotifySink* pNotifySink) : _pNotifySink(pNotifySink)
		{}
		FormCtrl* Create(HWND hDlg)
		{ return new AnyTextEdit(hDlg, _pNotifySink); }
	private:
		PrjNotifySink* _pNotifySink;
	};

	AnyTextEditCreator creator(pNotifySink);

	return CreateDialogParam
		(
		hInst,
		MAKEINTRESOURCE(IDD_ANYTEXT),
		hParent,
		reinterpret_cast<DLGPROC>(FormCtrl::DlgProc),
		reinterpret_cast<LPARAM>(&creator)
		);
}


bool AnyTextEdit::Command(int id, Window ctl, UINT notify)
{
	try
	{
		switch (id)
		{
		case IDC_OPEN :
			_Load();
			break;
		case IDC_SAVE :
			Save();
			break;
		case IDC_SAVEAS :
			_SaveAs();
			break;
		case IDC_CLOSE :
			_Close();
			break;
		case IDC_STATUS :
			if (STN_DBLCLK==notify)
				_Reload();
			break;
		default:
			TextEdit<EmptyName, IDD_ANYTEXT>::Command(id, ctl, notify);
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}



void AnyTextEdit::_Load()
{
	if (ShouldSave())
	{
		ResString msg(64, IDS_SAVEANYTEXT);
		int res = ::MessageBox(Hwnd(), msg.c_str(), __TEXT("L-studio"), MB_YESNOCANCEL | MB_ICONQUESTION);
		switch (res)
		{
		case IDCANCEL : return;
		case IDYES :
			if (0 != _Filename[0])
				Generate();
			else
				_SaveAs();
			break;
		}
	}

	OpenFilename ofn(Hwnd(), IDS_ANYTEXTFILTER, __TEXT(""));
	ofn.SetDirectory(_pNotifySink->GetLabTable());
	ofn.Flags |= OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;
	if (ofn.Open())
	{
		try
		{
			Import(ofn.Filename());
			_Edit.SetModify(false);
			const TCHAR* fn = _tcsrchr(ofn.Filename(), __TEXT('\\'));
			if (0 == fn)
				fn = ofn.Filename();
			else
				fn++;
			_Status.SetText(fn);
		}
		catch (Exception e) 
		{
			ErrorBox(e);
			_Edit.SetText(__TEXT(""));
			_Status.SetText(__TEXT("<untitled>"));
			_Filename[0] = 0;
			_Edit.SetModify(false);
		}

	}
}


void AnyTextEdit::_Reload()
{
	if (0 != _Filename[0])
	{
		try
		{
			Import(_Filename);
			_Edit.SetModify(false);
			const TCHAR* fn = _tcsrchr(_Filename, __TEXT('\\'));
			if (0 == fn)
				fn = _Filename;
			else
				fn++;
			_Status.SetText(fn);
		}
		catch (Exception e)
		{
			ErrorBox(e);
			_Edit.SetText(__TEXT(""));
			_Status.SetText(__TEXT("<untitled>"));
			_Filename[0] = 0;
			_Edit.SetModify(false);
		}
	}
	else
		MessageBeep(MB_ICONASTERISK);
}


void AnyTextEdit::_SaveAs()
{
	OpenFilename ofn(Hwnd(), IDS_ANYTEXTFILTER, __TEXT(""));
	ResString savetitle(64, IDS_SAVEANYTEXTFILE);
	ofn.SetDirectory(_pNotifySink->GetLabTable());
	ofn.lpstrTitle = savetitle.c_str();
	ofn.SetDefault(_Filename);
	if (ofn.Save())
	{
		_tcscpy(_Filename, ofn.Filename());
		const TCHAR* fn = _tcsrchr(ofn.Filename(), __TEXT('\\'));
		if (0 == fn)
			fn = ofn.Filename();
		else
			fn++;
		_Status.SetText(fn);

		Save();
	}
}

void AnyTextEdit::Save()
{
	if (0 != _Filename[0])
	{
		Generate();
		_Edit.SetModify(false);

		if (!(_tcscmp(ShortFName(), Params::LSspecs)))
			_pNotifySink->ReadLSspecifications();
		else if (!(_tcscmp(ShortFName(), Params::specs)))
			_pNotifySink->ReadSpecifications();
	}
	else
		_SaveAs();
}


void AnyTextEdit::_Close()
{
	if (ShouldSave())
	{
		ResString msg(64, IDS_SAVEANYTEXT);
		int res = ::MessageBox(Hwnd(), msg.c_str(), __TEXT("L-studio"), MB_YESNOCANCEL | MB_ICONQUESTION);
		switch (res)
		{
		case IDCANCEL : return;
		case IDYES :
			if (0 != _Filename[0])
				Generate();
			else
				_SaveAs();
			break;
		}
	}
	_Edit.SetText(__TEXT(""));
	_Status.SetText(__TEXT("<untitled>"));
	_Filename[0] = 0;
	_Edit.SetModify(false);
	_UpdateLine();
}



bool AnyTextEdit::ShouldSave() const
{
	return _Edit.GetModify();
}


const TCHAR* AnyTextEdit::ShortFName() const
{
	const TCHAR* pRes = _tcsrchr(_Filename, __TEXT('\\'));
	if (0 == pRes)
		pRes = _Filename;
	else 
		pRes++;
	return pRes;
}

