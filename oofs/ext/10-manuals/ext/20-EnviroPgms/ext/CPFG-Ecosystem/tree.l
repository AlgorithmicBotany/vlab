#define SIZE 2
#define NR_INIT 150
#define NR_INCR 150
#define R_INIT 0.05
#define R_INCR 0.01
#define R_MAX 0.2

Lsystem: 1
derivation length: 1
Consider: ?E
Axiom: ;(1)&(90)|f(SIZE*0.5)|+f(SIZE/2)
	[;(5)&f(SIZE*0.01)^|f(SIZE*5)|+f(SIZE*5)
		{-f(SIZE*10)-f(SIZE*10)-f(SIZE*10)-f(SIZE*10)}]
	[;(1)-F(SIZE)-F(SIZE)-F(SIZE)-F(SIZE)]
	[;(4){-f(SIZE)-f(SIZE)-f(SIZE)-f(SIZE)}]
	-;(2)X(NR_INIT)

/* add set X of NR_INCR new plants to the  field */

X(n) : n==0 --> X(NR_INCR)

/* plant that is dominated or too old dies */

[f(y)-f(z)P(r)?E(x)] : x==0 || r >= R_MAX --> *

/* otherwise, plant grows */

P(r)?E(x) : * {r = r + ran(R_INCR);} --> P(r)?E(r)

decomposition
maximum depth: 510

/* Create n plants P at random positions in the square 20 x 20 */

X(n) : n>0 {r = 0.01 + ran(R_INIT);} 
	--> [f(ran(SIZE))-f(ran(SIZE))P(r)?E(r)]X(n-1)

homomorphism

/* visualize the plant */

P(r) --> ^#(r/2);(2)F(2*r);(3)@O(2*r)

endlsystem
