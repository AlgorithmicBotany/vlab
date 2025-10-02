#include <Windows.h>
#include <ShellAPI.h>
#include <ShlObj.h>
#include <ObjBase.h>

#include <string>

#include "exception.h"
#include "ShellShortcut.h"
#include "comtmplts.h"


Shortcut::Shortcut(const std::string& shortcutFilename) : _ShortcutFilename(shortcutFilename)
{}


bool Shortcut::Resolve(std::string& targetFilename, UINT errMsg)
{
	CoInst<IShellLink> sl
		(CLSID_ShellLink, 0, CLSCTX_INPROC_SERVER, IID_IShellLink, errMsg);

	Interface<IPersistFile> pf
		(sl, IID_IPersistFile, errMsg);

	WCHAR wsz[MAX_PATH+1];
	MultiByteToWideChar(CP_ACP, 0, _ShortcutFilename.c_str(), -1, wsz, MAX_PATH);
	HRESULT hRes = pf->Load(wsz, STGM_READ);
	if (!(SUCCEEDED(hRes)))
		return false;

	hRes = sl->Resolve(0, SLR_NO_UI);
	if (!(SUCCEEDED(hRes)))
		return false;

	TCHAR path[MAX_PATH+1];
	WIN32_FIND_DATA wfd;
	hRes = sl->GetPath(path, MAX_PATH, &wfd, 0);
	if (!(SUCCEEDED(hRes)))
		return false;
	targetFilename = path;
	return true;
}
