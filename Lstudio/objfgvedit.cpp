#include <memory>

#include <fw.h>

#include "objfgvedit.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvobject.h"


INIT_COUNTER(ObjectEdit);

bool ObjectEdit::SelectionChanged(int current, int newsel)
{
	assert(0 != _pGallery);
	assert(0 != _pView);
	_UpdateFromControls();
	const EditableObject* pObj = _pView->GetObject();
	const EditableObject* pCur = _pGallery->GetObject(current);
	const bool res = (*pCur != *pObj);
	if (res)
		_pGallery->SetObject(current, pObj);
	pObj = _pGallery->GetObject(newsel);
	_pGallery->SetCurrent(newsel);
	_pView->CopyObject(pObj);
	_UpdateControls();
	_pGallery->ForceRepaint();
	_pView->ForceRepaint();
	return res;
}

void ObjectEdit::ApplyNow()
{
	assert(0 != _pGallery);
	assert(0 != _pView);
	_UpdateFromControls();
	const EditableObject* pObj = _pView->GetObject();
	const int current = _pGallery->GetCurrent();
	_pGallery->SetObject(current, pObj);
	_pGallery->ForceRepaint();
}


void ObjectEdit::Retrieve()
{
	assert(0 != _pGallery);
	assert(0 != _pView);
	const int current = _pGallery->GetCurrent();
	const EditableObject* pObj = _pGallery->GetObject(current);
	_pView->CopyObject(pObj);
	_UpdateControls();
	_pView->ForceRepaint();
}


void ObjectEdit::AddAsNew()
{
	assert(0 != _pGallery);
	assert(0 != _pView);
	_UpdateFromControls();
	const EditableObject* pObj = _pView->GetObject();
	std::unique_ptr<EditableObject> pNew(pObj->Clone());
	_pGallery->Add(pNew);
	const int items = _pGallery->Items();
	assert(items>0);
	_pGallery->SetCurrent(items-1);
	_pGallery->ScrollToShow();
}


int ObjectEdit::Items() const
{
	return _pGallery->Items();
}


void ObjectEdit::Select(int i)
{
	assert(i>=0);
	assert(i<_pGallery->Items());
	_pGallery->SetCurrent(i);
	_UpdateControls();
	_pGallery->ForceRepaint();
}


void ObjectEdit::Clear()
{
	_pGallery->Clear();
	_pGallery->SetCurrent(0);
	Retrieve();
	_pGallery->ScrollToShow();
}


void ObjectEdit::Delete()
{
	assert(Items()>1);
	_pGallery->Delete();
}
