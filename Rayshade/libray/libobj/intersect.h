#ifndef INTERSECT_H
#define INTERSECT_H
int intersect( Geom *obj, /* Geom to be tested. */
	       Ray *ray,                         /* Ray origin, direction. */
	       HitList *hitlist,                 /* Intersection path */
	       Float mindist, Float *maxdist);

#endif
