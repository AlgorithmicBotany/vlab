#ifndef __PANELVIEW_H__
#define __PANELVIEW_H__


class PrjNotifySink;
class PanelDesign;

class PanelView : public Scrollable, public ObjectView
{
public:
	PanelView(HWND, const CREATESTRUCT*);
	~PanelView();

	static void Register(HINSTANCE);


	void DoPaint(Canvas&, const RECT&);
	bool LButtonDown(KeyState, int, int);
	bool LBDblClick(KeyState, int, int);
	bool LButtonUp(KeyState, int, int);
	bool MouseMove(KeyState, int, int);
	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);
	bool CaptureChanged();
	int GetDlgCode();
	bool KeyDown(UINT);
	bool KillFocus(HWND);
	bool Timer(UINT);



	void Invalidate()
	{
		Ctrl::Invalidate();
		UpdateScrollbars();
	}

	void ForceRepaint()
	{ Invalidate(); }


	void SwitchToExecute();
	void SwitchToDesign();
	void SwitchToSelect();
	void SwitchToAddSlider();
	void SwitchToAddButton();
	void SwitchToAddLabel();
	void SwitchToAddGroup();
	void StartDragging(POINT, POINT);
	void Delete();
	void MoveBy(int, int);
	RECT ExecuteHit(int, int);
	void ExecuteDrag(int, int, bool final = false);

	void AddSlider(int, int);
	void AddButton(int, int);
	void AddLabel(int, int);
	void AddGroup(POINT, POINT);

	void ItemProperties();

	bool SelectAt(int, int); // returns true if anything selected
	bool SelectAdd(int, int);
	bool SelectionEmpty() const;
	void SelectArea(POINT, POINT);
	bool InSelection(POINT) const;

	const char* GetName() const;
	void SetName(const char*);
	void DrawDragRect(const RECT&) const;
	void DrawDragRect(RECT&, int, int) const;

	void DrawMark(POINT, POINT) const;
	void DrawMark(POINT, POINT, POINT) const;

	void AlignLeft();
	void AlignRight();
	void AlignTop();
	void AlignBottom();
	void HCenter();
	void VCenter();

	void DistributeHorz();
	void DistributeVert();

	void DuplicateDown();
	void DuplicateRight();

	void CopyObject(const EditableObject*);

	PanelParameters::TriggerCommand Trigger() const;
	void SetTrigger(PanelParameters::TriggerCommand);
	bool IsValidTrigger(int) const;
	PanelParameters::PanelTarget PanelTarget() const;
	const char* PanelTargetFilename() const;
	void SetPanelTarget(PanelParameters::PanelTarget target);
	void SetPanelTargetFilename(const Window&);

	void SetProjectNotifySink(PrjNotifySink* pSink)
	{ _pSink = pSink; }

private:

	bool _IsExecuteMode() const
	{ return (void*)(_pTask) == (void*)(&_ExecuteTask); }

	void _AdaptExecuteCMenu(MenuManipulator&) const;

	void _ResetPanel();

	PanelItem* _GetItemAt(int, int);

	static const TCHAR* _ClassName()
	{ return __TEXT("PanelView"); }

	PanelDesign* _GetDesign();
	const PanelDesign* _GetDesign() const;

	void _NewLsystem();
	void _NewView();
	void _NewModel();
	void _Rerun();

	PanelItemsSelection _selection;

	PanelViewTask* _pTask;

	PanelViewSelectTask _SelectTask;
	PanelViewAddSliderTask _AddSliderTask;
	PanelViewAddButtonTask _AddButtonTask;
	PanelViewAddLabelTask _AddLabelTask;
	PanelViewAddGroupTask _AddGroupTask;
	PanelViewDragItemTask _DragItemTask;
	PanelViewExecuteTask _ExecuteTask;

	Pen _DragPen;
	Pen _MarkPen;

	HFONT _font;

	bool _dragging;
	FW::Timer _timer;
	enum
	{ 
		eMargin = 20,
		eTimerId = 1,
		eTimerTimeout = 250
	};
	int MaxScrollX() const;
	int MaxScrollY() const;
	int MaxX() const;
	int MaxY() const;

	PrjNotifySink* _pSink;

};

#else
	#error File already included
#endif
