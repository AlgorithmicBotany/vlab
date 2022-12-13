#define STEPS 180
#define dt 0.05

#define PL 1.00000   /* plastochron */
#define D 10         /* elongation time */
#define ANG 25       /* branching angle */

Lsystem: 1
derivation length: STEPS
Axiom: #(0.03)\(60)A(0)

A(t) --> A(t+dt)
I(t) --> I(t+dt)
K(t) --> K(t+dt)

decomposition

A(t) : {t=t-PL;} t>0 --> 
	I(t)/(90)[+(ANG)A(t)][-(ANG)A(t)]K(t)

homomorphism
maximum depth: 3

I(t) --> F(func(1,t/D))

K(t) --> F(0.4*func(2,t/D));@O(0.1*func(2,t/D))

endlsystem
