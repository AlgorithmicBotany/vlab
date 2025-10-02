#ifndef __CURVEDLG_H__
#define __CURVEDLG_H__


class CurveTurtleDlg : public Dialog
{
public:
	CurveTurtleDlg();
	
	void UpdateData(bool);
	
public:
	LongString _contactX, _contactY, _contactZ;
	LongString _endX, _endY, _endZ;
	LongString _headX, _headY, _headZ;
	LongString _upX, _upY, _upZ;
	LongString _size;
};


class CurveTransformDlg : public Dialog
{
public:
	CurveTransformDlg();
	
	void UpdateData(bool);
	
public:
	LongString _rotA, _rotX, _rotY, _rotZ;
	LongString _scaleX, _scaleY, _scaleZ;
	LongString _transX, _transY, _transZ;
	LongString _setX, _setY, _setZ;
	
protected:
	bool Command(int, Window, UINT);
};


class CurveCAGDDlg : public Dialog
{
public:
	CurveCAGDDlg();
	
	void UpdateData(bool);
	
public:
	LongString _crvA, _crvB, _crvC, _crvD;
	LongString _crvF;
	LongString _crvT;
	LongString _crvN;
};


#else
	#error File already included
#endif
