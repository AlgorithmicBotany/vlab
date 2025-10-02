#ifndef __TEXTUREPROPERTIESDLG_H__
#define __TEXTUREPROPERTIESDLG_H__


class TexturePropertiesDlg : public Dialog
{
public:
	TexturePropertiesDlg(const BITMAPINFOHEADER&);

	bool DoInit();
private:
	int _width;
	int _height;
	int _bitCount;
};


#else
	#error File already included
#endif
