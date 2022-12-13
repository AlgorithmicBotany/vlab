#define STEPS 400
#define dt 0.02

#define D 2
#define R 1.5

Lsystem: 1
derivation length: STEPS
Axiom: A(0)

A(t) : {t1=t+dt;} t1<D --> A(t1)
A(t) : {t1=t+dt;} t1 >= D {t2=t1-D; t3=t1-1;} --> 
	I(0.5*R^t2)[+A(t2)][-A(t2)]I(0.5*R^t2)
	A(t3)
I(x) --> I(x*R^dt)

homomorphism

A(t) --> ;F(t)
I(x) --> F(x)

endlsystem
