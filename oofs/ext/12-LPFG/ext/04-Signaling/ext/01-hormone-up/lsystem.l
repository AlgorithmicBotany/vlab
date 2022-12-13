#include <lpfgall.h>

#define LENGTH 1
#define WIDTH 5
#define ANGLE 45.0

derivation length: 1;

module A();
module B();
module X();

ignore: Left Right;


Axiom: SetWidth(WIDTH) X();

/* predefined structure */

X():
{
	produce
		B()
		SB() Right(ANGLE) A() EB() A()
		SB()  Left(ANGLE) A() EB() A()
		SB() Right(ANGLE) A() EB() A()
		SB()  Left(ANGLE) A() EB() A()
		SB() Right(ANGLE) A() EB() A();
}

/* acropetal flow */

B() < A():
{
	produce B();
}

interpretation:

A():
{
	produce SetColor(1) F(LENGTH);
}

B():
{
	produce SetColor(2) F(LENGTH);
}
