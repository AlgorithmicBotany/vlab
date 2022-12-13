#define dt 0.011

#define angle1 100*func(Ang_1,t)
#define angle2 100*func(Ang_2,t)
#define angle3 100*func(Ang_3,t)
#define angle4 100*func(Ang_4,t)
#define angle5 100*func(Ang_5,t)
#define y_displacement 1.0*func(y,t)
#define x_displacement 1.2*func(x_motion,t)


Lsystem: 0
Start: {t=0; }
EndEach: {t = t+dt;}

derivation length: 1
Axiom: A

homomorphism: 
maximum depth: 10

A --> /(90) &(90) P	// orient scene, create base plane
	f(x_displacement) ^(90) f(y_displacement) 	// motion of lamp base
	;(1) #(1.0) F(0.05) #(1.0) F(0.1) #(0.06) 	// draw lamp base
	/(angle1) L(0.1) 			// rotation at the base
	&(angle2) L(1) 				// joint angles
	&(angle3-angle2) L(1)		// angle differences: easier to manipulate...
	&(angle4-angle3) F(0.15)	// ...then individual angles
	/(angle5)				// rotation of the shade with bulb 
	#(0.06) f(0.1) &(90) 	// geometry of shade with bulb...
	!(0.2) |f(0.1)| #(0.15) [ ^(90) @c ] F(0.3) #(0.2) 
	[ ;(7) f(0.09) @O(0.22) ] F(0.2) #(0.4) 

// draw links as cylinders terminated by a sphere
L(x) --> F(x) @O

// base plane geometry
P --> [ |f(1) +(90) f(1) {;(2) +(90) f(3) +(90) f(2) +(90) f(3) +(90) f(2) } ]

endlsystem
