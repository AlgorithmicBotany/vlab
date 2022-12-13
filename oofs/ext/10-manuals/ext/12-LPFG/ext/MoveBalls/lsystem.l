#include "lpfgall.h"

derivation length: 1;

module AddBall();		// generates new balls
module Ball(V3d, int);	// position, currencly selected?

MouseStatus ms;

/* Flag "moving" is set to 1 after selecting a module, 
   and set to 0 after releasing the mouse button */

int moving;

Axiom: AddBall;

/* Initial state: there is nothing to move.
   Simulation starts running automatically */

Start: {RunSimulation(); moving=0;}

StartEach: 
{
	ms = GetMouseStatus();
}

/* An existing ball has been selected. Adjust its position to
   that of the mouse. Set flag "moving" to 1 to prevent the 
   addition of another ball at the same location. */

MouseIns() Ball(pos, selected) :
{
	moving = 1;
	produce Ball(ms.atMiddle, 1) ;
}

/* Move the selected ball as long as the mouse button
   is not released. */

Ball(pos, selected):
{
	if (selected) {
		moving = !ms.lbReleased;
		produce Ball(ms.atMiddle, moving); 
	}
}

/* Add sphere if mouse clicked outside any existing sphere. */

AddBall():
{
	if(!moving && ms.lbPushed) {
		moving = 1;
		produce Ball(ms.atMiddle, 1) AddBall();
	}
}

interpretation:

Ball(pos, selected) : {produce MoveTo3d(pos) SetColor(1+selected) Sphere(1);}
