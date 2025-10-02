#include <cmath>
#include <fw.h>

#include "panelprms.h"
#include "ped.h"
#include "edlin.h"

#include "resource.h"



void Ped::Action(const char* act)
{
	static char bf[16];
	if (0==act[0])
		return;
	if (1 != sscanf(act, "%14s", bf))
		throw Exception(IDERR_PEDCOMMAND, act);
	if (!(strcmp("d", bf)))
		_DoD(act);
	else if (!(strcmp("n", bf)))
		_DoN(act);
	else if (!(strcmp("o", bf)))
		_DoO(act);
	else
		_DoNum(act);
}



void Ped::_DoD(const char* act)
{
	static char idnt[64], idnt2[64];
	static char ln[PanelParameters::eMaxLineLength+1];
	BufFile buffile;

	TmpFile tbf
		(
		buffile.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0,
		CREATE_ALWAYS
		);

	int value; float scale;

	if (3 != sscanf(act, "d %63s %d %f", idnt, &value, &scale))
		throw Exception(IDERR_PEDCOMMAND, act);

	_OpenSrc();

	for (;;)
	{
		_ReadLine(ln, PanelParameters::eMaxLineLength);
		const char* t = ln;
		while (isspace(*t))
			++t;
		if (1 == sscanf(t, "#define %63s", idnt2))
		{
			if (!(strcmp(idnt, idnt2)))
				break;
		}
		tbf.Write(ln, PanelParameters::eMaxLineLength+1);
	}

	static char rmnd[PanelParameters::eMaxLineLength];
	{
		const char* rm = ln;
		// skip leading blanks
		while (isspace(*rm) && (0 != *rm))
			++rm;
		// skip #define
		while (!isspace(*rm) && (0 != *rm))
			++rm;
		// skip blanks
		while (isspace(*rm) && (0 != *rm))
			++rm;
		// skip identifier
		while (!isspace(*rm) && (0 != *rm))
			++rm;
		// skip blanks
		while (isspace(*rm) && (0 != *rm))
			++rm;
		// skip value
		while (!isspace(*rm) && (0 != *rm))
			++rm;

		strncpy(rmnd, rm, PanelParameters::eMaxLineLength-1);
	}

	if (fabs(scale - 1.0f)<0.0001)
		sprintf(ln, "#define %s %d", idnt, int(value/scale));
	else
		sprintf(ln, "#define %s %f", idnt, value/scale);

	strcat(ln, rmnd);

	tbf.Write(ln, PanelParameters::eMaxLineLength+1);

	_CopyRest(tbf);
	_Commit(tbf);
}


void Ped::_DoN(const char* act)
{
	static char ln[PanelParameters::eMaxLineLength+1];
	BufFile buffile;

	TmpFile tbf
		(
		buffile.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0,
		CREATE_ALWAYS
		);

	int line, field, scale, value;
	if (5 != sscanf(act, "%s %d %d %d %d", ln, &line, &field, &scale, &value))
		throw Exception(IDERR_PEDCOMMAND, act);

	_OpenSrc();
	line--;


	float v = static_cast<float>(value);

	if (abs(scale)>8)
		throw Exception(IDWRN_PEDNSUSPSCALE, act);

	int origsc = scale;
	if (scale>0)
	{
		while (scale>0)
		{
			v *= 10.0f;
			scale--;
		}
	}
	else
	{
		while (scale<0)
		{
			v /= 10.0f;
			scale++;
		}
	}

	for (int i=0; i<line; i++)
	{
		_ReadLine(ln, PanelParameters::eMaxLineLength);
		tbf.Write(ln, PanelParameters::eMaxLineLength+1);
	}

	_ReadLine(ln, PanelParameters::eMaxLineLength);

	{
		static char valstr[64];
		if (0==origsc)
			sprintf(valstr, "%d", value);
		else
			sprintf(valstr, "%.4f", v);
		EdLin resln(ln);
		int ix = resln.Find(0, ':');
		if (-1 == ix)
			throw Exception(IDERR_PEDNFINDFIELD, act);
		switch (field)
		{
		case 1 :
			break;
		case 2 :
			ix = resln.Find(ix, ',');
			if (-1 == ix)
				throw Exception(IDERR_PEDNFINDFIELD, act);
			break;
		case 3 :
			ix = resln.Find(ix, ',');
			if (-1 == ix)
				throw Exception(IDERR_PEDNFINDFIELD, act);
			ix = resln.Find(ix+1, ',');
			if (-1 == ix)
				throw Exception(IDERR_PEDNFINDFIELD, act);
		default :
			throw Exception(IDERR_PEDNINVALIDFIELD, act);
		}

		ix = resln.SkipBlanks(ix+1);
		if (-1 == ix)
			throw Exception(IDERR_PEDNFINDFIELD, act);
		resln.DeleteChars(ix, "0123456789-.");
		resln.Insert(ix, valstr);

		tbf.Write(resln, PanelParameters::eMaxLineLength+1);
	}
	_CopyRest(tbf);
	_Commit(tbf);
}

void Ped::_DoO(const char* act)
{
	BufFile buffile;

	TmpFile tbf
		(
		buffile.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0,
		CREATE_ALWAYS
		);

	static char line[PanelParameters::eMaxLineLength+1];

	int lineno, val;
	if (3 != sscanf(act, "%s %d %d", line, &lineno, &val))
		throw Exception(IDERR_PEDCOMMAND, act);

	_OpenSrc();
	lineno--;

	for (int i=0; i<lineno; i++)
	{
		_ReadLine(line, PanelParameters::eMaxLineLength);
		tbf.Write(line, PanelParameters::eMaxLineLength+1);
	}
	_ReadLine(line, PanelParameters::eMaxLineLength);
	char* colon = strchr(line, ':');
	if (0 != colon)
	{
		colon++;
		*colon = 0;
		if (val)
			strcpy(colon, " on");
		else
			strcpy(colon, " off");
	}
	else
		throw Exception(IDERR_PEDOMISSINGCOLON, act);

	tbf.Write(line, PanelParameters::eMaxLineLength+1);
	_CopyRest(tbf);
	_Commit(tbf);
}

void Ped::_DoNum(const char* act)
{
	static char realact[PanelParameters::eMaxLineLength+1];
	int line, field;
	float value, scale;
	if (4 != sscanf(act, "%d %d %f %f", &line, &field, &value, &scale))
		throw Exception(IDERR_PEDCOMMAND, act);

	if (fabs(scale)<0.00001)
		throw Exception(IDERR_PEDNMSCALESMALL, act);

	BufFile buffile;

	TmpFile tbf
		(
		buffile.c_str(),
		GENERIC_READ | GENERIC_WRITE, 0,
		CREATE_ALWAYS
		);

	static char ln[PanelParameters::eMaxLineLength+1];

	_OpenSrc();

	line--;
	float v = value/scale;

	for (int i=0; i<line; i++)
	{
		_ReadLine(ln, PanelParameters::eMaxLineLength);
		tbf.Write(ln, PanelParameters::eMaxLineLength+1);
	}
	_ReadLine(ln, PanelParameters::eMaxLineLength);

	{
		static char valstr[64];
		if (fabs(scale-1.0)<0.00001)
			sprintf(valstr, "%d", int(v));
		else
			sprintf(valstr, "%.4f", v);
		EdLin resln(ln);
		int ix = resln.Find(0, ':');
		if (-1 == ix)
			throw Exception(IDERR_PEDNUMFINDFIELD, act);
		
		switch (field)
		{
		case 1 :
			break;
		case 2 :
			ix = resln.Find(ix+1, ',');
			if (-1 == ix)
				throw Exception(IDERR_PEDNUMFINDFIELD, act);
			break;
		case 3 :
			ix = resln.Find(ix+1, ',');
			if (-1 == ix)
				throw Exception(IDERR_PEDNUMFINDFIELD, act);
			ix = resln.Find(ix+1, ',');
			if (-1 == ix)
				throw Exception(IDERR_PEDNUMFINDFIELD, act);
			break;
		default :
			throw Exception(IDERR_PEDNUMINVALIDFIELD, act);
		}
		ix = resln.SkipBlanks(ix+1);
		if (-1 == ix)
			throw Exception(IDERR_PEDNUMFINDFIELD, act);
		resln.DeleteChars(ix, "0123456789-.");
		resln.Insert(ix, valstr);

		tbf.Write(resln, PanelParameters::eMaxLineLength+1);
	}
	_CopyRest(tbf);
	_Commit(tbf);
}



FilePed::FilePed(const char* src)
{
	strcpy(_Fname, src);
	_fp = 0;
	_curline = -1;
}


FilePed::~FilePed()
{
	if (0 != _fp)
		fclose(_fp);
}


void FilePed::_OpenSrc()
{
	assert(0 == _fp);
	_fp = fopen(_Fname, "rt");
	if (0 == _fp)
		throw Exception(IDERR_SRCFILE, _Fname);
	_curline = 0;
}


void FilePed::_ReadLine(char *bf, int bfsz)
{
	assert(0 != _fp);
	int ix = 0;
	if (feof(_fp))
		throw Exception(IDERR_EOFFOUND, _Fname, _curline+1);

	memset(bf, 0, bfsz);
	for (;;)
	{
		int c = fgetc(_fp);
		if (('\n' == c) || (EOF == c))
		{
			bf[ix] = 0;
			if ('\n' == c)
				_curline++;
			break;
		}
		else
		{
			bf[ix] = (char) c;
			ix ++;
			if (bfsz==ix)
				throw Exception(IDERR_LINETOOLONG, _Fname, _curline+1);
		}
	}
}


void FilePed::_CopyRest(TmpFile& tmp)
{
	static char line[PanelParameters::eMaxLineLength+1];
	while (!(feof(_fp)))
	{
		_ReadLine(line, PanelParameters::eMaxLineLength);
		if (!(feof(_fp)))
			tmp.Write(line, PanelParameters::eMaxLineLength+1);
	}
}


void FilePed::_Commit(TmpFile& tmp)
{
	assert(0 != _fp);
	fclose(_fp);
	_fp = fopen(_Fname, "wt");
	if (0 == _fp)
		throw Exception(IDERR_SRCWRITE, _Fname);

	static char line[PanelParameters::eMaxLineLength+1];

	tmp.Reset();

	do
	{
		tmp.Read(line, PanelParameters::eMaxLineLength+1);
		if (!(tmp.Eof()))
		{
			fputs(line, _fp);
			fputc('\n', _fp);
		}
	} while (!(tmp.Eof()));
}



WndPed::WndPed(HWND hTarget) : 
_target(Window(hTarget)),
_maxlines(_target.GetLineCount())
{
	_curline = -1;
}


WndPed::~WndPed()
{}


void WndPed::_OpenSrc()
{
	_curline = 0;
}


void WndPed::_ReadLine(char* bf, int bfsz)
{
	assert(_curline>=0);
	int li = _target.LineIndex(_curline);
	if (-1 == li)
		throw Exception(IDERR_EOFTEXTFOUND, _curline+1);
	int length = _target.LineLength(li);
	if (length>bfsz)
		throw Exception(IDERR_WNDLINETOOLONG, _curline+1);
	std::string str;
	_target.GetLine(_curline, str);
	strcpy(bf, str.c_str());
	bf[length] = 0;
	_curline++;
}


void WndPed::_CopyRest(TmpFile& tmp)
{
	static char line[PanelParameters::eMaxLineLength+1];
	while (_curline<_maxlines)
	{
		_ReadLine(line, PanelParameters::eMaxLineLength);
		tmp.Write(line, PanelParameters::eMaxLineLength+1);
	}
}


void WndPed::_Commit(TmpFile& tmp)
{
	static char line[PanelParameters::eMaxLineLength+1];
	std::string cnts;

	tmp.Reset();

	for (int i=0; i<_maxlines; i++)
	{
		tmp.Read(line, PanelParameters::eMaxLineLength+1);
		cnts += line;
		if (i<_maxlines-1)
			cnts += "\r\n";
	}

	const int cl = _target.GetCurrentLine();
	const int fv = _target.GetFirstVisibleLine();
	_target.SetText(cnts);
	while (_target.GetFirstVisibleLine()<fv)
		_target.Scroll(SB_LINEDOWN);
	while (_target.GetFirstVisibleLine()>fv)
		_target.Scroll(SB_LINEUP);
	_target.SetCurrentLine(cl);
	//_target.ScrollCaret();
	_target.SetModify(true);
}

