#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module H;


Axiom: D A B B C;


Start: { Backward(); }

VerifyString: D A B B H;


B() B() < C() :
{
	bool firstpass = false;
	if (LContextReset() && AcceptContext(A_id) && GetModuleA() && AdvanceContext() && ConfirmContext())
	{
		firstpass = true;
	}

	if (firstpass && LContextReset() && AcceptContext(D_id) && GetModuleD() && AdvanceContext() && ConfirmContext())
	{
		produce H;
	}
}

