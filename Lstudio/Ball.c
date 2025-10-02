/***** Ball.c *****/
/* Ken Shoemake, 1993 */

#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#include <warningset.h>

#include "Ball.h"
#include "BallMath.h"

const HMatrix mId = {{1.0f,0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f,0.0f},{0.0f,0.0f,1.0f,0.0f},{0.0f,0.0f,0.0f,1.0f}};
float otherAxis[][4] = {{-0.48f, 0.80f, 0.36f, 1.0f}};
float rimColor[3] = {1.0f, 1.0f, 1.0f};
float farColor[3] = {0.8f, 0.5f, 0.1f};
float nearColor[3] = {1.0f, 1.0f, 0.2f};
float dragColor[3] = {0.5f, 1.0f, 1.0f};
float resColor[3] = {0.8f, 0.1f, 0.1f};


static void DrawCircle();
static void DrawAnyArc(HVect vFrom, HVect vTo);
static void DrawHalfArc(HVect n);
static void Ball_DrawResultArc(BallData *ball);

/* Establish reasonable initial values for controller. */
void Ball_Init(BallData *ball)
{
	int i;
	ball->center = qOne;
	ball->radius = 1.0;
	ball->vDown = ball->vNow = qOne;
	ball->qDown = ball->qNow = qOne;
	for (i=15; i>=0; i--)
		((float *)ball->mNow)[i] = ((float *)ball->mDown)[i] = ((float *)mId)[i];
	ball->showResult = ball->dragging = FALSE;
	ball->axisSet = NoAxes;
	ball->sets[CameraAxes] = mId[qpX]; ball->setSizes[CameraAxes] = 3;
	ball->sets[BodyAxes] = ball->mDown[qpX]; ball->setSizes[BodyAxes] = 3;
	ball->sets[OtherAxes] = otherAxis[qpX]; ball->setSizes[OtherAxes] = 1;
}

/* Set the center and size of the controller. */
void Ball_Place(BallData *ball, HVect center, float radius)
{
	ball->center = center;
	ball->radius = radius;
}

/* Incorporate new mouse position. */
void Ball_Mouse(BallData *ball, HVect vNow)
{
	ball->vNow = vNow;
}

/* Choose a constraint set, or none. */
void Ball_UseSet(BallData *ball, AxisSet axisSet)
{
	if (!ball->dragging)
		ball->axisSet = axisSet;
}

/* Begin drawing arc for all drags combined. */
void Ball_ShowResult(BallData *ball)
{
	ball->showResult = TRUE;
}

/* Stop drawing arc for all drags combined. */

/* Using vDown, vNow, dragging, and axisSet, compute rotation etc. */
void Ball_Update(BallData *ball)
{
	int setSize = ball->setSizes[ball->axisSet];
	HVect *set = (HVect *)(ball->sets[ball->axisSet]);
	ball->vFrom = MouseOnSphere(ball->vDown, ball->center, ball->radius);
	ball->vTo = MouseOnSphere(ball->vNow, ball->center, ball->radius);
	if (ball->dragging)
	{
		if (ball->axisSet!=NoAxes) 
		{
			ball->vFrom = ConstrainToAxis(ball->vFrom, set[ball->axisIndex]);
			ball->vTo = ConstrainToAxis(ball->vTo, set[ball->axisIndex]);
		}
		ball->qDrag = Qt_FromBallPoints(ball->vFrom, ball->vTo);
		ball->qNow = Qt_Mul(ball->qDrag, ball->qDown);
	}
	else
	{
		if (ball->axisSet!=NoAxes)
		{
			ball->axisIndex = NearestConstraintAxis(ball->vTo, set, setSize);
		}
	}
	Qt_ToBallPoints(ball->qDown, &ball->vrFrom, &ball->vrTo);
	Qt_ToMatrix(Qt_Conj(ball->qNow), ball->mNow); /* Gives transpose for GL. */
}

/* Return rotation matrix defined by controller use. */
void Ball_Value(BallData *ball, HMatrix mNow)
{
	int i;
	for (i=15; i>=0; i--) 
		((float *)mNow)[i] = ((float *)ball->mNow)[i];
}

/* Begin drag sequence. */
void Ball_BeginDrag(BallData *ball)
{
	ball->dragging = TRUE;
	ball->vDown = ball->vNow;
}

/* Stop drag sequence. */
void Ball_EndDrag(BallData *ball)
{
	int i;
	ball->dragging = FALSE;
	ball->qDown = ball->qNow;
	for (i=15; i>=0; i--)
		((float *)ball->mDown)[i] = ((float *)ball->mNow)[i];
}

/* Draw circle. */
void DrawCircle()
{
	int i;
	
	glBegin(GL_LINE_LOOP);	  
	for( i = 1; i < EDGES; i++ )
	{
		if ( i == EDGES - 1 )
		{
			glVertex2f((float)cos(0.0f), (float)sin(0.0f));
			glVertex2f((float)cos(0.0f), (float)sin(0.0f));
		}
		else
		{
			glVertex2f((float)cos((2.0f*M_PIf*i)/EDGES), (float)sin((2.0f*M_PIf*i)/EDGES));
			glVertex2f((float)cos((2.0f*M_PIf*i+1)/EDGES), (float)sin((2.0f*M_PIf*i+1)/EDGES));
		}
	}
	glEnd();
}

/* Draw the controller with all its arcs. */
void Ball_Draw(BallData *ball)
{
	const float r = ball->radius;
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glScalef(r, r, r);
	glColor3fv(rimColor);
	DrawCircle();
	Ball_DrawResultArc(ball);
	Ball_DrawConstraints(ball);
	Ball_DrawDragArc(ball);
	glPopMatrix();
}

/* Draw an arc defined by its ends. */
void DrawAnyArc(HVect vFrom, HVect vTo)
{
	int i;
	HVect pts[NSEGS+1];
	float dot;
	pts[0] = vFrom;
	pts[1] = pts[NSEGS] = vTo;
	for (i=0; i<LG_NSEGS; i++) 
		pts[1] = V3_Bisect(pts[0], pts[1]);
	dot = 2.0f*V3_Dot(pts[0], pts[1]);
	for (i=2; i<NSEGS; i++) 
	{
		pts[i] = V3_Sub(V3_Scale(pts[i-1], dot), pts[i-2]);
	}
	glBegin(GL_LINE_STRIP);
	for (i=0; i<=NSEGS; i++)
	{
		glVertex3f(pts[i].x, pts[i].y, pts[i].z);
	}
	glEnd();
}

/* Draw the arc of a semi-circle defined by its axis. */
void DrawHalfArc(HVect n)
{
	HVect p, m;
	p.z = 0;
	if (n.z != 1.0) 
	{
		p.x = n.y; p.y = -n.x;
		p = V3_Unit(p);
	}
	else 
	{
		p.x = 0; p.y = 1;
	}
	m = V3_Cross(p, n);
	DrawAnyArc(p, m);
	DrawAnyArc(m, V3_Negate(p));
}

/* Draw circle */
void Ball_DrawCircle()
{
	glColor3f(rimColor[0], rimColor[1], rimColor[2]);
	DrawCircle();
}

/* Draw all constraint arcs. */
void Ball_DrawConstraints(BallData *ball)
{
	ConstraintSet set;
	HVect axis;
	int axisI, setSize = ball->setSizes[ball->axisSet];
	if (ball->axisSet==NoAxes) 
		return;
	set = ball->sets[ball->axisSet];
	for (axisI=0; axisI<setSize; axisI++) 
	{
		if (ball->axisIndex!=axisI) 
		{
			if (ball->dragging) 
				continue;
			glColor3fv(farColor);
		} 
		else 
			glColor3fv(nearColor);
		axis = *(HVect *)&set[4*axisI];
		if (axis.z==1.0f) 
		{
			DrawCircle();
		}
		else 
		{
			DrawHalfArc(axis);
		}
	}
}

/* Draw "rubber band" arc during dragging. */
void Ball_DrawDragArc(BallData *ball)
{
	glColor3fv(dragColor);
	if (ball->dragging)
		DrawAnyArc(ball->vFrom, ball->vTo);
}

/* Draw arc for result of all drags. */
void Ball_DrawResultArc(BallData *ball)
{
	glColor3fv(resColor);
	if (ball->showResult) 
		DrawAnyArc(ball->vrFrom, ball->vrTo);
}
