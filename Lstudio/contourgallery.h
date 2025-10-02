#ifndef __CONTOURGALLERY_H__
#define __CONTOURGALLERY_H__



class Contour;

class ContourGallery : public GLGallery 
{
public:
	static void Register(HINSTANCE);

	ContourGallery(HWND, const CREATESTRUCT*);
	~ContourGallery();

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	void LoadGallery(const TCHAR*);
	void Generate() const;
	bool Bound() const
	{ return _version>0; }
	bool Empty() const;

	void Add(std::unique_ptr<EditableObject>&);
	bool Contains(const char*) const;
private:
	static const TCHAR* _ClassName()
	{ return __TEXT("ContourGallery"); }

	void _AdaptMenu(HMENU) const;

	void _UniqueName(Contour*) const;
	void _Insert(std::unique_ptr<EditableObject>&, int);

	EditableObject* _NewObject() const;

	const TCHAR* _GetObjectName(int) const;

	// Clipboard support
	static const TCHAR* _ClipboardFormatName()
	{ return __TEXT("LStudioContour"); }
	static UINT _ClipboardFormat;

	UINT _ClipboardFormatId() const
	{ return _ClipboardFormat; }

	void _PostPaste(int);
	void _Generate0000() const;
	void _Generate0010() const;
	void _Generate0101() const;
	void _Load0101(ReadTextFile&);

	void _SetObjectsVersion(int);
	void _Bind();
	void _Unbind();
	void _CurveFormat();

	int _version;
};


#else
	#error File alredy included
#endif
