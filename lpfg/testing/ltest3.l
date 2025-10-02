#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;

Axiom: A SB A B C EB SB EB B C;

Start: { Backward(); }

VerifyString: D SB D B C EB SB EB B C;

A() > B() :
{
   produce D;
}
