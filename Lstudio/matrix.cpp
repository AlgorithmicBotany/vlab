#include <fw.h>
#include <glfw.h>

#include "knot.h"
#include "matrix.h"


Matrix::Matrix(int rows, int cols, const float* aInit) : _rows(rows), _cols(cols)
{
	assert(_rows>0);
	assert(_cols>0);
	_arr = new float[_rows*_cols];
	if (0 != aInit)
	{
		for (int i=0; i<_rows*_cols; i++)
			_arr[i] = aInit[i];
	}
	else
	{
		for (int i=0; i<_rows*_cols; i++)
			_arr[i] = 0.0;
	}
}


Matrix4x3::Matrix4x3(WorldPointf r0, WorldPointf r1, WorldPointf r2, WorldPointf r3) : Matrix(4, 3)
{
	Set(0, 0, r0.X());
	Set(0, 1, r0.Y());
	Set(0, 2, r0.Z());

	Set(1, 0, r1.X());
	Set(1, 1, r1.Y());
	Set(1, 2, r1.Z());

	Set(2, 0, r2.X());
	Set(2, 1, r2.Y());
	Set(2, 2, r2.Z());

	Set(3, 0, r3.X());
	Set(3, 1, r3.Y());
	Set(3, 2, r3.Z());
}


void MulMatrix(const Matrix& l, const Matrix& r, Matrix& res)
{
	assert(l.Cols() == r.Rows());
	assert(res.Rows() == l.Rows());
	assert(res.Cols() == r.Cols());
	const int rows = res.Rows();
	const int cols = res.Cols();
	const int R = l.Cols();
	for (int i=0; i<rows; i++)
	{
		for (int j=0; j<cols; j++)
		{
			float sum = 0.0f;
			for (int k=0; k<R; k++)
				sum += l.Get(i, k)*r.Get(k, j);
			res.Set(i, j, sum);
		}
	}
}
