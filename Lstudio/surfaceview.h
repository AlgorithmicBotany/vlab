#ifndef __SURFACEVIEW_H__
#define __SURFACEVIEW_H__


class SurfaceEdit;

class SurfaceView : public GLTrackball, public ObjectView
{
public:
	static void Register(HINSTANCE);

	SurfaceView(HWND, const CREATESTRUCT*);
	~SurfaceView();

	bool LButtonDown(KeyState, int, int);
	bool MButtonDown(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool MButtonUp(KeyState, int, int);
	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	const WorldPointf& GetPoint(int ix) const;
	const WorldPointf& GetActivePoint() const
	{ return GetPoint(_activePoint); }
	void SetActivePointX(float x)
	{ _GetSurface()->SetPointX(_activePoint, x); }
	void SetActivePointY(float y)
	{ _GetSurface()->SetPointY(_activePoint, y); }
	void SetActivePointZ(float z)
	{ _GetSurface()->SetPointZ(_activePoint, z); }
	const char* GetName() const
	{ return _GetSurface()->GetName(); }
	void SetName(const char* name) 
	{ _GetSurface()->SetName(name); }
	void ForceRepaint()
	{ 
		CurrentContext cc(this);
		_DoPaint();
		cc.SwapBuffers();
	}
	void SetActivePoint(int ap)
	{ _activePoint = ap; }
	void SetEdit(SurfaceEdit* pEdit)
	{ _theEdit = pEdit; }

	void CopyObject(const EditableObject*);


	/*******************************

		T A S K S

    *******************************/

	void SwitchToIdleTask()
	{
		_pTask = &_IdleTask;
	}

	void SwitchToPanTask()
	{
		_pTask = &_PanTask;
		_pTask->Reset();
	}

	void SwitchToRotateTask()
	{
		_pTask = &_RotateTask;
		_pTask->Reset();
	}
	void SwitchToDragPointTask()
	{
		_pTask = &_DragPointTask;
		_pTask->Reset();
	}
	void SwitchToZoomTask()
	{
		_pTask = &_ZoomTask;
		_pTask->Reset();
	}

	// For use by _DragPointTask
	int GetXYClosestPoint(WorldPointf) const;
	void PointSelectedForDragging(int);
	void DragPoint(int, WorldPointf);
	void EndDrag();

	// For use by _ZoomTask
	void Zoom(int);

	// For use by _PanTask
	void PanBy(int, int);

	void ColorschemeModified();
private:

	void _DrawNonEditable() const;
	void _DoInit();
	void _DoExit();
	void _DoPaint() const;
	void _SetViewbox();
	void _LockXYDesign();
	void _ToggleDrawParam(int);
	void _SetContextMenuChecks(HMENU) const;

	bool _XYLock() const
	{ return _LockXY; }
	bool _YZSymmetric() const;

	void _KeepYZSymmetric();


	Surface* _GetSurface();
	const Surface* _GetSurface() const;
	SurfaceEdit* _theEdit;

	static const TCHAR* _ClassName()
	{ return __TEXT("SurfaceView"); }

	int _activePoint;
	int _drawWhat;
	bool _LockXY;
	WorldPointf _centershift;
	float _scale;
	SIZE _sz;

	GLIdleTask _IdleTask;
	SurfaceViewDragPointTask _DragPointTask;
	SurfaceViewZoomTask _ZoomTask;
	SurfaceViewPanTask _PanTask;

	FontList _fontList;
};



#else
	#error File already included
#endif
