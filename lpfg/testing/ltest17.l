#include <lpfgall.h>

derivation length: 1;


module A;
module B(int);
module C(int);
module D(int);
module E(int);
module H;


Axiom: A B(1) C(2) SB Left(23) Right(15) EB D(3) E(4) SB Up(66) Down(28) EB F(5.5) SB EB SB EB SB F(34) EB G(6.5);


Start: { Backward(); }

VerifyString: A B C SB Left Right EB H E SB Up Down EB F SB EB SB EB SB F EB G;


C(i1) < D(i2) > E(i3) :
{
	float f4, f5;
	if (InRightContext(F(f4) G(f5)))
	{
		int i6;
		if (InLeftContext(A() B(i6)))
		{
			if (i3 == 4 && i1 == 2 && i6 == 1 && f5 > 6.45f)
				produce H;
		}
	}
}

