#include <fw.h>

#include "worldpoint.h"
#include "sphere.h"


void Sphere::RayOnSurface(WorldPointf& p) const
{
	p -= _center; 
	float v = p.Len();
	p /= (v/_radius);
	p += _center;
}
