#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module X;

ignore: X;

Axiom: A X SB EB X B C;

Start: { Backward(); }


VerifyString: D X SB EB X B C;

A() > B() :
{
   produce D;
}
