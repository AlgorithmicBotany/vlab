#define LEAVES 1 /* 0: lines; 1 - surfaces */
#define L0 1.0   /* length of the main axis */

/* Function id numbers */

#define INT_LENGTH 1
#define INT_WIDTH  2
#define CURVATURE  3
#define LEAF_LENGTH 4
#define LEAF_WIDTH 5
#define BRANGLE 6

/* Scaling factors (multipliers for the function ranges) */

#define INT_LENGTH_SCALE 0.33
#define INT_WIDTH_SCALE 0.01 
#define CURVATURE_SCALE 100
#define LEAF_LENGTH_SCALE 0.33
#define LEAF_WIDTH_SCALE 0.01
#define BRANGLE_SCALE 100

/* Allometric relation exsponents */

#define INT_LENGTH_GAMMA 0.5
#define INT_WIDTH_GAMMA 1.0
#define CURVATURE_GAMMA 1.5
#define LEAF_LENGTH_GAMMA 0.5
#define LEAF_WIDTH_GAMMA 0.5

/* Function definitions */

#undef int_length
#define int_length(x) \\
	(INT_LENGTH_SCALE * func(INT_LENGTH,x) * L0^INT_LENGTH_GAMMA)
#undef int_width
#define int_width(x) \\
	(INT_WIDTH_SCALE * func(INT_WIDTH,x) * L0^INT_WIDTH_GAMMA)
#undef curvature
#define curvature(x) \\
	(CURVATURE_SCALE * func(CURVATURE,x) / L0^CURVATURE_GAMMA)
#undef leaf_length
#define leaf_length(x) \\
	(LEAF_LENGTH_SCALE * func(LEAF_LENGTH,x) * L0^LEAF_LENGTH_GAMMA)
#undef leaf_width
#define leaf_width(x) \\
	(LEAF_WIDTH_SCALE * func(LEAF_WIDTH,x) * L0^LEAF_WIDTH_GAMMA)
#undef brangle
#define brangle(x) (BRANGLE_SCALE * func(BRANGLE,x))

Lsystem: 1

derivation length: 1
Axiom: A(0,L0)
/* Axiom: [f(1.5)]A(0,L0) */

/* Arguments to A: 
   x - cumulative length of the axis
   x_max - target length of the axis
*/

homomorphism
maximum depth: 200

A(x,x_max) : {x_rel = x/x_max; dx = int_length(x_rel); new_x = x+dx; }
		new_x < x_max --> 
	!(int_width(x_rel))+(curvature(x_rel)*dx)F(dx)
	[+(brangle(x_rel))L(x_rel)] 
	[-(brangle(x_rel))L(x_rel)] 
	A(new_x,x_max)

L(x_rel) : !LEAVES
	{llen = leaf_length(x_rel);
	lwid = leaf_width(x_rel); } -->
	^(30)!(3*lwid)F(llen)

L(x_rel) : LEAVES
	{llen = leaf_length(x_rel);
	lwid = leaf_width(x_rel); } -->
	^(30)/(90) @#(1) @Gt(1.3,0.5) @Gr(0) ; !(0) 
	@Gr(0,0,-90,lwid) 
	@Gs +(10) f(llen) +(10) !(0) 
	@Gr(40,0.75*lwid,0,0) @Ge(8)

endlsystem
