#ifndef __FUNCTIONGALLERY_H__
#define __FUNCTIONGALLERY_H__


class Function;

class FuncGallery : public GLGallery
{
public:
	static void Register(HINSTANCE);

	FuncGallery(HWND, const CREATESTRUCT*);
	~FuncGallery();

	void ContextMenu(HWND, UINT, UINT);
	bool Command(int, Window, UINT);

	void LoadGallery(const TCHAR*);
	void Generate() const;
	void Unbind();
	bool Bound() const
	{ return _version>0; }
	const TCHAR* GalleryName() const
	{ return __TEXT("functions.fset"); }
	int Count() const;
	bool Empty() const
	{ return Count()==0; }
	const TCHAR* GetObjectName(int) const;
	void Duplicate(int, const char*);
	void Add(std::unique_ptr<EditableObject>&);
	bool Contains(const char*) const;
private:
	static const TCHAR* _ClassName()
	{ return __TEXT("FunctionGallery"); }

	void _AdaptMenu(HMENU) const;

	void _UniqueName(Function*) const;
	void _Insert(std::unique_ptr<EditableObject>&, int);

	EditableObject* _NewObject() const;

	const TCHAR* _GetObjectName(int) const;
	void _SetVersion(int);

	// Clipboard support
	static UINT _ClipboardFormat;
	static const TCHAR* _ClipboardFormatName()
	{ return __TEXT("LStudioFunction"); }
	UINT _ClipboardFormatId() const
	{ return _ClipboardFormat; }

	void _Bind();
	void _Unbind();
	void _ToggleNewFormat();

	void _PostPaste(int);
	void _Generate0000() const;
	void _Generate0101() const;
	void _Load0101(ReadTextFile&);

	int _version;
};


#else
	#error File already included
#endif
