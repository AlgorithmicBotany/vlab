#define dir 0
#define NR 32

Lsystem: 0
ring lsystem
derivation length: 1
Start: {ang = 180/NR;}
Axiom: X(NR)

A < B : dir==0 --> A
B > A : dir==1 --> A
A --> B

decomposition
maximum depth: 1000

X(n) : n>1 --> X(n-1)B
X(n) : n==1 --> A

homomorphism
maximum depth: 2

A --> ;(1)S
B --> ;(2)S
S --> [+(90)F(5)]-(ang)f-(ang)

endlsystem
