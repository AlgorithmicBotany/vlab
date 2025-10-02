#ifndef __AUTOVECTOR_H__
#define __AUTOVECTOR_H__

template <typename T>
class auto_vector
{
public:
	auto_vector()
	{
		_arr = new std::unique_ptr<T> [eDefSize];
		_capacity = eDefSize;
		_end = 0;
	}
	auto_vector(size_t cap)
	{
		int startSize = eDefSize;
		while(startSize < cap) startSize *= 2;
		_arr = new std::unique_ptr<T> [startSize];
		_capacity = startSize;
		_end = cap;
	}
	~auto_vector()
	{ free(); delete []_arr; }
	void free()
	{
		for (size_t n=0; n<_end; ++n)
			delete _arr[n].release();
		_end = 0;
	}
	void push_back(std::unique_ptr<T>& p)
	{
		assert(_end<=_capacity);
		if (_end == _capacity)
			grow(_end+1);
        _arr[_end] = std::move(p);
		_end++;
	}
	class const_iterator
	{
	public:
		const_iterator(const std::unique_ptr<T>* p) : _p(p) {}
		const T* operator->() { return _p->get(); }
		const T* operator*() { return _p->get(); }
		const_iterator operator++() { return ++_p; }
		const_iterator operator--() { return --_p; }
		bool operator != (const const_iterator& it) const
		{ return _p != it._p; }
		bool operator == (const T* p) const
		{ return (_p->get() == p); }
		const std::unique_ptr<T>* ptr() const
		{ return _p; }
	private:
		const std::unique_ptr<T>* _p;
	};
	class iterator
	{
	public:
		iterator(const std::unique_ptr<T>* p) : _p(p) {}
		T* operator->() { return _p->get(); }
		T* operator*() { return _p->get(); }
		iterator operator++() { return ++_p; }
		bool operator != (const iterator& it) const
		{ return _p != it._p; }
		bool operator != (const const_iterator& it) const
		{ return _p != it.get(); }
		const std::unique_ptr<T>* ptr() const
		{ return _p; }
		bool operator == (const T* p) const
		{ return (_p->get() == p); }
	private:
		const std::unique_ptr<T>* _p;
	};

	const T* at(int idx) const
	{ return _arr[idx].get(); }
	T* at(int idx)
	{ return _arr[idx].get(); }

	const T* front() const
	{ return _arr[0].get(); }
	T* front() 
	{ return _arr[0].get(); }
	const const_iterator begin() const
	{ return _arr; }
	const iterator begin() 
	{ return _arr; }
	const const_iterator end() const
	{ return _arr+_end; }
	const iterator end() 
	{ return _arr+_end; }
	void erase(iterator i)
	{ 
		size_t n = i.ptr()-_arr;
		delete _arr[n].release();
		for (size_t j=n; j<_end-1; ++j)
			_arr[j] = std::move(_arr[j+1]);
		--_end;
	}
	size_t size() const
	{ return _end; }
	bool empty() const
	{ return (0==size()); }
	void sort(int (*cmp)(const void*, const void*))
	{
		qsort(_arr, _end, sizeof(std::unique_ptr<T>), cmp);
	}
	void swap(size_t i,size_t j)
	{
		// Use std::move to transfer ownership of unique_ptrs
		static std::unique_ptr<T> tmpSpace;
		tmpSpace = std::move(_arr[i]);
		_arr[i] = std::move(_arr[j]);
		_arr[j] = std::move(tmpSpace);
	}
private:
	void grow(size_t req)
	{
		size_t newc = 2*_capacity;
		while (req>newc)
			newc *= 2;
		std::unique_ptr<T>* aNew = new std::unique_ptr<T> [newc];
		for (size_t i=0; i<_end; ++i)
            aNew[i] = std::move(_arr[i]);
		delete []_arr;
		_capacity = newc;
		_arr = aNew;
	}
	enum
	{ eDefSize = 32 };
	std::unique_ptr<T>* _arr;
	size_t _capacity;
	size_t _end;
};

#else
	#error File already included
#endif
