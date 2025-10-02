#ifndef __EDLIN_H__
#define __EDLIN_H__


class EdLin
{
public:
	EdLin(const char*);
	~EdLin();
	int Find(int, char) const;
	int SkipBlanks(int) const;
	void DeleteChars(int, const char*);
	void DeleteChar(int);
	void Insert(int, const char*);
	operator const char*() const
	{ return _bf; }
private:
	enum { eDefSize = PanelParameters::eMaxLineLength+1 };
	char _bf[eDefSize];
	int _size;
	int _len;
	void _Grow(int);
};

#else
	#error File already included
#endif
