#ifndef __FUNCEDIT_H__
#define __FUNCEDIT_H__


#define FUNCTIONEDITRESIZABLE

class FunctionView;
class FuncGallery;

class FuncEdit : public ContModeEdit
{
public:
	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);
	FuncEdit(HWND, HINSTANCE, PrjNotifySink*);

	bool Command(int, Window, UINT);
#ifdef FUNCTIONEDITRESIZABLE
	bool Size(SizeState, int, int);
#endif

	void LoadGallery(const TCHAR*);
	void Generate() const;
	void Import(const TCHAR*);

	void ColorschemeModified();
	void Flipped(bool);

	void ApplyNow();
	bool SelectionChanged(int, int);
	int Count() const;
	const TCHAR* GalleryName() const;
	const TCHAR* GetObjectName(int) const;
	void Duplicate(int, const char*);
protected:
	bool _ObjectModified(bool) const;
private:
	void _UpdateControls();
	void _UpdateFromControls();
	void _UpdateView();

	const Icon _moveButton;
	const Icon _addButton;
	const Icon _deleteButton;
	const Icon _setcoordButton;

	const Icon _pointsButton;
	const Icon _linesButton;
	const Icon _curveButton;
	const Icon _axisButton;
	const Icon _gridButton;
	const Icon _labelsButton;

	FunctionView* _theView;
	FuncGallery* _theGallery;

#ifdef FUNCTIONEDITRESIZABLE
	GeometryConstrain _Gallery;
	GeometryConstrain _Name;
	GeometryConstrain _NameLbl;

	GeometryConstrain _EditLbl;
	GeometryConstrain _MoveBtn;
	GeometryConstrain _AddBtn;
	GeometryConstrain _RemoveBtn;
	GeometryConstrain _InputPBtn;

	GeometryConstrain _DispLbl;
	GeometryConstrain _PointsBtn;
	GeometryConstrain _SegmentsBtn;
	GeometryConstrain _CurveBtn;
	GeometryConstrain _AxisBtn;
	GeometryConstrain _GridBtn;
	GeometryConstrain _LabelsBtn;

	GeometryConstrain _FlipBtn;
	GeometryConstrain _SmplsLbl;
	GeometryConstrain _SmplsEdt;

	GeometryConstrain _ViewWnd;
#endif
	ToolTip _tooltips;
};

#else
	#error File already included
#endif
