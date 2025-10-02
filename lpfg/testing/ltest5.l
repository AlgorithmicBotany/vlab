#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: A B B C;


Start: { Backward(); }

VerifyString: D B B C;

A() > B() B():
{
	if (RContextReset() && AcceptContext(C_id) && GetModuleC() && AdvanceContext() && ConfirmContext())
	{
		produce D;
	}
	else
	{
		produce E;
	}
}

