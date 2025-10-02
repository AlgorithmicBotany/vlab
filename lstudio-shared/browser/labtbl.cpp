#include <sstream>


#include <fw.h>

#include "labtbl.h"


VLB::LabTable::LabTable()
{
	TempPath Tmp;
	BOOL res = FALSE;
	int d = 0;
	while (0==res)
	{
		if (1000==d)
			throw Exception("Cannot create lab table");
		std::stringstream lbtblnm;
		lbtblnm << Tmp << "lbtbl" << d;
		res = ::CreateDirectory(lbtblnm.str().c_str(), 0);
		if (res)
			_path = lbtblnm.str();
		else
			++d;
	}
	CurrentDirectory cd;
	_PDir = cd;
	::SetCurrentDirectory(_path.c_str());
}


VLB::LabTable::~LabTable()
{
	Clean();
	::SetCurrentDirectory(_PDir.c_str());
	RemoveDirectory(_path.c_str());
}


void VLB::LabTable::Clean()
{
	TmpChangeDir tcd(_path);
	FindFile ff("*.*");
	while (ff.Found())
	{
		if (!ff.IsDirectory())
			DeleteFile(ff.FileName().c_str());
		ff.FindNext();
	}
}
