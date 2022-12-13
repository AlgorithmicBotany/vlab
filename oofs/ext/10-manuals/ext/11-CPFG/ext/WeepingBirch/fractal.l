#define STEPS 16
#define TOTAL  22500	// total number of internodes
#define LEN 2		// petiole length
#define LLEN 1.0	// controls leaf size
#define SEED 21 // random number seed

Lsystem: 0
Start: {srand(SEED);}
derivation length: STEPS+1
Axiom: ;(2)A(TOTAL) 

A(n) : n>1 {nl=floor(n*bran(5,16)+0.0); nr = n-nl-1;} --> 
	_!(0.40*n^0.3)F(n^0.4)/(180.0);[+(nr/n*30)A(nl)][-(nl/n*30)A(nr)]

A(n) : n==1 --> !(0.2)F(LEN);(250+ran(5))
	[_(0){+F(LLEN)-F(LLEN)-F(LLEN)+|+F(LLEN)-F(LLEN)-F(LLEN)}]

A(n) : n<=0 --> *

endlsystem
