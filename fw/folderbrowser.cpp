#include <cassert>

#include <string>

#include <windows.h>
#include <shlobj.h>
#include <tchar.h>

#include "warningset.h"

#include "lstring.h"
#include "exception.h"
#include "comtmplts.h"
#include "resstrng.h"
#include "folderbrowser.h"


void SpecialFolder::GetPath(std::string& path)
{
	path.resize(_MAX_PATH+1);
	SHGetPathFromIDList(_p, &(path[0]));
	std::string::size_type pz = path.find_first_of('\0');
	if (std::string::npos == pz)
		path.erase();
	else
		path.resize(pz);
}

FolderBrowser::FolderBrowser(HWND hOwner, UINT caption, UINT title) :
_caption(128, caption),
_title(128, title)
{
	_bi.hwndOwner = hOwner;
	_bi.pidlRoot = 0;
	{
		_DisplayName.reserve(MAX_PATH+1);
		_DisplayName.resize(MAX_PATH);
		_bi.pszDisplayName = &(_DisplayName[0]);
	}
	_bi.lpszTitle = _caption.c_str();
	_bi.ulFlags = BIF_RETURNONLYFSDIRS;
	_bi.lpfn = _BFF;
	_bi.lParam = reinterpret_cast<LPARAM>(this);
	_bi.iImage = 0;
	_InitSel.resize(0);
}


bool FolderBrowser::Browse(std::string& sel)
{
	if (!sel.empty())
		_InitSel = sel;
	_p = SHBrowseForFolder(&_bi);
	if (0 != _p)
	{
		sel.reserve(MAX_PATH+1);
		sel.resize(MAX_PATH);
		SHGetPathFromIDList(_p, &sel[0]);
		size_t n = sel.find('\0');
		assert(n != std::string::npos);
		sel.resize(n);
	}
	return (0 != _p);
}

int CALLBACK FolderBrowser::_BFF(HWND hWnd, UINT msg, LPARAM lParam, LPARAM pVoid)
{
	FolderBrowser* pSelf = reinterpret_cast<FolderBrowser*>(pVoid);
	return pSelf->_DoBFF(hWnd, msg, lParam);
}


int FolderBrowser::_DoBFF(HWND hWnd, UINT msg, LPARAM)
{
	switch (msg)
	{
	case BFFM_INITIALIZED :
		if (!_InitSel.empty())
			SendMessage(hWnd, BFFM_SETSELECTION, (WPARAM) true, (LPARAM) _InitSel.c_str());
		if (!_title.empty())
			SetWindowText(hWnd, _title.c_str());
		break;
	}
	return 0;
}



SpecialFolder::SpecialFolder(int id, HWND hOwner)
{
	HRESULT hRes = SHGetSpecialFolderLocation
		(hOwner, id, &_p);
	if (hRes != NOERROR)
		_p = 0;
}
