#include <cassert>
#include <cctype>
#include <cstring>
#include <string>

#include "warningset.h"

#include "configfile.h"

const char* ConfigFile::SkipBlanks(const char* s) const
{
	while (s[0] != 0 && isspace(s[0]))
		++s;
	return s;
}


const char* ConfigFile::FindNextBlank(const char* s) const
{
	while (!isspace(s[0]) && s[0] != 0)
		++s;

	return s;
}

int ConfigFile::_Label(const std::string& line, int &cntn, const char* lbls[], int count) const
{
	//if (0==cmnd[0])
	if (line.empty())
		return -2;
	//if (0==strncmp(cmnd, "//", 2))
	if (0==line.compare(0, 2, "//"))
		return -2;
	for (int i=0; /*i<count*/; ++i)
	{
		if ((count >= 0 && i==count) || lbls[i] == NULL)
		{
			break;
		}

		int l = strlen(lbls[i]);
		//if (0==strncmp(cmnd, lbls[i], l))
		if (0==line.compare(0, l, lbls[i]))
		{
			cntn = l;
			return i;
		}
	}
	cntn = 0;
	return -1;
}



bool ConfigFile::_ReadOnOff(const char* cmnd, unsigned int f)
{
	cmnd = SkipBlanks(cmnd);
	if (cmnd[0] == 0)
		return false;
	if (!strncmp(cmnd, "on", 2))
	{
		cmnd += 2;
		cmnd = SkipBlanks(cmnd);
		if (0 != cmnd[0])
			return false;
		_FlagSet(f);
		return true;
	}
	if (!strncmp(cmnd, "off", 3))
	{
		cmnd += 3;
		cmnd = SkipBlanks(cmnd);
		if (0 != cmnd[0])
			return false;
		_FlagClear(f);
		return true;
	}
	return false;
}



const char* ConfigFile::ReadQuotedString(const char* line, std::string& quotedString) const
{
	assert(*line == '\"');
	quotedString.clear();
	// skip \"
	++line;
	while (*line != '\"' && *line != 0)
	{
		quotedString.append(1, *line);
		++line;
	}
	if (*line == '\"')
		++line;
	else
		line = NULL;

	return line;
}

