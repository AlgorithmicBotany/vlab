#define LeafLife 3
#define LeafProd 3
Lsystem: 1
derivation length: 256
Consider: SL[]
Axiom: A(16)

/* S(alive,dead,flux) */

/* Grow a simple monopodial structure */

A(i) : i > 0 --> A(i-1)

A(i) : i == 0 --> 
	[++S(1,0,LeafProd)L(LeafLife*16+5)]
	S(1,0,0)F/(180)A(16) 

/* leaf ageing */
L(age) : age > 0 --> L(age-1)

/* Dead leaf contributes a dead pipe */
S(a,d,f) > L(age) : age == 0 --> S(0,1,0)

/* Determine branch attributes */

S(a1,d1,f1) > [S(a2,d2,f2)=]S(a3,d3,f3) --> 
	S(a2+a3,d2+d3,f2+f3-1)
S(a1,d1,f1) > [S(a2,d2,f2)=] -->
	S(a2,d2,f2-1)
S(a1,d1,f1) > S(a3,d3,f3) -->
	S(a3,d3,f3-1)

homomorphism

/* Stem width depends on the total number of pipes (dead + alive),
   and color depends on the flux */

S(a,d,f) --> !(2*(a+d));(127+f)

L(age) : * {col = (age>0)?3:4;} --> 
	;(col)F{+f--f+|+f--f+}

endlsystem




