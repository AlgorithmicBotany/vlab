#define STEPS 180
Lsystem: 1
derivation length: STEPS
ignore: EF/^&\,;+-012345678f|9pmZ#!
Axiom: E///T(0)FFFA(0)

T(a) : a==60 --> b
T(a) --> T(a+1)
b < a --> b
d < a --> b
b < A(a) --> U(0)[;B(0)~w]
d < A(a) --> U(0)[;B(0)~w]
A(a) : a==3 --> a[&FFFx][&&!{-F+F+F-|-F+F+F-}]FFF////////A(0)
A(a) --> A(a+1)
U(a) : a==2 --> cd
U(a) --> U(a+1)
b > c --> t(0)
c < x --> A(0)
c > d --> u(0)
t(a) : a==0 --> c
t(a) --> t(a+1)
u(a) : a==1 --> d
u(a) --> u(a+1)

B(a) : a==3 --> C(0)%
B(a) --> B(a+1)
C(a) : a==0 --> C(1)~z
C(a) : a==3 --> D(0)%
C(a) --> C(a+1)
D(a) : a==0 --> ,D(1)[FF,/[,^F][!{&&&&-F+F-|-F+F}]////
	[,^F][!{&&&&-F+F-|-F+F}]////
	[,^F][!{&&&&-F+F-|-F+F}]////
	[,^F][!{&&&&-F+F-|-F+F}]////
	[,^F][!{&&&&-F+F-|-F+F}]]
D(a) : a==20 --> D(a+1)%
D(a) : a>20 --> ;;~f
D(a) --> D(a+1)

E --> _E

endlsystem


