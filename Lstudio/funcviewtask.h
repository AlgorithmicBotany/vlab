#ifndef __FUNCVIEWTASK_H__
#define __FUNCVIEWTASK_H__


class FunctionView;

class FunctionViewTask : public GridViewTask
{
public:
	FunctionViewTask(FunctionView* pView, Cursor::dc);
	FunctionViewTask(FunctionView* pView, const char*);
};


class FunctionViewDragPointTask : public FunctionViewTask
{
public:
	FunctionViewDragPointTask(FunctionView* pView);
	void LButtonDown(KeyState, int, int);
	void LBDblClick(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{ _selected = -1; }
private:
	int _selected;
	POINT _initPos;
};



class FunctionViewAddPointTask : public FunctionViewTask
{
public:
	FunctionViewAddPointTask(FunctionView* pView);
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{ _added = -1; }
private:
	int _added;
};


class FunctionViewDeletePointTask : public FunctionViewTask
{
public:
	FunctionViewDeletePointTask(FunctionView* pView);
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
};

class FunctionViewSetPointTask : public FunctionViewTask
{
public:
	FunctionViewSetPointTask(FunctionView* pView);
	void LButtonDown(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
};



#else
	#error File already included
#endif
