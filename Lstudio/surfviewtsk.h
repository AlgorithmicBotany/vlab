#ifndef __SURFACEVIEWTSK_H__
#define __SURFACEVIEWTSK_H__


class SurfaceView;

class SurfaceViewPanTask : public GLTask
{
public:
	SurfaceViewPanTask(SurfaceView* pView);
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{
		if (_panning)
		{
			_panning = false;
			ReleaseCapture();
		}
	}
private:
	SurfaceView* _theView;
	bool _panning;
	POINT _LastPos;
};


class SurfaceViewZoomTask : public GLTask
{
public:
	SurfaceViewZoomTask(GLTrackball* pView);
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{
		if (_zooming)
		{
			_zooming = false;
			ReleaseCapture();
		}
	}
private:
	SurfaceView* _theView;
	bool _zooming;
	POINT _LastPos;
};


class SurfaceViewDragPointTask : public GLTask
{
public:
	SurfaceViewDragPointTask(GLTrackball* pView);
	void LButtonDown(KeyState, int, int);
	void MouseMove(KeyState, int, int);
	void LButtonUp(KeyState, int, int);
	void Reset()
	{ 
		if (-1 != _selectedPoint)
		{
			_selectedPoint = -1; 
			ReleaseCapture();
		}
	}
private:
	SurfaceView* _theView;
	int _selectedPoint;
	float _dragZ;
};


#else
	#error File already included
#endif
