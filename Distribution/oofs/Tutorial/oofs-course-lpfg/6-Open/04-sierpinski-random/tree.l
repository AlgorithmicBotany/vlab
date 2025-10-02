#include <lpfgall.h>

#define VIGOR ran(1)

#define LENGTH 1
#define WIDTH 3

module A();
module B();
module tentA();
module tentB();
module NoGrow();

derivation length: 31;

Axiom: 	SetWidth(WIDTH) TurnAround()
      	SB() Right(30) F(LENGTH) A() EB()
      	SB()  Left(30) F(LENGTH) B() EB();

tentA() E1(x):
{
	if (x == 1)
/* replace a tentacle that is allowed to grow
   with a line terminated by an apex */
		produce IncColor() F(LENGTH) A();
	else if (x == 0)
/* display a tentacle that is not alllowed to grow */
		produce NoGrow();
}

tentB() E1(x):
{
	if (x == 1)
/* replace a tentacle that is allowed to grow
   with a line terminated by an apex */
		produce IncColor() F(LENGTH) B();
	else if (x == 0)
/* display a tentacle that is not alllowed to grow */
		produce NoGrow();
}


decomposition:

maximum depth: 1;

/* replace an apex by tentacles with sensors */

A():
{
	produce SB()          tentA() E1(VIGOR) EB()
		SB() Left(60) tentB() E1(VIGOR) EB();
}

B():
{
	produce SB() Right(60) tentA() E1(VIGOR) EB()
		SB()           tentB() E1(VIGOR) EB();
}


interpretation:

maximum depth: 1;


/* tentA and tentB are interpreted as f(LENGTH) in order to
   properly set the position of the E1(x) modules before the
   environmental step */
tentA():
{
	produce f(LENGTH);
}

tentB():
{
	produce f(LENGTH);
}

NoGrow():
{
	produce SetWidth(1) SetColor(255) F(LENGTH);
}

