#include <memory>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "glgallery.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "resource.h"

#include "material.h"
#include "matgallery.h"
#include "matparamcb.h"
#include "materialedit.h"

#include "menuids.h"


INIT_COUNTER(MaterialGallery);

UINT MaterialGallery::_ClipboardFormat = 0;

void MaterialGallery::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<MaterialGallery>::Proc);
	wc.hCursor = 0;
	wc.Register();
	_ClipboardFormat = RegisterClipboardFormat(_ClipboardFormatName());
}


MaterialGallery::MaterialGallery(HWND hwnd, const CREATESTRUCT* pCS) : 
GLGallery(hwnd, pCS, 1.0, NumOfMaterials)
{
	_pQobj = 0;
	_pMaterialEdit = 0;
	for (int i=0; i<NumOfMaterials; i++)
	{
		std::unique_ptr<EditableObject> pNew(new Material);
		Add(pNew);
	}
	{
		CurrentContext cc(this);
		_DoInit();
	}
}




MaterialGallery::~MaterialGallery()
{
	{
		CurrentContext cc(this);
		_DoExit();
	}
	assert(0 == _pQobj);
}


EditableObject* MaterialGallery::_NewObject() const
{
	assert(0); 
	return new Material;
}


void MaterialGallery::SetMaterialEdit(MaterialEdit* pEdit)
{
	assert(0 == _pMaterialEdit);
	_pMaterialEdit = pEdit;
	ObjectGallery::SetEdit(pEdit);
}


bool MaterialGallery::HScroll(HScrollCode code, int pos, HWND)
{
	switch (code)
	{
	case hscLineLeft :
		if (_firstVisible > 0)
		{
			_firstVisible -= MaterialsPerColumn;
		}
		break;
	case hscLineRight :
		if (_firstVisible <= NumOfMaterials-_columns*MaterialsPerColumn)
		{
			_firstVisible += MaterialsPerColumn;
		}
		break;
	case hscPageLeft :
		if (_firstVisible > 0)
		{
			_firstVisible -= MaterialsPerColumn*(_columns-1);
			if (_firstVisible < 0)
				_firstVisible = 0;
		}
		break;
	case hscPageRight :
		if (_firstVisible <= NumOfMaterials-_columns*4)
		{
			_firstVisible += MaterialsPerColumn*(_columns-1);
			if (_firstVisible > NumOfMaterials-(_columns-1)*MaterialsPerColumn)
				_firstVisible = NumOfMaterials-(_columns-1)*MaterialsPerColumn;
		}
		break;
	case hscThumbTrack :
	case hscThumbPosition :
		if ((pos >=0) && (pos <= NumOfMaterials/MaterialsPerColumn - _columns + 1))
		{
			_firstVisible = pos * MaterialsPerColumn;
		}
		break;
	case hscRight :
		_firstVisible = NumOfMaterials-(_columns-1)*MaterialsPerColumn;
		break;
	case hscLeft :
		_firstVisible = 0;
		break;
	case hscEndScroll :
	default :
		return true;
	}

	_UpdateScrollbar();
	ForceRepaint();
	return true;
}



void MaterialGallery::_DoSize()
{
	if (0 == Height())
		return;
	_xItemSize = Height()/MaterialsPerColumn;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	{
		const float coeff = static_cast<float>(Width())/static_cast<float>(Height());
		float xrange = 2.0f * coeff;
		glOrtho(0.0f, xrange, -1.0f, 1.0f, -2.0f, 2.0f);
		_columns = static_cast<int>(ceil(2.0f*xrange));
		_upp = 2.0f/Height();
		_MinPoint.Set(0.0f, -1.0f, -2.0f);
		int maxfirstvisible = NumOfMaterials - MaterialsPerColumn*(_columns-1);
		if (_firstVisible>maxfirstvisible)
			_firstVisible = maxfirstvisible;
	}


	{
		GLfloat pos[4] = { 50.0f, 50.0f, 50.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
		GLfloat amb[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	}

	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, Width(), Height());

	_UpdateScrollbar();
}


void MaterialGallery::_DoInit()
{
	assert(0 == _pQobj);
	_pQobj = gluNewQuadric();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void MaterialGallery::_DoExit()
{
	gluDeleteQuadric(_pQobj);
	_pQobj = 0;
}

void MaterialGallery::Clear()
{
	for (int i=0; i<NumOfMaterials; ++i)
	{
		Material* pMat = dynamic_cast<Material*>(_GetObject(i));
		pMat->Reset();
	}
}


void MaterialGallery::_DoPaint() const
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int ix = _firstVisible;
#ifdef MATGALRESIZEABLE
	for (int i=0; (i<_columns) && (ix<NumOfMaterials); i++)
#else
	for (int i=0; i<8; i++)
#endif
	{
		for (int j=MaterialsPerColumn-1; j>=0; j--)
		{
			assert(ix<NumOfMaterials);
			PushPopMatrix ppm;
			const Material* pMaterial = dynamic_cast<const Material*>(_arr[ix].get());
			pMaterial->Apply();
#ifdef MATGALRESIZEABLE
			glTranslatef(0.25f+0.5f*i, -0.75f+0.5f*j, 0.0f);
#else
			glTranslatef(-1.75f+0.5f*i, -0.75f+0.5f*j, 0.0f);
#endif
			gluSphere(_pQobj, 0.23f, 18, 18);
			if (_current == ix)
			{
				GLdisable disableLight(GL_LIGHTING);
				glColor3f(1.0f, 1.0f, 1.0f);
				GLlineloop frame;
				frame.Vertex(-0.24f, -0.24f, 0.01f);
				frame.Vertex( 0.24f, -0.24f, 0.01f);
				frame.Vertex( 0.24f,  0.24f, 0.01f);
				frame.Vertex(-0.24f,  0.24f, 0.01f);
			}
			else if ((ix>=_FirstSelected) && (ix<=_LastSelected))
			{
				GLdisable disableLight(GL_LIGHTING);
				glColor3f(0.6f, 0.6f, 0.6f);
				GLlineloop frame;
				frame.Vertex(-0.24f, -0.24f, 0.01f);
				frame.Vertex( 0.24f, -0.24f, 0.01f);
				frame.Vertex( 0.24f,  0.24f, 0.01f);
				frame.Vertex(-0.24f,  0.24f, 0.01f);
			}
			ix++;
		}
	}
	glFlush();
}


bool MaterialGallery::LButtonUp(KeyState, int, int)
{
	if (_dragging)
	{
		_DragTo(_dragtarget);
		_dragtarget = -1;
		_dragging = false;
		ReleaseCapture();
		// If the selected item is not fully visible
		// scroll by one
		int selectedcol = _FirstSelected/MaterialsPerColumn;
		int firstvisiblecol = _firstVisible/MaterialsPerColumn;
		if ((selectedcol-firstvisiblecol+1)*_xItemSize>Width()) 
		{
			_firstVisible += MaterialsPerColumn;
			_UpdateScrollbar();
		}
		ForceRepaint();
	}
	return true;
}


bool MaterialGallery::MouseMove(KeyState, int, int)
{ 
	SetCursor(_normal);
	return true;
}

void MaterialGallery::_UpdateScrollbar()
{
	if (_xItemSize>0)
	{
		SCROLLINFO si;
		{
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
			si.nMin = 0;
			si.nMax = (_items-1)/MaterialsPerColumn;
			si.nPage = Width() / _xItemSize;
			si.nPos = _firstVisible/MaterialsPerColumn;
		}
		SetScrollInfo(Hwnd(), SB_HORZ, &si, true);
	}
}


int MaterialGallery::_DetermineSelection(int x, int y) const
{
	ScreenPoint sp(x, y);
	WorldPointf wp;
	_MapScreenToWorld(sp, wp);
	int ix = _firstVisible;
	int selection = -1;
#ifdef MATGALRESIZEABLE
	for (int i=0; i<_columns; i++)
#else
	for (int i=0; i<8; i++)
#endif
	{
		for (int j=MaterialsPerColumn-1; j>=0; j--)
		{
#ifdef MATGALRESIZEABLE
			WorldPointf bc(0.25f+0.5f*i, -0.75f+0.5f*j, 0.0f);
#else
			WorldPointf bc(-1.75f+0.5f*i, -0.75f+0.5f*j, 0.0f);
#endif
			if (Distance(wp, bc)<0.21f)
				selection = ix;
			ix++;
		}
	}
	if (selection<0 || selection>=NumOfMaterials)
		selection = -1;
	return selection;
}

int MaterialGallery::_DetermineInsertion(int x, int y) const
{
	int col = x/_xItemSize;
	int row = (y+_xItemSize/2)/_xItemSize;
	int res = col*MaterialsPerColumn+row;
	if ((res<0) || (res>NumOfMaterials))
		res = -1;
	return res;
}


void MaterialGallery::_DrawInsertLine(int i) const
{
	PushPopMatrix ppm;
	GLOffOn lght(GL_LIGHTING);
	GLOffOn depth(GL_DEPTH_TEST);
	i -= _firstVisible;
	int clmn = i/MaterialsPerColumn;
	int row = 3 - i%MaterialsPerColumn;
	glTranslatef(0.25f+0.5f*clmn, -0.75f+0.5f*row, 0.0f);
	GLlines gll;
	gll.Vertex(-0.24f, 0.24f, 0.01f);
	gll.Vertex( 0.24f, 0.24f, 0.01f);
}


void MaterialGallery::_MapScreenToWorld(const ScreenPoint& sp, WorldPointf& wp) const
{
	wp.X(_MinPoint.X() + _upp * sp.X());
	wp.Y(_MinPoint.Y() + _upp * (Height() - sp.Y()));
}




void MaterialGallery::Generate(WriteBinFile& trg) const
{
	for (int i=0; i<NumOfMaterials; i++)
	{
		unsigned char c = static_cast<unsigned char> (i);
		trg.Write(&c, sizeof(unsigned char));
		const Material* pMat = dynamic_cast<const Material*>(_arr[i].get());
		pMat->Generate(trg);
	}
}


void MaterialGallery::Import(const TCHAR* fname)
{
	const int SizeOfMatFileType = 14;
	ReadBinFile src(fname);
	int items = src.Size()/(SizeOfMatFileType+1);
	if (items>NumOfMaterials)
	{
		MessageBox(IDERR_TOOMANYMATERIALSIMPORT, items, NumOfMaterials);
		items = NumOfMaterials;
	}
	for (int i=0; i<items; i++)
	{
		int toput = i;
		{
			unsigned char c;
			src.Read(&c, sizeof(unsigned char));
			int nc = c;
			if (nc<NumOfMaterials)
				toput = nc;
		}
		Material* pMat = dynamic_cast<Material*>(_arr[toput].get());
		pMat->Load(src);
	}
}


void MaterialGallery::ContextMenu(HWND, UINT x, UINT y)
{
	{
		POINT p; p.x = x; p.y = y;
		ScreenToClient(Hwnd(), &p);
		int newselect = _DetermineSelection(p.x, p.y);
		if (-1 != newselect)
		{
			if ((newselect<_FirstSelected) || (newselect>_LastSelected))
			{
				_FirstSelected = newselect;
				_LastSelected = newselect;
				_pEdit->SelectionChanged(_current, newselect);
			}
		}
	}
	HMENU hMenu = App::theApp->GetContextMenu(MaterialGalleryCMenu);
	_AdaptMenu(hMenu);
	TrackPopupMenu
		(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0
		);
}


void MaterialGallery::_AdaptMenu(HMENU hMenu) const
{
	MenuManipulator mm(hMenu);

	if (IsClipboardFormatAvailable(_ClipboardFormat))
		mm.Enable(ID_MATERIALGALLERY_PASTE);
	else
		mm.Disable(ID_MATERIALGALLERY_PASTE);

	if (_LastSelected - _FirstSelected > 1)
		mm.Enable(ID_MATERIALGALLERY_INTERPOLATE);
	else
		mm.Disable(ID_MATERIALGALLERY_INTERPOLATE);
}


bool MaterialGallery::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_MATERIALGALLERY_COPY :
			_Copy();
			break;
		case ID_MATERIALGALLERY_PASTE :
			_PasteInto();
			_pMaterialEdit->ApplyNow();
			break;
		case ID_MATERIALGALLERY_INTERPOLATE :
			_Interpolate();
			_pMaterialEdit->ApplyNow();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}

void MaterialGallery::_PasteInto()
{
	if (IsClipboardFormatAvailable(_ClipboardFormatId()))
	{
		UseClipboard clpbrd(_ClipboardOwner());
		HGLOBAL hMem = GetClipboardData(_ClipboardFormatId());
		MemoryLock lock(hMem);
		_CopyClipboardToSelection(lock.Ptr());
		ForceRepaint();
		_pEdit->Retrieve();
	}
}

void MaterialGallery::_CopyClipboardToSelection(const void* pVoid)
{
	const char* pCur = reinterpret_cast<const char*>(pVoid);
	{
		const int* pItems = reinterpret_cast<const int*>(pCur);
		pCur += sizeof(int);
		assert(*pItems>0);
		{
			const int* pDummy = reinterpret_cast<const int*>(pCur);
			pCur += sizeof(int);
			pDummy = reinterpret_cast<const int*>(pCur);
			pCur += sizeof(int);
		}
		int last = _current + *pItems - 1;
		if (last>=NumOfMaterials)
			last = NumOfMaterials-1;
		for (int i=_current; i<=last; i++)
		{
			Material* pMat = dynamic_cast<Material*>(_GetObject(i));
			pCur = pMat->LoadFromClipboard(pCur);
		}
	}
	_pEdit->Retrieve();
}



void MaterialGallery::_Interpolate()
{
	const int steps = _LastSelected - _FirstSelected;
	assert(steps>1);

	MaterialParams p1;
	{
		const Material* pMat = dynamic_cast<const Material*>(GetObject(_FirstSelected));
		p1 = pMat->GetParams();
	}
	MaterialParams p2;
	{
		const Material* pMat = dynamic_cast<const Material*>(GetObject(_LastSelected));
		p2 = pMat->GetParams();
	}

	GLfloat ambient[4];
	{
		const GLfloat* a2 = p2.GetAmbient();
		const GLfloat* a1 = p1.GetAmbient();
		ambient[0] = (a2[0] - a1[0]) / steps;
		ambient[1] = (a2[1] - a1[1]) / steps;
		ambient[2] = (a2[2] - a1[2]) / steps;
		ambient[3] = (a2[3] - a1[3]) / steps;
	}

	GLfloat diffuse[4];
	{
		const GLfloat* d2 = p2.GetDiffuse();
		const GLfloat* d1 = p1.GetDiffuse();
		diffuse[0] = (d2[0] - d1[0]) / steps;
		diffuse[1] = (d2[1] - d1[1]) / steps;
		diffuse[2] = (d2[2] - d1[2]) / steps;
		diffuse[3] = (d2[3] - d1[3]) / steps;
	}

	GLfloat specular[4];
	{
		const GLfloat* s2 = p2.GetSpecular();
		const GLfloat* s1 = p1.GetSpecular();
		specular[0] = (s2[0] - s1[0]) / steps;
		specular[1] = (s2[1] - s1[1]) / steps;
		specular[2] = (s2[2] - s1[2]) / steps;
		specular[3] = (s2[3] - s1[3]) / steps;
	}

	GLfloat emission[4];
	{
		const GLfloat* e2 = p2.GetEmission();
		const GLfloat* e1 = p1.GetEmission();
		emission[0] = (e2[0] - e1[0]) / steps;
		emission[1] = (e2[1] - e1[1]) / steps;
		emission[2] = (e2[2] - e1[2]) / steps;
		emission[3] = (e2[3] - e1[3]) / steps;
	}
	float shininess = static_cast<float>(p2.GetShininess() - p1.GetShininess())/steps;
	for (int i=1; i<steps; i++)
	{
		Material* pMat = dynamic_cast<Material*>(_GetObject(_FirstSelected+i));
		GLfloat arr[4];
		{
			const GLfloat* a = p1.GetAmbient();
			arr[0] = a[0] + i*ambient[0];
			arr[1] = a[1] + i*ambient[1];
			arr[2] = a[2] + i*ambient[2];
			arr[3] = a[3] + i*ambient[3];
		}
		pMat->SetAmbient(arr);
		{
			const GLfloat* a = p1.GetDiffuse();
			arr[0] = a[0] + i*diffuse[0];
			arr[1] = a[1] + i*diffuse[1];
			arr[2] = a[2] + i*diffuse[2];
			arr[3] = a[3] + i*diffuse[3];
		}
		pMat->SetDiffuse(arr);
		{
			const GLfloat* a = p1.GetSpecular();
			arr[0] = a[0] + i*specular[0];
			arr[1] = a[1] + i*specular[1];
			arr[2] = a[2] + i*specular[2];
			arr[3] = a[3] + i*specular[3];
		}
		pMat->SetSpecular(arr);
		{
			const GLfloat* a = p1.GetEmission();
			arr[0] = a[0] + i*emission[0];
			arr[1] = a[1] + i*emission[1];
			arr[2] = a[2] + i*emission[2];
			arr[3] = a[3] + i*emission[3];
		}
		pMat->SetEmission(arr);

		pMat->SetShininess(static_cast<int>(p1.GetShininess() + i * shininess));

		pMat->SetTransparency(1.0f-arr[3]);
	}


	ForceRepaint();
}
