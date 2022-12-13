#define STEPS 20
#define SegmentLength 0.7

#define InitRotation 45
#define InitInclination 45
#define InitOrientation 45

#define OrthoStrength 0
#define PlagioStrength 0
#define TwistStrength 0

/* arrows */

#define AFL 0.4
#define AHL 0.2
#define ABL (AFL + AHL/2)

Lsystem: 0
derivation length: STEPS
Axiom: /(InitRotation) &(InitInclination) \(InitOrientation)@Tf 
		;(1) @#(1) @Gs [U][L] A @Ge(10)

A -->   @Ts(1,OrthoStrength)S(SegmentLength)N /* orthotropism */
	@Ts(2,PlagioStrength)f(0) N         /* plagiotropism */
	@Ts(3,TwistStrength)f(0) N       /* twist */
	@Gc(10) [U][L] A

homomorphism 
maximum depth: 2

S(x) --> [;(2) F(x)] f(x)

U --> [N#(0.10);(3)[^(90)F(AFL)[#(0.20)&(90)@c^(90)F(AHL)#(0)]]
	[&(90)F(ABL)&(90)@c]]
L --> [N#(0.10);(4)/(90)[^(90)F(AFL)[#(0.20)&(90)@c^(90)F(AHL)#(0)]]
	[&(90)F(ABL)&(90)@c]]

N --> @Ts(1,0)@Ts(2,0)@Ts(3,0)

endlsystem
