
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ecosystem.h"
#include "grid.h"

#define DIM 3

#define X 0
#define Y 1
#define Z 2

/* grid */
struct OBJECT_LIST_TYPE {
  item_type *prim;
  struct OBJECT_LIST_TYPE *next;
};
typedef struct OBJECT_LIST_TYPE OBJECT_LIST_TYPE;

struct CELL_TYPE {
  OBJECT_LIST_TYPE *list;
};
typedef struct CELL_TYPE CELL_TYPE;

struct grid_type {
  int size[DIM];    /* size of the grid (in nodes) */
  float range[DIM]; /* size in coordinates */
  float pos[DIM];   /* position of lower left corner */
  CELL_TYPE *data;
};
typedef struct grid_type grid_type;

grid_type grid = {0};

extern char verbose;
extern char is3d;
extern char vigor;

/****************************************************************************/
void FreeGrid(void) {
  if (grid.data != NULL)
    free(grid.data);
  grid.data = NULL;
}

/****************************************************************************/
void InitializeGrid(int *size) {
  int x, y, z, c;

  FreeGrid();

  for (c = 0; c <= (is3d ? Z : Y); c++)
    if ((grid.size[c] = size[c]) <= 0)
      grid.size[c] = 1;

  if (!is3d)
    grid.size[Z] = 1;

  if ((grid.data = (CELL_TYPE *)malloc(grid.size[X] * grid.size[Y] *
                                       grid.size[Z] * sizeof(CELL_TYPE))) ==
      NULL) {
    fprintf(stderr, "Cannot allocate enough memory for the grid\n");
    exit(0);
  }

  for (z = 0; z < grid.size[Z]; z++)
    for (y = 0; y < grid.size[Y]; y++)
      for (x = 0; x < grid.size[X]; x++)
        grid.data[z * grid.size[X] * grid.size[Y] + y * grid.size[X] + x].list =
            NULL;
}

/****************************************************************************/
CELL_TYPE *GetCell(grid_type *grid, int x, int y, int z) {
  if (x < 0)
    x = 0;
  if (x >= grid->size[X])
    x = grid->size[X] - 1;

  if (y < 0)
    y = 0;
  if (y >= grid->size[Y])
    y = grid->size[Y] - 1;

  if (is3d) {
    if (z < 0)
      z = 0;
    if (z >= grid->size[Z])
      z = grid->size[Z] - 1;
  } else
    z = 0;

  return grid->data + z * grid->size[X] * grid->size[Y] + y * grid->size[X] + x;
}

/****************************************************************************/
/* adds a new item to the beginning of a linked list */
void AddToList(OBJECT_LIST_TYPE **first, item_type *prim) {
  OBJECT_LIST_TYPE *ptr;

  if ((ptr = (OBJECT_LIST_TYPE *)malloc(sizeof(OBJECT_LIST_TYPE))) == NULL) {
    fprintf(stderr, "ecosystem - cannot allocate memory for an item!\n");
    exit(0);
  }

  ptr->next = (*first);
  ptr->prim = prim;
  *first = ptr;
}

/*************************************************************************/
/* Add primitive to cells iside its bounding box
   COULD BE TIGHTER!  */
void AddObject(item_type *prim) {
  int c, x, y, z;
  CELL_TYPE *cell;

  for (c = X; c <= (is3d ? Z : Y); c++) {
    prim->range[c][0] = floor((prim->position[c] - prim->radius - grid.pos[c]) /
                              grid.range[c] * (float)grid.size[c]) -
                        1;
    if (prim->range[c][0] < 0)
      prim->range[c][0] = 0;

    prim->range[c][1] =
        1 + floor((prim->position[c] + prim->radius - grid.pos[c]) /
                  grid.range[c] * (float)grid.size[c]);
    if (prim->range[c][1] >= grid.size[c])
      prim->range[c][1] = grid.size[c] - 1;
  }

  if (!is3d)
    prim->range[Z][0] = prim->range[Z][1] = 0;

  if (verbose)
    fprintf(stderr, "Primitive range: x:%d-%d; y:%d-%d;\n", prim->range[X][0],
            prim->range[X][1], prim->range[Y][0], prim->range[Y][1]);

  /* for all nodes in the range */
  for (z = prim->range[Z][0]; z <= prim->range[Z][1]; z++)
    for (y = prim->range[Y][0]; y <= prim->range[Y][1]; y++)
      for (x = prim->range[X][0]; x <= prim->range[X][1]; x++) {
        cell =
            grid.data + z * grid.size[X] * grid.size[Y] + y * grid.size[X] + x;

        AddToList(&(cell->list), prim);
      }
}

/*************************************************************************/
/* reset the grid */
void ResetGrid(float *min_pos, float *max_pos) {
  int x, y, z, c;
  CELL_TYPE *cell;
  OBJECT_LIST_TYPE *ptr, *ptr2;

  for (c = 0; c <= (is3d ? Z : Y); c++) {
    grid.pos[c] = min_pos[c] - 0.001;
    grid.range[c] = max_pos[c] - min_pos[c] + 0.002;
  }

  /* initialize all nodes */
  for (z = 0; z < grid.size[Z]; z++)
    for (y = 0; y < grid.size[Y]; y++)
      for (x = 0; x < grid.size[X]; x++) {

        cell =
            grid.data + z * grid.size[X] * grid.size[Y] + y * grid.size[X] + x;

        ptr = cell->list;

        while (ptr != NULL) {
          ptr2 = ptr;
          ptr = ptr->next;

          free(ptr2);
        }

        cell->list = NULL;
      }

  if (verbose)
    fprintf(stderr, "Grid reset.\n");
}

/*************************************************************************/
int TestIntersection(item_type *prim) {
  OBJECT_LIST_TYPE *ptr;
  item_type *prim2;
  char not_found = 1;
  float vec[DIM];
  int x, y, z;

  /* for all nodes in the range */
  for (z = prim->range[Z][0]; z <= prim->range[Z][1]; z++)
    for (y = prim->range[Y][0]; y <= prim->range[Y][1]; y++)
      for (x = prim->range[X][0]; x <= prim->range[X][1]; x++) {

        ptr = grid.data[z * grid.size[X] * grid.size[Y] + y * grid.size[X] + x]
                  .list;

        /* go through the linked list */
        while (ptr != NULL) {
          if ((prim2 = ptr->prim) != NULL){

            /* perform the test */
            if (prim2 != prim &&             /* don't test with itself */
                prim->index == prim2->index){ /* test only items from the
                                                same group */
              if (vigor) {

                if (!prim2->removed) {
                  /* already removed will not dominate this one */

                  /* first check the intersection */

                  /* get the distance */
                  vec[X] = prim->position[X] - prim2->position[X];
                  vec[Y] = prim->position[Y] - prim2->position[Y];

                  if (is3d)
                    vec[Z] = prim->position[Z] - prim2->position[Z];
                  else
                    vec[Z] = 0;

                  if (vec[X] * vec[X] + vec[Y] * vec[Y] + vec[Z] * vec[Z] <=
                      (prim->radius + prim2->radius) *
                          (prim->radius + prim2->radius)) {
                    if (prim->vigor < prim2->vigor) {
                      not_found = 0;
                      /* also mark as removed */
                      prim->removed = 1;
                    } else if (prim->vigor == prim2->vigor &&
                               prim->radius < prim2->radius) {
                      /* the one with lower radius goes  */
                      not_found = 0;
                      /* also mark as removed */
                      prim->removed = 1;
                    }
                  }
                }
              }
	      else {
		if (prim->radius <= prim2->radius) { /* only if the other
							sphere is bigger */
		  /* get the distance */
		  vec[X] = prim->position[X] - prim2->position[X];
		  vec[Y] = prim->position[Y] - prim2->position[Y];
		  
		  if (is3d)
		    vec[Z] = prim->position[Z] - prim2->position[Z];
		  else
		    vec[Z] = 0;
		  
		  if (vec[X] * vec[X] + vec[Y] * vec[Y] + vec[Z] * vec[Z] <=
		      (prim->radius + prim2->radius) *
		      (prim->radius + prim2->radius))
		    not_found = 0;
		}
	      }
	    }
	  }

          ptr = ptr->next;
        }
      }

  return not_found;
}
