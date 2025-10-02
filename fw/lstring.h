/**************************************************************************

  File:		lstring.h
  Created:	25-Nov-97


  Declaration of class LongString


**************************************************************************/


#ifndef __LONGSTRING_H__
#define __LONGSTRING_H__


class WriteTextFile;

class LongString
{
	friend class Window;
public:
	explicit LongString(int l=128);
	explicit LongString(const TCHAR*);
	LongString(int, UINT);
	LongString(int, UINT, HINSTANCE);
	~LongString();
	void SetSize(int); // Clears contents !!!
	operator const TCHAR*() const
	{ return _arr; }
	void operator=(const TCHAR*);
	void operator=(const LongString&);
	void operator+=(const TCHAR*);
	void operator+=(TCHAR);
	bool IsEmpty() const
	{ return 0==Length(); }
	int Length() const
	{ return _len; }
	TCHAR* Buf() 
	{ return _arr; }
	const TCHAR* Str() const
	{ return _arr; }
	int Size() const
	{ return _size; }
	void Write(WriteTextFile&) const;
	void Load(UINT);
	void ExchangeBuffers(LongString&);
	void AllTrim();
	void CutLast(int);
	void GrowTo(int);
	TCHAR operator[](int i) const
	{ 
		assert(i>=0);
		assert(i<=_len);
		return _arr[i];
	}
	int CalcLength();

	bool Format(const TCHAR*, ...);
	void ToUpper();
	void Pack();
	const TCHAR* FindLast(TCHAR ch) const
	{ return _tcsrchr(_arr, ch); }
	TCHAR Last() const
	{ 
		assert(!IsEmpty());
		return _arr[Length()-1]; 
	}
protected:
	void _Grow(int);
	int _size;
	int _len;
	TCHAR* _arr;
};



#endif
