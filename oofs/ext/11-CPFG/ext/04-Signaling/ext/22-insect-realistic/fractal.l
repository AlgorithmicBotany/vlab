#define STEPS 8
#define PL 6 /* plastochrone */
#define NR1 4  /* number of segements per internode */
#define NR2 12  /* number of segments per internode */
#define INR 4 /* Number of segments created after branch */
#define DL 79 /* delay before insect attack */
#define E0 0.1
#define EI 0.01
#define EM -0.5
#define SI 1.5
#define SINC 0.25

#define Wscale 0.25

#define GL 90  /* lenght of the ground on each side */

Lsystem: 0
Start: {c=SI+2*SINC;n=1;}
StartEach: {s=0;printf("Step %3f\n",n);n=n+1;}
EndEach: {s=1;}

derivation length: STEPS

ignore: [+-_WI/!;

Axiom: [#(1),(48)+(90)f(GL)[-(180)F(2*GL)]
	[-(90)f(1)-(90)f(0.5,0)/(70)h]]SM

f(l,t)/(n)h : l >=GL-0.5 --> *
f(l,t) --> f(l+(GL-1)/(DL+0.5),t)

S --> E/(30)+(10);(1,0)!(1*Wscale,0.015*Wscale)_(E0,0)d(DL)A(0,NR1)B

E > g --> [f(2)-(90)f(0.5,0)/(70)h]

/* delay for releasing the bug */
d(n) : n>0 --> d(n-1) 
d(n) --> f

f < F > * --> f
f < 0 > * --> 1
1 < aF > * --> f
f < A(n,m) > * : 1 {c=c+SINC;} --> Yf%
* < F > Y --> g
* < F > g --> g
* < 2 > g --> 3
3 < bF > * --> f
* < 4 > g --> 5
* < 4 > F=]g --> 5
* < F > 5 --> g 
* < f > * --> F
* < g > * --> F
* < 1 > * --> 2
* < 3 > * --> 4
* < 5 > * --> 6
* < Y > * --> *

* < _(e,i) > g --> _(e,EI)
* < _(e,i) > * : e>EM --> _(e-i,i)

+(i,j) > g --> +(i+1,1)
+(i,j) : i < 30 --> +(i+j,j)

!(d,i) > g --> !(d,0)
!(d,i) --> !(d+i,i)

;(d,i) > g --> ;(d,1)
;(d,i) : d < 32 --> ;(d+i,i)

/* in the absence of the bug, a branch  is created */
A(n,m) : n > 0 --> A(n-1,m)
A(n,m) : n == 0 && m > 0 --> FA(PL,m-1)
A(n,m)B : n == 0 && m == 0 
	--> I(PL,INR)/(90)0+(0,0)
	[;(1,0)!(1*Wscale,0.020*Wscale)_(E0,0)+aFA(PL,NR1)B]
	[;(1,0)!(1*Wscale,0.010*Wscale)_(E0,0)-bFA(PL,NR2)B]
	[--L][++L]
L --> [{.g(0)[&g(0).][g(0)g(0)g(0).]^g(0).}] 

I(n,m) : n>0 && m>0 --> I(n-1,m)
I(n,m) : n==0 && m>0 --> FI(PL,m-1)

g(i) : i<1.5 --> g(i+0.1)

homomorphism: no warnings
/* bud */
#define BSC 0.007

B --> ;(129)~b(BSC)
/*B --> ;(128)@O(1)*/

#define Iscale 0.25

1 --> [;(128)!(2*Iscale)@O(c)[-F(c)++F(c/2)]+F(c)--F(c/2)]f
3 --> [;(128)!(2*Iscale)@O(c)[-F(c)++F(c/2)]+F(c)--F(c/2)]f
5 --> [;(128)!(2*Iscale)@O(c)[-F(c)++F(c/2)]+F(c)--F(c/2)]f
f --> [_(0);(128)!(2*Iscale)@O(c)[-F(c)++F(c/2)]+F(c)--F(c/2)]f
h --> [_(0);(128)!(2*Iscale)@O(c)[-F(c)++F(c/2)]+F(c)--F(c/2)]f
g --> g[;(128)|!(2*Iscale)@O(c)[-F(c)++F(c/2)]+F(c)--F(c/2)]

H(n,m) --> ;(129){-F(2)++F(2)-|-F(2)++F(2)-}

endlsystem




