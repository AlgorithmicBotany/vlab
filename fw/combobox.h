/**************************************************************************

  File:		combobox.h
  Created:	02-Dec-97


  Declaration of class ComboBox


**************************************************************************/


#ifndef __COMBOBOX_H__
#define __COMBOBOX_H__


class ComboBox : public Window
{
public:
	ComboBox(Window w) : Window(w) {}
	int AddString(const std::string& str)
	{ return ComboBox_AddString(Hwnd(), str.c_str()); } const
	int AddStringAndId(const std::string& str, int id) const
	{
		int pos = ComboBox_AddString(Hwnd(), str.c_str());
		ComboBox_SetItemData(Hwnd(), pos, id);
		return pos;
	}
	int SetCurSel(int i) const
	{ return ComboBox_SetCurSel(Hwnd(), i); }
	int GetCurSel() const
	{ return ComboBox_GetCurSel(Hwnd()); }
	int GetCount() const
	{ return ComboBox_GetCount(Hwnd()); }
	int GetItemData(int pos) const
	{ return static_cast<int>(ComboBox_GetItemData(Hwnd(), pos)); }
	void SetItemData(int pos, int v) const
	{ ComboBox_SetItemData(Hwnd(), pos, v); }
	int FindItemData(int) const;
	int FindString(const std::string& str) const
	{ return ComboBox_FindString(Hwnd(), -1, str.c_str()); }
	void ResetContent() const
	{ ComboBox_ResetContent(Hwnd()); }
	void GetLBText(int, std::string& str) const;
	void SelectById(int id)
	{
		int pos = FindItemData(id);
		SetCurSel(pos);
	}
	int GetCurSelId() const
	{ return GetItemData(GetCurSel()); }
};


#endif
