#define STEPS 450
#define dt 0.05

#define PL 1.000000
#define MAX_INT_LEN 0.500000

Lsystem: 1
derivation length: STEPS
Axiom: -(5)/(20)#(0.12)A(0)

A(t) --> A(t+dt)
I(t) --> I(t+dt)
L(t) --> L(t+dt)

decomposition
A(t) : {t=t-PL;} t>0 --> I(t)[L(t)][/(180)L(t)]/(80.0)!A(t)

homomorphism

I(t) --> F(MAX_INT_LEN*func(1,t/15))

L(t) : 1 {len = func(2,t/10); 
	  wid = func(3,t/20);
	  ang = 90*func(4,t/30);
	  col = 32+floor(31*func(5,t/20));} 
	--> &(ang);(col) @Tx(1) ~l(1.0*wid,0.75*len,len)

A(t) --> f(1.5)

endlsystem
