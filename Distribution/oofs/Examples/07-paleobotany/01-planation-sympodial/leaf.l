#define STEPS 1100
#define T0 50
#define INTERVAL 60
#define T1 260
#define T2 340
#define T3 430
#define T4 460

#define T5 675
#define T6 755
#define T7 845
#define T8 870
#define T9 1020

#define sfun(n) ((n%INTERVAL)==2 && n> T0 && n<T1)
Lsystem: 1
Define: {array ang[4] = {15,20,22,30};}

Start: {n=1;}
StartEach: {n=n+1;}
derivation length: STEPS
Axiom: \(0,2);G
G : sfun(n) --> F(1.2)[+(ang[0],0)\(0,1);G][-(ang[0],0)\(0,1);G]
+(a,i) : sfun(n) --> +(ang[i+1],i+1)
-(a,i) : sfun(n) --> -(ang[i+1],i+1)
F(l) : sfun(n) --> F(l*1.2)
; : sfun(n) --> *

\(a,c) : c==1 && n>T2 && n<=T3 --> \(a+1,c)
\(a,c) : c==2 && n>T4 && n<=T5 --> \(a+1,c)
\(a,c) : c==1 && n>T6 && n<=T7 --> \(a-1,c)
\(a,c) : c==2 && n>T8 && n<=T9 --> \(a+1,c)

endlsystem
