#ifndef __SURFACEEDIT_H__
#define __SURFACEEDIT_H__


class SurfaceView;
class SurfaceGallery;
class LineThumb;



class SurfaceEdit : public ContModeEdit
{
public:
	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);

	SurfaceEdit(HWND, HINSTANCE, PrjNotifySink*);


	bool Command(int, Window, UINT);
	bool Size(SizeState, int, int);

	void MoveX(float, bool);
	void MoveY(float, bool);
	void MoveZ(float, bool);

	// For use by SurfaceView
	void PointSelected(int);
	void PointMoved(bool);
	void EndDrag();

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
	XMovedCallback _XMoved;
	YMovedCallback _YMoved;
	ZMovedCallback _ZMoved;

	LineThumb* _pXThumb;
	LineThumb* _pYThumb;
	LineThumb* _pZThumb;

	void _UpdateControls();
	void _UpdateView();
	void _ActivePointChanged(int);
	void _UpdateFromControls();


	void _Advanced();

	SurfaceView* _theView;
	SurfaceGallery* _theGallery;

	TCHAR _Filename[_MAX_PATH+1];

	GeometryConstrain _Gallery;

	
	GeometryConstrain _XThumb;
	GeometryConstrain _YThumb;
	GeometryConstrain _ZThumb;

	GeometryConstrain _CP01;
	GeometryConstrain _CP02;
	GeometryConstrain _CP03;
	GeometryConstrain _CP04;
	GeometryConstrain _CP05;
	GeometryConstrain _CP06;
	GeometryConstrain _CP07;
	GeometryConstrain _CP08;
	GeometryConstrain _CP09;
	GeometryConstrain _CP10;
	GeometryConstrain _CP11;
	GeometryConstrain _CP12;
	GeometryConstrain _CP13;
	GeometryConstrain _CP14;
	GeometryConstrain _CP15;
	GeometryConstrain _CP16;
	GeometryConstrain _PLCM;
	GeometryConstrain _CTPT;

	GeometryConstrain _Name;
	GeometryConstrain _NameLbl;

	GeometryConstrain _Preview;

	GeometryConstrain _AdvancedBtn;
	GeometryConstrain _SecretBtn;
};


#else
	#error File already included
#endif
