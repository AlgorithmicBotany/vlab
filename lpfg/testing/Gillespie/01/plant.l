/* using gillespie's method to solve Lotka-Volterra equations */

#include <lpfgall.h>
#include <cmath>

const float TIME_AXIS_SCALE = 1.f;
const float MOLECULES_AXIS_SCALE = 100.f;

const float c1 = 0.001;
const float c2 = 0.01;
const float c3 = 1.0;

const float X = 1000.0;

float gillespie_time = 0.f;

Start:{UseGroup(1);}
StartEach:{gillespie_time = GillespieTime();}

module Cell(float,float);
module Axes();
module PhaseSpace();
module ArrowHead();

derivation length: 40000;
ignore: F;

Axiom: SB() Axes() Cell(100,100) EB() PhaseSpace();

/* --------------------------------------------------------- */

ggroup 1:
Cell(Y1, Y2):
{
  propensity (X * Y1 * c1) produce Cell(Y1+1, Y2);
  propensity (Y1 * Y2 * c2) produce Cell(Y1-1, Y2+1);
  propensity (Y2 * c3) produce Cell(Y1, Y2-1);
}

/* --------------------------------------------------------- */

group 0:

interpretation:
maximum depth: 10;

Cell(Y1, Y2):
{
  /* draw the number of molecules over time */
  produce
          /* molecules of Y1 */
          SB()
            SetColor(2)
            f((Y1) / MOLECULES_AXIS_SCALE)
            Right(90) f(gillespie_time *0.1f) /// TIME_AXIS_SCALE)
            Circle(0.05)
          EB()

          /* molecules of Y2 */
          SB()
            SetColor(5)
            f((Y2) / MOLECULES_AXIS_SCALE)
            Right(90) f(gillespie_time *0.1f) /// TIME_AXIS_SCALE)
            Circle(0.05)
          EB()

          SB()
            SetColor(3)
            Right(90) f(12)
            f((Y1)/100.0)
            Left(90) f((Y2)/100.0)
            Circle(0.05)
          EB()
          ;
}

PhaseSpace():
{
  produce Right(90) f(12)
          SB() F(10) ArrowHead() EB()
          SB() f(0.5) Right(90) f(0.5) Label("Y1 molecules") EB()
          SB() Left(90) F(10) ArrowHead() EB()
          SB() f(-0.5) Left(90) f(10) Label("Y2 molecules") EB()
          ;
}

Axes():
{
  produce
  SB() SetColor(1)

  /* the time axis */
  SB()
    Right(90) f(0)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.2) Right(90) f(0.75)
    SetColor(3) Label("0")
  EB()

  SB()
    Right(90) f(1.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("10")
  EB()

  SB()
    Right(90) f(2.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("20")
  EB()

  SB()
    Right(90) f(3.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("30")
  EB()

  SB()
    Right(90) f(4.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("40")
  EB()

  SB()
    Right(90) f(5.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("50")
  EB()

  SB()
    Right(90) f(6.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("60")
  EB()

  SB()
    Right(90) f(7.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("70")
  EB()

  SB()
    Right(90) f(8.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("80")
  EB()

  SB()
    Right(90) f(9.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("90")
  EB()

  SB()
    Right(90) F(10.0*TIME_AXIS_SCALE)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Right(90) f(0.75)
    SetColor(3) Label("100")
  EB()

  SB()
    Right(90) F(11.0*TIME_AXIS_SCALE) ArrowHead
  EB()

  SB()
    f(-1.5) Right(90) f(0.2*TIME_AXIS_SCALE) Label("time")
  EB()

  /* the molecule axis */
  SB()
    f(0)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("0")
  EB()

  SB()
    f(1)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("100")
  EB()

  SB()
    f(2)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("200")
  EB()

  SB()
    f(3)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("300")
  EB()

  SB()
    f(4)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("400")
  EB()

  SB()
    f(5)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("500")
  EB()

  SB()
    f(6)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("600")
  EB()

  SB()
    f(7)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("700")
  EB()

  SB()
    f(8)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("800")
  EB()

  SB()
    f(9)
    SB() Left(90) F(0.1) EB()
    SB() Right(90) F(0.1) EB()
    f(0.0) Left(90) f(1.45)
    SetColor(3) Label("900")
  EB()

  SB()
    F(10) ArrowHead Left(90) f(2.0) Label("molecules")
  EB()

  EB();
}

ArrowHead():
{
  produce SP() SB() PP() Right(160) f(0.3) PP() EB()
               SB() Left(160) f(0.3) PP() EB()
          EP();
}
