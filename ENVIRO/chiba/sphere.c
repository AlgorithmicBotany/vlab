/*
 * This is a function by Jon Leech for generating the triangles for a sphere.
 * The code has been basically untouched, except for turning `main' into a
 * function.
 *
 * Mark Hammel
 *
 * sphere - generate a triangle mesh approximating a sphere by
 *  recursive subdivision. First approximation is an octahedron;
 *  each level of refinement increases the number of triangles by
 *  a factor of 4.
 * Level 3 (128 triangles) is a good tradeoff if gouraud
 *  shading is used to render the database.
 *
 * Jon Leech (leech@cs.unc.edu) 3/24/89
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sphere.h"

typedef struct {
    double  x, y, z;
} point;

typedef struct {
    point     pt[3];	/* Vertices of triangle */
    double    area;	/* Unused; might be used for adaptive subdivision */
} triangle;

typedef struct {
    int       npoly;	/* # of triangles in object */
    triangle *poly;	/* Triangles */
} object;

/* Six equidistant points lying on the unit sphere */
#define XPLUS {  1,  0,  0 }	/*  X */
#define XMIN  { -1,  0,  0 }	/* -X */
#define YPLUS {  0,  1,  0 }	/*  Y */
#define YMIN  {  0, -1,  0 }	/* -Y */
#define ZPLUS {  0,  0,  1 }	/*  Z */
#define ZMIN  {  0,  0, -1 }	/* -Z */

/* Vertices of a unit octahedron */
triangle octahedron[] = {
    { { XPLUS, ZPLUS, YPLUS }, 0.0 },
    { { YPLUS, ZPLUS, XMIN  }, 0.0 },
    { { XMIN , ZPLUS, YMIN  }, 0.0 },
    { { YMIN , ZPLUS, XPLUS }, 0.0 },
    { { XPLUS, YPLUS, ZMIN  }, 0.0 },
    { { YPLUS, XMIN , ZMIN  }, 0.0 },
    { { XMIN , YMIN , ZMIN  }, 0.0 },
    { { YMIN , XPLUS, ZMIN  }, 0.0 }
};

/* A unit octahedron */
object oct = {
    sizeof(octahedron) / sizeof(octahedron[0]),
    &octahedron[0]
};

/* local prototypes */
static point *normalize(point *p);
static point *midpoint(point *a, point *b);


void makePolySphere(sphtri *sphere, int resolution)
{
    object *old,
	   *new_;
    int     ccwflag = 1,	/* Reverse vertex order if true */
	    i, j,
	    level,		/* Current subdivision level */
	    maxlevel = 1;	/* Maximum subdivision level */

    maxlevel = resolution;

    if (ccwflag) {
	/* Reverse order of points in each triangle */
	for (i = 0; i < oct.npoly; i++) {
	    point tmp;
			  tmp = oct.poly[i].pt[0];
	    oct.poly[i].pt[0] = oct.poly[i].pt[2];
	    oct.poly[i].pt[2] = tmp;
	}
    }

    old = &oct;

    /* Subdivide each starting triangle (maxlevel - 1) times */
    for (level = 1; level < maxlevel; level++) {
	/* Allocate a new_ object */
	new_ = (object *)malloc(sizeof(object));
	if (new_ == NULL) {
	    fprintf(stderr, "Out of memory on subdivision level %d\n", level);
	    exit(1);
	}
	new_->npoly = old->npoly * 4;

	/* Allocate 4* the number of points in the current approximation */
	new_->poly  = (triangle *)malloc(new_->npoly * sizeof(triangle));
	if (new_->poly == NULL) {
	    fprintf(stderr, "Out of memory on subdivision level %d\n", level);
	    exit(1);
	}

	/* Subdivide each triangle in the old approximation and normalize
	 *  the new points thus generated to lie on the surface of the unit
	 *  sphere.
	 * Each input triangle with vertices labelled [0,1,2] as shown
	 *  below will be turned into four new triangles:
	 *
	 *			Make new points
	 *			    a = (0+2)/2
	 *			    b = (0+1)/2
	 *			    c = (1+2)/2
	 *	  1
	 *	 /\		Normalize a, b, c
	 *	/  \
	 *    b/____\ c		Construct new triangles
	 *    /\    /\		    [0,b,a]
	 *   /	\  /  \		    [b,1,c]
	 *  /____\/____\	    [a,b,c]
	 * 0	  a	2	    [a,c,2]
	 */
	for (i = 0; i < old->npoly; i++) {
	    triangle
		 *oldt = &old->poly[i],
		 *newt = &new_->poly[i*4];
	    point a, b, c;

	    a = *normalize(midpoint(&oldt->pt[0], &oldt->pt[2]));
	    b = *normalize(midpoint(&oldt->pt[0], &oldt->pt[1]));
	    c = *normalize(midpoint(&oldt->pt[1], &oldt->pt[2]));

	    newt->pt[0] = oldt->pt[0];
	    newt->pt[1] = b;
	    newt->pt[2] = a;
	    newt++;

	    newt->pt[0] = b;
	    newt->pt[1] = oldt->pt[1];
	    newt->pt[2] = c;
	    newt++;

	    newt->pt[0] = a;
	    newt->pt[1] = b;
	    newt->pt[2] = c;
	    newt++;

	    newt->pt[0] = a;
	    newt->pt[1] = c;
	    newt->pt[2] = oldt->pt[2];
	}

	if (level > 1) {
	    free(old->poly);
	    free(old);
	}

	/* Continue subdividing new triangles */
	old = new_;
    }

    /* Store the resulting approximation - MSH */
    for (i = 0; i < old->npoly; i++)
      {
	for (j = 0; j < 3; j++)
	  {
	    sphere[i][j][0] = (float)(old->poly[i].pt[j].x);
	    sphere[i][j][1] = (float)(old->poly[i].pt[j].y);
	    sphere[i][j][2] = (float)(old->poly[i].pt[j].z);
	  }
      }

    if (maxlevel > 1) {
      free(old->poly);
      free(old);
    }

}

/* Normalize a point p */
static point *normalize(point *p)
{
    static point r;
    double mag;

    r = *p;
    mag = r.x * r.x + r.y * r.y + r.z * r.z;
    if (mag != 0.0) {
	mag = 1.0 / sqrt(mag);
	r.x *= mag;
	r.y *= mag;
	r.z *= mag;
    }

    return &r;
}

/* Return the midpoint on the line between two points */
static point *midpoint(point *a, point *b)
{
    static point r;

    r.x = (a->x + b->x) * 0.5;
    r.y = (a->y + b->y) * 0.5;
    r.z = (a->z + b->z) * 0.5;

    return &r;
}
