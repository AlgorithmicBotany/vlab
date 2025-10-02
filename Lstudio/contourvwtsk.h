#ifndef __CONTOURVIEWDRAGTASK_H__
#define __CONTOURVIEWDRAGTASK_H__


class ContourView;


class ContourViewTask : public GridViewTask
{
public:
	ContourViewTask(ContourView* pView, const char*);
	ContourViewTask(ContourView* pView, Cursor::dc);
};


class ContourViewDragPointTask : public ContourViewTask
{
public:
	ContourViewDragPointTask(ContourView* pView);
	void LButtonDown(KeyState, int, int);
	void LBDblClick(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{ _selected = -1; }
private:
	int _selected;
};



class ContourViewAddPointTask : public ContourViewTask
{
public:
	ContourViewAddPointTask(ContourView* pView);
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{ _added = -1; }
private:
	int _added;
};


class ContourViewDeletePointTask : public ContourViewTask
{
public:
	ContourViewDeletePointTask(ContourView* pView);
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
};


class ContourViewSetPointTask : public ContourViewTask
{
public:
	ContourViewSetPointTask(ContourView* pView2);
	void LButtonDown(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
};


#else
	#error File already included
#endif
