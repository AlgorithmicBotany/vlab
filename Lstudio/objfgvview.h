#ifndef __OBJFGVVIEW_H__
#define __OBJFGVVIEW_H__


class EditableObject;
class ObjectEdit;

class ObjectView
{
public:
	ObjectView();
	virtual ~ObjectView();

	void SetEdit(ObjectEdit* pEdit)
	{ _pEdit = pEdit; }
	virtual void CopyObject(const EditableObject*);
	EditableObject* GetObject()
	{ return _pObj; }
	virtual void ForceRepaint() = 0;
protected:
	void _ApplyNow();
	void _Retrieve();
	void _AddAsNew();

	ObjectEdit* _pEdit;
	EditableObject* _pObj;

	DECLARE_COUNTER;
};


#else
	#error File already included
#endif
