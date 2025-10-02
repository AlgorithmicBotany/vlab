#ifndef __OBJFGVEDIT_H__
#define __OBJFGVEDIT_H__


class ObjectGallery;
class ObjectView;


class ObjectEdit
{
public:
	ObjectEdit()
	{ 
		_pGallery = 0;
		_pView = 0;
		INC_COUNTER;
	}
	virtual ~ObjectEdit() { DEC_COUNTER; }
	virtual bool SelectionChanged(int, int);
	virtual void ApplyNow();
	virtual void Retrieve();
	virtual void AddAsNew();
	virtual void Generate() const = 0;

	virtual void Clear();
	int Items() const;
	void Select(int);
	void Delete();
protected:
	virtual void _UpdateControls() = 0;
	virtual void _UpdateFromControls() {}
	virtual void _UpdateView() = 0;

	ObjectGallery* _pGallery;
	ObjectView* _pView;

	DECLARE_COUNTER;
};




#else
	#error File already included
#endif
