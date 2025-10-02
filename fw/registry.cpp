/**************************************************************************

  File:		registry.cpp
  Created:	25-Nov-97


  Implementation of classes Registry


**************************************************************************/


#include <string>

#include <cassert>

#include <windows.h>
#include <tchar.h>

#include "warningset.h"

#include "registry.h"
#include "exception.h"
#include "lstring.h"

#include "libstrng.h"



RegKey::RegKey(HKEY hKey, const std::string& subname, Security security)
{
	assert(0 != hKey);
	DWORD disp;
	LONG res = RegCreateKeyEx(
		hKey,
		subname.c_str(),
		0,
		0,	// lpClas ????
		0,		// dwOptions
		security,
		0,
		&_hKey,
		&disp);

	if (ERROR_SUCCESS != res)
		throw Exception(0, FWStr::ManipulatingKey);
	_IgnoreErrors = false;
}


RegKey::~RegKey()
{
	if (0 != _hKey)
		RegCloseKey(_hKey);
}


void RegKey::LoadBinary(const std::string& name, void* bf, int size) const
{
	assert(0 != _hKey);
	DWORD type = REG_BINARY;
	DWORD sz = size;
	LONG res = RegQueryValueEx(
		_hKey,
		name.c_str(), 
		0,
		&type,
		(BYTE*) bf, 
		&sz);
	if (!_IgnoreErrors)
	{
		if (ERROR_SUCCESS != res)
			throw Exception(0, FWStr::ManipulatingKey);
	}
}

void RegKey::StoreBinary(const std::string& name, const void* bf, int size)
{
	assert(0 != _hKey);
	LONG res = RegSetValueEx(
		_hKey, 
		name.c_str(),
		0,
		REG_BINARY,
		(const BYTE*) bf,
		size);
	if (!_IgnoreErrors)
	{
		if (ERROR_SUCCESS != res)
			throw Exception(0, FWStr::ManipulatingKey);
	}
}



bool RegKey::LoadString(const std::string& name, std::string& str) const
{
	assert(0 != _hKey);
	DWORD type = REG_SZ;
	DWORD sz = 0;
	LONG res = RegQueryValueEx
		(
		_hKey,
		name.c_str(),
		0, 
		&type,
		0,
		&sz
		);
	if (res != ERROR_SUCCESS)
	{
		if (_IgnoreErrors)
			return false;
		else
			throw Exception(0, FWStr::ManipulatingKey);
	}
	str.reserve(sz+1);
	str.resize(sz);
	res = RegQueryValueEx
		(
		_hKey,
		name.c_str(),
		0, &type,
		reinterpret_cast<BYTE*>(&(str[0])),
		&sz
		);
	if (res != ERROR_SUCCESS)
	{
		if (_IgnoreErrors)
			return false;
		else
			throw Exception(0, FWStr::ManipulatingKey);
	}
	str.resize(sz-1);
	return true;
}


void RegKey::StoreString(const std::string& name, const std::string& val)
{
	assert(0 != _hKey);
	int size = val.length()+1;
	LONG res = RegSetValueEx
		(
		_hKey, name.c_str(), 
		0, REG_SZ,
		reinterpret_cast<const BYTE*>(val.c_str()), size
		);
	if (!_IgnoreErrors)
	{
		if (ERROR_SUCCESS != res)
			throw Exception(0, FWStr::ManipulatingKey);
	}
}
