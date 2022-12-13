#define NUMBER 500
#define RADIUS 0.91

Lsystem: 1
derivation length: 2

Axiom: A(0)

C(n) > ?E(x): x==0 --> ;(1+floor(253*n/NUMBER))@c(RADIUS)
C(n) > ?E(x): x==1 --> ;(255){+F-F-F+|+F-F-F}

decomposition
maximum depth: 1000

A(n) : n < NUMBER --> 
	[+(n*137.5)f(0.5*n^0.5)C(n)?E(n)]A(n+1)

endlsystem
