#include <fw.h>


#include "panelprms.h"
#include "panelitem.h"
#include "pnlitmsel.h"


PanelItemsSelection::PanelItemsSelection()
{
	_items = 0;
}


void PanelItemsSelection::Add(PanelItem* pItem)
{
	assert(_items<eMaxSelection);
	_arr[_items] = pItem;
	_items++;
	if (1==_items)
		pItem->GetRect(_br);
	else
	{
		RECT pr;
		pItem->GetRect(pr);
		if (pr.left<_br.left)
			_br.left = pr.left;
		if (pr.top<_br.top)
			_br.top = pr.top;
		if (pr.right>_br.right)
			_br.right = pr.right;
		if (pr.bottom>_br.bottom)
			_br.bottom = pr.bottom;
	}
}


void PanelItemsSelection::Draw(Canvas& cnv) const
{
	if (_items>0)
	{
		{
			Pen rp(RGB(192, 192, 192), Pen::psDot);
			ObjectHolder srp(cnv, rp);
			cnv.Rectangle(_br);
		}
		HBRUSH hBr = (HBRUSH) GetStockObject(LTGRAY_BRUSH);
		RECT crnr;
		crnr.top = _br.top - 2;
		crnr.bottom = _br.top + 3;
		crnr.left = _br.left - 2;
		crnr.right = _br.left + 3;
		FillRect(cnv, &crnr, hBr);
		crnr.top = _br.bottom-2;
		crnr.bottom = _br.bottom+3;
		FillRect(cnv, &crnr, hBr);
		crnr.left = _br.right - 2;
		crnr.right = _br.right + 3;
		FillRect(cnv, &crnr, hBr);
		crnr.top = _br.top - 2;
		crnr.bottom = _br.top + 3;
		FillRect(cnv, &crnr, hBr);
		ObjectHolder sp(cnv, GetStockObject(WHITE_PEN));
		for (int i=0; i<_items; i++)
			_arr[i]->DrawSelected(cnv);
	}
}


void PanelItemsSelection::Clear()
{
	_items = 0;
}


void PanelItemsSelection::AlignLeft()
{
	assert(!IsEmpty());
	RECT bb;
	_arr[0]->GetRect(bb);
	int minl = bb.left;
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(bb);
		if (bb.left<minl)
			minl = bb.left;
	}
	for (int i=0; i<_items; i++)
		_arr[i]->AlignLeft(minl);
}


void PanelItemsSelection::AlignRight()
{
	assert(!IsEmpty());
	RECT bb;
	_arr[0]->GetRect(bb);
	int maxr = bb.right;
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(bb);
		if (bb.right>maxr)
			maxr = bb.right;
	}
	for (int i=0; i<_items; i++)
		_arr[i]->AlignRight(maxr);
}




void PanelItemsSelection::AlignTop()
{
	assert(!IsEmpty());
	RECT bb;
	_arr[0]->GetRect(bb);
	int mint = bb.top;
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(bb);
		if (bb.top<mint)
			mint = bb.top;
	}
	for (int i=0; i<_items; i++)
		_arr[i]->AlignTop(mint);
}




void PanelItemsSelection::AlignBottom()
{
	assert(!IsEmpty());
	RECT bb;
	_arr[0]->GetRect(bb);
	int maxb = bb.bottom;
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(bb);
		if (bb.bottom>maxb)
			maxb = bb.bottom;
	}
	for (int i=0; i<_items; i++)
		_arr[i]->AlignBottom(maxb);
}


void PanelItemsSelection::HCenter()
{
	assert(!IsEmpty());
	RECT bb;
	_arr[0]->GetRect(bb);
	int minx = bb.left;
	int maxx = bb.right;
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(bb);
		if (bb.left<minx)
			minx = bb.left;
		if (bb.right>maxx)
			maxx = bb.right;
	}
	int hc = (minx+maxx)/2;
	for (int i=0; i<_items; i++)
		_arr[i]->HCenter(hc);
}

void PanelItemsSelection::VCenter()
{
	assert(!IsEmpty());
	RECT bb;
	_arr[0]->GetRect(bb);
	int miny = bb.top;
	int maxy = bb.bottom;
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(bb);
		if (bb.top<miny)
			miny = bb.top;
		if (bb.bottom>maxy)
			maxy = bb.bottom;
	}
	int vc = (miny+maxy)/2;
	for (int i=0; i<_items; i++)
		_arr[i]->VCenter(vc);
}



void PanelItemsSelection::UpdateRect()
{
	assert(!IsEmpty());
	_arr[0]->GetRect(_br);
	RECT pr;
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(pr);
		if (pr.left<_br.left)
			_br.left = pr.left;
		if (pr.top<_br.top)
			_br.top = pr.top;
		if (pr.right>_br.right)
			_br.right = pr.right;
		if (pr.bottom>_br.bottom)
			_br.bottom = pr.bottom;
	}
}


void PanelItemsSelection::DistributeVert()
{
	assert(!IsEmpty());
	_SortVert();

	RECT r; 
	_arr[0]->GetRect(r);
	const int miny = r.bottom;
	_arr[_items-1]->GetRect(r);
	const int maxy = r.bottom;

	const float dy = (maxy-miny)/static_cast<float>(_items-1);
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(r);
		int y = static_cast<int>(miny + i*dy);
		int yinc = y - r.bottom;
		_arr[i]->MoveBy(0, yinc);
	}
}


void PanelItemsSelection::DistributeHorz()
{
	assert(!IsEmpty());
	_SortHorz();

	RECT r; 
	_arr[0]->GetRect(r);
	const int minx = r.left;
	_arr[_items-1]->GetRect(r);
	const int maxx = r.left;

	const float dx = (maxx-minx)/static_cast<float>(_items-1);
	for (int i=1; i<_items; i++)
	{
		_arr[i]->GetRect(r);
		int x = static_cast<int>(minx + i*dx);
		int incx = x - r.left;
		_arr[i]->MoveBy(incx, 0);
	}
}



void PanelItemsSelection::_SortVert()
{
	qsort(_arr, _items, sizeof(_arr[0]), PanelItem::VertSortFunc);
}



void PanelItemsSelection::_SortHorz()
{
	qsort(_arr, _items, sizeof(_arr[0]), PanelItem::HorzSortFunc);
}



void PanelItemsSelection::GetRect(RECT& r) const
{
	assert(!IsEmpty());
	r = _br;
}


void PanelItemsSelection::MoveBy(int dx, int dy)
{
	assert(!IsEmpty());

	if (_br.left + dx < 0)
		dx = -_br.left;
	if (_br.top + dy < 0)
		dy = -_br.top;

	for (int i=0; i<_items; i++)
		_arr[i]->MoveBy(dx, dy);
	_br.left += dx;
	_br.right += dx;
	_br.top += dy;
	_br.bottom += dy;
}

bool PanelItemsSelection::InSelection(POINT pt) const
{
	assert(!IsEmpty());
	return (0 != PtInRect(&_br, pt));
}



void PanelItemsSelection::TransferSelection(PanelItemsSelection& src)
{
	Clear();
	for (int i=0; i<src.Items(); i++)
		Add(src.GetSelected(i));
	src.Clear();
}
