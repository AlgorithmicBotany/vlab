/* Recursive model of Congo 3, formulated using the recursive rules */
/* Calgary, June 21, 1997, revised Dec. 13, 2020 */

/* Selection of the axiom - determines the part to be modeled.
	0 - the whole branch (not supported in this version)
	1 - the whole twig,
	2 - an inflorescence (flower cluster),
	3 - a floret
*/

#define A_SW 1
#define SHOW_PETALS 1

#define M 4    /* maximum branch order + 1 */
#define N 13   /* maximum number of internodes in an axis */

#define STEM_LEN 70 

#define TWIST 85

/* The values of branching angles, diameter of branches, and elasticity are
   interpolated along the axes using the values given below */

#define D_INIT 1
#define D_EXP 0.25

#define P_SIZE 3.2
#define P_ANG 40

#define br_ang(o,i) (100*func(1,i/(N-1)))
#define elas(o,i)  (0.1*func(2,i/(N-1)))

Lsystem: 1
consider: I[]

// The model is based on raw data for Congo 3.  

define: {
	// Maximum number of internodes in an axis of order o (main is 0)
	array max[M] = 
		{13,7,2,1}; 

   	// Lengths of internodes axis of order 0 (main), 1, 2, and 3.  
	// Nonexisting internodes (exceeding the number of internodes in an axis)
    // are assigned the length of 0.
	array len[M][N] = 
		{15.5,13.1,38.9,19.8,33.3,21.2,25.6,16.2,12.4,9.0,1.4,0,0,
          8.8, 8.0, 7.7, 4.1, 4.5, 4.4, 1.1, 0  , 0  ,0  ,0,  0,0,
	  	  4.0, 4.9, 0  , 0  , 0  , 0  , 0  , 0  , 0  ,0  ,0,  0,0,
	  	  5.0, 0  , 0  , 0  , 0  , 0  , 0  , 0  , 0  ,0  ,0,  0,0};

	// Number of internodes in the axis of lateral branch of order o, 
	// positioned at the distal end ot internode i of parent axis.
	array cnt[M-1][N] = 
		  {7,  7,   7,   7,   6,   6,   6,   5,   4,  3,  2,  1, 0,
	 	   2,  2,   1,   1,   1,   1,   0,   0,   0,  0,  0,  0, 0,
	 	   1,  0,   0,   0,   0,   0,   0,   0,   0,  0,  0,  0, 0};
	}

derivation length: N

Axiom: AXIOM

// Calculating internode width according to the pipe model

I(o,i,w) > [I(o1,i1,w1)=][I(o2,i2,w2)=]I(o3,i3,w3) --> I(o,i,w1+w2+w3)
I(o,i,w) > [I(o1,i1,w1)=][I(o2,i2,w2)=] --> I(o,i,w1+w2)

decomposition
maximum depth: 100

AXIOM : A_SW == 1 --> ^(10) /(20) -(30) ;(3)O 
AXIOM : A_SW == 2 --> ;(2)A(0,0)
AXIOM : A_SW == 3 --> ;(2)K 

O --> I(0,0,0)F(STEM_LEN);(2) [/+(20,0)/(90)A(0,0)][-(20,0)/(90)A(0,0)]

/* A(o,i) represents an apex
   o  -  branch order (main axis has order 0),
   i  -  internode number along the axis (0 at the base)

   For first-order branches we assume that branch with k metamers
   is the top part of a branch with k+1 metamers.
   The number of internodes in the first-order
   branch is a function of branch position, represented by the array
   cnt[o][i].  This is repeated for higher order branches.
*/

A(o,i) : i < max[o]-1 --> 
	I(o,i,0)
		[+(br_ang(o,i)) /(90) A(o+1,max[o+1]-cnt[o][i])]
		[-(br_ang(o,i)) /(90) A(o+1,max[o+1]-cnt[o][i])]
	/(TWIST)A(o,i+1)

A(o,i) : i == max[o]-1 --> 
	I(o,i,D_INIT^(1.0/D_EXP))K

homomorphism

I(o,i,d) --> !(d^D_EXP) _(elas(o,i)) F(len[o][i])

K : SHOW_PETALS  --> \\[F[;(4)#(0.1)gG],(1)~c(4)^^^
	[&(P_ANG)~p(P_SIZE)]+++
	[&(P_ANG)~p(P_SIZE)]+++
	[&(P_ANG)~p(P_SIZE)]+++
	[&(P_ANG)~p(P_SIZE)]]

endlsystem
