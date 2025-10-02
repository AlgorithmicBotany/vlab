#ifndef __SHELLSHORTCUT_H__
#define __SHELLSHORTCUT_H__


class Shortcut
{
public:
	Shortcut(const std::string& shortcutFilename);
	bool Resolve(std::string& targetFilename, UINT errMsg);

private:
	std::string _ShortcutFilename;
};


#endif
