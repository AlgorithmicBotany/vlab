/**************************************************************************

  File:		mdimenus.cpp
  Created:	24-Nov-97


  Implementation of class MDIMenus


**************************************************************************/

#include <cassert>


#include "mdimenus.h"



MDIMenus* MDIMenus::pGlobalMenus = 0;

MDIMenus::MDIMenus(const InitData* iarr, int count) 
{
	_arr.resize(count);
	int i = 0;
	for (std::vector<MenuData>::iterator it = _arr.begin() ; it != _arr.end(); ++it, ++i)
	{
		it->menu.Load(iarr[i].menuId);
		it->windowMenuPosition = iarr[i].windowMenuPosition;
	}
	pGlobalMenus = this;
}


MDIMenus::~MDIMenus()
{
	pGlobalMenus = 0;
}

HMENU MDIMenus::GetMenu(int i) const
{
	assert(i>=0);
	assert(static_cast<size_t>(i)<_arr.size());
	return _arr[i].menu.Handle();
}

HMENU MDIMenus::GetWindowMenu(int i) const
{
	assert(i>=0);
	assert(static_cast<size_t>(i)<_arr.size());
	return _arr[i].menu.SubMenu(_arr[i].windowMenuPosition);
}

