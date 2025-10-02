#define STEPS 400
#define dt 0.05

#define PL 0.500000
#define MAX_INT_LEN 0.500000
#define MAX_LEAF_LEN 1.500000
#define DURATION 20

Lsystem: 1
derivation length: STEPS
Axiom: #(0.05)A(0)

A(t) --> A(t+dt)
I(t) --> I(t+dt)
L(t) --> L(t+dt)

decomposition
A(t) : {t=t-PL;} t>0 --> 
	I(t)[+(45)L(t)][-(45)L(t)]A(t)

homomorphism

I(t) --> F(MAX_INT_LEN*func(1,t/DURATION))
L(t) --> ;F(MAX_LEAF_LEN*func(2,t/DURATION))

endlsystem
