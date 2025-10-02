/**************************************************************************

  File:		listbox.h
  Created:	13-Jan-98


  Declaration of class ListBox


**************************************************************************/


#ifndef __LISTBOX_H__
#define __LISTBOX_H__


class ListBox : public Window
{
public:
	ListBox(Window w) : Window(w) {}
	int AddValue(DWORD v)
	{
		int id = ListBox_AddString(Hwnd(), __TEXT(""));
		return ListBox_SetItemData(Hwnd(), id, v);
	}
	int AddString(const std::string& str)
	{ return ListBox_AddString(Hwnd(), str.c_str()); }
	void SetItemData(int i, DWORD val)
	{ ListBox_SetItemData(Hwnd(), i, val); }
	DWORD GetItemData(int id)
	{ return static_cast<DWORD>(ListBox_GetItemData(Hwnd(), id)); }
	int GetCurSel()
	{ return ListBox_GetCurSel(Hwnd()); }
	int SetCurSel(int i)
	{ return ListBox_SetCurSel(Hwnd(), i); }
	int GetCount()
	{ return ListBox_GetCount(Hwnd()); }
	void DeleteString(int i)
	{ ListBox_DeleteString(Hwnd(), i); }
	int GetTextLen(int i) const
	{ return ListBox_GetTextLen(Hwnd(), i); }
	void GetText(int i, TCHAR* buf) const
	{ ListBox_GetText(Hwnd(), i, buf); }
	void InsertString(int i, const std::string& str)
	{ ListBox_InsertString(Hwnd(), i, str.c_str()); }
	void GetText(int i, std::string& str) const
	{
		int sz = GetTextLen(i);
		str.resize(sz+1);
		GetText(i, &(str[0]));
		str.resize(sz);
	}
};



#endif
