#include <memory>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "matpreview.h"
#include "menuids.h"
#include "matparamcb.h"
#include "materialedit.h"
#include "material.h"

#include "resource.h"


INIT_COUNTER(MaterialPreview);


void MaterialPreview::Register(HINSTANCE hInst)
{
	WndClass wc(hInst, _ClassName(), Wnd<MaterialPreview>::Proc);
	wc.Register();
}


MaterialPreview::MaterialPreview(HWND hwnd, const CREATESTRUCT* pCS) : 
OpenGLWindow(hwnd, pCS)
{
	_pQobj = 0;
	_theEdit = 0;

	_pObj = new Material;
	{
		CurrentContext cc(this);
		_DoInit();
	}	
}


MaterialPreview::~MaterialPreview()
{
	
	{
		CurrentContext cc(this);
		_DoExit();
	}
	
	assert(0 == _pQobj);
}



void MaterialPreview::ApplyAndRedraw()
{
	CurrentContext cc(this);
	Material* pMaterial = _GetMaterial();
	pMaterial->Apply();
	_DoPaint();
	cc.SwapBuffers();
}


void MaterialPreview::_DoSize()
{
	if (0 == Height())
		return;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	const float coeff = static_cast<float>(Width())/static_cast<float>(Height());
	if (coeff>1.0f)
		glOrtho(-1.0f*coeff, 1.0f*coeff, -1.0f, 1.0f, -2.0f, 2.0f);
	else
		glOrtho(-1.0f, 1.0f, -1.0/coeff, 1.0/coeff, -2.0f, 2.0f);
	glMatrixMode(GL_MODELVIEW);
	glViewport(0, 0, Width(), Height());
}


void MaterialPreview::_DoInit()
{
	assert(0 == _pQobj);
	_pQobj = gluNewQuadric();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	{
		GLfloat pos[4] = { 5.0f, 5.0f, 5.0f, 0.0f };
		glLightfv(GL_LIGHT0, GL_POSITION, pos);
		GLfloat amb[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
	}
	Material* pMaterial = _GetMaterial();
	pMaterial->Apply();
}

void MaterialPreview::_DoExit()
{
	gluDeleteQuadric(_pQobj);
	_pQobj = 0;
}


void MaterialPreview::_DoPaint() const
{
	assert(0 != _pQobj);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
	
	{
		GLdisable disableLight(GL_LIGHTING);
		GLdisable disableAlpha(GL_ALPHA_TEST);
		glColor3f(1.0f, 1.0f, 1.0f);
		{
			GLpolygon square;
			square.Vertex(  0.0f,  0.0f);
			square.Vertex(  0.0f, 20.0f);
			square.Vertex(-20.0f, 20.0f);
			square.Vertex(-20.0f,  0.0f);
		}
		{
			GLpolygon square;
			square.Vertex( 0.0f,   0.0f);
			square.Vertex( 0.0f, -20.0f);
			square.Vertex(20.0f, -20.0f);
			square.Vertex(20.0f,   0.0f);
		}
		glColor3f(0.0, 0.0, 0.0);
		{
			GLpolygon square;
			square.Vertex( 0.0f,  0.0f);
			square.Vertex(20.0f,  0.0f);
			square.Vertex(20.0f, 20.0f);
			square.Vertex( 0.0f, 20.0f);
		}
		{
			GLpolygon square;
			square.Vertex(  0.0f,   0.0f);
			square.Vertex(-20.0f,   0.0f);
			square.Vertex(-20.0f, -20.0f);
			square.Vertex(  0.0f, -20.0f);
		}
	}
	
	gluSphere(_pQobj, 0.95f, 24, 24);

	glFlush();
}


void MaterialPreview::SetMaterialEdit(MaterialEdit* pEdit)
{
	assert(0 == _pEdit);
	assert(0 == _theEdit);
	assert(0 != pEdit);
	_theEdit = pEdit;
	_pEdit = pEdit;
}


void MaterialPreview::ContextMenu(HWND, UINT x, UINT y)
{
	HMENU hMenu = App::theApp->GetContextMenu(MaterialPreviewCMenu);
	TrackPopupMenu(
		hMenu,
		TPM_LEFTALIGN | TPM_TOPALIGN,
		x, y,
		0,
		Hwnd(),
		0);
}


bool MaterialPreview::Command(int id, Window, UINT)
{
	try
	{
		switch (id)
		{
		case ID_MATERIALPREVIEW_APPLY :
			_ApplyNow();
			break;
		case ID_MATERIALPREVIEW_RETRIEVE :
			_Retrieve();
			break;
		}
	}
	catch (Exception e)
	{
		ErrorBox(e);
	}
	return true;
}


const Material* MaterialPreview::_GetMaterial() const
{ 
	assert(0 != _pObj);
	return dynamic_cast<const Material*>(_pObj);
}



Material* MaterialPreview::_GetMaterial()
{ 
	assert(0 != _pObj);
	return dynamic_cast<Material*>(_pObj);
}


const MaterialParams& MaterialPreview::GetMaterialParams() const
{ return _GetMaterial()->GetParams(); }


void MaterialPreview::SetTransparency(GLfloat transp)
{
	_GetMaterial()->SetTransparency(transp);
}


void MaterialPreview::SetShininess(int shine)
{
	assert(shine>=0);
	assert(shine<=128);
	_GetMaterial()->SetShininess(shine);
}


void MaterialPreview::SetEmission(const float* emission)
{
	_GetMaterial()->SetEmission(emission);
}


void MaterialPreview::SetSpecular(const float* specular)
{
	_GetMaterial()->SetSpecular(specular);
}


void MaterialPreview::SetDiffuse(const float* diffuse)
{
	_GetMaterial()->SetDiffuse(diffuse);
}


void MaterialPreview::SetAmbient(const float* ambient)
{
	_GetMaterial()->SetAmbient(ambient);
}



