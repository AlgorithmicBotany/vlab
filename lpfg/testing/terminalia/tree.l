#include <lpfgall.h>

derivation length: 5;

#define R1 0.94     /* contraction ratio and vigour 1 */
#define R2 0.87     /* contraction ratio and vigour 1 */
#define A1 24.4     /* branching angle 1 */
#define A2 36.9     /* branching angle 2 */
#define ALPHA 138.5 /* divergence angle */

module A(float);    /* an apex */
module T(int);
module B(float);    /* a branch */

consider: E1;

/* draw the two plants */
Axiom:  SetWidth(0.072) Up (90.0) /* in cpfg this is done in the view file */
        SetColor(2) SB() T(1) EB()
        Right(90.0) f(2.2) Left(90.0) SB() SetColor(4) T(0) EB();

/* used to generate 5 lateral branches */
T(d) :
{
  produce SB() Right(90.0) RollR(180.0*d) B(1) E1(1) A(1) EB()
          RollR(ALPHA) SB() Right(90.0) RollR(180.0*(1-d)) B(1) E1(1) A(1) EB()
          RollR(ALPHA) SB() Right(90.0) RollR(180.0*d) B(1) E1(1) A(1) EB()
          RollR(ALPHA) SB() Right(90.0) RollR(180.0*(1-d)) B(1) E1(1) A(1) EB()
          RollR(ALPHA) SB() Right(90.0) RollR(180.0*d) B(1) E1(1) A(1) EB();
}

E1(r) < A(l) :
{
  if (r == 1)
  {
    /* grow to new branches */
    produce SB() Up(A2) B(l*R2) E1(0.8) A(l*R2) EB()
            SB() Down(A1) B(l*R1) E1(0.9) RollR(180.0) A(l*R1) EB();
  }
  else
  {
    /* change apex to have a vigour of one so nothing can grow around it */
    produce E1(1);
  }
}

/* in cpfg this is called homomorphism */
interpretation:

/* draws an apex as a sphere */
A(l) :
{
  produce Sphere0();
}

/* draws a branch */
B(l) :
{
  produce Sphere0() F(l)
          SB() DecColor() RollToVert() Down(90.0) f(ran(0.01)) Up(90.0) Circle(0.5) EB();
}
