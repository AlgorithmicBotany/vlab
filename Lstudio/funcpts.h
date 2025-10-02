#ifndef __FUNCTIONPOINTS_H__
#define __FUNCTIONPOINTS_H__



class FunctionPoints : public std::vector<WorldPointf>
{
public:
	void SetPoint(size_t i, WorldPointf wp)
	{
		assert(i<size());
		at(i) = wp;
	}
	void Insert(size_t, WorldPointf);
	bool CanDelete(size_t) const;
	void Delete(size_t);
	WorldPointf LastPoint() const
	{ 
		assert(size()>0);
		return at(size()-1); 
	}
	void SetSize(size_t sz)
	{
		reserve(sz);
		clear();
	}
	bool operator!=(const FunctionPoints&) const;
protected:
};


#else
	#error File already included
#endif
