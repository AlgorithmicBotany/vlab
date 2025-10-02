#ifndef __DROPFILES_H__
#define __DROPFILES_H__



class LongString;


class DroppedFiles
{
public:
	DroppedFiles(HDROP hDrop) : _hDrop(hDrop) {}
	~DroppedFiles()
	{ DragFinish(_hDrop); }
	UINT NumOfFiles()
	{ return DragQueryFile(_hDrop, 0xFFFFFFFF, 0, 0); }
	void GetFilename(UINT, std::string&);
private:
	HDROP _hDrop;
};


#endif
