#ifndef __SPHERE_H__
#define __SPHERE_H__


class Sphere
{
public:
	Sphere(float r)
	{ 
		assert(r>0.0f);
		_radius = r; 
	}
	void RayOnSurface(WorldPointf&) const;
private:
	WorldPointf _center;
	float _radius;
};


#endif
