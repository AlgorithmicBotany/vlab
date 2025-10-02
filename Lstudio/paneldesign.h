#ifndef __PANELDESIGN_H__
#define __PANELDESIGN_H__


class PrjNotifySink;
class PanelItemsSelection;
class PanelItem;
typedef PanelItem* pPanelItem;

class PanelDesign : public EditableObject
{
public:

	PanelDesign();
	~PanelDesign();
	void AddSlider(int, int);
	void AddButton(int, int);
	void AddLabel(int, int);
	void AddGroup(POINT, POINT);

	RECT ExecuteHit(Canvas&, int, int);
	bool ExecuteDrag(Canvas&, int, int, bool final = false);

	void Copy(const EditableObject*);
	EditableObject* Clone() const;

	void Reset();
	void Draw(Canvas&) const;
	void DrawInGallery(int, int, Canvas&) const;
	void DrawInGallery() const {}
	DWORD ClipboardSize() const;
	char* CopyToClipboard(char*) const;
	const char* LoadFromClipboard(const char*);

	void SetName(const char*);
	const char* GetName() const
	{ return _name; }
	PanelItem* GetItemAt(int, int);
	static int GetMaxItems()
	{ return PanelParameters::eMaxItems; }
	bool CanAddItems(int i=1) const
	{ return _items+i<=PanelParameters::eMaxItems; }
	PanelItem* Duplicate(const PanelItem*, int dx, int dy);

	int FindItem(const PanelItem*) const;
	int Items() const
	{ return _items; }
	void DeleteItem(int);
	const PanelItem* GetItem(int i) const
	{
		assert(i>=0);
		assert(i<_items);
		return _arr[i];
	}

	void GetRect(RECT& r) const
	{ r = _rect; }
	void UpdateRect();
	int GetWidth() const
	{ return _rect.right; }
	int GetHeight() const
	{ return _rect.bottom; }

	bool SelectArea(RECT, PanelItemsSelection&) const;  // true if anything selected
	void Generate(WriteTextFile&) const;
	void Import(ReadTextFile&);

	void Execute(HWND);
	void PanelClosed();

	PanelParameters::PanelTarget PanelTarget() const
	{ return _target; }
	const char* PanelTargetFilename() const
	{
		assert(PanelParameters::ptFile == _target);
		return _targetfile;
	}
	PanelParameters::TriggerCommand Trigger() const
	{ return _trigger; }
	void SetTrigger(PanelParameters::TriggerCommand trigger)
	{ 
		_trigger = trigger; 
		if (!DefaultTrigger() && _version < 101)
			_version = 101;
	}
	void SetPanelTarget(PanelParameters::PanelTarget target)
	{ _target = target; }
	void SetPanelTargetFilename(const char*);
	PrjNotifySink* Project() const
	{ 
		assert(0 != _pNotifySink);
		return _pNotifySink;
	}
	void SetNotifySink(PrjNotifySink*);

	bool Action(const char*, bool);
	void ResetValues();
	void DefaultValues(Canvas&);
	bool IsTearedOff() const
	{ return 0 != _hView; }
	void CloseTearedOff();

	static const char* DefaultName()
	{ return "unnamed"; }

	bool NewLsystem(bool);
	bool NewView(bool);
	bool NewModel(bool);
	bool Rerun(bool);

	const std::string& GetDirectory() const
	{ return _Directory; }

	bool IsNamed(const char*) const;

private:

	enum 
	{ 
		eMaxNameLength = 32
	};


	void Generate0099(WriteTextFile&) const;
	void Generate0101(WriteTextFile&) const;
	void Import0099(ReadTextFile&, std::string&);
	void Import0101(ReadTextFile&);

	bool DefaultTrigger() const;
	void _AddItem(PanelItem*);
	void _Empty();
	int _ParentGroup(const char*) const;
	void _ResetGroup(Canvas&, int);
	int _FindItem(PanelParameters::PanelItemType, const char*) const;

	pPanelItem _arr[PanelParameters::eMaxItems];
	int _items;

	char _name[eMaxNameLength+1];

	PanelParameters::PanelTarget _target;
	PanelParameters::TriggerCommand _trigger;
	char _targetfile[_MAX_PATH+1];
	PrjNotifySink* _pNotifySink;

	RECT _rect;

	HWND _hView;
	std::string _Directory;
	int _version;
};

#else
	#error File already included
#endif
