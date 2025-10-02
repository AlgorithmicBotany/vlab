#ifndef __CONTOURVIEW_H__
#define __CONTOURVIEW_H__


class ContourEdit;

class ContourView : public GridView, public ObjectView
{
public:
	static void Register(HINSTANCE);
	
	ContourView(HWND, const CREATESTRUCT*);
	~ContourView();

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	void SetDrawPoints(bool set=true);
	void SetDrawSegments(bool set=true);
	void SetDrawCurve(bool set=true);

	void SetClosed(bool set=true);
	bool IsClosed() const
	{ return _GetContour()->IsClosed(); }
	const char* GetName() const
	{ return _GetContour()->GetName(); }
	void SetName(const char* bf)
	{ _GetContour()->SetName(bf); }

	void SetContourEdit(ContourEdit* pEdit)
	{ _pTheEdit = pEdit; }

	void ForceRepaint()
	{ 
		CurrentContext cc(this);
		_DoPaint();
		cc.SwapBuffers();
	}
	
	void SwitchMovePointMode()
	{ _pPrevTask = _pTask = &_DragTask; }
	void SwitchAddPointMode()
	{ _pPrevTask = _pTask = &_AddPointTask; }
	void SwitchDeletePointMode()
	{ _pPrevTask = _pTask = &_DeletePointTask; }
	void SwitchSetPointMode()
	{ _pPrevTask = _pTask = &_SetPointTask; }
	
	int GetClosestPoint(WorldPointf p) const;
	int AddPoint(WorldPointf wp)
	{ return _GetContour()->AddPoint(wp); }
	void MovePointTo(int, int, int);
	void CopyObject(const EditableObject*);
	WorldPointf GetPoint(int i) const
	{
		return _GetContour()->GetPoint(i);
	}
	bool CanDelete(int i) const
	{ return _GetContour()->CanDelete(i); }
	void DeletePoint(int i)
	{ _GetContour()->DeletePoint(i); }

	bool CanIncMultiplicity(int i) const
	{ return _GetContour()->CanIncMultiplicity(i); }
	void IncMultiplicity(int i) 
	{ _GetContour()->IncMultiplicity(i); }
	void InputPoint(int);

	void Modified(bool);
private:

	void _DoPaint() const;

	void _ResetZoom();
	

	static const TCHAR* _ClassName()
	{ return __TEXT("ContourView"); }
	const Contour* _GetContour() const
	{ return dynamic_cast<const Contour*>(_pObj); }
	Contour* _GetContour()
	{ return dynamic_cast<Contour*>(_pObj); }

	ContourViewDragPointTask _DragTask;
	ContourViewAddPointTask _AddPointTask;
	ContourViewDeletePointTask _DeletePointTask;
	ContourViewSetPointTask _SetPointTask;

	ContourEdit* _pTheEdit;
	int _drawWhat;
};


#else
	#error File already included
#endif
