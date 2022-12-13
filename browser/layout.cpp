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



/******************************************************************************
 *
 * the code for compacting trees is based on an article in
 * IEEE software journal (July 1990, page 21-27)
 *
 * Original author: Sven Moen (from the article)
 * This source written by: Pavol Federl (implementer)
 *
 */

#include <cstdlib>

#include "graphics.h"
#include "layout.h"
#include "tree.h"
#include "xmemory.h"

#define LINES_RAY 1
#define LINES_COMB 2

#define PARENT_TOP 1
#define PARENT_CENTERED 2

struct POLYLINE {
  int dx, dy;
  int width;
  POLYLINE *link;
};

static int lines_type;           /* here we store the flag, so that we */
static int parent_position_type; /* don't have to call the graphics_get...() */
                                 /* functions all the time */

/* a datastructure that will keep all the pointers to the lines that
 * have been allocated, so that we can clear it up later */

static struct {
  POLYLINE **line;
  long nAlloc;
  long nUsed;
} lineMemory = {NULL, 0, 0};

typedef struct polygon_struc POLYGON;

struct polygon_struc {
  struct {
    POLYLINE *head;
    POLYLINE *tail;
  } upper, lower;
};

void _layout(NODE *t);
void layout_leaf(NODE *t);
void contour_combine(POLYLINE *c1, POLYLINE *c2, int d);
int contour_diff(POLYLINE *upper, POLYLINE *lower);
POLYLINE *line(int dx, int dy, POLYLINE *link);

/******************************************************************************
 *
 * this is basically a wrapper for _layout. It will set up an array
 * of pointers for the memory that line() allocates, and when called again,
 * this memory is released.
 *
 */

void layout(NODE *t, bool center_parent) {
  /* if there is no memory allocated, allocate some */
  if (lineMemory.nAlloc == 0) {
    lineMemory.line = (POLYLINE **)xmalloc(500 * sizeof(POLYLINE *));
    lineMemory.nAlloc = 500;
  } else {
    /* free the allocated memory for lines */
    for (int i = 0; i < lineMemory.nUsed; i++)
      xfree(lineMemory.line[i]);
  }

  lineMemory.nUsed = 0;

  lines_type = LINES_COMB;
  if (center_parent)
    parent_position_type = PARENT_CENTERED;
  else
    parent_position_type = PARENT_TOP;
  _layout(t);
  t->rx = t->ry = 0;
}

/******************************************************************************
 *
 * will calculate rx,ry for every node of the tree t, which represent the
 * relative (x,y) with respect to the immediate parent.
 *
 */

void _layout(NODE *root) {
  int i;
  int height;
  int prevChildNodeHeight;
  int d;

  if (root->nChildren > 0) {

    /* layout the first child, and 'inherit' its contour */
    _layout(root->child[0]);
    root->upperContour = root->child[0]->upperContour;
    root->lowerContour = root->child[0]->lowerContour;
    root->child[0]->rx =
        root->nodeWidth + text_to_line_distance + horiz_distance;

    /* now do the rest of the children, one at a time, and determine
     * the distances between the parent contour, and the child contour
     *
     * also, merge the parent's contour with the child's contour
     *
     */
    prevChildNodeHeight = height = root->child[0]->nodeHeight;

    for (i = 1; i < root->nChildren; i++) {
      _layout(root->child[i]);
      d = contour_diff(root->child[i]->upperContour, root->lowerContour);

      root->child[i]->rx = 0;
      root->child[i]->ry = -prevChildNodeHeight - d - vert_distance;

      /* combine the root's upper contour with that of the child */
      contour_combine(root->upperContour, root->child[i]->upperContour,
                      -d - height - vert_distance);

      /* and do the same thing for the lower contour, but
       * append the lower contour to the child's one */
      contour_combine(root->child[i]->lowerContour, root->lowerContour,
                      d + root->child[i]->nodeHeight + vert_distance);

      root->lowerContour = root->child[i]->lowerContour;

      /* increase the height of the children */
      height += d + root->child[i]->nodeHeight + vert_distance;

      /* 'remember' the height of the last child */
      prevChildNodeHeight = root->child[i]->nodeHeight;
    }

    /* now the root has a combined contour of all its children, so all
     * we need to do is to position the first child, and addjust the
     * contours so that it includes the parent as well */

    /* we will position the parent according to the parent_position flag */
    if (parent_position_type == PARENT_TOP) {
      root->child[0]->ry = 0;

      /* adjust the upper contour of the root */
      root->upperContour =
          line(root->nodeWidth + text_to_line_distance + horiz_distance, 0,
               root->upperContour);

      /* adjust the lower contour of the root, but this depends on the
       * line style */
      if (lines_type == LINES_RAY) {
        root->lowerContour = line(horiz_distance, -height + root->nodeHeight,
                                  root->lowerContour);
        root->lowerContour = line(root->nodeWidth + text_to_line_distance, 0,
                                  root->lowerContour);
      } else { /* LINES_COMB */
        root->lowerContour = line(horiz_distance / 2, 0, root->lowerContour);
        root->lowerContour =
            line(0, -height + root->nodeHeight, root->lowerContour);
        root->lowerContour = line(horiz_distance - horiz_distance / 2 +
                                      root->nodeWidth + text_to_line_distance,
                                  0, root->lowerContour);
      }

    } else { /* PARENT_CENTERED */
      root->child[0]->ry = (height - root->nodeHeight) / 2;
      if (root->child[0]->ry < 0) {
        root->child[0]->ry = 0;
      }

      if (lines_type == LINES_RAY) {
        root->upperContour =
            line(horiz_distance, root->child[0]->ry, root->upperContour);
        root->upperContour = line(root->nodeWidth + text_to_line_distance, 0,
                                  root->upperContour);
        root->lowerContour = line(
            horiz_distance, -height + root->child[0]->ry + root->nodeHeight,
            root->lowerContour);
        root->lowerContour = line(root->nodeWidth + text_to_line_distance, 0,
                                  root->lowerContour);
      } else { /* LINES_COMB */
        root->upperContour = line(horiz_distance / 2, 0, root->upperContour);
        root->upperContour = line(0, root->child[0]->ry, root->upperContour);
        root->upperContour = line(root->nodeWidth + text_to_line_distance +
                                      horiz_distance - horiz_distance / 2,
                                  0, root->upperContour);

        root->lowerContour = line(horiz_distance / 2, 0, root->lowerContour);
        root->lowerContour =
            line(0, -height + root->child[0]->ry + root->nodeHeight,
                 root->lowerContour);
        root->lowerContour = line(root->nodeWidth + text_to_line_distance +
                                      horiz_distance - horiz_distance / 2,
                                  0, root->lowerContour);
      }
    }
  } else { /* there are no children */
    layout_leaf(root);
  }
}

int offset(int p1, int p2, int a1, int a2, int b1, int b2) {
  int d, s, t;

  if (b1 <= p1 || p1 + a1 <= 0)
    return 0;

  t = b1 * a2 - a1 * b2;
  if (t > 0) {
    if (p1 < 0) {
      s = p1 * a2;
      d = s / a1 - p2;
    } else if (p1 > 0) {
      s = p1 * b2;
      d = s / b1 - p2;
    } else
      d = -p2;
  } else if (b1 < p1 + a1) {
    s = (b1 - p1) * a2;
    d = b2 - (p2 + s / a1);
  } else if (b1 > p1 + a1) {
    s = (a1 + p1) * b2;
    d = s / b1 - (p2 + a2);
  } else
    d = b2 - (p2 + a2);

  if (d > 0)
    return d;
  else
    return 0;
}

/******************************************************************************
 *
 * assign to this leaf simple contours
 *
 */

void layout_leaf(NODE *t) {
  t->upperContour = line(t->nodeWidth + 5, 0, NULL);
  t->lowerContour = line(t->nodeWidth + 5, 0, NULL);
}

POLYLINE *line(int dx, int dy, POLYLINE *link) {
  POLYLINE *newP;

  newP = (POLYLINE *)xmalloc(sizeof(POLYLINE));
  newP->dx = dx;
  newP->dy = dy;
  if (link == 0)
    newP->width = dx;
  else
    newP->width = link->width + dx;
  newP->link = link;

  /* insert this pointer into the memory pool, so that it can be
   * freed later on */
  if (lineMemory.nUsed == lineMemory.nAlloc) {
    lineMemory.line = (POLYLINE **)xrealloc(
        lineMemory.line, sizeof(POLYLINE **) * (lineMemory.nAlloc + 500));
    lineMemory.nAlloc += 500;
  }
  lineMemory.line[lineMemory.nUsed++] = newP;

  return newP;
}

/******************************************************************************
 *
 * will calculate the difference between two contours (or one can think about
 * it as the distance at which these two contours have to be displaced so
 * that they would not touch (or barely touch))
 *
 * it is assumed that both of these polylines (contours) start at the same
 * x coordinate
 */

int contour_diff(POLYLINE *upper, POLYLINE *lower) {
  int total, d, x, y;

  x = y = total = 0;

  while (lower && upper) { /* compute offset total */
    d = offset(x, y, lower->dx, lower->dy, upper->dx, upper->dy);
    y += d;
    total += d;

    if (x + lower->dx <= upper->dx) {
      y += lower->dy;
      x += lower->dx;
      lower = lower->link;
    } else {
      y -= upper->dy;
      x -= upper->dx;
      upper = upper->link;
    }
  }

  return total;
}

/******************************************************************************
 *
 * combine two contours together
 *    ( will append the rest of the p2 to p1,
 *      so that p2 starts 'd' units verticaly below p1)
 *
 */

void contour_combine(POLYLINE *p1, POLYLINE *p2, int d) {
  int x1, y1;
  int x2, y2;
  int w, h;
  POLYLINE *link;
  POLYLINE *c1;
  POLYLINE *c2;

  /* find the last segment of p1 into c1, and calculate the endpoint of
   * the last segment into x1, y1 */
  c1 = p1;
  x1 = c1->dx;
  y1 = c1->dy;
  while (c1->link != NULL) {
    c1 = c1->link;
    x1 += c1->dx;
    y1 += c1->dy;
  }

  /* if c1 is actually longer than c2, then we don't have to
   * do anything */
  if (p1->width >= p2->width) {
    return;
  }

  /* now, find the segment of p2 which will have to be split, and
   * appended to p1 (to c1). Also, in x2, y2 calculate the coordinates
   * where this segment begins */
  x2 = 0;
  y2 = d;
  c2 = p2;
  while (x2 + c2->dx < x1) {
    x2 += c2->dx;
    y2 += c2->dy;
    c2 = c2->link;
  }

  /* create a new segment from c2 by splitting it, and appending to
   * it the rest of c2 */
  w = x2 + c2->dx - x1;
  h = (c2->dy * w) / c2->dx;
  link = line(w, h, c2->link);

  /* now we have the appropriately split c2 tail in 'link', so add the
   * vertical line that will join it to the p1 */

  /* append the vertical line of length d to it */
  c1->link = line(0, y2 + c2->dy - y1 - h, link);

  /* and now we just have to adjust the width of p1 */
  p1->width = p2->width;

  /* done */
}
