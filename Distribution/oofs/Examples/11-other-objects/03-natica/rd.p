panel name: Activator-Substrate
background: 48
size: 250 570

type: PAGE
name: VIEW PARAMETERS
color: 0
origin: 57 530
message: p 1

type: SLIDER
name: Number of Cells (x10)
colors: 0 14
origin: 60 460
min/max: 1 100
value: 15
message: d NoCells %d 0.1

type: SLIDER
name: Cell Width (x0.01)
colors: 0 9
origin: 60 395
min/max: 1 100
value: 10
message: d CellWidth %d 10

type: SLIDER
name: Cell Height (x0.1)
colors: 0 9
origin: 60 330
min/max: 1 100
value: 2
message: d CWidthMult %d 10

type: LABEL
name: (x Cell Width)
color: 0
origin: 65 310

type: SLIDER
name: Time Step (x0.1)
colors: 0 13
origin: 60 240
min/max: 1 100
value: 10
message: d dt %d 10

type: SLIDER
name: Sin Horiz. Scale (x0.1)
colors: 0 12
origin: 60 175
min/max: 0 100
value: 40
message: d hscale %d 10

type: PAGE
name: ACTIVATOR PARAMETERS
color: 0
origin: 35 530
message: p 2

type: LABEL
name: -------------------------
color: 0
origin: 13 490

type: LABEL
name: Activator Production
color: 0
origin: 35 450

type: SLIDER
name: Saturation Level
colors: 0 13
origin: 60 385
min/max: 0 100
value: 20
message: d kappa %d 100

type: LABEL
name: (kappa - x0.01)
color: 0
origin: 59 370

type: SLIDER
name: Base production
colors: 0 13
origin: 60 305
min/max: 0 100
value: 10
message: d rho0 %d 1000

type: LABEL
name: (rho0 - x0.001)
color: 0
origin: 62 290

type: LABEL
name: -------------------------
color: 0
origin: 13 260

type: SLIDER
name: Activator Diffusion
colors: 0 10
origin: 60 195
min/max: 0 100
value: 15
message: d Da %d 1000

type: LABEL
name: (Da - x0.001)
color: 0
origin: 72 180

type: PAGE
name: SUBSTRATE PARAMETERS
color: 0
origin: 35 530
message: p 3

type: SLIDER
name: Substrate Diffusion
colors: 0 10
origin: 60 460
min/max: 0 100
value: 5
message: d Ds %d 100

type: LABEL
name: (Ds - x0.01)
color: 0
origin: 74 445

type: SLIDER
name: Substrate Decay
colors: 0 10
origin: 60 380
min/max: 0 100
value: 0
message: d nu %d 100

type: LABEL
name: (nu - x0.01)
color: 0
origin: 72 365

type: PAGE
name: MIN/MAX PARAMETERS
color: 0
origin: 35 530
message: p 3

type: SLIDER
name: Max. Activator Decay
colors: 0 10
origin: 60 460
min/max: 0 100
value: 10
message: d mumax %d 100

type: LABEL
name: (mu_max - x0.01)
color: 0
origin: 53 445

type: SLIDER
name: Min. Activator Decay
colors: 0 10
origin: 60 380
min/max: 0 100
value: 2
message: d mumin %d 100

type: LABEL
name: (mu_min - x0.01)
color: 0
origin: 53 365

type: SLIDER
name: Max. Substrate Production
colors: 0 12
origin: 60 300
min/max: 0 100
value: 50
message: d sigmamax %d 1000

type: LABEL
name: (sigma_max - x0.001)
color: 0
origin: 36 285

type: SLIDER
name: Min. Substrate Production
colors: 0 12
origin: 60 220
min/max: 0 100
value: 10
message: d sigmamin %d 1000

type: LABEL
name: (sigma_min - x0.001)
color: 0
origin: 36 205

type: SLIDER
name: Max. Coef. of Proport.
colors: 0 13
origin: 60 140
min/max: 0 100
value: 10
message: d rhomax %d 100

type: LABEL
name: (rho_max - x0.01)
color: 0
origin: 50 125

type: SLIDER
name: Min. Coef. of Proport.
colors: 0 13
origin: 60 60
min/max: 0 100
value: 2
message: d rhomin %d 100

type: LABEL
name: (rho_min - x0.01)
color: 0
origin: 50 45
