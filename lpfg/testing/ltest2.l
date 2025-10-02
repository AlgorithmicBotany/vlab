#include <lpfgall.h>

derivation length: 1;


module A;
module B;
module C;
module D;
module E;

Axiom: A SB EB B C A;

Start: { Backward(); }


VerifyString: D SB EB B C A;

A() > B() :
{
   produce D;
}
