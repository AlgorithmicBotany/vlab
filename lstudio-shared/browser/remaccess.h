#ifndef __REMOTEACCESS_H__
#define __REMOTEACCESS_H__


namespace VLB
{

class Options;

class RemoteAccess
{
public:
	RemoteAccess() : _refCount(0) {}
	void AddRef() { ++_refCount; }
	void Release()
	{
		assert(_refCount>0);
		--_refCount;
	}
	virtual bool PutFile(const char*, const char*) = 0;
	virtual bool MakeExtension(std::string& /* I: original location, O: location of extension */, const char*, const char*, const string_buffer&) = 0;
	virtual bool PrototypeObject(const char*) = 0;
	virtual bool Connected() const = 0;
	virtual const char* CurrentObjectPath() const = 0;
	virtual const char* OofsRoot() const = 0;
	virtual bool CompareFiles(const char*, const char*) const = 0;
	virtual bool GetFileList(const char*, string_buffer&, bool) const = 0;
	virtual bool FetchObject(const std::string&, const std::string&) = 0;
	virtual bool GetExtensions(const char*, string_buffer&) const = 0;
	virtual bool DeleteFile(const char* directory, const char* name) = 0;
	bool CanClose() const
	{ return 0==_refCount; }
	virtual void QuitSilently() = 0;
	virtual char PathSeparator() const = 0;
	virtual void PositionObject(const std::string&) = 0;
	virtual void ShowInFront() = 0;
	virtual void ApplyOptions(const Options&) = 0;
	virtual bool SupportsTar() const = 0;
	virtual bool Archive(const char*, TmpFile&, bool) = 0;
private:
	int _refCount;
};


}


#else
	#error File already included
#endif
