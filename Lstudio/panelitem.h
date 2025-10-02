#ifndef __PANELITEM_H__
#define __PANELITEM_H__


class PanelDesign;

class PanelItem
{
public:
	PanelItem(int, int, const char*);
	virtual ~PanelItem()
	{
		DEC_COUNTER;
	}
	virtual void Draw(Canvas&) const = 0;
	virtual void DrawInGallery(Canvas&) const = 0;
	virtual PanelItem* Clone() const = 0;
	virtual bool Contains(int, int) const = 0;
	virtual bool ContainedIn(RECT) const = 0;
	virtual void GetRect(RECT&) const = 0;
	virtual bool Hit(Canvas&, int, int, bool) = 0;
	virtual void MoveBy(int, int);
	virtual void AlignLeft(int);
	virtual void AlignRight(int);
	virtual void AlignTop(int);
	virtual void AlignBottom(int);
	virtual void HCenter(int);
	virtual void VCenter(int);
	virtual void Properties(const Window&);
	virtual void DrawSelected(Canvas&) const;

	virtual void Generate(WriteTextFile&) const = 0;
	virtual void ResetValue() {}
	virtual void DefaultValue(Canvas&) = 0;

	static int VertSortFunc(const void*, const void*);
	static int HorzSortFunc(const void*, const void*);
	void SetName(const char*);
	const char* GetName() const
	{ return _name; }
	int NameLength() const
	{ return _namelen; }
	const char* Action() const
	{ return _action; }
	void SetAction(const char*);

	POINT Origin() const
	{ return _origin; }

	PanelParameters::PanelItemType Type() const
	{
		assert(PanelParameters::pitUnknown != _type);
		return _type;
	}

	void SetDesign(PanelDesign* pDesign)
	{ _pDesign = pDesign; }

	static PanelItem* CreateFromFile(const char*, ReadTextFile&, int);
protected:
	POINT _origin;
	PanelParameters::PanelItemType _type;

	bool _Action(int, bool) const;
private:
	PanelDesign* _pDesign;
	char _name[PanelParameters::eMaxNameLength + 1];
	char _action[PanelParameters::eMaxActionLength + 1];
	int _namelen;


	DECLARE_COUNTER;
};


typedef PanelItem* pPanelItem;


class PanelSlider : public PanelItem
{
public:
	PanelSlider(int, int, const char* = "Slider");
	PanelSlider(ReadTextFile&, int);
	~PanelSlider();
	void Draw(Canvas&) const;
	void DrawInGallery(Canvas&) const;
	PanelItem* Clone() const;
	bool Contains(int, int) const;
	bool ContainedIn(RECT) const;
	void GetRect(RECT&) const;
	void Properties(const Window&);
	bool Hit(Canvas&, int, int, bool);

	void Generate(WriteTextFile&) const;
	void ResetValue()
	{ _val = _Defval; }
	void DefaultValue(Canvas&);

	int MinVal() const
	{ return _min; }
	int MaxVal() const
	{ return _max; }
	int Val() const
	{ return _Defval; }
	COLORREF Outline() const
	{ return _outline; }
	COLORREF Fill() const
	{ return _fill; }
private:
	int _min, _max, _Defval, _val;
	COLORREF _outline;
	COLORREF _fill;
};


class PanelButton : public PanelItem
{
public:
	PanelButton(int, int, const char*);
	PanelButton(ReadTextFile&, int);
	~PanelButton();
	void Draw(Canvas&) const;
	void DrawInGallery(Canvas&) const;
	PanelItem* Clone() const;
	bool Contains(int, int) const;
	bool ContainedIn(RECT) const;
	void GetRect(RECT&) const;
	void Properties(const Window&);
	bool Hit(Canvas&, int, int, bool);

	void Generate(WriteTextFile&) const;
	void ResetValue()
	{ _state = _Defstate; }
	void Reset(Canvas&);
	void DefaultValue(Canvas&);

	COLORREF Outline() const
	{ return _outline; }
	COLORREF Fill() const
	{ return _fill; }

	PanelParameters::ButtonState State() const
	{ return _Defstate; }
	static const char* GetUniqueName();
private:

	static int _counter;
	void _TurnOn(Canvas&);
	void _TurnOff(Canvas&);
	void _Pulse(Canvas&);

	bool IsOn() const
	{ return _state == PanelParameters::bsOn; }

	COLORREF _outline;
	COLORREF _fill;
	PanelParameters::ButtonState _Defstate;
	PanelParameters::ButtonState _state;
};


class PanelLabel : public PanelItem
{
public:
	PanelLabel(int, int, const char* = "Label");
	PanelLabel(ReadTextFile&, int);
	~PanelLabel();
	void Draw(Canvas&) const;
	void DrawInGallery(Canvas&) const;
	PanelItem* Clone() const;
	bool Contains(int, int) const;
	bool ContainedIn(RECT) const;
	void GetRect(RECT&) const;

	void Properties(const Window&);
	bool Hit(Canvas&, int, int, bool);
	void DefaultValue(Canvas&) {}

	COLORREF Color() const
	{ return _color; }
	void Generate(WriteTextFile&) const;
private:
	void MeasureSize();

	int _Width() const
	{ return _size.cx; }
	int _Height() const
	{ return _size.cy; }
	SIZE _size;
	COLORREF _color;
};


class PanelGroup : public PanelItem
{
public:
	PanelGroup();
	PanelGroup(ReadTextFile&);
	~PanelGroup();
	void Draw(Canvas&) const;
	void DrawInGallery(Canvas&) const;
	PanelItem* Clone() const;
	bool Contains(int, int) const;
	bool ContainedIn(RECT) const;
	void GetRect(RECT&) const;
	bool Hit(Canvas&, int, int, bool);
	void DefaultValue(Canvas&) {}

	void Generate(WriteTextFile&) const;
	void AddButton(const PanelItem*);
	bool CanAddButton() const
	{ return _items<PanelParameters::eMaxButtonsInGroup; }
	bool IsEmpty() const
	{ return 0==_items; }
	void Properties(const Window&);

	COLORREF Color() const
	{ return _color; }

	// return false if it cannot find the buttons belonging to the group
	// such a group should be deleted
	bool UpdateRect(const pPanelItem*, int);
	bool ContainsButton(const char*) const;
	int Items() const
	{ return _items; }
	const char* ButtonName(int i) const
	{
		assert(i>=0);
		assert(i<Items());
		return _buttons[i];
	}

private:
	int _FindButton(const char*) const;
	void _DeleteName(int);
	RECT _rect;
	char _buttons[PanelParameters::eMaxButtonsInGroup][PanelParameters::eMaxNameLength + 1];
	int _items;
	COLORREF _color;
};

#else
	#error File already included
#endif
