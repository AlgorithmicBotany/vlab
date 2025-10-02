#include <fw.h>

#include "patchclrinfo.h"

#include "resource.h"

void PatchColorInfo::Load(ReadTextFile& src)
{
	float td, bd;
	std::string line;
	src.Read(line);
	if (4 != sscanf(line.c_str(), _StoreFormat(), &_TopColor, &td, &_BottomColor, &bd))
		throw Exception(IDERR_READINGPATCHCLRINFO, src.Filename(), src.Line());
	_TopDiffuse = td;
	_BottomDiffuse = bd;
}

void PatchColorInfo::Write(WriteTextFile& trg) const
{
	trg.PrintF(_StoreFormat(), _TopColor, _TopDiffuse, _BottomColor, _BottomDiffuse);
}


DWORD PatchColorInfo::ClipboardSize() const
{
	DWORD res = 0;
	res += sizeof(int);
	res += sizeof(float);
	res += sizeof(int);
	res += sizeof(float);
	return res;
}


char* PatchColorInfo::CopyToClipboard(char* pCur) const
{
	ToClipboard<int>(_TopColor, pCur);
	ToClipboard<float>(_TopDiffuse, pCur);
	ToClipboard<int>(_BottomColor, pCur);
	ToClipboard<float>(_BottomDiffuse, pCur);

	return pCur;
}


const char* PatchColorInfo::LoadFromClipboard(const char* pCur) 
{
	FromClipboard<int>(_TopColor, pCur);
	FromClipboard<float>(_TopDiffuse, pCur);
	FromClipboard<int>(_BottomColor, pCur);
	FromClipboard<float>(_BottomDiffuse, pCur);

	return pCur;
}
