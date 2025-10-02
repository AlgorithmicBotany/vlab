#ifndef __VLBROWSRPARAMS_H__
#define __VLBROWSRPARAMS_H__


namespace VLB
{

namespace Parameters
{

enum ParamName
{
	pIconX = 0,
	pIconY,
	pSpaceY,
	pVspace,
	pObjIconX,
	pDefObjIconCx,
	pDefObjIconCy,
	pDefFontSize
};

const int Params[] =
{
	16, // IconX
	16, // IconY
	0,  // SpaceY
	16, // Vspace
	48, // ObjIconX
	41, // pDefObjIconCx
	41, // pDefObjIconCy
	16  // pDefFontSize
};

/*
enum IconId
{
	iObject = 0,
	iExt,
	iLinked,
	iIconCount
};

HICON GetIcon(IconId);
*/

enum ColorName
{
	Background = 0,
	Line,
	Text,
	SelectedTextBg
};

const COLORREF Colors[] =
{
	RGB(0, 0, 0),
	RGB(255, 255, 0),
	RGB(255, 255, 255),
	RGB(66, 41, 182)
};


}

}

#else
	#error File already included
#endif
