#ifndef __LINTRFC_H__
#define __LINTRFC_H__

#include <cmath>
#include <string>
#include <iostream>
/*

        L-interface:
        these are the declarations required by
        the Generate module as well as the
        lsys.i (l2c-ed L file) to compile

*/

typedef short int __lc_ModuleIdType;

// if the requirements for data alignment change
// change this type
// for example to ensure 4-byte alignment
// set it to float
// 8-byte alignment set it to double
// etc.
typedef double __lc_MainAlignType;

struct __lc_BasicParameterStruct {
  __lc_ModuleIdType moduleId;
};

// MIK 02/2013 - define a struct for a 'following module' in environmental
// program communication. This is need to get a pointer to the first parameter
// of the module.
struct __lc_FollowingModuleStruct {
  struct __lc_Data {
    __lc_ModuleIdType moduleId;
    float Param0;
  } data;
};

class __lc_ActualParameters {
public:
  __lc_ActualParameters() : _count(0) {}
  const char *pArg(int i) const { return _arr[i]; }
  void AddModuleAddr(const char *pX) {
    _arr[_count] = pX;
    ++_count;
  }
  int Count() const { return _count; }
  void Reset() { _count = 0; }
  void Reverse() {
    int high = _count - 1;
    int low = 0;
    while (high > low) {
      const char *tmp = _arr[high];
      _arr[high] = _arr[low];
      _arr[low] = tmp;
      --high;
      ++low;
    }
  }

private:
  const char *_arr[32]; // ***** CHANGED THIS FROM __lc_eMaxFormalModules to 32
  int _count;
};

class __lc_CallerData {
public:
  void Reset() {
    _RCntxt.Reset();
    _Strct.Reset();
    _LCntxt.Reset();
  }
  const __lc_ActualParameters &RCntxt() const { return _RCntxt; }
  const __lc_ActualParameters &Strct() const { return _Strct; }
  const __lc_ActualParameters &LCntxt() const { return _LCntxt; }
  __lc_ActualParameters &RCntxt() { return _RCntxt; }
  __lc_ActualParameters &Strct() { return _Strct; }
  __lc_ActualParameters &LCntxt() { return _LCntxt; }

private:
  __lc_ActualParameters _RCntxt;
  __lc_ActualParameters _Strct;
  __lc_ActualParameters _LCntxt;
};

typedef bool (*__lc_ProdCaller)(const __lc_CallerData *);

struct __lc_ProductionModules {
  __lc_ModuleIdType
      arr[32]; // ***** CHANGED THIS FROM __lc_eMaxFormalModules to 32
  int count;
};

struct __lc_ContextProto {
  bool HasNewContext() const { return NewCntxt.count > 0; }
  __lc_ProductionModules Cntxt;
  __lc_ProductionModules NewCntxt;
};

struct __lc_ProductionPredecessor {
  __lc_ContextProto LCntxt;
  __lc_ProductionModules Strct;
  __lc_ContextProto RCntxt;
  __lc_ProdCaller pCaller;
  int iConsiderGroup;
};

struct __lc_ModuleData {
  const char *Name;
  int size;
};

typedef const char *__lc_Text;

#ifdef NOAUTOOVERLOAD

extern "C" {
struct V2f {
  float x, y;
  V2f() {
    x = 0;
    y = 0;
  }

  V2f(float nx, float ny) {
    x = nx;
    y = ny;
  }
};
struct V3f {
  float x, y, z;
  V3f(float nx, float ny, float nz) {
    x = nx;
    y = ny;
    z = nz;
  }
  V3f() {
    x = 0;
    y = 0;
    z = 0;
  }
};

struct V2d {
  double x, y;
  V2d(double nx, double ny) {
    x = nx;
    y = ny;
  }
  V2d() {
    x = 0;
    y = 0;
  }
};

struct V3d {
  double x, y, z;
  V3d(double nx, double ny, double nz) {
    x = nx;
    y = ny;
    z = nz;
  }
  V3d() {
    x = 0;
    y = 0;
    z = 0;
  }
};

typedef V2f __lc_V2f;
typedef V2d __lc_V2d;
typedef V3f __lc_V3f;
typedef V3d __lc_V3d;
}

V2f Normalize(V2f v) {
  float len = std::sqrt(v.x * v.x + v.y * v.y);
  v.x /= len;
  v.y /= len;
  return v;
}

V2d Normalize(V2d v) {
  double len = std::sqrt(v.x * v.x + v.y * v.y);
  v.x /= len;
  v.y /= len;
  return v;
}

V3f Normalize(V3f v) {
  float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  v.x /= len;
  v.y /= len;
  v.z /= len;
  return v;
}

V3d Normalize(V3d v) {
  double len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
  v.x /= len;
  v.y /= len;
  v.z /= len;
  return v;
}

typedef V2f V2tf;
typedef V2d V2td;
typedef V3f V3tf;
typedef V3d V3td;

#else

extern "C" {
struct __lc_V2f {
  float x, y;
};
}

template <typename f> struct V2t {
  f x, y;

  void Set(f nx, f ny) {
    x = nx;
    y = ny;
  }

  friend V2t<f> operator-(V2t<f> o) {
    V2t<f> result;
    result.Set(-o.x, -o.y);
    return result;
  }

  friend V2t<f> operator+(V2t<f> l, V2t<f> r) {
    V2t<f> result;
    result.Set(l.x + r.x, l.y + r.y);
    return result;
  }

  friend V2t<f> operator-(V2t<f> l, V2t<f> r) {
    V2t<f> result;
    result.Set(l.x - r.x, l.y - r.y);
    return result;
  }

  friend V2t<f> operator*(V2t<f> l, f r) {
    V2t<f> result;
    result.Set(l.x * r, l.y * r);
    return result;
  }

  friend V2t<f> operator*(f l, V2t<f> r) { return r * l; }

  friend f operator*(V2t<f> l, V2t<f> r) { return l.x * r.x + l.y * r.y; }

  friend V2t<f> operator/(V2t<f> l, f r) {
    V2t<f> result;
    result.Set(l.x / r, l.y / r);
    return result;
  }

  V2t<f> &Normalize() { return operator/=(Length()); }
  
  V2t<f> Normalized() { 
    V2t<f> result;
    result.Set(x,y);
    return result /= (Length()); 
  }

  f Length() const { return std::sqrt(x * x + y * y); }

  V2t<f> &operator+=(V2t<f> r) {
    x += r.x;
    y += r.y;
    return *this;
  }
  V2t<f> &operator-=(V2t<f> r) {
    x -= r.x;
    y -= r.y;
    return *this;
  }
  V2t<f> &operator/=(f r) {
    x /= r;
    y /= r;
    return *this;
  }
  V2t<f> &operator*=(f r) {
    x *= r;
    y *= r;
    return *this;
  }
};

typedef V2t<float> V2tf;
typedef V2t<double> V2td;

template <typename f> struct V2 : V2t<f> {
  V2() {
    V2t<f>::x = static_cast<f>(0.0);
    V2t<f>::y = static_cast<f>(0.0);
  }
  V2(f nx, f ny) {
    V2t<f>::x = static_cast<f>(nx);
    V2t<f>::y = static_cast<f>(ny);
  }

  V2(const V2tf &src) {
    V2t<f>::x = static_cast<f>(src.x);
    V2t<f>::y = static_cast<f>(src.y);
  }
  V2(const V2td &src) {
    V2t<f>::x = static_cast<f>(src.x);
    V2t<f>::y = static_cast<f>(src.y);
  }

  template <typename g> explicit V2(const V2t<g> &v) {
    V2t<f>::x = static_cast<f>(v.x);
    V2t<f>::y = static_cast<f>(v.y);
  }
  V2(const __lc_V2f &v2f) {
    V2t<f>::x = v2f.x;
    V2t<f>::y = v2f.y;
  }
};

typedef V2<float> V2f;
typedef V2<double> V2d;

extern "C" {
struct __lc_V3f {
  float x, y, z;
};
}

template <typename f> struct V3t {
  f x, y, z;

  void Set(f nx, f ny, f nz) {
    x = nx;
    y = ny;
    z = nz;
  }

  friend V3t<f> operator-(V3t<f> o) {
    V3t<f> res;
    res.Set(-o.x, -o.y, -o.z);
    return res;
  }

  friend V3t<f> operator+(V3t<f> l, V3t<f> r) {
    V3t<f> res;
    res.Set(l.x + r.x, l.y + r.y, l.z + r.z);
    return res;
  }

  friend V3t<f> operator-(V3t<f> l, V3t<f> r) {
    V3t<f> res;
    res.Set(l.x - r.x, l.y - r.y, l.z - r.z);
    return res;
  }

  friend V3t<f> operator*(V3t<f> l, f r) {
    V3t<f> res;
    res.Set(l.x * r, l.y * r, l.z * r);
    return res;
  }

  friend V3t<f> operator*(f l, V3t<f> r) { return r * l; }

  friend f operator*(V3t<f> l, V3t<f> r) {
    return l.x * r.x + l.y * r.y + l.z * r.z;
  }

  friend V3t<f> operator%(V3t<f> l, V3t<f> r) {
    V3t<f> res;
    res.Set(l.y * r.z - l.z * r.y, l.z * r.x - l.x * r.z,
            l.x * r.y - l.y * r.x);
    return res;
  }

  friend V3t<f> operator/(V3t<f> l, f r) {
    V3t<f> res;
    res.Set(l.x / r, l.y / r, l.z / r);
    return res;
  }

  V3t<f> &Normalize() { return operator/=(Length()); }
  V3t<f> Normalized() { 
      V3t<f> res;
      res.Set(x,y,z);
      return res /= (Length());
  }
  f Length() const { return std::sqrt(x * x + y * y + z * z); }
  V3t<f> &operator+=(V3t<f> r) {
    x += r.x;
    y += r.y;
    z += r.z;
    return *this;
  }
  V3t<f> &operator-=(V3t<f> r) {
    x -= r.x;
    y -= r.y;
    z -= r.z;
    return *this;
  }
  V3t<f> &operator*=(f r) {
    x *= r;
    y *= r;
    z *= r;
    return *this;
  }
  V3t<f> &operator/=(f r) {
    x /= r;
    y /= r;
    z /= r;
    return *this;
  }

  double operator[](int c) {
    if (c == 0)
      return x;
    if (c == 1)
      return y;
    return z;
  }
};

typedef V3t<float> V3tf;
typedef V3t<double> V3td;

template <typename f> struct V3 : V3t<f> {
  V3() {
    V3t<f>::x = static_cast<f>(0.0);
    V3t<f>::y = static_cast<f>(0.0);
    V3t<f>::z = static_cast<f>(0.0);
  }
  V3(f nx, f ny, f nz) {
    V3t<f>::x = nx;
    V3t<f>::y = ny;
    V3t<f>::z = nz;
  }

  V3(const V3tf &src) {
    V3t<f>::x = static_cast<f>(src.x);
    V3t<f>::y = static_cast<f>(src.y);
    V3t<f>::z = static_cast<f>(src.z);
  }
  V3(const V3td &src) {
    V3t<f>::x = static_cast<f>(src.x);
    V3t<f>::y = static_cast<f>(src.y);
    V3t<f>::z = static_cast<f>(src.z);
  }

  V3(const V2tf &src) {
    V3t<f>::x = static_cast<f>(src.x);
    V3t<f>::y = static_cast<f>(src.y);
    V3t<f>::z = static_cast<f>(0.0);
  }

  V3(const V2td &src) {
    V3t<f>::x = static_cast<f>(src.x);
    V3t<f>::y = static_cast<f>(src.y);
    V3t<f>::z = static_cast<f>(0.0);
  }

  template <typename g> explicit V3(const V3t<g> &v) {
    V3t<f>::x = static_cast<f>(v.x);
    V3t<f>::y = static_cast<f>(v.y);
    V3t<f>::z = static_cast<f>(v.z);
  }
  V3(const __lc_V3f &v3f) {
    V3t<f>::x = v3f.x;
    V3t<f>::y = v3f.y;
    V3t<f>::z = v3f.z;
  }
};

typedef V3<float> V3f;
typedef V3<double> V3d;

template <class Vector> Vector Normalize(Vector v) { return v.Normalize(); }

#endif

class SurfaceObj {
public:
  void Set(int id, const float *arr) {
    _v[id].x = arr[0];
    _v[id].y = arr[1];
    _v[id].z = arr[2];
  }
  void Set(int id, const V3f &v) { _v[id] = v; }
  friend SurfaceObj operator*(SurfaceObj s, float r) {
    V3f scale(r, r, r);
    s.Scale(scale);
    return s;
  }
  friend SurfaceObj operator*(float r, const SurfaceObj &obj) {
    return obj * r;
  }
#ifndef NOAUTOOVERLOAD
  friend SurfaceObj operator+(const SurfaceObj &l, const SurfaceObj &r) {
    SurfaceObj res;
    for (int i = 0; i < CtrlPointsCount; ++i)
      res._v[i] = l._v[i] + r._v[i];
    return res;
  }
#endif
  const float *Points() const { return &(_v[0].x); }
  V3f Get(int i) const { return _v[i]; }
  void Scale(V3f scale) {
    for (int i = 0; i < CtrlPointsCount; ++i) {
      _v[i].x *= scale.x;
      _v[i].y *= scale.y;
      _v[i].z *= scale.z;
    }
  }

private:
  enum { CtrlPointsCount = 16 };
  V3tf _v[CtrlPointsCount];
};

template <int __lc_anIntN, int __lc_anIntM> class BsurfaceObjNM {

public:
  friend BsurfaceObjNM operator*(BsurfaceObjNM s, float r) {
    V3f scale(r, r, r);
    s.Scale(scale);
    return s;
  }
  friend BsurfaceObjNM operator*(float r, const BsurfaceObjNM &obj) {
    return obj * r;
  }

  friend V3f **GetControlPoints(const BsurfaceObjNM &l, int &n, int &m) {
    V3f **t;
    t = new V3f *[__lc_anIntN];

    n = l.n_u;
    m = l.n_v;

    for (int i = 0; i < __lc_anIntN; i++) {
      t[i] = new V3f[__lc_anIntM];
      for (int j = 0; j < __lc_anIntM; j++)
        t[i][j] = l._v[i][j];
    }

    return t;
  }

  friend BsurfaceObjNM &ConstructBsurfaceObj(V3f **t, int n, int m,
                                             BsurfaceObjNM &res) {
    res.n_u = n;
    res.n_v = m;

    for (int i = 0; i < res.n_u; i++) {
      for (int j = 0; j < res.n_v; j++)
        res._v[i][j] = t[i][j];
      delete t[i];
    }

    delete t;

    return res;
  }

  void Scale(V3f scale) {
    for (int i = 0; i < __lc_anIntN; ++i) {
      for (int j = 0; j <__lc_anIntM; ++j) {
        _v[i][j].Set(_v[i][j].x * scale.x, _v[i][j].y * scale.y,
                     _v[i][j].z * scale.z);
      }
    }
  }

  const V3f Get(int i, int j) const {
    if (i < __lc_anIntN && j <__lc_anIntM) {
      V3f v(_v[i][j].x, _v[i][j].y, _v[i][j].z);

      return v;
    } else {
      std::cout << "BsurfaceObjNM dimensions exceeded" << std::endl;
      return V3f(0, 0, 0);
    }
  }

  void Set(int i, int j, const V3f &v) {
    if (i < __lc_anIntN && j <__lc_anIntM)
      _v[i][j] = v;
    else
      std::cout << "BsurfaceObjNM dimensions exceeded" << std::endl;
  }

private:
  V3tf _v[__lc_anIntN][__lc_anIntM];
  int n_u;
  int n_v;
};

typedef BsurfaceObjNM<10, 10> BsurfaceObjS;
BsurfaceObjS operator+(const BsurfaceObjS &l, const BsurfaceObjS &r);

typedef BsurfaceObjNM<32, 32> BsurfaceObjM;
BsurfaceObjM operator+(const BsurfaceObjM &l, const BsurfaceObjM &r);

struct MouseStatus {
  int viewNum;
  int viewX, viewY;
  V3td atFront, atRear, atMiddle;
  double selectDepth;
  V3td selectPoint;
  int keypressed;
  bool lbPushed, lbDown, lbReleased;

  MouseStatus(void) : lbPushed(false), lbDown(false), lbReleased(false) {}
};

struct TabletStatus {
  bool connected;

  int viewX, viewY;
  float azimuth, altitude;
  double pressure;
  unsigned int cursorT, buttonState;

  V3d atFront, atRear;

  TabletStatus(void)
      : connected(false), viewX(0), viewY(0), azimuth(0), altitude(0),
        pressure(0), cursorT(0), buttonState(0), atFront(0, 0, 0),
        atRear(0, 0, 0) {}
};

struct CameraPosition {
  V3tf position, lookat;
  V3tf head, left, up;
  float scale;
  // CameraPosition(void) : position(), lookat(), head(), left(), up(),
  // scale(0.0) {}
};

// This is used for toggeling on and off the visibility of patches of terrain
enum VisibilityMode { Shaded, Hidden, Wireframe };
// Used to specify weather the output to POVRay function should output entire
// trees as one large mesh (Instance) (needed for propper instancing), or in a
// more condensed format (Single) which should only be used for trees that are
// used once per scene
enum POVRayMeshMode { Instance, Single };

/* We have to use an encapsulated array in the EA modules instead
 * of a normal array because a normal array would be auto-converted
 * to a pointer.
 */
template <class T, int length> struct __lc_EncapsulatedArray {
  T data[length];
  const T &operator[](int idx) const { return data[idx]; }
  T &operator[](int idx) { return data[idx]; }
  int Length() const { return length; }
  const T *Array() const { return data; }
};

typedef __lc_EncapsulatedArray<float, 20> EA20Array;

typedef void (*pfMessage)(const char *, ...);
typedef void (*pfAdd)(const void *, int);
typedef float (*pfFloatIntFloat)(int, float);
typedef float (*pfFloatIntFloatFloatFloat)(int, float, float, float);
typedef __lc_V2f (*pfV2fIntFloat)(int, float);
typedef __lc_V3f (*pfV3fIntFloat)(int, float);
typedef __lc_V3f (*pvV3fIntInt)(int, int);
typedef void (*pfVoidLong)(long);
typedef void (*pfVoidInt)(int);
typedef void (*pfVoid2Int3Float)(int, int, float, float, float);
typedef void (*pfVoidInt3Float)(int, float, float, float);
typedef void (*pfVoidString)(const char *);
typedef SurfaceObj (*pfSrfcInt)(int);
typedef void (*pfVoidVoid)();
typedef float (*pfFloatInt)(int);
typedef int (*pfIntVoid)();
typedef float (*pfFloatVoid)();
typedef double (*pfDoubleVoid)();
typedef double (*pfDoubleDouble)(double);
typedef void (*pfVoidString)(const char *);
typedef void (*pfVoidStringUInt)(const char *, unsigned int);
typedef MouseStatus (*pfMStat)(void);
typedef TabletStatus (*pfTStat)(void);
typedef CameraPosition (*pfCamPosInt)(int);
typedef char *(*pfCharVoid)(void);
typedef char *(*pfCharInt)(int);

typedef BsurfaceObjS (*pfBSSBinaryOp)(const BsurfaceObjS &,
                                      const BsurfaceObjS &);
typedef BsurfaceObjS (*pfBSSInterp)(const BsurfaceObjS &, const BsurfaceObjS &,
                                    float, int, int);
typedef V3f (*pfBSSGet)(const BsurfaceObjS &, int i, int j);
typedef BsurfaceObjS (*pfBSSSet)(const BsurfaceObjS &, int i, int j, V3f p);
typedef BsurfaceObjS (*pfBSSInt)(int);

typedef BsurfaceObjM (*pfBSMBinaryOp)(const BsurfaceObjM &,
                                      const BsurfaceObjM &);
typedef BsurfaceObjM (*pfBSMInterp)(const BsurfaceObjM &, const BsurfaceObjM &,
                                    float, int, int);
typedef V3f (*pfBSMGet)(const BsurfaceObjM &, int i, int j);
typedef BsurfaceObjM (*pfBSMSet)(const BsurfaceObjM &, int i, int j, V3f p);
typedef BsurfaceObjM (*pfBSMInt)(int);

// For surfaceHeightAt
typedef bool (*pfBoolV3fV3f)(V3f, V3f &);

// For Toggeling visibility of terrain
typedef void (*pfVoidVisibilityMode)(VisibilityMode);
typedef void (*pfVoidVisibilityModeIntV3f)(VisibilityMode, int, V3f);
typedef void (*pfVoidFloat)(float);

// For dealing with parameter files
typedef float (*pfFloatStringFloat)(const char *, float);
typedef int (*pfIntStringInt)(const char *, int);
typedef int (*pfIntString)(const char *);
typedef float (*pfFloatString)(const char *);
typedef void (*pfVoidStringFloat)(const char *, float);
typedef void (*pfVoidStringInt)(const char *, int);

typedef bool (*pfBoolVoid)();
typedef const char *(*pfPCharModuleId)(__lc_ModuleIdType);
typedef bool (*pfBoolModuleId)(__lc_ModuleIdType);

struct __lc_ExportedFromLpfg {
  pfMessage fMessage;
  pfFloatIntFloat fFunc;
  pfFloatIntFloat fTFunc;
  pfFloatInt fVFunc;
  pfFloatIntFloatFloatFloat fPFunc;
  pfV2fIntFloat fFuncTangent;
  pfFloatIntFloat fCurveX;
  pfFloatIntFloat fCurveY;
  pfFloatIntFloat fCurveZ;
  pfVoidInt fCurveReset;
  pfVoid2Int3Float fCurveSetPoint;
  pfVoidInt fCurveRecalc;
  pfV2fIntFloat fCurveV2fPoint;
  pfV3fIntFloat fCurveV3fPoint;
  pfV3fIntFloat fCurveV3fNormal;
  pfVoidInt3Float fCurveScale;
  pfVoidString fRun;
  pfVoidInt fUseView;
  pfVoidInt fCloseView;
  pfSrfcInt fGetSurface;
  pvV3fIntInt fGetSurfacePoint;
  pfVoidVoid fDisplayFrame;
  pfVoidString fOutputFrame;
  pfFloatInt fvvXmin;
  pfFloatInt fvvYmin;
  pfFloatInt fvvZmin;
  pfFloatInt fvvXmax;
  pfFloatInt fvvYmax;
  pfFloatInt fvvZmax;
  pfFloatInt fvvScale;
  pfVoidVoid fStop;
  pfIntVoid fStepNo;
  pfDoubleVoid fGillespieTime;
  pfVoidLong fSeedGillespie;
  pfVoidVoid fResetGillespie;
  pfDoubleDouble fRan;
  pfVoidLong fSeedRan;
  pfVoidString fLoadString;
  pfVoidString fOutputString;
  pfMStat fGetMouseStatus;
  pfTStat fGetTabletStatus;
  pfVoidStringUInt fUserMenuItem;
  pfVoidVoid fUserMenuClear;
  pfIntVoid fUserMenuChoice;
  pfVoidVoid fRunSimulation;
  pfVoidVoid fPauseSimulation;
  pfCamPosInt fGetCameraPosition;

  pfBSSBinaryOp fBsurfaceObjSAdd;
  pfBSSInterp fBsurfaceObjSInterp;
  pfBSSGet fBsurfaceObjSGet;
  pfBSSSet fBsurfaceObjSSet;
  pfBSSInt fGetBsurfaceS;

  pfBSMBinaryOp fBsurfaceObjMAdd;
  pfBSMInterp fBsurfaceObjMInterp;
  pfBSMGet fBsurfaceObjMGet;
  pfBSMSet fBsurfaceObjMSet;
  pfBSMInt fGetBsurfaceM;

  pfBoolV3fV3f fterrainHeightAt;
  pfBoolV3fV3f fterrainNormalAt;
  pfVoidVisibilityMode fterrainVisibilityAll;
  pfVoidVisibilityModeIntV3f fterrainVisibilityPatch;
  pfVoidFloat fscaleTerrainBy;

  pfFloatStringFloat fSetOrGetParameterf;
  pfIntStringInt fSetOrGetParameteri;
  pfBoolVoid fParametersNeedUpdating;
  pfFloatString fGetParameterf;
  pfIntString fGetParameteri;
  pfVoidStringFloat fSetParameterf;
  pfVoidStringInt fSetParameteri;
  pfVoidVoid fDelayWrite;
  pfVoidVoid fWrite;

  pfBoolVoid fLContextReset;
  pfBoolVoid fRContextReset;
  pfBoolVoid fLNContextReset;
  pfBoolVoid fRNContextReset;
  pfBoolModuleId fAcceptContext;
  pfBoolVoid fAdvanceContext;
  pfBoolVoid fConfirmContext;
  pfPCharModuleId fGetModuleAddr;

  pfBoolModuleId fIsConsidered;

  pfCharInt fGetNextModuleSpot;

  pfVoidVoid fStartPerformance;
  pfVoidVoid fStopPerformance;

  pfVoidVoid fPushString;
  pfVoidVoid fPopString;
  pfIntVoid fStoredStringCount;
};

#else
#ifdef WARN_MULTINC
#warning File already included
#endif
#endif
