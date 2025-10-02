#include <lpfgall.h>

#define ANGLE 60

derivation length: 4;

Axiom: Left(90) F(1.0);

F(v) :
{
	produce 
		F(v) Right(ANGLE)
		F(v) Left(ANGLE) 
		Left(ANGLE) F(v)
		Right(ANGLE) F(v);
} 
