#define SIZE 1200
#define CURVING 0.5
#define SPREAD 0.30
#define SEEDS 00
#define RAYS 89
#define CON SEEDS+RAYS
#define CURVV 90/(CON)*CURVING
#define PETAL1 8
#define PETAL2 5
#define PETANG 130
#define DROOPING 0.05
#define DROP PETANG/(PETAL1+PETAL2)*DROOPING
#define PETSIZE 1.5
#define SIZEINC 0.000
#define ADJ 90*CURVING
Lsystem: 1
Start: {srand(3);}
derivation length: 1
ignore: z
Axiom: C(20)
z --> z
decomposition
maximum depth: 100

C(n) : n>0 --> [&(90)f(4*n^0.5)^(90)P(n)]/(137.5)C(n-1)
P(n) : * {l = ran(1);} --> [&(20+2.5*n+ran(10));(1)S(4,l)^(90)
	A(0,0.8+0.2*l,70+ran(70))]

S(n,l) --> ;(8)!(1.2) @Gr(1.7,1.7) @Gs T(n,l)

T(n,l) : n>0 --> !(.8+0.1*n)+(-5+ran(10))&(5-ran(10))[L(n)]f(10+5*l+ran(10))
	/(137.5)@Gc(16)T(n-1,l) 
T(n,l) : n<=0 --> f(5.0+ran(2.5))@Ge(16)

homomorphism
maximum depth: 200

L(n) --> @Tx(2) @#(1) @Gt(1.3,1.3) @Gr(1) ;(8) @;(7) !(0) @Gs
	+(60) f(16+n+ran(n)) /(-30+ran(60)) +(15+ran(30)) !(0) @Gr(-10,0.1,0,0) @Ge(18)

/* flower head */
A(n,l,petang) : n<SEEDS+RAYS+PETAL1+PETAL2 
	--> A(n+1,l,petang)D[+(n*137.5)^(n*CURVV)f(SPREAD*l*n^0.5)C(n,l,petang)]
C(n,l,petang) : (n<SEEDS) --> [;(1)@;(1)~s(l)]
C(n,l,petang) : (n<CON) --> [/(180);(2)~r(l)]
C(n,l,petang) : n<CON+PETAL1 --> [&((petang-90)-(n-(CON))*DROP+ADJ);(3)
	f(1.8*l)&(90)f(0.3*l)^(90)~t((PETSIZE+SIZEINC*(n-CON))*l)]
C(n,l,petang) : * --> [&((petang-90)-(n-(CON))*DROP+ADJ);(3)
	f(1.8*l)&(90)f(0.3*l)^(90)~u((PETSIZE+SIZEINC*(n-CON))*l)]
endlsystem
