/**************************************************************************

  File:		combobox.cpp
  Created:	02-Dec-97


  Implementation of class ComboBox


**************************************************************************/


#include <assert.h>

#include <string>

#include <windows.h>
#include <windowsx.h>

#include "warningset.h"

#include "exception.h"
#include "menu.h"
#include "window.h"
#include "combobox.h"


int ComboBox::FindItemData(int v) const
{
	int n = ComboBox_GetCount(Hwnd());
	for (int i=0; i<n; i++)
	{
		int V = static_cast<int>(ComboBox_GetItemData(Hwnd(), i));
		if (v==V)
			return i;
	}
	return -1;
}


void ComboBox::GetLBText(int ix, std::string& str) const
{
	int len = ComboBox_GetLBTextLen(Hwnd(), ix);
	if (CB_ERR==len)
	{
		str.resize(0);
		return;
	}
	str.reserve(len+1);
	str.resize(len);
	ComboBox_GetLBText(Hwnd(), ix, &(str[0]));
	str.resize(len);
}
