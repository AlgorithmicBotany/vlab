#ifndef __COMMANDLINE_H__
#define __COMMANDLINE_H__


class CommandLine
{
public:
	int ArgC() const;
	const char* ArgV(int) const;
};

#endif
