#define STEPS 800
#define dt 0.02

#define D 2.5
#define R 1.3

#define RATE (R^dt)

Lsystem: 1
derivation length: STEPS
Axiom: A(0)
A(t) --> A(t+dt)
B(t) --> B(t+dt)
I(x) --> I(x*RATE)
decomposition
maximum depth: 2
A(t) : t >= D --> M(t-D) B(t-1)
B(t) : t >= D --> N(t-D) A(t-1)

M(t) : 1 {x=0.5*R^t;} --> 
	I(x)[+A(t)]I(x)
N(t) : 1 {x=0.5*R^t;} --> 
	I(x)[-B(t)]I(x)
homomorphism
A(t) --> ;F(t)
B(t) --> ;;F(t)
I(x) --> F(x)
endlsystem
