#ifndef __RANDFUNCS_H__
#define __RANDFUNCS_H__


struct drand48_data
{
	unsigned short int x[3];
	unsigned short int a[3];
	unsigned short int c;
	unsigned short int old_x[3];
	int init;
};


int __erand48_r (unsigned short int xsubi[3],
     struct drand48_data *buffer,
     double *result);

int __drand48_iterate (unsigned short int xsubi[3],
     struct drand48_data *buffer);

int __jrand48_r (unsigned short int xsubi[3],
     struct drand48_data *buffer,
     long int *result);

int __lcong48_r (unsigned short int param[7],
     struct drand48_data *buffer);

int __nrand48_r (unsigned short int xsubi[3],
     struct drand48_data *buffer,
     long int *result);

int __seed48_r (unsigned short int seed16v[3],
     struct drand48_data *buffer);

int __srand48_r (long int seedval,
     struct drand48_data *buffer);

typedef unsigned __int64 u_int64_t;



union ieee754_double
{
	double d;
	struct
	{
		unsigned int mantissa1:32;
		unsigned int mantissa0:20;
		unsigned int exponent:11;
		unsigned int negative:1;
	} ieee;
	struct
	{
		unsigned int mantissa1:32;
		unsigned int mantissa0:19;
		unsigned int quiet_nan:1;
		unsigned int exponent:11;
		unsigned int negative:1;
	} ieee_nan;
};


#define IEEE754_DOUBLE_BIAS 0x3ff;


#else
	#error File already included
#endif
