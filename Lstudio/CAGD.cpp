// source file CAGD.cpp


#include <warningset.h>

#include "CAGD.H"

void solve_system(float up[],float low[],float gamma[],int l,float rhs[],float d[])
/*	solve  tridiagonal linear system
	of size (l+1)(l+1) whose LU decompostion has entries up and low,
	and whose right hand side is rhs, and whose original matrix
	had upper diagonal gamma. Solution is d[0],...,d[l+2];
Input: up,low,gamma:  as above.
       l:             size of system: l+1 eqs in l+1 unknowns.
       rhs:           right hand side, i.e, data points with end
                      `tangent Bezier points' in rhs[1] and rhs[l+1].
Output:d:             solution vector.

Note:	Both rhs and d are from 0 to l+2.
*/
{
	int i;
	float aux[100];

	d[0] = rhs[0];
	d[1] = rhs[1];

	/* forward substitution:  */
	aux[0]=rhs[1];
	for(i=1; i<=l; i++) aux[i]=rhs[i+1]-low[i]*aux[i-1];
	
	/* backward substitution:  */
	d[l+1]=aux[l]/up[l];
	for(i=l-1; i>0; i--) d[i+1]=(aux[i]-gamma[i]*d[i+2])/up[i];
	d[l+2]=rhs[l+2];
}


void bessel_ends(float data[],float knot[],int l)
/*	Computes B-spline points data[1] and data[l+]
	according to Bessel end condition.

input:	data:	sequence of data coordinates data[0] to data[l+2].
		Note that data[1] and data[l+1] are expected to
		be empty, as they will be filled by this routine.
	knot:	knot sequence
	l:	number of intervals
 
output:	data:	completed, as above.
*/
{
	float alpha, beta;
	
	if (l==1)
	{/*  This is not really Bessel, but then what do you do
	     when you have only one interval? -- make it linear!
	 */
		data[1]= (2.0f*data[0] + data[3])/3.0f;
		data[2]= (2.0f*data[3] + data[0])/3.0f;
	}
	else if (l==2)
	{
		/* beginning:    */
		alpha= (knot[2]-knot[1])/(knot[2]-knot[0]);
		beta = 1.0f - alpha;

		data[1]=(data[2]-alpha*alpha*data[0]-beta*beta*data[4])
		/(2.0f*alpha*beta);
		data[1]=2.0f*(alpha*data[0]+beta*data[1])/3.0f + data[0]/3.0f;

		/* end:  */
		alpha= (knot[2]-knot[1])/(knot[2]-knot[0]);
		beta = 1.0f - alpha;

		data[3]=(data[2]-alpha*alpha*data[0]-beta*beta*data[4])
		/(2.0f*alpha*beta);

		data[3]=2.0f*(alpha*data[3]+beta*data[4])/3.0f+data[4]/3.0f;	
	}
	else
	{
		/* beginning:    */
		alpha= (knot[2]-knot[1])/(knot[2]-knot[0]);
		beta = 1.0f - alpha;

		data[1]=(data[2]-alpha*alpha*data[0]-beta*beta*data[3])
		/(2.0f*alpha*beta);
		data[1]=2.0f*(alpha*data[0]+beta*data[1])/3.0f + data[0]/3.0f;

		/* end:  */
		alpha= (knot[l]-knot[l-1])/(knot[l]-knot[l-2]);
		beta = 1.0f - alpha;

		data[l+1]=(data[l]-alpha*alpha*data[l-1]-beta*beta*data[l+2])
		/(2.0f*alpha*beta);

		data[l+1]=2.0f*(alpha*data[l+1]+beta*data[l+2])/3.0f+data[l+2]/3.0f;
	}
}


void l_u_system(float alpha[],float beta[],float gamma[],int l,float up[],float low[])
/*	perform LU decomposition of tridiagonal system with
	lower diagonal alpha, diagonal beta, upper diagonal gamma.

input:	alpha,beta,gamma: the coefficient matrix entries
	l:	matrix size [0,l]x[0,l]
	low:	L-matrix entries
	up:	U-matrix entries
*/
{
	int i;

	up[0]=beta[0];
	for(i=1; i<=l; i++)
	{
		low[i]=alpha[i]/up[i-1];
		up[i] =beta[i]-low[i]*gamma[i-1];
	}
}


void set_up_system(float knot[],int l,float alpha[],float beta[],float gamma[])
/*	given the knot sequence, the linear system 
	for clamped end condition B-spline interpolation
	is set up.
input:	knot:	knot sequence
	points:	points to be interpolated
	l:	number of intervals
output:	alpha,beta,gamma: 1-D arrays that constitute
		the elements of the interpolation matrix.

	Note: no data points needed so far! 
*/
{
	int i,l1;
	float delta_im2,delta_im1, delta_i, delta_ip1,sum;

	l1=l-1;

	/* some special cases: */
	if(l==1)
	{alpha[0]=0.0; alpha[1]=0.0; beta[0]=1.0; beta[1]=1.0;
	 gamma[0]=0.0; gamma[1]=0.0; return;}

	if(l==2)
	{
		beta[0]=1.0;
		delta_im1=(knot[1]-knot[0]);
		delta_i  =(knot[2]-knot[1]);
		delta_ip1=(knot[3]-knot[2]);
		sum = delta_im1+delta_i;

		alpha[1]=delta_i*delta_i/sum;
		beta[1] =(delta_i*delta_im1)/sum
                + delta_im1*delta_i/ sum;
		gamma[1]=delta_im1*delta_im1/sum;

		alpha[1]=alpha[1]/sum;
		beta[1] =beta[1]/sum;
		gamma[1]=gamma[1]/sum;

		beta[2]=1.0;
		alpha[2]=0.0;
		gamma[2]=0.0;
		return;
	}

	/* the rest does the cases l>2.  */

	delta_im1=(knot[1]-knot[0]);
	delta_i  =(knot[2]-knot[1]);
	delta_ip1=(knot[3]-knot[2]);
	sum = delta_im1+delta_i;

	beta[0]=1.0; gamma[0]=0.0;

	alpha[1]=delta_i*delta_i/sum;
	beta[1] =(delta_i*delta_im1)/sum
                + delta_im1*(delta_i+delta_ip1)
		/ (sum+delta_ip1);
	gamma[1]=delta_im1*delta_im1/(sum+delta_ip1);

	alpha[1]=alpha[1]/sum;
	beta[1] =beta[1]/sum;
	gamma[1]=gamma[1]/sum;

	/*Now for the main loop:   */
	for(i=2; i<l1; i++)
	{
		/* compute delta_i_minus_2,...  */
		delta_im2=(knot[i-1]-knot[i-2]);
		delta_im1=(knot[i]  -knot[i-1]);
		delta_i  =(knot[i+1]-knot[i]);
		delta_ip1=(knot[i+2]-knot[i+1]);
	
		sum = delta_im1+delta_i;

		alpha[i]=delta_i*delta_i/(delta_im2 +sum);
		beta[i] =delta_i*(delta_im2+delta_im1) 
			/(delta_im2 + sum)
			+
			 delta_im1*(delta_i+delta_ip1)
			/(sum + delta_ip1);
		gamma[i]=delta_im1*delta_im1
			/(sum + delta_ip1);
		
		alpha[i]=alpha[i]/sum;
		beta[i] =beta[i]/sum;
		gamma[i]=gamma[i]/sum;
	}

	/*  special care at the end:  */
	delta_im2=knot[l-2]-knot[l-3];
	delta_im1=knot[l1]-knot[l-2];
	delta_i  =knot[l]-knot[l1];
	sum=delta_im1+delta_i;

	alpha[l1]=delta_i*delta_i/(delta_im2+sum);
	beta[l1] =delta_i*(delta_im2+delta_im1)/(delta_im2  + sum)
                 +
		  delta_im1*delta_i / sum;
	gamma[l1]=delta_im1*delta_im1/sum;

	alpha[l1]=alpha[l1]/sum;
	beta[l1] =beta[l1]/sum;
	gamma[l1]=gamma[l1]/sum;


	alpha[l]=0.0; beta[l]=1.0; gamma[l]=0.0;
}


void c2_spline(float knot[],int l,float data_x[],float data_y[],float bspl_x[],float bspl_y[])
/* Finds the C2 cubic spline interpolant to
the data points in data_x, data_y.
Input:	knot:	the knot sequence  knot[0], ..., knot[l]
	l:	the number of intervals
	data_x, data_y: the data points data_x[0], ...,
		data[l+2]. Attention:
		data_x[1] and data_x[l+1] are filled
		by  Bessel end conditions and are
		thus ignored on input. Same for data_y.
Output:	bspl_x, bspl_y: the B-spline control polygon of
		the interpolant. Dimensions:
		bspl_x[0], ..., bspl_x[l+2]. Same for
		bspl_y.
*/
{
	float alpha[100], beta[100], gamma[100], up[100], low[100];
	set_up_system(knot,l,alpha,beta,gamma);

	l_u_system(alpha,beta,gamma,l,up,low);

	bessel_ends(data_x,knot,l);
	bessel_ends(data_y,knot,l);

	solve_system(up,low,gamma,l,data_x,bspl_x);
	solve_system(up,low,gamma,l,data_y,bspl_y);
}
