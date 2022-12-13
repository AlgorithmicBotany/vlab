target: Lsystem

type: LABEL
name: GENERAL
color: 255,255,0
origin: 129 0

type: SLIDER
name: Steps
colors: 255,255,255 255,0,0
origin: 20 30
min/max: 0 800
value: 650
message: d STEPS %d 1

type: SLIDER
name: Growth Limit
colors: 255,255,255 255,0,0
origin: 172 30
min/max: 0 800
value: 250
message: d GROWTH_LIMIT %d 1

type: LABEL
name: ANGLES
color: 255,255,0
origin: 131 90

type: SLIDER
name: Branching angle
colors: 255,255,255 255,0,0
origin: 20 120
min/max: 0 61
value: 40
message: d ANG %d 1

type: SLIDER
name: Phyllotaxis
colors: 255,255,255 255,0,0
origin: 172 120
min/max: 0 361
value: 90
message: d PHI %d 1

type: SLIDER
name: Angle change duration
colors: 255,255,255 255,0,0
origin: 20 180
min/max: 0 100
value: 30
message: d TBANG %d 1

type: SLIDER
name: Tropism
colors: 255,255,255 255,0,0
origin: 172 180
min/max: -200 200
value: 50
message: d TROPISM %d 1000

type: LABEL
name: PROPORTIONS
color: 255,255,0
origin: 41 245

type: LABEL
name: TIMINGS
color: 255,255,0
origin: 217 245

type: SLIDER
name: Segment scale
colors: 255,255,255 255,0,0
origin: 20 275
min/max: 0 150
value: 58
message: d INTLENSCALE %d 100

type: SLIDER
name: Branching delay 1
colors: 255,255,255 255,0,0
origin: 172 275
min/max: 0 100
value: 40
message: d D1 %d 10

type: SLIDER
name: Internode length
colors: 255,255,255 255,0,0
origin: 20 335
min/max: 1 500
value: 300
message: d LEN1 %d 10

type: SLIDER
name: Branching delay 2
colors: 255,255,255 255,0,0
origin: 172 335
min/max: 0 100
value: 70
message: d D2 %d 10

type: SLIDER
name: Pedicel length
colors: 255,255,255 255,0,0
origin: 20 395
min/max: 1 500
value: 140
message: d LEN2 %d 10

type: SLIDER
name: Bud opening time
colors: 255,255,255 255,0,0
origin: 172 395
min/max: 1 120
value: 18
message: d T1 %d 10

type: SLIDER
name: Leaf scale
colors: 255,255,255 255,0,0
origin: 20 455
min/max: 1 50
value: 10
message: d LEAFSCALE %d 10

type: SLIDER
name: Flowering time
colors: 255,255,255 255,0,0
origin: 172 455
min/max: 1 300
value: 120
message: d T2 %d 10

type: SLIDER
name: Leaf growth duration
colors: 255,255,255 255,0,0
origin: 20 515
min/max: 0 100
value: 39
message: d TLEAF %d 1

type: SLIDER
name: Fruit formation time
colors: 255,255,255 255,0,0
origin: 172 515
min/max: 1 300
value: 60
message: d T3 %d 10

type: SLIDER
name: Petal scale
colors: 255,255,255 255,0,0
origin: 20 575
min/max: 1 50
value: 20
message: d PETALSCALE %d 10

type: SLIDER
name: Fruit development time
colors: 255,255,255 255,0,0
origin: 172 575
min/max: 1 200
value: 100
message: d T4 %d 10
