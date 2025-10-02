#define Width 0.1
Lsystem: 1
derivation length: 1

Axiom: ;(9)@;(9) !(Width) @Gs I(0.3,Width) [\(137.5) &(45)L(0.9)] 
	^(5+ran(10)) I(0.3,Width)A(6,1.0)
/*
Axiom: K(0.1)
*/

homomorphism
maximum depth: 20

A(n,c) : n>0 --> [&(45)L(1)] [&(45)B] ^(5+ran(10))
	I(0.5,Width*sqrt(c)) 
	/(137.5) A(n-1,c-0.1)

A(n,c) : n==0 --> [&(45)L(0.5)] ^(5+ran(10)) I(0.5,Width*sqrt(c)) @Ge K(0.5)

B --> J(0.5,0.05) [/(137.5) &(45)L(0.7)] J(1.0,0.05) ^(ran(5))
	@Ge K(0.5) 

I(l,w) --> f(l/2) !(w) @Gc f(l/2)
J(l,w) --> f(l/2) !(w) ^(ran(5)) @Gc f(l/2)

L(s) --> /(90) @Tx(0) @#(1) @Gt(1.3,1.3) @Gr(1) ;(8) @;(7) 
	!(0.05*s) @Gr(0,0,0,1) @Gs 
	f(0.3*s) !(0.20*s) @Gr(10,1,10,1) @Gc(14)
	+(30) /(-15+ran(30)) f(0.5*s) !(0.40*s) @Gr(5,1,5,1) @Gc(14)
	+(30) /(-15+ran(30)) f(0.5*s) !(0.45*s) @Gr(0,1,0,1) @Gc(14)
	+(15) /(-15+ran(30)) f(0.5*s) !(0.35*s) @Gr(-7,1,-7,1) @Gc(14)
	f(0.5*s) !(0.0) @Gr(-20,1,0,1) @Ge(14)

K(s) --> &(90)[^(60)P(s)]+(120)[^(60)P(s)]+(120)[^(60)P(s)]
	+(60)[^(45)P(s)]+(120)[^(45)P(s)]+(120)[^(45)P(s)]
	[f(0.05*s)^(90)S(s)]
	+(60)[f(0.05*s)^(90)S(s)]
	+(60)[f(0.05*s)^(90)S(s)]
	+(60)[f(0.05*s)^(90)S(s)]
	+(60)[f(0.05*s)^(90)S(s)]
	+(60)[f(0.05*s)^(90)S(s)]
	[^(90)U(0.6*s)]

P(s) -->  /(90) @Tx(0) @#(3) @Gt(1.3,1.3) @Gr(1) ;(2) @;(1)
	!(0.05*s) @Gr(0,0,0,1) @Gs
	+(10) f(0.3*s) !(0.15*s) @Gr(10,1,10,1) @Gc(14)
	+(20) f(0.5*s) !(0.80*s) @Gr(15,1,15,1) @Gc(14)
	+(10) f(0.5*s) !(1.00*s) @Gr(0,1,0,1) @Gc(14)
	+(10) @#(4) f(0.5*s) !(0.60*s) @Gr(-25,1,-25,1) @Gc(14)
	-(40)f(0.5*s) !(0.0*s) @Gr(-90,0.2,0,1) @Ge(14)

U(s) --> @Gr(1) !(0.1*s);(5) @;(5)
	@#(5) @Gt(1.3,1.3) @Gr(0,1,5,0.8)
	@Gs f(s) &(8) f(s) !(0.2*s) @Gr(10,0.8,90,1) @Gc(14)
	f(s*0.1) !(0.0*s) @Gr(-80,0.8,90,1) @Ge(4)

S(s) --> !(0.08*s);(3) @;(3) @Gs f(0.7*s) &(10+ran(20)) f(0.7*s) !(0.03*s) @Ge
	T(0.2*s)

T(s) --> &(5+ran(20))|f(s)|@Gr(1) !(0.0 );(4) @;(4)
	@#(6) @Gt(1.3,1.3) @Gr(0,1,90,0.8)
	@Gs f(s) &(10) f(s) !(0.0) @Gr(-90,0.8,0,1) @Ge(14)

/* Notes 
   @Tx(0) no texture, @Tx(1) - use texture 1.
   @#(x)  use contour x
   !(1)   line width 1
   @Gs,   start cylinder, 
   @Gc(n) continure g. cylinder with 3 mesh strips.
   @Ge(n) end g. cylinder with n mesh strips.
   @Gt(x,y) length of tangents to the curve being swept.
   @Gr(angle1, length1, angle2, length2) slope and length of the two tangents
      vectors to the contour
   @Gr(0) - tangent adjustment off, @Gr(1) - tangent adjustment on
   ;(8), @;(7) - index to the front and back materials.
*/

endlsystem
