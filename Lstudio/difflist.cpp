#include <vector>

#include <fw.h>

#include <browser/remaccess.h>

#include "difflist.h"

void DiffList::AddSrc1(const TCHAR* nm)
{
	Entry e;
	_tcscpy(e.name, nm);
	e.status = stAdded;
	_aEntry.push_back(e);
}

void DiffList::AddSrc2(const TCHAR* nm)
{
	iter i=_Find(nm);
	if (_aEntry.end() == i)
	{
		Entry e;
		_tcscpy(e.name, nm);
		e.status = stDeleted;
		_aEntry.push_back(e);
	}
	else
	{
		i->status = stIdentical;
	}
}


void DiffList::Compare(const TCHAR* dir)
{
	for (iter i = _aEntry.begin(); i != _aEntry.end(); ++i)
	{
		if (stIdentical == i->status)
		{
			if (!(_Compare(i->name, dir)))
				i->status = stModified;
		}
	}
}


void DiffList::Compare(const VLB::RemoteAccess* pRA, const char* lcldir, const char* remdir)
{
	TmpChangeDir tcd(lcldir);
	for (iter i = _aEntry.begin(); i != _aEntry.end(); ++i)
	{
		if (stIdentical == i->status)
		{
			if (!pRA->CompareFiles(i->name, remdir))
				i->status = stModified;
		}
	}
}

bool DiffList::Identical() const
{
	for (citer i = _aEntry.begin(); i != _aEntry.end(); ++i)
	{
		if (stIdentical != i->status)
			return false;
	}
	return true;
}

DiffList::iter DiffList::_Find(const TCHAR* nm)
{
	for (iter i = _aEntry.begin(); i != _aEntry.end(); ++i)
	{
		if (!(_tcscmp(nm, i->name)))
			return i;
	}
	return _aEntry.end();
}

bool DiffList::_Compare(const TCHAR* f1, const TCHAR* dir) const
{
	TCHAR f2[MAX_PATH+1];
	_tcscpy(f2, dir);
	_tcscat(f2, __TEXT("\\"));
	_tcscat(f2, f1);

	try
	{
		ReadBinFile F1(f1);
		ReadBinFile F2(f2);
		long sz1 = F1.Size();
		long sz2 = F2.Size();
		if (sz1 != sz2)
			return false;
		for (long p = 0; p<sz1; p++)
		{
			unsigned char b1, b2;
			F1.Read(&b1, sizeof(char));
			F2.Read(&b2, sizeof(char));
			if (b1 != b2)
				return false;
		}
	}
	catch (Exception)
	{
		return false;
	}

	return true;
}

