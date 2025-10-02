#define SIZE 2		/* field size */
#define NR_INIT 100	/* initial number of plants */
#define NR_INCR 100	/* number of new plants per step */
#define R_INIT 0.001	/* initial plant radius */
#define R_INCR 0.05	/* max radius increment is 1 + R_INCR */
#define R_MAX 0.05	/* max plant radius */
#define SEED 22		/* random number generator seed */
#define NSIZES 30	/* number of plant size classes
			   in the x-y plot */
/* plant states:
	0 - young seedling
	1 - dominated
	2 - old
*/

Lsystem: 1
derivation length: 1
Consider: ?E
define: {array sizes[NSIZES+4];}
Start: {i=0;step=0;srand(SEED);}
EndEach: {step=step+1;printf("step %.0f\n",step);}
Axiom:  B
	 ;(1)&(90)#(SIZE*0.01)F(SIZE)-F(SIZE)-F(SIZE)-F(SIZE)
	 -;(2)X(NR_INIT)
	,(4)K(0)A

/* Set the number of plants to be added to the field. */

X(n) : n==0 --> X(NR_INCR)

/* A young plant that is not dominated grows. */

[f(y)-f(z)P(r,s)?E(x)] : x > 0 && r<R_MAX && s==0
	{r = r * (1 + ran(R_INCR));} --> [f(y)-f(z)P(r,0)?E(r)]

/* An old plant (that has reached the maximum size size R_MAX)
   does not grow and survives with probability 0.8 */

[f(y)-f(z)P(r,s)?E(x)] : r>=R_MAX && ran(1) < 0.8 --> [f(y)-f(z)P(r,2)?E(r)]

/* Plant that is dominated enters state 1 and will die in the next step. */

[f(y)-f(z)P(r,s)?E(x)] : s==0 && x==0 --> [f(y)-f(z)P(r,1)?E(r)]

/* Plant that dies is removed from the field. */

[f(y)-f(z)P(r,s)?E(x)] : x==0 || r>=R_MAX || s==1 --> *

decomposition
maximum depth: 25000

/* Add n plants P at random positions to the field. */

X(n) : n>0 {r = 0.01 + ran(R_INIT);} 
	--> [f(ran(SIZE))-f(ran(SIZE))P(r,0)?E(r)]X(n-1)

homomorphism
maximum depth: NSIZES+2

/* Set the attributes of the x-y plot. */

#define SIZEG (SIZE*0.9)
#define XPOS SIZE+SIZEG*0.19
#define YMAX 300
#define XMIN 0.01
#define XMAX R_MAX 

/* Set the number of plants in each size class to 0 at the beginning of
   a simulation step.  */

B : * {i=0;while(i<=NSIZES){sizes[i]=0;i=i+1;}} --> *

/* Draw a circle representing a plant in the field, 
   and increment the count in the corresponding size class. */

P(r,s) : * { i=floor((r-XMIN)/(XMAX-XMIN)*NSIZES+0.0);
	     sizes[i] = sizes[i]+1;}
	--> ;(2+s)@c(2*r)

/* Draw dots representing the counts in the x-y plot. */ 

K(n) : n<=NSIZES --> [@M(XPOS+0.1+n/NSIZES*SIZEG,0,
			 -sizes[n]/YMAX*SIZEG)@c(SIZEG/40)]
		     K(n+1)

/* Draw and label the coordinate system axes. */

A -->   @M(XPOS,0,0)F(SIZEG*1.08)T(SIZEG*0.035)
	[@M(XPOS-0.28,0,-SIZEG*1.1)@L("number")]
	Y(0,3)
	@M(XPOS,0,0)-(90)F(SIZEG*1.11)T(SIZEG*0.035)
	X(0,4)
	@M(XPOS+SIZEG*1.03-0.07,0,0.3)@L("radius")

Y(n,max) : n<=max --> [@M(XPOS-SIZEG*0.03,0,-SIZEG*n/max)
		-(90)G(SIZEG*0.03)
		@M(XPOS-SIZEG*(0.15),0,-SIZEG*(n/max-0.02))
		@L("%3.0f",n/max*YMAX)
	      ]Y(n+1,max)

X(n,max) : n<=max --> [@M(XPOS+n/max*SIZEG,0,0)
		-(90)G(SIZEG*0.03)
		@M(XPOS-SIZEG*0.07+SIZEG*(n/max),0,SIZEG*0.09)
		@L("%.2g",XMIN+n/max*(XMAX-XMIN))
	      ]X(n+1,max)

T(len) --> [-(155)F(len)][+(155)F(len)]

endlsystem
