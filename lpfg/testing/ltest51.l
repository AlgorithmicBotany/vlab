#include <lpfgall.h>

derivation length: 7;
ignore: Left Right;

Start: { Backward(); }

module Init;
module I(int, float);
module Lateral;

Axiom: Init;

ring L-system: 1;


I(i, length) > I(ri, rlength) :
{
	if (ri==2)
		produce I(2, length);
	else
		produce I(1, length);
}


decomposition:
maximum depth: 2;

Init() :
{
	const int kSegments = 4;
	nproduce Lateral I(2, 1.0) Left(360.0/kSegments);
	for (int iSegment=1; iSegment<kSegments; ++iSegment)
	{
		nproduce Lateral I(1, 1.0) Left(360.0/kSegments);
	}
	produce;
}

Lateral() :
{
	produce SB Left(15) I(1, 0.25) EB
			SB Right(15) I(1, 0.25) EB;
}

interpretation:

I(i, length) :
{
	produce SetColor(i) F(length);
}
