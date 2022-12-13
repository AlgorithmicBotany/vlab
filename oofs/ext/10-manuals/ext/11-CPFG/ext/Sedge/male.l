#define MI_RATE 1.025    /* internode growth rate */
#define MS_RATE 1.02     /* seed growth rate */

lsystem: 3
derivation length: 76
axiom: / / / /F(5)A(0)

spikem --> A(0)
A(t) : t<55 --> F(.2)[B]/(137.5)A(t+1)
B --> ^(5)[~n(1)]
F(t) : t<1 --> F(t*MI_RATE)
^(a) : a<15 --> ^(a*MS_RATE)
~n(t) : t<3 --> ~n(t*MS_RATE)

endlsystem
