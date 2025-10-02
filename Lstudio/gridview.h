#ifndef __GRIDVIEW_H__
#define __GRIDVIEW_H__



class GridView : public OpenGLWindow
{
public:
	GridView(HWND, const CREATESTRUCT*);
	~GridView();

	bool LButtonDown(KeyState, int, int);
	bool LBDblClick(KeyState, int, int);
	bool MButtonDown(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool MButtonUp(KeyState, int, int);
	bool CaptureChanged();


	void SetScale(float);
	void SetCenter(WorldPointf);

	void SetDrawAxis(bool set = true);
	void SetDrawGrid(bool set = true);
	void SetDrawLabels(bool set = true);
	void MapScreenToWorld(int, int, WorldPointf&) const;
	void Translate(int, int);
	void Zoom(int);

	void ColorschemeModified();
protected:
	void _PaintGrid() const;
	void _DoSize();

	WorldPointf _MinPoint;
	WorldPointf _MaxPoint;
	WorldPointf _center;
	float _scale;
	float _upp;
	WorldPointf _GridMin;
	WorldPointf _GridMax;
	float _GridStep;
	char _Labelformat[16];

	static const LogFont _logFont;
	int _charWidth;
	int _charHeight;

	enum
	{
		eDrawGrid = 1,
			eDrawLabels = 1 << 1,
			eDrawAxis = 1 << 2
	};

	int _GridDrawWhat;

	GridViewTask* _pTask;

	GridViewTask* _pPrevTask; 
	// Stored in case Translate, Rotate or Zoom preempts current task
	// Restored in LButtonUp

	GridViewTranslateTask _TranslateTask;
	GridViewZoomTask _ZoomTask;

	FontList _fontList;
};

#else
	#error File already included
#endif
