#ifndef __CONTOUREDIT_H__
#define __CONTOUREDIT_H__


#define CONTOUREDITRESIZABLE


class ContourView;
class ContourGallery;


class ContourEdit : public ContModeEdit
{
public:
	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);
	ContourEdit(HWND, HINSTANCE, PrjNotifySink*);

	bool Command(int, Window, UINT);
#ifdef CONTOUREDITRESIZABLE
	bool Size(SizeState, int, int);
#endif

	void LoadGallery(const TCHAR*);
	void Generate() const;
	void Import(const TCHAR*);
	void Export() const;

	void ColorschemeModified();

	void ApplyNow();
	bool SelectionChanged(int, int);

	void Clear()
	{
		ObjectEdit::Clear();
		_Filename[0] = 0;
	}
protected:
	bool _ObjectModified(bool) const;
private:

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
	void _UpdateControls();
	void _UpdateFromControls();
	void _UpdateView();

	void _BsplineRegular();
	void _BsplineEndPoint();

	ContourView* _theView;
	ContourGallery* _theGallery;

	TCHAR _Filename[_MAX_PATH+1];

#ifdef CONTOUREDITRESIZABLE
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

	GeometryConstrain _ClosedChk;
	GeometryConstrain _ViewWnd;
	GeometryConstrain _SmplsLbl;
	GeometryConstrain _SmplsEdt;

	GeometryConstrain _BTypeLbl;
	GeometryConstrain _BType;
#endif
	ToolTip _tooltips;
};




#else
	#error File already included
#endif
