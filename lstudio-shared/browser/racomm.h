#ifndef __RACOMM_H__
#define __RACOMM_H__


namespace VLB
{
	class Socket;
}

namespace RA
{


class Message
{
public:
	Message(int msg, int len, char* pData) : _msg(msg), _pData(pData), _len(len) {}
	virtual ~Message();
protected:
	Message() : _msg(-1), _len(-1), _pData(0) {}
	void _Transfer(Message&);
	int _msg;
	int _len;
	char* _pData;
private:
	Message(const Message&);
};


class Request : public Message
{
public:
	Request(int msg) : Message(msg, -1, 0), _end(0)
	{}
	void SetText(const char* txt, int len);
	void SetText(const std::string&);
	void SetData(const void* txt, int len);
	void Add(char);
	void AddText(const std::string&);
	void Send(VLB::Socket&);
private:
	enum
	{ eDefSize = 256 };
	int _end;
	void _Alloc();
	void _Grow();
	size_t FreeSpace() const { return _len - _end; }
};


class Response : public Message
{
public:
	Response(int msg) 
	{ _msg = msg; _len = -1; _pData = 0; }
	bool Read(VLB::Socket&);
	const char* Txt() const
	{ 
		assert(0 != _pData);
		return _pData; 
	}
	char* ReleaseData()
	{
		assert(0 != _pData);
		char* pRes = _pData;
		_pData = 0;
		return pRes;
	}
	int Length() const
	{ return _len; }
	bool IsOK() const
	{ 
		assert(0 != _pData);
		return 'y' == _pData[0]; 
	}
};

}


#endif
