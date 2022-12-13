
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "density3d.h"
#include "octree.h"

typedef unsigned int TREE_REF; /* pointers are not used so the arrays of
                                  leaves and nodes can be rellocated. */

typedef unsigned int LEAF_COORD;

struct OCTNODE_TYPE {
  TREE_REF children[8]; /* reference to children
                           lowest bit specifies whether it is a leaf (1)
                           or nonleaf (0) */
};

typedef struct OCTNODE_TYPE OCTNODE_TYPE;

extern char verbose;

/**** leaves ****/
LEAF_TYPE *leaves = NULL;
#define LEAF_ARRAY_SIZE 2000
unsigned int num_leaves;
unsigned int leaf_array_size;

/**** nonleaves - nodes ****/
OCTNODE_TYPE *octnodes = NULL;
#define OCTNODE_ARRAY_SIZE 1000
unsigned int num_octnodes;
unsigned int octnode_array_size;

/* root */
TREE_REF octree_root;
int root_level;
unsigned long octree_size;

/***************************************************************************/
void FreeOctreeStructures(void) {
  /* free array of leaves */
  if (leaves != NULL) {
    free(leaves);
    leaves = NULL;
  }
  num_leaves = 0;
  leaf_array_size = LEAF_ARRAY_SIZE;

  /* free array of nodes */
  if (octnodes != NULL) {
    free(octnodes);
    octnodes = NULL;
  }
  num_octnodes = 0;
  octnode_array_size = OCTNODE_ARRAY_SIZE;
}

/***************************************************************************/
static int AllocateOctreeStructures(void) {
  FreeOctreeStructures();

  /* allocate array of leaves */
  if ((leaves = (LEAF_TYPE *)malloc(leaf_array_size * sizeof(LEAF_TYPE))) ==
      NULL) {
    fprintf(stderr, "Cannot allocate memory for octree leaves\n");
    return 0;
  }

  /* allocate array of nodes */
  if ((octnodes = (OCTNODE_TYPE *)malloc(octnode_array_size *
                                         sizeof(OCTNODE_TYPE))) == NULL) {
    fprintf(stderr, "Cannot allocate memory for octree nodes\n");
    return 0;
  }
  return 1;
}

/***************************************************************************/
int InitializeOctree(int level) {
  if (level < 0)
    return 0;

  if (!AllocateOctreeStructures())
    return 0;

  /* 0 to octree_root index */
  octree_root = 0;
  root_level = level;

  octree_size = 1 << level;

  if (verbose)
    fprintf(stderr, "Octree initialized: size %lu.\n", octree_size);

  return 1;
}

/***************************************************************************/
static TREE_REF CreateEmptyNode(TREE_REF *node_ref) {
  int i;

  if (num_octnodes == octnode_array_size) {
    /* reallocate octnodes */
    octnode_array_size *= 2;
    octnodes = (OCTNODE_TYPE *)realloc(octnodes, octnode_array_size *
                                                     sizeof(OCTNODE_TYPE));
    if (octnodes == NULL) {
      fprintf(stderr, "Cannot reallocate memory for octree nodes\n");
      return 0;
    }
  }

  *node_ref = (num_octnodes + 1) << 1;

  if (verbose)
    fprintf(stderr, "Octree - num_octnodes %u\n", num_octnodes);

  for (i = 0; i < 8; i++)
    octnodes[num_octnodes].children[i] = 0;

  num_octnodes++;

  return *node_ref;
}

/***************************************************************************/
static TREE_REF CreateEmptyLeaf(TREE_REF *leaf_ref) {
  if (num_leaves == leaf_array_size) {
    leaf_array_size *= 2;

    if ((leaves = (LEAF_TYPE *)realloc(
             leaves, leaf_array_size * sizeof(LEAF_TYPE))) == NULL) {
      fprintf(stderr, "Cannot reallocate memory for octree leaves\n");
      return 0;
    }
  }

  if (verbose)
    fprintf(stderr, "Octree - new leaf (%u)\n", num_leaves + 1);

  leaves[num_leaves] = 0;

  return *leaf_ref = ((++num_leaves) << 1) + 1; /* lowest bit is 1 for leaves */
}

/***************************************************************************/
/* When a specified leaf doesn't exist it is created. */
static TREE_REF AccessLeafXYZ(TREE_REF *node_ref, LEAF_COORD x, LEAF_COORD y,
                              LEAF_COORD z, int level) {
  LEAF_COORD which;

  if (verbose)
    fprintf(stderr, "Octree - AccessLeaf %d %d %d, level %d\n", x, y, z, level);

  if (level == 0) {
    if (*node_ref == 0)
      return CreateEmptyLeaf(node_ref); /* returns 0 if not created */
    else
      return *node_ref;
  }

  level--;
  /* index of a child */
  which = ((x >> level) & 0x1) + (((y >> level) & 0x1) << 1) +
          (((z >> level) & 0x1) << 2);

  if (verbose)
    fprintf(stderr, "Octree - AccessLeaf: which is %d\n", which);

  if (*node_ref == 0)
    /* node doesn't exist */
    if (CreateEmptyNode(node_ref) == 0)
      return 0;

  /* node exist */
  return AccessLeafXYZ(&(octnodes[((*node_ref) - 1) >> 1].children[which]), x,
                       y, z, level);
}

/***************************************************************************/
/* pt[i] (i=0,1,2) must be within (0,1) */

CELL_TYPE *GetLeafPt(double *pt) {
  TREE_REF leaf_ref;
  LEAF_COORD x, y, z;
  int c;

  for (c = 0; c < 3; c++)
    if ((pt[c] < 0) || (pt[c] >= 1))
      return NULL;

  x = floor(pt[0] * (double)octree_size);
  y = floor(pt[1] * (double)octree_size);
  z = floor(pt[2] * (double)octree_size);

  if ((leaf_ref = AccessLeafXYZ(&octree_root, x, y, z, root_level)) == 0) {
    fprintf(stderr, "Octree leaf couln't be accessed - not created.\n");
    return NULL;
  }

  return leaves + ((leaf_ref - 1) >> 1);
}
