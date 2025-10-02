#ifndef __COMLINEPARAM_H__
#define __COMLINEPARAM_H__


class CommandLineParams
{
public:
	CommandLineParams();
	void Parse();
	const RECT& InitRect() const
	{ return _InitialRect; }
	bool InitialProjectSpecified() const
	{ return !_InitialProject.empty(); }
	const std::string& InitialProject() const
	{ return _InitialProject; }
	bool DemoMode() const
	{ return _flags.IsSet(ftDemoMode); }
	bool SkipSplash() const
	{ return _flags.IsSet(ftSkipSplash); }
	bool ShowMinimized() const
	{ return _flags.IsSet(ftShowMinimized); }
private:
	std::string _InitialProject;
	RECT _InitialRect;
	enum FlagTags
	{
		ftDemoMode   = 1 << 0,
		ftSkipSplash = 1 << 1,
		ftShowMinimized = 1 << 2
	};
	FlagSet _flags;
};

#else
	#error File already included
#endif
