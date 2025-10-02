#ifndef __MATERIALGALLERY_H__
#define __MATERIALGALLERY_H__

class MaterialEdit;

#define MATGALRESIZEABLE


class MaterialGallery : public GLGallery
{
public:
	static void Register(HINSTANCE);

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);
	bool HScroll(HScrollCode, int, HWND);
	bool LButtonUp(KeyState, int, int);
	bool MouseMove(KeyState, int, int);

	MaterialGallery(HWND, const CREATESTRUCT*);
	~MaterialGallery();

	void SetMaterialEdit(MaterialEdit* pEdit);
	void Import(const TCHAR*);
	
	void Clear();
	void Generate(WriteBinFile&) const;
private:
	static const TCHAR* _ClassName()
	{ return __TEXT("MaterialGallery"); }
	static const TCHAR* _ClipboardFormatName()
	{ return __TEXT("LStudioMaterial"); }

	void _Paste() { assert(0); }
	void _PasteBefore() { assert(0); }
	void _PasteAfter() { assert(0); }
	void _PasteInto();
	void _DoPaint() const;
	void _DoInit();
	void _DoExit();
	void _DoSize();

	void _UpdateScrollbar();
	// Clipboard support
	static UINT _ClipboardFormat;

	UINT _ClipboardFormatId() const
	{ return _ClipboardFormat; }
	void _CopyClipboardToSelection(const void*);

	void _Interpolate();

	void _AdaptMenu(HMENU) const;
	void _MapScreenToWorld(const ScreenPoint&, WorldPointf&) const;

	int _DetermineSelection(int, int) const;
	int _DetermineInsertion(int, int) const;
	void _DrawInsertLine(int) const;

	EditableObject* _NewObject() const;

	GLUquadricObj* _pQobj;

	enum 
	{ 
		NumOfMaterials = 256,
			MaterialsPerColumn = 4
	};

	MaterialEdit* _pMaterialEdit;

	float _upp;
	WorldPointf _MinPoint;
#ifdef MATGALRESIZEABLE
	int _columns;
#endif

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
