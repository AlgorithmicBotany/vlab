#define STEPS 151
#define R 1.02
#define NR 4
#define LEN 1
#define WID 5

#define ANG1 -20
#define ANG2 3
#define ANG3 80    /* rotation of the contour */

Lsystem: 0
derivation length: STEPS
Axiom: @#(2)&(90-ANG1/2)#(WID)@Gr(1)@Gt(1.0)\(ANG3)@Gs/(ANG3)A(1)

A(s) --> +(ANG1)\(ANG2)F(s)A(s*R)

homomorphism

F(s) --> f(s)#(s*WID)+(ANG1/2)\(ANG3)@Gc(NR)/(ANG3)-(ANG1/2)

endlsystem
