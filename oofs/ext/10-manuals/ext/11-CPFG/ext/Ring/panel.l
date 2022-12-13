target: Lsystem
panel name: L-system
background: 0,0,0
size: 170 198
font: Arial,14,-1,5,50,0,0,0,0,0

type: BUTTON
name: Left
colors: 255,255,255 0,0,255
origin: 32 116
value: 0
message: d dir 1 1

type: BUTTON
name: Right%Copy1
colors: 255,255,255 0,0,255
origin: 33 86
value: 1
message: d dir 0 1

type: GROUP
color: 255,255,255
Left
Right%Copy1
ENDGROUP

type: LABEL
name: Propagation direction
color: 255,255,0
origin: 17 153

type: SLIDER
name: Number of segments
colors: 255,255,255 255,0,0
origin: 17 25
min/max: 2 100
value: 32
message: d NR %d 1
