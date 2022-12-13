#include <lpfgall.h>

#define STEPS 180
#define dt 0.05

#define PL 1.00000   /* plastochron */
#define D 10         /* elongation time */
#define ANG 25       /* branching angle */

derivation length: STEPS;

module A(float);
module I(float);
module K(float);

Axiom: SetWidth(0.03) RollL(60) A(0);

production:

A(t): { produce A(t+dt); }
I(t): { produce I(t+dt); }
K(t): { produce K(t+dt); }

decomposition:

A(t):
{
	t = t - PL;
	
	if(t>0)
		produce
			I(t)
			RollR(90)
			SB() Left(ANG) A(t+0.3) EB()
			SB() Right(ANG) A(t-0.3) EB()
			K(t);
}

interpretation:
maximum depth: 3;

I(t):
{
	produce
		F(func(internode, t/D))
		SB() Down(45+30*t/D) Surface(0, 0.2*func(internode, t/D)) EB()
		SB() RollR(180)Down(45+30*t/D) 
			Surface(0, 0.2*func(internode, t/D)) EB();
}

K(t):
{
	produce
		F(0.4*func(flower, t/D)) 
		IncColor() SetWidth(0.1*func(flower, t/D)) Sphere0();
}
