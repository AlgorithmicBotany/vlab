#define PL 1 /* plastochron */
#define DEL 15 /* delay for introducing florigen at the base */

Lsystem: 0
derivation length: 50
ignore: +-
Axiom: D(0)I(0,0)A(0)

/* apex not reached by florigen produces internodes with
   plastochron PL */ 

I(f,a) < A(t) : f==0 && t<PL  --> A(t+1)
I(f,a) < A(t) : f==0 && t==PL --> I(0,0)A(0)

/* apex reached by florigen is transformed into a 
   flowering bud */

I(f,a) < A(t) : f==PL --> K

/* introduction of florigen at the base after a delay */

D(t) --> D(t+1)
D(t) < I(f,a) : t==DEL --> I(1,a)

/* introduction of an auxin-related signal at the top */

I(f,a) > K --> I(f,1)

/* propagation of florigen (up) and/or auxin (down) */

I(fl,al) < I(f,a) > I(fr,ar) --> I(fl,ar)

/* special cases - first and last internode */

I(fl,al) < I(f,a) > A(t) --> I(fl,a)
D(t) < I(f,a) > I(fr,ar) --> I(f,ar)

homomorphism

I(f,a) --> ;(1+f+a)F
K --> ;(4)@O(1.7)

endlsystem
