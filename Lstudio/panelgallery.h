#ifndef __PANELGALLERY_H__
#define __PANELGALLERY_H__


class PrjNotifySink;
class PanelDesign;

class PanelGallery : public Ctrl, public ObjectGallery
{
public:

	PanelGallery(HWND, const CREATESTRUCT*);
	~PanelGallery();

	static void Register(HINSTANCE);

	bool Paint();
	bool Size(SizeState, int, int);
	bool LButtonDown(KeyState, int, int);
	bool LBDblClick(KeyState, int, int);
	bool HScroll(HScrollCode, int, HWND);
	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	void ForceRepaint()
	{ Invalidate(); }
	void Add(std::unique_ptr<EditableObject>& pNew);
	bool TearedOffPanels() const;
	void CloseTearedOff();
	void TearOffAll();
	void SwitchToExecute()
	{ _designmode = false; }
	void SwitchToDesign()
	{ _designmode = true; }
	void SetProjectNotifySink(PrjNotifySink* pNotifySink)
	{ _pNotifySink = pNotifySink; }

	bool Contains(const char*) const;
private:
	static const TCHAR* _ClassName()
	{ return __TEXT("PanelGallery"); }
	int _DetermineSelection(int, int) const;
	EditableObject* _NewObject() const;
	void _UniqueName(PanelDesign*) const;
	void _Insert(std::unique_ptr<EditableObject>&, int);


	void _Execute();
	void _ExecuteAll();

	HWND _ClipboardOwner() const
	{ return Hwnd(); }
	static UINT _ClipboardFormat;
	static const TCHAR* _ClipboardFormatName()
	{ return __TEXT("LStudioPanel"); }
	UINT _ClipboardFormatId() const
	{ return _ClipboardFormat; }


	void _UpdateScrollbar();
	int _FirstVisible;

	void _AdaptMenu(HMENU);
	bool _designmode;

	PrjNotifySink* _pNotifySink;
};

#else
	#error File already included
#endif

