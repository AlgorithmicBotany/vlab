#define STEPS 400
#define IL 3	/* number of internodes in a mature left branch */
#define IR 5	/* number of internodes in a mature right branch */
#define P 5	/* plastochron (in simulation steps) */
#define DEL 35	/* delay before insect appears at the base */

Lsystem: 0
derivation length: STEPS

Axiom:  W(DEL)FA(0,IL)

/* Create a branching structure */

F < A(t,n) : t>0 --> A(t-1,n)
F < A(t,n) : t==0 && n>0 --> FA(P,n-1)
F < A(t,n) : t==0 && n==0 --> [+FA(P,IL)][-FA(P,IR)]

/* Insert an insect at the base after a wait period */

W(t) : t>0 --> W(t-1)
W(t) : t==0 --> U

/* Insect U moves upwards */

U < F  --> FU

/* In a branching point, insect U goes into left branch */

U < + --> +U

/* Having reached an apex, insect U removes it and 
   becomes a D */

U < A(t,n) --> D

/* Insect D moves downwards */

F > D  --> DF

/* D insect returning from the left branch becomes 
   temporarily V and goes into the right branch */

F > [+D=] --> FV
V < - --> -U

/* D insect returning from the right branch keeps 
   going down */

F > [=][-D=] --> FD

/* houskeeping - removing the insect from the previous step from the string */

U --> *
D --> *
V --> *

homomorphism

A(t,n) --> [;(2)@O(0.5)]

U --> [;(3)@O(0.7)]
V --> [;(4)@O(0.7)]
D --> [;(5)@O(0.7)]

endlsystem


