#define ANG1 0
#define ANG2 -45
#define ANG3 100
#define ANG4 -15
#define ANG5 0
#define Y 0
#define X 0

Lsystem: 0

derivation length: 1
Axiom: A

homomorphism: 
maximum depth: 10

A --> /(90) &(90) P	// orient scene, create base plane
	f(X) ^(90) f(Y)	// position lamp base
	;(1) #(1.0) F(0.05) #(1.0) F(0.1) #(0.06) // draw lamp base
	/(ANG1) L(0.1)	// rotation at the base
	&(ANG2) L(1) 	// joint angles...
	&(ANG3) L(1) 	
	&(ANG4) F(0.15) 
	/(ANG5) 				// rotation of the shade with bulb
	#(0.06)	f(0.1) &(90) 	// create shade with bulb...
	!(0.2) |f(0.1)| #(0.15) [ ^(90) @c ] 
	F(0.3) #(0.2) 
	[ ;(7) f(0.09) @O(0.22) ] F(0.2) #(0.4) 

// draw links as cylinders terminated by a sphere
L(x) --> F(x) @O

// base plane geometry
P --> [ |f(1) +(90) f(1) {;(2) +(90) f(3) +(90) f(2) +(90) f(3) +(90) f(2) } ]

endlsystem
