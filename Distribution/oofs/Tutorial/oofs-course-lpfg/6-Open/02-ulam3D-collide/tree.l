#include <lpfgall.h>

#define VIGOR 0.5 /* arbitrary value between 0 and 1 */

#define LENGTH 1
#define WIDTH 0.5
#define ANGLE 90

derivation length: 15;

module A();
module I();

Axiom: 	SetWidth(WIDTH)
	SB()              I() A() EB()
	SB() TurnAround() I() A() EB()
	SB() Right(ANGLE) I() A() EB()
	SB()  Left(ANGLE) I() A() EB()
	SB()    Up(ANGLE) I() A() EB()
	SB()  Down(ANGLE) I() A() EB();

f(l)E1(x):
{
	if (x == 1)
/* replace a tentacle that is allowed to grow
   with a line terminated by an apex */
		produce IncColor() I() A();
	else if (x == 0)
/* tentacle that is not allowed to grow persists */
		produce f(l) E1(VIGOR);
}


decomposition:

maximum depth: 1;

/* replace an apex by five tentacles with sensors */

A():
{
	produce	SB() Right(ANGLE) f(LENGTH) E1(VIGOR) EB()
		SB()  Left(ANGLE) f(LENGTH) E1(VIGOR) EB()
		SB()    Up(ANGLE) f(LENGTH) E1(VIGOR) EB()
		SB()  Down(ANGLE) f(LENGTH) E1(VIGOR) EB()
		SB()              f(LENGTH) E1(VIGOR) EB();
}


interpretation:

maximum depth: 1;

I():
{
	produce F(LENGTH) SB() Down(90) Circle(0.2) EB();
}
