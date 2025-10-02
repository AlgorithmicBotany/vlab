#ifndef __NODE_H__
#define __NODE_H__


#include <General/autovector.h>
#include "tGuid.h"

namespace VLB
{

class Node
{
public:
	enum eNodeType
	{
		tObject,
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

	Node(const char*, const char*, int, const Node*, const char* = NULL);
	~Node();
	void Paint(Canvas&, Canvas&, const POINT&, const RECT*, const Node* pActive) const;
	void Measure(Canvas&, bool = false);
	Node* Contains(int, int, int, int, eNodePart&) const;
	bool HasExtensions() const
	{ return _hasExtension; }
	const char* Name() const
	{ return _name.c_str(); }
	const char* Path() const
	{ return _path.c_str(); }
	tGUID& UUId()
	{ return _uuid; }
	bool ToggleExpand(Canvas&, Connection*, bool, LONG*, Window);
	void Expand(Canvas&, Connection*);
	bool Rename(Connection*, const char* /*oofs*/, const char* /*name*/);
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
	Node* Find(const char*) const; 
	Node* ForceLocate(const std::string&, Canvas&, Connection*);
	void AddChild(const char*, Canvas&, Connection*);
	bool Expanded() const
	{ return _Expanded; }
	void ForceExpand(Canvas&, Connection*);
	int CountChildren(bool) const;
	bool IsParentOf(const std::string&) const;

	// Return true if drawn, otherwise continue drawing children
	bool DrawLabel(Canvas&, const Node*, int, int, bool) const;
	bool IsLink() const
	{ return _type == tLink; }
	bool IsHyperobject() const
	{ return _type == tHyperobject; }
	bool BeginsWith(char c) const;
	const Node* GetFirstChildNode() const;
	Node* GetFirstChildNode();
	const Node* GetParent() const
	{ return _pParent; }
	Node* GetNextChild(const Node*) const;
	Node* GetPrevChild(const Node*) const;
	Node* FindFirstChild(char) const;

	bool SwapChildren(const Node*,const Node*);

	void ParentRenamed(const std::string&, const std::string&);
	bool GetRect(const Node*, POINT, RECT&) const;

	void SetHyperobject(const tGUID& guid, const std::string& targetPath);
	const std::string& GetTargetPath() const
	{ return _targetPath; }

	static int CompareNames(const void*, const void*);
private:

	bool _GetChildrenRect(const Node*, POINT, RECT&) const;
	HBITMAP _CreateObjIcon(Connection*, Canvas&, SIZE&, int);
	void _FetchIcon(Connection*, std::string&);
	void DrawIcon(Canvas& canvas, int x, int y) const;
	void _DrawChildren(Canvas&, Canvas&, int, int, const Node*) const;
	void _FreeChildren();
	bool _ExpandChildren(Canvas&, Connection*, bool, LONG*, Window);
	void _SortChildren(Connection*);
	Node* _SearchChildren(int, int, int, int, eNodePart&) const;
	int _VSpace() const;

	void AddChild(Connection* pConnect, int oType, const char* szName, const char* szPath, Canvas& cnv);

	const Node* _pParent;
	std::string _name;
	std::string _path;
	std::string _targetPath;
	std::string _oofsroot;

	eNodeType _type;
	tGUID _uuid;
	eNodeType _IconType;

	SIZE _txtSz;
	SIZE _LabelSize;
	SIZE _TotalSize;
	SIZE _IconDim;
	bool _hasExtension;
	bool _Expanded;
	bool _showIcon;
	bool _bHyperBroken;
	Bitmap _Icon;
	auto_vector<Node> _aChild;
	typedef auto_vector<Node>::const_iterator Child_const_iter;
	typedef auto_vector<Node>::iterator Child_iter;
	static HBITMAP _DefIcon;
	static HPEN _hGreenPen;
	static HPEN _hRedPen;
	static int _counter;
};


}


#else
	#error File already included
#endif
