#define VIGOR nran(1,0.02)

Lsystem: 1
derivation length: 31

Axiom: |[+(30)FA][-(30)FB]

/* replace a tentacle that is allowed to grow
   with a line terminated by an apex */

f?E(x) : x==1 --> ;FA
g?E(x) : x==1 --> ;FB

/* display a tentacle that is not alllowed to grow */

f?E(x) : x==0 --> h
g?E(x) : x==0 --> h

decomposition

/* replace an apex by tentacles with sensors */

A --> [f?E(VIGOR)][-(60)g?E(VIGOR)]
B --> [+(60)f?E(VIGOR)][g?E(VIGOR)]

homomorphism

f --> g
/* h --> #(1);(255)F */

endlsystem
