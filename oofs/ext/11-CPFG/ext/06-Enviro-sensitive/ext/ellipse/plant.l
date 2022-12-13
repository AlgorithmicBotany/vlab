#define STEPS 400
#define WIDTH 4
#define R 10
#define RB 10.8
#define l 1.0 
#define in(x,y) (WIDTH*x*x+(y-R)*(y-R) <= R*R)


Lsystem: 1
Seed: 1
Start: {n=1;}
StartEach: {p=(2*n+1)/(n*n); q=1-p; ey = 10.4+10.4*cos(n); ex = RB/(WIDTH^0.5)*sin(n);}
EndEach: {n=n+1;}
derivation length: STEPS
Axiom: E

/* The clipping ellipse */

E : n <= 360 --> [f(ey)-(90)f(ex)@o(0.2)]E
E : n > 360 -->, A

/* Main apex */

A --> [+B][-B]F?P(0,0)A
B --> F?P(0,0),@O(0.4);B
?P(x,y) : !in(x,y) --> !(2)[+(2*y)F(l)][-(2*y)F(l)]%

endlsystem
