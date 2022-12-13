#include <lpfgall.h>

#define ANGLE 60

derivation length: 4;

Axiom: Right(90) F(1.0);

F(v) :
{
	produce F(v) Left(ANGLE)
		F(v) Right(2*ANGLE) F(v)
		Left(ANGLE) F(v);
} 
