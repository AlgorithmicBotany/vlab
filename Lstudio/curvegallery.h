#ifndef __CURVEGALLERY_H__
#define __CURVEGALLERY_H__

class CurveGallery : public GLGallery
{
public:
	static void Register(HINSTANCE);

	CurveGallery(HWND, const CREATESTRUCT*);

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

private:
	static const TCHAR* _ClassName()
	{ return __TEXT("CurveGallery"); }

	EditableObject* _NewObject() const;

	// Clipboard support
	static const TCHAR* _ClipboardFormatName()
	{ return 0; }
	UINT _ClipboardFormatId() const
	{ return 0; }
};

#else
	#error File already included
#endif
