#include <memory>

#include <fw.h>

#include <shstrng.h>

#include <RAconsts.h>
#include "racomm.h"
#include "socket.h"



VLB::SocketUser::SocketUser()
{
	WSADATA data;
	int res = WSAStartup(MAKEWORD(2, 0), &data);
	if (0 != res)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrWinSock));
}

VLB::SocketUser::~SocketUser()
{
	WSACleanup();
}


VLB::Socket::Socket(int af, int type, int protocol)
{
	_s = socket(af, type, protocol);
	if (INVALID_SOCKET == _s)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrCreateSock));
	_msgId = -1;
}


VLB::Socket::~Socket()
{
	closesocket(_s);
}


bool VLB::Socket::Connect(const hostent* pHost)
{
	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(RA_PORT);
	memcpy(&(address.sin_addr), pHost->h_addr, pHost->h_length);
	int res = connect(_s, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
	return (res == 0);
}


void VLB::Socket::Send(int n)
{
	assert(sizeof(int) == 4);
	unsigned char bf[4];
	bf[0] = static_cast<unsigned char>( ( n / 1 ) % 256 );
	bf[1] = static_cast<unsigned char>( ( n / 256 ) % 256 );
	bf[2] = static_cast<unsigned char>( ( n / (256*256) ) % 256 );
	bf[3] = static_cast<unsigned char>( ( n / (256*256*256) ) % 256 );
	int sent = 0;
	while (sent < 4)
	{
		int res = send(_s, reinterpret_cast<const char*>(bf+sent), 4-sent, 0);
		int socketErr = WSAGetLastError();

		if (SOCKET_ERROR == res)
		{
			sent += socketErr;
			throw Exception(SharedStr::GetLibString(SharedStr::strErrSocketWrite));
		}
		sent += res;
	}
}


void VLB::Socket::Send(const char* bf, int sz)
{
	int sent = 0;
	while (sent < sz)
	{
		int res = send(_s, reinterpret_cast<const char*>(bf+sent), sz-sent, 0);
		int socketErr = WSAGetLastError();

		if (SOCKET_ERROR == res)
		{
			sent += socketErr;
			throw Exception(SharedStr::GetLibString(SharedStr::strErrSocketWrite));
		}
		sent += res;
	}
}



RA::Message* VLB::Socket::Find(int msg)
{
	if (_msgId == -1)
		_RetrieveMsgId();
	if (_msgId == msg)
	{
		int len = _ReadInt();
		Mem m(len);
		_Read(m.Ptr(), len);
		std::unique_ptr<RA::Message> result(new RA::Message(_msgId, len, m.Release()));
		_msgId = -1;
		return result.release();
	}
	else
		return 0;
}


int VLB::Socket::_ReadInt()
{
	assert(sizeof(int)==4);
	unsigned char bf[4];
	int read = 0;
	while (read < 4)
	{
		int res = recv(_s, reinterpret_cast<char*>(bf+read), 4-read, 0);
		if (SOCKET_ERROR == res)
			throw Exception(SharedStr::GetLibString(SharedStr::strErrSocketRead));
		else if (0==res)
			throw Exception(SharedStr::GetLibString(SharedStr::strErrConnectionClosed));
		read += res;
	}
	int res = bf[0] * 1 +
			  bf[1] * 256 +
			  bf[2] * 256*256 +
			  bf[3] * 256*256*256;
	return res;
}


void VLB::Socket::_RetrieveMsgId()
{
	_msgId = _ReadInt();
}

void VLB::Socket::_Read(char* pB, int sz)
{
	int read = 0;
	while (read<sz)
	{
		int res = recv(_s, pB+read, sz-read, 0);
		if (SOCKET_ERROR == res)
			throw Exception(SharedStr::GetLibString(SharedStr::strErrSocketRead));
		else if (0==res)
			throw Exception(SharedStr::GetLibString(SharedStr::strErrConnectionClosed));
		read += res;
	}
}
