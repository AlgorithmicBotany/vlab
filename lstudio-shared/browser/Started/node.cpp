#include <locale>
#include <string>
#include <vector>

#include <fw.h>

#include <General/autovector.h>
#include <General/scalable.h>
#include <General/rgbfile.h>
#include <General/primes.h>
#include <General/bmpdata.h>

#include "ra.h"
#include "racomm.h"
#include "socket.h"
#include "connection.h"
#include "node.h"
#include "params.h"


#include "resids.h"
#include "../shstrng.h"

int VLB::Node::_counter = 0;
HBITMAP VLB::Node::_DefIcon = 0;

#ifndef USE_ICONS
HPEN VLB::Node::_hGreenPen = 0;
#endif

VLB::Node::Node(const std::string& name, const std::string& path, int type, const Node* pParent) :
_pParent(pParent),
_name(name), _path(path), _Icon(0)
{
	_type = tObject;
#ifdef USE_ICONS
	_typeIcon = Parameters::GetIcon(Parameters::iObject);
#else
	_IconType = false;
#endif
	if ((type & 1) != 0)
	{
		_type = tLink;
#ifdef USE_ICONS
		_typeIcon = Parameters::GetIcon(Parameters::iLinked);
#else
		_IconType = true;
#endif
	}
	_hasExtension = ((type & 2) != 0);
	_Expanded = false;
	_showIcon = false;
	if (0==_DefIcon)
	{
		_DefIcon = LoadBitmap(App::GetInstance(), MAKEINTRESOURCE(IDB_BITMAP1));
	}
#ifndef USE_ICONS
	if (0==_hGreenPen)
	{
		_hGreenPen = CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
	}
#endif
	++_counter;
}


VLB::Node::~Node()
{
	--_counter;
	if (0==_counter)
	{
		if (0 != _DefIcon)
		{
			DeleteObject(_DefIcon);
			_DefIcon = 0;
		}
#ifndef USE_ICONS
		if (0 != _hGreenPen)
		{
			DeleteObject(_hGreenPen);
			_hGreenPen = 0;
		}
#endif
	}
}


void VLB::Node::Measure(Canvas& cnv, bool recursive)
{
	cnv.MeasureText(_name, _txtSz);
	_TotalSize.cy = max(_txtSz.cy, Parameters::Params[Parameters::pIconY]);
	_TotalSize.cx = _txtSz.cx + 2*Parameters::Params[Parameters::pIconX];
	_LabelSize = _TotalSize;
	if (recursive)
	{
		for (Child_iter i = _aChild.begin(); i != _aChild.end(); ++i)
			i->Measure(cnv, true);
	}
}


void VLB::Node::Paint(Canvas& cnv, Canvas& mcnv, const POINT& p, const RECT* pR, const Node* pActive) const
{
	RECT mr = { 0, 0, TotalWidth(), TotalHeight() };
	int x = p.x; int y = p.y;
	OffsetRect(&mr, x, y);

	RECT dm;
	if (pR !=0 && 0==IntersectRect(&dm, &mr, pR))
		return;
	if (_showIcon)
	{
		if (0 != _Icon.Handle())
			cnv.DrawBitmap(_Icon.Handle(), x, y, _IconDim.cx, _IconDim.cy, mcnv);
		else
			cnv.DrawBitmap(_DefIcon, x, y, _IconDim.cx, _IconDim.cy, mcnv);
	}
	y -= Parameters::Params[Parameters::pIconY];
#ifdef USE_ICONS
	cnv.DrawIcon(_typeIcon, x, y);
#else
	{
		POINT pts[8];
		if (_IconType)
		{
			pts[0].x = x+5;
			pts[0].y = y+4;
			pts[1].x = x+3;
			pts[1].y = y+11;
			pts[2].x = x+9;
			pts[2].y = y+11;
			Polyline(cnv, pts, 3);
		}
		pts[0].x = x+15;
		pts[0].y = y+8;
		pts[1].x = x+11;
		pts[1].y = y+8;
		pts[2].x = x+11;
		pts[2].y = y+13;
		pts[3].x = x+1;
		pts[3].y = y+13;
		pts[4].x = x+1;
		pts[4].y = y+2;
		pts[5].x = x+11;
		pts[5].y = y+2;
		pts[6].x = x+11;
		pts[6].y = y+7;
		pts[7].x = x+16;
		pts[7].y = y+7;
		ObjectHolder sgp(cnv, _hGreenPen);
		Polyline(cnv, pts, 8);
	}
#endif	
	x += Parameters::Params[Parameters::pIconX];
	y += Parameters::Params[Parameters::pIconY];
	if (pActive == this)
		cnv.BkMode(Canvas::bkOpaque);
	cnv.TextOut(x, y, _name);
	if (pActive == this)
		cnv.BkMode(Canvas::bkTransparent);
	if (_hasExtension)
	{
		x += _txtSz.cx;
		y -= Parameters::Params[Parameters::pIconY];
#ifdef USE_ICONS
		cnv.DrawIcon(Parameters::GetIcon(Parameters::iExt), x, y);
#else
		POINT pts[8];
		pts[0].x = x;
		pts[0].y = y+7;
		pts[1].x = x+4;
		pts[1].y = y+7;
		pts[2].x = x+4;
		pts[2].y = y+3;
		pts[3].x = x+13;
		pts[3].y = y+7;
		pts[4].x = x;
		pts[4].y = y+8;
		pts[5].x = x+4;
		pts[5].y = y+8;
		pts[6].x = x+4;
		pts[6].y = y+12;
		pts[7].x = x+13;
		pts[7].y = y+8;
		DWORD pl[2] = { 4, 4 };
		ObjectHolder sgp(cnv, _hGreenPen);
		PolyPolyline(cnv, pts, pl, 2);
#endif
	}

	if (_Expanded)
	{
		x += Parameters::Params[Parameters::pIconX];
		cnv.Line
			(
			x, y+Parameters::Params[Parameters::pIconY]/2, 
			x+_VSpace(), y+Parameters::Params[Parameters::pIconY]/2
			);
		x += _VSpace();
		y += Parameters::Params[Parameters::pIconY];
		_DrawChildren(cnv, mcnv, x, y, pActive);
	}
}

int VLB::Node::_VSpace() const
{
	if (_showIcon && _LabelSize.cx<_IconDim.cx)
		return Parameters::Params[Parameters::pVspace] + _IconDim.cx - _LabelSize.cx;
	else
		return Parameters::Params[Parameters::pVspace];
}


bool VLB::Node::DrawLabel(Canvas& cnv, const VLB::Node* pNode, int x, int y, bool active) const
{
	x += Parameters::Params[Parameters::pIconX];
	if (pNode == this)
	{
		cnv.TextOut(x, y, _name);
		return true;
	}
	if (_Expanded)
	{
		x += _txtSz.cx + Parameters::Params[Parameters::pIconX];
		x += _VSpace();
		for (Child_const_iter i = _aChild.begin(); i != _aChild.end(); ++i)
		{
			if (i->DrawLabel(cnv, pNode, x, y, active))
				return true;
			y += i->YSize()+Parameters::Params[Parameters::pSpaceY];
		}
	}
	return false;
}

void VLB::Node::_DrawChildren(Canvas& cnv, Canvas& mcnv, int x, int y, const Node* pActive) const
{
	int lastY = -1;
	for (Child_const_iter i = _aChild.begin(); i != _aChild.end(); ++i)
	{
		if (i != _aChild.begin())
		{
			cnv.Line
				(
				x-Parameters::Params[Parameters::pVspace]/2, lastY, 
				x-Parameters::Params[Parameters::pVspace]/2, y-Parameters::Params[Parameters::pIconY]/2
				);
			cnv.LineTo(x, y-Parameters::Params[Parameters::pIconY]/2);
		}
		lastY = y-Parameters::Params[Parameters::pIconY]/2;
		POINT p = { x, y };
		i->Paint(cnv, mcnv, p, 0, pActive);
		y += i->YSize()+Parameters::Params[Parameters::pSpaceY];
	}
}

int VLB::Node::YSize() const
{
	return _TotalSize.cy;
}

int VLB::Node::TotalWidth() const
{ 
	return _TotalSize.cx;
}

VLB::Node* VLB::Node::Contains(int x, int y, int px, int py, eNodePart& part) const
{
	if (x<px)
		return 0;
	else if (x>px+2*Parameters::Params[Parameters::pIconX]+_txtSz.cx+_VSpace())
		return _SearchChildren(x, y, px, py, part);
	else if (y>py)
		return 0;
	else if (y<py-_txtSz.cy)
		return 0;

	if (x>px+Parameters::Params[Parameters::pIconX]+_txtSz.cx)
		part = pExtension;
	else if (x>px+Parameters::Params[Parameters::pIconX])
		part = pName;
	else
		part = pTypeIcon;
	return const_cast<Node*>(this);
}


bool VLB::Node::GetRect(const Node* pNode, POINT pt, RECT& r) const
{
	if (this == pNode)
	{
		r.left = pt.x;
		r.bottom = pt.y;
		r.right = pt.x + 2*Parameters::Params[Parameters::pIconX]+_txtSz.cx;
		if (_LabelSize.cy > Parameters::Params[Parameters::pIconY])
			r.top = pt.y-_LabelSize.cy;
		else
			r.top = pt.y - Parameters::Params[Parameters::pIconY];
		return true;
	}
	else if (_Expanded)
	{
		return _GetChildrenRect(pNode, pt, r);
	}
	else
		return false;
}

bool VLB::Node::_GetChildrenRect(const Node* pNode, POINT pt, RECT& r) const
{
	pt.x += 2*Parameters::Params[Parameters::pIconX]+_txtSz.cx+_VSpace();
	for (Child_const_iter i = _aChild.begin(); i != _aChild.end(); ++i)
	{
		if (i->GetRect(pNode, pt, r))
			return true;
		else
			pt.y += i->YSize() + Parameters::Params[Parameters::pSpaceY];
	}
	return false;
}


VLB::Node* VLB::Node::_SearchChildren(int x, int y, int px, int py, eNodePart& part) const
{
	px += 2*Parameters::Params[Parameters::pIconX]+_txtSz.cx+_VSpace();
	for (Child_const_iter i = _aChild.begin(); i != _aChild.end(); ++i)
	{
		Node* pRes = i->Contains(x, y, px, py, part);
		if (0 != pRes)
			return pRes;
		py += i->YSize() + Parameters::Params[Parameters::pSpaceY];
	}
	return 0;
}


void VLB::Node::_FreeChildren()
{
	_aChild.free();
}

void VLB::Node::RecalcTotalSize()
{
	if (_Expanded)
	{
		int cy = max(_txtSz.cy, Parameters::Params[Parameters::pIconY]);
		if (_showIcon)
			cy += _IconDim.cy;
		_TotalSize.cy = 0;
		_TotalSize.cx = 0;
		for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
		{
			it->RecalcTotalSize();
			if (it->TotalWidth()>_TotalSize.cx)
				_TotalSize.cx = it->TotalWidth();
			_TotalSize.cy += it->YSize();
		}
		_TotalSize.cx += _txtSz.cx + 2*Parameters::Params[Parameters::pIconX] + _VSpace();
		if (cy>_TotalSize.cy)
			_TotalSize.cy = cy;
	}
	else
	{
		_TotalSize.cx = _txtSz.cx + 2*Parameters::Params[Parameters::pIconX];
		_TotalSize.cy = max(_txtSz.cy, Parameters::Params[Parameters::pIconY]);
		if (_showIcon)
			_TotalSize.cy += _IconDim.cy;
	}
}


void VLB::Node::ForceExpand(Canvas& cnv, Connection* pConnect)
{
	_aChild.free();
	_ExpandChildren(cnv, pConnect, false, 0, Window(0));
	_hasExtension = true;
	_Expanded = true;
}

void VLB::Node::Expand(Canvas& cnv, Connection* pConnect)
{
	assert(!Expanded());
	ForceExpand(cnv, pConnect);
}


bool VLB::Node::ToggleExpand(Canvas& cnv, Connection* pConnect, bool recursive, LONG* pAbort, Window Path)
{
	if (!_hasExtension)
		return true;
	if (_Expanded)
	{
		_FreeChildren();
		_Expanded = false;
		return true;
	}
	else
	{
		if ((0 != pAbort) && (0 != InterlockedExchange(pAbort, 0)))
			return false;
		_Expanded = true;
		if (!_ExpandChildren(cnv, pConnect, recursive, pAbort, Path))
			return false;
		return true;
	}
}


int VLB::Node::CompareNames(const void* p1, const void* p2)
{
	typedef const std::auto_ptr<Node>* caNode;
	caNode pN1 = reinterpret_cast<caNode>(p1);
	caNode pN2 = reinterpret_cast<caNode>(p2);
	return _stricmp((*pN1)->Name().c_str(), (*pN2)->Name().c_str());
}


bool VLB::Node::_ExpandChildren(Canvas& cnv, Connection* pConnect, bool recursive, LONG* pAbort, Window Path)
{
	if (Path.IsSet())
		Path.SetText(_path);

	// Get extensions
	{
		string_buffer Extensions;
		string_buffer Names;
		std::vector<int> Types;
		if (!pConnect->GetExtensions(Names, Extensions, Types, _path.c_str()))
			throw Exception(SharedStr::GetLibString(SharedStr::strErrVlbGetExtensions));
		typedef std::vector<int>::const_iterator Citer;
		size_t ppos = Extensions.begin();
		size_t npos = Names.begin();
		for (Citer it = Types.begin(); it != Types.end(); ++it)
		{
			assert(string_buffer::npos != ppos);
			std::auto_ptr<Node> pNode(new Node(Names.string(npos), Extensions.string(ppos), *it, this));
			pNode->Measure(cnv);
			_aChild.push_back(pNode);
			ppos = Extensions.find_next(ppos);
			npos = Names.find_next(npos);
		}

		_aChild.sort(CompareNames);
	}

	// If recursively then expand
	if (recursive)
	{
		for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
		{
			if (!it->IsLink())
			{
				bool keep_expanding = it->ToggleExpand(cnv, pConnect, true, pAbort, Path);
				if (!keep_expanding)
					return false;
			}
		}
	}
	return true;
}



VLB::Node* VLB::Node::GetFirstChildNode()
{
	if (_aChild.empty())
		return 0;
	return (_aChild.front());
}

bool VLB::Node::DelSubtree(Node* pNode)
{
	assert(pNode != this);
	for (auto_vector<Node>::iterator i=_aChild.begin(); i != _aChild.end(); ++i)
	{
		if (i == pNode)
		{
			_aChild.erase(i);
			if (_aChild.empty())
			{
				_Expanded = false;
				_hasExtension = false;
			}
			return true;
		}
		else if(i->DelSubtree(pNode))
			return true;
	}
	return false;
}


bool VLB::Node::Rename(Connection* pConnect, const std::string& oofs, const std::string& name)
{
	std::string origpath = _path;
	if (pConnect->RenameObj(oofs, _path, name))
	{
		_name = name;

		for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
			it->ParentRenamed(origpath, _path);

		return true;
	}
	else
		return false;
}


void VLB::Node::ParentRenamed(const std::string& prevnm, const std::string& newnm)
{
	std::string orignm(_path);
	assert(std::string::npos != orignm.find(prevnm));

	// grow the string
	if (prevnm.length() < newnm.length())
	{
		_path.insert
			(
			static_cast<std::string::size_type>(0), 
			newnm.length()-prevnm.length(), 'x'
			);
	}
	// shrink the string
	else if (prevnm.length() > newnm.length())
	{
		_path.erase(0, prevnm.length() - newnm.length());
	}

	_path.replace(0, newnm.length(), newnm);

	for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
		it->ParentRenamed(orignm, _path);
}

bool VLB::Node::ShowIcon(bool show, Connection* pConnect, Canvas& cnv, bool recursive, LONG* pAbort, ProgressBar Progress, int w)
{
	if ((0 != pAbort) && (0 != InterlockedExchange(pAbort, 0)))
		return false;
	if (show != _showIcon)
	{
		if (show)
		{
			assert(0 == _Icon.Handle());
			HBITMAP hI = _CreateObjIcon(pConnect, cnv, _IconDim, w);
			if (0 == hI)
			{
				_IconDim.cx = Parameters::Params[Parameters::pDefObjIconCx];
				_IconDim.cy = Parameters::Params[Parameters::pDefObjIconCy];
			}
			_Icon.Set(hI);
		}
		else
		{
			DeleteObject(_Icon.Release());
		}
		_showIcon = show;
	}

	if (show && Progress.IsSet())
		Progress.Advance();

	if (recursive)
	{
		for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
		{
			if (!it->ShowIcon(show, pConnect, cnv, recursive, pAbort, Progress, w))
				return false;
		}
	}
	return true;
}

void VLB::Node::RereadIcons(Connection* pConnect, Canvas& cnv, int w)
{
	if (_showIcon && _IconDim.cx != w)
	{
		HBITMAP hI = _CreateObjIcon(pConnect, cnv, _IconDim, w);
		if (0 == hI)
		{
			_IconDim.cx = Parameters::Params[Parameters::pDefObjIconCx];
			_IconDim.cy = Parameters::Params[Parameters::pDefObjIconCy];
		}
		if (_Icon.IsSet())
			DeleteObject(_Icon.Release());
		_Icon.Set(hI);
	}
	for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
		it->RereadIcons(pConnect, cnv, w);
}

HBITMAP VLB::Node::_CreateObjIcon(Connection* pConnect, Canvas& cnv, SIZE& sz, int w)
{
	HBITMAP hRes = 0;
	try
	{
		std::string iconfilename;
		_FetchIcon(pConnect, iconfilename);
		RgbFile orig(iconfilename.c_str());
		DeleteFile(iconfilename.c_str());
		SIZE origsize;
		orig.GetSize(origsize);
		SIZE trgsize;
		trgsize.cx = w;
		float scale = static_cast<float>(trgsize.cx)/static_cast<float>(origsize.cx);
		trgsize.cy = static_cast<int>(origsize.cy*scale + 0.5f);
		BitmapData data(trgsize.cx, trgsize.cy);
		data.Scale(orig);
		BitmapMaker bmpmk(trgsize);
		hRes = bmpmk.Create(cnv, data.Buf());
		sz = trgsize;
	}
	catch (...)
	{
		return 0;
	}
	return hRes;
}


void VLB::Node::_FetchIcon(Connection* pConnect, std::string& iconfilename)
{
	bool res = pConnect->GetTmpFile(_path.c_str(), "icon", iconfilename);
	if (!res)
		throw Exception(SharedStr::GetLibString(SharedStr::strErrFetchIcon));
}


VLB::Node* VLB::Node::Find(const std::string& path) const
{
	if (_path == path)
		return const_cast<Node*>(this);
	if (IsParentOf(path))
	{
		for (Child_const_iter it = _aChild.begin(); it != _aChild.end(); ++it)
		{
			Node* pRes = it->Find(path);
			if (0 != pRes)
				return pRes;
		}
	}

	return 0;
}

VLB::Node* VLB::Node::ForceLocate(const std::string& path, Canvas& cnv, Connection* pConnect)
{
	if (_path == path)
		return this;
	if (IsParentOf(path))
	{
		if (!Expanded())
			ToggleExpand(cnv, pConnect, false, 0, Window(0));
		for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
		{
			Node* pRes = it->ForceLocate(path, cnv, pConnect);
			if (0 != pRes)
				return pRes;
		}
	}
	return 0;
}


bool VLB::Node::IsParentOf(const std::string& path) const
{
	// it seems that compare is not doing what it should do.
	//return (0==_path.compare(0, _path.length(), path));
	return (0==strncmp(path.c_str(), _path.c_str(), _path.length()));
}

void VLB::Node::AddChild(const std::string& name, const std::string& path, Canvas& cnv)
{
	std::auto_ptr<Node> pNew(new Node(name, path, 0, this));
	pNew->Measure(cnv);
	_aChild.push_back(pNew);
	_aChild.sort(CompareNames);
}


VLB::Node* VLB::Node::GetNextChild(const Node* pN) const
{
	for (Child_const_iter it = _aChild.begin(); it != _aChild.end(); ++it)
	{
		if (it == pN)
		{
			++it;
			if (it != _aChild.end())
				return it.ptr()->get();
			else
				return 0;
		}
	}
	return 0;
}


VLB::Node* VLB::Node::GetPrevChild(const Node* pN) const
{
	for (Child_const_iter it = _aChild.begin(); it != _aChild.end(); ++it)
	{
		if (it == pN)
		{
			if (it != _aChild.begin())
			{
				--it;
				return it.ptr()->get();
			}
			else
				return 0;
		}
	}
	return 0;
}


int VLB::Node::CountChildren(bool recursive) const
{
	if (!recursive)
		return _aChild.size();
	else
	{
		int res = _aChild.size();
		for (Child_const_iter it = _aChild.begin(); it != _aChild.end(); ++it)
			res += it->CountChildren(true);
		return res;
	}
}


VLB::Node* VLB::Node::FindFirstChild(char c) const
{
	for (Child_const_iter it = _aChild.begin(); it != _aChild.end(); ++it)
	{
		if (it->BeginsWith(c))
			return const_cast<Node*>(*it);
	}
	return 0;
}


bool VLB::Node::BeginsWith(char c) const
{
	c = static_cast<char>(toupper(c));
	char nc = static_cast<char>(toupper(_name[0]));
	return (c==nc);
}

