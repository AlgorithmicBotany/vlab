#ifndef __GEOMETRYCONSTRAIN_H__
#define __GEOMETRYCONSTRAIN_H__


class GeometryConstrain
{
public:
	GeometryConstrain(Window w) : _w(w), _set(0) 
	{ assert(_w.IsSet()); }
	void SetLeft(const RECT&);
	void SetWidth();
	void SetTop(const RECT&);
	void SetHeight();
	void SetRight(const RECT&);
	void SetBottom(const RECT&);
	void SetMinWidth();
	void SetMinHeight();
	void SetMinTop(const RECT&);
	void SetMinLeft(const RECT&);
	void SetPropWidth(const RECT&);
	void SetMaxLeft(const RECT&);
	void SetPropLeft(const RECT&);
	void Adjust(int, int);
private:
	const Window _w;
	enum eAction
	{
		aLeft,
			aWidth,
			aTop,
			aHeight,
			aRight,
			aBottom,
			aMinWidth,
			aMinHeight,
			aMinTop,
			aMinLeft,
			aPropWidth,
			aMaxLeft,
			aPropLeft,
			aLast
	};	
	struct ConstrainInfo
	{
		eAction Action;
		int Value;
	};
	enum
	{ nSize = aLast };
	int _set;
	ConstrainInfo _arr[nSize];
};



#endif
