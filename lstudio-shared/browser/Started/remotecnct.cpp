#include <vector>
#include <strstream>
#include <fstream>

#include <fw.h>

#include "connection.h"

#include "ra.h"
#include "racomm.h"
#include "socket.h"
#include "remotecnct.h"

#include <shstrng.h>


VLB::RemoteConnection::RemoteConnection(const std::string& host, const std::string& user, const std::string& pswd, const string_buffer& crlf) : 
_host(host),
_socket(AF_INET, SOCK_STREAM, 0),
_crlf(crlf)
{
	_crlf.add(".dbase");
	_crlf.add(".id");
	const hostent* pHost = gethostbyname(host.c_str());
	if (0 == pHost)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCannotResolveHost), host.c_str());
	if (!_socket.Connect(pHost))
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCannotConnect), host.c_str());
	if (!_Login(user, pswd))
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCannotLogin));
}

bool VLB::RemoteConnection::_Login(const std::string& user, const std::string& paswd)
{
	std::strstream txt;
	txt << user << ":" << paswd;
	RA::Request rqs(RA::RA_LOGIN_REQUEST);
	rqs.SetText(txt.str(), txt.pcount());
	rqs.Send(_socket);
	RA::Response res(RA::RA_LOGIN_RESPONSE);
	if (res.Read(_socket) && 0==strncmp(res.Txt(), "login confirmed", 15))
		return true;
	else 
		return false;
}

bool VLB::RemoteConnection::HasExtensions(const std::string& path)
{
	assert(_ValidFname(path));
	//const int l = strlen(path);
	RA::Request rqs(RA::RA_GETDIR_REQUEST);
	rqs.SetText(path.c_str(), path.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_GETDIR_RESPONSE);
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


bool VLB::RemoteConnection::GetFileList(string_buffer& list, const std::string& path, bool)
{
	assert(_ValidFname(path));
	//const int l = strlen(path);
	RA::Request rqs(RA::RA_GETDIR_REQUEST);
	rqs.SetText(path.c_str(), path.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_GETDIR_RESPONSE);
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


bool VLB::RemoteConnection::FetchObject(const std::string& path)
{
	//const int l = strlen(path);
	RA::Request rqs(RA::RA_FETCH_OBJECT_REQUEST);
	rqs.SetText(path.c_str(), path.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_FETCH_OBJECT_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return _RetrieveMultipleFiles(res.Txt());
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

bool VLB::RemoteConnection::GetExtensions(string_buffer& nms, string_buffer& bf, std::vector<int>& tp, const std::string& path)
{
	assert(_ValidFname(path));
	RA::Request rqs(RA::RA_GET_EXTENSIONS_REQUEST);
	rqs.SetText(path.c_str(), path.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_GET_EXTENSIONS_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pRes = res.Txt();
	if ('y' != *pRes)
		return false;
	// Skip 'y'
	++pRes;
	while (0 != *pRes)
	{
		// Skip name
		nms.add(pRes);
		while (0 != *pRes)
			++pRes;
		// Skip terminating zero
		++pRes;
		// Append path
		bf.add(pRes);
		// Skip path
		while (0 != *pRes)
			++pRes;
		// Skip terminating zero
		++pRes;
		tp.push_back(*pRes);
		++pRes;
	}
	return true;
}


bool VLB::RemoteConnection::GetFile(const std::string& fname)
{
	assert(_ValidFname(fname));
	if (fname.length()>=3)
	{
		if (fname.substr(fname.length()-3) == "ext")
		//if (0==strcmp(fname+l-3, "ext"))
			return true;
	}
	RA::Request rqs(RA::RA_FETCH_FILE_REQUEST);
	rqs.SetText(fname.c_str(), fname.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_FETCH_FILE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pResponse = res.Txt();
	if (res.Length()==0)
		return false;
	if (*pResponse != 'y')
		return false;
	else
		pResponse++;
	//const char* fnm = strrchr(fname, '/');
	size_t pos = fname.find_last_of('/');
	std::string fnm;
	if (std::string::npos == pos)
		fnm = fname;
	else
		fnm = fname.substr(pos+1);

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



bool VLB::RemoteConnection::GetFile(const std::string& src, const std::string& trgnm)
{
	assert(_ValidFname(src));
	//const int l = strlen(src);
	if (src.length()>=3)
	{
		if (src.substr(src.length()-3)=="ext")
		//if (0==strcmp(src+l-3, "ext"))
			return true;
	}
	RA::Request rqs(RA::RA_FETCH_FILE_REQUEST);
	rqs.SetText(src.c_str(), src.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_FETCH_FILE_RESPONSE);
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



bool VLB::RemoteConnection::CompareFiles(const std::string& fname, const std::string& path)
{
	const bool crlf = ConvertCRLF(fname);
	assert(_ValidFname(path));
	std::string remfile(path);
	remfile += '/';
	remfile += fname;
	RA::Request rqs(RA::RA_FETCH_FILE_REQUEST);
	rqs.SetText(remfile.c_str(), remfile.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_FETCH_FILE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	const char* pResponse = res.Txt();
	if (res.Length()==0)
		return false;
	if (*pResponse != 'y')
		return false;

	int ir = 1;
	std::ifstream lcfl(fname.c_str(), std::ios::binary);
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
		RA::Request rqs(RA::RA_LOGOUT_REQUEST);
		rqs.SetText("y", 2);
		rqs.Send(_socket);
		RA::Response res(RA::RA_LOGOUT_RESPONSE);
		res.Read(_socket);
	}
	catch (...)
	{}
}


bool VLB::RemoteConnection::_ValidFname(const std::string& fname) const
{
	if ('/' != fname[0])
		return false;
	size_t pos = fname.find_first_of("\\ ");
	if (pos != std::string::npos)
		return false;
	/*while (0 != *fname)
	{
		if ('\\' == *fname || ' ' == *fname)
			return false;
		++fname;
	}*/

	if (fname[fname.length()-1] == '/')
		return false;
	/*if ('/' == *(fname-1))
		return false;*/
	return true;
}

bool VLB::RemoteConnection::PutFile(const std::string& src, const std::string& trg)
{
	const bool convertCRLF = ConvertCRLF(src);
	std::string trgfnm(trg);
	_ValidateFName(trgfnm);
	size_t l = trgfnm.length();
	if (l>=3)
	{
		if (0==trgfnm.compare(l-3, 3, "ext"))
			return true;
	}
	RA::Request rqs(RA::RA_PUTFILE_REQUEST);
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
	RA::Response res(RA::RA_PUTFILE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}


bool VLB::RemoteConnection::DeleteFile(const std::string& directory, const std::string& name) 
{
	std::string remnm(directory);
	//_ValidFname(remnm);
	remnm.append("/");
	remnm.append(name);
	//_ValidFname(remnm);
	RA::Request rqs(RA::RA_UNLINK_REQUEST);
	rqs.SetText(remnm.c_str(), remnm.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_UNLINK_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}

bool VLB::RemoteConnection::MakeExt(const std::string& location, const std::string& name, std::string& dirname)
{
	assert(_ValidFname(location));
	dirname = location;
	dirname += "/ext";
	_ValidateFName(dirname);
	_MakeDir(dirname);
	dirname += '/';
	dirname += name;
	_ValidateFName(dirname);
	assert(_ValidFname(dirname.c_str()));
	return _MakeDir(dirname);
}

bool VLB::RemoteConnection::_MakeDir(const std::string& dir)
{
	assert(_ValidFname(dir));
	RA::Request rqs(RA::RA_MKDIR_REQUEST);
	rqs.SetText(dir.c_str(), dir.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_MKDIR_RESPONSE);
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


bool VLB::RemoteConnection::DelTree(const std::string& dir)
{
	assert(_ValidFname(dir));
	RA::Request rqs(RA::RA_DELTREE_REQUEST);
	rqs.SetText(dir.c_str(), dir.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_DELTREE_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}


bool VLB::RemoteConnection::PrototypeObject(const std::string& dir)
{
	assert(_ValidFname(dir));
	RA::Request rqs(RA::RA_PROTOTYPE_OBJECT_REQUEST);
	rqs.SetText(dir.c_str(), dir.length());
	rqs.Send(_socket);
	RA::Response res(RA::RA_PROTOTYPE_OBJECT_RESPONSE);
	if (!res.Read(_socket))
		return false;
	return res.IsOK();
}


bool VLB::RemoteConnection::PutFile(const std::string& rempath, const std::string& lclname, const std::string& remname)
{
	assert(_ValidFname(rempath));
	std::string trg(rempath);
	trg += "/";
	trg += remname;
	//_ValidFname(trg.c_str());
	return PutFile(lclname, trg);
}


class DBase
{
public:
	DBase(const char* fname)
	{
		std::ifstream src(fname);
		if (!src.is_open())
			throw Exception("Error reading .dbase");
		std::string line;
		std::getline(src, line, '\n');
		_count = atoi(line.c_str());
		_lastid = 0;
		for (;;)
		{
			std::getline(src, line, '\n');
			if (!src.eof())
			{
				size_t id = atoi(line.c_str());
				size_t i = 0;
				while (isdigit(line[i]))
					++i;
				while (isspace(line[i]))
					++i;

				_ids.push_back(id);
				_pths.add(line.c_str()+i);
				if (id>_lastid)
					_lastid = id;
			}
			else
				break;
		}
	}
	size_t Add(const char* path)
	{
		_ids.push_back(_lastid+1);
		_pths.add(path);
		++_count;
		++_lastid;
		return _lastid;
	}
	void Write(const char* fnm) const
	{
		std::ofstream trg(fnm);
		trg << _count << std::endl;
		size_t it = _pths.begin();
		for (size_t i = 0; i<_count; ++i)
		{
			trg << _ids[i] << ' ' << _pths.string(it) << std::endl;
			it = _pths.find_next(it);
		}
	}
	void Delete(const char* fnm)
	{
		size_t pos = _pths.last_string();
		std::vector<size_t>::iterator it = _ids.end();
		--it;
		std::string pth = fnm;
		pth += '/';
		const size_t l = pth.length();
		while (pos != string_buffer::npos)
		{
			if (0==strncmp(pth.c_str(), _pths.string(pos), l) || 0==strcmp(fnm, _pths.string(pos)))
			{
				_pths.erase(pos);
				std::vector<size_t>::iterator tmp = it;
				--tmp;
				_ids.erase(it);
				it = tmp;
				--_count;
			}
			else
				--it;
			pos = _pths.find_prev(pos);
		}
	}
private:
	size_t _count;
	size_t _lastid;
	std::vector<size_t> _ids;
	string_buffer _pths;
};


class IdFile 
{
public:
	IdFile(const char* oofsroot, const char* dirname, VLB::Connection* pConnect)
	{
		std::string dbfile = oofsroot; dbfile += "/.dbase";
		static char tmpth[MAX_PATH+1];
		GetTempPath(MAX_PATH, tmpth);
		GetTempFileName(tmpth, "dbtmp", 0, _tmpnm);
		pConnect->GetFile(dbfile.c_str(), _tmpnm);
		DBase db(_tmpnm);
		const char* relpath = dirname + strlen(oofsroot);
		--relpath;
		while (*relpath != '/')
			--relpath;
		++relpath;
		int id = db.Add(relpath);
		std::ofstream idf(".id");
		idf << id << std::endl;
		db.Write(_tmpnm);
		pConnect->PutFile(_tmpnm, dbfile.c_str());
	}
	~IdFile()
	{
		DeleteFile(_tmpnm);
		DeleteFile(".id");
	}
private:
	char _tmpnm[MAX_PATH+1];
};


class TmpFetch
{
public:
	TmpFetch(const char* oofsroot, const char* fname, VLB::Connection* pConnect) : 
		_rmnm(oofsroot),
		_pConnect(pConnect)
	{
		  _rmnm += '/';
		  _rmnm += fname;
		  GetTempFileName(".", "tmpf", 0, _tmpnm);
		  _pConnect->GetFile(_rmnm.c_str(), _tmpnm);
	}
	~TmpFetch()
	{ DeleteFile(_tmpnm); }
	void Commit() const
	{ _pConnect->PutFile(_tmpnm, _rmnm.c_str()); }
	const char* Filename() const
	{ return _tmpnm; }
private:
	std::string _rmnm;
	VLB::Connection* _pConnect;
	char _tmpnm[MAX_PATH+1];
};




bool VLB::RemoteConnection::AddId(const std::string& oofsroot, const std::string& path)
{
	assert(_ValidFname(path));
	assert(_ValidFname(oofsroot));
	IdFile idf(oofsroot.c_str(), path.c_str(), this);
	std::string trg(path);
	trg.append("/.id");
	PutFile(".id", trg.c_str());
	return true;
}

const char* _RelPath(const std::string& root, const std::string& objpath)
{
	assert(root.length()<objpath.length());
	//assert(strlen(root)<strlen(objpath));
	const char* pRes = objpath.c_str() + root.length();
	--pRes;
	while ('/' != *pRes)
		--pRes;
	++pRes;
	return pRes;
}




bool VLB::RemoteConnection::DeleteIds(const std::string& oofsroot, const std::string& path)
{
	TmpFetch tf(oofsroot.c_str(), ".dbase", this);
	DBase dbase(tf.Filename());
	const char* relpath = _RelPath(oofsroot, path);
	dbase.Delete(relpath);
	dbase.Write(tf.Filename());
	tf.Commit();
	return true;
}


bool VLB::RemoteConnection::GetTmpFile(const std::string& rempath, const std::string& fname, std::string& lclpthnm)
{
	assert(_ValidFname(rempath));
	std::string src(rempath);
	src += "/";
	src += fname;
	char trg[MAX_PATH+1];
	GetTempPath(MAX_PATH, trg);
	UINT res = GetTempFileName(trg, "gtf", 0, trg);
	if (0==res)
		return false;

	BOOL b = GetFile(src, trg);

	if (!b)
		return false;
	lclpthnm = trg;
	return true;
}


bool VLB::RemoteConnection::Paste(const std::string& oofs, const std::string& from, const std::string& to, bool recursive)
{
	// First archive the source
	assert(_ValidFname(from));

	{
		RA::Request rqs(RA::RA_ARCHIVE_OBJECT_REQUEST);
		std::string rqst(oofs);
		rqst.append(1, '\0');
		rqst.append(from);
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
		RA::Response res(RA::RA_ARCHIVE_OBJECT_RESPONSE);
		if (!res.Read(_socket))
			return false;

		if (!res.IsOK())
			return false;
		const char* pResponse = res.Txt();

		++pResponse;
		size = _RetrieveIntRev(reinterpret_cast<const unsigned char*>(pResponse));
	}

	RawMemory tar(size);

	{
		int remain = size;

		int offset = 0;

		while (remain>0)
		{
			RA::Response res(RA::RA_ARCHIVE_BLOCK);
			if (!res.Read(_socket))
				return false;
			tar.Copy(offset, res.Txt(), res.Length());
			offset += res.Length();
			remain -= 16384;
		}
	}


	{
		RA::Request paste(RA::RA_PASTE_OBJECT_REQUEST);
		RawMemory pstdt(oofs.length()+1 + from.length()+1 + to.length()+1 + 1 /*move links*/ + sizeof(int));
		int offset = 0;
		pstdt.Copy(offset, oofs.c_str(), oofs.length()+1);
		offset += oofs.length()+1;
		pstdt.Copy(offset, to.c_str(), to.length()+1);
		offset += to.length()+1;
		pstdt.Copy(offset, from.c_str(), from.length()+1);
		offset += from.length()+1;
		{
			const char move_links = '\x01';
			pstdt.Copy(offset, &move_links, 1);
			++offset;
		}
		{
			unsigned char arch_size[sizeof(int)];
			_BuildIntRev(size, arch_size);
			pstdt.Copy(offset, arch_size, sizeof(int));
			offset += sizeof(int);
		}
		paste.SetData(pstdt.Buffer(), offset);
		paste.Send(_socket);
	}

	{
		int remain = size;
		int offset = 0;
		const int BlockSize = 16384;
		while (remain>0)
		{
			RA::Request arch_bl(RA::RA_ARCHIVE_BLOCK);

			int block_size = remain;
			
			if (block_size>BlockSize)
				block_size = BlockSize;

			arch_bl.SetData(tar.Buffer(offset), block_size);
			arch_bl.Send(_socket);
			offset += block_size;
			remain -= block_size;
		}
	}

	{
		RA::Response res(RA::RA_PASTE_OBJECT_RESPONSE);
		if (!res.Read(_socket))
			return false;
		return res.IsOK();
	}
}


bool VLB::RemoteConnection::RenameObj(const std::string& oofs, std::string& path, const std::string& name)
{
	assert(_ValidFname(oofs));
	assert(_ValidFname(path));
	std::string newnm(path);
	std::string::size_type pos = newnm.find_last_of('/')+1;
	newnm.resize(pos);
	newnm.append(name);
	
	RawMemory cmnd(oofs.length()+1 + path.length()+1 + newnm.length() +1);
	int offset = 0;
	cmnd.Copy(offset, oofs.c_str(), oofs.length()+1);
	offset += oofs.length()+1;

	cmnd.Copy(offset, path.c_str(), path.length()+1);
	offset += path.length()+1;

	cmnd.Copy(offset, newnm.c_str(), newnm.length()+1);

	RA::Request rqs(RA::RA_RENAME_OBJECT_REQUEST);
	rqs.SetData(cmnd.Buffer(), cmnd.Size());

	rqs.Send(_socket);

	RA::Response res(RA::RA_RENAME_OBJECT_RESPONSE);
	if (!res.Read(_socket))
		return false;

	if (!res.IsOK())
		return false;

	path = newnm;
	return true;
}

bool VLB::RemoteConnection::ConvertCRLF(const std::string& str) const
{
	std::string s;
	std::string::size_type pos = str.find_last_of('/');
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
