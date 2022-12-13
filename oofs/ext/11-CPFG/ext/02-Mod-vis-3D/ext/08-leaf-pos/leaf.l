#define STEPS 800
#define dt 0.05

#define PL 1.500000
#define DURATION 15
#define TOTAL 15

#define ANGLE nran(80,2)
#define INT_SIZE(n) func(3,n/TOTAL)
#define LEAF_SIZE(n) func(4,n/TOTAL)

Lsystem: 1
derivation length: STEPS
/* Start: {srand(SEED);} */

Axiom: -(5)#(0.05)A(0,0)

A(t,n) --> A(t+dt,n)
I(t,x) --> I(t+dt,x)
L(t,n,x) --> L(t+dt,n,x)

decomposition
A(t,n) : {t=t-PL;} t>0 && n<TOTAL --> 
	I(t,INT_SIZE(n))
	[+(ANGLE)L(t,n,LEAF_SIZE(n))]
	[-(ANGLE)L(t,n,LEAF_SIZE(n))]
	A(t,n+1)
A(t,n) : {t=t-PL;} t>0 && n>=TOTAL --> 
	I(t,INT_SIZE(n))L(t,n,LEAF_SIZE(n))

homomorphism
I(t,x) --> F(x*func(1,t/DURATION))
L(t,n,x) --> ;~l(x*func(2,t/DURATION))

endlsystem
