#include <lpfgall.h>

derivation length: 0;

int iCounter = 0;

module A;
module B;


Axiom: A;

VerifyString: B;

decomposition:
A() : { ++iCounter; }

A() : 
{ 
	if (1==iCounter)
		produce B;
}
