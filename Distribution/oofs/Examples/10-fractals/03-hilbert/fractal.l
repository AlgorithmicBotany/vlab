/* Visualization of the construction of the Hilbert curve */

#define LEFT 1	/* 0 or 1: left tile of right tile */
#define STEPS 3	/* derivation length */
#define c 0.5	/* 0.0 to 1.0 - the size of the miro-tile */
#define GRID 1	/* 0 - without grid (green), 1 - with the grid */
#define WW 3	/* width of white segments of the curve */
#define YW 3	/* width of colored segments of the curve */
#define GW 1	/* width of the grid lines */

Lsystem: 0
Start: {b = 1-c; a = b/2;}
derivation length: STEPS+1
Axiom: A

A : LEFT --> F(a)LF(a) 
A : !LEFT --> F(a)RF(a) 

L --> +RF(b)-LF(b)L-F(b)R+
R --> -LF(b)+RF(b)R+F(b)L-

homomorphism

L : GRID --> [,,,!(GW)|f(a)|-f(a)+F(1)+F(1)+F(1)+F(1)]
	[,{f(c)+f(c)+f(c)+f(c)}];!(YW)F(c),!(WW)
R : GRID --> [,,,!(GW)|f(a)|+f(a)-F(1)-F(1)-F(1)-F(1)]
	[,,{f(c)-f(c)-f(c)-f(c)}];!(YW)F(c),!(WW)

L : !GRID --> [,{f(c)+f(c)+f(c)+f(c)}];!(YW)F(c),!(WW)
R : !GRID --> [,,{f(c)-f(c)-f(c)-f(c)}];!(YW)F(c),!(WW)

endlsystem
