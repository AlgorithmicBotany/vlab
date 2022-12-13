#define STEPS 31
#define d 10
#define m 2
#define n 5
#define u 1
#define v 3
#define p 3
#define r1 1.0
#define r2 0.6
#define r3 1.0
#define r4 1.5
#define r5 0.5

Lsystem: 0
derivation length: STEPS
ignore: +-/,

axiom: D(0)F(3,0)A(0)

S(i) < A(j) --> C(0)
S(i) < B(j) --> C(0)
A(i) : i<m-1 --> A(i+1)
A(i) : i==m-1 --> [+F(3,1)B(0)]F(3,0)/(180)A(0)
B(i) : i<n-1 --> B(i+1)
B(i) : i==n-1 --> [+L]F(3,1)/(180)B(0)
D(i) : i<d --> D(i+1)
D(i) : i==d --> S(0)
S(i) : i<u+v --> S(i+1)
S(i) : i==u+v --> *
S(i) < F(l,o) : (o==0)&&(i==u-1) --> ,#F(l,o)!;S(0)
S(i) < F(l,o) : (o==1)&&(i==v-1) --> ,#F(l,o)!;S(0)

C(t) : t<=p --> C(t+1)

homomorphism
maximum depth: 1

L --> ,,,,,,{-f(r5)+f(r5)+f(r5)-|-f(r5)+f(r5)+f(r5)}

C(age) : age == 0 --> ,,,@o(r1);;;
C(age) : age <= p --> [,,[{-f+f|-f+f}]++[{-f+f|-f+f}]++
	[{-f+f|-f+f}]++ [{-f+f|-f+f}]++
	[{-f+f|-f+f}]++[{-f+f|-f+f}],,@o(r2)]
C(age) : age>p --> ,,,,,{.[-f(r4).][+f(r4).]}


endlsystem




