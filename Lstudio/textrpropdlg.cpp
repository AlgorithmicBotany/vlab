#include <warningset.h>

#include <fw.h>


#include "textrpropdlg.h"
#include "resource.h"


TexturePropertiesDlg::TexturePropertiesDlg(const BITMAPINFOHEADER& src) : Dialog(IDD_TEXTUREPROP)
{
	_height = src.biHeight;
	_width = src.biWidth;
	_bitCount = src.biBitCount;
}


bool TexturePropertiesDlg::DoInit()
{
	SetDlgItemInt(_hDlg, IDC_HEIGHT, _height, FALSE);
	SetDlgItemInt(_hDlg, IDC_WIDTH, _width, FALSE);

	Window BitCount(GetDlgItem(IDC_COLORS));
	switch (_bitCount)
	{
	case 1 : 
		BitCount.SetText(IDS_1BITMAP);
		break;
	case 4 : 
		BitCount.SetText(IDS_4BITMAP);
		break;
	case 8 :
		BitCount.SetText(IDS_8BITMAP);
		break;
	case 24 :
		BitCount.SetText(IDS_24BITMAP);
		break;
	}
	return true;
}
