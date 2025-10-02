#ifndef __PANELVIEWTSK_H__
#define __PANELVIEWTSK_H__


class PanelView;
class PanelItem;
class PanelItemsSelection;

class PanelViewTask
{
protected:
	PanelViewTask(PanelView* pView, const char* crid) : _pView(pView), _cursor(crid)
	{}
	PanelViewTask(PanelView* pView, Cursor::dc crid) : _pView(pView), _cursor(crid)
	{}

public:
	virtual ~PanelViewTask() {}

	virtual void LButtonDown(KeyState, int, int);
	virtual void LBDblClick(KeyState, int, int);
	virtual void MouseMove(KeyState, int, int);
	virtual void LButtonUp(KeyState, int, int) {}
	virtual void Reset() {}

protected:
	PanelView* _pView;
	Cursor _cursor;
};


class PanelViewSelectTask : public PanelViewTask
{
public:
	PanelViewSelectTask(PanelView* pView);
	~PanelViewSelectTask();
	void LButtonDown(KeyState, int, int);
	void LBDblClick(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset();
private:
	bool _marking;
	POINT _pt1, _pt2;
};


class PanelViewDragItemTask : public PanelViewTask
{
public:
	PanelViewDragItemTask(PanelView*);
	~PanelViewDragItemTask();
	void StartDragging(POINT, POINT, RECT);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
protected:
	ScreenPoint _lastPos;
	ScreenPoint _initPos;
	RECT _dragRect;
};

/*********************************************************

	PanelViewAddXXXXXXXXXXXXXTask -- see below

*********************************************************/



























class PanelViewAddSliderTask : public PanelViewTask
{
public:
	PanelViewAddSliderTask(PanelView* pView);
	~PanelViewAddSliderTask();
	void LButtonDown(KeyState, int, int);
};


class PanelViewAddButtonTask : public PanelViewTask
{
public:
	PanelViewAddButtonTask(PanelView* pView);
	~PanelViewAddButtonTask();
	void LButtonDown(KeyState, int, int);
};



class PanelViewAddLabelTask : public PanelViewTask
{
public:
	PanelViewAddLabelTask(PanelView* pView);
	~PanelViewAddLabelTask();
	void LButtonDown(KeyState, int, int);
};


class PanelViewAddGroupTask : public PanelViewTask
{
public:
	PanelViewAddGroupTask(PanelView* pView);
	~PanelViewAddGroupTask();
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{ _dragging = false; }
private:
	POINT _pt1;
	POINT _pt2;
	bool _dragging;
};


class PanelViewExecuteTask : public PanelViewTask
{
public:
	PanelViewExecuteTask(PanelView* pView);
	~PanelViewExecuteTask();
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{ _dragging = false; }
	POINT LastPoint() const
	{ return _last; }
private:
	POINT _last;
	bool _dragging;
};


#else
	#error File already included
#endif
