#define ANGLE 137.51
#define ds 0.001

Lsystem: 1
Start: {L=curveGAL(1);}
Derivation length: 1
Axiom: A(0,0)

decomposition
maximum depth: 3000

A(s,a) : s<L {
	while (a<1 && s<L) {
		rho = 0.2*func(1,s/L); rho_square = rho^2;
		x = curveX(1,s/L);
 		a = a + (2*x/rho_square)*ds;
		s = s+ds; 
		}
		a = a-1; 
		y = curveY(1,s/L);
		no_poly = 6 + 300*rho;	
	}
	--> [f(y)-(90)f(x);(floor(1+s/L*32))@!(no_poly)
		@O(2.1*rho)]/(ANGLE)A(s,a)

endlsystem
