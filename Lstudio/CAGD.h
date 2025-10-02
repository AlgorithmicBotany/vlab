// header file curveXYZ.h

#ifndef __CAGD_H__
#define __CAGD_H__

#ifdef __cplusplus
extern "C" {
#endif

void solve_system(float up[],float low[],float gamma[],int l,float rhs[],float d[]);
void bessel_ends(float data[],float knot[],int l);
void l_u_system(float alpha[],float beta[],float gamma[],int l,float up[],float low[]);
void set_up_system(float knot[],int l,float alpha[],float beta[],float gamma[]);
void c2_spline(float knot[],int l,float data_x[],float data_y[],float bspl_x[],float bspl_y[]);

#ifdef __cplusplus
}
#endif

#endif
