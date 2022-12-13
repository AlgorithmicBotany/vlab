/* matrix.h */

#ifndef _MATRIX_H
#define _MATRIX_H

#define X 0
#define Y 1
#define Z 2

#define STACK_SIZE 50

#define MAT_SIZE 4

#define MIN2D(x, y) ((x) < (y) ? (x) : (y))
#define MAX2D(x, y) ((x) > (y) ? (x) : (y))
#define MAX3D(x, y, z) (x > y && x > z ? x : (y > z ? y : z))
#define MIN3D(x, y, z) (x < y && x < z ? x : (y < z ? y : z))
#define MAX3Di(x, y, z) (x > y && x > z ? X : (y > z ? Y : Z))
#define MIN3Di(x, y, z) (x < y && x < z ? X : (y < z ? Y : Z))

/* can be changed for row first storage */
#define access(row, col) ((row)*MAT_SIZE + (col))

typedef float mtype;

#ifdef __cplusplus
extern "C" {
#endif

void PrintMatrix(char *str, mtype *mat);
void Normalize(mtype *vec);
void CrossProduct(float *v1, float *v2, float *res);
float DotProduct(float *v1, float *v2);
float Length(float *v1, float *v2);

void MakeUnitMatrix(mtype *mat);
void InitializeMatrixStack(void);
int PushMatrix(void);
int PopMatrix(void);
void MultMatrices(mtype *src1, mtype *src2, mtype *res);
void MultMatrix(mtype *mat);
void Scale(mtype *vals);
void Translate(mtype *vals);
void Rotate(mtype *vals);
void Rotate4(mtype angle, mtype x, mtype y, mtype z);

void GetMatrix(mtype *mat);
void TransformPoint(mtype *src, mtype *mat, mtype *dest);
void Transform3Point(mtype *src, mtype *mat, mtype *dest);
void Transform3Vector(mtype *src, mtype *mat, mtype *dest);
void InverseMatrix(mtype *srcmat, mtype *destmat);

#ifdef __cplusplus
}
#endif

#endif
