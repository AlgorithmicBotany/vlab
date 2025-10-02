#include <fw.h>
#include <glfw.h>

#include "resource.h"

#include "gridviewtask.h"
#include "gridview.h"
#include "lstudioptns.h"


const LogFont GridView::_logFont(15, __TEXT("Arial"));

const float MinZoom = 0.001f;
const float MaxZoom = 100.0f;
const float ZoomBase = 1.003f;
const float DefaultZoom = 1.2f;
const float MinCenterX = -100.0f;
const float MaxCenterX = 100.0f;
const float MinCenterY = -100.0f;
const float MaxCenterY = 100.0f;

GridView::GridView(HWND hwnd, const CREATESTRUCT* pCS) : 
OpenGLWindow(hwnd, pCS),
_TranslateTask(this),
_ZoomTask(this)
{

	_GridDrawWhat = 0;
	_pTask = 0;
	SetScale(DefaultZoom);
	SetCenter(WorldPointf(0.0f, 0.0f, 0.0f));

	{
		CurrentContext cc(this);
		HDC hDC = wglGetCurrentDC();
		assert(0 != hDC);
		Font font(_logFont);
		ObjectHolder sf(hDC, font);
		{
			TEXTMETRIC tm;
			GetTextMetrics(hDC, &tm);
			_charWidth = tm.tmAveCharWidth;
			_charHeight = tm.tmHeight;
		}
		_fontList.Init(hDC);
		COLORREF bg = options.GetGridColors()[Options::eBackground];
		glClearColor
			(
			GetRValue(bg)/255.0f, 
			GetGValue(bg)/255.0f, 
			GetBValue(bg)/255.0f, 
			0.0f
			);
	}
}


GridView::~GridView()
{}

void GridView::_DoSize()
{
	if (0 == Height())
		return;

	const float wc = static_cast<float>(Width())/static_cast<float>(Height());
	if (wc>1.0f)
	{
		_MinPoint.Set(_center.X()-_scale*wc, _center.Y()-_scale, -1.0f);
		_MaxPoint.Set(_center.X()+_scale*wc, _center.Y()+_scale,  1.0f);
		_upp = (2.0f*_scale) / Height();
	}
	else
	{
		_MinPoint.Set(_center.X()-_scale, _center.Y()-_scale/wc, -1.0f);
		_MaxPoint.Set(_center.X()+_scale, _center.Y()+_scale/wc,  1.0f);
		_upp = (2.0f*_scale) / Width();
	}


	// Y
	{
		int numoflabels = Height() / (3*_charHeight);
		float yrange = _MaxPoint.Y() - _MinPoint.Y();
		assert(yrange>0.0f);
		float step = yrange / numoflabels;
		float relstep = step;
		while (relstep>10.0f)
			relstep *= 0.1f;
		while (relstep<1.0f)
			relstep *= 10.0f;
		float order = step/relstep;
		int Order = static_cast<int>(floorf(0.5f+log10f(order)));
		if (relstep<2.0f)
			relstep = 1.0f;
		else if (relstep<5.0f)
			relstep = 2.0f;
		else
			relstep = 5.0f;
		_GridStep = relstep*order;

		strcpy(_Labelformat, "%.");

		switch (Order)
		{
		case -5 : strcat(_Labelformat, "5f");
			break;
		case -4 : strcat(_Labelformat, "4f");
			break;
		case -3 : strcat(_Labelformat, "3f");
			break;
		case -2 : strcat(_Labelformat, "2f");
			break;
		case -1 : strcat(_Labelformat, "1f");
			break;
		default : strcat(_Labelformat, "0f");
			break;
		}
	}

	{
		int steps = static_cast<int>(_MinPoint.X()/_GridStep)-1;
		if (1 & steps)
			steps--;
		_GridMin.X(_GridStep*steps);
		steps = static_cast<int>(_MaxPoint.X()/_GridStep)+1;
		_GridMax.X(_GridStep*steps);
	}


	{
		int steps = static_cast<int>(_MinPoint.Y()/_GridStep)-1;
		_GridMin.Y(_GridStep*steps);
		steps = static_cast<int>(_MaxPoint.Y()/_GridStep)+1;
		_GridMax.Y(_GridStep*steps);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho
		(
		_MinPoint.X(), _MaxPoint.X(), 
		_MinPoint.Y(), _MaxPoint.Y(), 
		_MinPoint.Z(), _MaxPoint.Z()
		);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, Width(), Height());
}


void GridView::MapScreenToWorld(int x, int y, WorldPointf& wp) const
{
	wp.X(_MinPoint.X() + _upp*x);
	wp.Y(_MaxPoint.Y() - _upp*y);
}

void GridView::Translate(int x, int y)
{
	{
		WorldPointf translate(-_upp*x, _upp*y);
		_center += translate;

		if (_center.X()>MaxCenterX)
			_center.X(MaxCenterX);
		else if (_center.X()<MinCenterX)
			_center.X(MinCenterX);

		if (_center.Y()>MaxCenterY)
			_center.Y(MaxCenterY);
		else if (_center.Y()<MinCenterY)
			_center.Y(MinCenterY);
	}

	{
		CurrentContext cc(this);
		_DoSize();
		_DoPaint();
		cc.SwapBuffers();
	}
}

void GridView::Zoom(int zoom)
{
	assert(zoom != 0);
	if (zoom > 0)
	{
		if (_scale > MaxZoom)
			return;
	}
	else
	{
		if (_scale < MinZoom)
			return;
	}

	_scale *= powf(ZoomBase, static_cast<float>(zoom));

	{
		CurrentContext cc(this);
		_DoSize();
		_DoPaint();
		cc.SwapBuffers();
	}
}


void GridView::SetScale(float scale)
{
	assert(scale>0.0f);
	if (scale > MaxZoom)
		scale = MaxZoom;
	else if (scale < MinZoom)
		scale = MinZoom;
	_scale = scale;
}


void GridView::SetCenter(WorldPointf c)
{
	_center = c;
	if (_center.X()>MaxCenterX)
		_center.X(MaxCenterX);
	else if (_center.X()<MinCenterX)
		_center.X(MinCenterX);

	if (_center.Y()>MaxCenterY)
		_center.Y(MaxCenterY);
	else if (_center.Y()<MinCenterY)
		_center.Y(MinCenterY);
}

void GridView::_PaintGrid() const
{
	if (_GridDrawWhat & eDrawGrid)
	{
		glColor3fv(options.GetGridColor(Options::eGrid));
		GLlines gll;
		for (float x = _GridMin.X(); x<_GridMax.X(); x += _GridStep)
		{
			gll.Vertex(x, _GridMin.Y());
			gll.Vertex(x, _GridMax.Y());
		}
		for (float y = _GridMin.Y(); y<_GridMax.Y(); y += _GridStep)
		{
			gll.Vertex(_GridMin.X(), y);
			gll.Vertex(_GridMax.X(), y);
		}
	}

	if (_GridDrawWhat & eDrawAxis)
	{
		glColor3fv(options.GetGridColor(Options::eAxis));
		GLlines gll;
		gll.Vertex(_GridMin.X(), 0.0f);
		gll.Vertex(_GridMax.X(), 0.0f);
		gll.Vertex(0.0f, _GridMin.Y());
		gll.Vertex(0.0f, _GridMax.Y());
	}

	if (_GridDrawWhat & eDrawLabels)
	{
		glColor3fv(options.GetGridColor(Options::eLabels));
		char bf[32];
		glListBase(_fontList.Base());

		const float yline = _MinPoint.Y() + _upp*4;

		if ((_GridMin.X()>0.0) || (_GridMax.X()<0.0))
		{
			for (float x = _GridMin.X(); x<_GridMax.X(); x += 2*_GridStep)
			{
				int l = sprintf(bf, _Labelformat, x);
				glRasterPos2f(x, yline);
				glCallLists(l, GL_BYTE, bf);
			}
		}
		else
		{
			float x;
			for (x = _GridMin.X(); x< -_GridStep*0.5f; x += 2*_GridStep)
			{
				int l = sprintf(bf, _Labelformat, x);
				glRasterPos2f(x, yline);
				glCallLists(l, GL_BYTE, bf);
			}
			int l = sprintf(bf, _Labelformat, 0.0f);
			glRasterPos2f(0.0f, yline);
			glCallLists(l, GL_BYTE, bf);
			for (x = 2*_GridStep; x<_GridMax.X(); x += 2*_GridStep)
			{
				l = sprintf(bf, _Labelformat, x);
				glRasterPos2f(x, yline);
				glCallLists(l, GL_BYTE, bf);
			}
		}

		const float xline = _MinPoint.X() + _upp*4;

		if ((_GridMin.Y()>0.0f) || (_GridMax.X()<0.0f))
		{
			for (float y = _GridMin.Y()+2*_GridStep; y<_GridMax.Y(); y += _GridStep)
			{
				const int l = sprintf(bf, _Labelformat, y);
				glRasterPos2f(xline, y+_upp*3.0f);
				glCallLists(l, GL_BYTE, bf);
			}
		}
		else
		{
			int l; float y;
			for (y = _GridMin.Y()+_GridStep; y< -_GridStep*0.5f; y += _GridStep)
			{
				l = sprintf(bf, _Labelformat, y);
				glRasterPos2f(xline, y+_upp*3.0f);
				glCallLists(l, GL_BYTE, bf);
			}
			l = sprintf(bf, _Labelformat, 0.0f);
			glRasterPos2f(xline, _upp*3.0f);
			glCallLists(l, GL_BYTE, bf);
			for (y = _GridStep; y<_GridMax.Y(); y += _GridStep)
			{
				l = sprintf(bf, _Labelformat, y);
				glRasterPos2f(xline, y+_upp*3.0f);
				glCallLists(l, GL_BYTE, bf);
			}
		}
	}
}


bool GridView::LButtonDown(KeyState ks, int x, int y)
{
	GrabFocus();
	_pPrevTask = _pTask;

	if (ks.IsShift())
	{
		_pTask = &_TranslateTask;
		_pTask->Reset();
	} 
	else if (ks.IsCtrl())
	{
		_pTask = &_ZoomTask;
		_pTask->Reset();
	} 

	_pTask->LButtonDown(ks, x, y);
	return true;
}

bool GridView::LBDblClick(KeyState ks, int x, int y)
{
	GrabFocus();
	_pPrevTask = _pTask;

	if (ks.IsShift())
	{
		_pTask = &_TranslateTask;
		_pTask->Reset();
	} 
	else if (ks.IsCtrl())
	{
		_pTask = &_ZoomTask;
		_pTask->Reset();
	} 

	_pTask->LBDblClick(ks, x, y);
	return true;
}


bool GridView::MButtonDown(KeyState ks, int x, int y)
{
	_pPrevTask = _pTask;

	_pTask = &_ZoomTask;
	_pTask->Reset();
	_pTask->MButtonDown(ks, x, y);
	return true;
}


bool GridView::LButtonUp(KeyState ks, int x, int y)
{
	_pTask->LButtonUp(ks, x, y);
	_pTask = _pPrevTask;
	return true;
}

bool GridView::MButtonUp(KeyState ks, int x, int y)
{
	return LButtonUp(ks, x, y);
}


bool GridView::MouseMove(KeyState ks, int x, int y)
{
	_pTask->MouseMove(ks, x, y);
	return true;
}


void GridView::SetDrawAxis(bool set)
{
	if (set)
		_GridDrawWhat |= eDrawAxis;
	else
		_GridDrawWhat &= ~(eDrawAxis);
}

void GridView::SetDrawGrid(bool set)
{
	if (set)
		_GridDrawWhat |= eDrawGrid;
	else
		_GridDrawWhat &= ~(eDrawGrid);
}

void GridView::SetDrawLabels(bool set)
{
	if (set)
		_GridDrawWhat |= eDrawLabels;
	else
		_GridDrawWhat &= ~(eDrawLabels);
}


void GridView::ColorschemeModified()
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


bool GridView::CaptureChanged()
{
	_pTask->LButtonUp(KeyState(0), 0, 0);
	_pTask = _pPrevTask;
	return true;
}
