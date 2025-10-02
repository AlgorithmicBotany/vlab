// header file curveXYZv.h

#ifndef __CURVEXYZV_H__
#define __CURVEXYZV_H__


#include <GL/gl.h>
#include <GL/glu.h>

#include "BallAux.h"
#include "Ball.h"

#include "CAGD.h"

#include "curveXYZ.h"


#define MAXBSPLINEBZUNDO 20


class CurveEdit;
class CurveView;


class CVector
{
public:
	CVector();
	
	void   SAdd(float);
	void   VAdd(CVector);
	void   VSub(CVector);
	void   SMult(float);
	float SProduct(CVector);
	void   VProduct(CVector, CVector);
	void   NVector();
	float VLength();
	float XYAngle();
	float YZAngle();
	float XZAngle();
	
	void setPoint(CPoint);
	void setPoints(CPoint, CPoint);
	CPoint getPoint();
public:
	float x, y, z;
	
};


class CMatrix
{
public:
    CMatrix();
	
    void SetRX(float alpha);
    void SetRY(float beta);
    void SetRZ(float gamma);
	
    void Mult(CMatrix hM);
    
    CVector CalcVector(CVector hV);
	
    void SetRotate(CVector hV, float alpha);
    void SetRotate2(CVector hVt1, CVector hVn1, CVector hVb1,
		CVector hVt2, CVector hVn2, CVector hVb2);
public:
    float Coeff[3][3]; // row / column
	
};


class CurveXYZView : public EditableObject
{
public:
	CurveEdit *_curveEdit;
	CurveView *_curveView;
	
	CCurveXYZ *_curveXYZ;
	
	int 	   _width, _height;
	
	bool	   _axes, _grid, _gridXY, _gridXZ, _gridYZ;
	bool	   _dpoints, _dconvex, _dcurve;
	
	CPoint	   _arrow;
	int 	   _selecti, _selectl, _selectp, _selects;
	CCurve	  *_selectc;
	
	int 	   _undoindex, _undocount;
	CCurveXYZ  _undo[MAXBSPLINEBZUNDO];
	bool	   _undosave;
	
	float _clear_r, _clear_g, _clear_b;
	
	float _xaxis_radius, _xaxis_length, _xaxis_base, _xaxis_top, _xaxis_height;
	int    _xaxis_slices, _xaxis_stacks;
	float _xaxis_r, _xaxis_g, _xaxis_b;
	float _yaxis_radius, _yaxis_length, _yaxis_base, _yaxis_top, _yaxis_height;
	int    _yaxis_slices, _yaxis_stacks;
	float _yaxis_r, _yaxis_g, _yaxis_b;
	float _zaxis_radius, _zaxis_length, _zaxis_base, _zaxis_top, _zaxis_height;
	int    _zaxis_slices, _zaxis_stacks;
	float _zaxis_r, _zaxis_g, _zaxis_b;
	
	float _ppoint_radius;
	int    _ppoint_slices, _ppoint_stacks;
	float _ppoint_r, _ppoint_g, _ppoint_b;
	float _vpoint_radius;
	int    _vpoint_slices, _vpoint_stacks;
	float _vpoint_r, _vpoint_g, _vpoint_b;
	
	float _pconvex_radius;
	int    _pconvex_slices, _pconvex_stacks;
	float _pconvex_r, _pconvex_g, _pconvex_b;
	float _vconvex_radius;
	int    _vconvex_slices, _vconvex_stacks;
	float _vconvex_r, _vconvex_g, _vconvex_b;
	
	float _pcurve_radius;
	int    _pcurve_slices, _pcurve_stacks, _pcurve_points, _pcurve_lines;
	float _pcurve_r, _pcurve_g, _pcurve_b;
	float _vcurve_radius;
	int    _vcurve_slices, _vcurve_stacks, _vcurve_points, _vcurve_lines;
	float _vcurve_r, _vcurve_g, _vcurve_b;
	
	float _selectb_radius, _selectb_length;
	int    _selectb_slices, _selectb_stacks;
	float _selectb_r, _selectb_g, _selectb_b;
	float _selectx_radius, _selectx_length, _selectx_base, _selectx_top;
	float _selectx_height;
	int    _selectx_slices, _selectx_stacks;
	float _selectx_r[2], _selectx_g[2], _selectx_b[2];
	float _selecty_radius, _selecty_length, _selecty_base, _selecty_top;
	float _selecty_height;
	int    _selecty_slices, _selecty_stacks;
	float _selecty_r[2], _selecty_g[2], _selecty_b[2];
	float _selectz_radius, _selectz_length, _selectz_base, _selectz_top;
	float _selectz_height;
	int    _selectz_slices, _selectz_stacks;
	float _selectz_r[2], _selectz_g[2], _selectz_b[2];
	
	float _grid_size[2];
	float _gridxy_radius;
	int    _gridxy_slices, _gridxy_stacks;
	float _gridxy_r, _gridxy_g, _gridxy_b;
	float _gridxz_radius;
	int    _gridxz_slices, _gridxz_stacks;
	float _gridxz_r, _gridxz_g, _gridxz_b;
	float _gridyz_radius;
	int    _gridyz_slices, _gridyz_stacks;
	float _gridyz_r, _gridyz_g, _gridyz_b;
	
	float _mouse_rotXY, _mouse_scale;
	
	float _stat_clear_r, _stat_clear_g, _stat_clear_b;
	float _stat_font_r, _stat_font_g, _stat_font_b;
	
	bool   _arcball_wrap, _arcball_circle, _arcball_constraints, _arcball_arc;
	float _arcball_circle_radius, _arcball_circle_r, _arcball_circle_g, _arcball_circle_b;
	float _arcball_constraints_radius, _arcball_constraints_r[2], _arcball_constraints_g[2], _arcball_constraints_b[2];
	float _arcball_arc_radius, _arcball_arc_r, _arcball_arc_g, _arcball_arc_b;
	
	bool   _arrowX, _arrowY, _arrowZ;
	
	float _xcoeff, _ycoeff;
	
	bool   _pointer;
	
	bool   _rot, _rotTB, _arcball, _wrap;
	float _rotX, _rotY;
	
	bool   _scale, _scaleTB;
	float _scaleX, _scaleY, _scaleZ;
	
	bool   _trans, _transTB;
	float _transX, _transY, _transZ;
	
	float _xmin, _xmax, _ymin, _ymax, _zmin, _zmax;
	
	HVect	 vNow;
	Place	 winsize, winorig;
	CPoint	 mouseNow, mousePre;
	BallData ball;
	
public:
	CurveXYZView();
	~CurveXYZView();
	
	void Copy(const EditableObject*);
	EditableObject* Clone() const;
	void Reset() {}
	void DrawInGallery() const;
	void Generate(WriteTextFile&) const;
	
	DWORD ClipboardSize() const { return 0; }
	char* CopyToClipboard(char*) const { return 0; }
	const char* LoadFromClipboard(const char*) { return 0; }
	
	void initView();
	
	void calcGrid();
	void drawAxes(GLenum mode);
	void drawGrid(GLenum mode);
	
	void drawPPoint(CCurve *curve, int level, GLenum mode);
	void drawPConvex(CCurve *curve, int level, GLenum mode);
	void drawPCurve(CCurve *curve, int level, GLenum mode);
	
	void drawSelection(CCurve *curve, int level, GLenum mode);
	void findSelection(CCurve *curve, int level);
	void emitSelection();
	void drawArrows(GLenum mode);
	void drawView(GLenum mode);
	
	void addPointPred();
	void addPointSucc();
	void delPoint();
	
	void addSurfaceNS(int row);
	void addSurfaceWE(int col);
	void addSurfaceNESESWNW(int row, int col);
	void delSurface();
	
	void setRotAxes(AxisSet);
	
	void undoSave();
	void undoUndo();
	void undoRedo();
	
	void deleteSelection(CCurve *curve, int min, int max, int r);
	void selectObject(int, int, bool);
	
	CVector calcMovement(CPoint);
	
	void getMinMax(CCurve *curve, int level);
	void fitPoints();
	
	void defaultView(int, int, GLint*, GLenum);
	
	void setEdit(CurveEdit*);
	void setView(CurveView*);
	
	void initGraph();
	void updateGraph();
	void updateEdit() {}
	
	void movePoints(CCurve *curve, CVector move);
	void centerCurve(CCurve *curve, CPoint center);
	
	void initializeGL();
	void resizeGL(int, int);
	void paintGL();
	
	void mouseLDown(int, int, bool, bool, bool);
	void mouseLUp(int, int);
	void mouseMove(int, int);
	
public:
	char	_dirname[_MAX_PATH+1], _filename[256];
	
	CCurve *_copy;
	
	void updateView();
	
	void openFile(const char*);
	void saveFile(const char*) const;
	
	void openContour(const char*);
	void saveContour(const char*) const;
	
	void openFuncEdit(const char*);
	void saveFuncEdit(const char*) const;
	
	void openSurface(const char*);
	void saveSurface(const char*, bool) const;
	
	void openCurve(const char*);
	void writeCurve(CCurve *curve, std::ofstream &outFile) const;
	void saveCurve(const char*) const;
	
	void updateAllWindows();
	void updateSubWindows();
	
	void readError(const char*);
	void readConfig(char*);
	void openConfig(const char*);
	void initConfig();
	
	void createVariation(CCurve *curve, ECURVE type);
	
	void alignPoints(CCurve *curve, CPoint point);
	
	void selectAll(CCurve *curve);
	void selectCurve(CCurve *curve);
	void selectPoint(CCurve *curve, int selects);
	
	void copyCurve();
	void pasteCurve();
	
	void rotatePoints(CCurve *curve, float alpha, CVector vct);
	void scalePoints(CCurve *curve, CVector vct);
	void transPoints(CCurve *curve, CVector vct);
	void setPoints(CCurve *curve, CVector vct);
	
	const char* findPatch(int, int, int, int, int, int, int, int, int) const;
	
	void setPMult(CCurve *curve, int pmult);
	
	void setShowAll(CCurve *curve);
	void setShowCurve(CCurve *curve);
	void setShowPatch(CCurve *curve);
	void setShowPoint(CCurve *curve);
	
	void setHideCurve(CCurve *curve);
	void setHidePatch(CCurve *curve);
	void setHidePoint(CCurve *curve);
	
	void setBrightAll(CCurve *curve);
	void setBrightCurve(CCurve *curve);
	void setBrightPatch(CCurve *curve);
	void setBrightPoint(CCurve *curve);
	
	void setDarkCurve(CCurve *curve);
	void setDarkPatch(CCurve *curve);
	void setDarkPoint(CCurve *curve);
	
	void chgPoints(CPoint p, CPoint n);
	void sppPoints();
	
	void setFPatchX(CCurve *curve);
	void setFPatchY(CCurve *curve);
	
	void mInitCurve(ECURVE);
	void mInitBSpline();
	void mInitBzSurface();
	
	void mOpen(const char*);
	void mSave(const char*);
	
	void mInitView();
	void mInitFit();
	
	void mUndo();
	void mRedo();
	
	void mPointer();
	void mRotate();
	void mScale();
	void mTranslate();
	
	void mAxes();
	void mGridGrid();
	void mGridXY();
	void mGridXZ();
	void mGridYZ();
	
	void mAddPred();
	void mAddSucc();
	void mDelPoint();
	
	void mAddN();
	void mAddNE();
	void mAddE();
	void mAddSE();
	void mAddS();
	void mAddSW();
	void mAddW();
	void mAddNW();
	void mDelSurface();
	
	void mMultX(int);
	void mMult1();
	void mMult2();
	void mMult3();
	void mMult4();
	
	void mTypeX(ECURVE);
	void mTypeBSpline();
	void mTypeBClosed();
	void mTypeBEndpoint();
	void mTypeBPhantom();
	void mTypeBzSpline();
	void mTypeBzSurface();
	
	void mVarX(ECURVE);
	void mVarBSpline();
	void mVarBzSurface();
	void mVarDel();
	
	void mDivide();
	
	void mSelectAll();
	void mSelectCurve();
	void mSelectPX(int);
	
	void mAlign();
	
	void mDPoints();
	void mDConvex();
	void mDCurve();
	
	void mShowAll();
	void mShowCurve();
	void mShowPatch();
	void mShowPoint();
	
	void mHideCurve();
	void mHidePatch();
	void mHidePoint();
	
	void mBrightAll();
	void mBrightCurve();
	void mBrightPatch();
	void mBrightPoint();
	
	void mDarkCurve();
	void mDarkPatch();
	void mDarkPoint();
	
	void mRotate(float, float, float, float);
	void mScale(float, float, float);
	void mTranslate(float, float, float);
	void mSet(float, float, float);
	
	void mSPatchAll();
	void mSPatchCorner();
	void mSPatchLeft();
	void mSPatchRight();
	void mSPatchTop();
	void mSPatchBottom();
	
	void mFPatchY();
	void mFPatchX();
	
	void mCBlossom(float, float, float, float, float, float);
	void mCInterpolation(float, float, float, float, float, float, int);
};


#endif
