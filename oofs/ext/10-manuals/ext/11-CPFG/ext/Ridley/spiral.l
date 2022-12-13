#define ANGLE 137.51	// Divergence angle
#define ds 0.001		// Arc length increment for integrating band areas 

Lsystem: 1
Start: {L=curveGAL(1);}	// curveGAL returns the length of the contour curve 
Derivation length: 1	// Non-developmental model 
Axiom: A(0,0) 			// 1st parameter: arc length along the curve so far
			   			// 2nd parameter: accumulated ring area...
						// towards current primordium 

decomposition
maximum depth: 3000

A(s,a) : s<L {			// If the end of the curve has not been reached ... 
						// input current primordium radius and calculate
			   			// area (PI is neglected, as it cancels out)
	rho = 0.1*func(1,s/L); rho_square = rho^2;
						//  While the accumulated ring area is less than 
			   			// the primordium area, move along the curve by
			   			// increments ds to accumulate ring area
	while (a<rho_square && s<L) {
		x = curveX(1,s/L);
 		a = a + 2*x*ds;
		s = s+ds; 
		}
						// At this point the ring area exceeds that needed 
			   			// for the primordium. Remove primordium area from
			   			// that of the ring (as a preparation for creating
			   			// new primordium)
		a = a-rho_square; 
						// Find y cordinate of new current primordium
		y = curveY(1,s/L);
						// Estimate the needed accuracy of the polygon
			   			// mesh representing the primordium
		no_poly = 6 + 300*rho;	

						// Create the primordium in cylindricdal coordiates
			   			// x, y, and the azimuth given by divergence

	} --> [f(y)-(90)f(x);(floor(1+s/L*32))@!(no_poly)
		@O(2.1*rho)]/(ANGLE)A(s,a)

endlsystem
