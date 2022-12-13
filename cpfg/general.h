/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef __GENERAL_H__
#define __GENERAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DEF_CONTOUR 0   /* id of a default contour */
#define DEFAULT_RINGS 4 /* default number of rings */

/* type of the spline : open/closed. MUST BE 0,1 */
enum { SPLINE_TYPE_OPEN, SPLINE_TYPE_CLOSED, SPLINE_TYPES };

/* base of the spline center line: Hermit/B-spline MUST BE 0,1,...*/
enum { SPLINE_BASE_HERMIT, SPLINE_BASE_BSPLINE, SPLINE_BASES };

/* prototypes */
void FreeSpline(void);        /* at the very end */
void InitializeSplines(void); /* after each new view (because of contours) */

void InitializeSpline(TURTLE *tu, int type, int rings);
void FinishSpline(TURTLE *tu, DRAWPARAM *dr /*, int type*/);
void SetControlPoint(TURTLE *tu, DRAWPARAM *dr, int contours);

void make_contour(float *r, const TURTLE *tu, int contour1, int contour2,
                  float length, float radius, const float *radius_tangent,
                  float t, const DRAWPARAM *dr); /* used in inventor.c */
void strip_to_mesh(float *ring1, const TURTLE *tu1, float *ring2,
                   const TURTLE *tu2,
                   const DRAWPARAM *dr); /* used in inventor.c */

/* old function calls for generalized cylinders */
void start_spline(TURTLE *turtle, DRAWPARAM *dr);
void continue_spline(TURTLE *turtle, DRAWPARAM *dr, int contours);
void finish_spline(TURTLE *turtle, DRAWPARAM *dr, int contours);

/* called when [ and ] interpreted */
void PushSpline(const TURTLE *tu);
void PopSpline(TURTLE *tu);

void reset_spline_stack(void);

int get_contour_index(int id);
int read_in_contour(char *filename, int id);
void store_contour(char *filename, int id);
int process_contours(void);
void UpdateContourArraySize(int con, float **cont1, float **cont2);

#ifdef __cplusplus
}
#endif

#endif
