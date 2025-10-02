#ifndef __SURFTHUMBCALLBACKS_H__
#define __SURFTHUMBCALLBACKS_H__


class SurfaceEdit;

class XMovedCallback : public LineThumbCallback
{
public:
	XMovedCallback(SurfaceEdit* pEdit) : _pEdit(pEdit)
	{}
	void Moved(float, bool);
private:
	SurfaceEdit* _pEdit;
};

class YMovedCallback : public LineThumbCallback
{
public:
	YMovedCallback(SurfaceEdit* pEdit) : _pEdit(pEdit)
	{}
	void Moved(float, bool);
private:
	SurfaceEdit* _pEdit;
};

class ZMovedCallback : public LineThumbCallback
{
public:
	ZMovedCallback(SurfaceEdit* pEdit) : _pEdit(pEdit)
	{}
	void Moved(float, bool);
private:
	SurfaceEdit* _pEdit;
};


#else
	#error File already included
#endif
