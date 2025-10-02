#include <memory>
#include <vector>

#include <fw.h>
#include <glfw.h>


#include "patchclrinfo.h"
#include "traditional.h"
#include "lstudioptns.h"

#include "resource.h"

INIT_COUNTER(TraditionalSurface);
INIT_COUNTER(TraditionalPatch);

TraditionalSurface::TraditionalSurface(ReadTextFile& src, int num)
{
	_precisionT = -1;
	_precisionS = -1;
	// Read general info:
	std::string line;

	// Bounding box
	{
		float xmin, xmax, ymin, ymax, zmin, zmax;
		src.Read(line);
		int res = sscanf(line.c_str(), "%f %f %f %f %f %f", &xmin, &xmax, &ymin, &ymax, &zmin, &zmax);
		if (res != 6)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		if (fabsf(xmin-xmax)<0.01f)
		{
			xmin -= 0.005f;
			xmax += 0.005f;
		}
		_bb.SetX(xmin, xmax);

		if (fabsf(ymin-ymax)<0.01f)
		{
			ymin -= 0.005f;
			ymax += 0.005f;
		}
		_bb.SetY(ymin, ymax);

		if (fabsf(zmin-zmax)<0.01f)
		{
			zmin -= 0.005f;
			zmax += 0.005f;
		}
		_bb.SetZ(zmin, zmax);
	}

	src.Read(line);

	// Possibly precision
	if (!(strncmp(line.c_str(), "PRECISION", strlen("PRECISION"))))
	{
		int res = sscanf(line.c_str(), "PRECISION S: %d T: %d", &_precisionT, &_precisionS);
		if (res != 2)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		src.Read(line);
	}

	// Contact point
	{
		float x, y, z;
		int res = sscanf(line.c_str(), "CONTACT POINT  X: %f Y: %f Z: %f", &x, &y, &z);
		if (res != 3)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		_cp.Set(x, y, z);
	}

	// End point
	{
		float x, y, z;
		src.Read(line);
		int res = sscanf(line.c_str(), "END POINT  X: %f Y: %f Z: %f", &x, &y, &z);
		if (res != 3)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		_ep.Set(x, y, z);
	}
	
	// Heading
	{
		float x, y, z;
		src.Read(line);
		int res = sscanf(line.c_str(), "HEADING  X: %f Y: %f Z: %f", &x, &y, &z);
		if (res != 3)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		_heading.Set(x, y, z);
	}

	// Up
	{
		float x, y, z;
		src.Read(line);
		int res = sscanf(line.c_str(), "UP  X: %f Y: %f Z: %f", &x, &y, &z);
		if (res != 3)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		_up.Set(x, y, z);
	}

	// Size
	{
		float sz;
		src.Read(line);
		int res = sscanf(line.c_str(), "SIZE: %f", &sz);
		if (res != 1)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		_size = sz;
	}


	while (!(src.Eof()))
	{
		try
		{
			TraditionalPatch NewPatch(src);
			_aPatches.push_back(NewPatch);
		}
		catch (Exception)
		{}
		if (num == Patches())
			break;
	}

	INC_COUNTER;
}


void TraditionalSurface::Generate(WriteTextFile& trg) const
{
	// Bounding box
	trg.PrintF
		(
		"%.2f %.2f   %.2f %.2f   %.2f %.2f\n", 
		_bb.MinX(),
		_bb.MaxX(),
		_bb.MinY(),
		_bb.MaxY(),
		_bb.MinZ(),
		_bb.MaxZ()
		);

	// Possibly precision
	if (-1 != _precisionT)
		trg.PrintF("PRECISION S: %d T: %d\n", _precisionT, _precisionS);
	else
		trg.WriteLn("PRECISION S: 5 T: 5");

	trg.PrintF("CONTACT POINT  X: %.2f Y: %.2f Z: %.2f\n", _cp.X(), _cp.Y(), _cp.Z());
	trg.PrintF("END POINT  X: %.2f Y: %.2f Z: %.2f\n", _ep.X(), _ep.Y(), _ep.Z());
	trg.PrintF("HEADING  X: %.2f Y: %.2f Z: %.2f\n", _heading.X(), _heading.Y(), _heading.Z());
	trg.PrintF("UP  X: %.2f Y: %.2f Z: %.2f\n", _up.X(), _up.Y(), _up.Z());
	trg.PrintF("SIZE: %f\n", _size);

	for (cit it=_aPatches.begin(); it != _aPatches.end(); ++it)
		it->Generate(trg);
}


void TraditionalSurface::Draw() const
{
	GLOnOff map(GL_MAP2_VERTEX_3);
	glMapGrid2f(20, 0.0f, 1.0f, 20, 0.0f, 1.0f);
	glColor3fv(options.GetGridColor(Options::eCurve));
	for (cit it=_aPatches.begin(); it != _aPatches.end(); ++it)
		it->Draw();
}

DWORD TraditionalSurface::ClipboardSize() const
{
	DWORD res = 0;
	res += sizeof(float) * 6; // BoundingBox
	res += sizeof(int) * 2; // precision
	res += sizeof(WorldPointf); // _cp
	res += sizeof(WorldPointf); // _ep
	res += sizeof(WorldPointf); // _heading
	res += sizeof(WorldPointf); // _up
	res += sizeof(float);     // _size

	for (cit it = _aPatches.begin(); it != _aPatches.end(); ++it)
		res += it->ClipboardSize();

	res += sizeof(char) * 4; // terminating 0xFF * 4
	return res;
}


char* TraditionalSurface::CopyToClipboard(char* pCur) const
{
	float xmin = _bb.MinX();
	float xmax = _bb.MaxX();
	float ymin = _bb.MinY();
	float ymax = _bb.MaxY();
	float zmin = _bb.MinZ();
	float zmax = _bb.MaxZ();

	ToClipboard<float>(xmin, pCur);
	ToClipboard<float>(xmax, pCur);
	ToClipboard<float>(ymin, pCur);
	ToClipboard<float>(ymax, pCur);
	ToClipboard<float>(zmin, pCur);
	ToClipboard<float>(zmax, pCur);

	ToClipboard<int>(_precisionT, pCur);
	ToClipboard<int>(_precisionS, pCur);

	ToClipboard<WorldPointf>(_cp, pCur);
	ToClipboard<WorldPointf>(_ep, pCur);
	ToClipboard<WorldPointf>(_heading, pCur);
	ToClipboard<WorldPointf>(_up, pCur);
	ToClipboard<float>(_size, pCur);

	for (cit it = _aPatches.begin(); it != _aPatches.end(); ++it)
		pCur = it->CopyToClipboard(pCur);

	{
		BYTE c = BYTE('\xFF');
		ToClipboard<BYTE>(c, pCur);
		ToClipboard<BYTE>(c, pCur);
		ToClipboard<BYTE>(c, pCur);
		ToClipboard<BYTE>(c, pCur);
	}

	return pCur;
}

TraditionalSurface::TraditionalSurface(const char** ppCur)
{
	_precisionT = -1;
	_precisionS = -1;
	float xmin, xmax, ymin, ymax, zmin, zmax;
	const char* pCur = *ppCur;

	FromClipboard<float>(xmin, pCur);
	FromClipboard<float>(xmax, pCur);
	FromClipboard<float>(ymin, pCur);
	FromClipboard<float>(ymax, pCur);
	FromClipboard<float>(zmin, pCur);
	FromClipboard<float>(zmax, pCur);

	_bb.SetX(xmin, xmax);
	_bb.SetY(ymin, ymax);
	_bb.SetZ(zmin, zmax);

	FromClipboard<int>(_precisionT, pCur);
	FromClipboard<int>(_precisionS, pCur);

	FromClipboard<WorldPointf>(_cp, pCur);
	FromClipboard<WorldPointf>(_ep, pCur);
	FromClipboard<WorldPointf>(_heading, pCur);
	FromClipboard<WorldPointf>(_up, pCur);
	FromClipboard<float>(_size, pCur);

	BYTE c[4];
	memcpy(c, pCur, sizeof(BYTE)*4);

	while ((c[0] != BYTE('\xFF')) || (c[0] != BYTE('\xFF')) || (c[0] != BYTE('\xFF')) || (c[0] != BYTE('\xFF')))
	{
		TraditionalPatch NewPatch(&pCur);
		_aPatches.push_back(NewPatch);
		memcpy(c, pCur, sizeof(BYTE)*4);
	}
	pCur += sizeof(BYTE) * 4;

	*ppCur = pCur;
	INC_COUNTER;
}

TraditionalSurface::~TraditionalSurface()
{
	DEC_COUNTER;
}



TraditionalPatch::TraditionalPatch(ReadTextFile& src)
{
	_Name[0] = 0;
	_ANeighbours[0] = 0;
	_LNeighbours[0] = 0;
	_BNeighbours[0] = 0;

	std::string line;
	// Name
	src.Read(line);
	strncpy(_Name, line.c_str(), MaxNameLength);
	_Name[MaxNameLength-1] = 0;
	
	// Colors info
	src.Read(line);
	if (0!=strncmp(line.c_str(), "TOP COLOR:", 10))
		throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
	else
	{
		int tc, bc;
		float td, bd;
		int res = sscanf
			(
			line.c_str(), 
			"TOP COLOR: %d DIFFUSE: %f BOTTOM COLOR: %d DIFFUSE: %f", 
			&tc, &td, &bc, &bd
			);
		if (res != 4)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());
		_ColorInfo.SetTopColor(tc);
		_ColorInfo.SetBottomColor(bc);
		_ColorInfo.SetTopDiffuse(td);
		_ColorInfo.SetBottomDiffuse(bd);
	}

	// Neighbours
	src.Read(line);
	strncpy(_ANeighbours, line.c_str(), MaxNeighboursLength);
	src.Read(line);
	strncpy(_LNeighbours, line.c_str(), MaxNeighboursLength);
	src.Read(line);
	strncpy(_BNeighbours, line.c_str(), MaxNeighboursLength);

	int pt = 0;
	for (int i=0; i<4; i++)
	{
		src.Read(line);
		float x0, y0, z0, x1, y1, z1, x2, y2, z2, x3, y3, z3;
		int res = sscanf
			(
			line.c_str(), 
			"%f %f %f %f %f %f %f %f %f %f %f %f", 
			&x0, &y0, &z0, 
			&x1, &y1, &z1, 
			&x2, &y2, &z2, 
			&x3, &y3, &z3
			);
		if (res != 12)
			throw Exception(IDERR_READINGSURFACE, src.Filename(), src.Line());

		_pts[pt].Set(x0, y0, z0);
		pt++;
		_pts[pt].Set(x1, y1, z1);
		pt++;
		_pts[pt].Set(x2, y2, z2);
		pt++;
		_pts[pt].Set(x3, y3, z3);
		pt++;
	}
	INC_COUNTER;
}


TraditionalPatch::~TraditionalPatch()
{
	DEC_COUNTER;
}


void TraditionalPatch::Draw() const
{
	glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, reinterpret_cast<const float*>(_pts));
	for (int j=0; j<=8; ++j)
	{
		{
			glBegin(GL_LINE_STRIP);
			for (int i=0; i<=30; ++i)
				glEvalCoord2f(i/30.0f, j/8.0f);
			glEnd();
		}
		{
			glBegin(GL_LINE_STRIP);
			for (int i=0; i<=30; ++i)
				glEvalCoord2f(j/8.0f, i/30.0f);
			glEnd();
		}
	}
	/*
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
	*/
}

void TraditionalPatch::Generate(WriteTextFile& trg) const
{
	// Name
	trg.WriteLn(_Name);
	trg.PrintF
		(
		"TOP COLOR: %d DIFFUSE: %.2f BOTTOM COLOR: %d DIFFUSE: %.2f\n", 
		_ColorInfo.GetTopColor(),
		_ColorInfo.GetTopDiffuse(),
		_ColorInfo.GetBottomColor(),
		_ColorInfo.GetBottomDiffuse()
		);
	trg.WriteLn(_ANeighbours);
	trg.WriteLn(_LNeighbours);
	trg.WriteLn(_BNeighbours);

	for (int i=0; i<16; i++)
	{
		trg.PrintF("%.2f %.2f %.2f  ", _pts[i].X(), _pts[i].Y(), _pts[i].Z());
		if (3 == i%4)
			trg.WriteEOL();
	}

}


DWORD TraditionalPatch::ClipboardSize() const
{
	DWORD res = 0;
	res += MaxNameLength;
	res += _ColorInfo.ClipboardSize();
	res += MaxNeighboursLength;
	res += MaxNeighboursLength;
	res += MaxNeighboursLength;

	res += 16 * sizeof(WorldPointf);
	return res;
}


char* TraditionalPatch::CopyToClipboard(char* pCur) const
{
	strcpy(pCur, _Name);
	pCur += MaxNameLength;

	pCur = _ColorInfo.CopyToClipboard(pCur);

	strcpy(pCur, _ANeighbours);
	pCur += MaxNeighboursLength;

	strcpy(pCur, _LNeighbours);
	pCur += MaxNeighboursLength;

	strcpy(pCur, _BNeighbours);
	pCur += MaxNeighboursLength;

	for (int i=0; i<16; i++)
	{
		WorldPointf wp = _pts[i];
		ToClipboard<WorldPointf>(wp, pCur);
	}

	return pCur;
}


TraditionalPatch::TraditionalPatch(const char** ppCur)
{
	_Name[0] = 0;
	_ANeighbours[0] = 0;
	_LNeighbours[0] = 0;
	_BNeighbours[0] = 0;

	const char* pCur = *ppCur;
	strcpy(_Name, pCur);
	pCur += MaxNameLength;

	pCur = _ColorInfo.LoadFromClipboard(pCur);

	strcpy(_ANeighbours, pCur);
	pCur += MaxNeighboursLength;

	strcpy(_LNeighbours, pCur);
	pCur += MaxNeighboursLength;

	strcpy(_BNeighbours, pCur);
	pCur += MaxNeighboursLength;

	for (int i=0; i<16; i++)
	{
		WorldPointf wp;
		FromClipboard<WorldPointf>(wp, pCur);
		_pts[i] = wp;
	}

	*ppCur = pCur;

	INC_COUNTER;
}

