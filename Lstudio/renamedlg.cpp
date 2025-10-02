#include <warningset.h>

#include <fw.h>

#include "renamedlg.h"
#include "resource.h"

RenameDialog::RenameDialog(UINT title, const TCHAR* name) : 
Dialog(IDD_RENAME),
_titleId(title),
_name(name)
{
}


bool RenameDialog::DoInit()
{
	LongString title(64, _titleId);
	SetWindowText(_hDlg, title);
	return true;
}

void RenameDialog::UpdateData(bool what)
{
	DX(_name, IDC_NAME, what);
}


bool RenameDialog::_Check()
{
	_name.AllTrim();
	if (_name.Length() == 0)
	{
		_CheckFailed(IDERR_NOEMPTYNAMES, IDC_NAME);
		return false;
	}
	return true;
}
