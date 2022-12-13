#define L0 3.0 		/* length of the main axis */
#define L0_MAX 3.1 	/* max length of the main axis */
#define RELATIVE 1 	/* curvature wrt the length */
#define BANG 180 	/* branching angle scale */
#define CURV_SCALE 100	/* scale for stem turning angles */
#define LEN_SCALE 0.5
#define STEM_LEN_SCALE 0.3
#define STEM_WID_SCALE 0.05
#define COLOR_SCALE 16

Lsystem: 1

derivation length: 1
Axiom: E A(0,L0,0)

/* Arguments to A: 
   x     - arc length along the axis, measured from the base,
   x_max - target length of the axis
   ang   - curent value of the rotation along the axis
*/

homomorphism
maximum depth: 300

E : 1 {srand(1);} --> *

A(x,x_max,ang) : 
	{x_rel = x/x_max;
	dx = func(13,L0/L0_MAX) * LEN_SCALE * func(2,x_rel);}
		x + dx < x_max
	{new_x = x+dx;
	turn = CURV_SCALE * func(3,x_rel) * dx;
	bend = CURV_SCALE * func(4,x_rel) * dx;
	roll = CURV_SCALE * func(5,x_rel) * dx;
        if (RELATIVE) {turn = turn/L0; bend = bend/L0; roll - roll/L0;}
	} --> 
	!(STEM_WID_SCALE*func(11,x_rel))F(dx*STEM_LEN_SCALE)
	[/(ang)
	\(10+nran(0,4)) +(BANG*func(6,x_rel)+nran(0,4)) L(x_rel)] 
	+(turn) ^(bend) /(roll)
	A(new_x,x_max,ang+137.5)

L(x_rel) : 1 
	{len = func(1,x_rel)*(1+nran(0,0.05));
	wid = func(10,x_rel);
	} --> 
	[;(16+COLOR_SCALE*func(14,x_rel))
	@#(1) @Gt(1.3,0.5) @Gr(0) !(0) @Gr(0,0,-0,0.012) 
	/(0)@Gs \(0) /(90) C(0,len,wid)/(0)@Ge\(0)]

C(x,x_max,wid) : { x_rel = x/x_max;
		dx = 0.1*func(8,x_rel);
		new_x = x + dx;
		} new_x < x_max -->
	f(dx)!(0.05*func(7,x_rel)*wid)/(90)@Gc(1)\(90)
	+(CURV_SCALE*func(9,x_rel)*dx/x_max)
	^(CURV_SCALE*func(12,x_rel)*dx/x_max)
	C(new_x,x_max,wid)

endlsystem
