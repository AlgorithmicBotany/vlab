#include <locale>
#include <string>
#include <vector>

#include <fw.h>

#include <General/scalable.h>
#include <General/rgbfile.h>
#include <General/primes.h>
#include <General/bmpdata.h>

#include <RAconsts.h>
#include "racomm.h"
#include "socket.h"
#include "connection.h"
#include "node.h"
#include "params.h"


#include "resids.h"
#include "../shstrng.h"

int VLB::Node::_counter = 0;
HBITMAP VLB::Node::_DefIcon = 0;

HPEN VLB::Node::_hGreenPen = 0;
HPEN VLB::Node::_hRedPen = 0;

VLB::Node::Node(const char* name, const char* path, int type, const Node* pParent, const char* oofsroot) :
_pParent(pParent),
_name(name), _path(path), /*_id(0),*/ _Icon(0)
{
	if(oofsroot == NULL)
	{
		if(pParent == NULL)
			_oofsroot = _path;
		else
			_oofsroot = pParent->_oofsroot;
	}
	else // oofsroot != NULL
		_oofsroot = std::string(oofsroot);

	_type = tObject;
	_IconType = tObject;
	if ((type & Connection::tRALink) != 0)
	{
		_type = tLink;
		_IconType = tLink;
	}

	_hasExtension = ((type & Connection::tRAHasExtensions) != 0);
	_Expanded = false;
	_showIcon = false;
	_bHyperBroken = false;
	if (0==_DefIcon)
	{
		_DefIcon = LoadBitmap(App::GetInstance(), MAKEINTRESOURCE(IDB_BITMAP1));
	}

	if (0==_hGreenPen)
	{
		_hGreenPen = CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
	}

	if (0==_hRedPen)
	{
		_hRedPen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
	}

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
		if (0 != _hGreenPen)
		{
			DeleteObject(_hGreenPen);
			_hGreenPen = 0;
		}
		if (0 != _hRedPen)
		{
			DeleteObject(_hRedPen);
			_hRedPen = 0;
		}
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



void VLB::Node::DrawIcon(Canvas& cnv, int x, int y) const
{
	POINT pts[8];
	if (_IconType == tLink)
	// yellow "L"
	{
		pts[0].x = x+5;
		pts[0].y = y+4;
		pts[1].x = x+3;
		pts[1].y = y+11;
		pts[2].x = x+9;
		pts[2].y = y+11;
		Polyline(cnv, pts, 3);
	}
	if(_IconType == tLink || _IconType == tObject)
	// green box
	{
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
	else if(_IconType == tHyperobject && _bHyperBroken)
	{
		ObjectHolder srp(cnv, _hRedPen);
		pts[0].x = x+15;
		pts[0].y = y+8;
		pts[1].x = x+11;
		pts[1].y = y+8;
		pts[2].x = x+1;
		pts[2].y = y+13;
		pts[3].x = x+1;
		pts[3].y = y+2;
		pts[4].x = x+11;
		pts[4].y = y+7;
		pts[5].x = x+16;
		pts[5].y = y+7;
		Polyline(cnv, pts, 6);
	}
	else if (_IconType == tHyperobject)
	// yellow arrow
	{
		pts[0].x = x+15;
		pts[0].y = y+8;
		pts[1].x = x+11;
		pts[1].y = y+8;
		pts[2].x = x+1;
		pts[2].y = y+13;
		pts[3].x = x+1;
		pts[3].y = y+2;
		pts[4].x = x+11;
		pts[4].y = y+7;
		pts[5].x = x+16;
		pts[5].y = y+7;
		Polyline(cnv, pts, 6);
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
	DrawIcon(cnv, x, y);

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
	typedef const std::unique_ptr<Node>* caNode;
	caNode pN1 = reinterpret_cast<caNode>(p1);
	caNode pN2 = reinterpret_cast<caNode>(p2);
	return _stricmp((*pN1)->Name(), (*pN2)->Name());
}


void VLB::Node::_SortChildren(Connection* pConnect)
{
	_aChild.sort(CompareNames);

	std::vector<std::string> order;
	if(!pConnect->GetOrdering(_path.c_str(), order) || order.empty())
		return;

	std::vector<size_t> redirect(_aChild.size());
	std::vector<std::string> names(_aChild.size());
	for(size_t i = 0 ; i < names.size() ; i++)
	{
		size_t pos = _aChild.at(i)->_path.find_last_of(pConnect->PathSeparator());
		if(pos != std::string::npos)
			names[i] = _aChild.at(i)->_path.substr(pos+1);
		else
			names[i] = _aChild.at(i)->_path;
	}

	size_t idx = 0;

	// First, everything from the order
	for(size_t i = 0 ; i < order.size() ; i++)
	{
		for(size_t j = 0 ; j < _aChild.size() ; j++)
		{
			if(order[i] == names[j])
			{
				redirect[idx++] = j;
				names[j].clear();
				break;
			}
		}
	}

	// Then everything that hasn't already been placed
	for(size_t rdx = 0 ; idx < redirect.size() ; )
	{
		while(names[rdx].empty()) rdx++;
		redirect[idx++] = rdx++;
	}

	// Now move the elements around
	std::vector<size_t> redirectInv(redirect.size());
	for(size_t i = 0 ; i < redirect.size() ; i++)
		redirectInv[i] = i;
	for(size_t i = 0 ; i < redirect.size() ; i++)
	{
		if(redirect[i] == redirectInv[i])
			continue;

		size_t j;
		for(j = i+1 ; j < redirect.size() ; j++)
			if(redirectInv[j] == redirect[i]) break;
		assert(j < redirect.size());
		_aChild.swap(i,j);
		redirectInv[j] = redirectInv[i];
	}
}

bool VLB::Node::_ExpandChildren(Canvas& cnv, Connection* pConnect, bool recursive, LONG* pAbort, Window Path)
{
	if (Path.IsSet())
		Path.SetText(_path);

	// Get extensions
	{
		string_buffer Paths;
		string_buffer Names;
		std::vector<int> Types;
		
		if (!pConnect->GetExtensions(Names, Paths, Types, _path.c_str()))
		{
			throw Exception(SharedStr::GetLibString(SharedStr::strErrVlbGetExtensions));
		}
		typedef std::vector<int>::const_iterator Citer;
		size_t ppos = Paths.begin();
		size_t npos = Names.begin();
		for (Citer it = Types.begin(); it != Types.end(); ++it)
		{
			assert(string_buffer::npos != ppos);
			AddChild(pConnect, *it, Names.string(npos), Paths.string(ppos), cnv);
			ppos = Paths.find_next(ppos);
			npos = Names.find_next(npos);
		}

	}

	_SortChildren(pConnect);

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


void VLB::Node::AddChild(Connection* pConnect, int objType, const char* szName, const char* szPath, Canvas& cnv)
{
	std::string nodeName;
	bool bIsHyperobject = false;
	tGUID guid;
	std::string realPath;
	if(objType & Connection::tRALink)
	{
		// If it's a symlink, we use the link name, not the path
		nodeName = std::string(szName);
	}
	else if (pConnect->IsHyperobject(szPath))
	{
		//std::string name;
		bIsHyperobject = true;
		pConnect->GetHyperobjectInfo(_oofsroot, szPath, guid, realPath, nodeName);
	}
			
	if (nodeName.empty())
	{
		pConnect->GetObjectName(szPath, _oofsroot, nodeName);
	}

	std::unique_ptr<Node> pNode(new Node(nodeName.c_str(), szPath, objType, this));
	if(bIsHyperobject)
	{
		pNode->SetHyperobject(guid, realPath);
	}
	pNode->Measure(cnv);
	_aChild.push_back(pNode);
}


const VLB::Node* VLB::Node::GetFirstChildNode() const
{
	if (_aChild.empty())
		return 0;
	return (_aChild.front());
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


bool VLB::Node::Rename(Connection* pConnect, const char* oofs, const char* name)
{
	std::string origpath = _path;
	if (pConnect->RenameObj(oofs, _path, name))
	{
		if(!IsHyperobject())
		{
			size_t pos = _path.rfind(_name);
			_path = _path.substr(0,pos) + name;
		}
		_name = name;

		if(!IsHyperobject())
		{
			for (Child_iter it = _aChild.begin(); it != _aChild.end(); ++it)
			{
				it->ParentRenamed(origpath, _path);
			}
		}

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
	std::string iconfilename;
	try
	{
		_FetchIcon(pConnect, iconfilename);
		RgbFile orig(iconfilename.c_str());
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
		::DeleteFile(iconfilename.c_str());
		return 0;
	}
	::DeleteFile(iconfilename.c_str());
	return hRes;
}


void VLB::Node::_FetchIcon(Connection* pConnect, std::string& iconfilename)
{
	std::string sourcePath;
	if(IsHyperobject())
	{
		if(!pConnect->LookupPath(_uuid,_oofsroot.c_str(),sourcePath))
		{
			throw Exception(SharedStr::GetLibString(SharedStr::strErrDeadHyperlink), _uuid);
		}
	}
	else // normal object
	{
		sourcePath = _path;
	}

	bool res = pConnect->GetTmpFile(sourcePath.c_str(), "icon", iconfilename);
	if (!res)
	{
		throw Exception(SharedStr::GetLibString(SharedStr::strErrFetchIcon));
	}
}


VLB::Node* VLB::Node::Find(const char* path) const
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

void VLB::Node::AddChild(const char* name, Canvas& cnv, Connection* pConnect)
{
	std::string Name(name);
	string_buffer Paths;
	string_buffer Names;
	std::vector<int> Types;
	if (!pConnect->GetExtensions(Names, Paths, Types, _path.c_str()))
		throw Exception(SharedStr::GetLibString(SharedStr::strErrVlbGetExtensions));
	typedef std::vector<int>::const_iterator Citer;
	size_t ppos = Paths.begin();
	size_t npos = Names.begin();
	for (Citer it = Types.begin(); it != Types.end(); ++it)
	{
		if (0==Name.compare(Names.string(npos)))
		{
			AddChild(pConnect, *it, Names.string(npos), Paths.string(ppos), cnv);
			break;
		}
		ppos = Paths.find_next(ppos);
		npos = Names.find_next(npos);
	}
	_SortChildren(pConnect);
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

bool VLB::Node::SwapChildren(const Node* pN1,const Node* pN2)
{
	size_t i,j;
	for(i = 0 ; i < _aChild.size() ; i++)
		if(_aChild.at(i) == pN1) break;
	for(j = 0 ; j < _aChild.size() ; j++)
		if(_aChild.at(j) == pN2) break;
	if(i >= _aChild.size() || j >= _aChild.size())
		return false;

	_aChild.swap(i,j);
	return true;
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


void VLB::Node::SetHyperobject(const tGUID& guid, const std::string& targetPath)
{
	_uuid = guid;
	_targetPath = targetPath;
	_type = tHyperobject;
	_IconType = tHyperobject;
	if (_targetPath.empty())
	{
		_bHyperBroken = true;
	}
}
