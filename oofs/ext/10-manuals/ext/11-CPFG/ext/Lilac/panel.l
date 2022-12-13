target: Lsystem
panel name: L-system
background: 0,0,0
size: 192 590
font: Arial,14,-1,5,50,0,0,0,0,0

type: PAGE
name: Set 1
color: 255,255,255
origin: 67 570
message: d NOTHING %d 1

type: BUTTON
name: Twig
colors: 255,255,255 0,0,255
origin: 44 28
value: 1
message: d A_SW 1 1

type: BUTTON
name: Cluster
colors: 255,255,255 0,0,255
origin: 45 58
value: 0
message: d A_SW 2 1

type: BUTTON
name: Floret
colors: 255,255,255 0,0,255
origin: 45 87
value: 0
message: d A_SW 3 1

type: GROUP
color: 255,255,255
Twig
Cluster
Floret
ENDGROUP

type: LABEL
name: Branching structure
color: 255,255,255
origin: 34 120

type: BUTTON
name: Yes
colors: 255,255,255 0,0,255
origin: 44 153
value: 1
message: d SHOW_PETALS 1 1

type: BUTTON
name: No
colors: 255,255,255 0,0,255
origin: 45 183
value: 0
message: d SHOW_PETALS 0 1

type: GROUP
color: 255,255,255
Yes
No
ENDGROUP

type: LABEL
name: Show petals
color: 255,255,255
origin: 57 220

type: SLIDER
name: Initial branch width
colors: 255,255,255 255,0,0
origin: 25 258
min/max: 1 500
value: 100
message: d D_INIT %d 100

type: SLIDER
name: Pipe model exponent
colors: 255,255,255 255,0,0
origin: 25 309
min/max: 1 100
value: 25
message: d D_EXP %d 100

type: SLIDER
name: Petal size
colors: 255,255,255 255,0,0
origin: 25 386
min/max: 0 100
value: 32
message: d P_SIZE %d 10

type: SLIDER
name: Petal angle
colors: 255,255,255 255,0,0
origin: 25 445
min/max: 0 100
value: 40
message: d P_ANG %d 1

type: SLIDER
name: Branch twist
colors: 255,255,255 255,0,0
origin: 25 514
min/max: 0 181
value: 85
message: d TWIST %d 1
