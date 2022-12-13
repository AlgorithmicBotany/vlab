#define LEFT 1
#define STEPS 3
#define c 0.5
#define GRID 1
#define TILE 1
#define WW 3
#define YW 3
#define GW 1

Lsystem: 0
Start: {b = 1-c; a = b/2;}
derivation length: STEPS+1
Axiom: A

A : LEFT --> F(a)LF(a) 
A : !LEFT --> F(a)RF(a) 

L --> +RF(b)-LF(b)L-F(b)R+
R --> -LF(b)+RF(b)R+F(b)L-

homomorphism
maximum depth: 2

L --> Sl Tl ;!(YW)F(c),!(WW)
R --> Sr Tr ;!(YW)F(c),!(WW)

Sl : GRID --> [,,,!(GW)|f(a)|-f(a)+F(1)+F(1)+F(1)+F(1)]
Sr : GRID --> [,,,!(GW)|f(a)|+f(a)-F(1)-F(1)-F(1)-F(1)]

Tl : TILE --> [,{f(c)+f(c)+f(c)+f(c)}] 
Tr : TILE --> [,,{f(c)-f(c)-f(c)-f(c)}]

endlsystem
