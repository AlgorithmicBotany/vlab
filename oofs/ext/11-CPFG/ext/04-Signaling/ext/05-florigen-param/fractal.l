#define PL 1 /* plastochron */
#define DEL 15 /* delay for introducing florigen at the base */

Lsystem: 0
derivation length: 40
ignore: +-
Axiom: D(0)I(0)A(0)

/* apex not reached by florigen produces internodes with
   plastochron PL */ 

I(s) < A(t) : s==0 && t<PL  --> A(t+1)
I(s) < A(t) : s==0 && t==PL --> I(0)A(0)

/* apex reached by florigen is transformed into a 
   flowering bud */

I(s) < A(t) : s==1 --> K

/* introduction of florigen at the base after a delay */

D(t) --> D(t+1)
D(t) < I(s) : t==DEL --> I(1)

/* propagation of florigen */

I(r) < I(s) --> I(r)

homomorphism

I(s) --> ;(1+s)F

K --> ;(3)@O(1.7)

endlsystem
