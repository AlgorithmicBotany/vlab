#include <lpfgall.h>

#define ANGLE 90

derivation length: 4;

Axiom: Right(90) F(1.0);

F(v) :
{
	produce F(v) Left(ANGLE) F(v) Right(ANGLE) F(v) Right(ANGLE) F(v) 
		F(v) Left(ANGLE) F(v) Left(ANGLE)  F(v) Right(ANGLE) F(v);
} 
					