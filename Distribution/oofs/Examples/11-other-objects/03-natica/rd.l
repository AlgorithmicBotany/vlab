/* Number of cells in row */
#define NoCells 150

/* Activator diffusion rate */
#define Da 0.015
/* Activator decay rate */
/*#define mumax 0.1*/
/*#define mumin 0.02*/
#define mumax 0.38
#define mumin 0.08
/* Substrate diffusion rate */
#define Ds 0.05
/* Substrate decay rate */
#define nu 0

/* Substrate production rate */
/*#define sigmamax 0.05*/
/*#define sigmamin 0.01*/
#define sigmamax 0.19
#define sigmamin 0.04

/* Activator production: */
/* Coefficient of proportionality */
/*#define rhomax 0.1*/
/*#define rhomin 0.02*/
#define rhomax 0.38
#define rhomin 0.08
/* Level of activator saturation */
/*#define kappa 0.2*/
#define kappa 0.5
/* Base production */
#define rho0 0.01

/* Sin curve definition points */
/*
   |      |      |      |      |
   |    __|______|__    |      |
   |   /  |      |  \   |      |
   |__/   |      |   \__|______|
   |______|______|______|______|__
   |      |      |      |      |
   0      a      b      c      d
*/

/*    30  | 90  |  30  | 90  */
#define sina 30
#define sinb 120
#define sinc 150
#define sind 240

/* Sin curve scale */
#define hscale 4

/* Time Step */
#define dt 1

/* Width of each cell */
#define CellWidth 1

/* Cell width multiplier */
/*#define CWidthMult 0.1*/
#define CWidthMult 0.2

/* Height of each cell */
#define CellHeight ((CellWidth)*(CWidthMult))

/* Estimated height of the whole image, for placement purposes */
#define ImHeight (CellHeight*1500)

Lsystem: 0

consider: ;F

Start: {Step = 0;}
EndEach: {Step = Step + 1;}

derivation length: NoCells - 2

/* Each cell consists of two symbols: ; and F
	; - sets the colour for the cell; calculated from the activator
	F - has 6 parameters:
		l - width of the cell (CellWidth)
		a - concentration of activator
		s - concentration of substrate
		g - sigma
		r - rho
		m - mu
*/

Axiom: [|f(ImHeight)]!(CellHeight)|B+(90)
	;(1)F(CellWidth,0,0,sigmamin,rhomin,mumin)
	A(NoCells - 2)
	;(1)F(CellWidth,0,0,sigmamin,rhomin,mumin)

/*#define rhorand 5*/
#define rhorand 15
A(n) : n > 0 {x = (NoCells - n - 1)*hscale;
	radd = ran(rhomax)*rhorand/100;
	newrho = (ran(1) < 0.5) ? rhomax + radd : rhomax - radd;
	T = (x%sind);
	if (T < sina)
	{
		Tprime = T*90/sina;
		g = ((1 + sin(Tprime))/2)*(sigmamax - sigmamin) + sigmamin;
		r = ((1 + sin(Tprime))/2)*(newrho - rhomin) + rhomin;
		m = ((1 + sin(Tprime))/2)*(mumax - mumin) + mumin;
	} else {
	if (T < sinb)
	{
		g = sigmamax;
		r = newrho;
		m = mumax;
	} else {
	if (T < sinc)
	{
		Tprime = (T - sinb)*90/(sinc-sinb) + 90;
		g = ((1 + sin(Tprime))/2)*(sigmamax - sigmamin) + sigmamin;
		r = ((1 + sin(Tprime))/2)*(newrho - rhomin) + rhomin;
		m = ((1 + sin(Tprime))/2)*(mumax - mumin) + mumin;
	} else
	{
		g = sigmamin;
		r = rhomin;
		m = mumin;
	}}}
	} -->
	;(81)F(CellWidth,0,0,g,r,m)A(n-1)
A(n) --> *

B : (Step < NoCells - 2) --> B
B --> g(0)

;(cl)F(ll,al,sl,gl,rl,ml) < ;(c)F(l,a,s,g,r,m) > ;(cr)F(lr,ar,sr,gr,rr,mr) :
	(Step >= NoCells - 2)
	{ a2 = a*a;
	  a_prod = r*s*((a2)/(1 + (kappa*a2)) + rho0);
	  new_a = a + (a_prod - (m*a) +	Da*((al - a) + (ar - a)))*dt;
	  new_s = s + (g - a_prod - (nu*s) + Ds*((sl - s) + (sr - s)))*dt;
	  tmp_c = 80*new_a + 1;
	  new_c = (tmp_c > 255) ? 255 : ((tmp_c < 1) ? 1 : tmp_c);
	} -->
	;(new_c)F(l,new_a,new_s,g,r,m)

g(l) --> g(l+CellHeight)

endlsystem
