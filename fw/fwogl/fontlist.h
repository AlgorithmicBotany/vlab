#ifndef __FONTLIST_H__
#define __FONTLIST_H__


class FontList
{
public:
	void Init(HDC);
	unsigned int Base() const
	{ return _lists.Base(); }
private:
	enum
	{ eFontRange = 128 };
	DisplayLists _lists;
};


#endif
