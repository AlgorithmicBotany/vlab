#include <lpfgall.h>

derivation length: 1;


module A(float);
module B(float);
module C(float);
module D(float);
module E(float);
module H(float);

Axiom: A(0.1f) B(0.2f) SB C(0.3f) EB SB D(0.4f) E(0.5f) EB F(0.6f) G(0.7f) H(0.8f);


VerifyString: A B SB C EB SB H E EB F G H;

Start: { Forward(); }

G(gf) > H(hf) :
{
}

D(df) > E(ef) :
{
	float af, bf;
	if (InNewLeftContext(A(af) B(bf)))
	{
		if (af+bf > 0.29f)
			produce H(df+ef);
	}
}

