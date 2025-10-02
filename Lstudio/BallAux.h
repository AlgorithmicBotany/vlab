/***** BallAux.h - Vector and quaternion routines for Arcball. *****/

#ifndef _H_BallAux
#define _H_BallAux

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int Bool;
typedef struct {float x, y, z, w;} Quat;
enum QuatPart {qpX, qpY, qpZ, qpW, QuatLen};
typedef Quat HVect;
extern Quat qOne;
typedef float HMatrix[QuatLen][QuatLen];

extern Quat qOne;
void Qt_ToMatrix(Quat q, HMatrix out);
Quat Qt_Conj(Quat q);
Quat Qt_Mul(Quat qL, Quat qR);
HVect V3_(float x, float y, float z);
float V3_Norm(HVect v);
HVect V3_Unit(HVect v);
HVect V3_Scale(HVect v, float s);
HVect V3_Negate(HVect v);
HVect V3_Sub(HVect v1, HVect v2);
float V3_Dot(HVect v1, HVect v2);
HVect V3_Cross(HVect v1, HVect v2);
HVect V3_Bisect(HVect v0, HVect v1);

#ifdef __cplusplus
}
#endif

#endif
