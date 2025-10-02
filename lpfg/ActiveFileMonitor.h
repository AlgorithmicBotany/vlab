/* ********************************************************************
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 * University of Calgary. All rights reserved.
 * ********************************************************************/


/*****
 * AcitiveFileMonitor.h
 *
 * Written by Steven Longay
 * Feburary 20/2009
 *****/
#ifndef ACTIVE_FILE_MONITOR_H
#define ACTIVE_FILE_MONITOR_H

#ifdef _WINDOWS
#include <windows.h>
#define CALLPREFIX WINAPI*
#else
#include <dlfcn.h>
#define CALLPREFIX *
#endif

#include <string>
using std::string;

#define MAXMODULE 500 

typedef void (CALLPREFIX cfuncStrStrStrFn)(const char*, const char*, const char*, void (*func)());
typedef float (CALLPREFIX cfuncStrFloat)(const char *, float);
typedef void (CALLPREFIX cfuncVoidStrFloat)(const char *, float);
typedef void (CALLPREFIX cfuncVoidStr)(const char *);
typedef float (CALLPREFIX cfuncFloatStr)(const char *);
typedef void (CALLPREFIX cfuncVoid)();

typedef void(*Proc)();
class ActiveFileMonitor
	{
	public:
		ActiveFileMonitor();
		~ActiveFileMonitor();
		
		bool parametersNeedUpdating();
		
		float setOrGetParameter(const char *name, float defValue);
		void setParameter(const char *name, float defValue);
		float getParameter(const char *name);
		void setParameterToDefault(const char *name);
		void monitorFile(const char*fn, const char *dir);
		void delayWrite();
		void write();
		
		string getMonitoredFileName();
		
	private:
		cfuncStrStrStrFn MonitorFile; 
		cfuncStrFloat SetOrGet;
		cfuncVoidStrFloat Set;
		cfuncFloatStr Get;
		cfuncVoidStr SetToDefault;
		cfuncVoid DelayWrite;
		cfuncVoid Write;
		
		string outFn, inFn;
		
		
#ifdef _WINDOWS
	HINSTANCE hLib;
#else
	void* hLib;
#endif

};


#endif
