#include <stdio.h>
#include <stdarg.h>

#include "warningset.h"

#include "log.h"


LogFile LogFile::theLog;

LogFile::LogFile()
{
	_fp = 0;
}

LogFile::~LogFile()
{
	if (0 != _fp)
	{
		fputs("Flushing\n", _fp);
		fclose(_fp);
	}
}


void LogFile::Open(const char* fname)
{
	_fp = fopen(fname, "wt");
	if (0 != _fp)
	{
		setvbuf(_fp, 0, _IONBF, 0);
	}
}

void LogFile::Log(const char* format, ...)
{
	if (0 != _fp)
	{
		va_list args;
		va_start(args, format);
		vfprintf(_fp, format, args);
		va_end(args);
	}
}
