/**************************************************************************

  File:		registry.h
  Created:	25-Nov-97


  Declaration of classes Registry


**************************************************************************/


#ifndef __REGISTRY_H__
#define __REGISTRY_H__


class LongString;

class RegKey
{
public:
	enum Security
	{
		sAllAccess = KEY_ALL_ACCESS,
		sQueryValue = KEY_QUERY_VALUE,
		sWrite = KEY_WRITE,
		sRead = KEY_READ,
		sSetValue = KEY_SET_VALUE
	};

	RegKey(HKEY, const std::string&, Security);
	~RegKey();
	HKEY GetKey() const
	{ return _hKey; }
	HKEY Release() 
	{
		HKEY toret = _hKey;
		_hKey = 0;
		return toret;
	}
	void SetIgnore(bool ignore)
	{ _IgnoreErrors = ignore; }
	void LoadBinary(const std::string&, void*, int) const;
	void StoreBinary(const std::string&, const void*, int);
	bool LoadString(const std::string&, std::string&) const;
	void StoreString(const std::string&, const std::string&);
private:
	// Whether to ignore I/O errors
	bool _IgnoreErrors;
	HKEY _hKey;
};




#endif
