#define STEPS 8

#define LEN  10		// stem length
#define WID  1.5	// stem diameter
#define NR 4		// number of mesh strips per internode
#define LR 0.95		// internode length reduction SENS == 0
#define WR 0.85		// internode with reduction
#define SL 3 // sporangium length

#define SENS 1 // sensitivitiy to the orientation

Lsystem: 1
derivation length: STEPS
Start: {n=0;}		// n is branch order counter 
StartEach: {n = n+1;}

Axiom: /(30) #(WID) +(20) @Gs -(20) A(LEN,WID)?H(0,0,0)

/* Branching vegetative structure - the length of internodes 
   is controlled by orientation of the turtle. */

A(l,w) > ?H(x,y,z) : n < STEPS --> f(l)#(w*WR)@Gc(NR) 
	[-(20)/(90)A(l*LR*y^SENS,w*WR)?H(0,0,0)]
	[+(20)/(90)A(l*LR*y^SENS,w*WR)?H(0,0,0)]

/* Sporangium */
A(l,w) --> [;(3)#(w)f(SL)@Ge(NR)@O]

homomorphism

A(l,w) --> [@Ge(NR)@O]	// apex is represented by a spehere

endlsystem


