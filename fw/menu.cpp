#include <string>

#include <cassert>

#include <windows.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "mdimenus.h"
#include "window.h"
#include "app.h"

#include "libstrng.h"

const char* MenuManipulator::tMenuBuildData::kLastItemTag = "";
const char* MenuManipulator::tMenuBuildData::kSeparatorTag = "---";

Menu::Menu(UINT id)
{
	_hMenu = LoadMenu(App::GetInstance(), MAKEINTRESOURCE(id));
	if (0 == _hMenu)
		throw Exception(0, FWStr::LoadingMenu);
}


void Menu::Load(UINT id)
{
	assert(!IsSet());
	_hMenu = LoadMenu(App::GetInstance(), MAKEINTRESOURCE(id));
	if (0 == _hMenu)
		throw Exception(0, FWStr::LoadingMenu);
}

void MenuManipulator::AppendCommand(int pos, UINT id, const std::string& lbl)
{

	HMENU hM = _hMenu;
	if (pos != -1)
		hM = GetSubMenu(pos);

	AppendMenu(hM, MF_STRING, id, lbl.c_str());
}

void MenuManipulator::Enable(UINT id, bool bEnable)
{
	EnableMenuItem(_hMenu, id, bEnable ? MF_ENABLED : MF_GRAYED);
}

void MenuManipulator::Disable(UINT id)
{
	EnableMenuItem(_hMenu, id, MF_GRAYED);
}

void MenuManipulator::Check(UINT id)
{
	CheckMenuItem(_hMenu, id, MF_CHECKED);
}

void MenuManipulator::Uncheck(UINT id)
{
	CheckMenuItem(_hMenu, id, MF_UNCHECKED);
}

void MenuManipulator::CheckRadio(UINT from, UINT to, UINT cmd)
{
	CheckMenuRadioItem(_hMenu, from, to, cmd, MF_BYCOMMAND);
}

void MenuManipulator::SetOwnerDraw(UINT cmd)
{
	MIInfo mii;
	mii.SetType(MIInfo::OwnerDraw);
	_SetMII(cmd, mii, false);
}

void MenuManipulator::SetText(UINT id, const std::string& cptn)
{
	MIInfo mii;
	mii.SetType(MIInfo::String);
	mii.SetTypeData(const_cast<char*>(cptn.c_str()));
	_SetMII(id, mii, false);
}

void MenuManipulator::SetTextBP(UINT id, const std::string& cptn)
{
	MIInfo mii;
	mii.SetType(MIInfo::String);
	mii.SetTypeData(const_cast<char*>(cptn.c_str()));
	_SetMII(id, mii, true);
}

const UINT MenuManipulator::nPos = static_cast<UINT>(-1);

UINT MenuManipulator::FindItem(const std::string& caption) const
{
	const int BfSize = 32;
	char lbl[BfSize+1];
	MIInfo mii;
	mii.SetType(MIInfo::String);
	mii.SetTypeData(lbl);
	mii.SetCount(BfSize);

	const UINT cnt = ItemCount();
	for (UINT i=0; i<cnt; ++i)
	{
		_GetMII(i, mii, true);
		lbl[BfSize] = 0;
		if (0==caption.compare(lbl))
			return i;
		mii.SetTypeData(lbl);
		mii.SetCount(BfSize);
	}

	return nPos;
}


UINT MenuManipulator::FindItem(UINT uFindID) const
{
	UINT count = ItemCount();
	for (UINT uiItem=0; uiItem<count; ++uiItem)
	{
		UINT uID = ::GetMenuItemID(_hMenu, uiItem);
		if (uID == uFindID)
			return uiItem;
	}
	return static_cast<UINT>(kInvalidId);
}


MenuManipulator::MIInfo::MIInfo()
{
	_info.cbSize = sizeof(_info);
	_info.fMask = 0;
	_info.fType = 0;
	_info.fState = 0;
	_info.wID = 0;
	_info.hSubMenu = 0;
	_info.hbmpChecked = 0;
	_info.hbmpUnchecked = 0;
	_info.dwItemData = 0;
	_info.dwTypeData = 0;
	_info.cch = 0;
}

void MenuManipulator::MIInfo::SetType(Type t)
{
	_info.fMask |= MIIM_TYPE;
	_info.fType |= t;
}

void MenuManipulator::BuildMenu(const std::vector<tMenuBuildData>& aBuildData)
{
	// first clean the menu
	while (ItemCount()>0)
	{
		::RemoveMenu(_hMenu, 0, MF_BYPOSITION);
	}

	typedef std::vector<tMenuBuildData>::const_iterator Citer;

	for (Citer it = aBuildData.begin(); it != aBuildData.end(); ++it)
	{
		if (0 != it->szLabel.compare(tMenuBuildData::kLastItemTag))
		{
			if (0==it->szLabel.compare(tMenuBuildData::kSeparatorTag))
			{
				AppendSeparator();
			}
			else
			{
				AppendCommand(-1, it->uiCommandId, it->szLabel);
			}
		}
	}
}

void MenuManipulator::BuildMenu(const tMenuBuildData* aBuildData)
{
	// first clean the menu
	while (ItemCount()>0)
	{
		::RemoveMenu(_hMenu, 0, MF_BYPOSITION);
	}

	int iItem=0;
	while (0 != aBuildData[iItem].szLabel.compare(tMenuBuildData::kLastItemTag)) // empty string terminates building
	{
		if (0 == aBuildData[iItem].szLabel.compare(tMenuBuildData::kSeparatorTag))
		{
			AppendSeparator();
		}
		else
		{
			AppendCommand(-1, aBuildData[iItem].uiCommandId, aBuildData[iItem].szLabel);
		}

		++iItem;
	}
}

void MenuManipulator::AppendSeparator()
{
	::AppendMenu(_hMenu, MF_SEPARATOR, 0, NULL);
}


void MenuManipulator::InsertCommand(UINT position, UINT commandID, const std::string& label)
{
	::InsertMenu(_hMenu, position, MF_BYPOSITION | MF_STRING, commandID, label.c_str());
}

