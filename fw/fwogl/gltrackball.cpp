#include <fw.h>
#include <gl\gl.h>

#include "worldpoint.h"
#include "sphere.h"
#include "oglcntxt.h"
#include "openglwnd.h"
#include "gltask.h"
#include "viewbox.h"
#include "gltrackball.h"
#include "glutils.h"


GLTrackball::GLTrackball(HWND hwnd, const CREATESTRUCT* pCS) : OpenGLWindow(hwnd, pCS),
_RotateTask(this)
{
	_rotX = _rotY = 0.0f;

	_pTask = &_RotateTask;
}


GLTrackball::~GLTrackball()
{}


void GLTrackball::ResetRotation()
{
	_rotX = _rotY = 0.0f;
	{
		CurrentContext cc(this);
		_DoOther();
		cc.SwapBuffers();
	}
}


void GLTrackball::_DoSize()
{
	if (Height()>0)
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glViewport(0, 0, Width(), Height());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		_SetViewbox();
		_upp = (_viewbox.MaxX()-_viewbox.MinX())/Width();
		_viewbox.Apply();
		glRotatef(_rotX, 1.0f, 0.0f, 0.0f);
		glRotatef(_rotY, 0.0f, 1.0f, 0.0f);
	}
}

void GLTrackball::RotateBy(int x, int y)
{
	_rotX += y/3.0f;
	if (_rotX > 360.0f)
		_rotX -= 360.0f;
	else if (_rotX < -360.0f)
		_rotX += 360.0f;
	_rotY += x/3.0f;
	if (_rotY > 360.0f)
		_rotY -= 360.0f;
	else if (_rotY < -360.0f)
		_rotY += 360.0f;
	{
		CurrentContext cc(this);
		_DoOther();
		cc.SwapBuffers();
	}
}		


void GLTrackball::MapScreenToWorld(int x, int y, WorldPointf& p) const
{
	p.X(_upp*x + _viewbox.MinX());
	p.Y(_viewbox.MaxY() - _upp*y);
}


void GLTrackball::_DoOther()
{
	_DoSize();
	_DoPaint();
}




bool GLTrackball::LButtonDown(KeyState ks, int x, int y)
{
	_pTask->LButtonDown(ks, x, y);
	return true;
}


bool GLTrackball::LButtonUp(KeyState ks, int x, int y)
{
	_pTask->LButtonUp(ks, x, y);
	return true;
}


bool GLTrackball::MouseMove(KeyState ks, int x, int y)
{
	_pTask->MouseMove(ks, x, y);
	return true;
}


bool GLTrackball::CaptureChanged()
{
	_pTask->Reset();
	return true;
}


