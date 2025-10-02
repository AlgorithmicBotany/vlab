#ifndef __FUNCTIONVIEW_H__
#define __FUNCTIONVIEW_H__


#ifndef __FUNCVIEWTASK_H__
#include "funcviewtask.h"
#endif


class Function;
class FuncEdit;

class FunctionView : public GridView, public ObjectView
{
public:
	static void Register(HINSTANCE);
	
	FunctionView(HWND, const CREATESTRUCT*);
	~FunctionView();

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	void ForceRepaint()
	{
		CurrentContext cc(this);
		_DoPaint();
		cc.SwapBuffers();
	}

	int GetClosestPoint(WorldPointf) const;
	int AddPoint(WorldPointf wp);
	WorldPointf GetPoint(int i) const;
	bool CanDelete(int i) const;
	void DeletePoint(int i);

	void SwitchMovePointMode()
	{ _pPrevTask = _pTask = &_DragTask; }
	void SwitchAddPointMode()
	{ _pPrevTask = _pTask = &_AddPointTask; }
	void SwitchDeletePointMode()
	{ _pPrevTask = _pTask = &_DeletePointTask; }
	void SwitchSetPointMode()
	{ _pPrevTask = _pTask = &_SetPointTask; }


	void MovePointTo(int id, int x, int y);
	const char* GetName() const;
	void SetName(const char* txt);
	void SetFunctionEdit(FuncEdit* pEdit)
	{ _pTheEdit = pEdit; }

	void SetDrawPoints(bool draw = true);
	void SetDrawSegments(bool draw = true);
	void SetDrawCurve(bool draw = true);
	bool Flip() const;
	void Flip(bool);
	void InputPoint(int);
	bool ImplementsSamples() const;
	int GetSamples() const;
	void SetSamples(int);

	void CopyObject(const EditableObject*);

	void Modified(bool);
private:
	static const TCHAR* _ClassName()
	{ return __TEXT("FunctionView"); }
	void _DoPaint() const;
	const Function* _GetFunction() const;
	Function* _GetFunction();
	void _ResetView();
	void _FlipFunction();

	FunctionViewDragPointTask _DragTask;
	FunctionViewAddPointTask _AddPointTask;
	FunctionViewDeletePointTask _DeletePointTask;
	FunctionViewSetPointTask _SetPointTask;

	int _drawWhat;
	FuncEdit* _pTheEdit;
};

#else
	#error File already included
#endif
