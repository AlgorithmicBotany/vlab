/**************************************************************************

  File:		mdimenus.h
  Created:	24-Nov-97


  Declaration of class MDIMenus


**************************************************************************/


#ifndef __MDIMENUS_H__
#define __MDIMENUS_H__

#pragma warning (push, 3)

#include <vector>

#include <windows.h>

#pragma warning (pop)

#include "warningset.h"

#include "menu.h"

class MDIMenus
{
public:
	struct InitData
	{
		UINT menuId;
		int windowMenuPosition;
		int helpMenuPosition;
	};

	MDIMenus(const InitData* arr, int);
	~MDIMenus();

	HMENU GetDefaultMenu() const
	{
		return GetMenu(0);
	}
	HMENU GetDefaultWindowMenu() const
	{
		return GetWindowMenu(0);
	}

	HMENU GetMenu(int) const;
	HMENU GetWindowMenu(int) const;
	static MDIMenus* pGlobalMenus;
private:
	struct MenuData
	{
		Menu menu;
		int windowMenuPosition;
	};
	std::vector<MenuData> _arr;
};


#endif
