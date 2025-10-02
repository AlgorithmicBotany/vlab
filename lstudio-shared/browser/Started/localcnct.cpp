#include <string>
#include <vector>
#include <fstream>

#include <fw.h>

#include "connection.h"
#include "localcnct.h"

VLB::LocalConnection::LocalConnection() : _host("[local]")
{}


VLB::LocalConnection::~LocalConnection()
{}



bool VLB::LocalConnection::PutFile(const std::string& rempath, const std::string& name, const std::string& remnm)
{
	std::string trg(rempath);
	assert(_ValidFname(trg));
	trg.append("\\");
	trg.append(remnm);
	assert(_ValidFname(trg));
	BOOL res = CopyFile(name.c_str(), trg.c_str(), FALSE);
	return (0!=res);
}

bool VLB::LocalConnection::DeleteFile(const std::string& directory, const std::string& name)
{
	std::string trg(directory);
	assert(_ValidFname(trg));
	trg.append("\\");
	trg.append(name);
	assert(_ValidFname(trg));
	BOOL res = ::DeleteFile(trg.c_str());
	return (0 != res);
}

bool VLB::LocalConnection::_ValidFname(const std::string& nm) const
{
	if (nm.empty())
		return false;
	size_t pos = nm.find('/');
	if (pos != std::string::npos)
		return false;
	pos = nm.length()-1;
	if ('\\' == nm[pos])
		return false;
	return true;
}

bool VLB::LocalConnection::FetchObject(const std::string& path)
{
	TmpChangeDir tcd(path);
	for (FindFile ff("*.*"); ff.Found(); ff.FindNext())
	{
		if (!ff.IsDirectory())
		{
			std::string trg(tcd.OrigDir());
			trg.append("\\");
			trg.append(ff.FileName());
			BOOL res = CopyFile(ff.FileName(), trg.c_str(), FALSE);
			if (0==res)
			{
				DWORD err = GetLastError();
				++err;
				return false;
			}
			{
				DWORD attr = GetFileAttributes(trg.c_str());
				attr &= ~FILE_ATTRIBUTE_READONLY;
				SetFileAttributes(trg.c_str(), attr);
			}
		}
	}
	return true;
}

bool VLB::LocalConnection::PutObject(const string_buffer& sb)
{
	string_buffer::const_iterator it(sb);
	const char* trgdir = it.str();
	it.advance();
	while (!it.at_end())
	{
		std::string trg(trgdir);
		trg.append(it.str());
		BOOL res = CopyFile(it.str(), trg.c_str(), FALSE);
		if (!res)
			return false;
	}
	return true;
}

bool VLB::LocalConnection::HasExtensions(const std::string& path)
{
	assert(_ValidFname(path));
	std::string pth(path);
	pth += "\\*.*";

	for (FindFile ff(pth); ff.Found(); ff.FindNext())
	{
		if (ff.IsDirectory() && ff.FileName()[0] != '.')
			return true;
	}
	return false;
}


bool VLB::LocalConnection::GetFileList(string_buffer& list, const std::string& path, bool filesonly)
{
	assert(_ValidFname(path));
	std::string pth(path);
	pth += "\\*.*";
	for (FindFile ff(pth); ff.Found(); ff.FindNext())
	{
		if (!(filesonly && ff.IsDirectory()))
			list.add(ff.FileName());
	}
	return true;
}

bool VLB::LocalConnection::GetExtensions(string_buffer& nms, string_buffer& bf, std::vector<int>& tp, const std::string& path)
{
	assert(_ValidFname(path));
	std::string pth(path);
	pth += "\\*.*";
	for (FindFile ff(pth); ff.Found(); ff.FindNext())
	{
		if (ff.IsDirectory() && ff.FileName()[0] != '.')
		{
			std::string objpth(path);
			objpth += '\\';
			objpth += ff.FileName();
			nms.add(ff.FileName());
			bf.add(objpth.c_str());
			if (HasExtensions(objpth.c_str()))
				tp.push_back(2);
			else
				tp.push_back(0);
		}
	}
	return true;
}



bool VLB::LocalConnection::GetFile(const std::string& fname)
{
	assert(_ValidFname(fname));
	size_t pos = fname.find_last_of('\\');
	if (std::string::npos == pos)
		return false;
	++pos;
	std::string trg(fname.substr(pos));
	CopyFile(fname.c_str(), trg.c_str(), FALSE);
	{
		DWORD attr = GetFileAttributes(trg.c_str());
		attr &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes(trg.c_str(), attr);
	}
	return true;
}


bool VLB::LocalConnection::GetFile(const std::string& src, const std::string& trg)
{
	assert(_ValidFname(src));
	CopyFile(src.c_str(), trg.c_str(), FALSE);
	{
		DWORD attr = GetFileAttributes(trg.c_str());
		attr &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes(trg.c_str(), attr);
	}
	return true;
}


bool VLB::LocalConnection::CompareFiles(const std::string& lclfname, const std::string& rempath)
{
	std::string orgfile(rempath);
	orgfile += '\\';
	orgfile += lclfname;

	std::ifstream lcf(lclfname.c_str(), std::ios::binary);
	std::ifstream rmf(orgfile.c_str(), std::ios::binary);

	if (!lcf.is_open())
		return false;
	if (!rmf.is_open())
		return false;
	for (;;)
	{
		char c1;
		lcf.read(&c1, sizeof(char));
		char c2;
		rmf.read(&c2, sizeof(char));
		if (lcf.eof() && rmf.eof())
			return true;
		if (lcf.eof() || rmf.eof())
			return false;
		if (c1 != c2)
			return false;
	}
	//assert(0);
	//return false;
}


bool VLB::LocalConnection::PutFile(const std::string& src, const std::string& trg)
{
	assert(_ValidFname(src));
	assert(_ValidFname(trg));
	CopyFile(src.c_str(), trg.c_str(), FALSE);
	return true;
}

bool VLB::LocalConnection::MakeExt(const std::string& location, const std::string& name, std::string& dirname)
{
	assert(_ValidFname(location));
	dirname = location;
	dirname += '\\';
	dirname += name;
	return (0!=CreateDirectory(dirname.c_str(), 0));
}

bool VLB::LocalConnection::DelTree(const std::string& dir)
{
	std::string cdir(dir);
	cdir.append("\\*.*");
	for (FindFile ff(cdir); ff.Found(); ff.FindNext())
	{
		std::string fl(dir);
		fl.append("\\");
		fl.append(ff.FileName());
		if (ff.IsSubDirectory())
		{
			if (!DelTree(fl.c_str()))
				return false;
		}
		else if (!ff.IsDirectory())
		{
			if (0==::DeleteFile(fl.c_str()))
				return false;
		}
	}
	return ( 0!=RemoveDirectory(dir.c_str()) );
}


bool VLB::LocalConnection::PrototypeObject(const std::string&)
{
	return true;
}


bool VLB::LocalConnection::DirCopy(const std::string& from, const std::string& to, bool recursive) const
{
	std::string src(from);
	std::string trg(to);
	size_t bs = src.find_last_of('\\')+1;
	trg.append("\\");
	trg.append(src.substr(bs));
	if (0==CreateDirectory(trg.c_str(), 0))
		return false;
	src.append("\\*.*");
	for (FindFile ff(src); ff.Found(); ff.FindNext())
	{
		std::string fn(from);
		fn.append("\\");
		fn.append(ff.FileName());
		if (ff.IsSubDirectory() && recursive)
		{
			if (!DirCopy(fn.c_str(), trg.c_str(), true))
				return false;
		}
		else if (!ff.IsDirectory())
		{
			std::string ltrg(trg);
			ltrg.append("\\");
			ltrg.append(ff.FileName());
			if (0==::CopyFile(fn.c_str(), ltrg.c_str(), FALSE))
				return false;
		}
	}
	return true;
}


bool VLB::LocalConnection::Paste(const std::string& /* oofs */, const std::string& from, const std::string& to, bool recursive)
{
	assert(_ValidFname(from));
	assert(_ValidFname(to));
	std::string src(from);
	std::string trg(to);
	if (trg.substr(0, src.length()) == src)
	{
		// copying on itself
		// first copy to temp
		char tmppth[MAX_PATH+1];
		GetTempPath(MAX_PATH+1, tmppth);
		if (!DirCopy(from, tmppth, recursive))
			return false;
		// then from temp to trg
		src = tmppth;
		size_t pos = from.find_last_of('\\')+1;
		src.append(from.substr(pos));
		bool res = DirCopy(src, to, recursive);
		// Remove the temp copy
		DelTree(src);
		return res;
	}
	else
	{
		return DirCopy(from, to, recursive);
	}
}

bool VLB::LocalConnection::GetTmpFile(const std::string& rempath, const std::string& fname, std::string& lclpthnm)
{
	assert(_ValidFname(rempath));
	std::string src(rempath);
	src += "\\";
	src += fname;
	char trg[MAX_PATH+1];
	GetTempPath(MAX_PATH, trg);
	UINT res = GetTempFileName(trg, "gtf", 0, trg);
	if (0==res)
		return false;

	BOOL b = ::CopyFile(src.c_str(), trg, FALSE);
	{
		DWORD attr = GetFileAttributes(trg);
		attr &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes(trg, attr);
	}

	if (!b)
		return false;
	lclpthnm = trg;
	return true;
}


bool VLB::LocalConnection::RenameObj(const std::string&, std::string& path, const std::string& name)
{
	std::string newnm(path);
	std::string::size_type pos = newnm.find_last_of('\\')+1;
	newnm.resize(pos);
	newnm.append(name);

	if (0==MoveFile(path.c_str(), newnm.c_str()))
		return false;
	else
	{
		path = newnm;
		return true;
	}
}
