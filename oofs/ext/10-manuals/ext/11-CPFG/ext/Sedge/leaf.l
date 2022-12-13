#define W_RATE 1.04
#define WIDTH .05
/* there's a problem if the rates are different and I change
   one of the lengths to f from F , like now */
#define L_RATE 1.05
#define LENGTH .16
lsystem: 4
derivation length: 90
/* axiom parameters: starting delay, time to turn, new elasticity */
axiom: +(45)\(45)#(.1)leaf(1,11,-.02)

leaf(S,A,T) : S>0 --> leaf(S-1,A,T)
leaf(S,A,T) : !(S>0) --> [A(S,S+A,T)][B(S,S+A,T)]
f(n) : n<15 --> f(n*W_RATE)
F(n) : n<15 --> F(n*L_RATE)
A(a,A,T) : a==A --> _(T){[-(90)f(.05)].{.F(.16)/(45)[-(90)f(.05)}.}],A(a+1,A,T)
B(a,A,T) : a==A --> _(T){[+(90)f(.05)].{.F(.16)/(45)[+(90)f(.05)}.}],B(a+1,A,T)
A(a,A,T) --> {[-(90)f(WIDTH)].{.F(LENGTH)[-(90)f(WIDTH)}.}],A(a+1,A,T)
B(a,A,T) --> {[+(90)f(WIDTH)].{.F(LENGTH)[+(90)f(WIDTH)}.}],B(a+1,A,T)
endlsystem
