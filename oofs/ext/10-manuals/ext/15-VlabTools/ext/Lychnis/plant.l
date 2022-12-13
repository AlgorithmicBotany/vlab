#include <lpfgall.h>

/*With a few simple timeline functions */

#define LEAF_SURFACE 1
#define LEAF_TEXTURE 0

/* parameters accessible via panels */
#define VANG -45 // viewing angle - rotates plant on its base
#define PHI 90 //phylllotaxis
#define BANGSCALE 40 // target branching angle
#define TROPISM 0.05

#define INTLENSCALE 0.58 // controls the overall length of branches
#define LEN1 30 // target internode length
#define LEN2 14 // target pedicel length
#define LEAFSCALE 4 // controls leaf size
#define PETALSCALE 2 // controls petal size
#define eta 5.0 // the exponent in the pipe model

/* timings delays */
#define D1 -4 // delay in the development of the first branch
#define D2 -7 // delay in the development of the second branch
#define Pa 1.8 // petals appear
#define Pd 10.8118 // petals dropped

/* control of discretization (non-exposed parameters) */
#define N_SEG 15	// internode consists of N_SEG segments
#define NR_HORI 10	// calyx consists of NR_HORI segments

#define dt 0.1 // simulation time step

/* Preset parameters */
#define int_width  0.25
#define br_angle	 40
#define leaf_angle  45
#define leaf_color  2
#define pod_color 63
#define petal_length PETALSCALE
#define petal_angle  90
#define br_color  63

#define calyx_section(x_rel) func(CALYX_PROFILE, x_rel)


/* graphically-defined functions of time */
#define int_length(t)		INTLENSCALE*tfunc(INTERNODE_LENGTH,t)
#define leaf_length(t)		LEAFSCALE*tfunc(LEAF_LENGTH,t)
#define calyx_length(t) 		10*tfunc(CALYX_LENGTH, t)

/* structural module declarations */
module A(float); 					// apex (age)
module I(float,float,float);	// internode (target length, width, age)
module L(float);					// leaf (age)
module K(float);					// bud/flower/fruit (age)
module B(float);					// branching point (age)

module S(float);					// sepal (time)
module P(float);					// petal (time)

float T;   //global time
float branching_ends, simulation_ends;  //Aging parameters

Start: { 
	T = 0; 
	branching_ends = val(BR_END); 
	simulation_ends = val(SIM_END); 
}

EndEach: { T +=dt; Backward(); }
 
// derivation length: STEPS;
derivation length: (int) (simulation_ends/dt);

Axiom: RollL(VANG) SetElasticity(0,TROPISM) Down(5) 
	TextureVCoeff(0.01) A(0);

production:
/* all plant components age */
A(t) : produce A(t+dt);
L(t) : produce L(t+dt);
K(t) : produce K(t+dt);
B(t) : produce B(t+dt);

/* internode width propagates according to the pipe model */
consider: I K; 
I(l,w,t) >> SB I(l1,w1,t1) EB SB I(l2,w2,t2) EB I(l3,w3,t3) : 
		 produce I(l, w1+w2+w3, t+dt);
I(l,w,t) >> SB I(l1,w1,t1) EB I(l2,w2,t2) : produce I(l, w1+w2, t+dt);
I(l,w,t) >> I(l1,w1,t1) : produce I(l, w1, t+dt);
I(l,w,t) >> K(t1) : produce I(l, int_width, t+dt);

decomposition:
A(t):
{
	if (t > 0 && T < branching_ends) produce
		I(LEN1,0,t) 
		SB RollR(PHI) B(t) SB L(t) EB A(D1+t) EB
		SB RollL(PHI) B(t) SB L(t) EB A(D2+t) EB
		I(LEN2,0,t) K(t); 
}

interpretation:
B(t) : produce Down(br_angle);	// branching angle changes over time

maximum depth: 30;

/* internode specification */
I(l,w,t) : 
{
	nproduce SetColor(br_color) SetWidth(pow(w, 1/eta));
	for (int i=0; i < N_SEG; i++)
		nproduce F(l*int_length(t) / N_SEG);
	produce;
}

/* a leaf is an interactively modeled Bezier surface */
L(t) : 
{
	produce Down(leaf_angle) RollToVert() CurrentTexture(LEAF_TEXTURE)
			  SetColor(leaf_color) Surface(LEAF_SURFACE, leaf_length(t)); 
}

/* a flower is modeled as five sepals and petals */
K(t) : 
{
	nproduce CurrentTexture(-1);
	for (int i=0; i < 5; i++) {
		nproduce SB S(t) P(t) EB; 
		if (i<4) nproduce RollR(72);
	}
	produce;
}

/* sepals are modeled as generalized cylinders */ 
S(t) : 
{
	float calyx_segment_length = calyx_length(t) / NR_HORI; 
	float calyx_width = 0.25*pow(calyx_length(t),1.5);
	nproduce SetColor(pod_color) CurrentContour(1) SetWidth(0.1);
	nproduce StartGC;
	for (float x=1.0/NR_HORI; x<1.0 + 1.0/NR_HORI; x+=1.0/NR_HORI) {
		nproduce SetWidth (calyx_width * calyx_section(x));
		nproduce F(calyx_segment_length);
	}
	produce EndGC;
}

/* a petal is an allometrically scaled Bezier surface */
P(t) : 
{
	if (t>Pa && t < Pd) {
		float petal_len = petal_length;
		float petal_width = 0.5*pow(petal_len,2.0);
		float petal_height =  0.5*pow(petal_len,2.0);
		nproduce SetColor(3) CurrentTexture(1) Down(petal_angle);
		produce Surface3(0, 2*petal_width, 2*petal_len, 2*petal_height);
	}
}

