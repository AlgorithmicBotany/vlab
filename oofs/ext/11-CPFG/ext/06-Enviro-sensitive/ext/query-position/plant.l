#define STEPS 40
#define a 5
#define b 10
#define ANG 60
#define l 1
#define TROPISM 0

#define in(x,y) ((x*x)  / (a*a) + (y-b)*(y-b) / (b*b) <= 1)

Lsystem: 1
derivation length: STEPS
consider: ?P
Axiom: [;(1) E(0)] @Ts(1,TROPISM) ;(2) A?P(1,1)

A > ?P(x,y) : in(x,y) --> I [+(ANG) B][-(ANG) B] A ?P(0,0)
B > ?P(x,y) : in(x,y) --> N I B ?P(0,0) 

decomposition
maximum depth: 500

E(alpha) : alpha < 360 -->
	 [@M(a*cos(alpha), b*(1+sin(alpha)), 0) @c(0.3)] E(alpha+1)

E(alpha) --> *

homomorphism
I --> F(l)
A --> [;(3) @O(0.8)]
B --> [;(4) @O(0.5)]
N --> [;(5) @O(0.3)]

endlsystem
