#define PL 1 /* plastochron */
#define DEL 15 /* delay for introducing florigen at the base */

Lsystem: 0
derivation length: 50
ignore: +-/
Axiom: D(0)I(0,0)A(0)

/* apex not reached by florigen produces internodes 
   and the associated (dormant) buds with plastochron PL */ 

I(f,a) < A(t) : f==0 && t<1  --> A(t+1)
I(f,a) < A(t) : f==0 && t==1 --> [+B]I(0,0)/(180)A(0)

/* apex reached by florigen is transformed into a 
   flowering bud */

I(f,a) < A(t) : f==1 --> K

/* introduction of florigen at the base after a delay */

D(t) --> D(t+1)
D(t) < I(f,a) : t==15 --> I(1,a)

/* introduction of an auxin-related signal at the top */

I(f,a) > K --> I(f,1)

/* propagation of florigen (up) and/or auxin (down) */

I(fl,al) < I(f,a) > I(fr,ar) --> I(fl,ar)

/* special cases - first and last internode */

I(fl,al) < I(f,a) > A(t) --> I(fl,a)
D(t) < I(f,a) > I(fr,ar) --> I(f,ar)

/* dormant bud reached by the downward signal becomes 
   active */

I(f,a) < B : a==1 --> C(0)

/* active lateral bud produces internodes with the 
   plastochron PL */

C(t) : t<PL --> C(t+1)
C(t) : t >= PL --> FC(0)

homomorphism

I(f,a) --> ;(1+f+a)F
B --> ;(1)@O(0.3)
K --> ;(4)@O(1.0)

endlsystem
