#ifndef __PANELSEDIT_H__
#define __PANELSEDIT_H__


#define PANELSEDITRESIZEABLE

class PrjNotifySink;
class PanelView;
class PanelGallery;

class PanelEdit : public FormCtrl, public ObjectEdit
{
public:
	PanelEdit(HWND, HINSTANCE, PrjNotifySink*);
	~PanelEdit();

	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);

	bool Command(int, Window, UINT);
	bool MeasureItem(OwnerDraw::Measure);
	bool DrawItem(OwnerDraw::Draw);

#ifdef PANELSEDITRESIZEABLE
	bool Size(SizeState, int, int);
#endif

	void Generate() const;
	void Import(const TCHAR*);

	void ExecuteMode();
	void TearOffAll();
private:

	void LsystemSelected();
	void ViewSelected();
	void FileSelected();

	enum PanelMode
	{
		pmDesign,
			pmExecute
	};

	void _UpdateControls();
	void _UpdateFromControls();
	void _UpdateView();

	void _BrowseTargetFile();
	void _Mode(PanelMode);

	void _AlignHorz();
	void _AlignVert();

	// if the file is in current directory 
	// the directory is removed from the buffer
	bool _InCurrentDir(TCHAR*) const;

	void _DrawFace(OwnerDraw::Draw&) const;
	void _DrawSelect(OwnerDraw::Draw&) const;

	HINSTANCE _hInst;

	Icon _SelectButton;
	Icon _DeleteButton;
	Icon _SliderButton;
	Icon _ButtonButton;
	Icon _GroupButton;
	Icon _LabelButton;

	Icon _AlignLButton;
	Icon _AlignLButtonA;
	Icon _AlignRButton;
	Icon _AlignTButton;
	Icon _AlignBButton;
	Icon _AlignBButtonA;

	Icon _CenterHButton;
	Icon _CenterVButton;

	Icon _DisrtVButton;
	Icon _DisrtHButton;

	Icon _DuplDwnButton;
	Icon _DuplRgtButton;

	PanelView* _theView;
	PanelGallery* _theGallery;

	PanelMode _curMode;

#ifdef PANELSEDITRESIZEABLE
	GeometryConstrain _Gallery;
	GeometryConstrain _Name;
	GeometryConstrain _NameLbl;

	GeometryConstrain _DuplDwn;
	GeometryConstrain _DuplRgt;

	GeometryConstrain _AlignLft;
	GeometryConstrain _AlignBtm;
	GeometryConstrain _DistHrz;
	GeometryConstrain _DistVrt;
	GeometryConstrain _AlignLbl;

	GeometryConstrain _SelectBtn;
	GeometryConstrain _DeleteBtn;
	GeometryConstrain _SliderBtn;
	GeometryConstrain _GroupBtn;
	GeometryConstrain _ButtonBtn;
	GeometryConstrain _LabelBtn;
	GeometryConstrain _ActionLbl;

	GeometryConstrain _AssocLbl;
	GeometryConstrain _AssocLs;
	GeometryConstrain _AssocVw;
	GeometryConstrain _AssocFile;
	GeometryConstrain _AssocFlnm;
	GeometryConstrain _AssocFlBr;

	GeometryConstrain _TriggerLbl;
	GeometryConstrain _Trigger;

	GeometryConstrain _ModeGrp;
	GeometryConstrain _DesignBtn;
	GeometryConstrain _ExecuteBtn;

	GeometryConstrain _ViewWnd;
#endif
	ToolTip _tooltips;

	DECLARE_COUNTER;
};

#else
	#error File already included
#endif
