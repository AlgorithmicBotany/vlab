#include <memory>

#include <fw.h>

#include "objfgvgallery.h"
#include "objfgvobject.h"
#include "objfgvedit.h"

#include "resource.h"

ObjectGallery::ObjectGallery(int initSize)
{
	_pEdit = 0;
	_arr = new std::unique_ptr<EditableObject>[initSize];
	_arrSize = initSize;
	_items = 0;
	_current = 0;
	_FirstSelected = _LastSelected = _current;
}


ObjectGallery::~ObjectGallery()
{
	delete []_arr;
}


const EditableObject* ObjectGallery::GetObject(int i) const
{
	assert(i>=0);
	assert(i<_items);
	return _arr[i].get();
}


EditableObject* ObjectGallery::_GetObject(int i) const
{
	assert(i>=0);
	assert(i<_items);
	return _arr[i].get();
}



void ObjectGallery::SetObject(int i, const EditableObject* pObj)
{
	assert(i>=0);
	assert(i<_items);
	_arr[i]->Copy(pObj);
}


void ObjectGallery::Add(std::unique_ptr<EditableObject>& pNew)
{
	if (_items == _arrSize)
		_Grow();
    _arr[_items] = std::move(pNew);
	_items++;
}



void ObjectGallery::_Grow(int sz)
{
	if (-1 == sz)
		sz = _arrSize*2;
	
	std::unique_ptr<EditableObject>* aNew = new std::unique_ptr<EditableObject>[sz];
	for (int i=0; i<_items; i++)
        aNew[i] = std::move(_arr[i]);
	delete []_arr;
	_arr = aNew;
	_arrSize = sz;
}


void ObjectGallery::Clear()
{
	_Empty();
	std::unique_ptr<EditableObject> pNew(_NewObject());
	Add(pNew);
}




void ObjectGallery::_Empty()
{
	for (int i=0; i<_items; i++)
		delete _arr[i].release();
	_items = 0;
	_current = 0;
	_FirstSelected = _LastSelected = 0;
}


void ObjectGallery::_Delete()
{
	for (int i=_FirstSelected; i<=_LastSelected; i++)
		delete _arr[i].release();

	if (0==_FirstSelected && _LastSelected==_items-1)
	{
		_items = 0;
		std::unique_ptr<EditableObject> pNew(_NewObject());
		Add(pNew);
		SetCurrent(0);
	}
	else
	{
		int from = _LastSelected+1;
		int to = _FirstSelected;
		while (from<_items)
		{
            _arr[to] = std::move(_arr[from]);
			to++;
			from++;
		}
		_items = to;
		if (_current>=_items)
			SetCurrent(_items-1);
		else
			SetCurrent(_current);
	}
	_pEdit->Retrieve();
	ForceRepaint();
}


void ObjectGallery::_New()
{
	std::unique_ptr<EditableObject> pNew(_NewObject());
	Add(pNew);
	_pEdit->SelectionChanged(_current, _items-1);
	ScrollToShow();
}



/* Clipboard support */


void ObjectGallery::_Cut()
{
	_Copy();
	_Delete();
}

void ObjectGallery::_Copy() const
{
	UseClipboard clpbrd(_ClipboardOwner());
	ClipboardMemory mem(_ClipboardSize());
	clpbrd.Empty();
	{
		MemoryLock lock(mem.Handle());
		_CopySelectionToClipboard(lock.Ptr());
	}
	SetClipboardData(_ClipboardFormatId(), mem.Release());
}


void ObjectGallery::_Paste()
{
	if (IsClipboardFormatAvailable(_ClipboardFormatId()))
	{
		UseClipboard clpbrd(_ClipboardOwner());
		HGLOBAL hMem = GetClipboardData(_ClipboardFormatId());
		MemoryLock lock(hMem);
		_CopyClipboardToSelection(lock.Ptr(), _items);
		ScrollToShow();
		_pEdit->Retrieve();
	}
}

void ObjectGallery::_PasteAfter()
{
	if (IsClipboardFormatAvailable(_ClipboardFormatId()))
	{
		UseClipboard clpbrd(_ClipboardOwner());
		HGLOBAL hMem = GetClipboardData(_ClipboardFormatId());
		MemoryLock lock(hMem);
		_CopyClipboardToSelection(lock.Ptr(), _current+1);
		ScrollToShow();
		_pEdit->Retrieve();
	}
}



void ObjectGallery::_PasteBefore()
{
	if (IsClipboardFormatAvailable(_ClipboardFormatId()))
	{
		UseClipboard clpbrd(_ClipboardOwner());
		HGLOBAL hMem = GetClipboardData(_ClipboardFormatId());
		MemoryLock lock(hMem);
		_CopyClipboardToSelection(lock.Ptr(), _current);
		ScrollToShow();
		_pEdit->Retrieve();
	}
}



DWORD ObjectGallery::_ClipboardSize() const
{
	DWORD res = 0;
	res += sizeof(int); // Number of objects
	res += sizeof(int); // First object index
	res += sizeof(int); // Last object index
	for (int i=_FirstSelected; i<=_LastSelected; i++)
	{
		const EditableObject* pObj = GetObject(i);
		res += pObj->ClipboardSize();
	}
	return res;
}

void ObjectGallery::_CopySelectionToClipboard(void* pVoid) const
{
	char* pCur = reinterpret_cast<char*>(pVoid);
	{
		int* pInt = reinterpret_cast<int*>(pCur);
		*pInt = _LastSelected - _FirstSelected + 1; // First: the number of objects
		pCur += sizeof(int);
		pInt++;
		*pInt = _FirstSelected; // First object index
		pCur += sizeof(int);
		pInt++;
		*pInt = _LastSelected; // Last object index
		pCur += sizeof(int);
		pInt++;
	}
	for (int i=_FirstSelected; i<=_LastSelected; i++)
	{
		const EditableObject* pObject = GetObject(i);
		pCur = pObject->CopyToClipboard(pCur);
	}
}


void ObjectGallery::_CopyClipboardToSelection(const void* pVoid, int insertpos)
{
	const char* pCur = reinterpret_cast<const char*>(pVoid);
	{
		const int* pItems = reinterpret_cast<const int*>(pCur);
		pCur += sizeof(int);
		assert(*pItems>0);
		{
			const int* pDummy = reinterpret_cast<const int*>(pCur);
			pCur += sizeof(int); // Skip first object index
			pDummy = reinterpret_cast<const int*>(pCur);
			pCur += sizeof(int); // Skip last object index
		}
		
		for (int i=0; i<*pItems; i++)
		{
			std::unique_ptr<EditableObject> pNew(_NewObject());
			pCur = pNew->LoadFromClipboard(pCur);
			_Insert(pNew, insertpos);
			_PostPaste(insertpos);
			insertpos++;
		}
	}
}


void ObjectGallery::_Insert(std::unique_ptr<EditableObject>& pNew, int ins)
{
	assert(ins>=0);
	assert(ins<=_items);
	if (_items==_arrSize)
		_Grow();
	for (int i=_items; i>ins; i--)
        _arr[i] = std::move(_arr[i-1]);
    _arr[ins] = std::move(pNew);
	_items++;
}
