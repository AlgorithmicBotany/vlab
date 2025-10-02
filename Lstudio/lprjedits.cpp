#include <memory>

#include <fw.h>
#include <glfw.h>

#include "prjnotifysnk.h"
#include "lprjctrl.h"

#include "resource.h"

#include "tedit.h"
#include "lsysedit.h"
#include "viewedit.h"
#include "anytextedit.h"
#include "descredit.h"
#include "animdata.h"
#include "animedit.h"
//#include "stdout.h"
#include "colormapedit.h"

#include "matparamcb.h"

#include "linethcb.h"


#include "objfgvobject.h"
#include "objfgvview.h"
#include "objfgvgallery.h"
#include "objfgvedit.h"

#include "contmodedit.h"

#include "materialedit.h"
#include "surfthumbcb.h"
#include "surfaceedit.h"
#include "contouredit.h"
#include "funcedit.h"
#include "curveedit.h"
#include "panelsedit.h"

#include "lstudioptns.h"

LProjectCtrl::Editors::Editors()
{
	for (int i=0; i<edCount; ++i)
		_arr[i] = 0;
}


void LProjectCtrl::Editors::Create(Window w, HINSTANCE h, PrjNotifySink* pSink)
{
	_arr[edLsystem] = LSystemEdit::Create(w, h);
	if (options.ExternalLsysEdit())
		_arr[edLsystem]->Enable(false);

	_arr[edView] = ViewEdit::Create(w, h);

	_arr[edAnim] = AnimateEdit::Create(w, h);

	_arr[edColormap] = ColormapEdit::Create(w, h, pSink);

	_arr[edMaterial] = MaterialEdit::Create(w, h, pSink);

	_arr[edSurface] = SurfaceEdit::Create(w, h, pSink);

	_arr[edCurve] = CurveEdit::Create(w, h, pSink);

	_arr[edContour] = ContourEdit::Create(w, h, pSink);

	_arr[edFunction] = FuncEdit::Create(w, h, pSink);

	_arr[edAnyText] = AnyTextEdit::Create(w, h, pSink);

	_arr[edPanel] = PanelEdit::Create(w, h, pSink);

	_arr[edDescription] = DescriptionEdit::Create(w, h);
}

