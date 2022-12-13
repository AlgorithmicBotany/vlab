#include "lpfgall.h"

module Cursor();

MouseStatus ms;			// mouse status

derivation length: 1;

Start: {RunSimulation();}

StartEach:
{
	ms = GetMouseStatus();
}

Axiom: SetColor(2) SetWidth(0.3) Cursor();

production:

Cursor() :
{
	if(ms.lbPushed) // start a line
		produce MoveTo3d(ms.atMiddle) Cursor();
	if(ms.lbDown)	// continue drawing
			produce LineTo3d(ms.atMiddle) Cursor();
}
