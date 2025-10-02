/**************************************************************************

  File:		glutils.h
  Created:	11-Dec-97


  Declaration of various OpenGL utility classes


**************************************************************************/


#ifndef __GLUTILS_H__
#define __GLUTILS_H__


class PushPopMatrix
{
public:
	PushPopMatrix()
	{ glPushMatrix(); }
	~PushPopMatrix()
	{ glPopMatrix(); }
};


class GLprimitive
{
protected:
	GLprimitive(GLenum mode)
	{ 
		assert(!_exists);
		glBegin(mode);
#ifdef _DEBUG
		_exists = true;	
		_counter = 0;
#endif
	}
public:
	~GLprimitive()
	{
		glEnd();
#ifdef _DEBUG
		_exists = false;
#endif
	}
	void Vertex(float x, float y, float z = 0.0)
	{
		glVertex3f(x, y, z);
#ifdef _DEBUG
		_counter++;
#endif
	}
	void Vertex(const float* arr)
	{
		glVertex3fv(arr);
#ifdef _DEBUG
		_counter++;
#endif
	}
	void Normal(const float* arr)
	{
		glNormal3fv(arr);
	}
	void EvalCoord(float x, float y)
	{
		glEvalCoord2f(x, y);
#ifdef _DEBUG
		_counter++;
#endif
	}

#ifdef _DEBUG
protected:
	int _counter;
	static bool _exists;
#endif
};

class GLpolygon : public GLprimitive
{
public:
	GLpolygon() : GLprimitive(GL_POLYGON)
	{}
#ifdef _DEBUG
	~GLpolygon()
	{
		assert(_counter>2);
	}
#endif
};

class GLlines : public GLprimitive
{
public:
	GLlines() : GLprimitive(GL_LINES)
	{}
#ifdef _DEBUG
	~GLlines()
	{
		assert(!(_counter % 2));
	}
#endif
};

class GLlineloop : public GLprimitive
{
public:
	GLlineloop() : GLprimitive(GL_LINE_LOOP)
	{}
#ifdef _DEBUG
	~GLlineloop()
	{
		assert(_counter>1);
	}
#endif
};

class GLlinestrip : public GLprimitive
{
public:
	GLlinestrip( 
#ifdef _DEBUG
		bool nocountercheck = false
#endif
		) : GLprimitive(GL_LINE_STRIP)
	{
#ifdef _DEBUG
		if (nocountercheck)
			_counter = 2;
#endif
	}
#ifdef _DEBUG
	~GLlinestrip()
	{
		assert(_counter>1);
	}
#endif
};

class GLpoints : public GLprimitive
{
public:
	GLpoints() : GLprimitive(GL_POINTS)
	{}
#ifdef _DEBUG
	~GLpoints()
	{
		assert(_counter>0);
	}
#endif
};

class GLenable
{
public:
	GLenable(GLenum what) : _what(what)
	{ 
		glGetBooleanv(what, &_on);
		glEnable(_what); 
	}
	~GLenable()
	{ 
		if (!_on)
			glDisable(_what);
	}
private:
	GLboolean _on;
	GLenum _what;
};

class GLdisable
{
public:
	GLdisable(GLenum what) : _what(what)
	{ 
		glGetBooleanv(what, &_on);
		glDisable(_what); 
	}
	~GLdisable()
	{ 
		if (_on)
			glEnable(_what);	
	}
private:
	GLboolean _on;
	GLenum _what;
};

class GLpointSize
{
public:
	GLpointSize(float sz)
	{
		glGetFloatv(GL_POINT_SIZE, &_prevSize);
		glPointSize(sz);
	}
	~GLpointSize()
	{
		glPointSize(_prevSize);
	}
private:
	float _prevSize;
};



class GLlineWidth
{
public:
	GLlineWidth(float sz)
	{
		glGetFloatv(GL_LINE_WIDTH, &_prevWidth);
		glLineWidth(sz);
	}
	~GLlineWidth()
	{
		glLineWidth(_prevWidth);
	}
private:
	float _prevWidth;
};

class GLselectmode
{
public:
	GLselectmode(int& hits) : _hits(hits)
	{ glRenderMode(GL_SELECT); }
	~GLselectmode()
	{ _hits = glRenderMode(GL_RENDER); }
private:
	int& _hits;
};


class GLOnOff
{
public:
	GLOnOff(GLenum cap) : _cap(cap)
	{ glEnable(_cap); }
	~GLOnOff()
	{ glDisable(_cap); }
private:
	const GLenum _cap;
};


class GLOffOn
{
public:
	GLOffOn(GLenum cap) : _cap(cap)
	{ glDisable(_cap); }
	~GLOffOn()
	{ glEnable(_cap); }
private:
	const GLenum _cap;
};


#endif
