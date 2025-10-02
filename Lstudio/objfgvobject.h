#ifndef __OBJFGVOBJECT_H__
#define __OBJFGVOBJECT_H__


class EditableObject
{
public:
	EditableObject()
	{ INC_COUNTER; }
	virtual ~EditableObject() 
	{ DEC_COUNTER; }
	virtual void Copy(const EditableObject*) = 0;
	virtual EditableObject* Clone() const = 0;
	virtual void Reset() = 0;
	virtual void DrawInGallery() const = 0;
	virtual DWORD ClipboardSize() const = 0;
	virtual char* CopyToClipboard(char*) const = 0;
	virtual const char* LoadFromClipboard(const char*) = 0;
	virtual bool operator!=(const EditableObject&) const
	{ return true; }
private:
	void operator=(EditableObject);

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
