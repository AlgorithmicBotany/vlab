/* Terminalia catappa */
#define R1 0.94   
#define R2 0.87
#define A1  24.4  
#define A2  36.9  
#define ALPHA 138.5
#define A3  90

Lsystem: 1
derivation length: 1
Consider: ?E
Axiom: B;(2)[C T(1)]-(90)f(2.2)+(90)[;(4)C T(0)]

/* generate the desired number of lateral branches */
T(d) --> [-(A3)/(180*d)F(1)?E(1)A(1)]
	/(ALPHA)[-(A3)/(180*(1-d))F(1)?E(1)A(1)]
	/(ALPHA)[-(A3)/(180*d)F(1)?E(1)A(1)]
	/(ALPHA)[-(A3)/(180*(1-d))F(1)?E(1)A(1)]
	/(ALPHA)[-(A3)/(180*d)F(1)?E(1)A(1)]

?E(r) < A(l) : r == 1 --> 
	[^(A2)F(l*R2)?E(0.8)A(l*R2)]
	&(A1)F(l*R1)?E(0.9)/(180)A(l*R1)

?E(n) --> ?E(1)

homomorphism: no warnings

B --> [@M(-15,0,0)f(-2)F(4)]

A(l) --> @O

F(l) --> @OF(l)[,@v&(90)f(ran(0.01))^(90)@c(0.5*2)]

C --> [@R(0,0,1),&(90)f(ran(0.0001))^(90)@c(0.5*2)]		

endlsystem
