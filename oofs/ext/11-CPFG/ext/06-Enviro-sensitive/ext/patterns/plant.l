#define STEPS 700
#define WIDTH 4
#define R 10
#define RB 10.8
#define l 1.0 
#define in(x,y) (WIDTH*x*x+(y-R)*(y-R) <= R*R)
/*
#define in(x,y) (y<0 || y>R+R || x<-R || x > R)
*/


Lsystem: 1
Seed: 1
Start: {n=1;}
StartEach: {p=(2*n+1)/(n*n); q=1-p; ey = 10.4+10.4*cos(n); ex = RB/(WIDTH^0.5)*sin(n);}
EndEach: {n=n+1;}
derivation length: STEPS
Axiom: E

/* The clipping ellipse */

E : n <= 360 --> [f(ey)-(90)f(ex)@o(0.2)]E
E : n > 360 --> +(45),FA?P(0,0)

/* Main apex */

FA > ?P(x,y) : in(x,y) --> FFA
FA > ?P(x,y) : !in(x,y) --> -(90)Fb

Fb > ?P(x,y) : in(x,y) --> FFB
Fb > ?P(x,y) : !in(x,y) --> -(180)Fc

FB > ?P(x,y) : in(x,y) --> FFB
FB > ?P(x,y) : !in(x,y) --> +(90)Fa

Fa > ?P(x,y) : in(x,y) --> FFA
Fa > ?P(x,y) : !in(x,y) --> Fd

Fc > ?P(x,y) : in(x,y) --> FFA
Fc > ?P(x,y) : !in(x,y) --> @o(0.5)

Fd > ?P(x,y) : in(x,y) --> FFB
Fd > ?P(x,y) : !in(x,y) --> @o(0.5)

endlsystem
