/***** Ball.h *****/

#ifndef _H_Ball
#define _H_Ball

#include "BallAux.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {long x, y;} Place;

#define RADIUS	  (0.75f)
#define LG_NSEGS 4
#define NSEGS (1<<LG_NSEGS)

extern const HMatrix mId;
extern float otherAxis[][4];
extern float rimColor[3];
extern float farColor[3];
extern float nearColor[3];
extern float dragColor[3];
extern float resColor[3];

#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif

#define EDGES 180

typedef enum AxisSet{NoAxes, CameraAxes, BodyAxes, OtherAxes, NSets} AxisSet;
typedef const float *ConstraintSet;
typedef struct 
{
	HVect center;
	float radius;
	Quat qNow, qDown, qDrag;
	HVect vNow, vDown, vFrom, vTo, vrFrom, vrTo;
	HMatrix mNow, mDown;
	Bool showResult, dragging;
	ConstraintSet sets[NSets];
	int setSizes[NSets];
	AxisSet axisSet;
	int axisIndex;
} BallData;

/* Public routines */
void Ball_Init(BallData *ball);
void Ball_Place(BallData *ball, HVect center, float radius);
void Ball_Mouse(BallData *ball, HVect vNow);
void Ball_UseSet(BallData *ball, AxisSet axisSet);
void Ball_ShowResult(BallData *ball);
void Ball_HideResult(BallData *ball);
void Ball_Update(BallData *ball);
void Ball_Value(BallData *ball, HMatrix mNow);
void Ball_BeginDrag(BallData *ball);
void Ball_EndDrag(BallData *ball);
void Ball_Draw(BallData *ball);
void Ball_DrawCircle();
void Ball_DrawConstraints(BallData *ball);
void Ball_DrawDragArc(BallData *ball);

#ifdef __cplusplus
}
#endif

#endif
