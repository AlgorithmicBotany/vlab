#ifndef __MATRIX_H__
#define __MATRIX_H__


class Matrix
{
public:
	Matrix(int rows, int cols=1, const float* aInit = 0);
	~Matrix()
	{ delete []_arr; }
	void Set(int r, int c, float v)
	{
		assert(r>=0);
		assert(r<_rows);
		assert(c>=0);
		assert(c<_cols);
		_Set(c + r*_cols, v);
	}
	float Get(int r, int c) const
	{
		assert(r>=0);
		assert(r<_rows);
		assert(c>=0);
		assert(c<_cols);
		return _Get(c + r*_cols);
	}
	friend void MulMatrix(const Matrix&, const Matrix&, Matrix&);
	int Rows() const
	{ return _rows; }
	int Cols() const
	{ return _cols; }
protected:
	void _Set(int i, float v)
	{
		assert(i>=0);
		assert(i<_rows*_cols);
		_arr[i] = v;
	}
	float _Get(int i) const
	{
		assert(i>=0);
		assert(i<_rows*_cols);
		return _arr[i];
	}

	const int _rows;
	const int _cols;
	float* _arr;
};

class Matrix4x3 : public Matrix
{
public:
	Matrix4x3(WorldPointf, WorldPointf, WorldPointf, WorldPointf);
};


#else
	#error File already included
#endif
