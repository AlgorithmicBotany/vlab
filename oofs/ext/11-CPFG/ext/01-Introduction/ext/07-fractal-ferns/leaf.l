#define STEPS 18
#define D 3
#define R 1.28

/* Sample parameter values: 
	STEPS== 4, D==0, R==2
	STEPS==11, D==1, R==1.5
	STEPS==18, D==3, R==1.28
*/

Lsystem: 1
derivation length: STEPS
Axiom: A(0)

A(t) : t<D  --> A(t+1)
A(t) : t==D --> I(1)[+A(0)][-A(0)]I(1)A(t)
I(x) --> I(R*x)

homomorphism

A(t) --> ;F(t)
I(x) --> F(x)

endlsystem
 
