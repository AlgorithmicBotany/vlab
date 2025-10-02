#define LEAVES 1
#define L0 2.25 /* length of the main axis */
#define RELATIVE 1 /* curvature wrt the length */
#define L0_MAX 3.1 /* length of the main axis */
#define AS 5.0 /* angular scale */
#define STEM_WID_SCALE 0.05 /* angular scale */
#define ANG 180
#define LEN_SCALE 1.0
#define ASYM_SCALE 0.1
#define CURV_SCALE 100
#define L_CURV_SCALE 100
#define COLOR_SCALE 16
#define RAISE_SCALE 0.1

Lsystem: 1
Start: {xl0 =
	func(14,L0/L0_MAX) * LEN_SCALE * func(2,0) * (1-ASYM_SCALE*func(13,0));
	xr0 =
	func(14,L0/L0_MAX) * LEN_SCALE * func(2,0) * (1+ASYM_SCALE*func(13,0));
}

derivation length: 1
Axiom: E &(2)+(7)A(0,L0,xl0,xr0)
/* Axiom: ;(16)L(0.1,0.3,1) */

/* Arguments to A: 
   x - cumulative length of the axis
   L - target length of the axis
*/

homomorphism
maximum depth: 300

E : 1 {srand(1);} --> *

A(x,x_max,xl,xr) : xl<xr && x+xl<x_max 
		{new_x = x+xl; 
		x_rel = new_x/x_max;
		new_xr = xr-xl;
		new_xl = func(14,L0/L0_MAX) * LEN_SCALE * func(2,x_rel)
			* (1 - ASYM_SCALE*func(13,x_rel));
		turn = CURV_SCALE * func(3,x_rel) * xl;
                if (RELATIVE) {turn = turn/L0;}
		bang = ANG*func(6,x_rel);
		} --> 
	!(STEM_WID_SCALE*func(11,x_rel))F(xl)
	[^(90)f(RAISE_SCALE*xr)&(90)
	\(10+nran(0,4))+(bang+nran(0,4))B(new_x,x_max,1)] 
	+(turn) ^(AS*func(4,x_rel)) /(AS*func(5,x_rel))
	A(new_x,x_max,new_xl,new_xr)

A(x,x_max,xl,xr) : xr<=xl && x+xr<x_max 
		{new_x = x+xr; 
		x_rel = new_x/x_max;
		new_xl = xl-xr;
		new_xr = func(14,L0/L0_MAX) * LEN_SCALE * func(2,x_rel)
			* (1 + ASYM_SCALE*func(13,x_rel));
		turn = CURV_SCALE * func(3,x_rel) * xr;
                if (RELATIVE) {turn = turn/L0;}
		bang = ANG*func(6,x_rel);
		} --> 
	!(STEM_WID_SCALE*func(11,x_rel))F(xr)
	[^(90)f(RAISE_SCALE*xr)&(90)
	/(10+nran(0,4))-(bang+nran(0,4))B(new_x,x_max,-1)] 
	+(turn) ^(AS*func(4,x_rel)) /(AS*func(5,x_rel))
	A(new_x,x_max,new_xl,new_xr)

B(x,x_max,side) : * {x_rel = x/x_max;
		wid = func(1,x_rel)*(1+nran(0,0.05));
		leaflet_width = func(10,x_rel);
		}
		--> ;(16+COLOR_SCALE*func(15,x_rel))L(wid,leaflet_width,side)

L(wid,leaflet_width,side) : !LEAVES --> F(wid)
L(len,wid,side) : LEAVES --> 
	[@#(1) @Gt(1.3,0.5) @Gr(0) !(0) @Gr(0,0,-0,0.012) 
	/(90)@Gs \(90) C(0,len,wid,side)/(90)@Ge\(90)]

C(x,x_max,wid,side) : { x_rel = x/x_max;
		dx = 0.1*func(8,x_rel);
		new_x = x + dx;
		} new_x < x_max -->
	f(dx)!(0.05*func(7,x_rel)*wid)/(90)@Gc(1)\(90)
	+(L_CURV_SCALE*func(9,x_rel)*side*dx/x_max)
	^(L_CURV_SCALE*func(12,x_rel)*dx/x_max)
	C(new_x,x_max,wid,side)

endlsystem
