#define STEPS 300
#define BR_PATTERN 1

#define VANG 45
#define S0 1
#define ANG 40
#define D1 4
#define D2 7
#define LEN1 20
#define LEN2 10
#define LENLEAF 30
#define LENBR 30
#define T1 1 /* bud --> young flower */
#define T2 5 /* young flower --> flower */
#define T3 11 /* flower --> fruit */
#define T4 10 /* fruit reaches maturity */
#define LEAFSCALE 1
#define PETALSCALE 2
#define PHYLLOTAXY 90
#define TROPISM 0.05

#define NR_HORI 10
#define NR_VERT 5

#define dt 0.1

Lsystem: 1
derivation length: STEPS
Axiom: _(TROPISM)/(VANG) &(5) ;(1) A(0)
/* Axiom: B(1) */

A(t) --> A(t+dt)	/* apex */
I(t) --> I(t+dt)	/* internode below the branching point */
J(t) --> J(t+dt)	/* internode above the branching point */
L(t) --> L(t+dt)	/* leaf */
K(t) --> K(t+dt)	/* bud - flower - fruit */
B(dir,t) --> B(dir, t+dt)	/* branching angle */

decomposition

A(t) : BR_PATTERN == 0 && t > 0 --> 
	I(t) [ J(t) K(t)] 
	[@v B(1,t) [ @v L(t)] A(t-D1)] 
	[@v B(-1,t) [ @v L(t)] A(t-D2)]

A(t) : BR_PATTERN == 1 && t > 0 --> 
	I(t) [ J(t) K(t)] \(PHYLLOTAXY)
	[B(1,t) [\ (90) L(t)] A(t-D1)] 
	[B(-1,t) [/ (90) L(t)] A(t-D2)]


homomorphism
maximum depth: 300

B(dir, t) --> +(dir*ANG*func(BR_ANGLE, t/LENBR))

I(t) --> #(0.01*(40+2*t))C(LEN1*S0*func(INT_GROWTH,t/LEN1)/15,15)
J(t) --> #(0.01*(40+1.2*t)) C(LEN2*S0*func(INT_GROWTH,t/LEN2)/15,15)

/*
C(l) : l>S0 --> F(S0) C(l-S0)
C(l) : l<=S0 --> F(l)
*/

C(l,n) : n>0.5 --> F(l) C(l,n-1)
C(l,n) --> * 

L(t) : { leaf_length = LEAFSCALE*func(LEAF_GROWTH,t/LENLEAF);
	 leaf_width = leaf_length^2.0;
	 leaf_height = leaf_length^3.0; } 1 --> 
	 ;(64+floor(63*func(LEAF_COLOR,t/LENLEAF))) &(90*func(LEAF_ANGLE, t/LENLEAF)) 
	 ~l(leaf_width, leaf_length, leaf_height)

K(t) --> [R(t)/(72)R(t)/(72)R(t)/(72)R(t)/(72)R(t)]
	
R(t) : {calyx_length = 10*func(CALYX_GROWTH, t/(T1+T2+T3+T4)); } 1 -->
	[;(32+floor(31*func(CALYX_COLOR,(t-(T1+T2+T3))/T4)))
	@!(NR_VERT) @#(1) #(0.5*calyx_length*func(CALYX_PROFILE, 0)) 
	@Gs S(0,calyx_length) @Ge(1)] [f(calyx_length) E(t)]

S(x_rel,x_max) : {x_rel = x_rel + 1/NR_HORI;} x_rel<=1+0.001 --> 
	f(x_max/NR_HORI) #(0.25*x_max^1.5*func(CALYX_PROFILE, x_rel)) @Gc(1) S(x_rel, x_max)

S(x_rel,x_max) : x_rel > 1+0.001 --> *

E(t) : t>T1 && t < T1+T2+T3 {
	stage = (t-T1) / (T2+T3); 
	petal_angle = 90*func(PETAL_ANGLE, stage); 
	petal_size = PETALSCALE*func(PETAL_SIZE, stage); 
	} --> ;(3) ^(petal_angle) 
		~p(0.5*petal_size*1.7, petal_size, 0.5*petal_size^2.0)

E(t) : t >= T1+T2+T3 {
	stage = (t-(T1+T2+T3)) / T4; 
	petal_angle = 40*func(PETAL_ANGLE, stage); 
	petal_size = 2*func(PETAL_SIZE, stage); 
	} --> ;(63)^(petal_angle) 
		F(petal_size)

endlsystem
