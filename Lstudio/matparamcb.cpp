#include <fw.h>
#include <glfw.h>

#include "objfgvedit.h"
#include "prjnotifysnk.h"
#include "contmodedit.h"

#include "matparamcb.h"
#include "materialedit.h"


void MaterialParamCallback::ColorChanged(COLORREF clr, bool final)
{
	switch (_parameter)
	{
	case eAmbient :
		_pEdit->UpdateAmbient(clr, final);
		break;
	case eDiffuse :
		_pEdit->UpdateDiffuse(clr, final);
		break;
	case eEmission :
		_pEdit->UpdateEmission(clr, final);
		break;
	case eSpecular :
		_pEdit->UpdateSpecular(clr, final);
		break;
	case eShininess :
		_pEdit->UpdateShininess(clr, final);
		break;
	case eTransparency :
		_pEdit->UpdateTransparency(clr, final);
		break;
	default :
		assert(!"Invalid parameter");
	}
}
