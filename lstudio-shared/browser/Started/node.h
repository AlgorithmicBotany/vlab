#ifndef __NODE_H__
#define __NODE_H__


//#define USE_ICONS

namespace VLB
{

class Node
{
public:
	enum eNodeType
	{
		tObject,
		tObjectExtension,
		tHyperobjectExtension,
		tHyperobject,
		tLink,
		tBrokenLink
	};
	enum eNodePart
	{
		pName,
		pExtension,
		pTypeIcon,
		pNone
	};

	Node(const std::string&, const std::string&, int, const Node*);
	~Node();
	void Paint(Canvas&, Canvas&, const POINT&, const RECT*, const Node* pActive) const;
	void Measure(Canvas&, bool = false);
	Node* Contains(int, int, int, int, eNodePart&) const;
	bool HasExtensions() const
	{ return _hasExtension; }
	const std::string& Path() const
	{ return _path; }
	bool ToggleExpand(Canvas&, Connection*, bool, LONG*, Window);
	void Expand(Canvas&, Connection*);
	bool Rename(Connection*, const std::string& /*oofs*/, const std::string& /*name*/);
	void RereadIcons(Connection*, Canvas&, int);
	bool ShowIcon(bool /*on|off*/, Connection*, Canvas&, bool /*recursive*/, LONG* /*abort*/, ProgressBar /*hProgress*/, int /*width*/);
	bool HasIcon() const
	{ return _showIcon; }
	int YSize() const;
	int TotalHeight() const
	{ return YSize(); }
	int TotalWidth() const;
	void RecalcTotalSize();
	bool DelSubtree(Node*);
	Node* Find(const std::string&) const; 
	Node* ForceLocate(const std::string&, Canvas&, Connection*);
	void AddChild(const std::string&, const std::string&, Canvas&);
	bool Expanded() const
	{ return _Expanded; }
	void ForceExpand(Canvas&, Connection*);
	int CountChildren(bool) const;
	bool IsParentOf(const std::string&) const;

	// Return true if drawn, otherwise continue drawing children
	bool DrawLabel(Canvas&, const Node*, int, int, bool) const;
	bool IsLink() const
	{ return _type == tLink; }
	const std::string& Name() const
	{ return _name; }
	bool BeginsWith(char c) const;
	Node* GetFirstChildNode();
	const Node* GetParent() const
	{ return _pParent; }
	Node* GetNextChild(const Node*) const;
	Node* GetPrevChild(const Node*) const;
	Node* FindFirstChild(char) const;

	void ParentRenamed(const std::string&, const std::string&);
	bool GetRect(const Node*, POINT, RECT&) const;

	static int CompareNames(const void*, const void*);
private:

	bool _GetChildrenRect(const Node*, POINT, RECT&) const;
	HBITMAP _CreateObjIcon(Connection*, Canvas&, SIZE&, int);
	void _FetchIcon(Connection*, std::string&);
	void _DrawChildren(Canvas&, Canvas&, int, int, const Node*) const;
	void _FreeChildren();
	bool _ExpandChildren(Canvas&, Connection*, bool, LONG*, Window);
	Node* _SearchChildren(int, int, int, int, eNodePart&) const;
	int _VSpace() const;

	const Node* _pParent;
	std::string _name;
	std::string _path;
	eNodeType _type;
#ifdef USE_ICONS
	HICON _typeIcon;
#else
	bool _IconType;
#endif
	SIZE _txtSz;
	SIZE _LabelSize;
	SIZE _TotalSize;
	SIZE _IconDim;
	bool _hasExtension;
	bool _Expanded;
	bool _showIcon;
	Bitmap _Icon;
	auto_vector<Node> _aChild;
	typedef auto_vector<Node>::const_iterator Child_const_iter;
	typedef auto_vector<Node>::iterator Child_iter;
	static HBITMAP _DefIcon;
#ifndef USE_ICONS
	static HPEN _hGreenPen;
#endif
	static int _counter;
};


}


#else
	#error File already included
#endif
