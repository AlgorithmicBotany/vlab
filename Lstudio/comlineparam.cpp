#include <fw.h>

#include "comlineparam.h"

#include "resource.h"

CommandLineParams::CommandLineParams()
{
	_InitialProject.erase();
	_InitialRect.left =
		_InitialRect.right =
		_InitialRect.top =
		_InitialRect.bottom = CW_USEDEFAULT;
	_flags.Set(ftDemoMode, false);
	_flags.Set(ftSkipSplash, false);
}


void CommandLineParams::Parse()
{
	int n = 1;
	POINT sz = { -1, -1 };
	POINT pos = { -1, -1 };
	POINT scr;
	scr.x = GetSystemMetrics(SM_CXSCREEN);
	scr.y = GetSystemMetrics(SM_CYSCREEN);
	while (n<__argc)
	{
		if (!strcmp(__argv[n], "-spl"))
			_flags.Set(ftSkipSplash, true);
		else if (!strcmp(__argv[n], "-w"))
		{
			++n;
			if (n<__argc)
				sz.x = atoi(__argv[n]);
			else 
				throw Exception(IDERR_DASHWINCOMPLETE);
			++n;
			if (n<__argc)
				sz.y = atoi(__argv[n]);
			else
				throw Exception(IDERR_DASHWINCOMPLETE);
		}
		else if (!strcmp(__argv[n], "-wp"))
		{
			++n;
			if (n<__argc)
				pos.x = atoi(__argv[n]);
			else
				throw Exception(IDERR_DASHWPINCOMPLETE);
			++n;
			if (n<__argc)
				pos.y = atoi(__argv[n]);
			else
				throw Exception(IDERR_DASHWPINCOMPLETE);
		}
		else if (!strcmp(__argv[n], "-wr"))
		{
			float x=0.0f, y=0.0f;
			++n;
			if (n<__argc)
				x = static_cast<float>(atof(__argv[n]));
			else
				throw Exception(IDERR_DASHWRINCOMPLETE);
			++n;
			if (n<__argc)
			{
				y = static_cast<float>(atof(__argv[n]));
				sz.x = static_cast<int>(x*scr.x);
				sz.y = static_cast<int>(y*scr.y);
			}
			else
				throw Exception(IDERR_DASHWRINCOMPLETE);
		}
		else if (!strcmp(__argv[n], "-wpr"))
		{
			float x=0.0f, y=0.0f;
			++n;
			if (n<__argc)
				x = static_cast<float>(atof(__argv[n]));
			else
				throw Exception(IDERR_DASHWPRINCOMPLETE);
			++n;
			if (n<__argc)
			{
				y = static_cast<float>(atof(__argv[n]));
				pos.x = static_cast<int>(x*scr.x);
				pos.y = static_cast<int>(y*scr.y);
			}
			else
				throw Exception(IDERR_DASHWPRINCOMPLETE);
		}
		else if (!(strcmp(__argv[n], "-demo")))
			_flags.Set(ftDemoMode, true);
		else if (!(strcmp(__argv[n], "-smn")))
			_flags.Set(ftShowMinimized, true);
		else
			_InitialProject.assign(__argv[n]);
		++n;
	}

	if (pos.x != -1)
	{
		_InitialRect.left = pos.x;
		_InitialRect.top = pos.y;
	}
	if (sz.x != -1)
	{
		if (_InitialRect.left == CW_USEDEFAULT)
		{
			_InitialRect.left = 0;
			_InitialRect.top = 0;
		}
		_InitialRect.right = _InitialRect.left+sz.x;
		_InitialRect.bottom = _InitialRect.top+sz.y;
	}
}

