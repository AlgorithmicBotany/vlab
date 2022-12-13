#ifndef __RAND_H__
#define __RAND_H__

#ifdef __cplusplus
extern "C" {
#endif

void srand48(long);
double drand48(void);
long lrand48(void);
double erand48 (unsigned short int xsubi[3]);
long int nrand48 (unsigned short int xsubi[3]);

#ifdef __cplusplus
}
#endif


#else
//	#error File already included
#endif
