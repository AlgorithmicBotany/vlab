// header file curveXYZ.h

#ifndef __CURVEXYZ_H__
#define __CURVEXYZ_H__

#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif

enum ECURVE {eBSPLINE = 0, eBSPLINECLOSED, eBSPLINEENDPOINT, eBSPLINEPHANTOM, 
             eBEZIER, eBEZIERSURFACE};


class CCPoint;
class CCurve;


class CPoint
{
public:
	CPoint();
public:
	float x, y, z;
	
};


class CTurtle
{
public:
	CTurtle();
public:
	bool   _p;
	int    _ps, _pt;
	
	CPoint _cp, _ep, _h, _u;
	float _sz;
	
};


class CColor
{
public:
	CColor();
public:
	int    _tc, _bc;
	float _td, _bd;
	
};


class CCPoint
{
public:
	CCPoint();
	~CCPoint();
public:
	CPoint	_point;
	
	float	_r[3];
	CCurve *_curve;
	
	int 	_pmult, _visible, _select;
	float	_saturation;
	
};


class CCurve
{
public:
	CCurve();
	~CCurve();
	
	CCurve* cloneCurve();
public:
	int 	 _csize;
	CCPoint *_point;
	ECURVE	 _ctype;
	
};


class CAPoint
{
public:
	CAPoint();
public:
	float _l;
	
	CPoint _point;
	
};


class CCurveXYZ
{
public:
	CCurveXYZ();
	CCurveXYZ(const char* filename, int samples);
	~CCurveXYZ();
	
	void operator=(const CCurveXYZ&);
	
	CCurve* init(ECURVE type);
	
	float ti(int i);
	float Bi1(int i, float t);
	float Bi2(int i, float t);
	float Bi3(int i, float t);
	float Bi4(int i, float t);
	
	float uk(int k);
	float Nk1(int k, float u);
	float Nk2(int k, float u);
	float Nk3(int k, float u);
	float Nk4(int k, float u);
	
	CPoint calcPValue(int i, float t, float s, float r);
	CPoint calcAValue(int i, float t, float s, float r);
	CPoint calcXValue(int i, float t, float s, float r);
	
	void calcVariety();
	
	void createPPoints(CCurve *curve);
	void createAPoints();
	
	void addPoint(CCurve *curve, int num, float x, float y, float z);
	void deletePoint(CCurve *curve, int num);
	
	void divideCurve(CCurve *curve);
	
	void translatePoints(float x, float y, float z);
	void scalePoints(float x, float y, float z);
	void rotatePoints(float alpha, float x, float y, float z);
	void rotateFrames(float t1x, float t1y, float t1z, float n1x, 
		float n1y, float n1z, float t2x, float t2y, float t2z, float n2x,
		float n2y, float n2z);
	
	void readCurve(char* command);
	void openCurve(const char* filename);
	void rereadCurve();

public:
	char	  _filename[_MAX_PATH+1];
	
	CTurtle   _turtle;
	CColor	  _color;
	
	CCurve	 *_curve;
	
	int 	  _psize;
	CPoint	 *_ppoint;
	ECURVE	  _ptype;
	
	int 	  _asize;
	CAPoint  *_apoint;
	float	  _alength;
	
	int 	  _cmax, _clevel;
	CCurve	**_ccurve;
	int 	 *_cpoint, *_cdimen;
	
};


#endif
