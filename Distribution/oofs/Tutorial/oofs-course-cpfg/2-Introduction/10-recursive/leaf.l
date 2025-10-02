#define T 8

#define D 2
#define R 1.5

Lsystem: 1
derivation length: 1
Axiom: A(T)

homomorphism
maximum depth: 100

A(t) : t >= D --> M(t-D) A(t-1)

M(t) : 1 {x=0.5*R^t;} --> 
	I(x)[+A(t)][-A(t)]I(x)

A(t) : t < D --> ;F(t)
I(x) --> F(x)

endlsystem
