/***** BallAux.c *****/

#include <math.h>

#include <warningset.h>

#include "BallAux.h"

Quat qOne = {0, 0, 0, 1};

/* Return quaternion product qL * qR.  Note: order is important!
 * To combine rotations, use the product Mul(qSecond, qFirst),
 * which gives the effect of rotating by qFirst then qSecond. */
Quat Qt_Mul(Quat qL, Quat qR)
{
	Quat qq;
	qq.w = qL.w*qR.w - qL.x*qR.x - qL.y*qR.y - qL.z*qR.z;
	qq.x = qL.w*qR.x + qL.x*qR.w + qL.y*qR.z - qL.z*qR.y;
	qq.y = qL.w*qR.y + qL.y*qR.w + qL.z*qR.x - qL.x*qR.z;
	qq.z = qL.w*qR.z + qL.z*qR.w + qL.x*qR.y - qL.y*qR.x;
	return (qq);
}

/* Construct rotation matrix from (possibly non-unit) quaternion.
 * Assumes matrix is used to multiply column vector on the left:
 * vnew = mat vold.  Works correctly for right-handed coordinate system
 * and right-handed rotations. */
/*
HMatrix *Qt_ToMatrix(Quat q, HMatrix out)
*/
void Qt_ToMatrix(Quat q, HMatrix out)
{
	const float Nq = q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w;
	const float s = (Nq > 0.0f) ? (2.0f / Nq) : 0.0f;
	const float xs = q.x*s,		  ys = q.y*s,	  zs = q.z*s;
	const float wx = q.w*xs, 	  wy = q.w*ys,	  wz = q.w*zs;
	const float xx = q.x*xs, 	  xy = q.x*ys,	  xz = q.x*zs;
	const float yy = q.y*ys, 	  yz = q.y*zs,	  zz = q.z*zs;
	out[qpX][qpX] = 1.0f - (yy + zz); out[qpY][qpX] = xy + wz; out[qpZ][qpX] = xz - wy;
	out[qpX][qpY] = xy - wz; out[qpY][qpY] = 1.0f - (xx + zz); out[qpZ][qpY] = yz + wx;
	out[qpX][qpZ] = xz + wy; out[qpY][qpZ] = yz - wx; out[qpZ][qpZ] = 1.0f - (xx + yy);
	out[qpX][qpW] = out[qpY][qpW] = out[qpZ][qpW] = out[qpW][qpX] = out[qpW][qpY] = out[qpW][qpZ] = 0.0f;
	out[qpW][qpW] = 1.0f;
}

/* Return conjugate of quaternion. */
Quat Qt_Conj(Quat q)
{
	Quat qq;
	qq.x = -q.x; qq.y = -q.y; qq.z = -q.z; qq.w = q.w;
	return (qq);
}

/* Return vector formed from components */
HVect V3_(float x, float y, float z)
{
	HVect v;
	v.x = x; v.y = y; v.z = z; v.w = 0;
	return (v);
}

/* Return norm of v, defined as sum of squares of components */
float V3_Norm(HVect v)
{
	return ( v.x*v.x + v.y*v.y + v.z*v.z );
}

/* Return unit magnitude vector in direction of v */
HVect V3_Unit(HVect v)
{
	static HVect u = {0.0f, 0.0f, 0.0f, 0.0f};
	float vlen = (float)sqrt(V3_Norm(v));
	if (vlen != 0.0f) 
	{
		u.x = v.x/vlen; u.y = v.y/vlen; u.z = v.z/vlen;
	}
	return (u);
}

/* Return version of v scaled by s */
HVect V3_Scale(HVect v, float s)
{
	HVect u;
	u.x = s*v.x; u.y = s*v.y; u.z = s*v.z; u.w = v.w;
	return (u);
}

/* Return negative of v */
HVect V3_Negate(HVect v)
{
	static HVect u = {0.0f, 0.0f, 0.0f, 0.0f};
	u.x = -v.x; u.y = -v.y; u.z = -v.z;
	return (u);
}

/* Return sum of v1 and v2 */
HVect V3_Add(HVect v1, HVect v2)
{
	static HVect v = {0.0f, 0.0f, 0.0f, 0.0f};
	v.x = v1.x+v2.x; v.y = v1.y+v2.y; v.z = v1.z+v2.z;
	return (v);
}

/* Return difference of v1 minus v2 */
HVect V3_Sub(HVect v1, HVect v2)
{
	static HVect v = {0.0f, 0.0f, 0.0f, 0.0f};
	v.x = v1.x-v2.x; v.y = v1.y-v2.y; v.z = v1.z-v2.z;
	return (v);
}

/* Halve arc between unit vectors v0 and v1. */
HVect V3_Bisect(HVect v0, HVect v1)
{
    HVect v = {0.0f, 0.0f, 0.0f, 0.0f};
    float Nv;
    v = V3_Add(v0, v1);
    Nv = V3_Norm(v);
    if (Nv < 1.0e-5f) 
	{
		v = V3_(0.0f, 0.0f, 1.0f);
    } 
	else
	{
		v = V3_Scale(v, 1.0f/(float)sqrt(Nv));
    }
    return (v);
}

/* Return dot product of v1 and v2 */
float V3_Dot(HVect v1, HVect v2)
{
	return (v1.x*v2.x + v1.y*v2.y + v1.z*v2.z);
}

/* Return cross product, v1 x v2 */
HVect V3_Cross(HVect v1, HVect v2)
{
	static HVect v = {0.0f, 0.0f, 0.0f, 0.0f};
	v.x = v1.y*v2.z-v1.z*v2.y;
	v.y = v1.z*v2.x-v1.x*v2.z;
	v.z = v1.x*v2.y-v1.y*v2.x;
	return (v);
}
