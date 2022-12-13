#define LEAVES 1
#define L0 3.0 /* length of the main axis */
#define RELATIVE 1 /* curvature wrt the length */
#define L0_MAX 3.1 /* length of the main axis */
#define AS 5.0 /* angular scale */
#define STEM_WID_SCALE 0.05 /* angular scale */
#define ANG 180
#define LEN_SCALE 0.5
#define CURV_SCALE 100
#define L_CURV_SCALE 100
#define COLOR_SCALE 16
#define RAISE_SCALE 0.1
#define STEM_LEN_SCALE 0.3

Lsystem: 1
Start: {srand(1);
	dx0 =
	func(13,L0/L0_MAX) * LEN_SCALE * func(2,0);
}

derivation length: 1
Axiom: E S(10)


homomorphism
maximum depth: 300

E : 1 {srand(1);} --> *

S(n) : n>0 --> [&(90)f(0.2*n^0.5)^(90)A(0,L0,dx0,0)]/(137.5)S(n-1)

A(x,x_max,dx,ang) : x+dx<x_max 
		{new_x = x+dx; 
		x_rel = new_x/x_max;
		new_dx = func(13,L0/L0_MAX) * LEN_SCALE * func(2,x_rel);
		turn = CURV_SCALE * func(3,x_rel) * dx;
                if (RELATIVE) {turn = turn/L0;}
		bang = ANG*func(6,x_rel);
		} --> 
	!(STEM_WID_SCALE*func(11,x_rel))F(dx*STEM_LEN_SCALE)
	[/(ang)
	\(10+nran(0,4))+(bang+nran(0,4))B(new_x,x_max,1)] 
	+(turn) ^(AS*func(4,x_rel)) /(AS*func(5,x_rel))
	A(new_x,x_max,new_dx,ang+137.5)

B(x,x_max,side) : * {x_rel = x/x_max;
		wid = func(1,x_rel)*(1+nran(0,0.05));
		leaflet_width = func(10,x_rel);
		}
		--> ;(16+COLOR_SCALE*func(14,x_rel))L(wid,leaflet_width,side)

L(wid,leaflet_width,side) : !LEAVES --> F(wid)
L(len,wid,side) : LEAVES --> 
	[@#(1) @Gt(1.3,0.5) @Gr(0) !(0) @Gr(0,0,-0,0.012) 
	/(0)@Gs \(0) /(90) C(0,len,wid,side)/(0)@Ge\(0)]

C(x,x_max,wid,side) : { x_rel = x/x_max;
		dx = 0.1*func(8,x_rel);
		new_x = x + dx;
		} new_x < x_max -->
	f(dx)!(0.05*func(7,x_rel)*wid)/(90)@Gc(1)\(90)
	+(L_CURV_SCALE*func(9,x_rel)*side*dx/x_max)
	^(L_CURV_SCALE*func(12,x_rel)*dx/x_max)
	C(new_x,x_max,wid,side)

endlsystem
