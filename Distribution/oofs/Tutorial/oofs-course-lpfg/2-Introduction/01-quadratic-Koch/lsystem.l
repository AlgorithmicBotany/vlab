#include <lpfgall.h>

#define ANGLE 90
#define WIDTH 2

derivation length: 4;

Axiom: Right(ANGLE) SetWidth(1) SetColor(1) F(1.0);

F(v) :
{
produce Label("TEST");
	produce 	
		F(v) Left(ANGLE) F(v) Right(ANGLE) F(v) Right(ANGLE) F(v) 
		F(v) Left(ANGLE) F(v) Left(ANGLE)  F(v) Right(ANGLE) F(v);
} 
					