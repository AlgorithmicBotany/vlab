#ifndef __MENU_H__
#define __MENU_H__

#include <vector>
#include <string>

class Menu
{
public:
	Menu() { _hMenu = 0; }
	Menu(UINT);
	~Menu()
	{ 
		if (IsSet())
			DestroyMenu(_hMenu); 
	}

	HMENU Handle() const
	{ return _hMenu; }

	HMENU Release()
	{
		HMENU hRes = _hMenu;
		_hMenu = 0;
		return hRes;
	}
	void Load(UINT);
	bool IsSet() const
	{ return 0 != _hMenu; }
	HMENU SubMenu(int id) const
	{ 
		assert(IsSet());
		return ::GetSubMenu(_hMenu, id); 
	}

	void Swap(Menu& menu)
	{
		HMENU hTemp = _hMenu;
		_hMenu = menu._hMenu;
		menu._hMenu = hTemp;
	}
private:
	HMENU _hMenu;
};


class MenuManipulator
{
public:

	enum eParam
	{
		kInvalidId = -1
	};

	explicit MenuManipulator(HMENU hMenu) : _hMenu(hMenu) {}
	explicit MenuManipulator(HWND hWnd) : _hMenu(GetMenu(hWnd)) {}
	void AppendCommand(int, UINT, const std::string&);
	void AppendSeparator();
	void Enable(UINT, bool bEnable = true);
	void Disable(UINT);
	void Check(UINT);
	void Uncheck(UINT);
	void SetCheck(UINT id, bool ch)
	{ 
		if (ch)
			Check(id);
		else
			Uncheck(id);
	}
	void CheckRadio(UINT, UINT, UINT);
	void SetText(UINT, const std::string&);
	void SetTextBP(UINT, const std::string&);
	void SetBitmaps(UINT id, HBITMAP hChk, HBITMAP hUnchk)
	{ ::SetMenuItemBitmaps(_hMenu, id, MF_BYCOMMAND, hChk, hUnchk); }
	void SetOwnerDraw(UINT);
	HMENU Handle() const
	{ return _hMenu; }
	UINT FindItem(const std::string&) const;
	UINT FindItem(UINT uID) const;
	int ItemCount() const
	{ return ::GetMenuItemCount(_hMenu); }
	HMENU GetSubMenu(UINT iSubMenu) const
	{ return ::GetSubMenu(_hMenu, iSubMenu); }
	bool DeleteItem(UINT iItem)
	{ 
		BOOL res = ::DeleteMenu(_hMenu, iItem, MF_BYPOSITION); 
		DWORD err = ::GetLastError();
		return (res != 0) && (err == 0);
	}

	void InsertMenu(const MenuManipulator& newMenu, UINT position, const std::string& label)
	{
		::InsertMenu(
			_hMenu, position, 
			MF_BYPOSITION | MF_POPUP | MF_STRING, 
			reinterpret_cast<UINT_PTR>(newMenu.Handle()), 
			label.c_str());
	}
	void InsertCommand(UINT position, UINT commandId, const std::string& label);

	static const UINT nPos;


	struct tMenuBuildData
	{
		std::string szLabel;
		UINT uiCommandId;
		static const char* kSeparatorTag;
		static const char* kLastItemTag;
	};

	class MIInfo
	{
	public:
		enum Type
		{
			OwnerDraw = MFT_OWNERDRAW,
			String = MFT_STRING
		};
		MIInfo();
		const MENUITEMINFO* Info() const
		{ return &_info; }
		bool IsString() const
		{ return 0 != (MFT_STRING & _info.fType); }
		MENUITEMINFO* Info() 
		{ return &_info; }
		void SetType(Type t);
		void SetTypeData(char* pV)
		{ _info.dwTypeData = pV; }
		void SetCount(int c)
		{ _info.cch = c; }
		UINT ID() const
		{ return _info.wID; }
		void SetID(UINT id)
		{ _info.wID = id; }
		void SetMask(UINT uiMask)
		{
			_info.fMask = uiMask;
		}
		void AddMask(UINT uiMask)
		{
			_info.fMask |= uiMask;
		}
		bool IsCommand() const
		{
			return (_info.fMask & (MFT_SEPARATOR)) == 0 && _info.hSubMenu == NULL;
		}
		bool IsPopup() const
		{
			return (_info.hSubMenu != NULL);
		}
		void SetPopup(HMENU hPopup)
		{
			_info.hSubMenu = hPopup;
			if (_info.hSubMenu != NULL)
			{
				_info.fMask |= MIIM_SUBMENU;
			}
			else
			{
				_info.fMask &= ~MIIM_SUBMENU;
			}
		}
		void SetLabel(const TCHAR* szLabel)
		{
			_info.dwTypeData = const_cast<TCHAR*>(szLabel);
			if (NULL != _info.dwTypeData)
			{
				_info.fMask |= MIIM_STRING;
			}
			else
			{
				_info.fMask &= ~MIIM_STRING;
			}
		}
	private:
		MENUITEMINFO _info;
	};

	void GetMenuItemInfo(UINT uiItem, MIInfo& mii)
	{ _GetMII(uiItem, mii, true); }

	// this version requires that the last item in aBuildData 
	// contains label equal to tMenuBuildData::kLastItemTag
	void BuildMenu(const tMenuBuildData* aBuildData);

	// this version does not require the last item to be
	// tMenuBuildData::kLastItemTag but if it is included it will not be added to the menu
	void BuildMenu(const std::vector<tMenuBuildData>& aBuildData);

private:
	void _SetMII(UINT id, const MIInfo& mii, bool byposition) const
	{ ::SetMenuItemInfo(_hMenu, id, byposition ? TRUE : FALSE, mii.Info()); }
	bool _GetMII(UINT id, MIInfo& mii, bool byposition) const
	{ 
		BOOL res = ::GetMenuItemInfo(_hMenu, id, byposition ? TRUE : FALSE, mii.Info()); 
		DWORD err = ::GetLastError();
		return (0!=res && err == 0);
	}
	HMENU _hMenu;
};


#endif
