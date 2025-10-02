#ifndef __GRIDVIEWTASK_H__
#define __GRIDVIEWTASK_H__


class GridView;

class GridViewTask
{
protected:
	GridViewTask(GridView* pView, Cursor::dc crsr) : _cursor(crsr), _pView(pView) {}
	GridViewTask(GridView* pView, const char* crsr) : _cursor(crsr), _pView(pView) {}
public:
	virtual void LButtonDown(KeyState, int, int) = 0;
	virtual void LBDblClick(KeyState, int, int);
	virtual void MButtonDown(KeyState, int, int) {}
	virtual void LButtonUp(KeyState, int, int) = 0;
	virtual void MouseMove(KeyState, int, int)
	{ 
		SetCursor(_cursor); 
	}
	virtual void Reset() {};
protected:
	Cursor _cursor;
	GridView* _pView;
};


class GridViewIdleTask : public GridViewTask
{
public:
	GridViewIdleTask(GridView* pView) : GridViewTask(pView, Cursor::Arrow) {}
	void LButtonDown(KeyState, int, int) {}
	void LButtonUp(KeyState, int, int) {}
};


class GridViewTranslateTask : public GridViewTask
{
public:
	GridViewTranslateTask(GridView* pView) : GridViewTask(pView, MAKEINTRESOURCE(IDC_PAN))
	{ 
		Reset(); 
	}
	void LButtonDown(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void Reset() 
	{ _translating = false; }
private:
	bool _translating;
	POINT _lastpos;
};


class GridViewZoomTask : public GridViewTask
{
public:
	GridViewZoomTask(GridView* pView) : GridViewTask(pView, MAKEINTRESOURCE(IDC_ZOOM))
	{ 
		Reset(); 
	}
	void LButtonDown(KeyState, int, int);
	void MButtonDown(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void Reset()
	{ _zooming = false; }
private:
	bool _zooming;
	POINT _lastPos;
};




#else
	#error File already included
#endif
