#define STEPS 2
#define TEXTURE 2
#define WID 0.8
#define TANG 1.2
#define NR 10
#define SEC 50

Lsystem: 1
derivation length: STEPS

Axiom: #(WID) @!(SEC) @Gt(TANG,1.5) @Tx(TEXTURE) @Gs X--f--X--f-- @Ge(NR)

X --> X+f+X--f--X+f+X

homomorphism

X --> @Gc(NR)

endlsystem
