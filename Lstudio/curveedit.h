#ifndef __CURVEEDIT_H__
#define __CURVEEDIT_H__


#include "curveXYZv.h"
#include "curvedlg.h"


#define CURVEEDITRESIZABLE


class CurveEdit : public FormCtrl, public ObjectEdit
{
public:
	CurveEdit(HWND, HINSTANCE, PrjNotifySink*);
	~CurveEdit();

	static HWND Create(HWND, HINSTANCE, PrjNotifySink*);
	static BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

	
	bool Command(int, Window ctl, UINT);

#ifdef CURVEEDITRESIZABLE
	bool Size(SizeState, int, int);
#endif


	void Import(const TCHAR*);
	void Generate() const;

	void updateEdit();
	void createSelect(CCurve *curve, int level);
	void updateMenu();
	void updateTool();
	void updateStat();
	void updateStat(const char*);

	void Modified();
private:
	PrjNotifySink *_pNotifySink;

	CurveView    *_curveView;
	CurveXYZView *_curveXYZView;

	int           _selects;

	float        _rotA, _rotX, _rotY, _rotZ;
	float        _scaleX, _scaleY, _scaleZ;
	float        _transX, _transY, _transZ;
	float        _setX, _setY, _setZ;

	float        _crvA, _crvB, _crvC, _crvD, _crvF, _crvT;
	int           _crvN;

	HWND _hwndTT;
	void _UpdateControls();
	void _UpdateFromControls();
	void _UpdateView() {}

	const Icon _iInitBSpline;
	const Icon _iInitBzSurface;
	const Icon _iOpen;
	const Icon _iSave;
	const Icon _iInitView;
	const Icon _iInitFit;
	const Icon _iUndo;
	const Icon _iRedo;
	const Icon _iPointer;
	const Icon _iRotate;
	const Icon _iScale;
	const Icon _iTranslate;
	const Icon _iAxes;
	const Icon _iGrid;
	const Icon _iInsert;
	const Icon _iDelete;
	const Icon _iMult;
	const Icon _iType;
	const Icon _iSelect;
	const Icon _iAlign;
	const Icon _iDPoints;
	const Icon _iDConvex;
	const Icon _iDCurve;
	const Icon _iDivide;
	const Icon _iShow;
	const Icon _iHide;
	const Icon _iBright;
	const Icon _iDark;
	const Icon _iTurtle;
	const Icon _iTransform;
	const Icon _iSPaper;
	const Icon _iFlip;
	const Icon _iCAGD;

#ifdef CURVEEDITRESIZABLE
	GeometryConstrain _ViewWnd;
	GeometryConstrain _Gallery;
	GeometryConstrain _NameLbl;
	GeometryConstrain _Name;
	GeometryConstrain _StatWnd;
	GeometryConstrain _gSimple;
	GeometryConstrain _gInitBSpline;
	GeometryConstrain _gInitBzSurface;
	GeometryConstrain _gOpen;
	GeometryConstrain _gSave;
	GeometryConstrain _gInitView;
	GeometryConstrain _gInitFit;
	GeometryConstrain _gUndo;
	GeometryConstrain _gRedo;
	GeometryConstrain _gPointer;
	GeometryConstrain _gRotate;
	GeometryConstrain _gScale;
	GeometryConstrain _gTranslate;
	GeometryConstrain _gAxes;
	GeometryConstrain _gGrid;
	GeometryConstrain _gInsert;
	GeometryConstrain _gDelete;
	GeometryConstrain _gMult;
	GeometryConstrain _gType;
	GeometryConstrain _gSelect;
	GeometryConstrain _gAlign;
	GeometryConstrain _gDPoints;
	GeometryConstrain _gDConvex;
	GeometryConstrain _gDCurve;
	GeometryConstrain _gDivide;
	GeometryConstrain _gShow;
	GeometryConstrain _gHide;
	GeometryConstrain _gBright;
	GeometryConstrain _gDark;
	GeometryConstrain _gTurtle;
	GeometryConstrain _gTransform;
	GeometryConstrain _gSPaper;
	GeometryConstrain _gFlip;
	GeometryConstrain _gCAGD;
#endif
	
	void _Simple();

	void mInitBSpline();
	void mInitBzSurface();

	void mOpen();
	void mSave();

	void mInitView();
	void mInitFit();

	void mUndo();
	void mRedo();

	void mPointer();
	void mRotate();
	void mScale();
	void mTranslate();

	void mAxes();

	void mGrid(int, int);

	void mGridGrid();
	void mGridXY();
	void mGridXZ();
	void mGridYZ();

	void mInsert(int, int);

	void mAddPred();
	void mAddSucc();

	void mAddN();
	void mAddNE();
	void mAddE();
	void mAddSE();
	void mAddS();
	void mAddSW();
	void mAddW();
	void mAddNW();

	void mVarBSpline();
	void mVarBzSurface();

	void mDelete(int, int);

	void mDelPoint();
	void mDelSurface();
	void mDelVariation();

	void mMult(int, int);

	void mMult1();
	void mMult2();
	void mMult3();
	void mMult4();

	void mType(int, int);

	void mTypeBSpline();
	void mTypeBClosed();
	void mTypeBEndpoint();
	void mTypeBPhantom();
	void mTypeBzSpline();
	void mTypeBzSurface();

	void mSelect(int, int);

	void mSelectAll();
	void mSelectCurve();
	void mSelectPX(int);

	void mAlign();

	void mDPoints();
	void mDConvex();
	void mDCurve();

	void mVariation(int, int);

	void mVPoints();
	void mVConvex();
	void mVCurve();

	void mDivide();

	void mShow(int, int);
	void mShowAll();
	void mShowCurve();
	void mShowPatch();
	void mShowPoint();

	void mHide(int, int);
	void mHideAll();
	void mHideCurve();
	void mHidePatch();
	void mHidePoint();

	void mBright(int, int);
	void mBrightAll();
	void mBrightCurve();
	void mBrightPatch();
	void mBrightPoint();

	void mDark(int, int);
	void mDarkAll();
	void mDarkCurve();
	void mDarkPatch();
	void mDarkPoint();

	void mTurtle();
	void mTransform();

	void mSPaper(int, int);
	void mSPatchAll();
	void mSPatchCorner();
	void mSPatchLeft();
	void mSPatchRight();
	void mSPatchTop();
	void mSPatchBottom();

	void mFlip(int, int);
	void mFPatchX();
	void mFPatchY();

	void mCAGD(int, int);
	void mCBlossom();
	void mCInterpolation();
};


#else
	#error File already included
#endif
