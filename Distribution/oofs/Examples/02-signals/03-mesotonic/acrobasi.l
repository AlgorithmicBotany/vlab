#define STEPS 60
#define d 0 /* delay with which the signal is sent out */
#define m 3 /* plastochrone of the main axis */
#define n 4 /* plastochrone of the branch */
#define u 4 /* singal propagation rate in the main axis */
#define v 2 /* signal propagation rate in the branch */
#define p 6
#define r1 1.0
#define r2 0.6
#define r3 1.0
#define r4 1.5
#define r5 0.5
#define ANG 60
#define LEN 2

Lsystem: 0
derivation length: STEPS
ignore: +-/,

axiom: S(0)F(LEN,0)A(0)

S(i) < A(j) --> C(0),,,@o(r1);;;K
S(i) < B(j) --> C(0),,,@o(r1);;;K
A(i) : i<m-1 --> A(i+1)
A(i) : i==m-1 --> [+(ANG)F(LEN,1)B(0)]F(LEN,0)/(180)A(0)
B(i) : i<n-1 --> B(i+1)
B(i) : i==n-1 --> [+,,,,,,{-f(r5)+f(r5)+f(r5)-|-f(r5)+f(r5)+f(r5)}]
	F(LEN,1)/(180)B(0)
D(i) : i<d --> D(i+1)
D(i) : i==d --> S(0)
S(i) : i<u+v --> S(i+1)
S(i) : i==u+v --> *
S(i) < F(l,o) : (o==0)&&(i==u-1) --> ,#F(l,o)!;S(0)
S(i) < F(l,o) : (o==1)&&(i==v-1) --> ,#F(l,o)!;S(0)

,,,@o(s);;;  --> *
K --> [,,[{-f+f|-f+f}]++[{-f+f|-f+f}]++
	[{-f+f|-f+f}]++ [{-f+f|-f+f}]++
	[{-f+f|-f+f}]++[{-f+f|-f+f}],,@o(r2)]
C(t) : t<p --> C(t+1)
C(t) : t==p --> Z%
Z --> ,,,,,{.[-f(r4).][+f(r4).]}
L --> [{-f(r5)+f(r5)+f(r5)-|-f(r5)+f(r5)+f(r5)}]

endlsystem
