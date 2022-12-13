#define I_RATE 1.01    /* internode growth rate */
#define S_RATE 1.05    /* seed growth rate */

lsystem: 2
derivation length: 76
axiom: / / / /F(5)spikef

spikef --> [&(30)/(180)~f(2.25)#(.1)F(.5)]F(.1)/(180)[&(30)/(180)~f(2.25)#(.1)F(.5)]/(137.5)A(0)
A(t): t<75 --> F(.2)[B]/(137.5)A(t+1)
B > * : * --> &(35)[~f(1)]/(180)[~f(1)][~c(1)#(.1)F(.5)]
F(t) : t<1 --> F(t*I_RATE)
&(a) : a<50 --> &(a*S_RATE)
~f(t) : t<2 --> ~f(t*S_RATE)
~c(t) : t<2 --> ~c(t*S_RATE)

endlsystem
