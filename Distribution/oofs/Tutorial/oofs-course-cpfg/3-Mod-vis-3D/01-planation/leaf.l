#define STEPS 1000
#define T0 50
#define INTERVAL 60
#define T1 230
#define T2 310
#define T3 400
#define T4 430

#define T5 645
#define T6 725
#define T7 815
#define T8 840
#define T9 990

#define sfun(n) ((n%INTERVAL)==2 && n> T0 && n<T1)
Lsystem: 1
Start: {n=1;}
StartEach: {n=n+1;}
derivation length: STEPS
Axiom: \(0,2);G
G : sfun(n) --> F[+\(0,1);G][-\(0,1);G]F;G
F : sfun(n) --> FF
; : sfun(n) --> *

\(a,c) : c==1 && n>T2 && n<=T3 --> \(a+1,c)
\(a,c) : c==2 && n>T4 && n<=T5 --> \(a+1,c)
\(a,c) : c==1 && n>T6 && n<=T7 --> \(a-1,c)
\(a,c) : c==2 && n>T8 && n<=T9 --> \(a+1,c)

endlsystem
