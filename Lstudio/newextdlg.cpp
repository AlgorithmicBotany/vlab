#include <fw.h>

#include "newextdlg.h"
#include "resource.h"


NewExtensionDlg::NewExtensionDlg(const std::string& location, char separator, bool sibling) : 
Dialog(IDD_NEWVERSION),
_Location(location),
_ChangeIdentity(false),
_StoreLocally(false)
{
	std::string::size_type pos = _Location.find_last_of(separator);
	if (std::string::npos != pos)
		_Name = _Location.substr(pos+1);
	if (sibling)
		_Location.erase(pos);
}

bool NewExtensionDlg::DoInit()
{
	EditLine loc(GetDlgItem(IDC_LOCATION));
	loc.SetText(_Location);
	loc.SetReadOnly(true);
	return true;
}

void NewExtensionDlg::UpdateData(bool which)
{
	DX(_Name, IDC_NAME, which);
	DXButton(_ChangeIdentity, IDC_CHANGEIDENTITY, which);
}


bool NewExtensionDlg::_Check()
{
	if (_Name.empty())
	{
		_CheckFailed(IDERR_NEWEXTNMEMPTY, IDC_NAME);
		return false;
	}
	return true;
}


bool NewExtensionDlg::Command(int id, Window, UINT)
{
	switch (id)
	{
	case IDC_BROWSE:
		_BrowseLocation();
		return true;
	}
	return false;
}


void NewExtensionDlg::_BrowseLocation()
{
	FolderBrowser fb(Hdlg(), IDS_BROWSENEWVERDIR, IDS_EMPTY);
	if (fb.Browse(_Location))
	{
		_StoreLocally = true;
		Window loc(GetDlgItem(IDC_LOCATION));
		loc.SetText(_Location);
		_ChangeIdentity = false;
		Button ci(GetDlgItem(IDC_CHANGEIDENTITY));
		ci.SetCheck(false);
		ci.Enable(false);
	}
}
