#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A SB B EB C;


VerifyString: E SB B EB C;


A() :
{
	if (InRightContext(SB() B() EB() C()))
		produce E;
}
