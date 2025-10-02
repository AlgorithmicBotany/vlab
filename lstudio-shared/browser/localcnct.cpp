#include <string>
#include <vector>
#include <fstream>

#include <fw.h>

#include "connection.h"
#include "localcnct.h"



const char DotId[] = ".id";
const char DotNode[] = "node";
const char DotOrdering[] = ".ordering";
const char DotUUids[] = ".uuids";



VLB::LocalConnection::LocalConnection()
{}


VLB::LocalConnection::~LocalConnection()
{}



bool VLB::LocalConnection::PutFile(const char* rempath, const char* name, const char* remnm)
{
	std::string trg(rempath);
	assert(ValidFname(trg.c_str()));
	trg.append("\\");
	trg.append(remnm);
	assert(ValidFname(trg.c_str()));
	BOOL res = CopyFile(name, trg.c_str(), FALSE);
	return (0!=res);
}

bool VLB::LocalConnection::DeleteFile(const char* directory, const char* name)
{
	std::string trg(directory);
	assert(ValidFname(trg.c_str()));
	trg.append("\\");
	trg.append(name);
	assert(ValidFname(trg.c_str()));
	BOOL res = ::DeleteFile(trg.c_str());
	return (0 != res);
}

bool VLB::LocalConnection::ValidFname(const std::string& nm)
{
	if (std::string::npos != nm.find_first_of('/'))
		return false;

	std::string::const_iterator it = nm.end();
	--it;

	if ('\\' == *it)
		return false;
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
	assert(ValidFname(path));
	std::string pth(path);
	pth += "\\*.*";

	FindFile ff(pth);
	while (ff.Found())
	{
		if (ff.IsDirectory() && ff.FileName()[0] != '.')
			return true;
		ff.FindNext();
	}
	return false;
}

bool VLB::LocalConnection::GetParentPath(const std::string& path,
					 std::string& parent)
{
  size_t pos = path.find_last_of(PathSeparator());
  if(pos == std::string::npos)
    return false;

  parent = path.substr(0,pos);
  return true;
}

bool VLB::LocalConnection::GetFileList(string_buffer& list, const char* path, bool filesonly)
{
	assert(ValidFname(path));
	std::string pth(path);
	pth += "\\*.*";
	FindFile ff(pth);
	while (ff.Found())
	{
		if (!(filesonly && ff.IsDirectory()))
			list.add(ff.FileName().c_str());
		ff.FindNext();
	}
	return true;
}

bool VLB::LocalConnection::GetExtensions(string_buffer& nms, string_buffer& bf, std::vector<int>& tp, const char* path)
{
	assert(ValidFname(path));
	std::string pth(path);
	pth += "\\*.*";
	FindFile ff(pth);
	while (ff.Found())
	{
		if (ff.IsDirectory() && ff.FileName()[0] != '.')
		{
			std::string objpth(path);
			objpth += '\\';
			objpth += ff.FileName();
			nms.add(ff.FileName().c_str());
			bf.add(objpth.c_str());
			if (HasExtensions(objpth))
				tp.push_back(2);
			else
				tp.push_back(0);
		}
		ff.FindNext();
	}
	return true;
}



bool VLB::LocalConnection::GetFile(const char* fname)
{
	assert(ValidFname(fname));
	const char* fnm = strrchr(fname, '\\');
	if (0 == fnm)
		return false;
	++fnm;
	CopyFile(fname, fnm, FALSE);
	{
		DWORD attr = GetFileAttributes(fnm);
		attr &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes(fnm, attr);
	}
	return true;
}


bool VLB::LocalConnection::GetFile(const char* src, const char* trg)
{
	assert(ValidFname(src));
	CopyFile(src, trg, FALSE);
	{
		DWORD attr = GetFileAttributes(trg);
		attr &= ~FILE_ATTRIBUTE_READONLY;
		SetFileAttributes(trg, attr);
	}
	return true;
}


bool VLB::LocalConnection::CompareFiles(const char* lclfname, const char* rempath)
{
	std::string orgfile(rempath);
	orgfile += '\\';
	orgfile += lclfname;

	std::ifstream lcf(lclfname, std::ios::in | std::ios::binary);
	std::ifstream rmf(orgfile.c_str(), std::ios::in | std::ios::binary);

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
}


bool VLB::LocalConnection::PutFile(const char* src, const char* trg)
{
	assert(ValidFname(src));
	assert(ValidFname(trg));
	CopyFile(src, trg, FALSE);
	return true;
}

bool VLB::LocalConnection::MakeExt(const char* location, const char* name, std::string& dirname)
{
	assert(ValidFname(location));
	dirname = location;
	dirname += '\\';
	dirname += name;
	return (0!=CreateDirectory(dirname.c_str(), 0));
}

bool VLB::LocalConnection::DelTree(const char* dir)
{
	std::string cdir(dir);
	cdir.append("\\*.*");
	FindFile ff(cdir);
	while (ff.Found())
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
		ff.FindNext();
	}
	return ( 0!=RemoveDirectory(dir) );
}


bool VLB::LocalConnection::PrototypeObject(const char*)
{
	return true;
}


bool VLB::LocalConnection::DirCopy(const char* from, const char* to, bool recursive) const
{
	std::string src(from);
	std::string trg(to);
	size_t bs = src.find_last_of('\\')+1;
	trg.append("\\");
	trg.append(src.substr(bs));
	if (0==CreateDirectory(trg.c_str(), 0))
		return false;
	src.append("\\*.*");
	FindFile ff(src);
	while (ff.Found())
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
			if (0==CopyFile(fn.c_str(), ltrg.c_str(), FALSE))
				return false;
		}
		ff.FindNext();
	}
	return true;
}


bool VLB::LocalConnection::Paste(const char* /* oofs */, const char* from, const char* to, bool recursive)
{
	assert(ValidFname(from));
	assert(ValidFname(to));
	std::string src(from);
	std::string trg(to);
	if (trg.substr(0, src.length()) == src)
	{
		// copying on itself
		// first copy to temp
		TempPath tp;
		if (!DirCopy(from, tp.c_str(), recursive))
			return false;
		// then from temp to trg
		src = tp;
		const char* name = strrchr(from, '\\');
		src.append(name);
		bool res = DirCopy(src.c_str(), to, recursive);
		// Remove the temp copy
		DelTree(src.c_str());
		return res;
	}
	else
	{
		return DirCopy(from, to, recursive);
	}
}

bool VLB::LocalConnection::GetTmpFile(const char* rempath, const char* fname, std::string& lclpthnm)
{
	assert(ValidFname(rempath));
	std::string src(rempath);
	src += "\\";
	src += fname;
	std::string trg;
	try
	{
		TempFileName tfn("gtf");
		trg = tfn;
	}
	catch (...)
	{
		return false;
	}
	BOOL b = ::CopyFile(src.c_str(), trg.c_str(), FALSE);
	if (!b)
		return false;

	{
		DWORD attr = ::GetFileAttributes(trg.c_str());
		attr &= ~FILE_ATTRIBUTE_READONLY;
		::SetFileAttributes(trg.c_str(), attr);
	}

	lclpthnm = trg;
	return true;
}


bool VLB::LocalConnection::RenameObj(const std::string&, std::string& path, const char* name)
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


bool VLB::LocalConnection::GetOrdering(const char* szParentPath, std::vector<std::string>& order)
{
	std::string filename;
	BuildOrderingFilename(szParentPath, filename);

	std::ifstream orderFile(filename);
	if (!orderFile.is_open())
	{
		order.clear();
		return false;
	}

	while (!orderFile.eof())
	{
		std::string name;
		std::getline(orderFile, name);
		if (name.empty())
			continue;
		order.push_back(name);
	}

	return true;
}


bool VLB::LocalConnection::WriteOrdering(const char* szParentPath, const std::vector<std::string>& order) 
{
	std::string filename;
	BuildOrderingFilename(szParentPath, filename);

	std::ofstream orderFile(filename);
	if (!orderFile.is_open())
	{
		return false;
	}

	for (size_t item=0; item<order.size(); ++item)
	{
		orderFile << order[item] << std::endl;
	}

	return true;
}

void VLB::LocalConnection::BuildOrderingFilename(const char* szParentPath, std::string& result) const
{
	result = szParentPath;
	result.append(1, PathSeparator());
	result.append(DotOrdering);
}


bool VLB::LocalConnection::MakeHyperobject(const char* szPath, const char* szName, const tGUID& uuid)
{
	TmpFile nodeFile(false);
	std::ofstream nodeData(nodeFile.Filename());

	std::string line;
	uuid.ToString(line);
	nodeData << line << std::endl << szName << std::endl;

	return PutFile(szPath, nodeFile.Filename().c_str(), DotNode);
}


void VLB::LocalConnection::LoadUUIDBase(const std::string& oofsroot)
{
	std::string dbfile;
	BuildUUIDBaseFilename(oofsroot, dbfile);
	_lastUuidsOofs = oofsroot;
	_uuidsBase.Load(dbfile);
}


void VLB::LocalConnection::ReloadUUIDBase()
{
	if (!_lastUuidsOofs.empty())
	{
		LoadUUIDBase(_lastUuidsOofs);
	}
}

void VLB::LocalConnection::BuildUUIDBaseFilename(const std::string& oofsroot, std::string& result) const
{
	result = oofsroot;
	result += PathSeparator();
	result += DotUUids;
}

bool VLB::LocalConnection::IsHyperobject(const std::string& path)
{
	std::string filename;
	BuildNodeFilename(path, filename);

	FindFile ff(filename);
	return ff.Found();
}

bool VLB::LocalConnection::LookupPath(const tGUID& id, const char* oofsroot, std::string& path)
{
	std::string nodePath;
	if (_uuidsBase.LookupPath(id, nodePath))
	{
		path = oofsroot;
		path.append(nodePath);
		return true;
	}
	else
	{
		return false;
	}
}


bool VLB::LocalConnection::GetUUID(const char* szPath, tGUID& id, std::string& name)
{
	std::string filename;
	BuildNodeFilename(szPath, filename);
	std::ifstream input(filename);
	if (!input.is_open())
	{
		return false;
	}
	std::string line;
	std::getline(input, line);
	id.FromString(line);
	std::getline(input, name);
	return true;
}


bool VLB::LocalConnection::GetUUID(const char* szOofsroot, const char* szPath, tGUID& id)
{
	std::string filename;
	BuildUUIDFilename(szPath, filename);
	std::ifstream input(filename);
	if (input.is_open())
	{
		std::string line;
		std::getline(input, line);
		id.FromString(line);
	}
	else
	{
		id.Generate();
		std::string line;
		id.ToString(line);
		std::ofstream output(filename);
		output << line << std::endl;
		std::string path(szPath);
		path.erase(0, strlen(szOofsroot));
		_uuidsBase.AddEntry(id, path);
		std::string uuidbaseFilename;
		BuildUUIDBaseFilename(szOofsroot, uuidbaseFilename);
		_uuidsBase.Save(uuidbaseFilename);
	}

	return true;
}

bool VLB::LocalConnection::ReconcileGuids(const std::string& /*oofs*/, const std::string& /*path*/)
{
	return false;
}

bool VLB::LocalConnection::FixOofs()
{
	return false;
}



void VLB::LocalConnection::BuildNodeFilename(const std::string& path, std::string& result) const
{
	result = path;
	result += PathSeparator();
	result += DotNode;
}


void VLB::LocalConnection::BuildUUIDFilename(const std::string& path, std::string& result) const
{
	result = path;
	result += PathSeparator();
	result += DotId;
}

