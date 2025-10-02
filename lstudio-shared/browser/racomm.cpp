
#include <memory>

#include <fw.h>
#include <winsock2.h>

#include <RAconsts.h>
#include "racomm.h"
#include "socket.h"


RA::Message::~Message()
{
	if (0 != _pData)
		free(_pData);
}

void RA::Message::_Transfer(Message& msg)
{
	_msg = msg._msg; msg._msg = -1;
	_len = msg._len; msg._len = -1;
	_pData = msg._pData; msg._pData = 0;
}


void RA::Request::SetText(const char* txt, int len)
{
	assert(_pData == 0);
	if (len>0)
	{
		_len = len;
		_pData = reinterpret_cast<char*>(malloc(len+1));
		strncpy(_pData, txt, _len);
		_pData[_len] = 0;
		++_len;
		_end = _len;
	}
}

void RA::Request::SetText(const std::string& txt)
{
	assert(_pData == 0);
	if (txt.length()>0)
	{
		_len = txt.length();
		_pData = reinterpret_cast<char*>(malloc(_len+1));
		strncpy(_pData, txt.c_str(), _len);
		_pData[_len] = 0;
		++_len;
		_end = _len;
	}
}


void RA::Request::SetData(const void* txt, int len)
{
	assert(_pData == 0);
	if (len>0)
	{
		_len = len;
		_pData = reinterpret_cast<char*>(malloc(len));
		memcpy(_pData, txt, len);
		_end = _len;
	}
}


void RA::Request::AddText(const std::string& txt)
{
	if (_pData == NULL)
	{
		_Alloc();
	}
	while (FreeSpace() < txt.length()+1)
	{
		_Grow();
	}

	memcpy(_pData+_len, txt.c_str(), txt.length()+1);
	_len += txt.length()+1;
}



void RA::Request::Add(char c)
{
	if (0 == _pData)
		_Alloc();
	if (_end == _len)
		_Grow();
	_pData[_len] = c;
	++_len;
}

void RA::Request::_Alloc()
{
	_pData = reinterpret_cast<char*>(malloc(eDefSize));
	_len = 0;
	_end = eDefSize;
}

void RA::Request::_Grow()
{
	int sz = 2*_end;
	_pData = reinterpret_cast<char*>(realloc(_pData, sz));
	if (0 == _pData)
		throw Exception("Out of memory");
	_end = sz;
}


void RA::Request::Send(VLB::Socket& s) 
{
	s.Send(_msg);
	s.Send(_len);
	s.Send(_pData, _len);
}


bool RA::Response::Read(VLB::Socket& s)
{
	std::unique_ptr<Message> msg(s.Find(_msg));
	if (0 != msg.get())
	{
		_Transfer(*msg);
		return true;
	}
	else
		return false;
}

