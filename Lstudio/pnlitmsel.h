#ifndef __PANELITEMSSELECTION_H__
#define __PANELITEMSSELECTION_H__


typedef PanelItem* pPanelItem;



class PanelItemsSelection
{
public:
	PanelItemsSelection();
	void Clear();
	void Draw(Canvas&) const;
	void Add(PanelItem*);
	bool IsEmpty() const
	{ return 0 == _items; }
	void AlignLeft();
	void AlignRight();
	void AlignTop();
	void AlignBottom();
	void HCenter();
	void VCenter();

	void DistributeHorz();
	void DistributeVert();

	void UpdateRect();
	void GetRect(RECT&) const;
	void MoveBy(int, int);

	int Items() const
	{ return _items; }
	PanelItem* GetSelected(int i)
	{
		assert(i>=0);
		assert(i<_items);
		return _arr[i];
	}

	bool InSelection(POINT pt) const;
	void TransferSelection(PanelItemsSelection&);
private:


	void _SortVert();
	void _SortHorz();

	enum { eMaxSelection = 256 };
	pPanelItem _arr[eMaxSelection];
	int _items;
	RECT _br;
};


#else
	#error File already included
#endif
