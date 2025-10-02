#ifndef __LISTVIEW_H__
#define __LISTVIEW_H__



class ListView : public Window
{
public:
	ListView(Window w) : Window(w) {}
	void AddColumn(int clmn, const std::string& lbl, int width)
	{
		LV_COLUMN lvc;
		lvc.mask = LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvc.cx = width;
		lvc.pszText = const_cast<char*>(lbl.c_str());
		lvc.iSubItem = clmn;
		ListView_InsertColumn(Hwnd(), clmn, &lvc);
	}
	int AddItem(int id, const std::string& txt)
	{
		LV_ITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = id;
		lvi.iSubItem = 0;
		lvi.pszText = const_cast<char*>(txt.c_str());
		return ListView_InsertItem(Hwnd(), &lvi); 
	}
	void SetItem(int id, int clmn, std::string& txt)
	{
		LV_ITEM lvi;
		lvi.mask = LVIF_TEXT;
		lvi.iItem = id;
		lvi.iSubItem = clmn;
		lvi.pszText = const_cast<char*>(txt.c_str());
		ListView_SetItem(Hwnd(), &lvi);
	}

};



#endif
