#include <fstream>
#include <strstream>

#include <fw.h>

#include "../shstrng.h"

#include "vlboptns.h"

namespace VLB
{
	Options options;
	const char* Labels[] =
	{
		"RA cr-lf:",
		"Ignored files:",
		"Browser logon:",
		"Browser background:",
		0
	};
}

const COLORREF kDefaultBackgroundColor = RGB(0, 64, 0);

VLB::Options::Options(int fontsize, const std::string& fontname, int iconwidth) 
{
	_IconWidth = iconwidth;
	_FontSize = fontsize;
	_FontName = fontname;
	_BgColor = kDefaultBackgroundColor;
}

VLB::Options::Options()
{
	_IconWidth = 48;
	_FontSize = 16;
	_FontName = "Arial";
	_BgColor = kDefaultBackgroundColor;
}

void VLB::Options::Load(const std::string& fname)
{
	std::ifstream src(fname.c_str());
	if (!src.good())
		throw Exception(SharedStr::GetLibString(SharedStr::strErrOpenConfig), fname.c_str());
	while (!src.eof())
	{
		std::string line;
		std::getline(src, line);
		int ln;
		int lbl = Label(line, ln);
		switch (lbl)
		{
		case lblUnknown :
			break;
		case lblCrlf :
			ReadCrLf(line.substr(ln));
			break;
		case lblIgnored :
			ReadIgnored(line.substr(ln));
			break;
		case lblLogon :
			ReadLogon(line.substr(ln));
			break;
		case lblBackgroundColor:
			_BgColor = ReadColor(line.substr(ln));
			break;
		}
	}
}



COLORREF VLB::Options::ReadColor(const std::string& line)
{
	COLORREF result = RGB(0, 0, 0);
	int r, g, b;
	if (3==sscanf(line.c_str(), "%d %d %d", &r, &g, &b))
	{
		result = RGB(r, g, b);
	}

	return result;
}



VLB::Options::eLabel VLB::Options::Label(const std::string& str, int& ln) const
{
	for (int i=0; Labels[i] != 0; ++i)
	{
		size_t l = strlen(Labels[i]);
		if (0==strncmp(Labels[i], str.c_str(), l))
		{
			assert(i>=lblUnknown);
			assert(i<lblLastLabel);
			ln = l;
			while (isspace(str[ln]))
				++ln;
			return static_cast<eLabel>(i);
		}
	}
	return lblUnknown;
}

void VLB::Options::ReadLogon(const std::string& cstr) 
{
	std::string str(cstr);
	std::strstream line(&str[0], str.length(), std::ios_base::out);
	_Host.erase();
	_Oofs.erase();
	_User.erase();
	_Paswd.erase();
	line >> _Host >> _Oofs >> _User >> _Paswd;
	/*
	_Host.clear();
	std::string::const_iterator it = str.begin();
	if (it == str.end())
		return;
	while (!isspace(*it) && it != str.end())
	{
		_Host.append(*it);
		++it;
	}
	while (isspace(*it) && it != str.end())
		++it;
	if (it == str.end())
		return;
	while (!isspace(*it) && it != str.end())
	{
		_Oofs.append(*it);
		++it;
	}
	if (it == str.end())
		return;
	while (isspace(*it) && it != str.end())
		++it;
	if (it == str.end())
		return;
	*/
}

void VLB::Options::ReadCrLf(const std::string& line)
{
	AddBuffer(line, _Crlf);
}

void VLB::Options::ReadIgnored(const std::string& line)
{
	AddBuffer(line, _Ignored);
}

void VLB::Options::AddBuffer(const std::string& line, string_buffer& sb)
{
	const char* p = line.c_str();
	assert(!isspace(*p));
	while (0 != *p)
	{
		p = sb.add_str(p);
		while (isspace(*p))
			++p;
	}
}
