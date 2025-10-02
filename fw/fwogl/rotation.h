#ifndef __ROTATION_H__
#define __ROTATION_H__


#ifndef M_PIf
#define M_PIf 3.14159265358979323846f
#endif


class Rotation
{
public:
	Rotation(float angle, float xrot, float yrot, float zrot)
	{
		_angle = angle;
		_xrot = xrot;
		_yrot = yrot;
		_zrot = zrot;
	}
	void Apply()
	{
		glRotatef(_angle, _xrot, _yrot, _zrot);
	}
	void ApplyIndividually()
	{
		glRotatef(_zrot, 0.0, 0.0, 1.0);
		glRotatef(_yrot, 0.0, 1.0, 0.0);
		glRotatef(_xrot, 1.0, 0.0, 0.0);
	}
	void Reset()
	{
		_angle = 0.0f;
		_xrot = 
			_yrot =
			_zrot = 0.0f;
	}
	float GetAngle() const
	{ return _angle; }
	float GetRotX() const
	{ return _xrot; }
	float GetRotY() const
	{ return _yrot; }
	float GetRotZ() const
	{ return _zrot; }
	void SetAngle(float a)
	{ _angle = a; }
	void SetRotX(float x)
	{ _xrot = x; }
	void SetRotY(float y)
	{ _yrot = y; }
	void SetRotZ(float z)
	{ _zrot = z; }
	void AddX(float x)
	{ _xrot += x; }
	void AddY(float y)
	{ _yrot += y; }
	void AddZ(float z)
	{ _zrot += z; }
private:
	float _angle;
	float _xrot, _yrot, _zrot;
};


template<typename f>
f R2D(f radians)
{ return radians*180.0f/M_PIf; }


#endif
