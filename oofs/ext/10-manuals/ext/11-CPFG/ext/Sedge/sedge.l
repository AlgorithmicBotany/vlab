#define RATE 1.02   /* internode growth rate */
/*  For a doubling in branch length we want 1.26 times the width */
/*  The exponent is equivalent to log(1.26)/log(2) approximately */
/*  for 1.1 we use an exponent of .1375 */
/*  for 1.2 we use an exponent of .2630 */
/*  for 1.26 we use an exponent of .3334 */
/*  for 1.3 we use an exponent of .3785 */
#define STEMWIDTH .0075    /* width of stem at start of internode */
#define STEMRATE 1.06       

/* Sub L-systems for Female spike, Male spike and leaf */
#define F_SPIKE ?(2,1.25)spikef$
#define M_SPIKE ?(3,1.25)spikem$
#define LEAF ?(4,1)leaf((a-10),a,(a-13)/100)$
              /* Three parameters: starting delay, time to turn, and new elasticity */

lsystem: 1
/* nice derivation length 90+ (95?) */
derivation length: 95
axiom: /(30)+(10)#(STEMWIDTH)A(4,4)
A(a,t) : a==30 --> F(1)/(137.5)M_SPIKE
A(a,t) : t<10 --> F(1)A(a+1,t+1)
A(a,t) : t==10 --> F(1)/(137.5)[L(a)][S(a)]#(STEMWIDTH)A(a+1,0)
* < #(d) : d<200 --> #(d*STEMRATE)
                    /* left context needs so that # is not at beginning of line */
S(a) --> [^(25)_(0-.1)!(.3)F((30-a)/5)F((30-a)/5)_(0)F_SPIKE]
                    /* ! is used here so that width won't be increased */
L(a) --> [^(60)!(.1)LEAF]
F(t) : t<2 --> F(t*RATE)
F(t) : !(t<2) --> F(t*RATE/2)F(t*RATE/2)
endlsystem

#include "female.l"
#include "male.l"
#include "leaf.l"
