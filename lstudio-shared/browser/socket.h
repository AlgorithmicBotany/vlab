#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <winsock2.h>

#include "racomm.h"

namespace VLB
{


class SocketUser
{
public:
	SocketUser();
	~SocketUser();
};


class Socket
{
public:
	Socket(int, int, int);
	~Socket();
	bool Connect(const hostent*);
	void Send(int);
	void Send(const char*, int);
	RA::Message* Find(int);
private:
	void _RetrieveMsgId();
	int _ReadInt();
	void _Read(char*, int);
	SOCKET _s;
	int _msgId;
};

class Mem
{
public:
	Mem(int sz)
	{
		_a = reinterpret_cast<char*>(malloc(sz));
		if (0 == _a)
			throw Exception("Out of memory");
	}
	Mem(char* a)
	{ _a = a; }
	~Mem()
	{
		if (0 != _a)
			free(_a);
	}
	char* Release()
	{
		char* pRes = _a;
		_a = 0;
		return pRes;
	}
	char* Ptr()
	{ return _a; }
	char operator[](int i) const
	{ return _a[i]; }
private:
	char* _a;
};



}

#endif
