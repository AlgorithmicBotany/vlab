#ifndef __OBJFGVGALLERY_H__
#define __OBJFGVGALLERY_H__

#include <memory>

class EditableObject;
class ObjectEdit;


class ObjectGallery
{
public:
	ObjectGallery(int initSize = 8);
	virtual ~ObjectGallery();
	void SetEdit(ObjectEdit* pEdit)
	{ _pEdit = pEdit; }
	const EditableObject* GetObject(int) const;
	void SetObject(int i, const EditableObject*);
	void SetCurrent(int i)
	{
		assert(i>=0);
		assert(i<_items);
		_current = i;
		_FirstSelected = _current;
		_LastSelected = _current;
	}
	int GetCurrent() const
	{ return _current; }
	virtual void ForceRepaint() = 0;
	virtual void Add(std::unique_ptr<EditableObject>&);

	virtual void Clear();
	int Items() const
	{ return _items; }
	void Delete()
	{ _Delete(); }
	virtual void ScrollToShow() {}
protected:
	EditableObject* _GetObject(int) const;

	virtual EditableObject* _NewObject() const = 0;
	void _New();
	virtual void _Delete();


	// Clipboard functionality
	UINT _ClipboardFormat;
	virtual void _Cut();
	virtual void _Copy() const;
	virtual void _Paste();
	virtual void _PasteBefore();
	virtual void _PasteAfter();
	virtual void _PostPaste(int) {}

	virtual HWND _ClipboardOwner() const = 0;
	virtual DWORD _ClipboardSize() const;
	virtual void _CopySelectionToClipboard(void*) const;
	virtual UINT _ClipboardFormatId() const = 0;
	virtual void _CopyClipboardToSelection(const void*, int);


	void _Insert(std::unique_ptr<EditableObject>&, int);

	virtual int _DetermineSelection(int, int) const = 0;
	std::unique_ptr<EditableObject>* _arr;

	void _Grow(int sz=-1);
	void _Empty();
	int _arrSize;
	int _items;
	int _current;
	int _FirstSelected;
	int _LastSelected;

	ObjectEdit* _pEdit;

	DECLARE_COUNTER;
};



#else
	#error File already included
#endif
