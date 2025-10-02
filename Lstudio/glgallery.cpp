#include <memory>

#include <fw.h>
#include <glfw.h>

#include "objfgvgallery.h"
#include "glgallery.h"
#include "objfgvobject.h"
#include "objfgvedit.h"
#include "lstudioptns.h"


#include "resource.h"

GLGallery::GLGallery
(
 HWND hwnd, const CREATESTRUCT* pCS,
 float objectDim, int size, 
 bool usetooltips
) : 
OpenGLWindow(hwnd, pCS),
ObjectGallery(size),
_ObjectDim(objectDim),
_TT(Hwnd(), HInstance()),
_useTT(usetooltips),
_normal(Cursor::Arrow),
_drag(MAKEINTRESOURCE(IDC_DRAG))
{
	_firstVisible = 0;
	_xItemSize = 0;
	{
		CurrentContext cc(this);
		COLORREF bg = options.GetGridColors()[Options::eBackground];
		glClearColor
			(
			GetRValue(bg)/255.0f, 
			GetGValue(bg)/255.0f, 
			GetBValue(bg)/255.0f, 
			0.0f
			);
	}

	_dragging = false;
}


GLGallery::~GLGallery()
{
}

void GLGallery::ColorschemeModified()
{
	CurrentContext cc(this);
	COLORREF bg = options.GetGridColors()[Options::eBackground];
	glClearColor
		(
		GetRValue(bg)/255.0f, 
		GetGValue(bg)/255.0f, 
		GetBValue(bg)/255.0f, 
		0.0f
		);
}



void GLGallery::_DoSize()
{
	if (0 == Height())
		return;
	_xItemSize = Height();

	{
		const int CanFit = Width()/_xItemSize;
		int MaxFirst = Items()-CanFit;
		if (MaxFirst<0)
			MaxFirst = 0;
		if (_firstVisible>MaxFirst)
			_firstVisible = MaxFirst;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const float coeff = float(Width())/float(Height());
	glOrtho
		(
		-_ObjectDim/2.0f, _ObjectDim*coeff-_ObjectDim/2.0f, 
		-_ObjectDim/2.0f, _ObjectDim/2.0f, 
		-100.0f, 100.0f
		);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, Width(), Height());
	_UpdateScrollbar();

	// Tooltips stuff
	if (_useTT)
	{
		_TT.Clear(); 
		for (int i=_firstVisible; i<=_LastVisible(); ++i)
		{
			RECT r;
			r.left = _xItemSize*i;
			r.top = 0;
			r.right = r.left + _xItemSize;
			r.bottom = Height();
			_TT.Add(*this, i, r);
		}
	}
}



void GLGallery::_DoPaint() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		PushPopMatrix ppm;
		int x = 0;
		int n = _firstVisible;
		while ((x<Width()) && (n<_items))
		{
			_arr[n]->DrawInGallery();
			glLineWidth(2.0f);
			if (_current == n)
			{
				glColor3fv(options.GetGridColor(Options::eAxis));
				GLlineloop ll;
				ll.Vertex(-0.495f*_ObjectDim, 0.495f*_ObjectDim);
				ll.Vertex( 0.495f*_ObjectDim, 0.495f*_ObjectDim);
				ll.Vertex( 0.495f*_ObjectDim,-0.480f*_ObjectDim);
				ll.Vertex(-0.495f*_ObjectDim,-0.480f*_ObjectDim);
			} 
			else if ((n>=_FirstSelected) && (n<=_LastSelected))
			{
				glColor3fv(options.GetGridColor(Options::eGrid));
				GLlineloop ll;
				ll.Vertex(-0.495f*_ObjectDim, 0.495f*_ObjectDim);
				ll.Vertex( 0.495f*_ObjectDim, 0.495f*_ObjectDim);
				ll.Vertex( 0.495f*_ObjectDim,-0.480f*_ObjectDim);
				ll.Vertex(-0.495f*_ObjectDim,-0.480f*_ObjectDim);
			}
			glLineWidth(1.0f);
			x += Height();
			n++;
			glTranslatef(_ObjectDim, 0.0f, 0.0f);
		}
	}
	glFlush();
}


void GLGallery::_HandleWinSize()
{
	_UpdateScrollbar();
}



bool GLGallery::LButtonDown(KeyState ks, int x, int y)
{
	assert(0 != _pEdit);
	int newselection = _DetermineSelection(x, y);
	if (-1 != newselection)
	{
		if (ks.IsShift())
		{
			if (newselection<_current)
			{
				_FirstSelected = newselection;
				_LastSelected = _current;
			} 
			else if (newselection>_current)
			{
				_FirstSelected = _current;
				_LastSelected = newselection;
			} 
			else
			{
				_FirstSelected = _LastSelected = _current;
			}
			ForceRepaint();
		}
		else
		{
			_FirstSelected = _LastSelected = newselection;
			if (newselection == _current)
			{
				_pEdit->Retrieve();
			}
			else
				_pEdit->SelectionChanged(_current, newselection);
			_dragging = true;
			_dragtarget = _current;
			SetCapture(Hwnd());
		}
	}
	return true;
}


bool GLGallery::LButtonUp(KeyState, int, int)
{
	if (_dragging)
	{
		_DragTo(_dragtarget);
		_dragtarget = -1;
		_dragging = false;
		ReleaseCapture();
		// If the selected item is not fully visible
		// scroll by one
		if ((_FirstSelected-_firstVisible+1)*_xItemSize>Width()) 
		{
			_firstVisible++;
			_UpdateScrollbar();
		}
		ForceRepaint();
	}
	return true;
}

bool GLGallery::MouseMove(KeyState, int x, int y)
{
	if (_dragging)
	{
		SetCursor(_drag);
		int dt = _DetermineInsertion(x, y);
		if ((dt != _dragtarget) && (-1 != dt))
		{
			CurrentContext cc(this);
			GLOnOff clo(GL_COLOR_LOGIC_OP);
			GLOnOff ls(GL_LINE_STIPPLE);
			glLogicOp(GL_XOR);
			glDrawBuffer(GL_FRONT);
			glColor3f(1.0f, 1.0f, 1.0f);
			glLineStipple(1, 0xAAAA);
			if (_dragtarget != _current)
			{
				_DrawInsertLine(_dragtarget);
			}
			_dragtarget = dt;
			if (_dragtarget != _current)
			{
				_DrawInsertLine(_dragtarget);
			}
			glFlush();
			glDrawBuffer(GL_BACK);
		}
	}
	else
		SetCursor(_normal);
	return true;
}


void GLGallery::_DrawInsertLine(int i) const
{
	PushPopMatrix ppm;
	glTranslatef(_ObjectDim*(i-_firstVisible), 0.0f, 0.0f);
	GLlines gll;
	gll.Vertex(-0.495f*_ObjectDim, -0.480f*_ObjectDim);
	gll.Vertex(-0.495f*_ObjectDim,  0.480f*_ObjectDim);
}

bool GLGallery::CaptureChanged()
{
	_dragging = false;
	return true;
}


int GLGallery::_DetermineSelection(int x, int) const
{
	int res = x / _xItemSize + _firstVisible;
	if ((res>=_items) || (res<0))
		res = -1;
	return res;
}

int GLGallery::_DetermineInsertion(int x, int) const
{
	int res = (x+_xItemSize/2) / _xItemSize + _firstVisible;
	if (res>_items)
		res = _items;
	else if (res<0)
		res = 0;
	return res;
}


void GLGallery::ScrollToShow()
{
	assert(_current>=0);
	assert(_current<_items);
	if (_current<_firstVisible)
		_firstVisible = _current;
	else
	{
		while (_current>_LastVisible())
			++_firstVisible;
	}
	_UpdateScrollbar();
	ForceRepaint();
}


void GLGallery::_Delete()
{
	ObjectGallery::_Delete();
	if (_current<_firstVisible)
		_firstVisible = _current;
	int sr = _LastVisible()-_items;
	if (sr>0)
	{
		_firstVisible -= sr;
		if (_firstVisible<0)
			_firstVisible = 0;
	}

	_UpdateScrollbar();
}


void GLGallery::_PasteAfter()
{
	ObjectGallery::_PasteAfter();
	_UpdateScrollbar();
}



void GLGallery::_PasteBefore()
{
	ObjectGallery::_PasteBefore();
	_UpdateScrollbar();
}


void GLGallery::_UpdateScrollbar()
{
	if (_xItemSize>0)
	{
		SCROLLINFO si;
		{
			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
			si.nMin = 0;
			si.nMax = _items-1;
			si.nPage = Width() / _xItemSize;
			si.nPos = _firstVisible;
		}
		SetScrollInfo(Hwnd(), SB_HORZ, &si, true);
	}
}


bool GLGallery::HScroll(HScrollCode code, int pos, HWND)
{
	switch (code)
	{
	case SB_LINELEFT :
		if (_firstVisible>0)
		{
			_firstVisible--;
			_UpdateScrollbar();
			ForceRepaint();
		}
		break;
	case SB_LINERIGHT :
		if (_firstVisible<_items - Width()/_xItemSize)
		{
			_firstVisible++;
			_UpdateScrollbar();
			ForceRepaint();
		}
		break;
	case SB_THUMBPOSITION :
	case SB_THUMBTRACK :
		_firstVisible = pos;
		_UpdateScrollbar();
		ForceRepaint();
		break;
	case SB_PAGELEFT :
		_firstVisible -= Width()/_xItemSize;
		if (_firstVisible<0)
			_firstVisible = 0;
		_UpdateScrollbar();
		ForceRepaint();
		break;
	case SB_PAGERIGHT :
		_firstVisible += Width()/_xItemSize;
		if (_firstVisible>_items - Width()/_xItemSize)
			_firstVisible = _items - Width()/_xItemSize;
		_UpdateScrollbar();
		ForceRepaint();
		break;
	case SB_BOTTOM :
		if (_firstVisible != 0)
		{
			_firstVisible = 0;
			_UpdateScrollbar();
			ForceRepaint();
		}
		break;
	}
	return true;
}


LRESULT GLGallery::Notify(int, const NMHDR* pHdr)
{
	if (_useTT)
	{
		if ((_TT.Is(pHdr->hwndFrom)) && (pHdr->code == TTN_GETDISPINFO))
		{
			NMHDR* pH = const_cast<NMHDR*>(pHdr);
			NMTTDISPINFO* pDI = reinterpret_cast<NMTTDISPINFO*>(pH);
			POINT p; GetCursorPos(&p);
			ScreenToClient(Hwnd(), &p);
			int sel = _DetermineSelection(p.x, p.y);
			if (-1 != sel)
			{
				const TCHAR* nm = _GetObjectName(sel);
				if (0 == nm)
					pDI->szText[0] = 0;
				else
				{
					_tcsncpy(pDI->szText, nm, 79);
					pDI->szText[79] = 0;
				}
			}
			else
				pDI->szText[0] = 0;
		}
	}
	return 0;
}


const TCHAR* GLGallery::_GetObjectName(int) const
{
	return 0;
}


void GLGallery::_DragTo(int target)
{
	if (target == _current || target == _current+1)
		return;

	std::unique_ptr<EditableObject> pDragged(std::move(_arr[_current]));
	if (target<_current)
	{
		for (int i=_current; i>target; i--)
            _arr[i] = std::move(_arr[i-1]);
        _arr[target] = std::move(pDragged);
		_FirstSelected = _LastSelected = _current = target;
	}
	else
	{
		for (int i=_current; i<target-1; i++)
            _arr[i] = std::move(_arr[i+1]);
        _arr[target-1] = std::move(pDragged);
		_FirstSelected = _LastSelected = _current = target-1;
	}
}

