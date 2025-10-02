#include <fw.h>
#include <gl\gl.h>

#include "worldpoint.h"
#include "viewbox.h"

ViewBox::ViewBox()
{
	_xmin = -1.0f;
	_xmax = 1.0f;
	_dx = 2.0f;
	_ymin = -1.0f;
	_ymax = 1.0f;
	_dy = 2.0f;
	_zmin = -1.0f;
	_zmax = 1.0f;
	_dz = 2.0f;
}
