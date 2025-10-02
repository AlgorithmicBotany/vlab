#define STEPS 400
#define dt 0.02

#define D 2
#define R 1.5

#define RATE (R^dt)

Lsystem: 1
derivation length: STEPS
Axiom: A(0)
A(t) --> A(t+dt)
I(x) --> I(x*RATE)

decomposition
maximum depth: 2
A(t) : t >= D --> /(90) M(t-D) A(t-1)
M(t) : 1 {x=0.5*R^t;} 
	--> I(x)[+A(t)][-A(t)]I(x)

homomorphism
A(t) --> ;F(t)
I(x) --> F(x)

endlsystem
