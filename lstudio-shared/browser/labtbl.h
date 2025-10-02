#ifndef __LABTABLE_H__
#define __LABTABLE_H__



namespace VLB 
{

class LabTable
{
public:
	LabTable();
	~LabTable();
	const char* Path() const
	{ return _path.c_str(); }
	void Clean();
private:
	std::string _path;
	std::string _PDir;
};



}

#else
	#error File already included
#endif
