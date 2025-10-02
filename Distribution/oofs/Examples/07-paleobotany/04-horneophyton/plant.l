#define STEPS 7
#define NR 4
#define LR 0.95
#define WR 0.85
#define SL 3
#define SENS 1.0 /* sensitivitiy to the orientation */

#define LEN  10
#define WID  1.5

Lsystem: 1
derivation length: STEPS
Axiom: /(30)#(WID)+(20)@Gt(1.2)@Gs-(20)A(LEN,WID)?H(0,0,0)

/* The length of internodes is controlled by orientation 
   of the turtle. */

A(l,w)> ?H(x,y,z) --> f(l)#(w*WR)@Gc(NR)T(w)
	[-(20)/(90)A(l*LR*y^SENS,w*WR)?H(0,0,0)]
	[+(20)/(90)A(l*LR*y,w*WR)?H(0,0,0)]

T(w) --> *

homomorphism
maximum depth: 2

T(w) --> ;(3)f(0)#(w*WR)@Gc(NR)
	[-(20)/(90)f(SL)@Ge(NR)@O]
	[+(20)/(90)f(SL)@Ge(NR)@O]

endlsystem


