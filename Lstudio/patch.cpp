#include <fw.h>
#include <glfw.h>


#include "patchclrinfo.h"
#include "patch.h"
#include "lstudioptns.h"

#include "resource.h"

static const float clrs[][3] =
{
	{ 0.6f, 0.6f, 1.0f },
	{ 1.0f, 1.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.5f, 1.0f, 0.0f },
	{ 1.0f, 1.0f, 1.0f }
};

enum ColorIndex
{
	eMesh = 0,
		eWireframe,
		eInactiveKnot,
		eInactiveShadedKnot,
		eActiveKnot
};

Patch::Patch()
{
	assert(CountOf(clrs) == eActiveKnot+1);
	_pts[0].Set(-1.5f, 1.5f, 0.0f);
	_pts[1].Set(-0.5f, 1.5f, 0.0f);
	_pts[2].Set( 0.5f, 1.5f, 0.0f);
	_pts[3].Set( 1.5f, 1.5f, 0.0f);

	_pts[4].Set(-1.5f, 0.5f, 0.0f);
	_pts[5].Set(-0.5f, 0.5f, 0.0f);
	_pts[6].Set( 0.5f, 0.5f, 0.0f);
	_pts[7].Set( 1.5f, 0.5f, 0.0f);

	_pts[8].Set(-1.5f, -0.5f, 0.0f);
	_pts[9].Set(-0.5f, -0.5f, 0.0f);
	_pts[10].Set( 0.5f, -0.5f, 0.0f);
	_pts[11].Set( 1.5f, -0.5f, 0.0f);

	_pts[12].Set(-1.5f, -1.5f, 0.0f);
	_pts[13].Set(-0.5f, -1.5f, 0.0f);
	_pts[14].Set( 0.5f, -1.5f, 0.0f);
	_pts[15].Set( 1.5f, -1.5f, 0.0f);
	_isUsed = false;
	_YZSymmetric = false;
	_cntLeft = false;
	_cntRight = false;
	_cntUp = false;
	_cntDown = false;
}


void Patch::Reset()
{
	_pts[0].Set(-1.5f, 1.5f, 0.0f);
	_pts[1].Set(-0.5f, 1.5f, 0.0f);
	_pts[2].Set( 0.5f, 1.5f, 0.0f);
	_pts[3].Set( 1.5f, 1.5f, 0.0f);

	_pts[4].Set(-1.5f, 0.5f, 0.0f);
	_pts[5].Set(-0.5f, 0.5f, 0.0f);
	_pts[6].Set( 0.5f, 0.5f, 0.0f);
	_pts[7].Set( 1.5f, 0.5f, 0.0f);

	_pts[8].Set(-1.5f, -0.5f, 0.0f);
	_pts[9].Set(-0.5f, -0.5f, 0.0f);
	_pts[10].Set( 0.5f, -0.5f, 0.0f);
	_pts[11].Set( 1.5f, -0.5f, 0.0f);

	_pts[12].Set(-1.5f, -1.5f, 0.0f);
	_pts[13].Set(-0.5f, -1.5f, 0.0f);
	_pts[14].Set( 0.5f, -1.5f, 0.0f);
	_pts[15].Set( 1.5f, -1.5f, 0.0f);
	_isUsed = false;
	_YZSymmetric = false;
	_cntLeft = false;
	_cntRight = false;
	_cntUp = false;
	_cntDown = false;
}


void Patch::operator=(const Patch& src)
{
	for (int i=0; i<16; i++)
		_pts[i] = src._pts[i];
	_isUsed = src._isUsed;
	_YZSymmetric = src._YZSymmetric;
	_ColorInfo = src._ColorInfo;
}

void Patch::Draw(int drawWhat, int active, unsigned int base) const
{
	enum eDensity
	{
		dnWireframe = 9,
		dnWireframeDense = 18,
		dnWireframeVDense = 36
	};
	if (!_isUsed)
		return;

	int Density = 0;
	if (drawWhat & PatchSpace::DrawWireframe)
		Density = dnWireframe;
	else if (drawWhat & PatchSpace::DrawDenseWireframe)
		Density = dnWireframeDense;
	else if (drawWhat & PatchSpace::DrawVeryDenseWireframe)
		Density = dnWireframeVDense;

	if (Density>0)
	{
		glColor3fv(options.GetGridColor(Options::eCurve));
		glMap2f
			(
			GL_MAP2_VERTEX_3,
			0.0f, 1.0f,
			3, 4, 
			0.0f, 1.0f,
			12, 4, _pts[0].Vertex()
			);
	
		GLenable map(GL_MAP2_VERTEX_3);
		for (int j=0; j<=Density; j++)
		{
			{
				GLlinestrip gll;
				for (int i=0; i<=Density; i++)
					gll.EvalCoord(i/static_cast<float>(Density), j/static_cast<float>(Density));
			}
			{
				GLlinestrip gll;
				for (int i=0; i<=Density; i++)
					gll.EvalCoord(j/static_cast<float>(Density), i/static_cast<float>(Density));
			}
		}
	}

	
	if (drawWhat & PatchSpace::DrawMesh)
	{
		glColor3fv(options.GetGridColor(Options::eSegments));
		{
			GLlinestrip gls;
			gls.Vertex(_pts[0].Vertex());
			gls.Vertex(_pts[4].Vertex());
			gls.Vertex(_pts[5].Vertex());
			gls.Vertex(_pts[9].Vertex());
			gls.Vertex(_pts[8].Vertex());
			gls.Vertex(_pts[12].Vertex());
			gls.Vertex(_pts[13].Vertex());
			gls.Vertex(_pts[9].Vertex());
			gls.Vertex(_pts[10].Vertex());
			gls.Vertex(_pts[14].Vertex());
			gls.Vertex(_pts[15].Vertex());
			gls.Vertex(_pts[11].Vertex());
			gls.Vertex(_pts[10].Vertex());
			gls.Vertex(_pts[6].Vertex());
			gls.Vertex(_pts[7].Vertex());
			gls.Vertex(_pts[3].Vertex());
			gls.Vertex(_pts[2].Vertex());
			gls.Vertex(_pts[6].Vertex());
			gls.Vertex(_pts[5].Vertex());
			gls.Vertex(_pts[1].Vertex());
			gls.Vertex(_pts[0].Vertex());
		}
		{
			GLlines gll;
			gll.Vertex(_pts[1].Vertex());
			gll.Vertex(_pts[2].Vertex());
			gll.Vertex(_pts[4].Vertex());
			gll.Vertex(_pts[8].Vertex());
			gll.Vertex(_pts[7].Vertex());
			gll.Vertex(_pts[11].Vertex());
			gll.Vertex(_pts[13].Vertex());
			gll.Vertex(_pts[14].Vertex());
		}
	}

	if (drawWhat & PatchSpace::DrawShaded)
	{
		glMap2f
			(
			GL_MAP2_VERTEX_3,
			0.0f, 1.0f,
			3, 4, 
			0.0f, 1.0f,
			12, 4, _pts[0].Vertex()
			);
		GLenable map2(GL_MAP2_VERTEX_3);
		GLenable autonormal(GL_AUTO_NORMAL);
		GLenable normalize(GL_NORMALIZE);
		glMapGrid2f(20, 0.0f, 1.0f, 20, 0.0f, 1.0f);
		GLenable lighting(GL_LIGHTING);
		GLenable light0(GL_LIGHT0);
		const float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
		const float position[] = { 0.0f, 0.0f, -100.0f, 1.0f };
		const float mat_diff[] = { 0.1f, 0.6f, 0.1f, 1.0f };
		const float mat_spec[] = { 0.4f, 0.4f, 0.4f, 1.0f };
		const float mat_shin[] = { 1.0f };
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_POSITION, position);

		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diff);
		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_spec);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shin);

		glEvalMesh2(GL_FILL, 0, 20, 0, 20);
	}

	if (drawWhat & PatchSpace::DrawKnots)
	{
		GLenable smooth(GL_POINT_SMOOTH);
		GLpointSize ps(options.GetPointSize());
		GLpoints glp;
		if (drawWhat & PatchSpace::DrawShaded)
			glColor3fv(clrs[eInactiveShadedKnot]);
		else
			glColor3fv(clrs[eInactiveKnot]);
		for (int i=0; i<16; i++)
		{
			if (active==i)
				glColor3fv(clrs[eActiveKnot]);
			glp.Vertex(_pts[i].Vertex());
			if (active==i)
				glColor3fv(clrs[eInactiveKnot]);
		}
	}

	
	if (drawWhat & PatchSpace::DrawKnotNumbers)
	{
		glColor3fv(clrs[eInactiveKnot]);
		glListBase(base);
		static const TCHAR* Nums[] =
		{
			__TEXT("1"),
				__TEXT("2"),
				__TEXT("3"),
				__TEXT("4"),
				__TEXT("5"),
				__TEXT("6"),
				__TEXT("7"),
				__TEXT("8"),
				__TEXT("9"),
				__TEXT("10"),
				__TEXT("11"),
				__TEXT("12"),
				__TEXT("13"),
				__TEXT("14"),
				__TEXT("15"),
				__TEXT("16")
		};
		for (int i=0; i<16; i++)
		{
			if (i==active)
				glColor3fv(clrs[eActiveKnot]);
			glRasterPos3f(_pts[i].X()+0.07f, _pts[i].Y()+0.07f, _pts[i].Z()+0.07f);
			glCallLists((i<9) ? 1 : 2,
#ifdef UNICODE
				GL_SHORT,
#else
				GL_BYTE,
#endif
				Nums[i]);
			if (i==active)
				glColor3fv(clrs[eInactiveKnot]);
		}
	}

}


void Patch::SetX(int i, float x, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	assert(i>=0);
	assert(i<16);
	
	_pts[i].X(x);
	if (_YZSymmetric)
	{
		float nx = _SymmetricX(x, cp, h, up);
		switch (i)
		{
		case 0 :
			_pts[3].X(nx);
			break;
		case 1 :
			_pts[2].X(nx);
			break;
		case 2 :
			_pts[1].X(nx);
			break;
		case 3 :
			_pts[0].X(nx);
			break;
		case 4 :
			_pts[7].X(nx);
			break;
		case 5 :
			_pts[6].X(nx);
			break;
		case 6 :
			_pts[5].X(nx);
			break;
		case 7 :
			_pts[4].X(nx);
			break;
		case 8 :
			_pts[11].X(nx);
			break;
		case 9 :
			_pts[10].X(nx);
			break;
		case 10 :
			_pts[9].X(nx);
			break;
		case 11 :
			_pts[8].X(nx);
			break;
		case 12 :
			_pts[15].X(nx);
			break;
		case 13 :
			_pts[14].X(nx);
			break;
		case 14 :
			_pts[13].X(nx);
			break;
		case 15 :
			_pts[12].X(nx);
			break;
		}
	}
}

void Patch::SetY(int i, float y, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	assert(i>=0);
	assert(i<16);
	
	_pts[i].Y(y);
	if (_YZSymmetric)
	{
		float ny = _SymmetricY(y, cp, h, up);
		switch (i)
		{
		case 0 :
			_pts[3].Y(ny);
			break;
		case 1 :
			_pts[2].Y(ny);
			break;
		case 2 :
			_pts[1].Y(ny);
			break;
		case 3 :
			_pts[0].Y(ny);
			break;
		case 4 :
			_pts[7].Y(ny);
			break;
		case 5 :
			_pts[6].Y(ny);
			break;
		case 6 :
			_pts[5].Y(ny);
			break;
		case 7 :
			_pts[4].Y(ny);
			break;
		case 8 :
			_pts[11].Y(ny);
			break;
		case 9 :
			_pts[10].Y(ny);
			break;
		case 10 :
			_pts[9].Y(ny);
			break;
		case 11 :
			_pts[8].Y(ny);
			break;
		case 12 :
			_pts[15].Y(ny);
			break;
		case 13 :
			_pts[14].Y(ny);
			break;
		case 14 :
			_pts[13].Y(ny);
			break;
		case 15 :
			_pts[12].Y(ny);
			break;
		}
	}
}

void Patch::SetZ(int i, float z, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	assert(i>=0);
	assert(i<16);
	
	_pts[i].Z(z);
	if (_YZSymmetric)
	{
		float nz = _SymmetricZ(z, cp, h, up);
		switch (i)
		{
		case 0 :
			_pts[3].Z(nz);
			break;
		case 1 :
			_pts[2].Z(nz);
			break;
		case 2 :
			_pts[1].Z(nz);
			break;
		case 3 :
			_pts[0].Z(nz);
			break;
		case 4 :
			_pts[7].Z(nz);
			break;
		case 5 :
			_pts[6].Z(nz);
			break;
		case 6 :
			_pts[5].Z(nz);
			break;
		case 7 :
			_pts[4].Z(nz);
			break;
		case 8 :
			_pts[11].Z(nz);
			break;
		case 9 :
			_pts[10].Z(nz);
			break;
		case 10 :
			_pts[9].Z(nz);
			break;
		case 11 :
			_pts[8].Z(nz);
			break;
		case 12 :
			_pts[15].Z(nz);
			break;
		case 13 :
			_pts[14].Z(nz);
			break;
		case 14 :
			_pts[13].Z(nz);
			break;
		case 15 :
			_pts[12].Z(nz);
			break;
		}
	}
}

void Patch::SetPoint(int ix, WorldPointf p)
{
	assert(ix>=0);
	assert(ix<16);
	_pts[ix] = p;
}


void Patch::SetPoint(int ix, WorldPointf p, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	assert(ix>=0);
	assert(ix<16);

	_pts[ix] = p;
	if (_YZSymmetric)
	{	
		p = _Symmetric(p, cp, h, up);
		switch (ix)
		{
		case 0 :
			_pts[3] = p;
			break;
		case 1 :
			_pts[2] = p;
			break;
		case 2 :
			_pts[1] = p;
			break;
		case 3 :
			_pts[0] = p;
			break;
		case 4 :
			_pts[7] = p;
			break;
		case 5 :
			_pts[6] = p;
			break;
		case 6 :
			_pts[5] = p;
			break;
		case 7 :
			_pts[4] = p;
			break;
		case 8 :
			_pts[11] = p;
			break;
		case 9 :
			_pts[10] = p;
			break;
		case 10 :
			_pts[9] = p;
			break;
		case 11 :
			_pts[8] = p;
			break;
		case 12 :
			_pts[15] = p;
			break;
		case 13 :
			_pts[14] = p;
			break;
		case 14 :
			_pts[13] = p;
			break;
		case 15 :
			_pts[12] = p;
			break;
		}
	}
}


void Patch::Write(WriteTextFile& trg) const
{
	_ColorInfo.Write(trg);
	for (int i=0; i<16; i++)
		trg.PrintF("%f %f %f\n", _pts[i].X(), _pts[i].Y(), _pts[i].Z());
}


void Patch::Load(ReadTextFile& src)
{
	_ColorInfo.Load(src);
	std::string line;
	float x, y, z;
	for (int i=0; i<16; i++)
	{
		src.Read(line);
		if (3 != sscanf(line.c_str(), "%f %f %f\n", &x, &y, &z))
			throw Exception(IDERR_LOADINGPATCH, src.Filename(), src.Line());
		_pts[i].X(x);
		_pts[i].Y(y);
		_pts[i].Z(z);
	}
}


WorldPointf Patch::_Symmetric(WorldPointf p, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	WorldPointf l = Product(h, up);
	if (l.Len()<0.001f)
		l.Set(0.0f, 1.0f, 0.0f);
	else
		l.Normalize();
	const float a = l.X();
	const float b = l.Y();
	const float c = l.Z();
	float d = - ( a*cp.X() + b*cp.Y() + c*cp.Z() ) ;
	float dist = 2.0f*(a*p.X() + b*p.Y() + c*p.Z() + d);

	WorldPointf res = p - l*dist;
	return res;
}


float Patch::_SymmetricX(float x, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	WorldPointf p(x, 0.0f, 0.0f);
	WorldPointf res = _Symmetric(p, cp, h, up);
	return res.X();
}


float Patch::_SymmetricY(float y, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	WorldPointf p(0.0f, y, 0.0f);
	WorldPointf res = _Symmetric(p, cp, h, up);
	return res.Y();
}


float Patch::_SymmetricZ(float z, WorldPointf cp, WorldPointf h, WorldPointf up)
{
	WorldPointf p(0.0f, 0.0f, z);
	WorldPointf res = _Symmetric(p, cp, h, up);
	return res.Z();
}




void Patch::ConnectDown(const Patch& p)
{
	if (!_cntDown)
	{
		_pts[12] = p._pts[0];
		_pts[13] = p._pts[1];
		_pts[14] = p._pts[2];
		_pts[15] = p._pts[3];
		for (int i=11; i>=0; i--)
		{
			_pts[i] = _pts[i+4];
			_pts[i].Y(_pts[i].Y()+1.0f);
		}
		_cntDown = true;
	}
}

void Patch::GetBoundingBox(BoundingBox& bb) const
{
	bb.Reset(_pts[0]);
	for (int i=1; i<16; i++)
		bb.Adapt(_pts[i]);
}


DWORD Patch::ClipboardSize() const
{
	DWORD res = 0;
	res += 16*sizeof(WorldPointf); // Control points
	res += _ColorInfo.ClipboardSize();
	return res;
}


char* Patch::CopyToClipboard(char* pCur) const
{
	WorldPointf* aPts = reinterpret_cast<WorldPointf*>(pCur);
	for (int i=0; i<16; i++)
	{
		aPts[i] = _pts[i];
		pCur += sizeof(WorldPointf);
	}
	pCur = _ColorInfo.CopyToClipboard(pCur);
	return pCur;
}


const char* Patch::LoadFromClipboard(const char* pCur)
{
	const WorldPointf* aPts = reinterpret_cast<const WorldPointf*>(pCur);
	for (int i=0; i<16; i++)
	{
		_pts[i] = aPts[i];
		pCur += sizeof(WorldPointf);
	}
	pCur = _ColorInfo.LoadFromClipboard(pCur);
	return pCur;
}



bool Patch::IsEqual(const Patch* pR) const
{
	for (int i=0; i<16; i++)
	{
		if (_pts[i] != pR->_pts[i])
			return false;
	}
	return true;
}
