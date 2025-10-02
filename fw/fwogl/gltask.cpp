#include <fw.h>
#include <gl\gl.h>

#include "worldpoint.h"
#include "sphere.h"
#include "gltask.h"
#include "oglcntxt.h"
#include "openglwnd.h"
#include "viewbox.h"
#include "gltrackball.h"
#include "rotation.h"


GLRotateTask::GLRotateTask(GLTrackball* pView) : GLTask(pView, "ROTATE")
{ 
	_rotating = false; 
}

void GLRotateTask::LButtonDown(KeyState, int x, int y)
{
	_PrevPos.x = x;
	_PrevPos.y = y;
	_rotating = true;
	SetCapture(_pView->Hwnd());
}


void GLRotateTask::LButtonUp(KeyState, int, int)
{
	if (_rotating)
	{
		ReleaseCapture();
		_rotating = false;
	}
}


void GLRotateTask::MouseMove(KeyState, int x, int y)
{
	if (_rotating)
	{
		_pView->RotateBy(x - _PrevPos.x, y - _PrevPos.y);
		_PrevPos.x = x;
		_PrevPos.y = y;
	}
	GLTask::MouseMove(KeyState(0),x,y);
}
