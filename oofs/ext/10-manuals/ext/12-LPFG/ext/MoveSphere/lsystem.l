#include "lpfgall.h"

derivation length: 1;

module AddSphere();
module PosSphere(V3d, int);

/* The axiom defines the size of the window (between SB and EB),
sets the color, and defines a polygon (between SP and EP).
*/

Axiom: SB f(20) Right(90) f(20) EB
       SetColor(4) AddSphere;

MouseStatus ms;


StartEach: 
{
	ms = GetMouseStatus();
	if (ms.lbPushed)
		Printf("Pushed\n");
	if (ms.lbReleased)
		Printf("Released\n");
}

production:

/* An existing sphere has been selected.  Position it at the
   location of the mouse.  Set flag to 1 to prevent the addition
   of a new sphere */

MouseIns() PosSphere(pos, selected) :
{
	Printf("MouseIns\n");
	produce PosSphere(ms.atMiddle, 1) ;
}

/* Move the selected sphere.  Set flag to 0 when button released
   to end the operation. */

PosSphere(pos, selected) :
{
	if (selected && ms.lbReleased)
		produce PosSphere(ms.atMiddle, 0) ;
	if (selected && !ms.lbReleased)
		produce PosSphere(ms.atMiddle, 1); 
}

/* Add sphere if mouse clicked outside of any existing sphere. */

AddSphere():
{
 if(ms.lbPushed) {
	Printf("AddSphere\n");
	produce PosSphere(ms.atMiddle, 1);
	}
}

interpretation:

PosSphere(pos, selected) : {produce MoveTo3d(pos) SB SetColor(2+selected) Sphere(1) EB ;}
