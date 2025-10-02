#define SIZE 2
#define NR_INIT 3000
#define NR_INCR 0
#define R_INIT 0.0003
#define R_INCR 0.05
#define R_MAX 0.1

/* plant states:
	0 - young seedling
	1 - dominated
	2 - old
*/

Lsystem: 1
derivation length: 1
Consider: ?E
Axiom: ;(1)&(90)F(SIZE)-F(SIZE)-F(SIZE)-F(SIZE)-;(2)X(NR_INIT)

/* A young plant that is not dominated grows */

[f(y)-f(z)P(r,s)?E(x)] : x > 0 && r<R_MAX && s==0
	{r = r * (1+ran(R_INCR));} --> [f(y)-f(z)P(r,0)?E(r)]

/* A plant that is dominated changes state to 1 ... */

[f(y)-f(z)P(r,s)?E(x)] : x==0 && s==0 --> [f(y)-f(z)P(r,1)?E(r)]

/* ... and dies in the next simulation step */

[f(y)-f(z)P(r,s)?E(x)] : s==1 --> *

/* A plant that has reached its maximum size changes state to 2 ("old") */

[f(y)-f(z)P(r,s)?E(x)] : r>=R_MAX && s==0 --> [f(y)-f(z)P(r,2)?E(r)]

/* An old plant does not grow anymore */

[f(y)-f(z)P(r,s)?E(x)] : s==2 --> [f(y)-f(z)P(r,2)?E(r)]

decomposition
maximum depth: 15000

/* Create n plants P at random positions in the square
   of dimensions SIZE x SIZE. */

X(n) : n>0 {r = 0.003 + ran(R_INIT);} 
	--> [f(ran(SIZE))-f(ran(SIZE))P(r,0)?E(r)]X(n-1)

homomorphism

/* Visualize the plant as a circle of radius r, with the
   color representing state s */

P(r,s) --> ;(2+s)@c(2*r)

endlsystem
