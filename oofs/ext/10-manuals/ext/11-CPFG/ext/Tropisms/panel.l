target: Lsystem
panel name: L-system
background: 0,0,0
size: 173 589
font: Arial,14,-1,5,50,0,0,0,0,0,Regular

type: SLIDER
name: Number of segments
colors: 255,255,255 227,232,9
origin: 16 25
min/max: 0 40
value: 20
message: d STEPS %d 1

type: SLIDER
name: Rotation around vertical
colors: 255,255,255 239,242,237
origin: 17 197
min/max: -180 180
value: 45
message: d InitRotation %d 1

type: SLIDER
name: Inclination
colors: 255,255,255 236,239,233
origin: 16 258
min/max: 0 180
value: 45
message: d InitInclination %d 1

type: SLIDER
name: Rotation around heading
colors: 255,255,255 232,236,228
origin: 16 317
min/max: -180 180
value: 45
message: d InitOrientation %d 1

type: SLIDER
name: Orthotropism x 100
colors: 255,255,255 214,159,255
origin: 16 417
min/max: -100 100
value: 0
message: d OrthoStrength %d 100

type: LABEL
name: TROPISM STRENGTH
color: 255,255,0
origin: 14 383

type: SLIDER
name: Plagiotropism x 100
colors: 255,255,255 203,135,255
origin: 16 473
min/max: 0 100
value: 0
message: d PlagioStrength %d 100

type: SLIDER
name: Twist x 100
colors: 255,255,255 0,232,9
origin: 16 526
min/max: -100 100
value: 0
message: d TwistStrength %d 100

type: SLIDER
name: Segment length x 100
colors: 255,255,255 227,232,9
origin: 16 85
min/max: 0 100
value: 70
message: d SegmentLength %d 100

type: LABEL
name: INITIAL ORIENTATION
color: 255,255,0
origin: 15 155
