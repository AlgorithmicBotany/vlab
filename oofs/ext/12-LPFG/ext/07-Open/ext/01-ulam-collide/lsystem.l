#include <lpfgall.h>

#define VIGOR 0.5 /* arbitrary value between 0 and 1 */

#define LENGTH 1
#define WIDTH 0.5
#define ANGLE 90

derivation length: 15;

module A();

Axiom: 	SetWidth(WIDTH)
	SB()              F(LENGTH) A() EB()
	SB() Right(ANGLE) F(LENGTH) A() EB()
	SB() Left(ANGLE)  F(LENGTH) A() EB()
	SB() TurnAround() F(LENGTH) A() EB();

f(l)E1(x):
{
	if (x == 1)
/* replace a tentacle that is allowed to grow
   with a line terminated by an apex */
		produce IncColor() F(l) A();
	else if (x == 0)
/* tentacle that is not allowed to grow persists */
		produce f(l) E1(VIGOR);
}


decomposition:

maximum depth: 1;

/* replace an apex by three tentacles with sensors */

A():
{
	produce	SB() Right(ANGLE) f(LENGTH) E1(VIGOR) EB()
		SB()  Left(ANGLE) f(LENGTH) E1(VIGOR) EB()
		SB()              f(LENGTH) E1(VIGOR) EB();
}

