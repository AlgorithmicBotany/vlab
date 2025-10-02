#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;
module X;

Axiom: A X X;

Start: { Backward(); }

ignore: X;

VerifyString: A X X;

A() > B() :
{
   produce D;
}
