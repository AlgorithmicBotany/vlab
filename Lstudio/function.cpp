#include <vector>
#include <memory>

#include <fw.h>
#include <glfw.h>

#include "objfgvobject.h"
#include "funcpts.h"
#include "function.h"
#include "lstudioptns.h"

#include "resource.h"


INIT_COUNTER(Function);

Function::Function()
{
	WorldPointf wp(0.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	wp.Set(1.0f/3.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	wp.Set(2.0f/3.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	wp.Set(1.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	strcpy(_name, "unnamed");
	_Flipped = false;
	_version = 101;
	_samples = 100;

	INC_COUNTER;
}

Function::~Function()
{
	DEC_COUNTER;
}


void Function::DrawInGallery() const
{
	PushPopMatrix ppm;
	WorldPointf center;
	BoundingBox bb(center);
	GetBoundingBox(bb);
	center.Set(0.0f, 0.0f, 0.0f);
	bb.Adapt(center);
	center = bb.Center();
	const float xsz = bb.XSize();
	const float ysz = bb.YSize();
	float ms = (xsz>ysz) ? xsz : ysz;
	if (ms<0.001f)
		ms = 0.001f;
	ms = 1.82f/ms;
	glScalef(ms, ms, ms);

	glTranslatef(-center.X(), -center.Y(), -center.Z());
	{
		glColor3fv(options.GetGridColor(Options::eGrid));
		GLlines gll;
		if (_Flipped)
		{
			gll.Vertex(0.0f, 0.0f);
			gll.Vertex(0.0f, 1.0f);
			float minx = bb.minX();
			float maxx = bb.maxX();
			float xrng = maxx - minx;
			if (xrng < 0.1f)
				xrng = 0.1f;
			xrng *= 0.1f;
			maxx += xrng;
			minx -= xrng;
			gll.Vertex(minx, 0.0f);
			gll.Vertex(maxx, 0.0f);
		}
		else
		{
			gll.Vertex(0.0f, 0.0f);
			gll.Vertex(1.0f, 0.0f);
			float miny = bb.minY();
			float maxy = bb.maxY();
			float yrng = maxy - miny;
			if (yrng < 0.1f)
				yrng = 0.1f;
			yrng *= 0.1f;
			maxy += yrng;
			miny -= yrng;
			gll.Vertex(0.0f, miny);
			gll.Vertex(0.0f, maxy);
		}
	}
	Draw(FunctionSpace::DrawCurve);
}


WorldPointf Function::GetPoint(size_t i) const
{
	assert(i<_arr.size());
	return _arr[i];
}

bool Function::IsNamed(const char* nm) const
{
	if (0==strcmp("unnamed", GetName()))
		return false;
	else
		return (0==strcmp(nm, GetName()));
}


int Function::GetClosestPoint(WorldPointf ref) const
{
	float mindist = XYDistance(ref, _arr[0]);
	int toret = 0;
	for (size_t i=1; i<_arr.size(); ++i)
	{
		float newdist = XYDistance(ref, _arr[i]);
		if (newdist<mindist)
		{
			mindist = newdist;
			toret = i;
		}
	}
	return toret;
}


void Function::Reset()
{
	_arr.clear();
	WorldPointf wp(0.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	wp.Set(1.0f/3.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	wp.Set(2.0f/3.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	wp.Set(1.0f, 0.0f, 0.0f);
	_arr.push_back(wp);
	_Flipped = false;
	_version = 101;
	_samples = 100;
}



void Function::Draw(int drawWhat) const
{
	if (drawWhat & FunctionSpace::DrawSegments)
		_DrawSegments();
	if (drawWhat & FunctionSpace::DrawCurve)
		_DrawCurve();
	if (drawWhat & FunctionSpace::DrawPoints)
		_DrawPoints();
}



void Function::_DrawPoints() const
{
	glEnable(GL_POINT_SMOOTH);
	glPointSize(options.GetPointSize());
	glColor3fv(options.GetGridColor(Options::ePoints));
	{
		GLpoints glp;
		for (size_t i=0; i<_arr.size(); ++i)
			glp.Vertex(_arr[i].Vertex());
	}
	glPointSize(1.0f);
	glDisable(GL_POINT_SMOOTH);
}


void Function::_DrawSegments() const
{
	glColor3fv(options.GetGridColor(Options::eSegments));
	glLineWidth(options.GetSegmentsWidth());
	{
		GLlinestrip gll;
		for (size_t i=0; i<_arr.size(); ++i)
			gll.Vertex(_arr[i].Vertex());
	}
	glLineWidth(1.0f);
}


int Function::AddPoint(WorldPointf wp)
{
	assert(wp.X()>=0.0f);
	assert(wp.X()<=1.0f);

	size_t toinsert = 0;
	while (toinsert<_arr.size())
	{
		if (_Flipped)
		{
			if (wp.Y()<_arr[toinsert].Y())
				break;
			else
				++toinsert;
		}
		else
		{
			if (wp.X()<_arr[toinsert].X())
				break;
			else
				++toinsert;
		}
	}
	_arr.Insert(toinsert, wp);
	return toinsert;
}

void Function::DeletePoint(int i)
{
	assert(CanDelete(i));
	_arr.Delete(i);
}


bool Function::CanDelete(int i) const
{
	if (i == 0)
		return false;
	else if (i == static_cast<int>(_arr.size()-1))
		return false;
	return _arr.CanDelete(i);
}


void Function::MovePoint(int i, WorldPointf wp)
{
	if (_Flipped)
	{
		if (0 == i)
			wp.Y(0.0f);
		else if (static_cast<int>(_arr.size()-1) == i)
			wp.Y(1.0f);
		else
		{
			float ly = _arr[i-1].Y();
			if (wp.Y()<ly)
				wp.Y(ly);
			else
			{
				float ry = _arr[i+1].Y();
				if (wp.Y()>ry)
					wp.Y(ry);
			}
		}
	}
	else
	{
		if (0 == i)
			wp.X(0.0f);
		else if (static_cast<int>(_arr.size()-1) == i)
			wp.X(1.0f);
		else
		{
			float lx = _arr[i-1].X();
			if (wp.X()<lx)
				wp.X(lx);
			else
			{
				float rx = _arr[i+1].X();
				if (wp.X()>rx)
					wp.X(rx);
			}
		}
	}
	_arr.SetPoint(i, wp);
}


void Function::Generate(WriteTextFile& trg) const
{
	switch (_version)
	{
	case 0 : 
		_Generate0000(trg);
		break;
	case 101 :
		_Generate0101(trg);
		break;
	}
}

void Function::_Generate0000(WriteTextFile& trg) const
{
	trg.PrintF("range: %f %f\n", 0.0, 1.0);

	trg.PrintF("points: %d\n", _arr.size());
	for (size_t i=0; i<_arr.size(); ++i)
	{
		if (_Flipped)
			trg.PrintF("%f %f\n", _arr[i].Y(), _arr[i].X());
		else
			trg.PrintF("%f %f\n", _arr[i].X(), _arr[i].Y());
	}
}


void Function::_Generate0101(WriteTextFile& trg) const
{
	trg.PrintF("fver 1 1\n");
	trg.PrintF("name: %s\n", GetName());
	trg.PrintF("samples: %d\n", _samples);
	trg.PrintF("flip: %s\n", _Flipped ? "on" : "off");
	trg.PrintF("points: %d\n", _arr.size());
	for (size_t i=0; i<_arr.size(); ++i)
	{
		if (_Flipped)
			trg.PrintF("%f %f\n", _arr[i].Y(), _arr[i].X());
		else
			trg.PrintF("%f %f\n", _arr[i].X(), _arr[i].Y());
	}
}


void Function::Import(ReadTextFile& src)
{
	_Flipped = false;
	std::string line;
	src.Read(line);
	if (0 == strncmp("fver", line.c_str(), 4))
	{
		int vmaj, vmin;
		if (2 != sscanf(line.c_str(), "fver %d %d", &vmaj, &vmin))
			throw Exception(IDERR_READINGFUNCTION, src.Filename());
		_version = 100*vmaj+vmin;
	}
	else
		_version = 0;
	switch (_version)
	{
	case 0:
		_Import0000(src);
		break;
	case 101 :
		_Import0101(src);
		break;
	default:
		throw Exception(IDERR_READINGFUNCTION);
	}
}

void Function::_Import0000(ReadTextFile& src)
{
	std::string line;
	src.Read(line);
	int pts;
	if (1 != sscanf(line.c_str(), "points: %d", &pts))
		throw Exception(IDERR_READINGFUNCTION, src.Filename());
	if (pts<4)
		throw Exception(IDERR_READINGFUNCTION, src.Filename());
	_arr.clear();
	for (int i=0; i<pts; i++)
	{
		float x, y;
		src.Read(line);
		if (2 != sscanf(line.c_str(), "%f %f", &x, &y))
			throw Exception(IDERR_READINGFUNCTION, src.Filename());
		WorldPointf wp(x, y, 0.0f);
		_arr.push_back(wp);
	}
}


void Function::_Import0101(ReadTextFile& src)
{
	std::string line;
	src.Read(line);
	if (1 != sscanf(line.c_str(), "name: %30s", _name))
		throw Exception(IDERR_READINGFUNCTION, src.Filename());
	src.Read(line);
	if (1 != sscanf(line.c_str(), "samples: %d", &_samples))
		throw Exception(IDERR_READINGFUNCTION, src.Filename());
	src.Read(line);
	{
		char flip[5];
		if (1 != sscanf(line.c_str(), "flip: %4s\n", flip))
			throw Exception(IDERR_READINGFUNCTION, src.Filename());
		if (0==strcmp("on", flip))
			_Flipped = true;
		else
			_Flipped = false;
	}
	src.Read(line);
	int pts;
	if (1 != sscanf(line.c_str(), "points: %d", &pts))
		throw Exception(IDERR_READINGFUNCTION, src.Filename());
	if (pts<4)
		throw Exception(IDERR_READINGFUNCTION, src.Filename());
	_arr.clear();
	for (int i=0; i<pts; i++)
	{
		float x, y;
		src.Read(line);
		if (2 != sscanf(line.c_str(), "%f %f", &x, &y))
			throw Exception(IDERR_READINGFUNCTION, src.Filename());
		WorldPointf wp(x, y, 0.0);
		_arr.push_back(wp);
	}
	if (_Flipped)
	{
		_Flipped = false;
		Flip();
	}
}


void Function::_DrawCurve() const
{
	glColor3fv(options.GetGridColor(Options::eCurve));
	glLineWidth(options.GetCurveWidth());
	{
		GLlinestrip gll;
		int num = _arr.size() * 12;
		for (int i=0; i<num; i++)
		{
			WorldPointf wp = _P(float(i)/num);
			gll.Vertex(wp.Vertex());
		}
		gll.Vertex(_arr.LastPoint().Vertex());
	}
	glLineWidth(1.0f);
}


void Function::GetBoundingBox(BoundingBox& bb) const
{
	bb.Reset(_arr[0]);
	for (size_t i=1; i<_arr.size(); ++i)
		bb.Adapt(_arr[i]);
}



EditableObject* Function::Clone() const
{
	std::unique_ptr<Function> pNew(new Function);
	pNew->Copy(this);
	return pNew.release();
}

void Function::Copy(const EditableObject* pObj)
{
	const Function* pSrc = dynamic_cast<const Function*>(pObj);
	_arr = pSrc->_arr;
	strcpy(_name, pSrc->_name);
	_Flipped = pSrc->_Flipped;
	_samples = pSrc->_samples;
	_version = pSrc->_version;
}


void Function::Flip()
{
	_Flipped = !_Flipped;
	for (size_t i=0; i<_arr.size(); ++i)
	{
		WorldPointf wp = _arr[i];
		float tmp = wp.X();
		wp.X(wp.Y());
		wp.Y(tmp);
		_arr.SetPoint(i, wp);
	}
}


WorldPointf Function::_P(float d) const
{
	assert(d>=0.0f);
	assert(d<=1.0f);
	const int n = _arr.size()-1;
	const int t = 4;
	float u = d * (n-t+2);
	WorldPointf sum;
	for (int k=0; k<=n; k++)
		sum += _arr[k]*_N(k, t, u);
	return sum;
}


float Function::_N(int k, int t, float u) const
{
	if (1==t)
		return _Nk1(k, u);
	else
		return _Nkt(k, t, u);
}


float Function::_Nk1(int k, float u) const
{
	if (_Uk(k)<=u)
	{
		if (u<_Uk(k+1))
			return 1.0;
	}
	return 0.0;
}

float Function::_Nkt(int k, int t, float u) const
{
	float sum = 0.0f;
	int div = _Uk(k+t-1) - _Uk(k);
	if (0 != div)
		sum = (u - _Uk(k))/div * _N(k, t-1, u);

	div = _Uk(k+t) - _Uk(k+1);
	if (0 != div)
		sum += (_Uk(k+t)-u)/div * _N(k+1, t-1, u);

	return sum;
}


int Function::_Uk(int j) const
{
	const int n = _arr.size()-1;
	const int t = 4;
	if (j<t)
		return 0;
	if (j>n)
		return n-t+2;
	return j-t+1;
}



DWORD Function::ClipboardSize() const
{
	DWORD res = 0;
	res += MaxNameLength;
	res += sizeof(int); // No. of points
	res += sizeof(WorldPointf) * _arr.size();
	res += sizeof(bool); // Flipped
	res += sizeof(int); // samples
	res += sizeof(int); // version
	return res;
}


char* Function::CopyToClipboard(char* pCur) const
{
	strcpy(pCur, _name);
	pCur += MaxNameLength;

	{
		size_t pts = _arr.size();
		ToClipboard<size_t>(pts, pCur);
	}

	{
		for (size_t i=0; i<_arr.size(); ++i)
		{
			WorldPointf* pWp = reinterpret_cast<WorldPointf*>(pCur);
			*pWp = _arr[i];
			pCur += sizeof(WorldPointf);
		}
	}

	{
		bool* pFlipped = reinterpret_cast<bool*>(pCur);
		*pFlipped = _Flipped;
	}
	pCur += sizeof(bool);
	{
		int* pSamples = reinterpret_cast<int*>(pCur);
		*pSamples = _samples;
	}
	pCur += sizeof(int);
	{
		int* pVersion = reinterpret_cast<int*>(pCur);
		*pVersion = _version;
	}
	pCur += sizeof(int);

	return pCur;
}


const char* Function::LoadFromClipboard(const char* pCur)
{
	strcpy(_name, pCur);
	pCur += MaxNameLength;

	size_t pts;
	FromClipboard<size_t>(pts, pCur);

	while (pts>_arr.size())
	{
		WorldPointf wp;
		_arr.Insert(1, wp);
	}
	while (pts<_arr.size())
	{
		_arr.Delete(1);
	}

	for (size_t i=0; i<pts; ++i)
	{
		const WorldPointf* pWp = reinterpret_cast<const WorldPointf*>(pCur);
		_arr.SetPoint(i, *pWp);
		pCur += sizeof(WorldPointf);
	}

	{
		bool flipped;
		FromClipboard<bool>(flipped, pCur);
		_Flipped = flipped;
	}

	{ 
		int Samples;
		FromClipboard<int>(Samples, pCur);
		_samples = Samples;
	}
	{
		int Version;
		FromClipboard<int>(Version, pCur);
		_version = Version;
	}


	return pCur;
}


void Function::SetVersion(int ver)
{
	_version = ver;
}


bool Function::operator !=(const EditableObject& pObj) const
{
	const Function* pRght = dynamic_cast<const Function*>(&pObj);
	if (_arr != pRght->_arr)
		return true;
	if (_Flipped != pRght->_Flipped)
		return true;
	if (_samples != pRght->_samples)
		return true;
	if (strcmp(_name, pRght->_name))
		return true;
	if (_version != pRght->_version)
		return true;
	return false;
}
