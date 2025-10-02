/**************************************************************************

  File:		appctrl.cpp
  Created:	11-Dec-97


  Implementation of class AppCtrl


**************************************************************************/


#include <assert.h>

#include <string>

#include <windows.h>

#include "warningset.h"

#include "exception.h"
#include "window.h"
#include "keystate.h"
#include "sizestate.h"
#include "ownerdraw.h"
#include "menu.h"
#include "ctrl.h"
#include "statusbar.h"
#include "mdimenus.h"
#include "app.h"

#include "appctrl.h"
#include "resstrng.h"

void StatusBarOn::ShowPrompt(int id)
{
	const int BfSize = 256;
	ResString str(BfSize, id);
	_statusBar.SetText(str);
}
