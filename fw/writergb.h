#ifndef __ICON_H__
#define __WRITERGB_H__

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

int WriteIRIS(FILE * fp, 
		  unsigned char* pic,
	      int ptype,
	      int w,
	      int h, 
	      unsigned char * rmap,
	      unsigned char * gmap,
	      unsigned char * bmap,
	      int colorstyle);


#ifdef __cplusplus
}
#endif


#endif
