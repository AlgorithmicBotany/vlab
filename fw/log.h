#ifndef __LOG_H__
#define __LOG_H__


//#define DEBUGLOG

class LogFile
{
public:
	LogFile();
	~LogFile();
	void Open(const char*);
	void Log(const char*, ...);
	static LogFile theLog;
private:
	FILE* _fp;
};

#ifdef DEBUGLOG
#define LOG LogFile::theLog.Log("%s %d\n", __FILE__, __LINE__)
#else
#define LOG
#endif


#endif
