#ifndef __IMAGELIST_H__
#define __IMAGELIST_H__


class ImageList
{
public:
	ImageList(int w, int h, UINT flags, int size, int grow)
	{
		_hList = ImageList_Create
			(w, h, flags, size, grow);
	}
	~ImageList()
	{
		ImageList_Destroy(_hList);
	}
	HIMAGELIST Handle() const
	{ return _hList; }
	void Build(HINSTANCE hInst, const UINT*, int);
private:
	HIMAGELIST _hList;
};



#endif
