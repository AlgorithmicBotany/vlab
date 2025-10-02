#define SIZE 1200
Lsystem: 1
derivation length: 630
ignore: z
Axiom: /(180)f(SIZE+SIZE)-(90)f(SIZE+SIZE)-(90)f(SIZE)-(90)f(SIZE)A(0)
* < A(n) > * : * --> A(n+1)[+(n*137.5)f(30*n^0.5)C(n)]
* < C(m) > * : !(m>440) --> [;(120)~s(1)]
* < C(m) > * : m>440&&!(m>565) --> [/(180);(184)~r(1)]
* < C(m) > * : m>565&&!(m>580) --> [^(5)~m(1)]
* < C(m) > * : m>580&&!(m>595) --> [&(35)~n(1)]
* < C(m) > * : m>595&&!(m>610) --> [&(10)~o(1)]
* < C(m) > * : m>610 --> [^(15)~l(1)]
endlsystem






