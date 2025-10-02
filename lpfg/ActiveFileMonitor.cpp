/* ********************************************************************
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 * University of Calgary. All rights reserved.
 * ********************************************************************/


#include "ActiveFileMonitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include "utils.h"

#include "bundle.h"

using namespace std;

bool g_UpdateParameters = true;
void parameterFileChanged()
{
	g_UpdateParameters = true;
}

ActiveFileMonitor::ActiveFileMonitor()
{
	hLib = NULL;
#ifdef _WINDOWS
	hLib=LoadLibrary("ActiveFileMonitor.dll");
	
	if(hLib!=NULL) {            
		Utils::Log("Active file monitor loaded.\n");
	}      
	
	MonitorFile=(cfuncStrStrStrFn)GetProcAddress((HMODULE)hLib, "_MonitorFile@16");
	SetOrGet=(cfuncStrFloat) GetProcAddress((HMODULE)hLib, "_SetOrGetParameter@8");
	Set=(cfuncVoidStrFloat) GetProcAddress((HMODULE)hLib, "_SetParameter@8");
	Get=(cfuncFloatStr) GetProcAddress((HMODULE)hLib, "_GetParameter@4");
	SetToDefault=(cfuncVoidStr) GetProcAddress((HMODULE)hLib, "_SetParameterToDefault@4");
	DelayWrite = (cfuncVoid) GetProcAddress((HMODULE)hLib, "_DelayWrite@0");
	Write = (cfuncVoid) GetProcAddress((HMODULE)hLib, "_Write@0");
#elif defined(VLAB_MACX)
	//cout << "Loading FileMonitor" << std::endl;
	std::string bundlePath = findBundlePath();
	size_t ind = bundlePath.find_last_of("/");
	bundlePath.resize(ind);
	bundlePath.append("/libActiveFileMonitor.dylib");
	char* lib_name = "libActiveFileMonitor.dylib"; 
    hLib = dlopen(bundlePath.c_str(), RTLD_NOW) ;
    if (hLib) { 
		MonitorFile=(cfuncStrStrStrFn) dlsym(hLib, "MonitorFile");
		SetOrGet=(cfuncStrFloat) dlsym(hLib, "SetOrGetParameter");
		Set=(cfuncVoidStrFloat) dlsym(hLib, "SetParameter");
		Get=(cfuncFloatStr) dlsym(hLib, "GetParameter");
		SetToDefault=(cfuncVoidStr) dlsym(hLib, "SetParameterToDefault");
		DelayWrite = (cfuncVoid) dlsym(hLib, "DelayWrite");
		Write = (cfuncVoid) dlsym(hLib, "Write");
    } 
    else {
		//cout << "Load Failed\n" << std::endl;
        dlclose(hLib);
    } 
#endif    
	
	if( hLib != NULL && (MonitorFile==NULL || SetOrGet == NULL || Set == NULL || SetToDefault == NULL || Get == NULL || DelayWrite == NULL || Write == NULL) ) 
	{            
		Utils::Log("Unable to load ActiveFileMonitor modules!\n");
#ifdef _WINDOWS
		FreeLibrary((HMODULE)hLib);
		hLib = NULL;
#else
		dlclose(hLib);
		hLib = NULL;
#endif
	}
}

ActiveFileMonitor::~ActiveFileMonitor()
{
	if (hLib != NULL)
	{
#ifdef _WINDOWS
		FreeLibrary((HMODULE)hLib);
		hLib = NULL;
#else
		dlclose(hLib);
		hLib = NULL;
#endif
		
	}
}

float ActiveFileMonitor::setOrGetParameter(const char *name, float defValue)
{
	if (hLib != NULL)
		return SetOrGet(name, defValue);
	else
		return defValue;
}

void ActiveFileMonitor::setParameter(const char *name, float defValue)
{
	if (hLib != NULL)
		Set(name, defValue);
}

float ActiveFileMonitor::getParameter(const char *name)
{
	if (hLib != NULL)
		return Get(name);
	else
		return 0;
}

void ActiveFileMonitor::setParameterToDefault(const char *name)
{
	if (hLib != NULL)
		SetToDefault(name);
}

void ActiveFileMonitor::delayWrite()
{
	if (hLib != NULL)
		DelayWrite();
}
void ActiveFileMonitor::write()
{
	if (hLib != NULL)
		Write();
}

void ActiveFileMonitor::monitorFile(const char* fn, const char *dir)
{
	outFn = string("");
	inFn = string("");
	
	outFn.append(fn);
	inFn.append(fn);
	
	inFn.append("in");
	outFn.append("out");
	
	if (hLib != NULL)
		MonitorFile(inFn.c_str(), outFn.c_str(), dir, parameterFileChanged );
}

string ActiveFileMonitor::getMonitoredFileName()
{
	return inFn;
}

bool ActiveFileMonitor::parametersNeedUpdating()
{
	bool ret = g_UpdateParameters;
	g_UpdateParameters = false;
	return ret;
}

