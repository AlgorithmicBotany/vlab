/**************************************************************************

  File:		stack.h
  Created:	22-Dec-97


  Template class Stack


**************************************************************************/


#ifndef __STACK_H__
#define __STACK_H__


template <class T>
class Stack
{
public:
	Stack()
	{
		_size = 8;
		_top = 0;
		_arr = new T[_size];
	}
	~Stack()
	{
		delete []_arr;
	}
	void Push(T& v)
	{
		if (_top == _size)
			_Grow();
		_arr[_top++] = v;
	}
	bool IsEmpty() const
	{ return 0==_top; }
	T& Pop()
	{
		assert(!IsEmpty());
		if (_top < _size/4)
			_Shrink();
		return _arr[_top--];
	}
	void Clear()
	{
		delete []_arr;
		_size = 8;
		_top = 0;
		_arr = new T[_size];
	}

protected:
	int _size;
	int _top;
	T* _arr;
	void _Grow()
	{
		T* aTmp = new T[_size*2];
		_size *= 2;
		for (int i=0; i<_top; i++)
			aTmp[i] = _arr[i];

		delete []_arr;
		_arr = aTmp;
	}
	void _Shrink()
	{
		assert(_top < _size/2);
		T* aTmp = new T[_size/2];
		_size /= 2;
		for (int i=0; i<_top; i++)
			aTmp[i] = _arr[i];
		delete []_arr;
		_arr = aTmp;
	}
};





#else
	#error File already included
#endif
