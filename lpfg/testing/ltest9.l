#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module X;

Axiom: A X SB B C D SB X EB E EB X C;

Start: { Backward(); }

ignore: X;

VerifyString: A X SB B C D SB X EB E EB X D;

A() < C() :
{
   produce D;
}
