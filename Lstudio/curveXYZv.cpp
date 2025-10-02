// source file curveXYZv.c++


#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <fw.h>
#include <glfw.h>

#include "prjnotifysnk.h"
#include "objfgvobject.h"
#include "objfgvedit.h"
#include "objfgvview.h"

#include "curveview.h"
#include "curveedit.h"
#include "curveXYZv.h"


#ifndef WIN32
#define _stricmp strcmp
#endif

#define CONFIGFILE "3Dedit1.4.cfg"


CVector::CVector()
{
	x = 0.0;
	y = 0.0;
	z = 0.0;
}


void CVector::SAdd(float s)
{
	x = x + s;
	y = y + s;
	z = z + s;
}


void CVector::VAdd(CVector hV)
{
	x = x + hV.x;
	y = y + hV.y;
	z = z + hV.z;
}


void CVector::VSub(CVector hV)
{
	x = x - hV.x;
	y = y - hV.y;
	z = z - hV.z;
}


void CVector::SMult(float s)
{
	x = s*x;
	y = s*y;
	z = s*z;
}


float CVector::SProduct(CVector hV)
{
	float s;
	
	s = x*hV.x + y*hV.y + z*hV.z;
	
	return s;
}


void CVector::VProduct(CVector hV1, CVector hV2)
{
	x = hV1.y*hV2.z - hV1.z*hV2.y;
	y = hV1.z*hV2.x - hV1.x*hV2.z;
	z = hV1.x*hV2.y - hV1.y*hV2.x;
}


void CVector::NVector()
{
	float l;
	
	l = sqrtf(x*x + y*y + z*z);
	
	x = x/l;
	y = y/l;
	z = z/l; 
}


float CVector::VLength()
{
	float l;
	
	l = sqrtf(x*x + y*y + z*z);
	
	return l;
}


float CVector::XYAngle()
{
	float l, theta;
	
	l = sqrtf(x*x + y*y);
	
	if ( y > 0 )
	{
		theta = acosf(x/l);
	}
	else
	{
		theta = -acosf(x/l);
	}
	
	return (theta/M_PIf*180.0f);
}


float CVector::YZAngle()
{
	float l, theta;
	
	l = sqrtf(y*y + z*z);
	
	if ( z > 0 )
	{
		theta = acosf(y/l);
	}
	else
	{
		theta = -acosf(y/l);
	}
	
	return (theta/M_PIf*180.0f);
}


float CVector::XZAngle()
{
	float l, theta;
	
	l = sqrtf(x*x + z*z);
	
	if ( z > 0 )
	{
		theta = acosf(x/l);
	}
	else
	{
		theta = -acosf(x/l);
	}
	
	return (theta/M_PIf*180.0f);
}


void CVector::setPoint(CPoint hP)
{
	x = hP.x;
	y = hP.y;
	z = hP.z;
}


void CVector::setPoints(CPoint hP1, CPoint hP2)
{
	x = hP2.x - hP1.x;
	y = hP2.y - hP1.y;
	z = hP2.z - hP1.z;
}


CPoint CVector::getPoint()
{
	CPoint hP;
	
	hP.x = x;
	hP.y = y;
	hP.z = z;
	
	return hP;
}


CMatrix::CMatrix()
{
	int i, j;
	
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if ( i == j)
				Coeff[i][j] = 1;
			else
				Coeff[i][j] = 0;
		}
	}
}


void CMatrix::SetRX(float alpha)
{
	float theta;
	
	theta = alpha/180.0f*M_PIf;
	
	Coeff[1][1] =  cosf(theta);
	Coeff[1][2] = -sinf(theta);
	Coeff[2][1] =  sinf(theta);
	Coeff[2][2] =  cosf(theta);
}


void CMatrix::SetRY(float beta)
{
	float theta;
	
	theta = beta/180.0f*M_PIf;
	
	Coeff[0][0] =  cosf(theta);
	Coeff[0][2] =  sinf(theta);
	Coeff[2][0] = -sinf(theta);
	Coeff[2][2] =  cosf(theta);
}


void CMatrix::SetRZ(float gamma)
{
	float theta;
	
	theta = gamma/180.0f*M_PIf;
	
	Coeff[0][0] =  cosf(theta);
	Coeff[0][1] = -sinf(theta);
	Coeff[1][0] =  sinf(theta);
	Coeff[1][1] =  cosf(theta);
}


void CMatrix::Mult(CMatrix hM)
{
	int     i, j, k;
	CMatrix nM;
	
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			nM.Coeff[i][j] = 0;
			for (k = 0; k < 3; k++)
			{
				nM.Coeff[i][j] += Coeff[i][k]*hM.Coeff[k][j];
			}
		}
	}
	
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			Coeff[i][j] = nM.Coeff[i][j];
		}
	}
}


CVector CMatrix::CalcVector( CVector hV )
{
	CVector nV;
	
	nV.x = Coeff[0][0]*hV.x + Coeff[0][1]*hV.y + Coeff[0][2]*hV.z;
	nV.y = Coeff[1][0]*hV.x + Coeff[1][1]*hV.y + Coeff[1][2]*hV.z;
	nV.z = Coeff[2][0]*hV.x + Coeff[2][1]*hV.y + Coeff[2][2]*hV.z;
	
	return nV;
}


void CMatrix::SetRotate( CVector hV, float alpha )
{
	float s, c, t, theta;
	
	theta = alpha/180.0f*M_PIf;
	
	s = sinf(theta);
	c = cosf(theta);
	t = 1 - c;
	
	Coeff[0][0] = t*hV.x*hV.x + c;
	Coeff[0][1] = t*hV.x*hV.y - s*hV.z;
	Coeff[0][2] = t*hV.x*hV.z + s*hV.y;
	
	Coeff[1][0] = t*hV.x*hV.y + s*hV.z;
	Coeff[1][1] = t*hV.y*hV.y + c;
	Coeff[1][2] = t*hV.y*hV.z - s*hV.x;
	
	Coeff[2][0] = t*hV.x*hV.z - s*hV.y;
	Coeff[2][1] = t*hV.y*hV.z + s*hV.x;
	Coeff[2][2] = t*hV.z*hV.z + c;
}


void CMatrix::SetRotate2(CVector hVt1, CVector hVn1, CVector hVb1,
						 CVector hVt2, CVector hVn2, CVector hVb2)
{
	Coeff[0][0] = hVt2.SProduct(hVt1);
	Coeff[0][1] = hVt2.SProduct(hVn1);
	Coeff[0][2] = hVt2.SProduct(hVb1);
	
	Coeff[1][0] = hVn2.SProduct(hVt1);
	Coeff[1][1] = hVn2.SProduct(hVn1);
	Coeff[1][2] = hVn2.SProduct(hVb1);
	
	Coeff[2][0] = hVb2.SProduct(hVt1);
	Coeff[2][1] = hVb2.SProduct(hVn1);
	Coeff[2][2] = hVb2.SProduct(hVb1);
}


CurveXYZView::CurveXYZView()
{
	_curveEdit = 0;
	_curveView = 0;
	
	_curveXYZ = new CCurveXYZ;
	
	_width = _height = 100;
	
	_axes = false;
	_grid = _gridXY = true;
	_gridXZ = _gridYZ = false;
	
	_dpoints = _dconvex = _dcurve = true;
	
	_selecti = _selectl = _selectp = 0;
	_selectc = 0;
	
	_undoindex = _undocount = 0;
	_undosave = false;
	
	_clear_r = _clear_g = _clear_b = 0;
	
	_xaxis_radius = 1.5;
	_xaxis_length = 24;
	_xaxis_base = 4;
	_xaxis_top = 0;
	_xaxis_height = 8;
	_xaxis_slices = 8;
	_xaxis_stacks = 1;
	_xaxis_r = 1.0;
	_xaxis_g = 0.0;
	_xaxis_b = 0.0;
	
	_yaxis_radius = 1.5;
	_yaxis_length = 24;
	_yaxis_base = 4;
	_yaxis_top = 0;
	_yaxis_height = 8;
	_yaxis_slices = 8;
	_yaxis_stacks = 1;
	_yaxis_r = 0.0;
	_yaxis_g = 1.0;
	_yaxis_b = 0.0;
	
	_zaxis_radius = 1.5;
	_zaxis_length = 24;
	_zaxis_base = 4;
	_zaxis_top = 0;
	_zaxis_height = 8;
	_zaxis_slices = 8;
	_zaxis_stacks = 1;
	_zaxis_r = 0.0;
	_zaxis_g = 0.0;
	_zaxis_b = 1.0;
	
	_grid_size[0] = 32;
	_grid_size[1] = 1;
	
	_gridxy_radius = 0.5;
	_gridxy_slices = 8;
	_gridxy_stacks = 1;
	_gridxy_r = 0.4f;
	_gridxy_g = 0.4f;
	_gridxy_b = 0.4f;
	
	_gridxz_radius = 0.5;
	_gridxz_slices = 8;
	_gridxz_stacks = 1;
	_gridxz_r = 0.4f;
	_gridxz_g = 0.4f;
	_gridxz_b = 0.4f;
	
	_gridyz_radius = 0.5;
	_gridyz_slices = 8;
	_gridyz_stacks = 1;
	_gridyz_r = 0.4f;
	_gridyz_g = 0.4f;
	_gridyz_b = 0.4f;
	
	_ppoint_radius = 2.6f;
	_ppoint_slices = 16;
	_ppoint_stacks = 16;
	_ppoint_r = 1.0f;
	_ppoint_g = 1.0f;
	_ppoint_b = 1.0f;
	
	_vpoint_radius = 2.6f;
	_vpoint_slices = 16;
	_vpoint_stacks = 16;
	_vpoint_r = 0.8f;
	_vpoint_g = 0.8f;
	_vpoint_b = 0.8f;
	
	_pconvex_radius = 0.5;
	_pconvex_slices = 8;
	_pconvex_stacks = 1;
	_pconvex_r = 0.6f;
	_pconvex_g = 0.6f;
	_pconvex_b = 1.0f;
	
	_vconvex_radius = 0.5;
	_vconvex_slices = 8;
	_vconvex_stacks = 1;
	_vconvex_r = 0.4f;
	_vconvex_g = 0.4f;
	_vconvex_b = 1.0f;
	
	_pcurve_radius = 0.51f;
	_pcurve_slices = 8;
	_pcurve_stacks = 1;
	_pcurve_points = 26;
	_pcurve_lines = 13;
	_pcurve_r = 1.0f;
	_pcurve_g = 1.0f;
	_pcurve_b = 0.0f;
	
	_vcurve_radius = 0.5;
	_vcurve_slices = 8;
	_vcurve_stacks = 1;
	_vcurve_points = 26;
	_vcurve_lines = 13;
	_vcurve_r = 0.8f;
	_vcurve_g = 0.8f;
	_vcurve_b = 0.0f;
	
	_selectb_radius = 0.51f;
	_selectb_length = 8;
	_selectb_slices = 8;
	_selectb_stacks = 1;
	_selectb_r = 1.0f;
	_selectb_g = 1.0f;
	_selectb_b = 1.0f;
	
	_selectx_radius = 1.5;
	_selectx_length = 16;
	_selectx_base = 4;
	_selectx_top = 0;
	_selectx_height = 8;
	_selectx_slices = 8;
	_selectx_stacks = 1;
	_selectx_r[0] = 1.0f;
	_selectx_g[0] = 0.0f;
	_selectx_b[0] = 0.0f;
	_selectx_r[1] = 1.0f;
	_selectx_g[1] = 1.0f;
	_selectx_b[1] = 1.0f;
	
	_selecty_radius = 1.5;
	_selecty_length = 16;
	_selecty_base = 4;
	_selecty_top = 0;
	_selecty_height = 8;
	_selecty_slices = 8;
	_selecty_stacks = 1;
	_selecty_r[0] = 0.0;
	_selecty_g[0] = 1.0;
	_selecty_b[0] = 0.0;
	_selecty_r[1] = 1.0;
	_selecty_g[1] = 1.0;
	_selecty_b[1] = 1.0;
	
	_selectz_radius = 1.5;
	_selectz_length = 16;
	_selectz_base = 4;
	_selectz_top = 0;
	_selectz_height = 8;
	_selectz_slices = 8;
	_selectz_stacks = 1;
	_selectz_r[0] = 0.0;
	_selectz_g[0] = 0.0;
	_selectz_b[0] = 1.0;
	_selectz_r[1] = 1.0;
	_selectz_g[1] = 1.0;
	_selectz_b[1] = 1.0;
	
	_mouse_rotXY   = 0.5;
	_mouse_scale   = 1.0;
	
	_stat_clear_r = _stat_clear_g = _stat_clear_b = 0.0;
	_stat_font_r = _stat_font_g = _stat_font_b = 0.5;
	
	_arcball_wrap = _arcball_circle = _arcball_constraints = _arcball_arc = true;
	_arcball_circle_radius = _arcball_constraints_radius = _arcball_arc_radius = 0.5;
	_arcball_circle_r = _arcball_constraints_r[0] = _arcball_constraints_r[1] = _arcball_arc_r = 0.4f;
	_arcball_circle_g = _arcball_constraints_g[0] = _arcball_constraints_g[1] = _arcball_arc_g = 0.4f;
	_arcball_circle_b = _arcball_constraints_b[0] = _arcball_constraints_b[1] = _arcball_arc_b = 0.4f;
	
	_arrowX = _arrowY = _arrowZ = false;
	
	_xcoeff = _ycoeff = 1.0;
	
	_pointer = true;
	
	_rot = _rotTB = _wrap = false;
	_arcball = true;
	_trans = _transTB = false;
	_scale = _scaleTB = false;
	
	initView();
	fitPoints();
	
	strcpy(_dirname, ".");
	strcpy(_filename, "unnamed.s");
	
	_copy = 0;
	
	initConfig();
	
	rimColor[0] = _arcball_circle_r;
	rimColor[1] = _arcball_circle_g;
	rimColor[2] = _arcball_circle_b;
	farColor[0] = _arcball_constraints_r[0];
	farColor[1] = _arcball_constraints_g[0];
	farColor[2] = _arcball_constraints_b[0];
	nearColor[0] = _arcball_constraints_r[1];
	nearColor[1] = _arcball_constraints_g[1];
	nearColor[2] = _arcball_constraints_b[1];
	dragColor[0] = _arcball_arc_r;
	dragColor[1] = _arcball_arc_g;
	dragColor[2] = _arcball_arc_b;
}


CurveXYZView::~CurveXYZView()
{
	delete _curveXYZ;
}


void CurveXYZView::Copy(const EditableObject* src)
{
	const CurveXYZView* curveXYZView = 
		dynamic_cast<const CurveXYZView*>(src);
	
	strcpy(_filename, curveXYZView->_filename);
	
	*_curveXYZ = *(curveXYZView->_curveXYZ);
	
	_selectc = 0;
	
	fitPoints();
}


EditableObject* CurveXYZView::Clone() const
{
	CurveXYZView *curveXYZView;
	
	curveXYZView = new CurveXYZView;
	
	strcpy(curveXYZView->_filename, _filename);
	
	*(curveXYZView->_curveXYZ) = *_curveXYZ;
	
	curveXYZView->_selectc = 0;
	
	curveXYZView->fitPoints();
	
	return curveXYZView;
}


void CurveXYZView::DrawInGallery() const
{
	CPoint hP;
	int    i, j, k;
	float t, s;
	
	glPushMatrix();
	
	glScalef(_scaleX, _scaleY, _scaleZ);
	glTranslatef(_transX, _transY, _transZ);
	
	glLineWidth(float(2*_pcurve_radius));
	glColor3f(_pcurve_r, _pcurve_g, _pcurve_b);
	
	_curveXYZ->createPPoints(_curveXYZ->_curve);
	
	if ( _curveXYZ->_curve->_ctype == eBEZIERSURFACE )
	{
		for (i = 0; i < _curveXYZ->_curve->_csize/16; i++)
		{
			for (j = 0; j < _pcurve_lines; j++)
			{
				s = (float)j/(_pcurve_lines-1);				
				glBegin( GL_LINE_STRIP );
				for (k= 0; k < _pcurve_lines; k++)
				{
					t = (float)k/(_pcurve_lines-1);
					hP = _curveXYZ->calcPValue(i, t, s, 0.0);
					glVertex3f(hP.x, hP.y, hP.z);
				}
				glEnd();
			}
			
			for (j = 0; j < _pcurve_lines; j++)
			{
				s = (float)j/(_pcurve_lines-1);
				glBegin( GL_LINE_STRIP ); 
				for ( k = 0; k < _pcurve_lines; k++)
				{
					t = (float)k/(_pcurve_lines-1);
					hP = _curveXYZ->calcPValue(i, s, t, 0.0);
					glVertex3f(hP.x, hP.y, hP.z);
				}
				glEnd();
			}
		}
	}
	else
	{
		_curveXYZ->createAPoints();
		
		glBegin( GL_LINE_STRIP ); 
		for (i = 0; i < _curveXYZ->_curve->_csize*_pcurve_points; i++)
		{
			s = (float)i/(_curveXYZ->_curve->_csize*_pcurve_points - 1);
			hP = _curveXYZ->calcAValue(0,s,0,0);
			glVertex3f(hP.x, hP.y, hP.z);
		}
		glEnd();
	}
	
	glPopMatrix();
}


void CurveXYZView::initView()
{
	_rotX = _rotY = 0.0;
	_transX = _transY = _transZ = 0.0;
	_scaleX = _scaleY = _scaleZ = 1.0;
	
	Ball_Init(&ball);
	Ball_Place(&ball, qOne, RADIUS);
}


void CurveXYZView::addPointPred()
{
	int    p;
	float x, y, z;
	
	if ( _selectc != 0 )
	{
		if ( _selectp == 0 )
			p = 1;
		else
			p = -1;
		
		x = _selectc->_point[_selectp]._point.x + float(p)/2*
			(_selectc->_point[_selectp]._point.x -
			_selectc->_point[_selectp+p]._point.x);
		y = _selectc->_point[_selectp]._point.y + float(p)/2*
			(_selectc->_point[_selectp]._point.y -
			_selectc->_point[_selectp+p]._point.y);
		z = _selectc->_point[_selectp]._point.z + float(p)/2*
			(_selectc->_point[_selectp]._point.z -
			_selectc->_point[_selectp+p]._point.z);
		
		_curveXYZ->addPoint(_selectc, _selectp, x, y, z);
		
		updateGraph();
	}
}


void CurveXYZView::addPointSucc()
{
	int    p;
	float x, y, z;
	
	if ( _selectc != 0 )
	{
		if ( _selectp == _selectc->_csize-1 )
			p = -1;
		else
			p = 1;
		
		x = _selectc->_point[_selectp]._point.x + float(p)/2*
			(_selectc->_point[_selectp+p]._point.x -
			_selectc->_point[_selectp]._point.x);
		y = _selectc->_point[_selectp]._point.y + float(p)/2*
			(_selectc->_point[_selectp+p]._point.y -
			_selectc->_point[_selectp]._point.y);
		z = _selectc->_point[_selectp]._point.z + float(p)/2*
			(_selectc->_point[_selectp+p]._point.z -
			_selectc->_point[_selectp]._point.z);
		
		_curveXYZ->addPoint(_selectc, _selectp+1, x, y, z);
		
		updateGraph();
	}
}


void CurveXYZView::delPoint()
{
	if ( _selectc != 0 )
	{
		if ( _selectc->_csize > 4 )
		{
			_curveXYZ->deletePoint(_selectc, _selectp);
			
			updateGraph();
		}
	}
}


void CurveXYZView::addSurfaceNS(int row)
{
	int    c, i, j;
	float x, y, z;
	
	if ( _selectc != 0 )
	{
		if ( _selectc->_ctype == eBEZIERSURFACE )
		{
			c = _selectp/16;
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 4; j++)
				{
					x=2*_selectc->_point[c*16+row*4+j]._point.x - 
						_selectc->_point[c*16+(3-i)*4+j]._point.x;
					y=2*_selectc->_point[c*16+row*4+j]._point.y - 
						_selectc->_point[c*16+(3-i)*4+j]._point.y;      
					z=2*_selectc->_point[c*16+row*4+j]._point.z - 
						_selectc->_point[c*16+(3-i)*4+j]._point.z;
					
					_curveXYZ->addPoint(_selectc, (c+1)*16+i*4+j, x, y, z); 
				}
			}
			
			updateGraph();
		}
	}
}


void CurveXYZView::addSurfaceWE(int col)
{
	int    c, i, j;
	float x, y, z;
	
	if ( _selectc != 0 )
	{
		if ( _selectc->_ctype == eBEZIERSURFACE )
		{
			c = _selectp/16;
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 4; j++)
				{
					x=2*_selectc->_point[c*16+i*4+col]._point.x - 
						_selectc->_point[c*16+i*4+(3-j)]._point.x;
					y=2*_selectc->_point[c*16+i*4+col]._point.y - 
						_selectc->_point[c*16+i*4+(3-j)]._point.y;      
					z=2*_selectc->_point[c*16+i*4+col]._point.z - 
						_selectc->_point[c*16+i*4+(3-j)]._point.z;
					
					_curveXYZ->addPoint(_selectc, (c+1)*16+i*4+j, x, y, z); 
				}
			}
			
			updateGraph();
		}
	}
}


void CurveXYZView::addSurfaceNESESWNW(int row, int col)
{
	int    c, i, j;
	float x, y, z;
	
	if ( _selectc != 0 )
	{
		if ( _selectc->_ctype == eBEZIERSURFACE )
		{
			c = _selectp/16;
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 4; j++)
				{
					x=2*_selectc->_point[c*16+row*4+col]._point.x - 
						_selectc->_point[c*16+(3-i)*4+(3-j)]._point.x;
					y=2*_selectc->_point[c*16+row*4+col]._point.y - 
						_selectc->_point[c*16+(3-i)*4+(3-j)]._point.y;      
					z=2*_selectc->_point[c*16+row*4+col]._point.z - 
						_selectc->_point[c*16+(3-i)*4+(3-j)]._point.z;
					
					_curveXYZ->addPoint(_selectc, (c+1)*16+i*4+j, x, y, z); 
				}
			}
			
			updateGraph();
		}
	}
}


void CurveXYZView::delSurface()
{
	int c, i, j;
	
	if ( _selectc != 0 )
	{
		if ( _selectc->_ctype == eBEZIERSURFACE )
		{
			if ( _selectc->_csize > 16)
			{
				c = _selectp/16;
				for (i = 0; i < 4; i++)
				{
					for (j = 0; j < 4; j++)
					{
						_curveXYZ->deletePoint(_selectc, c*16+(3-i)*4+(3-j));
					}
				}				
				updateGraph();
			}
		}
	}
}


void CurveXYZView::setRotAxes(AxisSet hA)
{
	Ball_UseSet(&ball, hA);
	
	Ball_BeginDrag(&ball);
	Ball_EndDrag(&ball);
	
	updateGraph();
}


void CurveXYZView::undoSave()
{
	int i;
	
	if ( _undoindex == MAXBSPLINEBZUNDO )
	{
		_undoindex--;
		for (i = 0; i < _undoindex; i++)
		{
			_undo[i] = _undo[i+1];
		}
	}
	
	_undo[_undoindex] = *_curveXYZ;
	_undoindex++;
	_undocount = _undoindex;
}


void CurveXYZView::undoUndo()
{
	if ( _undoindex > 0 )
	{
		if ( _undoindex == _undocount )
		{
			undoSave();
			
			_undoindex--;
		}
		
		_undoindex--;
		
		*_curveXYZ = _undo[_undoindex];
		
		updateGraph();
	}
	else
	{
		_curveEdit->updateStat("no more undo...");
	}
}


void CurveXYZView::undoRedo()
{
	if ( _undoindex < _undocount-1 )
	{
		_undoindex++;
		
		*_curveXYZ = _undo[_undoindex];
		
		updateGraph();
	}
	else
	{
		_curveEdit->updateStat( "no more redo..." );
	}
}

void CurveXYZView::calcGrid()
{
	float d;
	int    i, j, k;
	
	d = 2.0f*_xcoeff/float(int(_width/_grid_size[0]))/(_scaleX*RADIUS);
	
	_grid_size[1] = 0.0000001f;
	for (i = -7; i < 8; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if ( j < 2 )
				k = j+1;
			else
				k = 5;
			
			if ( k*pow(10.0,i) < d )
			{
				_grid_size[1] = k*powf(10.0f,(float)i);
			}
		}
	}
}


void CurveXYZView::drawAxes(GLenum mode)
{
	float         r, l, b, t, h;
	GLUquadricObj *quad = gluNewQuadric();
	
	if ( (mode == GL_RENDER) && _axes )
	{
		glPushMatrix();
		
		glTranslatef(int(-_transX/_grid_size[1])*_grid_size[1],
			int(-_transY/_grid_size[1])*_grid_size[1], 
			int(-_transZ/_grid_size[1])*_grid_size[1]);
		
		// x-axis
		r = 2.0f*_xcoeff/_width*_xaxis_radius/(_scaleX*RADIUS);
		l = 2.0f*_xcoeff/_width*_xaxis_length/(_scaleX*RADIUS);
		b = 2.0f*_xcoeff/_width*_xaxis_base/(_scaleX*RADIUS);
		t = 2.0f*_xcoeff/_width*_xaxis_top/(_scaleX*RADIUS);
		h = 2.0f*_xcoeff/_width*_xaxis_height/(_scaleX*RADIUS);
		
		glColor3f(_xaxis_r, _xaxis_g, _xaxis_b);
		
		glPushMatrix();
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		gluCylinder(quad, r, r, l, _xaxis_slices, _xaxis_stacks);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(l, 0.0f, 0.0);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		gluCylinder(quad, b, t, h, _xaxis_slices, _xaxis_stacks);
		glPopMatrix();
		
		// y-axis
		r = 2.0f*_xcoeff/_width*_yaxis_radius/(_scaleX*RADIUS);
		l = 2.0f*_xcoeff/_width*_yaxis_length/(_scaleX*RADIUS);
		b = 2.0f*_xcoeff/_width*_yaxis_base/(_scaleX*RADIUS);
		t = 2.0f*_xcoeff/_width*_yaxis_top/(_scaleX*RADIUS);
		h = 2.0f*_xcoeff/_width*_yaxis_height/(_scaleX*RADIUS);
		
		glColor3f(_yaxis_r, _yaxis_g, _yaxis_b);
		
		glPushMatrix();
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(quad, r, r, l, _yaxis_slices, _yaxis_stacks);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(0.0f, l, 0.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(quad, b, t, h, _yaxis_slices, _yaxis_stacks);
		glPopMatrix();
		
		// z-axis
		r = 2.0f*_xcoeff/_width*_zaxis_radius/(_scaleX*RADIUS);
		l = 2.0f*_xcoeff/_width*_zaxis_length/(_scaleX*RADIUS);
		b = 2.0f*_xcoeff/_width*_zaxis_base/(_scaleX*RADIUS);
		t = 2.0f*_xcoeff/_width*_zaxis_top/(_scaleX*RADIUS);
		h = 2.0f*_xcoeff/_width*_zaxis_height/(_scaleX*RADIUS);
		
		glColor3f(_zaxis_r, _zaxis_g, _zaxis_b);
		
		glPushMatrix();
		gluCylinder(quad, r, r, l, _zaxis_slices, _zaxis_stacks);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, l);
		gluCylinder(quad, b, t, h, _zaxis_slices, _zaxis_stacks);
		glPopMatrix();
		
		glPopMatrix();
	}
	gluDeleteQuadric(quad);
}


void CurveXYZView::drawGrid(GLenum mode)
{
	float xmin, xmax, ymin, ymax, zmin, zmax, p, l;
	
	if ( (mode == GL_RENDER) && _grid && (_gridXY || _gridXZ || _gridYZ) )
	{
		xmin = -(1/_scaleX)-_transX;
		if ( xmin > _xmin )
			xmin = _xmin;
		xmin = int(xmin/_grid_size[1])*_grid_size[1];
		xmax = (1/_scaleX)-_transX;
		if ( xmax < _xmax )
			xmax = _xmax;
		xmax = int(xmax/_grid_size[1])*_grid_size[1];
		
		ymin = -(1/_scaleY)-_transY;
		if ( ymin > _ymin )
			ymin = _ymin;
		ymin = int(ymin/_grid_size[1])*_grid_size[1];
		ymax = (1/_scaleY)-_transY;
		if ( ymax < _ymax )
			ymax = _ymax;
		ymax = int(ymax/_grid_size[1])*_grid_size[1];
		
		zmin = -(1/_scaleZ)-_transZ;
		if ( zmin > _zmin )
			zmin = _zmin;
		zmin = int(zmin/_grid_size[1])*_grid_size[1];
		zmax = (1/_scaleZ)-_transZ;
		if ( zmax < _zmax )
			zmax = _zmax;
		zmax = int(zmax/_grid_size[1])*_grid_size[1];
		
		if ( _gridXY )
		{ 
			glLineWidth(float(2*_gridxy_radius));
			glColor3f(_gridxy_r, _gridxy_g, _gridxy_b);
			
			p = xmin;
			l = xmax+_grid_size[1]/2;
			while ( p < l )
			{
				glBegin(GL_LINE_STRIP);
				{
					glVertex3f(p, ymin, int(-_transZ/_grid_size[1])*_grid_size[1]);
					glVertex3f(p, ymax, int(-_transZ/_grid_size[1])*_grid_size[1]);
				}
				glEnd();
				
				p += _grid_size[1];    
			}
			
			p = ymin;
			l = ymax+_grid_size[1]/2;
			while ( p < l )
			{
				glBegin(GL_LINE_STRIP);
				{
					glVertex3f(xmin, p, int(-_transZ/_grid_size[1])*_grid_size[1]);
					glVertex3f(xmax, p, int(-_transZ/_grid_size[1])*_grid_size[1]);
				}
				glEnd();
				
				p += _grid_size[1];
			}
		}
		
		if ( _gridXZ )
		{    
			glLineWidth(float(2*_gridxy_radius));
			glColor3f(_gridxz_r, _gridxz_g, _gridxz_b);
			
			p = xmin;
			l = xmax+_grid_size[1]/2;
			while ( p < l )
			{
				glBegin(GL_LINE_STRIP);
				{
					glVertex3f(p, int(-_transY/_grid_size[1])*_grid_size[1], zmin);
					glVertex3f(p, int(-_transY/_grid_size[1])*_grid_size[1], zmax);
				}
				glEnd();
				
				p += _grid_size[1];    
			}
			
			p = zmin;
			l = zmax+_grid_size[1]/2;
			while ( p < l )
			{
				glBegin(GL_LINE_STRIP);
				{
					glVertex3f(xmin, int(-_transY/_grid_size[1])*_grid_size[1], p);
					glVertex3f(xmax, int(-_transY/_grid_size[1])*_grid_size[1], p);
				}
				glEnd();
				
				p += _grid_size[1];
			}
		}
		
		if ( _gridYZ )
		{    
			glLineWidth(float(2*_gridyz_radius));
			glColor3f(_gridyz_r, _gridyz_g, _gridyz_b);
			
			p = ymin;
			l = ymax+_grid_size[1]/2;
			while ( p < l )
			{
				glBegin(GL_LINE_STRIP);
				{
					glVertex3f(int(-_transX/_grid_size[1])*_grid_size[1], p, zmin);
					glVertex3f(int(-_transX/_grid_size[1])*_grid_size[1], p, zmax);
				}
				glEnd();
				
				p += _grid_size[1];
			}
			
			p = zmin;
			l = zmax+_grid_size[1]/2;
			while ( p < l )
			{
				glBegin(GL_LINE_STRIP);
				{
					glVertex3f(int(-_transX/_grid_size[1])*_grid_size[1], ymin, p);
					glVertex3f(int(-_transX/_grid_size[1])*_grid_size[1], ymax, p);
				}
				glEnd();
				
				p += _grid_size[1];
			}
		}
	}
}


void CurveXYZView::drawPPoint(CCurve *curve, int level, GLenum mode)
{
	int           i, s, t;
	float        r;
	GLUquadricObj *quad;
	
	if ( curve != 0 )
	{
		if ( (level == 0) && (mode == GL_SELECT) )
			glLoadName(0);
		
		quad = gluNewQuadric();
		for (i = 0; i < curve->_csize; i++)
		{
			if ( level == 0 )
				glColor3f(_ppoint_r, _ppoint_g, _ppoint_b);
			else
				glColor3f(_vpoint_r, _vpoint_g, _vpoint_b);
			
			if ( mode == GL_SELECT )
				glPushName(i);
			
			if ( (_dpoints || (mode == GL_SELECT)) && curve->_point[i]._visible )
			{
				glColor3f(_ppoint_r*curve->_point[i]._saturation, 
					_ppoint_g*curve->_point[i]._saturation,
					_ppoint_b*curve->_point[i]._saturation);
				
				glPushMatrix();
				
				glTranslatef(curve->_point[i]._point.x, curve->_point[i]._point.y,
					curve->_point[i]._point.z);
				
				if ( level == 0 )
				{
					r = 2.0f*_xcoeff/_width*_ppoint_radius/(_scaleX*RADIUS);
					s = _ppoint_slices;
					t = _ppoint_stacks;
				}
				else
				{
					r = 2.0f*_xcoeff/_width*_vpoint_radius/(_scaleX*RADIUS);
					s = _vpoint_slices;
					t = _vpoint_stacks;
				}
				
				gluSphere(quad, r, s, t);
				
				glPopMatrix();
			}
			
			drawPPoint(curve->_point[i]._curve, level+1, mode);
			
			if ( mode == GL_SELECT )
				glPopName();
		}
		
		gluDeleteQuadric(quad);
	}
}


void CurveXYZView::drawPConvex(CCurve *curve, int level, GLenum mode)
{
	int i, j, k;
	
	if ( (curve != 0) && (mode == GL_RENDER) )
	{
		if ( level == 0 )
		{
			glLineWidth(float(2*_pconvex_radius));
			glColor3f(_pconvex_r, _pconvex_g, _pconvex_b);
		}
		else
		{
			glLineWidth(float(2*_vconvex_radius));
			glColor3f(_vconvex_r, _vconvex_g, _vconvex_b);
		}
		
		if ( _dconvex )
		{
			if ( curve->_ctype == eBEZIERSURFACE )
			{
				for (i = 0; i < curve->_csize/16; i++)
				{
					for (j = 0; j < 4; j++)
					{
						for (k = 0; k < 3; k++)
						{
							if ( curve->_point[i*16+j*4+k]._visible && 
								curve->_point[i*16+j*4+k+1]._visible )
							{
								glBegin(GL_LINES);
								glColor3f(_pconvex_r*curve->_point[i*16+j*4+k]._saturation, 
									_pconvex_g*curve->_point[i*16+j*4+k]._saturation,
									_pconvex_b*curve->_point[i*16+j*4+k]._saturation);
								glVertex3f(curve->_point[i*16+j*4+k]._point.x,
									curve->_point[i*16+j*4+k]._point.y,
									curve->_point[i*16+j*4+k]._point.z);
								glColor3f(_pconvex_r*curve->_point[i*16+j*4+k+1]._saturation, 
									_pconvex_g*curve->_point[i*16+j*4+k+1]._saturation,
									_pconvex_b*curve->_point[i*16+j*4+k+1]._saturation);
								glVertex3f(curve->_point[i*16+j*4+k+1]._point.x,
									curve->_point[i*16+j*4+k+1]._point.y,
									curve->_point[i*16+j*4+k+1]._point.z);
								glEnd();
							}
						}
					}
					for (j = 0; j < 4; j++)
					{
						for (k = 0; k < 3; k++)
						{
							if ( curve->_point[i*16+k*4+j]._visible && 
								curve->_point[i*16+(k+1)*4+j]._visible )
							{
								glBegin(GL_LINES);
								glColor3f(_pconvex_r*curve->_point[i*16+k*4+j]._saturation, 
									_pconvex_g*curve->_point[i*16+k*4+j]._saturation,
									_pconvex_b*curve->_point[i*16+k*4+j]._saturation);
								glVertex3f(curve->_point[i*16+k*4+j]._point.x,
									curve->_point[i*16+k*4+j]._point.y,
									curve->_point[i*16+k*4+j]._point.z);
								glColor3f(_pconvex_r*curve->_point[i*16+(k+1)*4+j]._saturation, 
									_pconvex_g*curve->_point[i*16+(k+1)*4+j]._saturation,
									_pconvex_b*curve->_point[i*16+(k+1)*4+j]._saturation);
								glVertex3f(curve->_point[i*16+(k+1)*4+j]._point.x,
									curve->_point[i*16+(k+1)*4+j]._point.y,
									curve->_point[i*16+(k+1)*4+j]._point.z);
								glEnd();
							}
						}
					}
				}
			}
			else
			{
				for (i = 0; i < curve->_csize-1; i++)
				{
					if ( curve->_point[i]._visible && curve->_point[i+1]._visible )
					{
						glBegin(GL_LINES);
						glColor3f(_pconvex_r*curve->_point[i]._saturation, 
							_pconvex_g*curve->_point[i]._saturation,
							_pconvex_b*curve->_point[i]._saturation);
						glVertex3f(curve->_point[i]._point.x, curve->_point[i]._point.y,
							curve->_point[i]._point.z);
						glColor3f(_pconvex_r*curve->_point[i+1]._saturation, 
							_pconvex_g*curve->_point[i+1]._saturation,
							_pconvex_b*curve->_point[i+1]._saturation);
						glVertex3f(curve->_point[i+1]._point.x, curve->_point[i+1]._point.y,
							curve->_point[i+1]._point.z);
						glEnd();
					}
				}
				if ( curve->_ctype == eBSPLINECLOSED )
				{
					if ( curve->_point[curve->_csize-1]._visible && curve->_point[0]._visible )
					{
						glBegin(GL_LINES);
						glColor3f(_pconvex_r*curve->_point[curve->_csize-1]._saturation, 
							_pconvex_g*curve->_point[curve->_csize-1]._saturation,
							_pconvex_b*curve->_point[curve->_csize-1]._saturation);
						glVertex3f(curve->_point[curve->_csize-1]._point.x, 
							curve->_point[curve->_csize-1]._point.y,
							curve->_point[curve->_csize-1]._point.z);
						glColor3f(_pconvex_r*curve->_point[0]._saturation, 
							_pconvex_g*curve->_point[0]._saturation,
							_pconvex_b*curve->_point[0]._saturation);
						glVertex3f(curve->_point[0]._point.x, curve->_point[0]._point.y,
							curve->_point[0]._point.z);
						glEnd();
					}          
				}
			}
		}	
		for (i = 0; i < curve->_csize; i++)
			drawPConvex(curve->_point[i]._curve, level+1, mode);
	}
}


void CurveXYZView::drawPCurve(CCurve *curve, int level, GLenum mode)
{
	CPoint hP;
	int    i, j, k, visible;
	float t, s, saturation;
	
	if ( (curve != 0) && (mode == GL_RENDER) )
	{
		if ( level == 0 )
		{
			glLineWidth(float(2*_pcurve_radius));
			glColor3f(_pcurve_r, _pcurve_g, _pcurve_b);
		}
		else
		{
			glLineWidth(float(2*_vcurve_radius));
			glColor3f(_vcurve_r, _vcurve_g, _vcurve_b);
		}
		
		if ( _dcurve )
		{
			_curveXYZ->createPPoints(curve);
			
			if ( curve->_ctype == eBEZIERSURFACE )
			{
				for (i = 0; i < curve->_csize/16; i++)
				{
					visible = 0;
					saturation = 1.0;
					for (j = 0; j < 16; j++)
					{
						visible += curve->_point[i*16+j]._visible;
						if ( curve->_point[i*16+j]._saturation < saturation )
							saturation = curve->_point[i*16+j]._saturation;
					}
					
					if ( visible == 16 )
					{
						glColor3f(_pcurve_r*saturation, _pcurve_g*saturation, 
							_pcurve_b*saturation);
						
						for (j = 0; j < _pcurve_lines; j++)
						{
							s = (float)j/(_pcurve_lines-1);							
							glBegin( GL_LINE_STRIP );
							for (k= 0; k < _pcurve_lines; k++)
							{
								t = (float)k/(_pcurve_lines-1);
								hP = _curveXYZ->calcPValue(i, t, s, 0.0);
								glVertex3f(hP.x, hP.y, hP.z);
							}
							glEnd();
						}
						
						for (j = 0; j < _pcurve_lines; j++)
						{
							s = (float)j/(_pcurve_lines-1);
							glBegin( GL_LINE_STRIP ); 
							for ( k = 0; k < _pcurve_lines; k++)
							{
								t = (float)k/(_pcurve_lines-1);
								hP = _curveXYZ->calcPValue(i, s, t, 0.0);
								glVertex3f(hP.x, hP.y, hP.z);
							}
							glEnd();
						}
					}
				}
			}
			else
			{
				visible = 0;
				saturation = 1.0;
				for (j = 0; j < curve->_csize; j++)
				{
					visible += curve->_point[j]._visible;
					if ( curve->_point[j]._saturation < saturation )
						saturation = curve->_point[j]._saturation;
				}
				
				if ( visible == curve->_csize )
				{
					glColor3f(_pcurve_r*saturation, _pcurve_g*saturation, 
						_pcurve_b*saturation);
					
					_curveXYZ->createAPoints();
					
					glBegin(GL_LINE_STRIP);
					for (i = 0; i < _curveXYZ->_curve->_csize*_pcurve_points; i++)
					{
						s = (float)i/(_curveXYZ->_curve->_csize*_pcurve_points - 1);
						
						hP = _curveXYZ->calcAValue(0,s,0,0);
						
						glVertex3f(hP.x, hP.y, hP.z);
					}
					glEnd();
				}
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			drawPCurve(curve->_point[i]._curve, level+1, mode);
	}
}


void CurveXYZView::drawSelection(CCurve *curve, int level, GLenum mode)
{
	int     i;
	float  r, l;
	
	if ( (curve != 0) && (mode == GL_RENDER) )
	{
		r = 2.0f*_xcoeff/_width*_selectb_radius/(_scaleX*RADIUS);
		l = 2.0f*_xcoeff/_width*_selectb_length/(_scaleX*RADIUS);
		
		glLineWidth(float(2*r));
		glColor3f(_selectb_r, _selectb_g, _selectb_b);
		
		for (i = 0; i < curve->_csize; i++ )
		{
			if ( curve->_point[i]._select > 0 )
			{
				glPushMatrix();
				
				glTranslatef(curve->_point[i]._point.x, curve->_point[i]._point.y,
					curve->_point[i]._point.z );
				
				// front square
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f(-l, -l, -l);
					glVertex3f( l, -l, -l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f( l, -l, -l);
					glVertex3f( l,  l, -l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f( l,  l, -l);
					glVertex3f(-l,  l, -l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f(-l,  l, -l);
					glVertex3f(-l, -l, -l);
				}
				glEnd();
				
				// back square
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f(-l, -l,  l);
					glVertex3f( l, -l,  l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f( l, -l,  l);
					glVertex3f( l,  l,  l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f( l,  l,  l);
					glVertex3f(-l,  l,  l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f(-l,  l,  l);
					glVertex3f(-l, -l,  l);
				}
				glEnd();
				
				// top
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f(-l,  l, -l);
					glVertex3f(-l,  l,  l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f( l,  l, -l);
					glVertex3f( l,  l,  l);
				}
				glEnd();
				
				// bottom
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f(-l, -l, -l);
					glVertex3f(-l, -l,  l);
				}
				glEnd();
				
				glBegin( GL_LINE_STRIP );
				{
					glVertex3f( l, -l, -l);
					glVertex3f( l, -l,  l);
				}
				glEnd();
				
				glPopMatrix();
			}
			
			drawSelection(curve->_point[i]._curve, level+1, mode);
		}
	}
}


void CurveXYZView::findSelection(CCurve *curve, int level)
{
	int i;
	
	if ( curve != 0 )
	{
		if ( level == 0 )
		{
			_selecti = 0;
			_selectc = 0;
			_selectp = 0;
		}
		
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > _selecti )
			{
				_selecti = curve->_point[i]._select;
				_selectl = level;
				_selectc = curve;
				_selectp = i;
				
				_arrow.x = curve->_point[i]._point.x;
				_arrow.y = curve->_point[i]._point.y;
				_arrow.z = curve->_point[i]._point.z;
			}
			
			findSelection(curve->_point[i]._curve, level+1);
		}
	}
}


void CurveXYZView::drawArrows(GLenum mode)
{
	float         r, l, b, t, h;
	GLUquadricObj *quad = gluNewQuadric();
	
	if ( _selectc != 0 )
	{
		glPushMatrix();
		
		glTranslatef(_arrow.x, _arrow.y, _arrow.z);
		
		// x axis
		r = 2.0f*_xcoeff/_width*_selectx_radius/(_scaleX*RADIUS);
		l = 2.0f*_xcoeff/_width*_selectx_length/(_scaleX*RADIUS);
		b = 2.0f*_xcoeff/_width*_selectx_base/(_scaleX*RADIUS);
		t = 2.0f*_xcoeff/_width*_selectx_top/(_scaleX*RADIUS);
		h = 2.0f*_xcoeff/_width*_selectx_height/(_scaleX*RADIUS);
		
		if ( mode == GL_SELECT )
			glLoadName(1);
		
		if ( _arrowX )
			glColor3f(_selectx_r[1], _selectx_g[1], _selectx_b[1]);
		else
			glColor3f(_selectx_r[0], _selectx_g[0], _selectx_b[0]);
		
		glPushMatrix();
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		gluCylinder(quad, r, r, l, _selectx_slices, _selectx_stacks);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(l, 0.0f, 0.0f);
		glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
		gluCylinder(quad, b, t, h, _selectx_slices, _selectx_stacks);
		glPopMatrix();
		
		// y axis
		r = 2.0f*_xcoeff/_width*_selecty_radius/(_scaleX*RADIUS);
		l = 2.0f*_xcoeff/_width*_selecty_length/(_scaleX*RADIUS);
		b = 2.0f*_xcoeff/_width*_selecty_base/(_scaleX*RADIUS);
		t = 2.0f*_xcoeff/_width*_selecty_top/(_scaleX*RADIUS);
		h = 2.0f*_xcoeff/_width*_selecty_height/(_scaleX*RADIUS);
		
		if ( mode == GL_SELECT )
			glLoadName(2);
		
		if ( _arrowY )
			glColor3f(_selecty_r[1], _selecty_g[1], _selecty_b[1]);
		else
			glColor3f(_selecty_r[0], _selecty_g[0], _selecty_b[0]);
		
		glPushMatrix();
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(quad, r, r, l, _selecty_slices, _selecty_stacks);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(0.0f, l, 0.0f);
		glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
		gluCylinder(quad, b, t, h, _selecty_slices, _selecty_stacks);
		glPopMatrix();
		
		// z axis
		r = 2.0f*_xcoeff/_width*_selectz_radius/(_scaleX*RADIUS);
		l = 2.0f*_xcoeff/_width*_selectz_length/(_scaleX*RADIUS);
		b = 2.0f*_xcoeff/_width*_selectz_base/(_scaleX*RADIUS);
		t = 2.0f*_xcoeff/_width*_selectz_top/(_scaleX*RADIUS);
		h = 2.0f*_xcoeff/_width*_selectz_height/(_scaleX*RADIUS);
		
		if ( mode == GL_SELECT )
			glLoadName(3);
		
		if ( _arrowZ )
			glColor3f(_selectz_r[1], _selectz_g[1], _selectz_b[1]);
		else
			glColor3f(_selectz_r[0], _selectz_g[0], _selectz_b[0]);
		
		glPushMatrix();
		gluCylinder(quad, r, r, l, _selectz_slices, _selectz_stacks);
		glPopMatrix();
		
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, l);
		gluCylinder(quad, b, t, h, _selectz_slices, _selectz_stacks);
		glPopMatrix();
		
		glPopMatrix();
	}
	gluDeleteQuadric(quad);
}
 

void CurveXYZView::drawView(GLenum mode)
{
	HMatrix mNow;
	
	glPushMatrix();
	
	if ( _arcball )
	{
		Ball_Update(&ball);
		Ball_Value(&ball, mNow);
		
		glMultMatrixf((const float*)mNow);
		
		glScalef(_scaleX, _scaleY, _scaleZ);
		glScalef(RADIUS, RADIUS, RADIUS);
		
		glTranslatef(_transX, _transY, _transZ);
	}
	else
	{
		glRotatef(_rotX, 1.0f, 0.0f, 0.0f);
		glRotatef(_rotY, 0.0f, 1.0f, 0.0f);
		
		glScalef(_scaleX, _scaleY, _scaleZ);
		glScalef(RADIUS, RADIUS, RADIUS);
		
		glTranslatef(_transX, _transY, _transZ);
	}
	
	// calc gridsize
	calcGrid();
	
	// draw axis
	drawAxes(mode);
	
	// draw grid
	drawGrid(mode);
	
	// draw convex
	drawPConvex(_curveXYZ->_curve, 0, mode);
	
	// draw curve
	drawPCurve(_curveXYZ->_curve, 0, mode);
	
	// draw points
	drawPPoint(_curveXYZ->_curve, 0, mode);
	
	// draw selection
	drawSelection(_curveXYZ->_curve, 0, mode);
	
	// draw arrows
	drawArrows(mode);
	
	glPopMatrix();
	
	if ( (_rotTB || _rot) && _arcball && (mode == GL_RENDER) )
	{
		glPushMatrix();
		glLoadIdentity();
		gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
		glScalef(ball.radius, ball.radius, ball.radius);
		
		if ( _arcball_circle )
		{
			glLineWidth(float(2*_arcball_circle_radius));
			Ball_DrawCircle();
		}
		
		if ( _arcball_constraints )
		{
			glLineWidth(float(2*_arcball_constraints_radius));
			Ball_DrawConstraints(&ball);
		}
		
		if ( _arcball_arc )
		{
			glLineWidth(float(2*_arcball_arc_radius));
			Ball_DrawDragArc(&ball);
		}
		
		glPopMatrix();
	}
}


void CurveXYZView::deleteSelection(CCurve *curve, int min, int max, int r)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( (curve->_point[i]._select > min) && 
				(curve->_point[i]._select < max) )
				curve->_point[i]._select = 0;
			if ( curve->_point[i]._select < 0 )
				curve->_point[i]._select = r*curve->_point[i]._select;
			
			deleteSelection(curve->_point[i]._curve, min, max, r);
		}
	}
}


void CurveXYZView::selectObject(int x, int y, bool shift)
{
	int           i, selecti;
	unsigned int  j;
	bool          arrow = false;
	GLuint        buffer[1024], n;
	GLuint       *ptr;
	GLint         viewport[4], hits;
	CCurve       *curve = 0;
	
	findSelection(_curveXYZ->_curve, 0);
	
	selecti = _selecti;
	
	CurrentContext cc(_curveView);
	
	glSelectBuffer(1024, buffer);
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	(void) glRenderMode(GL_SELECT);
	
	glInitNames();
	glPushName(0);
	
	defaultView(x, viewport[3]-y, viewport, GL_SELECT);
	
	drawView(GL_SELECT);
	glFlush();
	
	hits = glRenderMode(GL_RENDER);
	if ( hits > 0 )
	{  
		ptr = (GLuint *)buffer;
		for (i = 0; i < hits; i++)
		{
			n = *ptr;
			ptr++;
			ptr++;
			ptr++;
			
			curve = _curveXYZ->_curve;
			_selectc = 0;
			_selectp = 0;
			for (j = 0; j < n; j++)
			{
				if ( j == 0 )
				{
					if ( *ptr > 0 )
						arrow = true;
					
					switch (*ptr)
					{
					case 1:
						_arrowX = true;
						break;
					case 2:
						_arrowY = true;
						break;
					case 3:
						_arrowZ = true;
						break;
					}
				}
				else
				{
					if ( curve != 0 )
					{
						if ( *ptr < (unsigned)curve->_csize )
						{
							_selectc = curve;
							_selectp = *ptr;
							
							curve = curve->_point[*ptr]._curve;
						}
					}
				}
				ptr++;
			}
			
			if ( _selectc != 0 )
			{
				if ( _selectc->_point[_selectp]._select > 0 )
				{
					_selectc->_point[_selectp]._select = 
						-_selectc->_point[_selectp]._select;
				}
				else
				{
					_selecti++;
					_selectc->_point[_selectp]._select = _selecti;
				}
			}
		}
		if ( arrow )
			deleteSelection(_curveXYZ->_curve, selecti, _selecti+1, -1);
	}
	if ( !arrow )
	{
		if ( !shift )
			deleteSelection(_curveXYZ->_curve, 0, selecti+1, 0);
		
		_arrowX = _arrowY = _arrowZ = true;
	}
}


CVector CurveXYZView::calcMovement(CPoint hP)
{
	int     i, j;
	CVector hV;
	CMatrix hM[3];
	HMatrix mNow;
	
	hV.x = (float)hP.x/(float)_width*2.0f*_xcoeff/(_scaleX*RADIUS);
	hV.y = -(float)hP.y/(float)_height*2.0f*_ycoeff/(_scaleY*RADIUS);
	hV.z = 0.0f;
	
	if ( _arcball )
	{
		Ball_Value(&ball, mNow);
		
		for ( i = 0; i < 3; i++ )
		{
			for ( j = 0; j < 3; j++ )
			{
				hM[0].Coeff[i][j] = mNow[i][j];
			}
		}
	}
	else 
	{
		hM[1].SetRX(_rotX);
		hM[2].SetRY(_rotY);
		
		hM[0].Mult(hM[1]);
		hM[0].Mult(hM[2]);
	}
	
	hV = hM[0].CalcVector(hV);
	
	return hV;
}


void CurveXYZView::getMinMax(CCurve *curve, int level)
{
	if ( curve != 0 )
	{
		for (int i = 0; i < curve->_csize; i++)
		{ 
			if ( (level == 0) && (i == 0) )
			{
				_xmin = _xmax = curve->_point[i]._point.x;
				_ymin = _ymax = curve->_point[i]._point.y;
				_zmin = _zmax = curve->_point[i]._point.z;
			}
			
			if ( _xmin > curve->_point[i]._point.x )
				_xmin = curve->_point[i]._point.x;
			if ( _xmax < curve->_point[i]._point.x )
				_xmax = curve->_point[i]._point.x;
			if ( _ymin > curve->_point[i]._point.y )
				_ymin = curve->_point[i]._point.y;
			if ( _ymax < curve->_point[i]._point.y )
				_ymax = curve->_point[i]._point.y;
			if ( _zmin > curve->_point[i]._point.z )
				_zmin = curve->_point[i]._point.z;
			if ( _zmax < curve->_point[i]._point.z )
				_zmax = curve->_point[i]._point.z;
			
			getMinMax(curve->_point[i]._curve, level+1);
		}
	}
}


void CurveXYZView::fitPoints()
{
	float s;
	
	getMinMax(_curveXYZ->_curve, 0);
	
	_transX = -(_xmax + _xmin)/2;
	_transY = -(_ymax + _ymin)/2;
	_transZ = -(_zmax + _zmin)/2;
	
	_scaleX = (_xmax - _xmin)/2;
	_scaleY = (_ymax - _ymin)/2;
	_scaleZ = (_zmax - _zmin)/2;
	
	if ( (_scaleX >= _scaleY) && (_scaleX >= _scaleZ) )
	{
		s = 1/_scaleX;
	}
	else if ( (_scaleY >= _scaleX) && (_scaleY >= _scaleZ) )
	{
		s = 1/_scaleY;
	}
	else
	{
		s = 1/_scaleZ;
	}
	
	if ( s < 0.00001f )
		s = 0.00001f;
	
	_scaleX = _scaleY = _scaleZ = s;
}


void CurveXYZView::defaultView(int x, int y, GLint *viewport, GLenum mode)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	if ( mode == GL_SELECT )
		gluPickMatrix((GLdouble)x, (GLdouble)y, 5.0, 5.0, viewport);
	
	if ( _width > _height )
	{
		_xcoeff = float(_width)/float(_height);
		_ycoeff = 1.0;
	}
	else
	{
		_xcoeff = 1.0;
		_ycoeff = float(_height)/float(_width);
	}
	
	glOrtho(-1.0*_xcoeff, 1.0*_xcoeff, -1.0*_ycoeff, 1.0*_ycoeff, -20.0, 20.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void CurveXYZView::setEdit(CurveEdit* curveEdit)
{
	_curveEdit = curveEdit;
}


void CurveXYZView::setView(CurveView* curveView)
{
	_curveView = curveView;
}


void CurveXYZView::initGraph()
{
	findSelection(_curveXYZ->_curve, 0);
	getMinMax(_curveXYZ->_curve, 0);
}


void CurveXYZView::updateGraph()
{
	initGraph();
	_curveEdit->updateEdit();
	_curveView->updateView();
}


void CurveXYZView::initializeGL()
{
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
}


void CurveXYZView::resizeGL(int w, int h)
{
	if ( h > 0 )
	{
		_width = w;
		_height = h;
		
		winsize.x = w;
		winsize.y = h;
		winorig.x = 0;
		winorig.y = 0;
		
		defaultView(0, 0, 0, GL_RENDER);
		glViewport(0, 0, w, h);
	}
}


void CurveXYZView::paintGL()
{
	glClearColor((float)_clear_r, (float)_clear_g, (float)_clear_b, 0.0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	drawView(GL_RENDER);
	
	glFlush();
}


void CurveXYZView::mouseLDown(int x, int y, bool shift, bool ctrl, bool alt)
{
	mouseNow.x = (float)x;
	mouseNow.y = (float)y;
	
	if ( !ctrl && !alt && _pointer )
	{
		selectObject(x, y, shift);
		
		if ( _arrowX || _arrowY || _arrowZ )
			_undosave = true;
		
		updateGraph();
	}
    
	if ( !shift && !ctrl && alt || _rotTB )
	{
		_rot = true;
		
		vNow.x = (2.0f*(mouseNow.x-winorig.x)/winsize.x - 1.0f)*_xcoeff;
		vNow.y = (2.0f*((winsize.y-mouseNow.y)-winorig.y)/winsize.y - 1.0f)*
			_ycoeff;
		
		if ( vNow.x*vNow.x + vNow.y*vNow.y > RADIUS*RADIUS )
			_wrap = false;
		else
			_wrap = _arcball_wrap;
		
		Ball_Mouse(&ball, vNow);
		
		Ball_BeginDrag(&ball);
	}
	
	if ( shift && !ctrl && alt || _scaleTB )
		_scale = true;
	
	if ( !shift && ctrl && alt || _transTB )
		_trans = true;
	
	mousePre = mouseNow;
}


void CurveXYZView::mouseLUp(int x, int y)
{
	mouseNow.x = (float)x;
	mouseNow.y = (float)y;
	
    if ( _rot )
	{    
		_rot = false;
		Ball_EndDrag(&ball);
    }
	
    if ( _scale )
    {
		_scale = false;
    }
	
    if ( _trans )
    {
		_trans = false;
    }
	
    if (_arrowX || _arrowY || _arrowZ )
    {
		CPoint point;
		
		_curveXYZ->createPPoints(_curveXYZ->_curve);
		point = _curveXYZ->calcPValue(0, 0.5, 0.5, 0.5);
		
		centerCurve(_curveXYZ->_curve, point);
    }    
	
    _arrowX = _arrowY = _arrowZ = false;
	
    _undosave = false;
	
	mousePre = mouseNow;
	
	updateGraph();
}


void CurveXYZView::movePoints(CCurve *curve, CVector move)
{
	if ( curve != 0 )
	{
		for (int i = 0; i < curve->_csize; i++)
		{ 
			if ( curve->_point[i]._select > 0 )
			{
				if ( _arrowX )
					curve->_point[i]._point.x += move.x;
				if ( _arrowY )
					curve->_point[i]._point.y += move.y;
				if ( _arrowZ )
					curve->_point[i]._point.z += move.z;
			}
			
			movePoints(curve->_point[i]._curve, move);
		}
	}
}


void CurveXYZView::centerCurve(CCurve *curve, CPoint center)
{
	CPoint point, vcenter;
	
	if ( curve != 0 )
	{
		_curveXYZ->createPPoints(curve);
		point = _curveXYZ->calcPValue(0, 0.5, 0.5, 0.5);
		
		for (int i = 0; i < curve->_csize; i++)
		{ 
			curve->_point[i]._point.x += center.x - point.x;
			curve->_point[i]._point.y += center.y - point.y;
			curve->_point[i]._point.z += center.z - point.z;
			
			centerCurve(curve->_point[i]._curve, curve->_point[i]._point);
		}
	}
}


void CurveXYZView::mouseMove(int x, int y)
{
	CPoint  mouseRel;
	CVector hV;
	float  r1, x0, y0;
	
	mouseNow.x = (float)x;
	mouseNow.y = (float)y;
	mouseRel.x = mouseNow.x - mousePre.x;
	mouseRel.y = mouseNow.y - mousePre.y;
	
	if ( _rot )
	{    
		if ( _arcball )
		{
			vNow.x = (2.0f*(mouseNow.x-winorig.x)/winsize.x - 1.0f)*_xcoeff;
			vNow.y = (2.0f*((winsize.y-mouseNow.y)-winorig.y)/winsize.y - 1.0f)*
				_ycoeff;
			
			if ( _wrap )
			{
				r1 = sqrtf(vNow.x*vNow.x + vNow.y*vNow.y);
				if ( r1 > RADIUS )
				{
					x0 = vNow.x/r1*RADIUS;
					y0 = vNow.y/r1*RADIUS;
					
					vNow.x = -2.0f*x0 + vNow.x;
					vNow.y = -2.0f*y0 + vNow.y;
				}
			}
			
			Ball_Mouse(&ball, vNow);
		}
		else
		{
			_rotX += _mouse_rotXY*mouseRel.y;
			if ( _rotX > 360.0 )
				_rotX -= 360.0;
			else if (_rotX < 0.0 )
				_rotX += 360.0;
			
			_rotY += _mouse_rotXY*mouseRel.x;
			if ( _rotY > 360.0 )
				_rotY -= 360.0;
			else if ( _rotY < 0.0 )
				_rotY += 360.0;
		}
		
		updateGraph();
	}
	
	if ( _scale )
	{
		_scaleX = _scaleY = _scaleZ *= powf(1.01f, -_mouse_scale*mouseRel.y);
		updateGraph();
	}
	
	if ( _trans )
	{
		hV = calcMovement(mouseRel);
		_transX += hV.x;
		_transY += hV.y;
		_transZ += hV.z;
		updateGraph();
	}
	
	if ( _arrowX || _arrowY || _arrowZ )
	{
		if ( _undosave )
		{
			_undosave = false;
			undoSave();
		}
		hV = calcMovement(mouseRel);
		movePoints(_curveXYZ->_curve, hV);
		updateGraph();
	}
	
	mousePre = mouseNow;
}


void CurveXYZView::updateView()
{
  updateGraph();
}


void CurveXYZView::updateAllWindows()
{
	updateView();
	
	updateGraph();
}


void CurveXYZView::updateSubWindows()
{
	updateGraph();
}


void CurveXYZView::openFile(const char* filename)
{
	const char* pDot = strrchr(filename, '.');
	if (0 != pDot)
	{
		if ( (_stricmp(pDot, ".con") == 0) || 
			(_stricmp(pDot, ".crv-con") == 0) )
		{
			openContour(filename);
		}
		else if ( (_stricmp(pDot, ".func") == 0) || 
			(_stricmp(pDot, ".crv-func") == 0) )
		{
			openFuncEdit(filename);
		}
		else if ( (_stricmp(pDot, ".s") == 0) || 
			(_stricmp(pDot, ".crv-s") == 0) )
		{
			openSurface(filename);
		}
		else if ( (_stricmp(pDot, ".srf") == 0) || 
			(_stricmp(pDot, ".crv-srf") == 0) )
		{
			openSurface(filename);
		}
		else if ( _stricmp(pDot, ".crv") == 0 )
		{
			openCurve(filename);
		}
		else if ( (_stricmp(pDot, ".cfg") == 0) ||
			(_stricmp(pDot, ".crv-cfg") == 0) )
		{
			openConfig(filename);
		}
		else
			std::cerr << "Warning: Didn't load the file " << filename << std::endl;
	}
}


void CurveXYZView::saveFile(const char* filename) const
{
	const char* pDot = strrchr(filename, '.');
	if ( 0 != pDot )
	{
		if ( (_stricmp(pDot, ".con") == 0) || 
			(_stricmp(pDot, ".crv-con") == 0) )
		{
			saveContour(filename);
		}
		else if ( (_stricmp(pDot, ".func") == 0) || 
			(_stricmp(pDot, ".crv-func") == 0) )
		{
			saveFuncEdit(filename);
		}
		else if ( (_stricmp(pDot, ".s") == 0) || 
			(_stricmp(pDot, ".crv-s") == 0) )
		{
			saveSurface(filename, true);
		}
		else if ( (_stricmp(pDot, ".srf") == 0) || 
			(_stricmp(pDot, ".crv-srf") == 0) )
		{
			saveSurface(filename, false);
		}
		else if ( _stricmp(pDot, ".crv") == 0 )
		{
			saveCurve(filename);
		}
		else
			std::cerr << "Warning: Didn't save the file " << filename << std::endl;
	}
	else
		std::cerr << "Warning: Didn't save the file " << filename << std::endl;
}


void CurveXYZView::openContour(const char* filename)
{
	int      i, p, d;
	CCPoint *cpoint;
	
	strcpy(_filename, filename);
	
	std::ifstream inFile(filename, std::ios::in);
	if ( inFile )
	{ 
		const int bufsize = 1024;
		char      buf[3][bufsize], pre[3][bufsize];
		
		undoSave();
		
		delete _curveXYZ->_curve;
		
		_curveXYZ->_curve = new CCurve;
		_curveXYZ->_curve->_csize = -1;
		_curveXYZ->_curve->_ctype = eBSPLINE;
		
		strcpy(pre[0], "nil");
		strcpy(pre[1], "nil");
		strcpy(pre[2], "nil");
		
		// points
		inFile >> std::setw(bufsize) >> buf[0];
		p = int(atof(buf[0]));
		
		cpoint = new CCPoint[p];
		
		// dimension
		inFile >> std::setw(bufsize) >> buf[0];
		d = int(atof(buf[0]));
		
		// closed
		inFile >> std::setw(bufsize) >> buf[0];
		if ( strcmp(buf[0], "closed") == 0 )
			_curveXYZ->_curve->_ctype = eBSPLINECLOSED;
		
		// x, y, z values
		for (i = 0; i < p; i++)
		{
			inFile >> std::setw(bufsize) >> buf[0];
			if ( (i == 0) && (strcmp(buf[0], "Bezier") == 0) )
			{
				_curveXYZ->_curve->_ctype = eBEZIER;
				
				inFile >> std::setw(bufsize) >> buf[0];
			}
			if ( (i == 0) && (strcmp(buf[0], "Catmull-Rom") == 0) )
			{
				inFile >> std::setw(bufsize) >> buf[0];
			}
			inFile >> std::setw(bufsize) >> buf[1];
			if ( d == 3 )
				inFile >> std::setw(bufsize) >> buf[2];
			
			if ( (strcmp(buf[0], pre[0]) != 0) || (strcmp(buf[1], pre[1]) != 0) ||
				(strcmp(buf[2], pre[2]) != 0) )
			{
				_curveXYZ->_curve->_csize += 1;
				
				cpoint[_curveXYZ->_curve->_csize]._pmult = 1;
				cpoint[_curveXYZ->_curve->_csize]._point.x = (float)atof(buf[0]);
				cpoint[_curveXYZ->_curve->_csize]._point.y = (float)atof(buf[1]);
				if ( d == 3 )
					cpoint[_curveXYZ->_curve->_csize]._point.z = (float)atof(buf[2]);
				else 
					cpoint[_curveXYZ->_curve->_csize]._point.z = 0.0f;
			}
			else
			{
				cpoint[_curveXYZ->_curve->_csize]._pmult += 1;
			}
			
			strcpy(pre[0], buf[0]);
			strcpy(pre[1], buf[1]);
			strcpy(pre[2], buf[2]);
		}
		
		_curveXYZ->_curve->_csize += 1;
		if ( _curveXYZ->_curve->_csize > 0 )
		{
			_curveXYZ->_curve->_point = new CCPoint[_curveXYZ->_curve->_csize];
			for (i = 0; i < _curveXYZ->_curve->_csize; i++)
			{
				_curveXYZ->_curve->_point[i] = cpoint[i];
			}
		}
		else
		{
			std::cerr << "Warning: No points defined" << std::endl;
			
			_curveXYZ->_curve = _curveXYZ->init(eBSPLINE);
		}
		
		delete []cpoint;
	}
	else
	{
		std::cerr << "Error: Can't open the file " << filename << std::endl;
	}
}


void CurveXYZView::saveContour(const char* filename) const
{ 
	int i, j, p;
	
	std::ofstream outFile(filename, std::ios::out);
	if ( outFile )
	{ 
		p = 0;
		for (i = 0; i < _curveXYZ->_curve->_csize; i++)
			p += _curveXYZ->_curve->_point[i]._pmult;
		
		// point, dimension
		outFile << p << " 3 ";
		
		// closed
		if ( _curveXYZ->_curve->_ctype == eBSPLINECLOSED )
			outFile << "closed";
		else
			outFile << "open";
		
		// type
		if ( _curveXYZ->_curve->_ctype == eBEZIER )
			outFile << " Bezier" << std::endl;
		else
			outFile << std::endl;
		
		// x, y, z values
		for (i = 0; i < _curveXYZ->_curve->_csize; i++)
		{
			for (j = 0; j < _curveXYZ->_curve->_point[i]._pmult; j++)
			{
				outFile << _curveXYZ->_curve->_point[i]._point.x << " ";
				outFile << _curveXYZ->_curve->_point[i]._point.y << " ";
				outFile << _curveXYZ->_curve->_point[i]._point.z << std::endl;
			}
		}
	}
	else
		std::cerr << "Error: Can't write the file " << filename << std::endl;
}


void CurveXYZView::openSurface(const char* filename)
{
	const int MAXP = 64;
	
	int      i, j, p;
	CCPoint *cpoint;
	
	strcpy(_filename, filename);
	
	std::ifstream inFile(filename, std::ios::in);
	if ( inFile )
	{ 
		const int bufsize = 1024;
		char      buf[3][bufsize];
		
		undoSave();
		
		delete _curveXYZ->_curve;
		
		_curveXYZ->_curve = new CCurve;
		_curveXYZ->_curve->_csize = -1;
		_curveXYZ->_curve->_ctype = eBEZIERSURFACE;
		
		cpoint = new CCPoint[MAXP*16];
		
		// min and max values
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		
		// precision
		inFile >> std::setw(bufsize) >> buf[0];
		if ( strcmp(buf[0], "PRECISION") == 0 )
		{
			_curveXYZ->_turtle._p = true;
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			_curveXYZ->_turtle._ps = int(atof(buf[0]));
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			_curveXYZ->_turtle._pt = int(atof(buf[0]));
			
			inFile >> std::setw(bufsize) >> buf[0];
		}
		else
			_curveXYZ->_turtle._p = false;
		
		// contact point
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._cp.x = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._cp.y = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._cp.z = (float)atof(buf[0]);
		
		// end point
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._ep.x = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._ep.y = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._ep.z = (float)atof(buf[0]);
		
		// heading
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._h.x = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._h.y = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._h.z = (float)atof(buf[0]);
		
		// up
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._u.x = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._u.y = (float)atof(buf[0]);
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._u.z = (float)atof(buf[0]);
		
		// size
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		_curveXYZ->_turtle._sz = (float)atof(buf[0]);
		
		// read patches
		p = 0;
		while ( p < MAXP )
		{
			// patch name
			inFile >> std::setw(bufsize) >> buf[0];
			if ( !inFile )
				break;
			
			// top color
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			_curveXYZ->_color._tc = int(atof(buf[0]));
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			_curveXYZ->_color._td = (float)atof(buf[0]);
			
			// bottom color
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			_curveXYZ->_color._bc = int(atof(buf[0]));
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			_curveXYZ->_color._bd = (float)atof(buf[0]);
			
			// above left, above, above right
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			
			// left, right
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			
			// below left, below, below right
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[0];
			
			for (i = 0; i < 4; i++)
			{
				for (j = 0; j < 4; j++)
				{
					inFile >> std::setw(bufsize) >> buf[0];
					inFile >> std::setw(bufsize) >> buf[1];
					inFile >> std::setw(bufsize) >> buf[2];
					
					cpoint[p*16+i*4+j]._pmult = 1;
					cpoint[p*16+i*4+j]._point.x = (float)atof(buf[0]);
					cpoint[p*16+i*4+j]._point.y = (float)atof(buf[1]);
					cpoint[p*16+i*4+j]._point.z = (float)atof(buf[2]);
				}
			}
			
			p += 1;
		}
		
		if ( p > 0 )
		{
			_curveXYZ->_curve->_csize = p*16;
			_curveXYZ->_curve->_point = new CCPoint[_curveXYZ->_curve->_csize];
			for (i = 0; i < _curveXYZ->_curve->_csize; i++)
			{
				_curveXYZ->_curve->_point[i] = cpoint[i];
			}
		}
		else
		{
			std::cerr << "Warning: No patches defined" << std::endl;
			
			_curveXYZ->_curve = _curveXYZ->init(eBSPLINE);
		}
		
		delete []cpoint;
	}
	else
	{
		std::cerr << "Error: Can't open the file " << filename << std::endl;
	}
}


const char* CurveXYZView::findPatch(int p, int p1, int p2, int p3, int p4,
									int i1, int i2, int i3, int i4) const
{
	int         i, f1, f2, f3, f4;
	static char buf[256];
	
	strcpy(buf, "~");
	
	// check all patches
	for (i = 0; i < _curveXYZ->_curve->_csize/16; i++)
	{
		if ( i != p )
		{
			f1 = f2 = f3 = f4 = 0;
			
			if ( (_curveXYZ->_curve->_point[p*16+p1]._point.x == 
				_curveXYZ->_curve->_point[i*16+i1]._point.x) &&
				(_curveXYZ->_curve->_point[p*16+p1]._point.y == 
				_curveXYZ->_curve->_point[i*16+i1]._point.y) &&
				(_curveXYZ->_curve->_point[p*16+p1]._point.z == 
				_curveXYZ->_curve->_point[i*16+i1]._point.z) )
				f1 = 1;
			if ( (_curveXYZ->_curve->_point[p*16+p2]._point.x == 
				_curveXYZ->_curve->_point[i*16+i2]._point.x) &&
				(_curveXYZ->_curve->_point[p*16+p2]._point.y == 
				_curveXYZ->_curve->_point[i*16+i2]._point.y) &&
				(_curveXYZ->_curve->_point[p*16+p2]._point.z == 
				_curveXYZ->_curve->_point[i*16+i2]._point.z) )
				f2 = 1;
			if ( (_curveXYZ->_curve->_point[p*16+p3]._point.x == 
				_curveXYZ->_curve->_point[i*16+i3]._point.x) &&
				(_curveXYZ->_curve->_point[p*16+p3]._point.y == 
				_curveXYZ->_curve->_point[i*16+i3]._point.y) &&
				(_curveXYZ->_curve->_point[p*16+p3]._point.z == 
				_curveXYZ->_curve->_point[i*16+i3]._point.z) )
				f3 = 1;
			if ( (_curveXYZ->_curve->_point[p*16+p4]._point.x == 
				_curveXYZ->_curve->_point[i*16+i4]._point.x) &&
				(_curveXYZ->_curve->_point[p*16+p4]._point.y == 
				_curveXYZ->_curve->_point[i*16+i4]._point.y) &&
				(_curveXYZ->_curve->_point[p*16+p4]._point.z == 
				_curveXYZ->_curve->_point[i*16+i4]._point.z) )
				f4 = 1;
			
			if ( f1 && f2 && f3 && f4 )
				sprintf(buf, "Patch_%d", i+1);
		}
	}
	
	return buf;
}


void CurveXYZView::saveSurface(const char* filename, bool flag) const
{
	int    i, j, k;
	float xmin, xmax, ymin, ymax, zmin, zmax;
	
	if ( _curveXYZ->_curve->_ctype == eBEZIERSURFACE )
	{
		std::ofstream outFile(filename, std::ios::out);
		if ( outFile )
		{ 
			outFile << std::setprecision(2) << std::setiosflags(std::ios::fixed);
			
			{
				xmin = _curveXYZ->_curve->_point[0]._point.x;
				xmax = _curveXYZ->_curve->_point[0]._point.x;
				ymin = _curveXYZ->_curve->_point[0]._point.y;
				ymax = _curveXYZ->_curve->_point[0]._point.y;
				zmin = _curveXYZ->_curve->_point[0]._point.z;
				zmax = _curveXYZ->_curve->_point[0]._point.z;
			}
			// determine min and max values
			for (i = 0; i < _curveXYZ->_curve->_csize; i++)
			{
				if ( xmin > _curveXYZ->_curve->_point[i]._point.x )
					xmin = _curveXYZ->_curve->_point[i]._point.x;
				if ( xmax < _curveXYZ->_curve->_point[i]._point.x )
					xmax = _curveXYZ->_curve->_point[i]._point.x;
				if ( ymin > _curveXYZ->_curve->_point[i]._point.y )
					ymin = _curveXYZ->_curve->_point[i]._point.y;
				if ( ymax < _curveXYZ->_curve->_point[i]._point.y )
					ymax = _curveXYZ->_curve->_point[i]._point.y;
				if ( zmin > _curveXYZ->_curve->_point[i]._point.z )
					zmin = _curveXYZ->_curve->_point[i]._point.z;
				if ( zmax < _curveXYZ->_curve->_point[i]._point.z )
					zmax = _curveXYZ->_curve->_point[i]._point.z;
			}
			
			// min and max values
			outFile << xmin << " " << xmax << " " << ymin << " " << ymax << " " <<
				zmin << " " << zmax << std::endl;
			
			// precision
			if ( _curveXYZ->_turtle._p )
			{
				outFile << "PRECISION S: " << _curveXYZ->_turtle._ps << " T: " <<
					_curveXYZ->_turtle._pt << std::endl;
			}
			
			// contact point
			outFile << "CONTACT POINT X: " << _curveXYZ->_turtle._cp.x << " Y: " << 
				_curveXYZ->_turtle._cp.y << " Z: " << _curveXYZ->_turtle._cp.z << std::endl;
			
			// end point
			outFile << "END POINT X: " << _curveXYZ->_turtle._ep.x << " Y: " << 
				_curveXYZ->_turtle._ep.y << " Z: " << _curveXYZ->_turtle._ep.z << std::endl;
			
			// heading
			outFile << "HEADING X: " << _curveXYZ->_turtle._h.x << " Y: " << 
				_curveXYZ->_turtle._h.y << " Z: " << _curveXYZ->_turtle._h.z << std::endl;
			
			// up
			outFile << "UP X: " << _curveXYZ->_turtle._u.x << " Y: " << 
				_curveXYZ->_turtle._u.y << " Z: " << _curveXYZ->_turtle._u.z << std::endl;
			
			// size
			outFile << "SIZE: " << _curveXYZ->_turtle._sz << std::endl;
			
			for (i = 0; i < _curveXYZ->_curve->_csize/16; i++)
			{
				// patchname
				outFile << "Patch_" << i+1 << std::endl;
				
				// top color
				outFile << "TOP COLOR: " << _curveXYZ->_color._tc << " DIFFUSE: " << 
					_curveXYZ->_color._td << " ";
				
				// bottom color
				outFile << "BOTTOM COLOR: " << _curveXYZ->_color._bc << " DIFFUSE: " << 
					_curveXYZ->_color._bd << std::endl;
				
				if ( flag ) 
				{
					// above left
					outFile << "AL: " << findPatch(i, 12, 12, 12, 12, 3, 3, 3, 3);
					
					// above 
					outFile << " A: " << findPatch(i, 12, 13, 14, 15, 0, 1, 2, 3);
					
					// above right
					outFile << " AR: " << findPatch(i, 15, 15, 15, 15, 0, 0, 0, 0) << std::endl;
					
					// left
					outFile << "L: " << findPatch(i, 0, 4, 8, 12, 3, 7, 11, 15);
					
					// right
					outFile << " R: " << findPatch(i, 3, 7, 11, 15, 0, 4, 8, 12) << std::endl;
					
					// below left
					outFile << "BL: " << findPatch(i, 0, 0, 0, 0, 15, 15, 15, 15);
					
					// below 
					outFile << " B: " << findPatch(i, 0, 1, 2, 3, 12, 13, 14, 15);
					
					// below right
					outFile << " BR: " << findPatch(i, 3, 3, 3, 3, 12, 12, 12, 12) << std::endl;
				}
				else
				{
					// above left, above, above right
					outFile << "AL: ~ A: ~ AR: ~" << std::endl;
					
					// left, right
					outFile << "L: ~ R: ~" << std::endl;
					
					// below left, below, below right
					outFile << "BL: ~ B: ~ BR: ~" << std::endl;
				}
				
				// points
				for (j = 0; j < 4; j++)
				{
					for (k = 0; k < 4; k++)
					{
						outFile << _curveXYZ->_curve->_point[i*16+j*4+k]._point.x << " " <<
							_curveXYZ->_curve->_point[i*16+j*4+k]._point.y << " " <<
							_curveXYZ->_curve->_point[i*16+j*4+k]._point.z;
						
						if ( k < 3 )
							outFile << " ";
						else
							outFile << std::endl;
					}
				}
			}
		}
		else
			std::cerr << "Error: Can't write the file " << filename << std::endl;
	}
}


void CurveXYZView::openFuncEdit(const char* filename)
{
	int      i, p;
	CCPoint *cpoint;
	
	strcpy(_filename, filename);
	
	std::ifstream inFile(filename, std::ios::in);
	if ( inFile )
	{ 
		const int bufsize = 1024;
		char      buf[2][bufsize], pre[2][bufsize];
		
		undoSave();
		
		delete _curveXYZ->_curve;
		
		_curveXYZ->_curve = new CCurve;
		_curveXYZ->_curve->_csize = -1;
		_curveXYZ->_curve->_ctype = eBSPLINEENDPOINT;
		
		strcpy(pre[0], "nil");
		strcpy(pre[1], "nil");
		
		// range
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		
		// points
		inFile >> std::setw(bufsize) >> buf[0];
		inFile >> std::setw(bufsize) >> buf[0];
		p = int(atof(buf[0]));
		
		cpoint = new CCPoint[p];
		
		// x, y values
		for (i = 0; i < p; i++)
		{
			inFile >> std::setw(bufsize) >> buf[0];
			inFile >> std::setw(bufsize) >> buf[1];
			
			if ( (strcmp(buf[0], pre[0]) != 0) || (strcmp(buf[1], pre[1]) != 0) )
			{
				_curveXYZ->_curve->_csize += 1;
				
				cpoint[_curveXYZ->_curve->_csize]._pmult = 1;
				cpoint[_curveXYZ->_curve->_csize]._point.x = (float)atof(buf[0]);
				cpoint[_curveXYZ->_curve->_csize]._point.y = (float)atof(buf[1]);
				cpoint[_curveXYZ->_curve->_csize]._point.z = 0.0f;
			}
			else
			{
				cpoint[_curveXYZ->_curve->_csize]._pmult += 1;
			}
			
			strcpy(pre[0], buf[0]);
			strcpy(pre[1], buf[1]);
		}
		
		_curveXYZ->_curve->_csize += 1;
		if ( _curveXYZ->_curve->_csize > 0 )
		{
			_curveXYZ->_curve->_point = new CCPoint[_curveXYZ->_curve->_csize];
			for (i = 0; i < _curveXYZ->_curve->_csize; i++)
			{
				_curveXYZ->_curve->_point[i] = cpoint[i];
			}
		}
		else
		{
			std::cerr << "Warning: No points defined" << std::endl;
			
			_curveXYZ->_curve = _curveXYZ->init(eBSPLINE);
		}
		
		delete []cpoint;
	}
	else
	{
		std::cerr << "Error: Can't open the file " << filename << std::endl;
	}
}


void CurveXYZView::saveFuncEdit(const char* filename) const
{ 
	int i, j, p;
	
	std::ofstream outFile(filename, std::ios::out);
	if ( outFile )
	{ 
		p = 0;
		for (i = 0; i < _curveXYZ->_curve->_csize; i++)
			p += _curveXYZ->_curve->_point[i]._pmult;
		
		// range
		outFile << "range: 0.0 1.0" << std::endl;
		
		// points
		outFile << "points: " << p << std::endl;
		
		// x, y values
		for (i = 0; i < _curveXYZ->_curve->_csize; i++)
		{
			for (j = 0; j < _curveXYZ->_curve->_point[i]._pmult; j++)
			{
				outFile << _curveXYZ->_curve->_point[i]._point.x << " ";
				outFile << _curveXYZ->_curve->_point[i]._point.y << std::endl;
			}
		}
	}
	else
		std::cerr << "Error: Can't write the file " << filename << std::endl;
}


void CurveXYZView::openCurve(const char* filename)
{
	undoSave();
	
	strcpy(_filename, filename);
	
	_curveXYZ->openCurve(filename);
}


void CurveXYZView::writeCurve(CCurve *curve, std::ofstream &outFile) const
{
	int  i;
	
	if ( (curve != 0 ) && outFile )
	{
		// points
		outFile << "points: " << curve->_csize << std::endl;
		
		// range
		outFile << "range: 0.0 1.0" << std::endl;
		
		// dimension
		outFile << "dimension: 4" << std::endl;
		
		// type
		if ( curve->_ctype == eBEZIERSURFACE )
			outFile << "type: bezier-surface" << std::endl;
		else if ( curve->_ctype == eBEZIER )
			outFile << "type: bezier" << std::endl;
		else if ( curve->_ctype == eBSPLINEPHANTOM )
			outFile << "type: bspline-phantom" << std::endl;
		else if ( curve->_ctype == eBSPLINEENDPOINT )
			outFile << "type: bspline-endpoint" << std::endl;
		else if ( curve->_ctype == eBSPLINECLOSED )
			outFile << "type: bspline-closed" << std::endl;
		else
			outFile << "type: bspline" << std::endl;
		
		// x, y, z values
		for (i = 0; i < curve->_csize; i++)
		{
			outFile << curve->_point[i]._point.x << " ";
			outFile << curve->_point[i]._point.y << " ";
			outFile << curve->_point[i]._point.z << " ";
			outFile << curve->_point[i]._pmult << std::endl;
			
			writeCurve(curve->_point[i]._curve, outFile);
		}
	}
}


void CurveXYZView::saveCurve(const char* filename) const
{
	if ( _curveXYZ->_curve != 0 )
	{
		std::ofstream outFile(filename, std::ios::out);
		if ( outFile )
		{
			// version
			outFile << "version: 1.4" << std::endl;
			
			// contact point
			outFile << "contact: " << _curveXYZ->_turtle._cp.x << " " <<
				_curveXYZ->_turtle._cp.y << " " << _curveXYZ->_turtle._cp.z << std::endl;
			
			// end point
			outFile << "end: " << _curveXYZ->_turtle._ep.x << " " <<
				_curveXYZ->_turtle._ep.y << " " << _curveXYZ->_turtle._ep.z << std::endl;
			
			// heading
			outFile << "heading: " << _curveXYZ->_turtle._h.x << " " <<
				_curveXYZ->_turtle._h.y << " " << _curveXYZ->_turtle._h.z << std::endl;
			
			// up
			outFile << "up: " << _curveXYZ->_turtle._u.x << " " << 
				_curveXYZ->_turtle._u.y << " " << _curveXYZ->_turtle._u.z << std::endl;
			
			// size
			outFile << "size: " << _curveXYZ->_turtle._sz << std::endl;   
			
			writeCurve(_curveXYZ->_curve, outFile);
		}
		else
			std::cerr << "Error: Can't write the file " << filename << std::endl;
	}
}


void CurveXYZView::copyCurve()
{
	int i;
	
	if ( _selectc != 0 )
	{
		delete _copy;
		
		_copy = new CCurve;
		
		_copy->_csize = _selectc->_csize;
		_copy->_point = new CCPoint[_copy->_csize];
		for (i = 0; i < _copy->_csize; i++)
			_copy->_point[i]._point = _selectc->_point[i]._point;
		_copy->_ctype = _selectc->_ctype;
	}
}


void CurveXYZView::pasteCurve()
{
	int i;
	
	if ( (_selectc != 0) && ( _copy != 0) )
	{
		if ( _selectc->_csize == _copy->_csize )
		{ 
			undoSave();
			
			for (i = 0; i < _selectc->_csize; i++)
				_selectc->_point[i]._point = _copy->_point[i]._point;
			_selectc->_ctype = _copy->_ctype;
		}
	}
}


void CurveXYZView::setPMult(CCurve *curve, int pmult)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{ 
			if ( curve->_point[i]._select > 0 )
			{
				curve->_point[i]._pmult = pmult;
			}
			
			setPMult(curve->_point[i]._curve, pmult);
		}
	}
}


void CurveXYZView::setShowAll(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			curve->_point[i]._visible = 1;
			
			setShowAll(curve->_point[i]._curve);
		}
	}
}


void CurveXYZView::setShowCurve(CCurve *curve)
{
	int i, v;
	
	if ( curve != 0 )
	{
		v = 0;
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
				v = 1;
		}
		for (i = 0; i < curve->_csize; i++)
			curve->_point[i]._visible = v;
		
		for (i = 0; i < curve->_csize; i++)
			setShowCurve(curve->_point[i]._curve);
	}
}


void CurveXYZView::setShowPatch(CCurve *curve)
{
	int i, j, v;
	
	if ( curve != 0 )
	{
		if ( curve->_ctype == eBEZIERSURFACE )
		{
			for (i = 0; i < curve->_csize/16; i++)
			{
				v = 0;
				for (j = 0; j < 16; j++)
				{
					if ( curve->_point[i*16+j]._select > 0 )
						v = 1;
				}
				for (j = 0; j < 16; j++)
					curve->_point[i*16+j]._visible = v;
			}
		}
		if ( curve->_ctype == eBEZIER )
		{
			for (i = 0; i < curve->_csize/4; i++)
			{
				v = 0;
				for (j = 0; j < 4; j++)
				{
					if ( curve->_point[i*4+j]._select > 0 )
						v = 1;
				}
				for (j = 0; j < 4; j++)
					curve->_point[i*4+j]._visible = v;
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			setShowPatch(curve->_point[i]._curve);
	}
}


void CurveXYZView::setShowPoint(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select == 0 )
				curve->_point[i]._visible = 0;
			
			setShowPoint(curve->_point[i]._curve);
		}
	}
}


void CurveXYZView::setHideCurve(CCurve *curve)
{
	int i, v;
	
	if ( curve != 0 )
	{
		v = 1;
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
				v = 0;
		}
		if ( v == 0 )
		{
			for (i = 0; i < curve->_csize; i++)
			{
				curve->_point[i]._visible = 0;
				curve->_point[i]._select = 0;
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			setHideCurve(curve->_point[i]._curve);
	}
}


void CurveXYZView::setHidePatch(CCurve *curve)
{
	int i, j, v;
	
	if ( curve != 0 )
	{
		if ( curve->_ctype == eBEZIERSURFACE )
		{
			for (i = 0; i < curve->_csize/16; i++)
			{
				v = 1;
				for (j = 0; j < 16; j++)
				{
					if ( curve->_point[i*16+j]._select > 0 )
						v = 0;
				}
				if ( v == 0 )
				{
					for (j = 0; j < 16; j++)
					{
						curve->_point[i*16+j]._visible = 0;
						curve->_point[i*16+j]._select = 0;
					}
				}
			}
		}
		if ( curve->_ctype == eBEZIER )
		{
			for (i = 0; i < curve->_csize/4; i++)
			{
				v = 1;
				for (j = 0; j < 4; j++)
				{
					if ( curve->_point[i*4+j]._select > 0 )
						v = 0;
				}
				if ( v == 0 )
				{
					for (j = 0; j < 4; j++)
					{
						curve->_point[i*4+j]._visible = 0;
						curve->_point[i*4+j]._select = 0;
					}
				}
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			setHidePatch(curve->_point[i]._curve);
	}
}


void CurveXYZView::setHidePoint(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
			{
				curve->_point[i]._visible = 0;
				curve->_point[i]._select = 0;
			}
			
			setHidePoint(curve->_point[i]._curve);
		}
	}
}


void CurveXYZView::setBrightAll(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			curve->_point[i]._saturation = 1.0;
			
			setBrightAll(curve->_point[i]._curve);
		}
	}
}


void CurveXYZView::setBrightCurve(CCurve *curve)
{
	int i, v;
	
	if ( curve != 0 )
	{
		v = 0;
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
				v = 1;
		}
		for (i = 0; i < curve->_csize; i++)
		{
			curve->_point[i]._saturation *= 2.0;
			if ( curve->_point[i]._saturation > 1.0 )
				curve->_point[i]._saturation = 1.0;
		}
		
		for (i = 0; i < curve->_csize; i++)
			setBrightCurve(curve->_point[i]._curve);
	}
}


void CurveXYZView::setBrightPatch(CCurve *curve)
{
	int i, j, v;
	
	if ( curve != 0 )
	{
		if ( curve->_ctype == eBEZIERSURFACE )
		{
			for (i = 0; i < curve->_csize/16; i++)
			{
				v = 0;
				for (j = 0; j < 16; j++)
				{
					if ( curve->_point[i*16+j]._select > 0 )
						v = 1;
				}
				for (j = 0; j < 16; j++)
				{
					curve->_point[i*16+j]._saturation *= 2.0;
					if ( curve->_point[i*16+j]._saturation > 1.0 )
						curve->_point[i*16+j]._saturation = 1.0;
				}
			}
		}
		if ( curve->_ctype == eBEZIER )
		{
			for (i = 0; i < curve->_csize/4; i++)
			{
				v = 0;
				for (j = 0; j < 4; j++)
				{
					if ( curve->_point[i*4+j]._select > 0 )
						v = 1;
				}
				for (j = 0; j < 4; j++)
				{
					curve->_point[i*4+j]._saturation *= 2.0;
					if ( curve->_point[i*4+j]._saturation > 1.0 )
						curve->_point[i*4+j]._saturation = 1.0;
				}
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			setBrightPatch(curve->_point[i]._curve);
	}
}


void CurveXYZView::setBrightPoint(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
			{
				curve->_point[i]._saturation *= 2.0;
				if ( curve->_point[i]._saturation > 1.0 )
					curve->_point[i]._saturation = 1.0;
			}
			
			setBrightPoint(curve->_point[i]._curve);
		}
	}
}


void CurveXYZView::setDarkCurve(CCurve *curve)
{
	int i, v;
	
	if ( curve != 0 )
	{
		v = 1;
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
				v = 0;
		}
		if ( v == 0 )
		{
			for (i = 0; i < curve->_csize; i++)
				curve->_point[i]._saturation *= 0.5;
		}
		
		for (i = 0; i < curve->_csize; i++)
			setDarkCurve(curve->_point[i]._curve);
	}
}


void CurveXYZView::setDarkPatch(CCurve *curve)
{
	int i, j, v;
	
	if ( curve != 0 )
	{
		if ( curve->_ctype == eBEZIERSURFACE )
		{
			for (i = 0; i < curve->_csize/16; i++)
			{
				v = 1;
				for (j = 0; j < 16; j++)
				{
					if ( curve->_point[i*16+j]._select > 0 )
						v = 0;
				}
				if ( v == 0 )
				{
					for (j = 0; j < 16; j++)
						curve->_point[i*16+j]._saturation *= 0.5;
				}
			}
		}
		if ( curve->_ctype == eBEZIER )
		{
			for (i = 0; i < curve->_csize/4; i++)
			{
				v = 1;
				for (j = 0; j < 4; j++)
				{
					if ( curve->_point[i*4+j]._select > 0 )
						v = 0;
				}
				if ( v == 0 )
				{
					for (j = 0; j < 4; j++)
						curve->_point[i*4+j]._saturation *= 0.5;
				}
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			setDarkPatch(curve->_point[i]._curve);
	}
}


void CurveXYZView::setDarkPoint(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
				curve->_point[i]._saturation *= 0.5;
			
			setDarkPoint(curve->_point[i]._curve);
		}
	}
}


void CurveXYZView::createVariation(CCurve *curve, ECURVE type)
{
	int i, j;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
			{
				delete curve->_point[i]._curve;
				
				curve->_point[i]._curve = _curveXYZ->init(type);
				
				for (j = 0; j < curve->_point[i]._curve->_csize; j++)
				{
					curve->_point[i]._curve->_point[j]._point.x += 
						curve->_point[i]._point.x;
					curve->_point[i]._curve->_point[j]._point.y += 
						curve->_point[i]._point.y;
					curve->_point[i]._curve->_point[j]._point.z += 
						curve->_point[i]._point.z;
				}
			}
			
			createVariation(curve->_point[i]._curve, type);
		}
	}
}


void CurveXYZView::selectAll(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			_selecti += 1;
			curve->_point[i]._select = _selecti;
			
			selectAll(curve->_point[i]._curve);
		}
	}
}


void CurveXYZView::selectCurve(CCurve *curve)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			_selecti += 1;
			curve->_point[i]._select = _selecti;
		}
	}
}


void CurveXYZView::selectPoint(CCurve *curve, int selects)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
			{
				if ( _selects != selects )
					curve->_point[i]._select = 0;
				
				_selects += 1;
			}
			
			selectPoint(curve->_point[i]._curve, selects);
		}
	}
}


void CurveXYZView::alignPoints(CCurve *curve, CPoint point)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{
			if ( curve->_point[i]._select > 0 )
			{
				curve->_point[i]._point.x = point.x;
				curve->_point[i]._point.y = point.y;
				curve->_point[i]._point.z = point.z;
			}
			
			alignPoints(curve->_point[i]._curve, point);
		}
	}
}


void CurveXYZView::rotatePoints(CCurve *curve, float alpha, CVector vct)
{
	int    i;
	float x, y, z, s, c, t, l, theta, m[3][3];
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{ 
			if ( curve->_point[i]._select > 0 )
			{
				x = vct.x;
				y = vct.y;
				z = vct.z;
				
				l = sqrtf(x*x + y*y + z*z);
				x = x/l;
				y = y/l;
				z = z/l;
				
				theta = alpha/180.0f*M_PIf;
				s = sinf(theta);
				c = cosf(theta);
				t = 1 - c;
				
				m[0][0] = t*x*x + c;
				m[0][1] = t*x*y - s*z;
				m[0][2] = t*x*z + s*y;
				
				m[1][0] = t*x*y + s*z;
				m[1][1] = t*y*y + c;
				m[1][2] = t*y*z - s*x;
				
				m[2][0] = t*x*z - s*y;
				m[2][1] = t*y*z + s*x;
				m[2][2] = t*z*z + c;
				
				x = curve->_point[i]._point.x;
				y = curve->_point[i]._point.y;
				z = curve->_point[i]._point.z;
				
				curve->_point[i]._point.x = m[0][0]*x + m[0][1]*y + m[0][2]*z;
				curve->_point[i]._point.y = m[1][0]*x + m[1][1]*y + m[1][2]*z;
				curve->_point[i]._point.z = m[2][0]*x + m[2][1]*y + m[2][2]*z;
			}
			
			rotatePoints(curve->_point[i]._curve, alpha, vct);
		}
	}
}


void CurveXYZView::scalePoints(CCurve *curve, CVector vct)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{ 
			if ( curve->_point[i]._select > 0 )
			{
				curve->_point[i]._point.x *= vct.x;
				curve->_point[i]._point.y *= vct.y;
				curve->_point[i]._point.z *= vct.z;
			}
			
			scalePoints(curve->_point[i]._curve, vct);
		}
	}
}


void CurveXYZView::transPoints(CCurve *curve, CVector vct)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{ 
			if ( curve->_point[i]._select > 0 )
			{
				curve->_point[i]._point.x += vct.x;
				curve->_point[i]._point.y += vct.y;
				curve->_point[i]._point.z += vct.z;
			}
			
			transPoints(curve->_point[i]._curve, vct);
		}
	}
}


void CurveXYZView::setPoints(CCurve *curve, CVector vct)
{
	int i;
	
	if ( curve != 0 )
	{
		for (i = 0; i < curve->_csize; i++)
		{ 
			if ( curve->_point[i]._select > 0 )
			{
				curve->_point[i]._point.x = vct.x;
				curve->_point[i]._point.y = vct.y;
				curve->_point[i]._point.z = vct.z;
			}
			
			setPoints(curve->_point[i]._curve, vct);
		}
	}
}


void CurveXYZView::chgPoints(CPoint p, CPoint n)
{
	int i;
	
	for (i = 0; i < _curveXYZ->_curve->_csize; i++)
	{
		if ( (_curveXYZ->_curve->_point[i]._point.x == p.x) &&
			(_curveXYZ->_curve->_point[i]._point.y == p.y) &&
			(_curveXYZ->_curve->_point[i]._point.z == p.z) )
		{
			_curveXYZ->_curve->_point[i]._point.x = n.x;
			_curveXYZ->_curve->_point[i]._point.y = n.y;
			_curveXYZ->_curve->_point[i]._point.z = n.z;
		}
	}
}


void CurveXYZView::sppPoints()
{
	int     i;
	int     l, r, a, al, ar, b, bl, br;
	char    buf[256];
	CPoint  p;
	CVector v1, v2;
	float  s;
	
	std::ofstream outFile("c:/test.log", std::ios::out);
	
	outFile << "create file" << std::endl;
	
	if ( _curveXYZ->_curve != 0 )
	{
		
		outFile << " _curveXYZ->_curve != 0" << std::endl;
		for (i = 0; i < _curveXYZ->_curve->_csize/16; i++)
		{ 
			l = r = a = al = ar = b = bl = br = -1;
			// left
			strcpy(buf, findPatch(i, 0, 4, 8, 12, 3, 7, 11, 15));
			if ( strchr(buf, '_') != 0 )
			{
				if ( sscanf(buf+6, "%i", &l) == 1 ) l = l-1;
				else l = -1;
			}
			
			outFile << " i " << i << "  left  " << l << std::endl;
			
			// right
			strcpy(buf, findPatch(i,  3,  7, 11, 15,  0,  4,  8, 12));
			if ( strchr(buf, '_') != 0 )
			{
				if ( sscanf(buf+6, "%i", &r) == 1 ) r = r-1;
				else r = -1;
			}
			
			outFile << " i " << i << "  right " << r << std::endl;
			
			// above
			strcpy(buf, findPatch(i, 12, 13, 14, 15,  0,  1,  2,  3));
			if ( strchr(buf, '_') != 0 )
			{
				if ( sscanf(buf+6, "%i", &a) == 1 ) a = a-1;
				else a = -1;
			}
			
			outFile << " i " << i << "  above " << a << std::endl;
			
			// above left
			if ( a != -1 )
			{
				strcpy(buf, findPatch(a, 0, 4, 8, 12, 3, 7, 11, 15));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &al) == 1 ) al = al-1;
					else al = -1;
				}
			}
			if ( l != -1 )
			{
				strcpy(buf, findPatch(l, 12, 13, 14, 15,  0,  1,  2,  3));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &al) == 1 ) al = al-1;
					else al = -1;
				}
			}
			
			outFile << " i " << i << "  ablef " << al << std::endl;
			
			// above right
			if ( a != -1 )
			{
				strcpy(buf, findPatch(a,  3,  7, 11, 15,  0,  4,  8, 12));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &ar) == 1 ) ar = ar-1;
					else ar = -1;
				}
			}
			if ( r != -1 )
			{
				strcpy(buf, findPatch(r, 12, 13, 14, 15,  0,  1,  2,  3));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &ar) == 1 ) ar = ar-1;
					else ar = -1;
				}
			}
			
			outFile << " i " << i << "  abrig " << ar << std::endl;
			
			// below
			strcpy(buf, findPatch(i, 0, 1, 2, 3, 12, 13, 14, 15));
			if ( strchr(buf, '_') != 0 )
			{
				if ( sscanf(buf+6, "%i", &b) == 1 ) b = b-1;
				else b = -1;
			}
			
			outFile << " i " << i << "  below " << b << std::endl;
			
			// below left
			if ( b != -1 )
			{
				strcpy(buf, findPatch(b, 0, 4, 8, 12, 3, 7, 11, 15));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &bl) == 1 ) bl = bl-1;
					else bl = -1;
				}
			}
			if ( l != -1 )
			{
				strcpy(buf, findPatch(l, 0, 1, 2, 3, 12, 13, 14, 15));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &bl) == 1 ) bl = bl-1;
					else bl = -1;
				}
			}
			
			outFile << " i " << i << "  belef " << br << std::endl;
			
			// below right
			if ( b != -1 )
			{
				strcpy(buf, findPatch(b,  3,  7, 11, 15,  0,  4,  8, 12));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &br) == 1 ) br = br-1;
					else br = -1;
				}
			}
			if ( r != -1 )
			{
				strcpy(buf, findPatch(r, 0, 1, 2, 3, 12, 13, 14, 15));
				if ( strchr(buf, '_') != 0 )
				{
					if ( sscanf(buf+6, "%i", &br) == 1 ) br = br-1;
					else br = -1;
				}
			}
			
			outFile << " i " << i << "  berig " << br << std::endl;
			
			// adjust right upper corner (a, ar, r)
			if ( (a != -1) && (r != -1) && (ar != -1) )
			{
				
				outFile << "adjust upper right corner" << std::endl;
				
				// adjust right point
				p.x = (_curveXYZ->_curve->_point[ar*16+ 5]._point.x +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ar*16+ 5]._point.y +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ar*16+ 5]._point.z +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ r*16+13]._point, p);
				
				// adjust center point
				p.x = (_curveXYZ->_curve->_point[ i*16+10]._point.x +
					_curveXYZ->_curve->_point[ i*16+11]._point.x +
					_curveXYZ->_curve->_point[ i*16+14]._point.x +
					_curveXYZ->_curve->_point[ a*16+ 6]._point.x +
					_curveXYZ->_curve->_point[ a*16+ 7]._point.x +
					_curveXYZ->_curve->_point[ar*16+ 5]._point.x +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.x +
					_curveXYZ->_curve->_point[ r*16+13]._point.x)/8;
				p.y = (_curveXYZ->_curve->_point[ i*16+10]._point.y +
					_curveXYZ->_curve->_point[ i*16+11]._point.y +
					_curveXYZ->_curve->_point[ i*16+14]._point.y +
					_curveXYZ->_curve->_point[ a*16+ 6]._point.y +
					_curveXYZ->_curve->_point[ a*16+ 7]._point.y +
					_curveXYZ->_curve->_point[ar*16+ 5]._point.y +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.y +
					_curveXYZ->_curve->_point[ r*16+13]._point.y)/8;
				p.z = (_curveXYZ->_curve->_point[ i*16+10]._point.z +
					_curveXYZ->_curve->_point[ i*16+11]._point.z +
					_curveXYZ->_curve->_point[ i*16+14]._point.z +
					_curveXYZ->_curve->_point[ a*16+ 6]._point.z +
					_curveXYZ->_curve->_point[ a*16+ 7]._point.z +
					_curveXYZ->_curve->_point[ar*16+ 5]._point.z +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.z +
					_curveXYZ->_curve->_point[ r*16+13]._point.z)/8;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+15]._point, p);
				
				// adjust upper diagonal
				v1.x = _curveXYZ->_curve->_point[ar*16+ 5]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v1.y = _curveXYZ->_curve->_point[ar*16+ 5]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v1.z = _curveXYZ->_curve->_point[ar*16+ 5]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				v2.x = _curveXYZ->_curve->_point[ i*16+10]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v2.y = _curveXYZ->_curve->_point[ i*16+10]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v2.z = _curveXYZ->_curve->_point[ i*16+10]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) s = 1;
				else s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+15]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+15]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+15]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+10]._point, p);
				
				// adjust lower diagonal
				v1.x = _curveXYZ->_curve->_point[ r*16+ 9]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v1.y = _curveXYZ->_curve->_point[ r*16+ 9]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v1.z = _curveXYZ->_curve->_point[ r*16+ 9]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				v2.x = _curveXYZ->_curve->_point[ a*16+ 6]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v2.y = _curveXYZ->_curve->_point[ a*16+ 6]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v2.z = _curveXYZ->_curve->_point[ a*16+ 6]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) s = 1;
				else s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+15]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+15]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+15]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ a*16+ 6]._point, p);
				
				// adjust right
				v1.x = _curveXYZ->_curve->_point[ r*16+13]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v1.y = _curveXYZ->_curve->_point[ r*16+13]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v1.z = _curveXYZ->_curve->_point[ r*16+13]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				v2.x = _curveXYZ->_curve->_point[ i*16+14]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v2.y = _curveXYZ->_curve->_point[ i*16+14]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v2.z = _curveXYZ->_curve->_point[ i*16+14]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) s = 1;
				else s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+15]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+15]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+15]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+14]._point, p);
				
				// adjust below
				v1.x = _curveXYZ->_curve->_point[ i*16+11]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v1.y = _curveXYZ->_curve->_point[ i*16+11]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v1.z = _curveXYZ->_curve->_point[ i*16+11]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				v2.x = _curveXYZ->_curve->_point[ a*16+ 7]._point.x -
					_curveXYZ->_curve->_point[ i*16+15]._point.x;
				v2.y = _curveXYZ->_curve->_point[ a*16+ 7]._point.y -
					_curveXYZ->_curve->_point[ i*16+15]._point.y;
				v2.z = _curveXYZ->_curve->_point[ a*16+ 7]._point.z -
					_curveXYZ->_curve->_point[ i*16+15]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) s = 1;
				else s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+15]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+15]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+15]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ a*16+ 7]._point, p);
			}
	  
			// adjust right lower corner
			if ( (b != -1) && (r != -1) && (br != -1) )
			{
  
				outFile << "adjust upper lower corner" << std::endl;
				
				// adjust right point
				p.x = (_curveXYZ->_curve->_point[ r*16+ 5]._point.x +
					_curveXYZ->_curve->_point[br*16+ 9]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ r*16+ 5]._point.y +
					_curveXYZ->_curve->_point[br*16+ 9]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ r*16+ 5]._point.z +
					_curveXYZ->_curve->_point[br*16+ 9]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ r*16+ 1]._point, p);
				
				// adjust center point
				p.x = (_curveXYZ->_curve->_point[ i*16+ 2]._point.x +
					_curveXYZ->_curve->_point[ i*16+ 6]._point.x +
					_curveXYZ->_curve->_point[ i*16+ 7]._point.x +
					_curveXYZ->_curve->_point[ r*16+ 1]._point.x +
					_curveXYZ->_curve->_point[ r*16+ 5]._point.x +
					_curveXYZ->_curve->_point[br*16+ 9]._point.x +
					_curveXYZ->_curve->_point[ b*16+10]._point.x +
					_curveXYZ->_curve->_point[ b*16+11]._point.x)/8;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 2]._point.y +
					_curveXYZ->_curve->_point[ i*16+ 6]._point.y +
					_curveXYZ->_curve->_point[ i*16+ 7]._point.y +
					_curveXYZ->_curve->_point[ r*16+ 1]._point.y +
					_curveXYZ->_curve->_point[ r*16+ 5]._point.y +
					_curveXYZ->_curve->_point[br*16+ 9]._point.y +
					_curveXYZ->_curve->_point[ b*16+10]._point.y +
					_curveXYZ->_curve->_point[ b*16+11]._point.y)/8;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 2]._point.z +
					_curveXYZ->_curve->_point[ i*16+ 6]._point.z +
					_curveXYZ->_curve->_point[ i*16+ 7]._point.z +
					_curveXYZ->_curve->_point[ r*16+ 1]._point.z +
					_curveXYZ->_curve->_point[ r*16+ 5]._point.z +
					_curveXYZ->_curve->_point[br*16+ 9]._point.z +
					_curveXYZ->_curve->_point[ b*16+10]._point.z +
					_curveXYZ->_curve->_point[ b*16+11]._point.z)/8;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 3]._point, p);
				
				// adjust upper diagonal
				v1.x = _curveXYZ->_curve->_point[ r*16+ 5]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v1.y = _curveXYZ->_curve->_point[ r*16+ 5]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v1.z = _curveXYZ->_curve->_point[ r*16+ 5]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				v2.x = _curveXYZ->_curve->_point[ b*16+10]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v2.y = _curveXYZ->_curve->_point[ b*16+10]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v2.z = _curveXYZ->_curve->_point[ b*16+10]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) s = 1;
				else s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+ 3]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+ 3]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+ 3]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ b*16+10]._point, p);
				
				// adjust lower diagonal
				v1.x = _curveXYZ->_curve->_point[br*16+ 9]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v1.y = _curveXYZ->_curve->_point[br*16+ 9]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v1.z = _curveXYZ->_curve->_point[br*16+ 9]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				v2.x = _curveXYZ->_curve->_point[ i*16+ 6]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v2.y = _curveXYZ->_curve->_point[ i*16+ 6]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v2.z = _curveXYZ->_curve->_point[ i*16+ 6]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) 
					s = 1;
				else 
					s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+ 3]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+ 3]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+ 3]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 6]._point, p);
				
				// adjust right
				v1.x = _curveXYZ->_curve->_point[ r*16+ 1]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v1.y = _curveXYZ->_curve->_point[ r*16+ 1]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v1.z = _curveXYZ->_curve->_point[ r*16+ 1]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				v2.x = _curveXYZ->_curve->_point[ i*16+ 2]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v2.y = _curveXYZ->_curve->_point[ i*16+ 2]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v2.z = _curveXYZ->_curve->_point[ i*16+ 2]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) 
					s = 1;
				else 
					s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+ 3]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+ 3]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+ 3]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 2]._point, p);
				
				// adjust below
				v1.x = _curveXYZ->_curve->_point[ b*16+11]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v1.y = _curveXYZ->_curve->_point[ b*16+11]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v1.z = _curveXYZ->_curve->_point[ b*16+11]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				v2.x = _curveXYZ->_curve->_point[ i*16+ 7]._point.x -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.x;
				v2.y = _curveXYZ->_curve->_point[ i*16+ 7]._point.y -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.y;
				v2.z = _curveXYZ->_curve->_point[ i*16+ 7]._point.z -
					_curveXYZ->_curve->_point[ i*16+ 3]._point.z;
				
				if ( v1.SProduct(v2) > 0 ) 
					s = 1;
				else 
					s = -1;
				
				p.x = _curveXYZ->_curve->_point[ i*16+ 3]._point.x + s*v1.x;
				p.y = _curveXYZ->_curve->_point[ i*16+ 3]._point.y + s*v1.y;
				p.z = _curveXYZ->_curve->_point[ i*16+ 3]._point.z + s*v1.z;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 7]._point, p);
			}
	  
			// adjust left 
			if ( (l != -1) )
			{

				outFile << "adjust left" << std::endl;
				
				// adjust left #0
				p.x = (_curveXYZ->_curve->_point[ i*16+ 1]._point.x +
					_curveXYZ->_curve->_point[ l*16+ 2]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 1]._point.y +
					_curveXYZ->_curve->_point[ l*16+ 2]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 1]._point.z +
					_curveXYZ->_curve->_point[ l*16+ 2]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 0]._point, p);
				
				// adjust left #4
				p.x = (_curveXYZ->_curve->_point[ i*16+ 5]._point.x +
					_curveXYZ->_curve->_point[ l*16+ 6]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 5]._point.y +
					_curveXYZ->_curve->_point[ l*16+ 6]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 5]._point.z +
					_curveXYZ->_curve->_point[ l*16+ 6]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 4]._point, p);
				
				// adjust left #8
				p.x = (_curveXYZ->_curve->_point[ i*16+ 9]._point.x +
					_curveXYZ->_curve->_point[ l*16+10]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 9]._point.y +
					_curveXYZ->_curve->_point[ l*16+10]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 9]._point.z +
					_curveXYZ->_curve->_point[ l*16+10]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 8]._point, p);
				
				// adjust left #12
				p.x = (_curveXYZ->_curve->_point[ i*16+13]._point.x +
					_curveXYZ->_curve->_point[ l*16+14]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+13]._point.y +
					_curveXYZ->_curve->_point[ l*16+14]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+13]._point.z +
					_curveXYZ->_curve->_point[ l*16+14]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+12]._point, p);	  
			}

			// adjust right
			if ( (r != -1) )
			{
				
				outFile << "adjust right" << std::endl;
				
				// adjust right #3
				p.x = (_curveXYZ->_curve->_point[ i*16+ 2]._point.x +
					_curveXYZ->_curve->_point[ r*16+ 1]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 2]._point.y +
					_curveXYZ->_curve->_point[ r*16+ 1]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 2]._point.z +
					_curveXYZ->_curve->_point[ r*16+ 1]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 3]._point, p);
				
				// adjust right #7
				p.x = (_curveXYZ->_curve->_point[ i*16+ 6]._point.x +
					_curveXYZ->_curve->_point[ r*16+ 5]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 6]._point.y +
					_curveXYZ->_curve->_point[ r*16+ 5]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 6]._point.z +
					_curveXYZ->_curve->_point[ r*16+ 5]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 7]._point, p);
				
				// adjust right #11
				p.x = (_curveXYZ->_curve->_point[ i*16+10]._point.x +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+10]._point.y +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+10]._point.z +
					_curveXYZ->_curve->_point[ r*16+ 9]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+11]._point, p);
				
				// adjust right #15
				p.x = (_curveXYZ->_curve->_point[ i*16+14]._point.x +
					_curveXYZ->_curve->_point[ r*16+13]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+14]._point.y +
					_curveXYZ->_curve->_point[ r*16+13]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+14]._point.z +
					_curveXYZ->_curve->_point[ r*16+13]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+15]._point, p);
			}
			
			// adjust above
			if ( (a != -1) )
			{
				
				outFile << "adjust above" << std::endl;
				
				// adjust above #12
				p.x = (_curveXYZ->_curve->_point[ i*16+ 8]._point.x +
					_curveXYZ->_curve->_point[ a*16+ 4]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 8]._point.y +
					_curveXYZ->_curve->_point[ a*16+ 4]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 8]._point.z +
					_curveXYZ->_curve->_point[ a*16+ 4]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+12]._point, p);
				
				// adjust above #13
				p.x = (_curveXYZ->_curve->_point[ i*16+ 9]._point.x +
					_curveXYZ->_curve->_point[ a*16+ 5]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 9]._point.y +
					_curveXYZ->_curve->_point[ a*16+ 5]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 9]._point.z +
					_curveXYZ->_curve->_point[ a*16+ 5]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+13]._point, p);
				
				// adjust above #14
				p.x = (_curveXYZ->_curve->_point[ i*16+10]._point.x +
					_curveXYZ->_curve->_point[ a*16+ 6]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+10]._point.y +
					_curveXYZ->_curve->_point[ a*16+ 6]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+10]._point.z +
					_curveXYZ->_curve->_point[ a*16+ 6]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+14]._point, p);
				
				// adjust above #15
				p.x = (_curveXYZ->_curve->_point[ i*16+11]._point.x +
					_curveXYZ->_curve->_point[ a*16+ 7]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+11]._point.y +
					_curveXYZ->_curve->_point[ a*16+ 7]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+11]._point.z +
					_curveXYZ->_curve->_point[ a*16+ 7]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+15]._point, p);	  
			}
	  
			// adjust below
			if ( (b != -1) )
			{
				
				outFile << "adjust below" << std::endl;
				
				// adjust below #0
				p.x = (_curveXYZ->_curve->_point[ i*16+ 4]._point.x +
					_curveXYZ->_curve->_point[ b*16+ 8]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 4]._point.y +
					_curveXYZ->_curve->_point[ b*16+ 8]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 4]._point.z +
					_curveXYZ->_curve->_point[ b*16+ 8]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 0]._point, p);
				
				// adjust below #1
				p.x = (_curveXYZ->_curve->_point[ i*16+ 5]._point.x +
					_curveXYZ->_curve->_point[ b*16+ 9]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 5]._point.y +
					_curveXYZ->_curve->_point[ b*16+ 9]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 5]._point.z +
					_curveXYZ->_curve->_point[ b*16+ 9]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 1]._point, p);
				
				// adjust below #2
				p.x = (_curveXYZ->_curve->_point[ i*16+ 6]._point.x +
					_curveXYZ->_curve->_point[ b*16+10]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 6]._point.y +
					_curveXYZ->_curve->_point[ b*16+10]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 6]._point.z +
					_curveXYZ->_curve->_point[ b*16+10]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 2]._point, p);
				
				// adjust below #3
				p.x = (_curveXYZ->_curve->_point[ i*16+ 7]._point.x +
					_curveXYZ->_curve->_point[ b*16+11]._point.x)/2;
				p.y = (_curveXYZ->_curve->_point[ i*16+ 7]._point.y +
					_curveXYZ->_curve->_point[ b*16+11]._point.y)/2;
				p.z = (_curveXYZ->_curve->_point[ i*16+ 7]._point.z +
					_curveXYZ->_curve->_point[ b*16+11]._point.z)/2;
				
				chgPoints(_curveXYZ->_curve->_point[ i*16+ 3]._point, p);	  
			}
		}
	}
}


void CurveXYZView::setFPatchX(CCurve *curve)
{
	int    i, j, k, v;
	CPoint p[4];
	
	if ( curve != 0 )
	{
		if ( curve->_ctype == eBEZIERSURFACE )
		{
			for (i = 0; i < curve->_csize/16; i++)
			{
				v = 1;
				for (j = 0; j < 16; j++)
				{
					if ( curve->_point[i*16+j]._select > 0 )
						v = 0;
				}
				if ( v == 0 )
				{
					for (j = 0; j < 4; j++)
					{ 
						for (k = 0; k < 4; k++)
							p[k] = curve->_point[i*16+j*4+k]._point;
						
						for (k = 0; k < 4; k++)
							curve->_point[i*16+j*4+k]._point = p[3-k];
					}
				}
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			setFPatchX(curve->_point[i]._curve);
	}
}


void CurveXYZView::setFPatchY(CCurve *curve)
{
	int    i, j, k, v;
	CPoint p[4];
	
	if ( curve != 0 )
	{
		if ( curve->_ctype == eBEZIERSURFACE )
		{
			for (i = 0; i < curve->_csize/16; i++)
			{
				v = 1;
				for (j = 0; j < 16; j++)
				{
					if ( curve->_point[i*16+j]._select > 0 )
						v = 0;
				}
				if ( v == 0 )
				{
					for (j = 0; j < 4; j++)
					{ 
						for (k = 0; k < 4; k++)
							p[k] = curve->_point[i*16+j+k*4]._point;
						
						for (k = 0; k < 4; k++)
							curve->_point[i*16+j+k*4]._point = p[3-k];
					}
				}
			}
		}
		
		for (i = 0; i < curve->_csize; i++)
			setFPatchY(curve->_point[i]._curve);
	}
}


void CurveXYZView::readError(const char* msg)
{
	std::cerr << "CurveXYZView::interpretError()" << msg << std::endl;
}


void CurveXYZView::readConfig(char* command)
{
	static char *labels[] = 
	{
		"//",                      /*  0 */
			"setClear3f:",
			"setXAxis5f2i3f:",
			"setYAxis5f2i3f:",
			"setZAxis5f2i3f:",
			"setGrid1f4s:",            /*  5 */
			"setGridXY1f2i3f:",
			"setGridXZ1f2i3f:",
			"setGridYZ1f2i3f:",
			"setPPoint1f2i3f:",
			"setVPoint1f2i3f:",        /* 10 */
			"setPConvex1f2i3f:",
			"setVConvex1f2i3f:",
			"setPCurve1f4i3f:",
			"setVCurve1f4i3f:",
			"setSelectB2f2i3f:",       /* 15 */
			"setSelectX5f2i6f:",
			"setSelectY5f2i6f:",
			"setSelectZ5f2i6f:",
			"setMouse2f:",
			"setStatClear3f:",         /* 20 */
			"setStatFont3f:",
			"setArcball2s4f1s7f1s4f:",
			0                       /* the last one must be 0 */
	};
	
	char **labelPtr;
	int  labelIndex;
	char format[200];
	
	/*  determine parameter type - index, when found, is used in switch */
	labelPtr = labels;
	for (labelIndex=0; *labelPtr != 0; labelIndex++) 
	{
		if ( strncmp(command, *(labelPtr), strlen(*(labelPtr))) == 0 ) 
			break;
		
		labelPtr++;
	}
	
	char  c20[4][20];
	float flo[15];
	int   num[8];
	
	/* process accordingly */
	switch (labelIndex) 
	{
    case 0: // //
		
		break;
		
    case 1: // setClear3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f");
		
		if ( 3 == sscanf(command,format,&flo[0],&flo[1],&flo[2]) )
		{
			_clear_r = flo[0];
			_clear_g = flo[1];
			_clear_b = flo[2];
		}
		else
			readError(command);
		
		break;
		
    case 2: // setXAxis5f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f %f %f %i %i %f %f %f");
		
		if ( 10 == sscanf(command,format,&flo[0],&flo[1],&flo[2],&flo[3],&flo[4],
			&num[0],&num[1],&flo[5],&flo[6],&flo[7]) )
		{
			_xaxis_radius = flo[0];
			_xaxis_length = flo[1];
			_xaxis_base = flo[2];
			_xaxis_top = flo[3];
			_xaxis_height = flo[4];
			_xaxis_slices = num[0];
			_xaxis_stacks = num[1];
			_xaxis_r = flo[5];
			_xaxis_g = flo[6];
			_xaxis_b = flo[7];
		}
		else
			readError(command);
		
		break;
		
    case 3: // setYAxis5f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f %f %f %i %i %f %f %f");
		
		if ( 10 == sscanf(command,format,&flo[0],&flo[1],&flo[2],&flo[3],&flo[4],
			&num[0],&num[1],&flo[5],&flo[6],&flo[7]) )
		{
			_yaxis_radius = flo[0];
			_yaxis_length = flo[1];
			_yaxis_base = flo[2];
			_yaxis_top = flo[3];
			_yaxis_height = flo[4];
			_yaxis_slices = num[0];
			_yaxis_stacks = num[1];
			_yaxis_r = flo[5];
			_yaxis_g = flo[6];
			_yaxis_b = flo[7];
		}
		else
			readError(command);
		
		break;
		
    case 4: // setZAxis5f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f %f %f %i %i %f %f %f");
		
		if ( 10 == sscanf(command,format,&flo[0],&flo[1],&flo[2],&flo[3],&flo[4],
			&num[0],&num[1],&flo[5],&flo[6],&flo[7]) )
		{
			_zaxis_radius = flo[0];
			_zaxis_length = flo[1];
			_zaxis_base = flo[2];
			_zaxis_top = flo[3];
			_zaxis_height = flo[4];
			_zaxis_slices = num[0];
			_zaxis_stacks = num[1];
			_zaxis_r = flo[5];
			_zaxis_g = flo[6];
			_zaxis_b = flo[7];
		}
		else
			readError(command);
		
		break;
		
    case 5: // setGrid1f4s
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %s %s %s %s");
		
		if ( 5 == sscanf(command,format,&flo[0],&c20[0],&c20[1],&c20[2],
			&c20[3]) )
		{
			_grid_size[0] = flo[0];
			_grid = strcmp(c20[0],"yes") == 0;
			_gridXY = strcmp(c20[1],"yes") == 0;
			_gridXZ = strcmp(c20[2],"yes") == 0;
			_gridYZ = strcmp(c20[3],"yes") == 0;
		}
		else
			readError(command);
		
		break;
		
    case 6: // setGridXY1f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %f %f %f");
		
		if ( 6 == sscanf(command,format,&flo[0],&num[0],&num[1],&flo[1],&flo[2],
			&flo[3]) )
		{
			_gridxy_radius = flo[0];
			_gridxy_slices = num[0];
			_gridxy_stacks = num[1];
			_gridxy_r = flo[1];
			_gridxy_g = flo[2];
			_gridxy_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 7: // setGridXZ1f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %f %f %f");
		
		if ( 6 == sscanf(command,format,&flo[0],&num[0],&num[1],&flo[1],&flo[2],
			&flo[3]) )
		{
			_gridxz_radius = flo[0];
			_gridxz_slices = num[0];
			_gridxz_stacks = num[1];
			_gridxz_r = flo[1];
			_gridxz_g = flo[2];
			_gridxz_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 8: // setGridYZ1f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %f %f %f");
		
		if ( 6 == sscanf(command,format,&flo[0],&num[0],&num[1],&flo[1],&flo[2],
			&flo[3]) )
		{
			_gridyz_radius = flo[0];
			_gridyz_slices = num[0];
			_gridyz_stacks = num[1];
			_gridyz_r = flo[1];
			_gridyz_g = flo[2];
			_gridyz_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 9: // setPPoint1f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %f %f %f");
		
		if ( 6 == sscanf(command,format,&flo[0],&num[0],&num[1],&flo[1],&flo[2],
			&flo[3]) )
		{
			_ppoint_radius = flo[0];
			_ppoint_slices = num[0];
			_ppoint_stacks = num[1];
			_ppoint_r = flo[1];
			_ppoint_g = flo[2];
			_ppoint_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 10: // setVPoint1f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %f %f %f");
		
		if ( 6 == sscanf(command,format,&flo[0],&num[0],&num[1],&flo[1],&flo[2],
			&flo[3]) )
		{
			_vpoint_radius = flo[0];
			_vpoint_slices = num[0];
			_vpoint_stacks = num[1];
			_vpoint_r = flo[1];
			_vpoint_g = flo[2];
			_vpoint_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 11: // setPConvex1f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %f %f %f");
		
		if ( 6 == sscanf(command,format,&flo[0],&num[0],&num[1],&flo[1],&flo[2],
			&flo[3]) )
		{
			_pconvex_radius = flo[0];
			_pconvex_slices = num[0];
			_pconvex_stacks = num[1];
			_pconvex_r = flo[1];
			_pconvex_g = flo[2];
			_pconvex_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 12: // setVConvex1f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %f %f %f");
		
		if ( 6 == sscanf(command,format,&flo[0],&num[0],&num[1],&flo[1],&flo[2],
			&flo[3]) )
		{
			_vconvex_radius = flo[0];
			_vconvex_slices = num[0];
			_vconvex_stacks = num[1];
			_vconvex_r = flo[1];
			_vconvex_g = flo[2];
			_vconvex_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 13: // setPCurve1f4i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %i %i %f %f %f");
		
		if ( 8 == sscanf(command,format,&flo[0],&num[0],&num[1],&num[2],&num[3],
			&flo[1],&flo[2],&flo[3]) )
		{
			_pcurve_radius = flo[0];
			_pcurve_slices = num[0];
			_pcurve_stacks = num[1];
			_pcurve_points = num[2];
			_pcurve_lines = num[3];
			_pcurve_r = flo[1];
			_pcurve_g = flo[2];
			_pcurve_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 14: // setVCurve1f4i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %i %i %i %i %f %f %f");
		
		if ( 8 == sscanf(command,format,&flo[0],&num[0],&num[1],&num[2],&num[3],
			&flo[1],&flo[2],&flo[3]) )
		{
			_vcurve_radius = flo[0];
			_vcurve_slices = num[0];
			_vcurve_stacks = num[1];
			_vcurve_points = num[2];
			_vcurve_lines = num[3];
			_vcurve_r = flo[1];
			_vcurve_g = flo[2];
			_vcurve_b = flo[3];
		}
		else
			readError(command);
		
		break;
		
    case 15: // setSelectB2f2i3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %i %i %f %f %f");
		
		if ( 7 == sscanf(command,format,&flo[0],&flo[1],&num[0],&num[1],&flo[2],
			&flo[3],&flo[4]) )
		{
			_selectb_radius = flo[0];
			_selectb_length = flo[1];
			_selectb_slices = num[0];
			_selectb_stacks = num[1];
			_selectb_r = flo[2];
			_selectb_g = flo[3];
			_selectb_b = flo[4];
		}
		else
			readError(command);
		
		break;
		
    case 16: // setSelectX5f2i6f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f %f %f %i %i %f %f %f %f %f %f");
		
		if ( 13 == sscanf(command,format,&flo[0],&flo[1],&flo[2],&flo[3],&flo[4],
			&num[0],&num[1],&flo[5],&flo[6],&flo[7],&flo[8],&flo[9],&flo[10]) )
		{
			_selectx_radius = flo[0];
			_selectx_length = flo[1];
			_selectx_base = flo[2];
			_selectx_top = flo[3];
			_selectx_height = flo[4];
			_selectx_slices = num[0];
			_selectx_stacks = num[1];
			_selectx_r[0] = flo[5];
			_selectx_g[0] = flo[6];
			_selectx_b[0] = flo[7];
			_selectx_r[1] = flo[8];
			_selectx_g[1] = flo[9];
			_selectx_b[1] = flo[10];
		}
		else
			readError(command);
		
		break;
		
    case 17: // setSelectY5f2i6f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f %f %f %i %i %f %f %f %f %f %f");
		
		if ( 13 == sscanf(command,format,&flo[0],&flo[1],&flo[2],&flo[3],&flo[4],
			&num[0],&num[1],&flo[5],&flo[6],&flo[7],&flo[8],&flo[9],&flo[10]) )
		{
			_selecty_radius = flo[0];
			_selecty_length = flo[1];
			_selecty_base = flo[2];
			_selecty_top = flo[3];
			_selecty_height = flo[4];
			_selecty_slices = num[0];
			_selecty_stacks = num[1];
			_selecty_r[0] = flo[5];
			_selecty_g[0] = flo[6];
			_selecty_b[0] = flo[7];
			_selecty_r[1] = flo[8];
			_selecty_g[1] = flo[9];
			_selecty_b[1] = flo[10];
		}
		else
			readError(command);
		
		break;
		
    case 18: // setSelectZ5f2i6f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f %f %f %i %i %f %f %f %f %f %f");
		
		if ( 13 == sscanf(command,format,&flo[0],&flo[1],&flo[2],&flo[3],&flo[4],
			&num[0],&num[1],&flo[5],&flo[6],&flo[7],&flo[8],&flo[9],&flo[10]) )
		{
			_selectz_radius = flo[0];
			_selectz_length = flo[1];
			_selectz_base = flo[2];
			_selectz_top = flo[3];
			_selectz_height = flo[4];
			_selectz_slices = num[0];
			_selectz_stacks = num[1];
			_selectz_r[0] = flo[5];
			_selectz_g[0] = flo[6];
			_selectz_b[0] = flo[7];
			_selectz_r[1] = flo[8];
			_selectz_g[1] = flo[9];
			_selectz_b[1] = flo[10];
		}
		else
			readError(command);
		
		break;
		
    case 19: // setMouse2f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f %f");
		
		if ( 2 == sscanf(command,format,&flo[0],&flo[1]) )
		{
			_mouse_rotXY   = flo[0];
			_mouse_scale   = flo[1];
		}
		else
			readError(command);
		
		break;
		
    case 20: // setStatClear3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f");
		
		if ( 3 == sscanf(command,format,&flo[0],&flo[1],&flo[2]) )
		{
			_stat_clear_r = flo[0];
			_stat_clear_g = flo[1];
			_stat_clear_b = flo[2];
		}
		else
			readError(command);
		
		break;
		
    case 21: // setStatFont3f
		strcpy(format,labels[labelIndex]);
		strcat(format," %f %f %f");
		
		if ( 3 == sscanf(command,format,&flo[0],&flo[1],&flo[2]) )
		{
			_stat_font_r = flo[0];
			_stat_font_g = flo[1];
			_stat_font_b = flo[2];
		}
		else
			readError(command);
		
		break;
		
    case 22: // setArcball2s4f1s7f1s4f
		strcpy(format,labels[labelIndex]);
		strcat(format," %s %s %f %f %f %f %s %f %f %f %f %f %f %f %s %f %f %f %f");
		
		if ( 19 == sscanf(command,format,&c20[0],&c20[1],&flo[0],&flo[1],
			&flo[2],&flo[3],&c20[2],&flo[4],&flo[5],&flo[6],&flo[7],
			&flo[8],&flo[9],&flo[10],&c20[3],&flo[11],&flo[12],&flo[13],
			&flo[14]) )
		{
			_arcball_wrap = strcmp(c20[0],"yes") == 0;
			
			_arcball_circle = strcmp(c20[1],"yes") == 0;
			_arcball_circle_radius = flo[0];
			_arcball_circle_r = flo[1];
			_arcball_circle_g = flo[2];
			_arcball_circle_b = flo[3];
			
			_arcball_constraints = strcmp(c20[2],"yes") == 0;
			_arcball_constraints_radius = flo[4];
			_arcball_constraints_r[0] = flo[5];
			_arcball_constraints_g[0] = flo[6];
			_arcball_constraints_b[0] = flo[7];
			_arcball_constraints_r[1] = flo[8];
			_arcball_constraints_g[1] = flo[9];
			_arcball_constraints_b[1] = flo[10];
			
			_arcball_arc = strcmp(c20[3],"yes") == 0;
			_arcball_arc_radius = flo[11];
			_arcball_arc_r = flo[12];
			_arcball_arc_g = flo[13];
			_arcball_arc_b = flo[14];
		}
		else
			readError(command);
		
		break;
		
    default: // 0
		if ( strlen(command) > 3 )
			std::cerr << "error: interpretCommand() -> " << command << std::endl;
	}
}


void CurveXYZView::openConfig(const char* filename)
{
	const int bufsize = 1024;
	
	FILE *fp;
	char  buffer[bufsize], c;
	int   index = 0;
	
	if ( (fp = fopen(filename,"r")) != 0 ) 
	{
		c = (char)fgetc(fp);
		
		while ( !feof(fp) )
		{
			index = 0;
			while ( c != '\n' )
			{
				buffer[index] = c;
				index++;
				c = (char)fgetc(fp);
			}
			
			buffer[index] = 0;
			
			readConfig(buffer);
			
			c = (char)fgetc(fp);
		}
		
		fclose(fp);
	}
	else
		std::cerr << "WARNING: Can't open the file " << buffer << std::endl;
}


void CurveXYZView::initConfig()
{
	const char* directory = getenv("VLABCONFIGDIR");
	
	FILE *fp;
	char  buffer[256];
	
	if ( (fp = fopen(CONFIGFILE, "r")) != 0 )
	{
		fclose(fp);
		
		openConfig(CONFIGFILE);
	}
	else
	{
		if ( directory != 0 )
		{
			strcpy(buffer, directory);
			strcat(buffer, "/");
			strcat(buffer, CONFIGFILE);
			
			openConfig(buffer);
		}
	}
}


void CurveXYZView::mInitCurve(ECURVE type)
{
	undoSave();
	
	delete _curveXYZ->_curve;
	
	_curveXYZ->_curve = _curveXYZ->init(type);
	
	fitPoints();
	
	updateEdit();
	updateView();
}


void CurveXYZView::mInitBSpline()
{
	mInitCurve(eBSPLINE);
}


void CurveXYZView::mInitBzSurface()
{
	mInitCurve(eBEZIERSURFACE);
}


void CurveXYZView::mOpen(const char* filename)
{
	undoSave();
	
	openFile(filename);
	
	fitPoints();
	
	updateEdit();
	updateView();
}


void CurveXYZView::mSave(const char* filename)
{
	saveFile(filename);
}


void CurveXYZView::mInitView()
{
	initView();
	fitPoints();
	
	updateView();
}


void CurveXYZView::mInitFit()
{
	fitPoints();
	
	updateView();
}


void CurveXYZView::mUndo()
{
	undoUndo();
}


void CurveXYZView::mRedo()
{
	undoRedo();
}


void CurveXYZView::mPointer()
{
	_pointer = true;
	_rotTB = false;
	_scaleTB = false;
	_transTB = false;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mRotate()
{
	_pointer = false;
	_rotTB = true;
	_scaleTB = false;
	_transTB = false;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mScale()
{
	_pointer = false;
	_rotTB = false;
	_scaleTB = true;
	_transTB = false;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mTranslate()
{
	_pointer = false;
	_rotTB = false;
	_scaleTB = false;
	_transTB = true;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mAxes()
{
	_axes = !_axes;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mGridGrid()
{
	_grid = !_grid;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mGridXY()
{
	_gridXY = !_gridXY;
	if ( _gridXY )
		_grid = true;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mGridXZ()
{
	_gridXZ = !_gridXZ;
	if ( _gridXZ )
		_grid = true;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mGridYZ()
{
	_gridYZ = !_gridYZ;
	if ( _gridYZ )
		_grid = true;
	
	updateEdit();
	updateView();
}


void CurveXYZView::mAddPred()
{
	undoSave();
	addPointPred();
	updateView();
}


void CurveXYZView::mAddSucc()
{
	undoSave();
	addPointSucc();
	updateView();
}


void CurveXYZView::mDelPoint()
{
	undoSave();
	delPoint();
	updateView();
}


void CurveXYZView::mAddN()
{
	undoSave();
	addSurfaceNS(3);
	updateView();
}


void CurveXYZView::mAddNE()
{
	undoSave();
	addSurfaceNESESWNW(3, 3);
	updateView();
}


void CurveXYZView::mAddE()
{
	undoSave();
	addSurfaceWE(3);
	updateView();
}


void CurveXYZView::mAddSE()
{
	undoSave();
	addSurfaceNESESWNW(0, 3);
	updateView();
}


void CurveXYZView::mAddS()
{
	undoSave();
	addSurfaceNS(0);
	updateView();
}


void CurveXYZView::mAddSW()
{
	undoSave();
	addSurfaceNESESWNW(0, 0);
	updateView();
}


void CurveXYZView::mAddW()
{
	undoSave();
	addSurfaceWE(0);
	updateView();
}


void CurveXYZView::mAddNW()
{
	undoSave();
	addSurfaceNESESWNW(3, 0);
	updateView();
}


void CurveXYZView::mDelSurface()
{
	undoSave();
	delSurface();
	updateView();
}


void CurveXYZView::mMultX(int mult)
{
	if ( _selectc != 0 )
	{
		undoSave();
		setPMult(_curveXYZ->_curve, mult);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mMult1()
{
	mMultX(1);
}


void CurveXYZView::mMult2()
{
	mMultX(2);
}


void CurveXYZView::mMult3()
{
	mMultX(3);
}


void CurveXYZView::mMult4()
{
	mMultX(4);
}


void CurveXYZView::mTypeX(ECURVE type)
{
	if ( _selectc != 0 )
	{
		undoSave();
		
		_selectc->_ctype = type;
		
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mTypeBSpline()
{
	mTypeX(eBSPLINE);
}


void CurveXYZView::mTypeBClosed()
{
	mTypeX(eBSPLINECLOSED);
}


void CurveXYZView::mTypeBEndpoint()
{
	mTypeX(eBSPLINEENDPOINT);
}


void CurveXYZView::mTypeBPhantom()
{
	mTypeX(eBSPLINEPHANTOM);
}


void CurveXYZView::mTypeBzSpline()
{
	mTypeX(eBEZIER);
}


void CurveXYZView::mTypeBzSurface()
{
	mTypeX(eBEZIERSURFACE);
}


void CurveXYZView::mVarX(ECURVE type)
{
	undoSave();
	createVariation(_curveXYZ->_curve, type);
	updateView();
}


void CurveXYZView::mVarBSpline()
{
	mVarX(eBSPLINE);  
}


void CurveXYZView::mVarBzSurface()
{
	mVarX(eBEZIERSURFACE);
}


void CurveXYZView::mVarDel()
{
	if ( _selectc != 0 )
	{
		undoSave();
		delete _selectc->_point[_selectp]._curve;
		_selectc->_point[_selectp]._curve = 0;
		updateView();
	}
}


void CurveXYZView::mDivide()
{
	if ( _selectc != 0 )
	{
		undoSave();
		_curveXYZ->divideCurve(_selectc);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mSelectAll()
{
	undoSave();
	_selecti = 0;
	selectAll(_curveXYZ->_curve);
	updateEdit();
	updateView();
}


void CurveXYZView::mSelectCurve()
{
	if ( _selectc != 0 )
	{
		undoSave();
		selectCurve(_selectc);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mSelectPX(int point)
{
	undoSave();
	_selects = 0;
	selectPoint(_curveXYZ->_curve, point);
	updateEdit();
	updateView();
}


void CurveXYZView::mAlign()
{
	if ( _selectc != 0 )
	{
		undoSave();
		alignPoints(_curveXYZ->_curve, _selectc->_point[_selectp]._point);
		updateView();
	}
}


void CurveXYZView::mDPoints()
{
	_dpoints = !_dpoints;
	updateView();
}


void CurveXYZView::mDConvex()
{
	_dconvex = !_dconvex;
	updateView();
}


void CurveXYZView::mDCurve()
{
	_dcurve = !_dcurve;
	updateView();
}


void CurveXYZView::mShowAll()
{
	undoSave();
	setShowAll(_curveXYZ->_curve);
	updateEdit();
	updateView();
}


void CurveXYZView::mShowCurve()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setShowCurve(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mShowPatch()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setShowPatch(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mShowPoint()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setShowPoint(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mHideCurve()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setHideCurve(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mHidePatch()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setHidePatch(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mHidePoint()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setDarkPoint(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mBrightAll()
{
	undoSave();
	setBrightAll(_curveXYZ->_curve);
	updateEdit();
	updateView();
}


void CurveXYZView::mBrightCurve()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setBrightCurve(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mBrightPatch()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setBrightPatch(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mBrightPoint()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setBrightPoint(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mDarkCurve()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setDarkCurve(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mDarkPatch()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setDarkPatch(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mDarkPoint()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setDarkPoint(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mRotate(float alpha, float x, float y, float z)
{
	CVector hV;
	undoSave();
	hV.x = x;
	hV.y = y;
	hV.z = z;
	rotatePoints(_curveXYZ->_curve, alpha, hV);
	updateView();
}


void CurveXYZView::mScale(float x, float y, float z)
{
	CVector hV;
	undoSave();
	hV.x = x;
	hV.y = y;
	hV.z = z;
	scalePoints(_curveXYZ->_curve, hV);
	updateView();
}


void CurveXYZView::mTranslate(float x, float y, float z)
{
	CVector hV;
	undoSave();
	hV.x = x;
	hV.y = y;
	hV.z = z;
	transPoints(_curveXYZ->_curve, hV);
	updateView();
}


void CurveXYZView::mSet(float x, float y, float z)
{
	CVector hV;
	undoSave();
	hV.x = x;
	hV.y = y;
	hV.z = z;
	setPoints(_curveXYZ->_curve, hV);
	updateView();
}


void CurveXYZView::mSPatchAll()
{
	undoSave();
	sppPoints();
	updateView();
}


void CurveXYZView::mSPatchCorner()
{
	undoSave();
	sppPoints();
	updateView();
}


void CurveXYZView::mSPatchLeft()
{
	undoSave();
	sppPoints();
	updateView();
}


void CurveXYZView::mSPatchRight()
{
	undoSave();
	sppPoints();
	updateView();
}


void CurveXYZView::mSPatchTop()
{
	undoSave();
	sppPoints();
	updateView();
}


void CurveXYZView::mSPatchBottom()
{
	undoSave();
	sppPoints();
	updateView();
}


void CurveXYZView::mFPatchX()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setFPatchX(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mFPatchY()
{
	if ( _selectc != 0 )
	{
		undoSave();
		setFPatchY(_curveXYZ->_curve);
		updateEdit();
		updateView();
	}
}


void CurveXYZView::mCBlossom(float a, float b, float c, float d, float f, float t)
{
	int    i;
	float u1, u2, u3;
	CPoint p[4];
	
	undoSave();
	
	delete _curveXYZ->_curve;
	
	_curveXYZ->_curve = new CCurve;
	_curveXYZ->_curve->_csize = 4;
	_curveXYZ->_curve->_point = new CCPoint[4];
	_curveXYZ->_curve->_ctype = eBEZIER;
	
	u1=f; u2=f; u3=f;
	p[0].x = (u1 + u2 + u3)/3;
	u1=f; u2=f; u3=t;
	p[1].x = (u1 + u2 + u3)/3;
	u1=f; u2=t; u3=t;
	p[2].x = (u1 + u2 + u3)/3;
	u1=t; u2=t; u3=t;
	p[3].x = (u1 + u2 + u3)/3;
	
	u1=f; u2=f; u3=f;
	p[0].y = a + b*((u1 + u2 + u3)/3) + c*((u1*u2 + u1*u3 + u2*u3)/3) + d*u1*u2*u3;
	u1=f; u2=f; u3=t;
	p[1].y = a + b*((u1 + u2 + u3)/3) + c*((u1*u2 + u1*u3 + u2*u3)/3) + d*u1*u2*u3;
	u1=f; u2=t; u3=t;
	p[2].y = a + b*((u1 + u2 + u3)/3) + c*((u1*u2 + u1*u3 + u2*u3)/3) + d*u1*u2*u3;
	u1=t; u2=t; u3=t;
	p[3].y = a + b*((u1 + u2 + u3)/3) + c*((u1*u2 + u1*u3 + u2*u3)/3) + d*u1*u2*u3;
	
	for (i = 0; i < 4; i++)
	{
		_curveXYZ->_curve->_point[i]._point.x = p[i].x;
		_curveXYZ->_curve->_point[i]._point.y = p[i].y;
		_curveXYZ->_curve->_point[i]._point.z = 0;
	}
	
	fitPoints();
	
	updateEdit();
	updateView();
}


void CurveXYZView::mCInterpolation(float a, float b, float c, float d, float f, float t, int n)
{
	int     i;
	float  x;
	float *knot, *datX, *datY, *resX, *resY;
	
	if ( n > 3 )
	{
		undoSave();
		
		delete _curveXYZ->_curve;
		
		_curveXYZ->_curve = new CCurve;
		_curveXYZ->_curve->_csize = n;
		_curveXYZ->_curve->_point = new CCPoint[n];
		_curveXYZ->_curve->_ctype = eBSPLINE;
		
		knot = new float[n-2];
		datX = new float[n];
		datY = new float[n];
		resX = new float[n];
		resY = new float[n];
		
		for (i = 0; i < n-2; i++)
			knot[i]=(float)i;
		
		for (i = 0; i < n; i++)
		{
			x = (t - f)*float(i)/float(n - 1) + f;
			datX[i] = x;
			datY[i] = a + b*x + c*x*x + d*x*x*x;
		}
		
		c2_spline(knot, n-3, datX, datY, resX, resY);
		
		for (i = 0; i < n; i++)
		{
			_curveXYZ->_curve->_point[i]._point.x = resX[i];
			_curveXYZ->_curve->_point[i]._point.y = resY[i];
			_curveXYZ->_curve->_point[i]._point.z = 0;
		}
		
		_curveXYZ->_curve->_point[0  ]._pmult = 3;
		_curveXYZ->_curve->_point[n-1]._pmult = 3;
		
		fitPoints();
		
		updateEdit();
		updateView();
	}
	
}
