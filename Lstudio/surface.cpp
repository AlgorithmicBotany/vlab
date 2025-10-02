#include <memory>
#include <vector>

#include <fw.h>
#include <glfw.h>


#include "patchclrinfo.h"
#include "patch.h"
#include "objfgvobject.h"
#include "surface.h"
#include "traditional.h"

#include "resource.h"

/*********************************

  Patch numbers

	0 - AL		1 - A		2 - AR
	3 - L		4 - C		5 - R
	6 - BL		7 - B		8 - BR


**********************************/


Surface::Surface()
{
	_pPatch = new Patch;
	_pPatch->SetUsed();
	strcpy(_Name, "unnamed");
	_editable = true;
	_pTraditional = 0;
	_size = 1.0f;
	_heading.Set(0.0f, 1.0f, 0.0f);
	_up.Set(0.0f, 0.0f, -1.0f);
}

Surface::~Surface()
{
	delete _pTraditional;
	delete _pPatch;
}


void Surface::Reset()
{
	_pPatch->Reset();
	_pPatch->SetUsed();
	strcpy(_Name, "unnamed");
}


void Surface::Draw(int drawWhat, int activePoint, unsigned int base) const
{
	if (_editable)
	{
		_pPatch->Draw(drawWhat, activePoint, base);
		_DrawAxis(ContactPointId == activePoint, base);
	}
}


void Surface::_DrawAxis(bool ActiveCP, unsigned int base) const
{
	glColor3f(0.8f, 0.0f, 0.0f);
	{
		GLlines gll;
		gll.Vertex(_cp.Vertex());
		gll.Vertex(_cp.X()+_heading.X(), _cp.Y()+_heading.Y(), _cp.Z()+_heading.Z());
		gll.Vertex(_cp.Vertex());
		gll.Vertex(_cp.X()-_up.X(), _cp.Y()-_up.Y(), _cp.Z()-_up.Z());
	}
	const TCHAR xyz[] = __TEXT("HUC");
	glListBase(base);
	glColor3f(1.0f, 0.0f, 0.0f);
	{
		glRasterPos3f
			(
			_cp.X()+_heading.X(), 
			_cp.Y()+_heading.Y(), 
			_cp.Z()+_heading.Z()
			);
		glCallLists(1, 
#ifdef UNICODE
			GL_SHORT,
#else
			GL_BYTE,
#endif
			xyz);
	}
	{
		glRasterPos3f
			(
			_cp.X()-_up.X(), 
			_cp.Y()-_up.Y(), 
			_cp.Z()-_up.Z()
			);
		glCallLists(1, 
#ifdef UNICODE
			GL_SHORT,
#else
			GL_BYTE,
#endif
			xyz+1);
	}
	{
		if (ActiveCP)
			glColor3f(1.0f, 1.0f, 1.0f);
		glPointSize(5.0f);
		{
			GLpoints glp;
			glp.Vertex(_cp.Vertex());
		}
		glPointSize(1.0f);
	}
}



void Surface::SetPoint(int i, WorldPointf p)
{
	assert(i>=0);
	assert(i<16 || i==ContactPointId);
	if (i==ContactPointId)
		_cp = p;
	else
		_pPatch->SetPoint(i, p, _cp, _heading, _up);
}



void Surface::DrawInGallery() const
{
	PushPopMatrix ppm;

	WorldPointf center;
	float ms = 0.0f;
	float zsz = 0.0f;
	if (_editable)
	{
		BoundingBox bb(WorldPointf(0.0f, 0.0f, 0.0f));
		_GetBoundingBox(bb);
		center = bb.Center();
		float xsz = bb.XSize();
		float ysz = bb.YSize();
		ms = (xsz>ysz) ? xsz : ysz;
		zsz = bb.ZSize();
		if (zsz>90.0f)
			zsz = 90.0f/zsz;
		else
			zsz = 1.0f;
	}
	else
	{
		const ViewBox& vb = _pTraditional->GetBoundingBox();
		center = vb.Center();
		float xsz = vb.XRange();
		float ysz = vb.YRange();
		ms = (xsz>ysz) ? xsz : ysz;
		zsz = vb.ZRange();
		if (zsz>90.0f)
			zsz = 90.0f/zsz;
		else
			zsz = 1.0f;
	}


	glScalef(3.6f/ms, 3.6f/ms, zsz);

	glTranslatef(-center.X(), -center.Y(), -center.Z());

	if (_editable)
		_pPatch->Draw(PatchSpace::DrawDenseWireframe, 0, 0);
	else
		_pTraditional->Draw();
}


void Surface::Copy(const EditableObject* pSrc)
{
	const Surface* pObj = dynamic_cast<const Surface*>(pSrc);
	_editable = pObj->_editable;
	if (_editable)
	{
		*_pPatch = *(pObj->_pPatch);

		_cp = pObj->_cp;
		_ep = pObj->_ep;
		_heading = pObj->_heading;
		_up = pObj->_up;
		_size = pObj->_size;
	}
	assert(strlen(pObj->_Name)<MaxNameLength);
	if (0 == pObj->_Name[0])
		strcpy(_Name, "unnamed");
	else
		strcpy(_Name, pObj->_Name);
}


EditableObject* Surface::Clone() const
{
	std::unique_ptr<Surface> pNew(new Surface);
	pNew->Copy(this);
	return pNew.release();
}


void Surface::Generate(WriteTextFile& trg) const
{
	if (_editable)
		_GenerateEditable(trg);
	else
	{
		assert(0 != _pTraditional);
		_pTraditional->Generate(trg);
	}
}


bool Surface::IsNamed(const char* nm) const
{
	if (0==strcmp("unnamed", GetName()))
		return false;
	else
		return (0==strcmp(nm, GetName()));
}

void Surface::_GenerateEditable(WriteTextFile& trg) const
{
	assert(0 == _pTraditional);

	// First line: bounding box

	BoundingBox bb(WorldPointf(0.0f, 0.0f, 0.0f));
	_GetBoundingBox(bb);
	trg.PrintF
		(
		"%.2f %.2f   %.2f %.2f   %.2f %.2f\n",
		bb.minX(), bb.maxX(),
		bb.minY(), bb.maxY(),
		bb.minZ(), bb.maxZ()
		);

	// Contact point

	WorldPointf cp = _cp;
	trg.PrintF
		(
		"CONTACT POINT  X: %.2f Y: %.2f Z: %.2f\n",
		cp.X(), cp.Y(), cp.Z()
		);

	// End point 
	trg.PrintF
		(
		"END POINT  X: %.2f Y: %.2f Z: %.2f\n",
		_ep.X(), _ep.Y(), _ep.Z()
		);

	trg.PrintF("HEADING  X: %.2f Y: %.2f Z: %.2f\n", _heading.X(), _heading.Y(), _heading.Z());

	trg.PrintF("UP  X: %.2f Y: %.2f Z: %.2f\n", _up.X(), _up.Y(), _up.Z());

	trg.PrintF("SIZE: %.2f\n", _size);

	// Description of the patch
	trg.PrintF("patch\n");
	{
		const PatchColorInfo& colorinfo = _pPatch->GetColorInfo();
		trg.PrintF
			(
			"TOP COLOR: %d DIFFUSE: %.2f BOTTOM COLOR: %d DIFFUSE: %.2f\n",
			colorinfo.GetTopColor(),
			colorinfo.GetTopDiffuse(),
			colorinfo.GetBottomColor(),
			colorinfo.GetBottomDiffuse()
			);
	}
	trg.PrintF("AL: ~ A: ~ AR: ~ \n");
	trg.PrintF("L: ~ R: ~ \n");
	trg.PrintF("BL: ~ B: ~ BR: ~ \n");

	int i = 0;
	for (int row=0; row<4; row++)
	{
		for (int col=0; col<4; col++)
		{
			WorldPointf p = _pPatch->GetPoint(i);
			trg.PrintF("%.2f %.2f %.2f  ", p.X(), p.Y(), p.Z());
			i++;
		}
		trg.WriteEOL();
	}
}



void Surface::Import(ReadTextFile& src) 
{
	assert(0 == _pTraditional);
	assert(_editable);
	std::unique_ptr<TraditionalSurface> pNew(new TraditionalSurface(src));
	if (pNew->Patches()>1)
	{
		_pTraditional = pNew.release();
		_editable = false;
	}
	else
	{
		assert(1==pNew->Patches());
		_cp = pNew->ContactPoint();
		_ep = pNew->EndPoint();
		_heading = pNew->Heading();
		_up = pNew->Up();
		_size = pNew->Size();

		const TraditionalPatch& Patch = pNew->AccessFirst();
		_pPatch->SetColorInfo(Patch.GetColorInfo());
		for (int i=0; i<16; ++i)
		{
			WorldPointf p(Patch.GetPoint(i));
			_pPatch->SetPoint(i, p);
		}
		WorldPointf p;
		BoundingBox bb(p);
		_pPatch->GetBoundingBox(bb);
	}
}


void Surface::GetBoundingBox(BoundingBox& bb) const
{
	if (_editable)
		_GetBoundingBox(bb);
	else
	{
		WorldPointf p(-1.0f, -1.0f, -1.0f);
		bb.Reset(p);
		p.Set(1.0f, 1.0f, 1.0f);
		bb.Adapt(p);
	}
}


void Surface::_GetBoundingBox(BoundingBox& bb) const
{
	assert(_editable);

	bb.Reset(_pPatch->GetPoint(0));

	for (int i=1; i<16; i++)
		bb.Adapt(_pPatch->GetPoint(i));

	bb.Adapt(_cp);
}


DWORD Surface::ClipboardSize() const
{
	DWORD res = 0;

	if (_editable)
	{
		res += MaxNameLength;
		res += sizeof(bool); // editable
		res += _pPatch->ClipboardSize();
		res += sizeof(WorldPointf); // cp
		res += sizeof(WorldPointf); // ep
		res += sizeof(WorldPointf); // heading
		res += sizeof(WorldPointf); // up
		res += sizeof(float);     // size
	}
	else
	{
		assert(0 != _pTraditional);
		res += MaxNameLength;
		res += sizeof(bool);
		res += _pTraditional->ClipboardSize();
	}
	return res;
}


char* Surface::CopyToClipboard(char* pCur) const
{
	strcpy(pCur, _Name);
	pCur += MaxNameLength;

	ToClipboard<bool>(_editable, pCur);

	if (_editable)
	{

		{
			pCur = _pPatch->CopyToClipboard(pCur);
		}

		ToClipboard<WorldPointf>(_cp, pCur);
		ToClipboard<WorldPointf>(_ep, pCur);
		ToClipboard<WorldPointf>(_heading, pCur);
		ToClipboard<WorldPointf>(_up, pCur);
		ToClipboard<float>(_size, pCur);
	}
	else
	{
		assert(0 != _pTraditional);
		pCur = _pTraditional->CopyToClipboard(pCur);
	}

	return pCur;
}

const char* Surface::LoadFromClipboard(const char* pCur)
{
	strcpy(_Name, pCur);
	pCur += MaxNameLength;

	FromClipboard<bool>(_editable, pCur);

	if (_editable)
	{
		{
			pCur = _pPatch->LoadFromClipboard(pCur);
		}

		FromClipboard<WorldPointf>(_cp, pCur);
		FromClipboard<WorldPointf>(_ep, pCur);
		FromClipboard<WorldPointf>(_heading, pCur);
		FromClipboard<WorldPointf>(_up, pCur);
		FromClipboard<float>(_size, pCur);
	}
	else
	{
		assert(0 == _pTraditional);
		_pTraditional = new TraditionalSurface(&pCur);
	}

	return pCur;
}


bool Surface::operator !=(const EditableObject& obj) const
{
	const Surface* pR = dynamic_cast<const Surface*>(&obj);
	if (strcmp(_Name, pR->_Name))
		return true;
	if (_editable)
	{
		if (!_pPatch->IsEqual(pR->_pPatch))
			return true;
		if (_cp != pR->_cp)
			return true;
		if (_ep != pR->_ep)
			return true;
		if (_heading != pR->_heading)
			return true;
		if (_up != pR->_up)
			return true;
		if (_size != pR->_size)
			return true;
	}

	return false;
}


const WorldPointf& Surface::GetPoint(int n) const
{
	if (ContactPointId == n)
		return GetCP();
	else
		return _pPatch->GetPoint(n); 
}
