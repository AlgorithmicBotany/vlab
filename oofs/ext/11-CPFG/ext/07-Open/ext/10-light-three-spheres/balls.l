#define D 	1	/* distance between spheres */
#define RATIO	0.9	/* sphere diam as % of distance between spheres */

Lsystem: 1
derivation length: 1
Start: {R = 0.9*D;}	/* sphere radius */

Axiom:  ?E(R)f(D)?E(R)f(D)?E(R)

?E(r) : 1 {printf("intensity = %lf\n", r);} --> ?E(1)S(r)

S(r) --> *

homomorphism

S(r) --> ;(floor(255*r))@O(R)

endlsystem


