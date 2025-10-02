#ifndef __THUMBTASK_H__
#define __THUMBTASK_H__


class LineThumb;

class ThumbTask
{
public:
	ThumbTask(LineThumb* pThumb) : _pThumb(pThumb)
	{}
	virtual void ButtonDown(int x, int y);
	virtual void MouseMove(int, int) = 0;
	virtual void ButtonUp() { ReleaseCapture(); }
protected:
	LineThumb* _pThumb;
	POINT _LastPos;
};

class MovingTask : public ThumbTask
{
public:
	MovingTask(LineThumb* pThumb) : ThumbTask(pThumb)
	{ _TimerId = 0; }
	void ButtonDown(int, int);
	void MouseMove(int, int);
	void ButtonUp();
private:
	int _TimerId;
};




#else
	#error File already included
#endif
