#ifndef __CONFIGFILE_H__
#define __CONFIGFILE_H__


class ConfigFile
{
protected:
	ConfigFile() : _Flags(0)
	{}
	int _Label(const std::string& line, int&, const char* lbls[], int) const;
	bool _ReadOnOff(const char*, unsigned int);
	void _FlagSet(unsigned int f)
	{ _Flags |= f; }
	void _FlagClear(unsigned int f)
	{ _Flags &= ~f; }
	bool _IsFlagSet(unsigned int f) const
	{ return f == (_Flags & f); }
	void _FlagReset(unsigned int f, bool b)
	{ 
		if (b)
			_FlagSet(f);
		else
			_FlagClear(f);
	}
	const char* SkipBlanks(const char* s) const;
	const char* ReadQuotedString(const char* line, std::string& quotedString) const;
	const char* FindNextBlank(const char* s) const;
protected:
	enum tPredefinedLabels
	{
		eEmptyLine = -1,
		eComment   = -2
	};
private:
	unsigned int _Flags;
};


#endif
