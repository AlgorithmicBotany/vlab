#ifndef __RESOURCE_H__
#define __RESOURCE_H__


class Resource
{
public:
	Resource(UINT id, const char* type)
	{
		_hRes = FindResource
			(
			App::GetInstance(),
			MAKEINTRESOURCE(id), type
			);
		if (0 == _hRes)
			throw Exception("Cannot find resource type %s", type);
		_hGlb = LoadResource(App::GetInstance(), _hRes);
		if (0 == _hGlb)
			throw Exception("Cannot load resource type %s", type);
		_size = SizeofResource(App::GetInstance(), _hRes);
	}
	~Resource()
	{
		FreeResource(_hGlb);
	}

	void* Data() const
	{ return LockResource(_hGlb); }
	DWORD Size() const
	{ return _size; }
private:
	HRSRC _hRes;
	HGLOBAL _hGlb;
	DWORD _size;
};


class TextResource : public Resource
{
public:
	TextResource(UINT id, const char* type) : Resource(id, type), _str(Size()+1, '\0')
	{
		_str = reinterpret_cast<const char*>(Data());
		_str.resize(Size());
	}
	const char* Text() const
	{ return _str.c_str(); }
private:
	std::string _str;
};


#endif
