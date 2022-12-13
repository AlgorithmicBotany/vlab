#define STEPS 800
#define dt 0.05

#define PL 1.500000
#define DURATION 15
#define TOTAL 10

#define ANGLE nran(80,4)
#define INT_SIZE nran(1,0.05)
#define LEAF_SIZE nran(0.6,0.02)

Lsystem: 1
derivation length: STEPS
/* Start: {srand(SEED);} */

Axiom: -(5)#(0.15)A(0,0)

A(t,n) --> A(t+dt,n)
I(t,x) --> I(t+dt,x)
L(t,n,x) --> L(t+dt,n,x)

decomposition
A(t,n) : {t=t-PL;} t>0 && n<TOTAL --> 
	I(t,INT_SIZE)
	[+(ANGLE)L(t,n,LEAF_SIZE)]
	[-(ANGLE)L(t,n,LEAF_SIZE)]
	A(t,n+1)
A(t,n) : {t=t-PL;} t>0 && n>=TOTAL --> 
	I(t,0.5)L(t,n,LEAF_SIZE)

homomorphism
I(t,x) --> F(x*func(1,t/DURATION))
L(t,n,x) --> ;~l(x*func(2,t/DURATION))

endlsystem
