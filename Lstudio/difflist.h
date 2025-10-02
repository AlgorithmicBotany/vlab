#ifndef __DIFFLIST_H__
#define __DIFFLIST_H__

namespace VLB
{
	class RemoteAccess;
}


class DiffList
{
public:
	// Files are stAdded
	void AddSrc1(const TCHAR*);
	// Files which have not been added by AddSrc1 are stDeleted
	// Others are assumed to be stIdentical
	void AddSrc2(const TCHAR*);

	// Compares the files in the current directory 
	// and the directory dir
	// filenames are taken from _aEntry
	void Compare(const TCHAR* dir);
	void Compare(const VLB::RemoteAccess*, const char* lcldir, const char* remdir);
	bool Identical() const;

	int Items() const
	{ return _aEntry.size(); }

	enum eStatus
	{
		stAdded,
			stDeleted,
			stModified,
			stIdentical
	};

	struct Entry
	{
		TCHAR name[MAX_PATH+1];
		eStatus status;
	};

	eStatus Status(size_t i) const
	{
		assert(i<_aEntry.size());
		return _aEntry[i].status;
	}
	const TCHAR* Name(size_t i) const
	{
		assert(i<_aEntry.size());
		return _aEntry[i].name;
	}
private:
	std::vector<Entry> _aEntry;
	typedef std::vector<Entry>::iterator iter;
	typedef std::vector<Entry>::const_iterator citer;
	iter _Find(const TCHAR*);
	bool _Compare(const TCHAR*, const TCHAR*) const;
};


#else
	#error File already included
#endif
