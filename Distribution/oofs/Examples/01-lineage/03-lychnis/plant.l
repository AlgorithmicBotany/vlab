	/* initial line widths were 2 and .2 */
	/* for ray-tracing use 20 and 2 */
#define STEPS 28

#define VANG 45
#define S0 100
#define ANG 31
#define L_SIZE 1.5
#define D1 3
#define D2 7
#define LEN1 20
#define LEN2 10
#define T1 1
#define T2 5
#define T3 11
Lsystem: 1
derivation length: STEPS
Axiom: E/(VANG)&(5)A(0)

	/*	Apex */
A(t) : t>0 --> A(t-1)
A(t) : t==0 --> F(S0)X(LEN1)[L][/(180)L]
	[!F(S0)X(LEN2);(120)QB(0)][@v-(ANG)!!!!A(D2)]
	[@v+(ANG)!!!!A(D1)]
X(a) : a>0 --> F(S0)X(a-1)

	/* Colour and line width */
Q --> ;;;Q
E --> E#

	/* Blossom */
B(a) : a==0 --> B(1)~b(1.0)
~b(s) : s<1.4 --> ~b(s*1.1)
B(a) : a==T1 --> B(a+1)%
B(a) : a==T1+1 --> B(a+1)~k(1.0)^(90)
	[&(36)~p(1.0)]+(72)[&(36)~p(1.0)]+(72)[&(36)~p(1.0)]
	+(72)[&(36)~p(1.0)]+(72)[&(36)~p(1.0)]
B(a) : a==T2 --> B(a+1)%
B(a) : a==T2+1 --> B(a+1)~k(1.33)^(90)
	[~p(2.0)]+(72)[~p(2.0)]+(72)[~p(2.0)]
	+(72)[~p(2.0)]+(72)[~p(2.0)]
~k(s) : s<1.5 --> ~k(s*1.1)
~p(s) : s<2.5 --> ~p(s*1.15)
B(a) : a==T3 --> B(a+1)%
B(a) : a==T3+1 --> ;(192)Q~f(1.0)
B(a) --> B(a+1)
~f(s) : s>.6 --> ~f(s*.9)

	/* Leaf */
L --> ~l(0.1)
~l(s) : s<L_SIZE --> ~l(s+.05)

endlsystem
