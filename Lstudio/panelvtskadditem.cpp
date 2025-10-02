#include <fw.h>

#include "objfgvview.h"

#include "panelprms.h"
#include "panelvwtsk.h"
#include "pnlitmsel.h"
#include "panelview.h"

#include "resource.h"

PanelViewAddSliderTask::PanelViewAddSliderTask(PanelView* pView) : 
PanelViewTask(pView, MAKEINTRESOURCE(IDC_ADDSLIDER))
{}


PanelViewAddSliderTask::~PanelViewAddSliderTask()
{}

void PanelViewAddSliderTask::LButtonDown(KeyState ks, int x, int y)
{
	PanelViewTask::LButtonDown(ks, x, y);
	_pView->AddSlider(x, y);
}

PanelViewAddButtonTask::PanelViewAddButtonTask(PanelView* pView) : 
PanelViewTask(pView, MAKEINTRESOURCE(IDC_ADDBUTTON))
{}


PanelViewAddButtonTask::~PanelViewAddButtonTask()
{}

void PanelViewAddButtonTask::LButtonDown(KeyState ks, int x, int y)
{
	PanelViewTask::LButtonDown(ks, x, y);
	_pView->AddButton(x, y);
}

PanelViewAddLabelTask::PanelViewAddLabelTask(PanelView* pView) : 
PanelViewTask(pView, MAKEINTRESOURCE(IDC_ADDLABEL))
{}


PanelViewAddLabelTask::~PanelViewAddLabelTask()
{}


void PanelViewAddLabelTask::LButtonDown(KeyState ks, int x, int y)
{
	PanelViewTask::LButtonDown(ks, x, y);
	_pView->AddLabel(x, y);
}


PanelViewAddGroupTask::PanelViewAddGroupTask(PanelView* pView) :
PanelViewTask(pView, MAKEINTRESOURCE(IDC_ADDGROUP))
{
	_dragging = false;
}


PanelViewAddGroupTask::~PanelViewAddGroupTask()
{}

void PanelViewAddGroupTask::LButtonDown(KeyState, int x, int y)
{
	_dragging = true;
	_pt1.x = _pt2.x = x;
	_pt1.y = _pt2.y = y;
	_pView->DrawMark(_pt1, _pt2);
}



void PanelViewAddGroupTask::MouseMove(KeyState, int x, int y)
{
	SetCursor(_cursor);
	if (_dragging)
	{
		POINT np = {x, y};
		_pView->DrawMark(_pt1, _pt2, np);
		_pt2 = np;
	}
}

void PanelViewAddGroupTask::LButtonUp(KeyState, int, int)
{
	if (_dragging)
	{
		_pView->DrawMark(_pt1, _pt2);
		_pView->AddGroup(_pt1, _pt2);
		Reset();
	}
}
