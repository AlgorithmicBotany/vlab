#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B C;

Start: { Backward(); }


VerifyString: E B C;


A() :
{
	// RContextReset() && AcceptContext(B_id) && GetModuleB() && AdvanceContext() && ConfirmContext()
	if (InRightContext(B()))
	{
		produce E;
	}
}

