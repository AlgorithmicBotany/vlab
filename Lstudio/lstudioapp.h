#ifndef __LSTUDIOAPP_H__
#define __LSTUDIOAPP_H__


#include "comlineparam.h"
class LStudioOptions;

class LStudioApp : public MDIApp
{
public:
	LStudioApp(HINSTANCE);
	~LStudioApp();

	const CommandLineParams& GetCommandLineParams() const
	{ return _Clp; }
	void Create();
	void LaunchMainWindow();
	HMENU GetProjectMenu() const
	{ return _GetMDIMenus().GetMenu(eProjectMenu); }
	const ImageList& GetTabsList() const
	{ return _TabsList; }
	HMENU GetDefaultMenu() const
	{ return _GetMDIMenus().GetMenu(eDefaultMenu); }
	int GetDefaultHelpMenuPosition() const;
	int GetProjectHelpMenuPosition() const;
	static UINT HelpMenuItemBaseID();
	void UpdateHelpMenu(MenuManipulator& helpMenu, const LStudioOptions& options);
	bool ExecuteShellCommand(const std::string& command) const;
	bool ExecuteCommand(const std::string& command) const;

	HMENU GetSimulatorMenu(int iWhich) const
	{ return _simulatorMenus.SubMenu(iWhich); }
private:

	enum eMDIMenus
	{
		eDefaultMenu = 0,
		eProjectMenu = 1
	};

	void _CleanTmp();
	void _RegisterClasses();
	ImageList _TabsList;
	DynLib _RichEdDll;
	void _CheckEval();

	CommandLineParams _Clp;
	Menu _simulatorMenus;
};


#else
	#error File already included
#endif
