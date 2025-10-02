#include <lpfgall.h>


module M;
module W;
module A;
module B;

derivation length: 1;

consider: M W;

Axiom: SB() f(20) Right(90) f(30) Left(90) EB() SetWidth(2)
	M W M
	SB() 
		TurnAround() f(2.0) Left (90) f(2.0) 
	     	W  M
        EB()
	W M;



W() < M() > W() :
{
	produce A;
}


W() < M() :
{
	produce B;
}


VerifyString: SB f Right f Left EB SetWidth
	M W A
	SB
		TurnAround f Left f
	     	W  B
    EB
	W B;