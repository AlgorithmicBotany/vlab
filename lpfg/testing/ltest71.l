#include <lpfgall.h>

derivation length: 0;

int iCounter = 0;

module A;


Axiom: A;

interpretation:
A() : { ++iCounter; }

A() : 
{ 
	if (1==iCounter)
		produce F(1);
}
