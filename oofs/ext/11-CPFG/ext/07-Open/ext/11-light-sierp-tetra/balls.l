#define D 1		/* sphere diameter */

Lsystem: 1
derivation length: 2
Start: {R = D/2;}	/* sphere radius */

Axiom: X

X --> K(8)

?E(r)S(s,i) : 1 {printf("intensity = %lf\n", r);} --> S(s,r)

decomposition
maximum depth: 7

K(s) : s > D -->
	[K(s/2)]
	[&(90)f(s)^(90)K(s/2)]
	[-(90)f(s)+(90)K(s/2)]
	[f(s)K(s/2)]

K(s) : s <= D --> ?E(s)S(s,0)

homomorphism

S(s,r) --> ;(floor(1+254*r))@O(2*s)

endlsystem


