#include <fw.h>

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvedit.h"


INIT_COUNTER(ObjectView);


ObjectView::ObjectView()
{
	_pEdit = 0;
	_pObj = 0;
	INC_COUNTER;
}



ObjectView::~ObjectView()
{
	DEC_COUNTER;
	delete _pObj;
}


void ObjectView::CopyObject(const EditableObject* pObj)
{
	_pObj->Copy(pObj);
}


void ObjectView::_ApplyNow()
{
	assert(0 != _pEdit);
	_pEdit->ApplyNow();
}


void ObjectView::_Retrieve()
{
	assert(0 != _pEdit);
	_pEdit->Retrieve();
}


void ObjectView::_AddAsNew()
{
	assert(0 != _pEdit);
	_pEdit->AddAsNew();
}

