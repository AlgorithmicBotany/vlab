#define PL 1 /* plastochron */
#define DEL 15 /* delay for introducing florigen at the base */

Lsystem: 0
derivation length: 40
ignore: +-
Axiom: D(0)IA(0)

/* A - apex
   I - internode not reached by signal
   J - internode reached by signal
   D - plant base (could be basal rosette) 
   K - flowering bud
*/

/* apex not reached by florigen produces internodes with
   plastochron PL */ 

I < A(t) : t<PL  --> A(t+1)
I < A(t) : t==PL --> IA(0)

/* apex reached by florigen is transformed into a 
   flowering bud */

J < A(t)  --> K

/* introduction of florigen at the base after a delay */

D(t) --> D(t+1)
D(t) < I : t==DEL --> J

/* propagation of florigen */ 

J < I --> J

homomorphism

I --> ;(1)F
J --> ;(2)F

K --> ;(3)@O(1.7)

endlsystem
