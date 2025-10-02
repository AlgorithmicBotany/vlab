#ifndef __GLTASK_H__
#define __GLTASK_H__


class GLTrackball;

class GLTask
{
public:
	GLTask(GLTrackball* pView, const char* crid) : _pView(pView), _cursor(crid)
	{}
	GLTask(GLTrackball* pView, Cursor::dc crid) : _pView(pView), _cursor(crid)
	{}
	virtual void LButtonDown(KeyState, int, int) = 0;
	virtual void LButtonUp(KeyState, int, int) = 0;
	virtual void MouseMove(KeyState, int, int)
	{ SetCursor(_cursor); }
	virtual void Reset() {}
protected:
	GLTrackball* _pView;
	Cursor _cursor;
};


class GLIdleTask : public GLTask
{
public:
	GLIdleTask(GLTrackball* pView) : GLTask(pView, Cursor::Arrow)
	{}
	void LButtonDown(KeyState, int, int) {}
	void LButtonUp(KeyState, int, int) {}
};


class GLRotateTask : public GLTask
{
public:
	GLRotateTask(GLTrackball* pView);
	void LButtonDown(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void Reset()
	{ _rotating = false; }
private:
	POINT _PrevPos;
	bool _rotating;

};


#endif

