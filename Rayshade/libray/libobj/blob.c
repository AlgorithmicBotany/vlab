/*
 * blob.c
 *
 * Copyright (C) 1990, 1991, Mark Podlipec, Craig E. Kolb
 * All rights reserved.
 *
 * This software may be freely copied, modified, and redistributed
 * provided that this copyright notice is preserved on all copies.
 *
 * You may not distribute this software, in whole or in part, as part of
 * any commercial product without the express consent of the authors.
 *
 * There is no warranty or other guarantee of fitness of this software
 * for any purpose.  It is provided solely "as is".
 *
 *
 */
#include "geom.h"
#include "blob.h"
#include "roots.h"
static Methods *iBlobMethods = NULL;
static char blobName[] = "blob";

unsigned long BlobTests, BlobHits;

/*
 * Blob/Metaball Description
 *
 * In this implementation a Blob is made up of a threshold T, and a
 * group of 1 or more Metaballs.  Each metaball 'i'  is defined by
 * its position (xi,yi,zi), its radius ri, and its strength si.
 * Around each Metaball is a density function di(Ri) defined by:
 *
 *         di(Ri) =  c4i * Ri^4  +  c2i * Ri^2  + c0i     0 <= Ri <= ri
 *         di(Ri) =  0                                    ri < Ri
 *
 * where
 *       c4i  = si / (ri^4)
 *       c2i  = -(2 * si) / (ri^2)
 *       c0i  = si
 *       Ri^2 is the distance from a point (x,y,z) to the center of
 *            the Metaball - (xi,yi,zi).
 *
 * The density function looks sort of like a W (Float-u) with the
 * center hump crossing the d-axis at  si  and the two humps on the
 * side being tangent to the R-axis at  -ri  and  +ri. Only the
 * range  [0,ri] is being used.
 * I chose this function so that derivative = 0  at the points  0 and +ri.
 * This keeps the surface smooth when the densities are added. I also
 * wanted no  Ri^3  and  Ri  terms, since the equations are easier to
 * solve without them. This led to the symmetry about the d-axis and
 * the derivative being equal to zero at -ri as well.
 *
 * The surface of the Blob is defined by:
 *
 *
 *                  N
 *                 ---
 *      F(x,y,z) = \    di(Ri)  = T
 *                 /
 *                 ---
 *                 i=0
 *
 * where
 *
 *     di(Ri)    is   given above
 *     Ri^2      =    (x-xi)^2  +  (y-yi)^2  + (z-zi)^2
 *      N        is   number of Metaballs in Blob
 *      T        is   the threshold
 *    (xi,yi,zi) is   the center of Metaball i
 *
 */

/*****************************************************************************
 * Create & return reference to a metaball.
 */
Blob *BlobCreate(T, mlist, npoints) Float T;
MetaList *mlist;
int npoints;
{
  Blob *blob;
  int i;
  MetaList *cur;

  /*
   * There has to be at least one metaball in the blob.
   * Note: if there's only one metaball, the blob
   * will be a sphere.
   */
  if (npoints < 1) {
    RLerror(RL_WARN, "blob field not correct.\n");
    return (Blob *)NULL;
  }

  /*
   * Initialize primitive and Geom structures
   */
  blob = (Blob *)Malloc(sizeof(Blob));
  blob->T = T;
  blob->list = (MetaVector *)Malloc((unsigned)(npoints * sizeof(MetaVector)));
  blob->num = npoints;

  /*
   * Initialize Metaball list
   */
  for (i = 0; i < npoints; i++) {
    cur = mlist;
    if ((cur->mvec.c0 < EPSILON) || (cur->mvec.rs < EPSILON)) {
      RLerror(RL_WARN, "Degenerate metaball in blob (sr).\n");
      return (Blob *)NULL;
    }
    /* store radius squared */
    blob->list[i].rs = cur->mvec.rs * cur->mvec.rs;
    /* Calculate and store coefficients for each metaball */
    blob->list[i].c0 = cur->mvec.c0;
    blob->list[i].c2 = -(2.0 * cur->mvec.c0) / blob->list[i].rs;
    blob->list[i].c4 = cur->mvec.c0 / (blob->list[i].rs * blob->list[i].rs);
    blob->list[i].x = cur->mvec.x;
    blob->list[i].y = cur->mvec.y;
    blob->list[i].z = cur->mvec.z;
    mlist = mlist->next;
    free((voidstar)cur);
  }
  /*
   * Allocate room for Intersection Structures and
   * Allocate room for an array of pointers to point to
   * the Intersection Structures.
   */
  blob->ilist = (MetaInt *)Malloc(2 * blob->num * sizeof(MetaInt));
  blob->iarr = (MetaInt **)Malloc(2 * blob->num * sizeof(MetaInt *));
  return blob;
}

Methods *BlobMethods() {
  if (iBlobMethods == (Methods *)NULL) {
    iBlobMethods = MethodsCreate();
    iBlobMethods->create = (GeomCreateFunc *)BlobCreate;
    iBlobMethods->methods = BlobMethods;
    iBlobMethods->name = BlobName;
    iBlobMethods->intersect = BlobIntersect;
    iBlobMethods->normal = BlobNormal;
    iBlobMethods->bounds = BlobBounds;
    iBlobMethods->stats = BlobStats;
    iBlobMethods->checkbounds = TRUE;
    iBlobMethods->closed = TRUE;
  }
  return iBlobMethods;
}

/*****************************************************************************
 * Function used by qsort() when sorting the Ray/Blob intersection list
 */
int MetaCompare(A, B) char *A, *B;
{
  MetaInt **AA, **BB;

  AA = (MetaInt **)A;
  BB = (MetaInt **)B;
  if (AA[0]->bound == BB[0]->bound)
    return (0);
  if (AA[0]->bound < BB[0]->bound)
    return (-1);
  return (1); /* AA[0]->bound is > BB[0]->bound */
}

/*****************************************************************************
 * Ray/metaball intersection test.
 */
int BlobIntersect(blob, ray, mindist, maxdist) Blob *blob;
Ray *ray;
Float mindist, *maxdist;
{
  double c[5], s[4];
  Float dist;
  MetaInt *ilist, **iarr;
  register int i, j, inum;
  extern void qsort();

  BlobTests++;

  ilist = blob->ilist;
  iarr = blob->iarr;

  /*
   * The first step in calculating the Ray/Blob intersection is to
   * divide the Ray into intervals such that only a fixed set of
   * Metaballs contribute to the density function along that interval.
   * This is done by finding the set of intersections between the Ray
   * and each Metaball's Sphere/Region of influence, which has a
   * radius  ri  and is centered at (xi,yi,zi).
   * Intersection information is kept track of in the MetaInt
   * structure and consists of:
   *
   *   type    indicates whether this intersection is the start(R_START)
   *           of a Region or the end(R_END) of one.
   *   pnt     the Metaball of this intersection
   *   bound   the distance from Ray origin to this intersection
   *
   * This list is then sorted by  bound  and used later to find the Ray/Blob
   * intersection.
   */

  inum = 0;
  for (i = 0; i < blob->num; i++) {
    register Float xadj, yadj, zadj;
    register Float b, t, rs;
    register Float dmin, dmax;

    rs = blob->list[i].rs;
    xadj = blob->list[i].x - ray->pos.x;
    yadj = blob->list[i].y - ray->pos.y;
    zadj = blob->list[i].z - ray->pos.z;

    /*
     * Ray/Sphere of Influence intersection
     */
    b = xadj * ray->dir.x + yadj * ray->dir.y + zadj * ray->dir.z;
    t = b * b - xadj * xadj - yadj * yadj - zadj * zadj + rs;

    /*
     * don't except imaginary or single roots. A single root is a ray
     * tangent to the Metaball's Sphere/Region. The Metaball's
     * contribution to the overall density function at this point is
     * zero anyway.
     */
    if (t > 0.0) /* we have two roots */
    {
      t = sqrt(t);
      dmin = b - t;
      /*
       * only interested in stuff in front of ray origin
       */
      if (dmin < mindist)
        dmin = mindist;
      dmax = b + t;
      if (dmax > dmin) /* we have a valid Region */
      {
        /*
         * Initialize min/start and max/end Intersections Structures
         * for this Metaball
         */
        ilist[inum].type = R_START;
        ilist[inum].pnt = i;
        ilist[inum].bound = dmin;
        for (j = 0; j < 5; j++)
          ilist[inum].c[j] = 0.0;
        iarr[inum] = &(ilist[inum]);
        inum++;

        ilist[inum].type = R_END;
        ilist[inum].pnt = i;
        ilist[inum].bound = dmax;
        for (j = 0; j < 5; j++)
          ilist[inum].c[j] = 0.0;
        iarr[inum] = &(ilist[inum]);
        inum++;
      } /* End of valid Region */
    }   /* End of two roots */
  }     /* End of loop through metaballs */

  /*
   * If there are no Ray/Metaball intersections there will
   * not be a Ray/Blob intersection. Exit now.
   */
  if (inum == 0) {
    return FALSE;
  }

  /*
   * Sort Intersection list. No sense using qsort if there's only
   * two intersections.
   *
   * Note: we actually aren't sorting the Intersection structures, but
   * an array of pointers to the Intersection structures.
   * This is faster than sorting the Intersection structures themselves.
   */
  if (inum == 2) {
    MetaInt *t;
    if (ilist[0].bound > ilist[1].bound) {
      t = iarr[0];
      iarr[0] = iarr[1];
      iarr[1] = t;
    }
  } else
    qsort((voidstar)(iarr), (unsigned)inum, sizeof(MetaInt *), MetaCompare);

  /*
   * Finding the Ray/Blob Intersection
   *
   * The non-zero part of the density function for each Metaball is
   *
   *   di(Ri) = c4i * Ri^4  +  c2i * Ri^2  +  c0i
   *
   * To find find the Ray/Blob intersection for one metaball
   * substitute for distance
   *
   *     Ri^2 = (x-xi)^2 + (y-yi)^2 + (z-zi)^2
   *
   * and then substitute the Ray equations:
   *
   *     x  = x0 + x1 * t
   *     y  = y0 + y1 * t
   *     z  = z0 + z1 * t
   *
   * to get a big mess :^). Actually, it's a Quartic in t and it's fully
   * listed farther down. Here's a short version:
   *
   *   c[4] * t^4  +  c[3] * t^3  +  c[2] * t^2  +  c[1] * t  +  c[0]  =  T
   *
   * Don't forget that the Blob is defined by the density being equal to
   * the threshold T.
   * To calculate the intersection of a Ray and two or more Metaballs,
   * the coefficients are calculated for each Metaball and then added
   * together. We can do this since we're working with polynomials.
   * The points of intersection are the roots of the resultant equation.
   *
   * The algorithm loops through the intersection list, calculating
   * the coefficients if an intersection is the start of a Region and
   * adding them to all intersections in that region.
   * When it detects a valid interval, it calculates the
   * roots from the starting intersection's coefficients and if any of
   * the roots are in the interval, the smallest one is returned.
   *
   */

  {
    register Float *tmpc;
    MetaInt *strt, *tmp;
    register int istrt, itmp;
    register int num, exitflag, inside;

    /*
     * Start the algorithm outside the first interval
     */
    inside = 0;
    istrt = 0;
    strt = iarr[istrt];
    if (strt->type != R_START)
      RLerror(RL_WARN, "MetaInt sanity check FAILED!\n");

    /*
     * Loop through intersection. If a root is found the code
     * will return at that point.
     */
    while (istrt < inum) {
      /*
       * Check for multiple intersections  at the same point.
       * This is also where the coefficients are calculated
       * and spread throughout that Metaball's sphere of
       * influence.
       */
      do {
        /* find out which metaball */
        i = strt->pnt;
        /* only at starting boundaries do this */
        if (strt->type == R_START) {
          register MetaVector *ml;
          register Float a1, a0;
          register Float xd, yd, zd;
          register Float c4, c2, c0;

          /* we're inside */
          inside++;

          /*  As promised, the full equations
           *
           *   c[4] = c4*a2*a2;
           *   c[3] = 4.0*c4*a1*a2;
           *   c[2] = 4.0*c4*a1*a1 + 2.0*c4*a2*a0 + c2*a2;
           *   c[1] = 4.0*c4*a1*a0 + 2.0*c2*a1;
           *   c[0] = c4*a0*a0 + c2*a0 + c0;
           *
           * where
           *        a2 = (x1*x1 + y1*y1 + z1*z1) = 1.0 because the ray
           *                                           is normalized
           *        a1 = (xd*x1 + yd*y1 + zd*z1)
           *        a0 = (xd*xd + yd*yd + zd*zd)
           *        xd = (x0 - xi)
           *        yd = (y0 - yi)
           *        zd = (z0 - zi)
           *        (xi,yi,zi) is center of Metaball
           *        (x0,y0,z0) is Ray origin
           *        (x1,y1,z1) is normalized Ray direction
           *        c4,c2,c0   are the coefficients for the
           *                       Metaball's density function
           *
           */

          /*
           * Some compilers would recalculate
           * this each time its used.
           */
          ml = &(blob->list[i]);

          xd = ray->pos.x - ml->x;
          yd = ray->pos.y - ml->y;
          zd = ray->pos.z - ml->z;
          a1 = (xd * ray->dir.x + yd * ray->dir.y + zd * ray->dir.z);
          a0 = (xd * xd + yd * yd + zd * zd);

          c0 = ml->c0;
          c2 = ml->c2;
          c4 = ml->c4;

          c[4] = c4;
          c[3] = 4.0 * c4 * a1;
          c[2] = 2.0 * c4 * (2.0 * a1 * a1 + a0) + c2;
          c[1] = 2.0 * a1 * (2.0 * c4 * a0 + c2);
          c[0] = c4 * a0 * a0 + c2 * a0 + c0;

          /*
           * Since we've gone through the trouble of calculating the
           * coefficients, put them where they'll be used.
           * Starting at the current intersection(It's also the start of
           * a region), add the coefficients to each intersection until
           * we reach the end of this region.
           */
          itmp = istrt;
          tmp = strt;
          while ((tmp->pnt != i) || (tmp->type != R_END)) {
            tmpc = tmp->c;
            for (j = 0; j < 5; j++)
              *tmpc++ += c[j];
            itmp++;
            tmp = iarr[itmp];
          }

        } /* End of start of a Region */
        /*
         * Since the intersection wasn't the start
         * of a region, it must the end of one.
         */
        else
          inside--;

        /*
         * If we are inside a region(or regions) and the next
         * intersection is not at the same place, then we have
         * the start of an interval. Set the exitflag.
         */
        if ((inside > 0) && (strt->bound != iarr[istrt + 1]->bound))
          exitflag = 1;
        else
        /* move to next intersection */
        {
          istrt++;
          strt = iarr[istrt];
          exitflag = 0;
        }
        /*
         * Check to see if we're at the end. If so then exit with
         * no intersection found.
         */
        if (istrt >= inum) {
          return FALSE;
        }
      } while (!exitflag);
      /* End of Search for valid interval */

      /*
       * Find Roots along this interval
       */

      /* get coefficients from Intersection structure */
      tmpc = strt->c;
      for (j = 0; j < 5; j++)
        c[j] = *tmpc++;

      /* Don't forget to put in threshold */
      c[0] -= blob->T;

      /* Use Graphic Gem's Quartic Root Finder */
      num = SolveQuartic(c, s);

      /*
       * If there are roots, search for roots within the interval.
       */
      dist = 0.0;
      if (num > 0) {
        for (j = 0; j < num; j++) {
          /*
           * Not sure if EPSILON is truly needed, but it might cause
           * small cracks between intervals in some cases. In any case
           * I don't believe it hurts.
           */
          if ((s[j] >= (strt->bound - EPSILON)) &&
              (s[j] <= (iarr[istrt + 1]->bound + EPSILON))) {
            if (dist == 0.0)
              /* first valid root */
              dist = s[j];
            else if (s[j] < dist)
              /* else only if smaller */
              dist = s[j];
          }
        }
        /*
         * Found a valid root
         */
        if (dist > mindist && dist < *maxdist) {
          *maxdist = dist;
          BlobHits++;
          return TRUE;
          /* Yeah! Return valid root */
        }
      }

      /*
       * Move to next intersection
       */
      istrt++;
      strt = iarr[istrt];

    } /* End of loop through Intersection List */
  }   /* End of Solving for Ray/Blob Intersection */

  /*
   * return negative
   */
  return FALSE;
}

/***********************************************
 * Find the Normal of a Blob at a given point
 *
 * The normal of a surface at a point (x0,y0,z0) is the gradient of that
 * surface at that point. The gradient is the vector
 *
 *            Fx(x0,y0,z0) , Fy(x0,y0,z0) , Fz(x0,y0,z0)
 *
 * where Fx(),Fy(),Fz() are the partial dervitives of F() with respect
 * to x,y and z respectively. Since the surface of a Blob is made up
 * of the sum of one or more polynomials, the normal of a Blob at a point
 * is the sum of the gradients of the individual polynomials at that point.
 * The individual polynomials in this case are di(Ri) i = 0,...,num .
 *
 * To find the gradient of the contributing polynomials
 * take di(Ri) and substitute U = Ri^2;
 *
 *      di(U)    = c4i * U^2  +  c2i * U  + c0i
 *
 *      dix(U)   = 2 * c4i * Ux * U  +  c2i * Ux
 *
 *        U      = (x-xi)^2 + (y-yi)^2 + (z-zi)^2
 *
 *        Ux     = 2 * (x-xi)
 *
 * Rearranging we get
 *
 *    dix(x0,y0,z0) = 4 * c4i * xdi * disti + 2 * c2i * xdi
 *    diy(x0,y0,z0) = 4 * c4i * ydi * disti + 2 * c2i * ydi
 *    diz(x0,y0,z0) = 4 * c4i * zdi * disti + 2 * c2i * zdi
 *
 * where
 *         xdi       =   x0-xi
 *         ydi       =   y0-yi
 *         zdi       =   y0-yi
 *        disti      =   xdi*xdi + ydi*ydi + zdi*zdi
 *       (xi,yi,zi)  is  the center of Metaball i
 *       c4i,c2i,c0i are the coefficients of Metaball i's density function
 */
int BlobNormal(blob, pos, nrm, gnrm) Blob *blob;
Vector *pos, *nrm, *gnrm;
{
  register int i;

  /*
   * Initialize normals to zero
   */
  nrm->x = nrm->y = nrm->z = 0.0;
  /*
   * Loop through Metaballs. If the point is within a Metaball's
   * Sphere of influence, calculate the gradient and add it to the
   * normals
   */
  for (i = 0; i < blob->num; i++) {
    register MetaVector *sl;
    register Float dist, xd, yd, zd;

    sl = &(blob->list[i]);
    xd = pos->x - sl->x;
    yd = pos->y - sl->y;
    zd = pos->z - sl->z;

    dist = xd * xd + yd * yd + zd * zd;
    if (dist <= sl->rs) {
      register Float temp;

      /* temp is negative so normal points out of blob */
      temp = -2.0 * (2.0 * sl->c4 * dist + sl->c2);
      nrm->x += xd * temp;
      nrm->y += yd * temp;
      nrm->z += zd * temp;
    }
  }
  (void)VecNormalize(nrm);
  *gnrm = *nrm;
  return FALSE;
}

/*****************************************************************************
 * Calculate the extent of the Blob
 */
void BlobBounds(blob, bounds) Blob *blob;
Float bounds[2][3];
{
  Float r;
  Float minx, miny, minz, maxx, maxy, maxz;
  int i;

  /*
   * Loop through Metaballs to find the minimun and maximum in each
   * direction.
   */
  for (i = 0; i < blob->num; i++) {
    register Float d;

    r = sqrt(blob->list[i].rs);
    if (i == 0) {
      minx = blob->list[i].x - r;
      miny = blob->list[i].y - r;
      minz = blob->list[i].z - r;
      maxx = blob->list[i].x + r;
      maxy = blob->list[i].y + r;
      maxz = blob->list[i].z + r;
    } else {
      d = blob->list[i].x - r;
      if (d < minx)
        minx = d;
      d = blob->list[i].y - r;
      if (d < miny)
        miny = d;
      d = blob->list[i].z - r;
      if (d < minz)
        minz = d;
      d = blob->list[i].x + r;
      if (d > maxx)
        maxx = d;
      d = blob->list[i].y + r;
      if (d > maxy)
        maxy = d;
      d = blob->list[i].z + r;
      if (d > maxz)
        maxz = d;
    }
  }
  bounds[LOW][X] = minx;
  bounds[HIGH][X] = maxx;
  bounds[LOW][Y] = miny;
  bounds[HIGH][Y] = maxy;
  bounds[LOW][Z] = minz;
  bounds[HIGH][Z] = maxz;
}

char *BlobName() { return blobName; }

void BlobStats(tests, hits) unsigned long *tests, *hits;
{
  *tests = BlobTests;
  *hits = BlobHits;
}

void BlobMethodRegister(meth) UserMethodType meth;
{
  if (iBlobMethods)
    iBlobMethods->user = meth;
}
