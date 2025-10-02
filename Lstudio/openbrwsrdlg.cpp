#include <fstream>

#include <fw.h>

#include <browser/connection.h>
#include <browser/localcnct.h>
#include <browser/remotecnct.h>

#include "lstudioptns.h"
#include "openbrwsrdlg.h"
#include "resource.h"

OpenBrowserLocalDlg::OpenBrowserLocalDlg(LStudioOptions& options, const std::string& fname) : 
	Dialog(IDD_BROWSER_LOCAL),
	_options(options),
	_fname(fname),
	_oofs(_options.OofsRoot())
{}


bool OpenBrowserLocalDlg::DoInit()
{
	if (_fname.empty())
		DisableItem(IDC_MAKE_DEFAULT);
	return true;
}

bool OpenBrowserLocalDlg::Command(int id, Window, UINT)
{
	switch (id)
	{
	case IDC_BROWSE:
		BrowseOofs();
		return true;
	case IDC_MAKE_DEFAULT :
		MakeDefault();
		return true;
	}
	return false;
}


void OpenBrowserLocalDlg::BrowseOofs()
{
	FolderBrowser fb(Hdlg(), IDS_BROWSEOOFS, IDS_EMPTY);
	if (fb.Browse(_oofs))
	{
		EditLine oofs(GetDlgItem(IDC_OOFS));
		oofs.SetText(_oofs);
	}
}


void OpenBrowserLocalDlg::MakeDefault()
{
	UpdateData(true);
	_options.SetOofsRoot(_oofs);
	std::ofstream trg(_fname);
	if (!trg.is_open())
		throw Exception(IDERR_WRITECONFIG, _fname.c_str());
	_options.Save(trg);
}

bool OpenBrowserLocalDlg::_Check()
{
	if (_oofs.empty())
	{
		_CheckFailed(IDERR_OOFSEMPTY, IDC_OOFS);
		return false;
	}
	if (!VLB::LocalConnection::ValidFname(_oofs))
	{
		_CheckFailed(IDERR_INVALIDOOFS, IDC_OOFS);
		return false;
	}
	return true;
}

void OpenBrowserLocalDlg::UpdateData(bool what)
{
	DX(_oofs, IDC_OOFS, what);
}


OpenBrowserRemoteDlg::OpenBrowserRemoteDlg(LStudioOptions& options, const std::string& fname) :
Dialog(IDD_CONNECTBROWSER),
_options(options),
_fname(fname),
_host(_options.Host()),
_user(_options.User()),
_pswd(_options.Password()),
_oofs(_options.Oofs())
{}


bool OpenBrowserRemoteDlg::DoInit()
{
	if (_fname.empty())
		DisableItem(IDC_MAKE_DEFAULT);
	return true;
}


void OpenBrowserRemoteDlg::UpdateData(bool what)
{
	DX(_host, IDC_HOST, what);
	DX(_user, IDC_USER, what);
	DX(_pswd, IDC_PASSWORD, what);
	DX(_oofs, IDC_OOFS, what);
}


bool OpenBrowserRemoteDlg::_Check()
{
	if (_host.empty())
	{
		_CheckFailed(IDERR_HOSTEMPTY, IDC_HOST);
		return false;
	}
	if (_user.empty())
	{
		_CheckFailed(IDERR_USEREMPTY, IDC_USER);
		return false;
	}
	if (_oofs.empty())
	{
		_CheckFailed(IDERR_OOFSEMPTY, IDC_OOFS);
		return false;
	}
	if (!VLB::RemoteConnection::ValidFname(_oofs))
	{
		_CheckFailed(IDERR_INVALIDOOFS, IDC_OOFS);
		return false;
	}

	return true;
}

bool OpenBrowserRemoteDlg::Command(int id, Window, UINT)
{
	switch (id)
	{
	case IDC_MAKE_DEFAULT :
		MakeDefault();
		return true;
	}
	return false;
}


void OpenBrowserRemoteDlg::MakeDefault()
{
	UpdateData(true);

	_options.Host(_host.c_str());
	_options.User(_user.c_str());
	_options.Password(_pswd.c_str());
	_options.Oofs(_oofs.c_str());

	std::ofstream trg(_fname);
	if (!trg.is_open())
		throw Exception(IDERR_WRITECONFIG, _fname.c_str());
	_options.Save(trg);
}
