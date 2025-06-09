#ifndef RENDER_H
#define RENDER_H
/*
 * "Dither matrices" used to encode the 'number' of a ray that samples a
 * particular portion of a pixel.  Hand-coding is ugly, but...
 */
static int *SampleNumbers;
static int OneSample[1] = {0};
static int TwoSamples[4] = {0, 2, 3, 1};
static int ThreeSamples[9] = {0, 2, 7, 6, 5, 1, 3, 8, 4};
static int FourSamples[16] = {0, 8,  2, 10, 12, 4, 14, 6,
                              3, 11, 1, 9,  15, 7, 13, 5};
static int FiveSamples[25] = {0, 8, 23, 17, 2,  19, 12, 4,  20, 15, 3,  21, 16,
                              9, 6, 14, 10, 24, 1,  13, 22, 7,  18, 11, 5};
static int SixSamples[36] = {6,  32, 3,  34, 35, 1,  7,  11, 27, 28, 8,  30,
                             24, 14, 16, 15, 23, 19, 13, 20, 22, 21, 17, 18,
                             25, 29, 10, 9,  26, 12, 36, 5,  33, 4,  2,  31};
static int SevenSamples[49] = {
    22, 47, 16, 41, 10, 35, 4,  5,  23, 48, 17, 42, 11, 29, 30, 6,  24,
    49, 18, 36, 12, 13, 31, 7,  25, 43, 19, 37, 38, 14, 32, 1,  26, 44,
    20, 21, 39, 8,  33, 2,  27, 45, 46, 15, 40, 9,  34, 3,  28};
static int EightSamples[64] = {
    8,  58, 59, 5,  4,  62, 63, 1,  49, 15, 14, 52, 53, 11, 10, 56,
    41, 23, 22, 44, 45, 19, 18, 48, 32, 34, 35, 29, 28, 38, 39, 25,
    40, 26, 27, 37, 36, 30, 31, 33, 17, 47, 46, 20, 21, 43, 42, 24,
    9,  55, 54, 12, 13, 51, 50, 16, 64, 2,  3,  61, 60, 6,  7,  57};
#define RFAC 0.299
#define GFAC 0.587
#define BFAC 0.114

#define NOT_CLOSED 0
#define CLOSED_PARTIAL 1
#define CLOSED_SUPER 2
/*
 * If a region has area < MINAREA, it is considered to be "closed",
 * (a permanent leaf).  Regions that meet this criterion
 * are drawn pixel-by-pixel rather.
 */
#define MINAREA 9

#define SQ_AREA(s) (((s)->xsize + 1) * ((s)->ysize + 1))

#define PRIORITY(s) ((s)->var * SQ_AREA(s))

#define INTENSITY(p) ((RFAC * (p)[0] + GFAC * (p)[1] + BFAC * (p)[2]) / 255.)

#define OVERLAPS_RECT(s)                                                       \
  (!Rectmode ||                                                                \
   ((s)->xpos <= Rectx1 && (s)->ypos <= Recty1 &&                              \
    (s)->xpos + (s)->xsize >= Rectx0 && (s)->ypos + (s)->ysize >= Recty0))

typedef unsigned char RGB[3];

static RGB **Image;
static char **SampleMap;

/*
 * Sample square
 */
typedef struct SSquare {
  short xpos, ypos, xsize, ysize;
  short depth;
  short leaf, closed;
  float mean, var, prio;
  struct SSquare *child[4], *parent;
} SSquare;

//SSquare *SSquares,
SSquare *SSquareCreate(), *SSquareInstall(), *SSquareSelect(),
    *SSquareFetchAtMouse();

Float SSquareComputeLeafVar();

#define SSCLOSED (SuperSampleMode + 1)
#define ALL_SUPERSAMPLED 2

void DivideFunc(void);

void Render(int argc,char **argv);
void SSquareSample(int x, int y, int supersample);
void SSquareDivideToDepth( SSquare *sq,int d);
int  RecomputePriority(SSquare *sq);
void SSquareDraw(SSquare *sq);
void DrawPixels(int xp, int yp, int xs, int ys);

void SSquareDivide(SSquare *sq);
void SSquareRecomputeStats(SSquare *sq);

#endif
