/* #define STEPS 90 */
#define I0 30
#define S0 150
#define K0 70
#define KANG1 10
#define KANG2 20
#define SRATE 1.01
#define LRATE 1.05
#define BANG0 0
#define BANGI 1.0
#define BANGL 10.0
#define T0 5   /* the number of leaf pairs */
#define T1 20  /* the number of buds */
#define T2 40  /* the number of open flowers */
#define T3 25  /* the number of fruits */

Lsystem: 1
derivation length: T0+T1+T2+T3+4
Axiom: [-(5)/(35)#(2),(47)F(I0*10)A(0)]

/* Produce the vegetative part - decussate phyllotaxis */
A(t) : t<T0 --> [-(BANG0,t)!(47)~l(0.5,t)]/(180)
	[-(BANG0,t)!(47)~l(0.5,t)]/(95)F(I0*6)!A(t+1)

/* Produce the inflorescence - spiral phyllotaxis */
A(t) : t >= T0 --> [&(BANG0+15)F(S0/4)&(15)F(S0/8)]
	[!(2)&(BANG0)G(S0)X(0)~b(0.5)]/(137.5)F(I0)!A(t+1)

b(s) --> b(s*SRATE)
X(t) : t<T1 --> X(t+1)
X(t) : t==T1 --> 
	[C(0),(127)K/(72),K/(72),K/(72),K/(72),K!(2),(122)G(120)]%

K --> [!(1)&(KANG1){-(KANG2)F(K0)+(KANG2)F(K0)+(KANG2)F(K0)-(KANG2)|
	-(KANG2)F(K0)+(KANG2)F(K0)+(KANG2)F(K0)}]

C(t) : t<T2 --> C(t+1)
C(t) : t==T2 --> D%

D --> ~f(0.3),(121)F(80)&(10)F(60)&(10)F(40)

F(s) --> F(s*SRATE)
f(s) --> f(s*SRATE)
G(s) --> G(s*SRATE)
l(s,t) : t<T0 && s<1.2 --> l(s*LRATE,t+1)

&(a) : a<135 --> &(a+BANGI)
-(a,t) : a<90 && t<T0 --> -(a+BANGL,t+1)
* < #(r) : r<6 --> #(r+0.1)

endlsystem
