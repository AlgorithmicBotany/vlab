#include <locale>
#include <vector>
#include <sstream>
#include <fstream>

#include <fw.h>

#include "connection.h"

#include <RAconsts.h>
#include "racomm.h"
#include "socket.h"
#include "remotecnct.h"

#include <shstrng.h>



const char DotId[] = ".id";
const char DotNode[] = "node";
const char DotOrdering[] = ".ordering";
const char DotUUids[] = ".uuids";

const int VLB::RemoteConnection::ArchiveBlockSize = 16384;

VLB::RemoteConnection::RemoteConnection(const char* host, const char* user, const char* pswd, const string_buffer& crlf) : 
_host(host),
_socket(AF_INET, SOCK_STREAM, 0),
_crlf(crlf)
{
	_crlf.add(DotId);
	_crlf.add(DotNode);
	_crlf.add(DotOrdering);
	const hostent* pHost = gethostbyname(host);
	if (0 == pHost)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCannotResolveHost), host);
	if (!_socket.Connect(pHost))
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCannotConnect), host);
	if (!_Login(user, pswd))
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCannotLogin));
}

bool VLB::RemoteConnection::_Login(const char* user, const char* paswd)
{
	std::stringstream txt;
	txt << user << ":" << paswd;
	RA::Request rqs(RA_LOGIN_REQUEST);
	rqs.SetText(txt.str());
	rqs.Send(_socket);
	RA::Response res(RA_LOGIN_RESPONSE);
	if (res.Read(_socket) && 0==strncmp(res.Txt(), "login confirmed", 15))
		return true;
	else 
		return false;
}

bool VLB::RemoteConnection::HasExtensions(const std::string& path)
{
	assert(ValidFname(path));
	RA::Request rqs(RA_GETDIR_REQUEST);
	rqs.SetText(path);
	rqs.Send(_socket);
	RA::Response res(RA_GETDIR_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pResponse = res.Txt();
	if (*pResponse != 'y')
		return false;
	else
		++pResponse;
	while (0 != *pResponse)
	{
		if (!strcmp(pResponse, "ext"))
			return true;
		pResponse += strlen(pResponse)+1;
	}

	return false;
}

bool VLB::RemoteConnection::GetParentPath(const std::string& path,
	std::string& parent)
{
	size_t pos = path.rfind("/ext/");
	if(pos == std::string::npos)
		return false;

	parent = path.substr(0,pos);
	return true;
}


bool VLB::RemoteConnection::GetFileList(string_buffer& list, const char* path, bool)
{
	assert(ValidPath(path));
	const int l = strlen(path);
	RA::Request rqs(RA_GETDIR_REQUEST);
	rqs.SetText(path, l);
	rqs.Send(_socket);
	RA::Response res(RA_GETDIR_RESPONSE);
	if (!res.Read(_socket))
		return false;
	if (!_RetrieveMultipleStrings(list, res.Txt()))
		return false;
	for (string_buffer::iterator it(list); !it.at_end(); it.advance())
	{
		if (it.is("ext"))
		{
			it.erase_string();
			return true;
		}
	}
	return true;
}


bool VLB::RemoteConnection::PutObject(const string_buffer&)
{

	return false;
}

bool VLB::RemoteConnection::_RetrieveMultipleFiles(const char* bf)
{
	if (*bf != 'y')
		return false;
	else
		++bf;

	while (0 != *bf)
	{
		const char* fnm = bf;
		bf += strlen(fnm)+1;
		int sz = _RetrieveInt(reinterpret_cast<const unsigned char*>(bf));
		assert(sizeof(int) == 4);
		bf += sizeof(int);
		if (ConvertCRLF(fnm))
		{
			WriteTextFile trg(fnm);
			trg.Write(bf, sz);
		}
		else
		{
			WriteBinFile trg(fnm);
			trg.Write(bf, sz);
		}
		bf += sz;
	}
	return true;
}

int VLB::RemoteConnection::_RetrieveInt(const unsigned char* bf)
{
	assert(sizeof(int) == 4);
	int res = bf[0] * 1 +
			  bf[1] * 256 +
			  bf[2] * 256*256 +
			  bf[3] * 256*256*256;
	return res;
}


int VLB::RemoteConnection::_RetrieveIntRev(const unsigned char* bf)
{
	assert(sizeof(int) == 4);
	int res = bf[3] * 1 +
			  bf[2] * 256 +
			  bf[1] * 256*256 +
			  bf[0] * 256*256*256;
	return res;
}

void VLB::RemoteConnection::_BuildIntRev(int v, unsigned char* bf)
{
	assert(sizeof(int) == 4);
	bf[0] = static_cast<unsigned char>(v >> 24);
	bf[1] = static_cast<unsigned char>(v >> 16);
	bf[2] = static_cast<unsigned char>(v >> 8);
	bf[3] = static_cast<unsigned char>(v >> 0);
}

bool VLB::RemoteConnection::_RetrieveMultipleStrings(string_buffer& bf, const char* pResponse)
{
	if (*pResponse != 'y')
		return false;
	else 
		++pResponse;
	while (0 != *pResponse)
	{
		bf.add(pResponse);
		pResponse += strlen(pResponse)+1;
	}
	return true;
}

bool VLB::RemoteConnection::GetExtensions(string_buffer& nms, string_buffer& bf, std::vector<int>& tp, const char* path)
{
	assert(ValidFname(path));
	const int l = strlen(path);
	RA::Request rqs(RA_GET_EXTENSIONS_REQUEST);
	rqs.SetText(path, l);
	rqs.Send(_socket);
	RA::Response res(RA_GET_EXTENSIONS_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pRes = res.Txt();
	if ('y' != *pRes)
		return false;
	// Skip 'y'
	++pRes;
	while (0 != *pRes)
	{
		// Skip the .ordering file
		if(strcmp(pRes,DotOrdering) == 0)
		{
			// Name
			while(0 != *pRes) ++pRes;
			++pRes;
			// Path
			while(0 != *pRes) ++pRes;
			++pRes;
			// Type
			++pRes;
			continue;
		}
		// Get name
		nms.add(pRes);
		// Skip name
		while (0 != *pRes)
			++pRes;
		++pRes;

		// Get path
		bf.add(pRes);

		// Skip path
		while (0 != *pRes)
		{
			++pRes;
		}

		++pRes;

		tp.push_back(*pRes);

		// Skip type
		++pRes;
	}

	return true;
}

bool VLB::RemoteConnection::GetOrdering(const char* parent,std::vector<std::string>& order)
{
	assert(ValidFname(parent));

	std::string extPath(parent);
	extPath += "/ext";

	std::string tmpName;
	if(!GetTmpFile(extPath.c_str(),DotOrdering,tmpName))
		return false;
	std::ifstream orderFile(tmpName.c_str());
	order.clear();
	while(!orderFile.eof())
	{
		std::string name;
		std::getline(orderFile,name);
		if(name.empty()) continue;
		order.push_back(name);
	}
	orderFile.close();
	::DeleteFile(tmpName.c_str());
	return true;
}


bool VLB::RemoteConnection::WriteOrdering(const char* parent, const std::vector<std::string>& order)
{
	assert(ValidFname(parent));

	std::string extPath(parent);
	extPath += "/ext";

	TempFileName tfn("ordtmp");
	std::ofstream out(tfn.c_str());
	for(size_t i = 0 ; i < order.size() ; i++)
		out << order[i] << std::endl;
	out.close();
	bool success = PutFile(extPath.c_str(),tfn.c_str(),DotOrdering);
	::DeleteFile(tfn.c_str());
	return success;
}


bool VLB::RemoteConnection::GetFile(const char* fname)
{
	assert(ValidFname(fname));
	const int l = strlen(fname);
	if (l>=3)
	{
		if (0==strcmp(fname+l-3, "ext"))
			return true;
	}
	RA::Request rqs(RA_FETCH_FILE_REQUEST);
	rqs.SetText(fname, l);
	rqs.Send(_socket);
	RA::Response res(RA_FETCH_FILE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pResponse = res.Txt();
	if (res.Length()==0)
		return false;
	if (*pResponse != 'y')
		return false;
	else
		pResponse++;
	const char* fnm = strrchr(fname, PathSeparator());
	if (0 == fnm)
		fnm = fname;
	else
		++fnm;
	if (ConvertCRLF(fname))
	{
		WriteTextFile trg(fnm);
		if (res.Length()>1)
		{
			for (int i=0; i<res.Length()-1; ++i)
			{
				if (*pResponse != '\r')
					trg.Write(*pResponse);
				++pResponse;
			}
		}
	}
	else
	{
		WriteBinFile trg(fnm);
		if (res.Length()>1)
			trg.Write(pResponse, res.Length()-1);
	}
	return true;
}



bool VLB::RemoteConnection::GetFile(const char* src, const char* trgnm)
{
	assert(ValidFname(src));
	const int l = strlen(src);
	if (l>=3)
	{
		if (0==strcmp(src+l-3, "ext"))
			return true;
	}
	RA::Request rqs(RA_FETCH_FILE_REQUEST);
	rqs.SetText(src, l);
	rqs.Send(_socket);
	RA::Response res(RA_FETCH_FILE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pResponse = res.Txt();
	if (res.Length()==0)
		return false;
	if (*pResponse != 'y')
		return false;
	else
		pResponse++;
	if (ConvertCRLF(src))
	{
		WriteTextFile trg(trgnm);
		if (res.Length()>1)
			trg.Write(pResponse, res.Length()-1);
	}
	else
	{
		WriteBinFile trg(trgnm);
		if (res.Length()>1)
			trg.Write(pResponse, res.Length()-1);
	}
	return true;
}



bool VLB::RemoteConnection::CompareFiles(const char* fname, const char* path)
{
	const bool crlf = ConvertCRLF(fname);
	assert(ValidFname(path));
	std::string remfile(path);
	remfile += PathSeparator();
	remfile += fname;
	RA::Request rqs(RA_FETCH_FILE_REQUEST);
	rqs.SetText(remfile.c_str(), remfile.length());
	rqs.Send(_socket);
	RA::Response res(RA_FETCH_FILE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pResponse = res.Txt();
	if (res.Length()==0)
		return false;
	if (*pResponse != 'y')
		return false;

	int ir = 1;
	std::ifstream lcfl(fname, std::ios::in | std::ios::binary);
	if (!lcfl.is_open())
		return false;

	for(;;)
	{
		char c;
		lcfl.read(&c, sizeof(char));

		// both same size and no differences
		if (lcfl.eof() && ir == res.Length())
			return true;
		else if (!lcfl.eof() && ir != res.Length())
		{
			if (crlf && (c == '\r'))
				continue;
			else if (c != pResponse[ir])
				return false;
			++ir;
		}
		else
			// different size
			return false;
	}
}



VLB::RemoteConnection::~RemoteConnection()
{
	try
	{
		RA::Request rqs(RA_LOGOUT_REQUEST);
		rqs.SetText("y", 2);
		rqs.Send(_socket);
		RA::Response res(RA_LOGOUT_RESPONSE);
		res.Read(_socket);
	}
	catch (...)
	{}
}


bool VLB::RemoteConnection::ValidPath(const std::string& path)
{
	if ('/' != path[0] && '~' != path[0])
		return false;

	if (std::string::npos != path.find_first_of('\\'))
		return false;
	if (std::string::npos != path.find_first_of(' '))
		return false;

	return true;
}

bool VLB::RemoteConnection::ValidFname(const std::string& fname)
{
	if (!ValidPath(fname))
		return false;
	if ('/' == fname.back())
		return false;
	return true;
}

bool VLB::RemoteConnection::PutFile(const char* src, const char* trg)
{
	const bool convertCRLF = ConvertCRLF(trg);
	std::string trgfnm(trg);
	_ValidateFName(trgfnm);
	size_t l = trgfnm.length();
	if (l>=3)
	{
		if (0==trgfnm.compare(l-3, 3, "ext"))
			return true;
	}
	RA::Request rqs(RA_PUTFILE_REQUEST);
	rqs.SetText(trgfnm.c_str(), trgfnm.length());
	ReadBinFile rf(src);
	long sz = rf.Size();
	for (long i=0; i<sz; ++i)
	{
		char c;
		rf.Read(&c, sizeof(char));
		if (!(convertCRLF && '\r' == c ))
			rqs.Add(c);
	}
	rqs.Send(_socket);
	RA::Response res(RA_PUTFILE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}


bool VLB::RemoteConnection::DeleteFile(const char* directory, const char* name) 
{
	std::string remnm(directory);
	ValidFname(remnm.c_str());
	remnm.append(PathSeparator(), 1);
	remnm.append(name);
	ValidFname(remnm.c_str());
	RA::Request rqs(RA_UNLINK_REQUEST);
	rqs.SetText(remnm.c_str(), remnm.length());
	rqs.Send(_socket);
	RA::Response res(RA_UNLINK_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}

bool VLB::RemoteConnection::MakeExt(const char* location, const char* name, std::string& dirname)
{
	assert(ValidFname(location));
	dirname = location;
	dirname += "/ext";
	_ValidateFName(dirname);
	_MakeDir(dirname.c_str(), dirname.length());
	dirname += PathSeparator();
	dirname += name;
	_ValidateFName(dirname);
	assert(ValidFname(dirname.c_str()));
	return _MakeDir(dirname.c_str(), dirname.length());
}

bool VLB::RemoteConnection::MakeHyperobject(const char* location, const char* name, const tGUID& id)
{
	assert(ValidFname(location));

	// create node file
	TempFileName nodeFile(DotNode);

	{
		std::ofstream nodeData(nodeFile.c_str());
		std::string line;
		id.ToString(line);
		nodeData << line << std::endl << name << std::endl;
		nodeData.close();
	}

	// place node file
	bool success = PutFile(location,nodeFile.c_str(),DotNode);

	::DeleteFile(nodeFile.c_str());
	return success;
}

bool VLB::RemoteConnection::_MakeDir(const char* dir, size_t length)
{
	assert(ValidFname(dir));
	RA::Request rqs(RA_MKDIR_REQUEST);
	rqs.SetText(dir, length);
	rqs.Send(_socket);
	RA::Response res(RA_MKDIR_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}


void VLB::RemoteConnection::_ValidateFName(std::string& fnm) const
{
	for (std::string::iterator it = fnm.begin(); it != fnm.end(); ++it)
	{
		if (*it == ' ')
			*it = '_';
	}
}


bool VLB::RemoteConnection::DelTree(const char* dir)
{
	assert(ValidFname(dir));
	RA::Request rqs(RA_DELTREE_REQUEST);
	rqs.SetText(dir, strlen(dir));
	rqs.Send(_socket);
	RA::Response res(RA_DELTREE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}


bool VLB::RemoteConnection::PrototypeObject(const char* dir)
{
	assert(ValidFname(dir));
	RA::Request rqs(RA_PROTOTYPE_OBJECT_REQUEST);
	rqs.SetText(dir, strlen(dir));
	rqs.Send(_socket);
	RA::Response res(RA_PROTOTYPE_OBJECT_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}


bool VLB::RemoteConnection::PutFile(const char* rempath, const char* lclname, const char* remname)
{
	assert(ValidFname(rempath));
	std::string trg(rempath);
	trg += PathSeparator();
	trg += remname;
	ValidFname(trg.c_str());
	return PutFile(lclname, trg.c_str());
}


class TmpFetch
{
public:
	TmpFetch(const char* oofsroot, const char* fname, VLB::Connection* pConnect) : 
		_rmnm(oofsroot),
		_pConnect(pConnect),
		_tmpfnm("tmpf")
	{
		  _rmnm += '/';
		  _rmnm += fname;
		  _pConnect->GetFile(_rmnm.c_str(), _tmpfnm.c_str());
	}
	~TmpFetch()
	{ DeleteFile(_tmpfnm.c_str()); }
	void Commit() const
	{ _pConnect->PutFile(_tmpfnm.c_str(), _rmnm.c_str()); }
	const std::string& Filename() const
	{ return _tmpfnm; }
private:
	std::string _rmnm;
	VLB::Connection* _pConnect;
	TempFileName _tmpfnm;
};




const char* VLB::RemoteConnection::_RelPath(const char* root, const char* objpath)
{
	assert(strlen(root)<strlen(objpath));
	const char* pRes = objpath + strlen(root);
	--pRes;
	while (PathSeparator() != *pRes)
		--pRes;
	++pRes;
	return pRes;
}



bool VLB::RemoteConnection::GetTmpFile(const char* rempath, const char* fname, std::string& lclpthnm)
{
	assert(ValidPath(rempath));
	std::string src(rempath);
	src += PathSeparator();
	src += fname;
	TempFileName tfn("gtf");
	BOOL b = GetFile(src.c_str(), tfn.c_str());

	if (!b)
		return false;
	lclpthnm = tfn;
	return true;
}


bool VLB::RemoteConnection::Paste(const char* oofs, const char* from, const char* to, bool recursive)
{
	// First archive the source
	assert(ValidFname(from));

	TmpFile tarfile;
	if (!Archive(tarfile, oofs, from, recursive))
		return false;

	return Dearchive(tarfile, oofs, from, to);
}

bool VLB::RemoteConnection::Archive(TmpFile& tarfile, const char* oofs, const char* path, bool recursive)
{
	{
		RA::Request rqs(RA_ARCHIVE_OBJECT_REQUEST);
		std::string rqst(oofs);
		rqst.append(1, '\0');
		rqst.append(path);
		rqst.append(1, '\0');
		if (recursive)
			rqst.append(1, '1');
		else
			rqst.append(1, '0');
		rqs.SetData(rqst.c_str(), rqst.length());
		rqs.Send(_socket);
	}

	int size = 0;

	{
		RA::Response res(RA_ARCHIVE_OBJECT_RESPONSE);
		if (!res.Read(_socket))
			return false;

		if (!res.IsOK())
			return false;
		const char* pResponse = res.Txt();

		++pResponse;
		size = _RetrieveIntRev(reinterpret_cast<const unsigned char*>(pResponse));
	}

	{
		int remain = size;
		while (remain>0)
		{
			RA::Response res(RA_ARCHIVE_BLOCK);
			if (!res.Read(_socket))
				return false;
			tarfile.Write(res.Txt(), res.Length());
			remain -= ArchiveBlockSize;
		}
	}

	return true;

}

bool VLB::RemoteConnection::Dearchive(TmpFile& tarfile, const char* oofsroot, const char* srcpath, const char* topath)
{
	{
		RA::Request rqs(RA_PASTE_OBJECT_REQUEST);
		std::string rqst(oofsroot);
		rqst.append(1, '\0');
		rqst.append(topath);
		rqst.append(1, '\0');
		rqst.append(srcpath);
		rqst.append(1, '\0');
		// do not move links
		rqst.append(1, '\x00');
		{
			unsigned char size[sizeof(int)];
			_BuildIntRev(tarfile.Size(), size);
			rqst.append(reinterpret_cast<const char*>(size), sizeof(int));
		}
		rqs.SetData(rqst.c_str(), rqst.length());
		rqs.Send(_socket);
	}

	{
		int remain = tarfile.Size();
		tarfile.Reset();
		RawMemory mem(ArchiveBlockSize);
		while (remain>0)
		{
			RA::Request block(RA_ARCHIVE_BLOCK);
			int blsz = remain;
			if (blsz>ArchiveBlockSize)
				blsz = ArchiveBlockSize;

			tarfile.Read(mem.Buffer(), blsz);
			block.SetData(mem.Buffer(), blsz);
			block.Send(_socket);
			remain -= blsz;

		}
	}
	{
		RA::Response res(RA_PASTE_OBJECT_RESPONSE);
		if (!res.Read(_socket))
			return false;
		return res.IsOK();
	}
}

bool VLB::RemoteConnection::RenameObj(const std::string& oofs, std::string& path, const char* name)
{
	assert(ValidFname(oofs.c_str()));
	assert(ValidFname(path.c_str()));

	if (IsHyperobject(path))
	{
		std::string nodeOld;
		GetTmpFile(path.c_str(),DotNode,nodeOld);
		std::ifstream nOld(nodeOld.c_str());
		TempFileName nodeNew(DotNode);
		std::ofstream nNew(nodeNew.c_str());
		std::string line;
		// first line: referenced id
		std::getline(nOld,line);
		nNew << line << std::endl;
		// second line: new name
		nNew << name << std::endl;
		nOld.close();
		::DeleteFile(nodeOld.c_str());
		nNew.close();
		bool success = PutFile(path.c_str(),nodeNew.c_str(),DotNode);
		::DeleteFile(nodeNew.c_str());
		return success;
	}
	else // normal object
	{
		std::string newnm(path);
		std::string::size_type pos = newnm.find_last_of(PathSeparator())+1;
		newnm.resize(pos);
		newnm.append(name);
	
		RawMemory cmnd(oofs.length()+1 + path.length()+1 + newnm.length() +1);
		int offset = 0;
		cmnd.Copy(offset, oofs.c_str(), oofs.length()+1);
		offset += oofs.length()+1;

		cmnd.Copy(offset, path.c_str(), path.length()+1);
		offset += path.length()+1;

		cmnd.Copy(offset, newnm.c_str(), newnm.length()+1);

		RA::Request rqs(RA_RENAME_OBJECT_REQUEST);
		rqs.SetData(cmnd.Buffer(), cmnd.Size());

		rqs.Send(_socket);

		RA::Response res(RA_RENAME_OBJECT_RESPONSE);
		if (!res.Read(_socket))
			return false;

		if (!res.IsOK())
			return false;

		path = newnm;
		return true;
	}
}

bool VLB::RemoteConnection::ConvertCRLF(const std::string& str) const
{
	std::string s;
	std::string::size_type pos = str.find_last_of(PathSeparator());
	if (pos != std::string::npos)
		s = str.substr(pos+1);
	else
		s = str;
	if (_crlf.contains(s.c_str()))
		return true;
	std::string::size_type dot = s.find_last_of('.');
	if (std::string::npos == dot)
		return false;
	std::string ext = s.substr(dot+1);
	return _crlf.contains(ext.c_str());
}


bool VLB::RemoteConnection::FindObject
(const std::string& oofs, 
 const std::string& start_from, 
 const ObjectFindInfo& fi, 
 std::string& output)
{
	assert(ValidFname(oofs.c_str()));
	{
		RA::Request rqs(RA_SEARCH_BEGIN_REQUEST);
		rqs.AddText(oofs);
		rqs.AddText(start_from);
		rqs.AddText(fi.Name());
		rqs.Add(fi.MatchCase() ? '\x1' : '\x0');
		rqs.Add(fi.WholeName() ? '\x1' : '\x0');
		rqs.Send(_socket);

		RA::Response res(RA_SEARCH_BEGIN_RESPONSE);
		if (!res.Read(_socket))
			return false;
	}

	{
		RA::Request rqs(RA_SEARCH_CONTINUE_REQUEST);
		rqs.SetText("void");
		rqs.Send(_socket);

		RA::Response res(RA_SEARCH_CONTINUE_RESPONSE);
		if (!res.Read(_socket))
			return false;

		output = res.Txt();
	}

	{
		RA::Request rqs(RA_SEARCH_END_REQUEST);
		rqs.SetText("void");
		rqs.Send(_socket);

		RA::Response res(RA_SEARCH_END_RESPONSE);
		res.Read(_socket);
	}
	return true;
}

bool VLB::RemoteConnection::IsHyperobject(const std::string& path)
{
	assert(ValidFname(path.c_str()));
	RA::Request rqs(RA_GETDIR_REQUEST);
	rqs.SetText(path.c_str(), path.length());
	rqs.Send(_socket);
	RA::Response res(RA_GETDIR_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pResponse = res.Txt();
	if (*pResponse != 'y')
		return false;
	else
		++pResponse;
	while (0 != *pResponse)
	{
		if (!strcmp(pResponse, DotNode))
			return true;
		pResponse += strlen(pResponse)+1;
	}

	return false;
}

bool VLB::RemoteConnection::GetUUID(const char* path, tGUID& id, std::string& name)
{
	assert(ValidFname(path));
	TempFileName tfn("idtmp");
	std::string tfile = tfn;
	std::string idfile = path; idfile += PathSeparator();
	idfile += DotNode;

	bool bSuccess = GetFile(idfile.c_str(), tfile.c_str());
	if (bSuccess)
	{
		std::ifstream idf(tfile.c_str());
		std::string line;
		std::getline(idf, line);
		id.FromString(line);
		std::getline(idf, name);
	}
	::DeleteFile(tfile.c_str());
	return bSuccess;
}


bool VLB::RemoteConnection::GetUUID(const char* oofsroot, const char* path, tGUID& id)
{
	RA::Request rqs(RA_GET_UUID_REQUEST);
	rqs.AddText(oofsroot);
	rqs.AddText(path);
	rqs.Add('\x1');
	rqs.Send(_socket);

	RA::Response res(RA_GET_UUID_RESPONSE);
	if (!res.Read(_socket))
		return false;

	const char* pResponse = res.Txt();
	id.FromString(pResponse);
	if (id.IsNull())
	{
		return false;
	}

	// check if this guid is already in the database
	std::string dbPath;
	if (_uuidsBase.LookupPath(id, dbPath))
	{
		return true;
	}

	// if it is a new guid add to database
	dbPath = path;
	dbPath.erase(0, strlen(oofsroot));
	_uuidsBase.AddEntry(id, dbPath);

	return true;
}


bool VLB::RemoteConnection::LookupPath
	(const tGUID& id, 
	const char* oofsroot, 
	std::string& path)
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


void VLB::RemoteConnection::LoadUUIDBase(const std::string& oofsroot)
{
	std::string dbfile = oofsroot; dbfile += PathSeparator(); dbfile += DotUUids;
	_lastUuidsOofs = oofsroot;
	TempFileName tfn("dbtmp");
	std::string tfile = tfn;
	bool bResult = GetFile(dbfile.c_str(), tfile.c_str());
	if (!bResult)
	{
		return;
	}

	_uuidsBase.Load(tfile);
	::DeleteFile(tfile.c_str());
}


void VLB::RemoteConnection::ReloadUUIDBase()
{
	if (!_lastUuidsOofs.empty())
	{
		LoadUUIDBase(_lastUuidsOofs);
	}
}

bool VLB::RemoteConnection::ReconcileGuids(const std::string& oofs, const std::string& path)
{
	RA::Request rqs(RA_RECONCILE_UUIDS_REQUEST);
	rqs.AddText(oofs);
	rqs.AddText(path);
	rqs.Add('\x1');
	rqs.Add(1);

	rqs.Send(_socket);
	RA::Response res(RA_RECONCILE_UUIDS_RESPONSE);
	if (!res.Read(_socket))
		return false;

	const char* pResponse = res.Txt();

	return (pResponse[0] == 'y');
}


bool VLB::RemoteConnection::FixOofs()
{

	return false;
}

