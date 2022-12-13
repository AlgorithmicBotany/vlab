
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "matrix.h"

#define EPSILON 0.0000001

/* matrix stack */
mtype matrix_stack[STACK_SIZE][MAT_SIZE * MAT_SIZE];

int stack_top = -1;

/*************************************************************************/
void Normalize(mtype *vec) {
  mtype len;
  int i;

  if ((len = sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2])) > 0.0)
    for (i = 0; i < 3; i++)
      vec[i] /= len;
}

/*************************************************************************/
/* all zeros except 1's on diagonal */
void MakeUnitMatrix(mtype *mat) {
  int row, col;

  for (row = 0; row < MAT_SIZE; row++)
    for (col = 0; col < MAT_SIZE; col++)
      mat[access(row, col)] = row == col ? 1.0 : 0.0;
}

/*************************************************************************/

void InitializeMatrixStack(void) {
  stack_top = 0;

  MakeUnitMatrix(matrix_stack[0]);
}

/*************************************************************************/
int PushMatrix(void) {
  int j;

  if (stack_top == STACK_SIZE - 1) {
    fprintf(stderr, "Error: matrix stack full. PushMatrix ignored.\n");
    return 0;
  }

  if (stack_top == -1)
    InitializeMatrixStack();

  /* copy the current top matrix */
  for (j = 0; j < MAT_SIZE * MAT_SIZE; j++)
    matrix_stack[stack_top + 1][j] = matrix_stack[stack_top][j];

  stack_top++;

  return 1;
}

/*************************************************************************/
int PopMatrix(void) {
  if (stack_top == -1) {
    InitializeMatrixStack();
    return 0;
  }

  if (stack_top == 0) {
    fprintf(stderr, "Error: the last matrix cannot be popped.\n");
    return 0;
  }

  stack_top--;

  return 1;
}

/*************************************************************************/
void MultMatrices(mtype *src1, mtype *src2, mtype *res) {
  int row, col, i;

  for (row = 0; row < MAT_SIZE; row++)
    for (col = 0; col < MAT_SIZE; col++) {
      res[access(row, col)] = 0.0;

      for (i = 0; i < MAT_SIZE; i++)
        res[access(row, col)] += src1[access(row, i)] * src2[access(i, col)];
    }
}

/*************************************************************************/
void MultMatrix(mtype *mat) {
  int i;
  mtype smat[MAT_SIZE * MAT_SIZE];

  /* temporarily store the matrix from the top of matrix stack */
  for (i = 0; i < MAT_SIZE * MAT_SIZE; i++)
    smat[i] = matrix_stack[stack_top][i];

  MultMatrices(mat, smat, matrix_stack[stack_top]);
}

/*************************************************************************/

void Scale(mtype *vals) {
  mtype mat[MAT_SIZE * MAT_SIZE] = {0};
  int j;

  for (j = 0; j < MAT_SIZE - 1; j++)
    mat[access(j, j)] = vals[j];

  mat[access(MAT_SIZE - 1, MAT_SIZE - 1)] = 1.0;

  MultMatrix(mat);
}

/*************************************************************************/

void Translate(mtype *vals) {
  mtype mat[MAT_SIZE * MAT_SIZE] = {0};
  int j;

  for (j = 0; j < MAT_SIZE; j++)
    mat[access(j, j)] = 1.0;

  for (j = 0; j < MAT_SIZE - 1; j++)
    mat[access(MAT_SIZE - 1, j)] = vals[j];

  MultMatrix(mat);
}

/****************************************************************************/
#ifndef X
#define X 0
#define Y 1
#define Z 2
#endif

void CrossProduct(mtype *v1, mtype *v2, mtype *res)
/*---------------------------------------------------------------------------
 * Loads the cross product of v1 with v2 into the vector res.
 *---------------------------------------------------------------------------
 */
{
  res[X] = v1[Y] * v2[Z] - v1[Z] * v2[Y];
  res[Y] = v1[Z] * v2[X] - v1[X] * v2[Z];
  res[Z] = v1[X] * v2[Y] - v1[Y] * v2[X];
}

/*************************************************************************/
void Rotate(mtype *vals) {
  mtype mat[MAT_SIZE * MAT_SIZE] = {0};
  mtype rot[MAT_SIZE * MAT_SIZE] = {0};
  mtype invmat[MAT_SIZE * MAT_SIZE], auxmat[MAT_SIZE * MAT_SIZE];
  double angle;
  int j;
  mtype vec[MAT_SIZE - 1];
  mtype axis1[MAT_SIZE - 1], axis2[MAT_SIZE - 1], axis3[MAT_SIZE - 1];

  /* get a vector not in the same line as the specified vector of rotation */
  if ((vals[1] == vals[2]) && (vals[2] == vals[3])) {
    vec[0] = 1;
    for (j = 1; j < MAT_SIZE - 1; j++)
      vec[j] = 0;
  } else
    for (j = 0; j < MAT_SIZE - 1; j++)
      vec[j] = 1;

  for (j = 0; j < MAT_SIZE - 1; j++)
    axis1[j] = vals[1 + j];
  Normalize(axis1);

  CrossProduct(vec, axis1, axis2);
  Normalize(axis2);

  CrossProduct(axis1, axis2, axis3);

  /* now rotation vector, axis2, and axis3 form a coordinate system which we
     want to transform into x,y,z system (using invmat) */

  for (j = 0; j < MAT_SIZE - 1; j++) {
    mat[access(0, j)] = axis1[j];
    mat[access(1, j)] = axis2[j];
    mat[access(2, j)] = axis3[j];
  }
  mat[access(MAT_SIZE - 1, MAT_SIZE - 1)] = 1.0;

  InverseMatrix(mat, invmat);

  /* set rot as rotation around x axis by vals[0] degrees */
  MakeUnitMatrix(rot);
  angle = vals[0] * (double)M_PI / 180.0;
  rot[access(2, 2)] = rot[access(1, 1)] = cos(angle);
  rot[access(2, 1)] = -(rot[access(1, 2)] = sin(angle));

  MultMatrices(invmat, rot, auxmat);
  MultMatrices(auxmat, mat, rot);

  /* rot is the desired matrix describing the rotation */
  MultMatrix(rot);
}

/*************************************************************************/
void Rotate4(mtype angle, mtype x, mtype y, mtype z) {
  mtype vals[4];

  vals[0] = angle;
  vals[1] = x;
  vals[2] = y;
  vals[3] = z;
  Rotate(vals);
}
/*************************************************************************/
void GetMatrix(mtype *mat) {
  int j;

  if (mat == NULL)
    return;
  if (stack_top == -1) {
    InitializeMatrixStack();
  }

  for (j = 0; j < MAT_SIZE * MAT_SIZE; j++)
    mat[j] = matrix_stack[stack_top][j];
}

/*************************************************************************/
void TransformPoint(mtype *src, mtype *mat, mtype *dest) {
  int row, i;

  for (row = 0; row < MAT_SIZE; row++) {
    dest[row] = 0.0;

    for (i = 0; i < MAT_SIZE; i++)
      dest[row] += src[i] * mat[access(i, row)];
  }
}

/*************************************************************************/
void Transform3Point(mtype *src, mtype *mat, mtype *dest) {
  int row, i;
  float pt[MAT_SIZE], ptc[MAT_SIZE];

  for (i = 0; i < MAT_SIZE - 1; i++)
    pt[i] = src[i];
  pt[MAT_SIZE - 1] = 1.0;

  for (row = 0; row < MAT_SIZE; row++) {
    ptc[row] = 0.0;

    for (i = 0; i < MAT_SIZE; i++)
      ptc[row] += pt[i] * mat[access(i, row)];
  }

  if (ptc[MAT_SIZE - 1] != 1.0)
    if (fabs(ptc[MAT_SIZE - 1]) >= EPSILON)
      for (i = 0; i < MAT_SIZE - 1; i++)
        ptc[i] /= ptc[MAT_SIZE - 1];

  for (i = 0; i < MAT_SIZE - 1; i++)
    dest[i] = ptc[i];
}

/*************************************************************************/
void Transform3Vector(mtype *src, mtype *mat, mtype *dest) {
  int row, i;
  float vec[MAT_SIZE], vecC[MAT_SIZE];

  for (i = 0; i < MAT_SIZE - 1; i++)
    vec[i] = src[i];
  vec[MAT_SIZE - 1] = 0.0; /* in fact, vec = src-(0,0,0,1) */

  for (row = 0; row < MAT_SIZE; row++) {
    vecC[row] = 0.0;

    for (i = 0; i < MAT_SIZE; i++)
      vecC[row] += vec[i] * mat[access(i, row)];
  }

  if (vecC[MAT_SIZE - 1] != 1.0)
    if (vecC[MAT_SIZE - 1] != 0.0)
      for (i = 0; i < MAT_SIZE - 1; i++)
        vecC[i] /= vecC[MAT_SIZE - 1];

  for (i = 0; i < MAT_SIZE - 1; i++)
    dest[i] = vecC[i];
}

/*************************************************************************/
void InverseMatrix(mtype *srcmat, mtype *destmat) {
  mtype mat[16], aux;
  int i, row, col, k;

  for (i = 0; i < MAT_SIZE * MAT_SIZE; i++) {
    mat[i] = srcmat[i];
    destmat[i] = 0;
  }

  /* destmat is unit matrix */
  for (i = 0; i < MAT_SIZE; i++)
    destmat[access(i, i)] = 1.0;

  /* using Gauss' method, transfer mat to unit matrix doing the same steps
     on destmat */

  for (row = 0; row < MAT_SIZE; row++) {
    /* divide the row by mat[row][row] */
    if ((aux = mat[access(row, row)]) != 1.0)
      if (fabs(aux) < EPSILON) {
        /* must find a row with nonzero in column 'row' and add it to the
           current row */
        for (k = row + 1; k < MAT_SIZE; k++)
          if (fabs(mat[access(k, row)]) >= EPSILON) {
            for (i = 0; i < MAT_SIZE; i++) {
              mat[access(row, i)] += mat[access(k, i)];
              destmat[access(row, i)] += destmat[access(k, i)];
            }
            break;
          }

        if (k == MAT_SIZE) {
          fprintf(stderr,
                  "Cannot get an inverse matrix for a singular matrix.\n");
          return;
        }
        aux = mat[access(row, row)];
      }

    for (col = 0; col < MAT_SIZE; col++) {
      mat[access(row, col)] /= aux;
      destmat[access(row, col)] /= aux;
    }

    /* subtract the row from all other rows to zero all items in
       the column row */
    for (k = 0; k < MAT_SIZE; k++)
      if ((k != row) && (fabs(aux = mat[access(k, row)]) >= EPSILON))
        for (col = 0; col < MAT_SIZE; col++) {
          mat[access(k, col)] -= aux * mat[access(row, col)];
          destmat[access(k, col)] -= aux * destmat[access(row, col)];
        }
  }

  /* now mat is E and destmat contains an inverse matrix to srcmat */
}
