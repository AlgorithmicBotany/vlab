#include <vector>
#include <memory>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "contour.h"
#include "matrix.h"
#include "knot.h"
#include "lstudioptns.h"

#include "resource.h"



INIT_COUNTER(Contour);

static const float SplineInit[16] =
{
	-1.0f/6.0f, 3.0f/6.0f, -3.0f/6.0f, 1.0f/6.0f,
		3.0f/6.0f, -6.0f/6.0f, 3.0f/6.0f, 0.0f/6.0f, 
		-3.0f/6.0f, 0.0f/6.0f, 3.0f/6.0f, 0.0f/6.0f,
		1.0f/6.0f, 4.0f/6.0f, 1.0f/6.0f, 0.0f/6.0f
};

const float _clrs[][3] = 
{
	{ 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }, // points multiplicity 1
	{ 0.0f, 1.0f, 0.0f }, // points multiplicity 2
	{ 1.0f, 0.0f, 0.0f }  // points multiplicity 3
};




static const Matrix _SplineMatrix(4, 4, SplineInit);


Contour::Contour()
{
	_size = 8;
	_arr = new CtrlPoint[_size];
	
	_arr[0].Set(-0.5f, 0.5f, 0.0f);
	_arr[0].SetMultiplicity(1);
	_arr[1].Set( 0.0f, 0.5f, 0.0f);
	_arr[1].SetMultiplicity(1);
	_arr[2].Set( 0.5f, 0.5f, 0.0f);
	_arr[2].SetMultiplicity(1);
	_arr[3].Set( 0.5f, 0.0f, 0.0f);
	_arr[3].SetMultiplicity(1);
	_arr[4].Set( 0.5f,-0.5f, 0.0f);
	_arr[4].SetMultiplicity(1);
	_arr[5].Set( 0.0f,-0.5f, 0.0f);
	_arr[5].SetMultiplicity(1);
	_arr[6].Set(-0.5f,-0.5f, 0.0f);
	_arr[6].SetMultiplicity(1);
	_arr[7].Set(-0.5f, 0.0f, 0.0f);
	_arr[7].SetMultiplicity(1);
	_items = 8;
	_pts = 8;
	
	_closed = true;

	_type = btRegular;
	_samples = DefaultSamples;

	_steps = 20;
	strcpy(_Name, _DefaultName());
	_version = DefaultVersion;

	INC_COUNTER;
}


Contour::~Contour()
{
	DEC_COUNTER;
	delete []_arr;
}

void Contour::Reset()
{
	if (_size<8)
		_SetSize(8);
	_arr[0].Set(-0.5f, 0.5f, 0.0f);
	_arr[0].SetMultiplicity(1);
	_arr[1].Set( 0.0f, 0.5f, 0.0f);
	_arr[1].SetMultiplicity(1);
	_arr[2].Set( 0.5f, 0.5f, 0.0f);
	_arr[2].SetMultiplicity(1);
	_arr[3].Set( 0.5f, 0.0f, 0.0f);
	_arr[3].SetMultiplicity(1);
	_arr[4].Set( 0.5f,-0.5f, 0.0f);
	_arr[4].SetMultiplicity(1);
	_arr[5].Set( 0.0f,-0.5f, 0.0f);
	_arr[5].SetMultiplicity(1);
	_arr[6].Set(-0.5f,-0.5f, 0.0f);
	_arr[6].SetMultiplicity(1);
	_arr[7].Set(-0.5f, 0.0f, 0.0f);
	_arr[7].SetMultiplicity(1);
	_items = 8;
	_pts = 8;
	
	_closed = true;

	_type = btRegular;
	_samples = DefaultSamples;

	strcpy(_Name, _DefaultName());
}


void Contour::Draw(int drawWhat) const
{
	if (0==_items)
		return;


	if (DrawPoints & drawWhat)
	{
		GLenable smooth(GL_POINT_SMOOTH);
		GLpointSize ps(options.GetPointSize());
		GLpoints p;
		for (int i=0; i<_items; i++)
		{
			glColor3fv(_clrs[_arr[i].GetMultiplicity()]);
			p.Vertex(_arr[i].Vertex());
		}
	}

	if (DrawSegments & drawWhat)
	{
		if (_items>1)
		{
			glColor3fv(options.GetGridColor(Options::eSegments));
			glLineWidth(options.GetSegmentsWidth());
			{
				GLlinestrip ls;
				for (int i=0; i<_items; i++)
					ls.Vertex(_arr[i].Vertex());
				if (_closed)
					ls.Vertex(_arr[0].Vertex());
			}
			glLineWidth(1.0f);
		}
	}

	if (DrawCurve & drawWhat)
	{
		if (_pts>3)
			_DrawCurve();
	}
}


void Contour::DrawInGallery() const
{
	PushPopMatrix ppm;
	WorldPointf center;
	BoundingBox bb(center);
	GetBoundingBox(bb);
	center = bb.Center();
	const float xsz = bb.XSize();
	const float ysz = bb.YSize();
	float ms = (xsz>ysz) ? xsz : ysz;
	if (ms<0.001f)
		ms = 0.001f;
	ms = 1.82f/ms;
	glScalef(ms, ms, ms);

	glTranslatef(-center.X(), -center.Y(), -center.Z());
	Draw(DrawCurve);
}


void Contour::Copy(const EditableObject* pObj)
{
	const Contour* pSrc = dynamic_cast<const Contour*>(pObj);

	if (pSrc->_items>=_size)
	{
		CtrlPoint* aTmp = new CtrlPoint[pSrc->_size];
		_size = pSrc->_size;
		delete []_arr;
		_arr = aTmp;
	}
	_items = pSrc->_items;
	for (int i=0; i<_items; i++)
		_arr[i] = pSrc->_arr[i];

	_closed = pSrc->_closed;
	_steps = pSrc->_steps;
	_pts = pSrc->_pts;

	_type = pSrc->_type;
	_samples = pSrc->_samples;

	if (0 == pSrc->_Name[0])
		strcpy(_Name, _DefaultName());
	else
		strcpy(_Name, pSrc->_Name);
	_version = pSrc->_version;
}


void Contour::_SetSize(int sz)
{
	assert(sz>0);
	CtrlPoint* aNew = new CtrlPoint[sz];
	delete []_arr;
	_arr = aNew;
	_size = sz;
	_items = 0;
	_pts = 0;
}


EditableObject* Contour::Clone() const
{
	std::unique_ptr<Contour> pNew(new Contour);
	pNew->Copy(this);
	return pNew.release();
}

bool Contour::IsNamed(const char* nm) const
{
	if (!IsNamed())
		return false;
	else 
		return (0==strcmp(nm, GetName()));
}


int Contour::GetClosestPoint(WorldPointf p) const
{
	if (0==_items)
		return -1;
	float mindist = Distance(p, _arr[0]);
	int toret = 0;
	for (int i=0; i<_items; i++)
	{
		float dist = Distance(p, _arr[i]);
		if (dist<mindist)
		{
			mindist = dist;
			toret = i;
		}
	}
	return toret;
}


int Contour::AddPoint(WorldPointf wp)
{
	if (_items == _size)
		_Grow();

	switch (_items)
	{
	case 0 :
	case 1 :
		_arr[_items].Set(wp);
		_arr[_items].SetMultiplicity(1);
		_pts++;
		return _items++;
		break;
	default :
		return _InsertSmart(wp);
	}
}


int Contour::_InsertSmart(WorldPointf wp)
{
	assert(_items>1);
	int no = -1;
	float dist = -1.0f;
	{
		WorldLinef line(_arr[0], _arr[1]);
		no = 0;
		dist = line.DistanceTo(wp);
	}
	assert(dist>=0.0f);
	for (int i=1; i<_items-1; i++)
	{
		WorldLinef line(_arr[i], _arr[i+1]);
		float ldist = line.DistanceTo(wp);
		if (ldist<dist)
		{
			no = i;
			dist = ldist;
		}
	}
	if (_closed)
	{
		WorldLinef line(_arr[_items-1], _arr[0]);
		float ldist = line.DistanceTo(wp);
		if (ldist<dist)
		{
			no = _items-1;
			dist = ldist;
		}
	}
	_Insert(no+1, wp);
	return no+1;
}



void Contour::DeletePoint(int ix)
{
	assert(ix>=0);
	assert(ix<_items);
	assert(CanDelete(ix));
	_pts -= _arr[ix].GetMultiplicity();
	for (int i=ix; i<_items-1; i++)
		_arr[i] = _arr[i+1];
	_items--;
}


void Contour::_Insert(int ix, WorldPointf wp)
{
	assert(ix>=0);
	assert(ix<=_items);
	assert(_items<_size);
	for (int i=_items; i>ix; i--)
		_arr[i] = _arr[i-1];
	_arr[ix].Set(wp);
	_arr[ix].SetMultiplicity(1);
	_items++;
	_pts++;
}


void Contour::_Grow()
{
	CtrlPoint* aNew = new CtrlPoint[_size*2];
	_size *= 2;
	for (int i=0; i<_items; i++)
		aNew[i] = _arr[i];
	delete []_arr;
	_arr = aNew;
}

WorldPointf Contour::GetPoint(int i) const
{ 
	assert(i>=0);
	assert(i<_items);
	return _arr[i];
}


void Contour::MovePoint(int i, WorldPointf p)
{
	assert(i>=0);
	assert(i<_items);
	_arr[i].Set(p);
}


void Contour::SetType(ContourType type)
{
	_type = type;
	// btEndPoint requires version 102
	if (type == btEndPoint && GetVersion()<102)
		SetVersion(102);
}


void Contour::SetSamples(int samples)
{
	assert(samples>=MinSamples);
	assert(samples<=MaxSamples);
	_samples = samples;
	if (_samples != DefaultSamples && GetVersion()<103)
		SetVersion(103);
}


void Contour::_DrawCurve() const
{
	assert(_pts>3);
	glColor3fv(options.GetGridColor(Options::eCurve));
	glLineWidth(options.GetCurveWidth());
	if (_type == btRegular)
		_DrawRegularCurve();
	else
		_DrawEndPtCurve();

	glLineWidth(1.0f);
}


void Contour::_DrawRegularCurve() const
{
	int items = 0;
	std::vector<WorldPointf> pa(_pts);
	for (int i=0; i<_items; i++)
	{
		for (int j=0; j<_arr[i].GetMultiplicity(); j++)
			pa[items++] = _arr[i];
	}
	glBegin(_closed ? GL_LINE_LOOP : GL_LINE_STRIP);
	if (_closed)
	{
		const Matrix4x3 m(pa[items-1], pa[0], pa[1], pa[2]);
		_DrawCurve(m);
	}
	for (int i=1; i<items-2; i++)
	{
		const Matrix4x3 m(pa[i-1], pa[i], pa[i+1], pa[i+2]);
		_DrawCurve(m);
	}
	if (_closed)
	{
		{
			const Matrix4x3 m(pa[items-3], pa[items-2], pa[items-1], pa[0]);
			_DrawCurve(m);
		}
		{
			const Matrix4x3 m(pa[items-2], pa[items-1], pa[0], pa[1]);
			_DrawCurve(m);
		}
	}
	glEnd();
}


void Contour::_DrawEndPtCurve() const
{
	int items = 0;
	int pts = _pts;
	if (_closed)
		pts += _arr[0].GetMultiplicity();
	std::vector<WorldPointf> pa(pts);
	for (int i=0; i<_items; ++i)
	{
		for (int j=0; j<_arr[i].GetMultiplicity(); ++j)
			pa[items++] = _arr[i];
	}
	if (_closed)
	{
		for (int j=0; j<_arr[0].GetMultiplicity(); ++j)
			pa[items++] = _arr[0];
	}

	glBegin(_closed ? GL_LINE_LOOP : GL_LINE_STRIP);

	{
		int num = pa.size() * 12;
		for (int i=0; i<num; i++)
		{
			WorldPointf wp = _P(float(i)/num, pa);
			glVertex3fv(wp.Vertex());
		}
		if (_closed)
			glVertex3fv(pa[0].Vertex());
		else
			glVertex3fv(pa.back().Vertex());
	}

	glEnd();
}


WorldPointf Contour::_P(float d, const std::vector<WorldPointf>& arr) 
{
	assert(d>=0.0f);
	assert(d<=1.0f);
	const int n = arr.size()-1;
	const int t = 4;
	float u = d * (n-t+2);
	WorldPointf sum;
	for (int k=0; k<=n; k++)
		sum += arr[k]*_N(k, t, u, n);
	return sum;
}


float Contour::_N(int k, int t, float u, int n) 
{
	if (1==t)
		return _Nk1(k, u, n);
	else
		return _Nkt(k, t, u, n);
}


float Contour::_Nk1(int k, float u, int n) 
{
	if (_Uk(k, n)<=u)
	{
		if (u<_Uk(k+1, n))
			return 1.0f;
	}
	return 0.0f;
}

float Contour::_Nkt(int k, int t, float u, int n) 
{
	float sum = 0.0f;
	int div = _Uk(k+t-1, n) - _Uk(k, n);
	if (0 != div)
		sum = (u - _Uk(k, n))/div * _N(k, t-1, u, n);

	div = _Uk(k+t, n) - _Uk(k+1, n);
	if (0 != div)
		sum += (_Uk(k+t, n)-u)/div * _N(k+1, t-1, u, n);

	return sum;
}


int Contour::_Uk(int j, int n) 
{
	const int t = 4;
	if (j<t)
		return 0;
	if (j>n)
		return n-t+2;
	return j-t+1;
}



void Contour::_DrawCurve(const Matrix4x3& m) const
{
	Matrix t(1, 4); 
	t.Set(0, 3, 1.0f); // Always
	for (int i=0; i<=_steps; i++)
	{
		float param = static_cast<float>(i)/_steps;
		t.Set(0, 2, param);
		param *= param;
		t.Set(0, 1, param);
		t.Set(0, 0, t.Get(0, 2)*param);
		WorldPointf wp = _BSpline(t, m);
		glVertex3f(wp.X(), wp.Y(), wp.Z());
	}
}


WorldPointf Contour::_BSpline(const Matrix& t, const Matrix4x3& m)
{
	Matrix stp(4, 3);
	MulMatrix(_SplineMatrix, m, stp);
	Matrix res(1, 3);
	MulMatrix(t, stp, res);
	WorldPointf wp(res.Get(0, 0), res.Get(0, 1), res.Get(0, 2));
	return wp;
}


bool Contour::CanDelete(int i) const
{
	return (_pts - _arr[i].GetMultiplicity()>=4);
}

bool Contour::CanIncMultiplicity(int i) const
{
	if (3 != _arr[i].GetMultiplicity())
		return true;
	return (_pts >= 6);
}


void Contour::IncMultiplicity(int i)
{
	assert(CanIncMultiplicity(i));
	int m = _arr[i].GetMultiplicity();
	if (m<3)
		_pts++;
	else
		_pts -= 2;
	_arr[i].IncMultiplicity();
}

int Contour::GetPointMultiplicity(int i) const
{
	assert(i>=0);
	assert(i<_items);
	return _arr[i].GetMultiplicity();
}


void Contour::Generate(WriteTextFile& trg) const
{
	switch (_version)
	{
	case 0 :
	case 2 :
		_Generate0000(trg);
		break;
	case 10 :
		_Generate0010(trg);
		break;
	case 101 :
		_Generate0101(trg);
		break;
	case 102 :
		_Generate0102(trg);
		break;
	case 103:
		_Generate0103(trg);
		break;
	default:
		assert(!"Unhandled contour version");
	}
}

void Contour::_Generate0000(WriteTextFile& trg) const
{
	// First count number of knots
	int sum = 0;
	for (int i=0; i<_items; i++)
		sum += _arr[i].GetMultiplicity();

	trg.PrintF("%d ", sum);
	if (_version == 2)
		trg.PrintF("2");
	else
		trg.PrintF("3");
	trg.PrintF(" %s\n", _closed ? "closed" : "open");

	for (int i=0; i<_items; i++)
	{
		for (int j=0; j<_arr[i].GetMultiplicity(); j++)
		{
			if (_version == 2)
				trg.PrintF("%f %f\n", _arr[i].X(), _arr[i].Y());
			else
				trg.PrintF("%f %f %f\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z());
		}
	}
}


void Contour::_Generate0010(WriteTextFile& trg) const
{
	trg.WriteLn("version: 1.4");
	trg.WriteLn("contact: 0 0 0");
	trg.WriteLn("end: 0 0 0");
	trg.WriteLn("heading: 0 1 0");
	trg.WriteLn("up: 0 0 -1");
	trg.WriteLn("size: 1");
	trg.PrintF("points: %d\n", _items);
	trg.WriteLn("range: 0.0 1.0");
	trg.WriteLn("dimension: 4");
	trg.WriteLn("type: bspline");
	for (int i=0; i<_items; i++)
		trg.PrintF("%f %f %f %d\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z(), _arr[i].GetMultiplicity());
}

void Contour::_Generate0101(WriteTextFile& trg) const
{
	trg.PrintF("cver 1 1\n");
	trg.PrintF("name: %s\n", _Name);
	int sum = 0;
	for (int i=0; i<_items; i++)
		sum += _arr[i].GetMultiplicity();
	trg.PrintF("points: %d %d\n", _items, sum);
	trg.PrintF("type: %s\n", _closed ? "closed" : "open");
	for (int i=0; i<_items; i++)
		trg.PrintF("%f %f %f %d\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z(), _arr[i].GetMultiplicity());
}


void Contour::_Generate0102(WriteTextFile& trg) const
{
	trg.PrintF("cver 1 2\n");
	trg.PrintF("name: %s\n", _Name);
	int sum = 0;
	for (int i=0; i<_items; i++)
		sum += _arr[i].GetMultiplicity();
	trg.PrintF("points: %d %d\n", _items, sum);
	trg.PrintF("type: ");
	if (_closed)
		trg.PrintF("c");
	else
		trg.PrintF("o");
	
	if (btRegular==_type)
		trg.PrintF("r");
	else
		trg.PrintF("e");

	trg.WriteEOL();

	for (int i=0; i<_items; i++)
		trg.PrintF("%f %f %f %d\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z(), _arr[i].GetMultiplicity());
}



void Contour::_Generate0103(WriteTextFile& trg) const
{
	trg.PrintF("cver 1 3\n");
	trg.PrintF("name: %s\n", _Name);
	int sum = 0;
	for (int i=0; i<_items; i++)
		sum += _arr[i].GetMultiplicity();
	trg.PrintF("points: %d %d\n", _items, sum);
	trg.PrintF("type: ");
	if (_closed)
		trg.PrintF("c");
	else
		trg.PrintF("o");
	
	if (btRegular==_type)
		trg.PrintF("r");
	else
		trg.PrintF("e");

	trg.WriteEOL();

	trg.PrintF("samples: %d\n", _samples);

	for (int i=0; i<_items; i++)
		trg.PrintF("%f %f %f %d\n", _arr[i].X(), _arr[i].Y(), _arr[i].Z(), _arr[i].GetMultiplicity());
}

void Contour::Import(ReadTextFile& src)
{
	std::string line;
	src.Read(line);
	if (0 == strncmp("cver", line.c_str(), 4))
	{
		int vmaj, vmin;
		if (2 != sscanf(line.c_str(), "cver %d %d", &vmaj, &vmin))
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		_version = 100*vmaj+vmin;
	}
	else if (0==strncmp("version: 1.4", line.c_str(), 12))
		_version = 10;
	else
		_version = 0;
	switch (_version)
	{
	case 0:
		_Import0000(line.c_str(), src);
		break;
	case 10:
		_Import0010(src);
		break;
	case 101:
		_Import0101(src);
		break;
	case 102:
		_Import0102(src);
		break;
	case 103:
		_Import0103(src);
		break;
	default:
		throw Exception(IDERR_CONTOURVERSION, src.Filename());
	}
}


void Contour::_Import0000(const char* firstline, ReadTextFile& src)
{
	int pts, dim;
	char type[32];
	sscanf(firstline, "%d %d %10s", &pts, &dim, type);

	if (2==dim)
		_version = 2;

	_items = 0;
	_pts = 0;
	
	if (!(strcmp("closed", type)))
		_closed = true;
	else
		_closed = false;

	std::string line;
	std::string prevline;
	for (int i=0; i<pts; i++)
	{
		src.Read(line);
		if (0==prevline.compare(line))
		{
			assert(_items>0);
			if (_arr[_items-1].GetMultiplicity()<3)
				_arr[_items-1].IncMultiplicity();
			else
			{
				if (_items == _size)
					_Grow();
				float x, y, z;
				if (3==dim)
					sscanf(line.c_str(), "%f %f %f", &x, &y, &z);
				else
				{
					z = 0.0f;
					sscanf(line.c_str(), "%f %f", &x, &y);
				}
				WorldPointf p(x, y, z);
				_arr[_items].Set(p);
				_arr[_items].SetMultiplicity(1);
				_items++;
			}
		}
		else
		{
			if (_items == _size)
				_Grow();
			float x, y, z;
			if (3==dim)
				sscanf(line.c_str(), "%f %f %f", &x, &y, &z);
			else
			{
				z = 0.0f;
				sscanf(line.c_str(), "%f %f", &x, &y);
			}
			WorldPointf p(x, y, z);
			_arr[_items].Set(p);
			_arr[_items].SetMultiplicity(1);
			_items++;
		}
		_pts++;
		prevline = line;
	}
}


void Contour::_Import0010(ReadTextFile& src)
{
	std::string line;
	// ignore next 5 lines
	src.Read(line);
	src.Read(line);
	src.Read(line);
	src.Read(line);
	src.Read(line);

	src.Read(line);
	int pts;
	int res = sscanf(line.c_str(), "points: %d", &pts);
	if (1 != res)
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	if (pts<0)
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	// ignore next three lines
	src.Read(line);
	src.Read(line);
	src.Read(line);

	while (_size<pts)
		_Grow();
	_items = pts;

	_closed = false;
	_pts = 0;
	for (int i=0; i<_items; i++)
	{
		src.Read(line);
		float x, y, z;
		int m;
		if (4 != sscanf(line.c_str(), "%f %f %f %d", &x, &y, &z, &m))
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		if (m<1 || m>3)
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		_arr[i].Set(x, y, z);
		_arr[i].SetMultiplicity(m);
		_pts += m;
	}
}

void Contour::_Import0101(ReadTextFile& src)
{
	std::string line;
	src.Read(line);
	if (1 != sscanf(line.c_str(), "name: %30s", _Name))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	src.Read(line);
	int items, pts;
	if (2 != sscanf(line.c_str(), "points: %d %d", &items, &pts))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	if (pts<0)
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	while (_size<pts)
		_Grow();
	_items = items;
	src.Read(line);
	char type[16];
	if (1 != sscanf(line.c_str(), "type: %10s", type))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	if (!strcmp("open", type))
		_closed = false;
	else if (!strcmp("closed", type))
		_closed = true;
	else
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	_pts = 0;
	for (int i=0; i<_items; i++)
	{
		src.Read(line);
		float x, y, z;
		int m;
		if (4 != sscanf(line.c_str(), "%f %f %f %d", &x, &y, &z, &m))
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		if (m<1 || m>3)
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		_arr[i].Set(x, y, z);
		_arr[i].SetMultiplicity(m);
		_pts += m;
	}
}


void Contour::_Import0102(ReadTextFile& src)
{
	std::string line;
	src.Read(line);
	if (1 != sscanf(line.c_str(), "name: %30s", _Name))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	src.Read(line);
	int items, pts;
	if (2 != sscanf(line.c_str(), "points: %d %d", &items, &pts))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	if (pts<0)
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	while (_size<pts)
		_Grow();
	_items = items;
	src.Read(line);
	char type[16];
	if (1 != sscanf(line.c_str(), "type: %10s", type))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	if ('o' == type[0])
		_closed = false;
	else if ('c' == type[0])
		_closed = true;
	else
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	if ('r' == type[1])
		_type = btRegular;
	else if ('e' == type[1])
		_type = btEndPoint;
	else
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	_pts = 0;
	for (int i=0; i<_items; i++)
	{
		src.Read(line);
		float x, y, z;
		int m;
		if (4 != sscanf(line.c_str(), "%f %f %f %d", &x, &y, &z, &m))
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		if (m<1 || m>3)
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		_arr[i].Set(x, y, z);
		_arr[i].SetMultiplicity(m);
		_pts += m;
	}
}


void Contour::_Import0103(ReadTextFile& src)
{
	std::string line;
	src.Read(line);
	if (1 != sscanf(line.c_str(), "name: %30s", _Name))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	src.Read(line);
	int items, pts;
	if (2 != sscanf(line.c_str(), "points: %d %d", &items, &pts))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	if (pts<0)
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
	while (_size<pts)
		_Grow();
	_items = items;
	src.Read(line);
	char type[16];
	if (1 != sscanf(line.c_str(), "type: %10s", type))
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	if ('o' == type[0])
		_closed = false;
	else if ('c' == type[0])
		_closed = true;
	else
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	if ('r' == type[1])
		_type = btRegular;
	else if ('e' == type[1])
		_type = btEndPoint;
	else
		throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());

	{
		src.Read(line);
		int samples = 0;
		if (1 != sscanf(line.c_str(), "samples: %d", &samples))
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		if (samples<MinSamples)
			samples = MinSamples;
		else if (samples>MaxSamples)
			samples = MaxSamples;
		_samples = samples;
	}

	_pts = 0;
	for (int i=0; i<_items; i++)
	{
		src.Read(line);
		float x, y, z;
		int m;
		if (4 != sscanf(line.c_str(), "%f %f %f %d", &x, &y, &z, &m))
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		if (m<1 || m>3)
			throw Exception(IDERR_READINGCONTOUR, src.Filename(), src.Line());
		_arr[i].Set(x, y, z);
		_arr[i].SetMultiplicity(m);
		_pts += m;
	}
}


void Contour::GetBoundingBox(BoundingBox& bb) const
{
	if (0 == _items)
	{
		WorldPointf p(1.2f, 1.2f);
		bb.Reset(p);
		p.Set(-1.2f, -1.2f, 0.0);
		bb.Adapt(p);
	}
	else
	{
		bb.Reset(_arr[0]);
		for (int i=1; i<_items; i++)
			bb.Adapt(_arr[i]);
	}
}


DWORD Contour::ClipboardSize() const
{
	DWORD res = 0;
	res += MaxNameLength; // Name
	res += sizeof(bool);  // Opened/closed
	res += sizeof(ContourType);
	res += sizeof(int);   // No. of control points
	res += _items * sizeof(CtrlPoint);
	return res;
}

char* Contour::CopyToClipboard(char* pCur) const
{
	strcpy(pCur, _Name);
	pCur += MaxNameLength;

	ToClipboard<bool>(_closed, pCur);
	ToClipboard<ContourType>(_type, pCur);
	ToClipboard<int>(_items, pCur);

	for (int i=0; i<_items; i++)
	{
		CtrlPoint* pPnt = reinterpret_cast<CtrlPoint*>(pCur);
		*pPnt = _arr[i];
		pCur += sizeof(CtrlPoint);
	}
	return pCur;
}


const char* Contour::LoadFromClipboard(const char* pCur)
{
	strcpy(_Name, pCur);
	pCur += MaxNameLength;

	FromClipboard<bool>(_closed, pCur);
	FromClipboard<ContourType>(_type, pCur);

	int pts;
	{
		FromClipboard<int>(pts, pCur);
		if (pts>_size)
		{
			int newsize = 2;
			while (newsize<pts)
				newsize *= 2;
			_SetSize(newsize);
		}
	}
	_items = 0;
	_pts = 0;

	assert(pts<=_size);

	for (int i=0; i<pts; i++)
	{
		
		const CtrlPoint* pPnt = reinterpret_cast<const CtrlPoint*>(pCur);
		_arr[i] = *pPnt;
		pCur += sizeof(CtrlPoint);
		_items++;
		_pts += pPnt->GetMultiplicity();
	}
	_items = pts;
	return pCur;
}


bool Contour::operator !=(const EditableObject& pObj) const
{
	const Contour* pRght = dynamic_cast<const Contour*>(&pObj);
	if (_pts != pRght->_pts)
		return true;
	if (_items != pRght->_items)
		return true;
	if (_closed != pRght->_closed)
		return true;
	if (_steps != pRght->_steps)
		return true;
	if (strcmp(_Name, pRght->_Name))
		return true;
	if (_type != pRght->_type)
		return true;
	for (int i=0; i<_items; i++)
	{
		if (_arr[i] != pRght->_arr[i])
			return true;
	}
	if (_version != pRght->_version)
		return true;
	return false;
}


void Contour::SetVersion(int ver)
{
	_version = ver;
}


bool Contour::IsNamed() const
{
	return (0 != strcmp(GetName(), _DefaultName()));
}
