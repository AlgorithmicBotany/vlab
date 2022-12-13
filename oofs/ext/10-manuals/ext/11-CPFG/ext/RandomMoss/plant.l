#define STEPS 5
#define P1 33
#define P2 33
#define P3 33

Lsystem: 1
seed: 2454
derivation length: STEPS
Axiom: F
F --> F[!,+F]F[!,-F]F : P1
F --> F[!,+F]F : P2
F --> F[!,-F]F : P3
Endlsystem
