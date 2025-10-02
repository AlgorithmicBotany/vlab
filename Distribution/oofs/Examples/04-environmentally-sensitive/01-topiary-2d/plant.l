#define ALPHA   32	/* branching angle */
#define PHI     180	/* Phyllotactic angle */
#define SIZE    2.6  	/* Size of the pruning box */
#define SM	0.05	/* Filling corners of the pruning box */

/* definition of the pruning box */

#define BOUNDARY [;(5)^(90)f(1)&(90)+(90)F(SIZE)[F(SM)] \
	-(90)F(2*SIZE)[F(SM)]-(90)F(2*SIZE)[F(SM)] \
	-F(2*SIZE)[F(SM)]-F(SIZE)]

/* definition of the pruning function */

#define whenprune(x,y,z) \
	((y<0)||(y>SIZE*2)||(x<-SIZE)||(x>SIZE)||(z<-SIZE)||(z>SIZE))

/* module definitions - this use of definitions works, but may 
easily introduce arrors that are difficult to debug */

#define Apex        A
#define Active_Bud  B
#define Dormant_Bud D
#define Signal      J

Lsystem: 1
derivation length: 1

Consider: JFD?P[]
Axiom: BOUNDARY X 

X --> ;(1)!(0.08)F(1) Apex ?P(0,0,0) 

/* branching */
Apex --> /(PHI) Dormant_Bud F(1.0) Apex

/* pruning */
F(n)Apex > ?P(x,y,z) : whenprune(x,y,z) 
	--> Signal F(0.02)%

/* bud initiation */
Dormant_Bud > Signal --> 
	[+(ALPHA)/(PHI)F(1.0)Apex?P(0,0,0)]

/* signal propagation */
F(n) > Signal --> Signal F(n)
F(n) > [Signal=] --> Signal F(n)
Signal --> * 

homomorphism
/* @R module are used to place Apices and Signals in front
   of the branching structure for visibility calculations */

Apex --> [@R(0,0,1)f(0.1)@R(1,0,0);(4)@c(0.3)]
Signal --> [@R(0,0,1)f(0.1)@R(1,0,0);@c(0.25)]
Dormant_Bud --> [+(ALPHA)!(0.05);(3)F(0.25)]

endlsystem
