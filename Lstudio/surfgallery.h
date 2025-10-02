#ifndef __SURFACEGALLERY_H__
#define __SURFACEGALLERY_H__


class Patch;
class SurfaceEdit;

class SurfaceGallery : public GLGallery 
{
public:
	static void Register(HINSTANCE);

	SurfaceGallery(HWND, const CREATESTRUCT*);
	~SurfaceGallery();

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	static const TCHAR* _ClassName()
	{ return __TEXT("SurfaceGallery"); }

	void Add(std::unique_ptr<EditableObject>&);
	bool Contains(const char*) const;
private:

	void _AdaptMenu(HMENU) const;

	EditableObject* _NewObject() const;

	void _UniqueName(Surface*) const;
	void _Insert(std::unique_ptr<EditableObject>&, int);

	const TCHAR* _GetObjectName(int) const;

	// Clipboard support
	static UINT _ClipboardFormat;
	static const TCHAR* _ClipboardFormatName()
	{ return __TEXT("LStudioSurface"); }

	UINT _ClipboardFormatId() const
	{ return _ClipboardFormat; }

};


#else
	#error File already included
#endif
