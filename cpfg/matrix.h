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



#ifndef __MATRIX_H__
#define __MATRIX_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define STACK_SIZE 50

#define MAT_SIZE 4

/* can be changed for row first storage */
#define access(row, col) ((row)*MAT_SIZE + (col))

typedef float mtype;

void InitializeMatrixStack(void);
int PushMatrix(void);
int PopMatrix(void);
void MultMatrices(const mtype *src1, const mtype *src2, mtype *res);
void MultMatrix(mtype *mat);
void Scale(mtype *vals);
void Scale3(mtype x, mtype y, mtype z);
void Translate(mtype *vals);
void Translate3(mtype x, mtype y, mtype z);
void Rotate(mtype *vals);
void Rotate4(mtype degree, mtype x, mtype y, mtype z);

void GetMatrix(mtype *mat);
void TransformPoint(const mtype *src, const mtype *mat, mtype *dest);
void Transform3Point(const mtype *src, const mtype *mat, mtype *dest);
void Transform3Vector(const mtype *src, const mtype *mat, mtype *dest);
void InverseMatrix(const mtype *srcmat, mtype *destmat);
int InverseMatrixFast(const mtype *srcmat, mtype *destmat);

#ifdef __cplusplus
}
#endif

#endif
