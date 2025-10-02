#ifndef __SNAPTASK_H__
#define __SNAPTASK_H__


class SnapIcon;

class SnapTask
{
public:
	enum eSide
	{
		sTop,
		sLeft,
		sBottom,
		sRight
	};
	SnapTask(SnapIcon* pSnap, Pen& pen, Canvas& cnv, HCURSOR hCursor) : 
		_pSnap(pSnap), 
		_pen(pen),
		_hCursor(hCursor),
		_capture(false),
		_cnv(cnv)
	{}
	virtual void LBDown(eSide, int, int) = 0;
	virtual void MouseMove(int, int) = 0;
	virtual void LBUp(int, int);
	virtual void CaptureChanged();
	HCURSOR Cursor() const
	{ return _hCursor; }
	virtual bool Capturing() const
	{ return false; }
protected:
	void DrawRubberband() const;
	static const ROp::Rop mode;
	SnapIcon* _pSnap;
	Pen& _pen;
	HCURSOR _hCursor;
	bool _capture;
	Canvas& _cnv;
	RECT _r;
};


class SnapMove : public SnapTask
{
public:
	SnapMove(SnapIcon*, Pen&, Canvas&);
	void LBDown(eSide, int, int);
	void MouseMove(int, int);
	bool Capturing() const
	{ return _capture; }
private:
	void MoveSnap(int, int);
	POINT _prevpos;
};

class SnapResize : public SnapTask
{
public:
	SnapResize(SnapIcon*, Pen&, Canvas&);
	void LBDown(eSide, int, int);
	void MouseMove(int, int);
	bool FixedAspect() const
	{ return _keepAspect; }
	void ToggleFixedAspect()
	{ _keepAspect = !_keepAspect; }
private:
	bool ResizeSnap(eSide, bool, int, int);
	eSide _side;
	POINT _prevpos;
	bool _keepAspect;
};



#endif
