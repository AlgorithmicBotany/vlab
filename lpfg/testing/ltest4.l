#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;

Axiom: A B C;


Start: { Backward(); }

VerifyString: D B C;

A() > B() :
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
