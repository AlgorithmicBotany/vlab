/***** BallMath.h - Essential routines for Arcball.  *****/

#ifndef _H_BallMath
#define _H_BallMath

#include "BallAux.h"

#ifdef __cplusplus
extern "C" {
#endif

HVect MouseOnSphere(HVect mouse, HVect ballCenter, float ballRadius);
HVect ConstrainToAxis(HVect loose, HVect axis);
int NearestConstraintAxis(HVect loose, HVect *axes, int nAxes);
Quat Qt_FromBallPoints(HVect from, HVect to);
void Qt_ToBallPoints(Quat q, HVect *arcFrom, HVect *arcTo);

#ifdef __cplusplus
}
#endif

#endif
