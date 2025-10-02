#include <memory>

#include <fw.h>

#include "prjnotifysnk.h"

#include "objfgvobject.h"

#include "panelprms.h"
#include "paneldesign.h"
#include "panelitem.h"
#include "pnlitmsel.h"
#include "panelctrl.h"
#include "ped.h"

#include "resource.h"

namespace StrLiterals
{
	const char unnamed[] = "unnamed";
	const char GalleryFontName[] = "Arial";
	const char target[] = "target:";
	const char Lsystem[] = "Lsystem";
	const char ViewFile[] = "ViewFile";
	const char File[] = "file";
	const char PVer0101[] = "pver 1 1";
}

const COLORREF GalleryTxtColor = RGB(255, 255, 0);

PanelDesign::PanelDesign()
{
	strcpy(_name, DefaultName());
	_items = 0;
	_hView = 0;
	_target = PanelParameters::ptLsystem;
	_trigger = PanelParameters::tcNewLsystem;
	_targetfile[0] = 0;
	UpdateRect();
	_pNotifySink = 0;
	_version = 101;
}


PanelDesign::~PanelDesign()
{
	_Empty();
	if (0 != _hView)
		DestroyWindow(_hView);
}


void PanelDesign::_Empty()
{
	for (int i=0; i<_items; i++)
		delete _arr[i];
	_items = 0;
}


void PanelDesign::Draw(Canvas& cnv) const
{
	for (int i=0; i<_items; i++)
		_arr[i]->Draw(cnv);
}


void PanelDesign::Reset()
{
	_Empty();
	_target = PanelParameters::ptLsystem;
	_trigger = PanelParameters::tcNewLsystem;
}

bool PanelDesign::IsNamed(const char* nm) const
{
	if (0==strcmp(StrLiterals::unnamed, GetName()))
		return false;
	else
		return (0==strcmp(nm, GetName()));
}


void PanelDesign::DrawInGallery(int w, int h, Canvas& cnv) const
{
	for (int i=0; i<_items; ++i)
		_arr[i]->DrawInGallery(cnv);

	const int FontSize = 20;
	LogFont lf(FontSize, StrLiterals::GalleryFontName);
	lf.lfWeight = FW_BOLD;
	Font fnt(lf);
	ObjectHolder sf(cnv, fnt);
	cnv.TextAlign(Canvas::taBaseline, Canvas::taCenter);
	cnv.BkMode(Canvas::bkTransparent);
	cnv.TextColor(GalleryTxtColor);
	cnv.TextOut(w/2, (h+FontSize)/2, _name);
}


EditableObject* PanelDesign::Clone() const
{
	std::unique_ptr<EditableObject> pNew(new PanelDesign);
	pNew->Copy(this);
	return pNew.release();
}

void PanelDesign::Copy(const EditableObject* pObj)
{
	_Empty();
	const PanelDesign* pSrc = dynamic_cast<const PanelDesign*>(pObj);

	for (int i=0; i<pSrc->_items; i++)
	{
		_arr[i] = pSrc->_arr[i]->Clone();
		_arr[i]->SetDesign(this);
	}
	_items = pSrc->_items;

	strcpy(_name, pSrc->_name);
	_target = pSrc->_target;
	_trigger = pSrc->_trigger;
	_version = pSrc->_version;
	strcpy(_targetfile, pSrc->_targetfile);
	_rect = pSrc->_rect;
	_pNotifySink = pSrc->_pNotifySink;
}


const char* PanelDesign::LoadFromClipboard(const char* pClp)
{
	return pClp;
}


char* PanelDesign::CopyToClipboard(char* pClp) const
{
	return pClp;
}

DWORD PanelDesign::ClipboardSize() const
{
	return 0;
}

void PanelDesign::AddSlider(int x, int y)
{
	PanelSlider* pNew = new PanelSlider(x, y);
	_AddItem(pNew);
}


void PanelDesign::AddButton(int x, int y)
{
	PanelButton* pNew = new PanelButton(x, y, PanelButton::GetUniqueName());
	_AddItem(pNew);
}

void PanelDesign::AddLabel(int x, int y)
{
	PanelLabel* pNew = new PanelLabel(x, y);
	_AddItem(pNew);
}

void PanelDesign::AddGroup(POINT p1, POINT p2)
{
	RECT reg;
	if (p1.x>p2.x)
	{
		reg.left = p2.x;
		reg.right = p1.x;
	}
	else
	{
		reg.left = p1.x;
		reg.right = p2.x;
	}

	if (p1.y>p2.y)
	{
		reg.top = p2.y;
		reg.bottom = p1.y;
	}
	else
	{
		reg.top = p1.y;
		reg.bottom = p2.y;
	}
	PanelGroup* pGroup = new PanelGroup;
	std::unique_ptr<PanelItem> pNew(pGroup);

	for (int i=0; i<_items; i++)
	{
		const PanelItem* pItem = _arr[i];
		if (PanelParameters::pitButton == pItem->Type())
		{
			if (pItem->ContainedIn(reg))
			{
				if (pGroup->CanAddButton())
					pGroup->AddButton(pItem);
				else
					throw Exception(IDERR_PNLGRPTOOMANYBUTTONS);
			}
		}
	}

	if (pGroup->IsEmpty())
		throw Exception(IDERR_PNLGRPNOBTNS);

	_AddItem(pNew.release());
}


void PanelDesign::SetName(const char* nm)
{
	strncpy(_name, nm, 32);
	_name[31] = 0;
}



void PanelDesign::_AddItem(PanelItem* pNew)
{
	assert(CanAddItems());
	pNew->SetDesign(this);
	_arr[_items] = pNew;
	_items++;
}



PanelItem* PanelDesign::GetItemAt(int x, int y)
{
	for (int i=0; i<_items; i++)
	{
		if (_arr[i]->Contains(x, y))
			return _arr[i];
	}
	return 0;
}


bool PanelDesign::SelectArea(RECT r, PanelItemsSelection& sel) const
{
	bool res = false;
	for (int i=0; i<_items; i++)
	{
		if (_arr[i]->ContainedIn(r))
		{
			sel.Add(_arr[i]);
			res = true;
		}
	}
	return res;
}


void PanelDesign::Generate(WriteTextFile& trg) const
{
	switch (_version)
	{
	case 99:
		Generate0099(trg);
		break;
	case 101:
		Generate0101(trg);
		break;
	}
}

void PanelDesign::Generate0099(WriteTextFile& trg) const
{
	switch (_target)
	{
	case PanelParameters::ptLsystem :
		trg.PrintF("%s %s\n\n", StrLiterals::target, StrLiterals::Lsystem);
		break;
	case PanelParameters::ptViewFile :
		trg.PrintF("%s %s\n\n", StrLiterals::target, StrLiterals::ViewFile);
		break;
	case PanelParameters::ptFile :
		trg.PrintF("%s %s %s\n\n", StrLiterals::target, StrLiterals::File, _targetfile);
		break;
	}

	for (int i=0; i<_items; i++)
	{
		_arr[i]->Generate(trg); 
		if (i<_items-1)
			trg.WriteLn("");
	}
}


bool PanelDesign::DefaultTrigger() const
{
	if (_target == PanelParameters::ptLsystem && _trigger == PanelParameters::tcNewLsystem)
		return true;
	if (_target == PanelParameters::ptViewFile && _trigger == PanelParameters::tcNewView)
		return true;
	if (_target == PanelParameters::ptFile && _trigger == PanelParameters::tcRerun)
		return true;
	return false;
}

void PanelDesign::Generate0101(WriteTextFile& trg) const
{
	trg.WriteLn(StrLiterals::PVer0101);
	trg.PrintF("name: %s\n", _name);
	trg.PrintF("associate: ");
	switch (_target)
	{
	case PanelParameters::ptLsystem :
		trg.WriteLn("Lsystem");
		break;
	case PanelParameters::ptViewFile :
		trg.WriteLn("View");
		break;
	case PanelParameters::ptFile :
		trg.PrintF("File %s\n", _targetfile);
		break;
	}

	trg.PrintF("trigger: ");
	switch (_trigger)
	{
	case PanelParameters::tcNewLsystem :
		trg.WriteLn("newlsystem");
		break;
	case PanelParameters::tcNewModel :
		trg.WriteLn("newmodel");
		break;
	case PanelParameters::tcNewView :
		trg.WriteLn("newview");
		break;
	case PanelParameters::tcNothing :
		trg.WriteLn("nothing");
		break;
	case PanelParameters::tcRerun :
		trg.WriteLn("rerun");
		break;
	}

	trg.PrintF("items: %d\n", _items);

	for (int i=0; i<_items; i++)
	{
		_arr[i]->Generate(trg);
		
		if (i<_items-1)
			trg.WriteLn("");
	}
}

void PanelDesign::Import(ReadTextFile& src)
{
	std::string bf;

	src.Read(bf);

	if (0==bf.compare(0, 5, "pver "))
	{
		int vmaj, vmin;
		int res = sscanf(bf.c_str(), "pver %d %d", &vmaj, &vmin);
		if (2 != res)
			throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());
		int _version = vmaj*100+vmin;
		switch (_version)
		{
		case 101 :
			Import0101(src);
			break;
		default:
			throw Exception(IDERR_PANELVER, src.Filename());
			break;
		}
	}
	else
		Import0099(src, bf);
}


void PanelDesign::Import0099(ReadTextFile& src, std::string& bf)
{
	_version = 99;
	int heightfix = -1;
	if (strncmp(bf.c_str(), "target: ", 8))
	{
		SetPanelTarget(PanelParameters::ptLsystem);
		_trigger = PanelParameters::tcNewLsystem;
		if (strncmp(bf.c_str(), "panel name:", 11))
			throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());
		src.Read(bf);
		{
			int bg;
			if (1 != sscanf(bf.c_str(), "background: %d", &bg))
				throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());
		}
		src.Read(bf);
		{
			int w, h;
			if (2 != sscanf(bf.c_str(), "size: %d %d", &w, &h))
				throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());
			heightfix = h;
		}
	}
	else
	{
		if (!(strncmp(bf.c_str()+8, "Lsystem", 7)))
		{
			SetPanelTarget(PanelParameters::ptLsystem);
			_trigger = PanelParameters::tcNewLsystem;
		}
		else if (!(strncmp(bf.c_str()+8, "ViewFile", 8)))
		{
			SetPanelTarget(PanelParameters::ptViewFile);
			_trigger = PanelParameters::tcNewView;
		}
		else if (!(strncmp(bf.c_str()+8, "file", 4)))
		{
			SetPanelTarget(PanelParameters::ptFile);
			strcpy(_targetfile, bf.c_str()+13);
			_trigger = PanelParameters::tcRerun;
		}
		else
			throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());
	}

	while (!src.Eof())
	{
		src.Read(bf);
		if (bf.length()>0)
		{
			if (!(CanAddItems()))
				throw Exception(IDERR_TOOMANYITEMSINPANEL, src.Filename());
			std::unique_ptr<PanelItem> pNew(PanelItem::CreateFromFile(bf.c_str(), src, heightfix));
			if (PanelParameters::pitGroup == pNew->Type())
			{
				PanelGroup* pGroup = dynamic_cast<PanelGroup*>(pNew.get());
				if (!(pGroup->UpdateRect(_arr, _items)))
				{
					delete pNew.release();
					std::unique_ptr<PanelItem> pnl(nullptr);
					pNew.reset(pnl.release());
				}
			}
			if (0 != pNew.get())
				_AddItem(pNew.release());
		}
	}
	UpdateRect();
}


void PanelDesign::Import0101(ReadTextFile& src)
{
	std::string line;
	src.Read(line);
	if (1 != sscanf(line.c_str(), "name: %30s", _name))
		throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());

	src.Read(line);

	if (line.compare(0, 11, "associate: "))
		throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());

	line = line.substr(11);

	if (0==line.compare("Lsystem"))
		SetPanelTarget(PanelParameters::ptLsystem);
	else if (0==line.compare("View"))
		SetPanelTarget(PanelParameters::ptViewFile);
	else if (0==line.compare(0, 5, "File "))
	{
		int res = sscanf(line.c_str(), "File %32s", _targetfile);
		if (1 != res)
			throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());
		SetPanelTarget(PanelParameters::ptFile);
	}
	else
		throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());

	src.Read(line);

	if (line.compare(0, 9, "trigger: "))
		throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());
	line = line.substr(9);

	if (0==line.compare("newmodel"))
		_trigger = PanelParameters::tcNewModel;
	else if (0==line.compare("newlsystem"))
		_trigger = PanelParameters::tcNewLsystem;
	else if (0==line.compare("newview"))
		_trigger = PanelParameters::tcNewView;
	else if (0==line.compare("rerun"))
		_trigger = PanelParameters::tcRerun;
	else if (0==line.compare("nothing"))
		_trigger = PanelParameters::tcNothing;
	else
		throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());

	src.Read(line);

	int items;

	int res = sscanf(line.c_str(), "items: %d", &items);
	if (res != 1)
		throw Exception(IDERR_READINGPANEL, src.Filename(), src.Line());

	for (int i=0; i<items;)
	{
		src.Read(line);
		if (line.length()>0)
		{
			if (!(CanAddItems()))
				throw Exception(IDERR_TOOMANYITEMSINPANEL, src.Filename());
			std::unique_ptr<PanelItem> pNew(PanelItem::CreateFromFile(line.c_str(), src, -1));
			if (PanelParameters::pitGroup == pNew->Type())
			{
				PanelGroup* pGroup = dynamic_cast<PanelGroup*>(pNew.get());
				if (!(pGroup->UpdateRect(_arr, _items)))
				{
					delete pNew.release();
					std::unique_ptr<PanelItem> pnl(nullptr);
					pNew.reset(pnl.release());
				}
			}
			if (0 != pNew.get())
			{
				_AddItem(pNew.release());
				++i;
			}
		}
	}
	UpdateRect();
}

int PanelDesign::FindItem(const PanelItem* pItm) const
{
	for (int i=0; i<_items; i++)
	{
		if (pItm == _arr[i])
			return i;
	}
	return -1;
}


void PanelDesign::DeleteItem(int id)
{
	assert(id>=0);
	assert(id<_items);
	delete _arr[id];
	for (int i=id; i<_items-1; i++)
		_arr[i] = _arr[i+1];
	_items--;
}


PanelItem* PanelDesign::Duplicate(const PanelItem* pItm, int dx, int dy)
{
	assert(CanAddItems());
	char NewName[PanelParameters::eMaxNameLength + 1];
	strcpy(NewName, "copy of ");
	strncat(NewName, pItm->GetName(), PanelParameters::eMaxNameLength - 8);

	_arr[_items] = pItm->Clone();
	_arr[_items]->MoveBy(dx, dy);
	_arr[_items]->SetName(NewName);
	_arr[_items]->SetDesign(this);
	_items++;
	return _arr[_items-1];
}


void PanelDesign::UpdateRect()
{
again:
	if (_items==0)
	{
		_rect.left = 0; _rect.right = 10;
		_rect.top = 0; _rect.bottom = 10;
	}
	else
	{
		_arr[0]->GetRect(_rect);
		RECT tmpr;
		for (int i=0; i<_items; i++)
		{
			if (PanelParameters::pitGroup==_arr[i]->Type())
			{
				PanelGroup* pGroup = dynamic_cast<PanelGroup*>(_arr[i]);
				if (!(pGroup->UpdateRect(_arr, _items)))
				{
					/* If all the buttons are gone
					delete the group here */
					delete _arr[i];
					for (int j=i; j<_items-1; j++)
						_arr[j] = _arr[j+1];
					_arr[_items-1] = 0;
					_items--;
					goto again;
				}
			}
			_arr[i]->GetRect(tmpr);
			if (tmpr.left<_rect.left)
				_rect.left = tmpr.left;
			if (tmpr.right>_rect.right)
				_rect.right = tmpr.right;
			if (tmpr.top<_rect.top)
				_rect.top = tmpr.top;
			if (tmpr.bottom>_rect.bottom)
				_rect.bottom = tmpr.bottom;
		}
	}
}

void PanelDesign::Execute(HWND hWnd)
{
	if (0 == _hView)
		_hView = PanelCtrl::Create(GetWindowInstance(hWnd), this);
	else
		BringWindowToTop(_hView);
}


void PanelDesign::ResetValues()
{
	for (int i=0; i<_items; i++)
		_arr[i]->ResetValue();
}


void PanelDesign::PanelClosed()
{
	assert(0 != _hView);
	_hView = 0;
}

void PanelDesign::CloseTearedOff()
{
	if (0 != _hView)
		PostMessage(_hView, WM_CLOSE, 0, 0);
}


void PanelDesign::SetPanelTargetFilename(const char* bf)
{
	strcpy(_targetfile, bf);
}



RECT PanelDesign::ExecuteHit(Canvas& cnv, int x, int y)
{
	RECT r = {0, 0, 0, 0};
	for (int i=0; i<_items; i++)
	{
		PanelItem* pItem = _arr[i];
		if (pItem->Contains(x, y))
		{
			pItem->GetRect(r);
			if (PanelParameters::pitButton==pItem->Type())
			{
				int group = _ParentGroup(pItem->GetName());
				if (-1 != group)
				{
					bool cm = _pNotifySink->ContinuousMode();
					_pNotifySink->ContinuousMode(false);
					_ResetGroup(cnv, group);
					if (cm)
						_pNotifySink->ContinuousMode(true);
				}
			}
			pItem->Hit(cnv, x, y, false);
			return r;
		}
	}
	return r;
}

bool PanelDesign::ExecuteDrag(Canvas& cnv, int x, int y, bool final)
{
	for (int i=0; i<_items; i++)
	{
		PanelItem* pItem = _arr[i];
		if (pItem->Contains(x, y))
		{
			if (PanelParameters::pitSlider == pItem->Type())
				return pItem->Hit(cnv, x, y, final);
			return true;
		}
	}
	return true;
}


bool PanelDesign::Action(const char* act, bool final)
{
	switch (_target)
	{
	case PanelParameters::ptFile :
		{
			if (strlen(_targetfile)>0)
			{
				FilePed fpd(_targetfile);
				fpd.Action(act);
			}
		}
		break;
	case PanelParameters::ptLsystem :
		{
			WndPed wpd(_pNotifySink->GetLsystemEditWnd());
			wpd.Action(act);
		}
		break;
	case PanelParameters::ptViewFile :
		{
			WndPed wpd(_pNotifySink->GetViewEditWnd());
			wpd.Action(act);
		}
		break;
	}
	if (_pNotifySink->ContinuousMode())
	{
		switch (_trigger)
		{
		case PanelParameters::tcNewLsystem :
			return NewLsystem(final);
			break;
		case PanelParameters::tcNewModel :
			return NewModel(final);
			break;
		case PanelParameters::tcNewView :
			return NewView(final);
			break;
		case PanelParameters::tcRerun :
			return Rerun(final);
			break;
		}
	}
	return true;
}


int PanelDesign::_ParentGroup(const char* btn) const
{
	for (int i=0; i<_items; i++)
	{
		if (PanelParameters::pitGroup == _arr[i]->Type())
		{
			const PanelGroup* pGroup = dynamic_cast<const PanelGroup*>(_arr[i]);
			if (pGroup->ContainsButton(btn))
				return i;
		}
	}
	return -1;
}


void PanelDesign::_ResetGroup(Canvas& cnv, int itm)
{
	assert(PanelParameters::pitGroup==_arr[itm]->Type());
	const PanelGroup* pGroup = dynamic_cast<const PanelGroup*>(_arr[itm]);

	for (int i=0; i<pGroup->Items(); i++)
	{
		int ix = _FindItem(PanelParameters::pitButton, pGroup->ButtonName(i));
		if (-1 != ix)
		{
			PanelButton* pButton = dynamic_cast<PanelButton*>(_arr[ix]);
			pButton->Reset(cnv);
		}
	}
}


int PanelDesign::_FindItem(PanelParameters::PanelItemType type, const char* nm) const
{
	for (int i=0; i<_items; i++)
	{
		if (type == _arr[i]->Type())
		{
			if (!(strcmp(_arr[i]->GetName(), nm)))
				return i;
		}
	}
	return -1;
}


void PanelDesign::DefaultValues(Canvas& cnv)
{
	bool cm = _pNotifySink->ContinuousMode();
	_pNotifySink->ContinuousMode(false);

	for (int i=0; i<_items; i++)
	{
		if (PanelParameters::pitGroup==_arr[i]->Type())
			_ResetGroup(cnv, i);
	}

	{
		for (int i=0; i<_items; i++)
				_arr[i]->DefaultValue(cnv);
	}

	if (cm)
	{
		_pNotifySink->ContinuousMode(true);
		switch (_trigger)
		{
		case PanelParameters::tcNewLsystem :
			NewLsystem(true);
			break;
		case PanelParameters::tcNewModel :
			NewModel(true);
			break;
		case PanelParameters::tcNewView :
			NewView(true);
			break;
		case PanelParameters::tcRerun :
			Rerun(true);
			break;
		}
	}
}


bool PanelDesign::NewLsystem(bool final)
{
	return _pNotifySink->LsystemModified(final);
}

bool PanelDesign::NewView(bool final)
{
	return _pNotifySink->ViewFileModified(final);
}

bool PanelDesign::NewModel(bool final)
{
	return _pNotifySink->NewModel(final);
}

bool PanelDesign::Rerun(bool final)
{
	return _pNotifySink->ExternalFileModified(final);
}


void PanelDesign::SetNotifySink(PrjNotifySink* pSink)
{ 
	_pNotifySink = pSink; 
	_Directory = _pNotifySink->GetLabTable();
}
