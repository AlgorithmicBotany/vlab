#include <fw.h>
#include <glfw.h>

#include "lstudioapp.h"


int RunTests();

int Main(HINSTANCE hInst, Window::sw)
{
	if (__argc>1 && strcmp(__argv[1], "--doTests")==0)
	{
		if (RunTests())
			return 0;
		else
			return -1;
	}
#ifdef DEBUGLOG
	LogFile::theLog.Open("c:\\lstudio.log");
#endif
	try
	{
		LStudioApp app(hInst);
		app.Create();
		return app.Execute();
	}
	catch (Exception e)
	{
		MessageBox(0, e.Msg(), "Error", MB_ICONSTOP);
	}
	catch (...)
	{
		MessageBox(0, __TEXT("Unknown error\nExiting"), __TEXT("Fatal error"), MB_ICONERROR);
	}
	return 0;
}
