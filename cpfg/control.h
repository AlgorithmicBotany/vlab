/* ******************************************************************** *
   Copyright (C) 1990-2022 University of Calgary
  
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * ******************************************************************** */



#ifndef _CPFG_CONTROL_
#define _CPFG_CONTROL_

#ifndef WIN32
#ifndef LINUX
#include <device.h>
#endif /* LINUX */
#endif /* WIN32 */

#include "drawparam.h"
#include "viewparam.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define D_TO_R(x) (((double)x) * (M_PI) / 180.0)
#define R_TO_D(x) (((double)x) * 180.0 / M_PI)

#define STOP 0
#define RUN 1
#define FOREVER 2
#define SINGLE 3

#define MAXSTR 150000  /* default maximum string size */
#define COLORMAP 1     /* default color map */
#define MAXPARMLEN 160 /* maximum length of parameter string */
#define MAXPARMS 20    /* maximum number of parameters in a module */
#define MAXLSYSTEMS 10 /* maximum number of (sub) L-systems */
#define MAXNESTING 100 /* maximum nesting of sub-L-systems */
#define MAXSTRINGS 255 /* maximum number of strings as parameters - JH1 */

#define PARAMSIZE sizeof(float)
#define SKIPSIZE (PARAMSIZE + 1)

/* s,t - texture coordinates */
#define TEX_S 0
#define TEX_T 1

/* In some rendering routines, the vertex is specified by an float array
   of 9 items: 3 point coordinates, 3 normal coordinates, 2 texture (texel)
               coordinates and 1 color index (int stored in float format).
   When computing surface points additional 2 field for s and t tangent is
   necessary.
   */
/* this array may have form: float vertex[5][3] */

enum { ePOINT = 0, eNORMAL = 1, eTEXTURE = 2, eSTANGENT = 3, eTTANGENT = 4 };

/* or float vertex[11] */
#define POINT_X 0
#define POINT_Y 1
#define POINT_Z 2
#define NORMAL_X 3
#define NORMAL_Y 4
#define NORMAL_Z 5
#define TEXTURE_S 6
#define TEXTURE_T 7
#define COLOR_FRONT 8
#define COLOR_BACK 9
#define DRAW_LINE 10
#define PITEM 11 /* size of one vertex */

/* for updateFlag */
enum eUpdate { NO_UPDATE = 0, UPDATE_ROTATION, UPDATE_SCALE, UPDATE_PAN };

/* cursor type */
#define CURSOR_ARROW 0
#define CURSOR_HOURGLASS 1

#define SAVE_OFFSET 256
#define NEW_FILE_OFFSET 512
#define OUTPUT_ANIMATE_OFFSET 768

#ifdef CPFG_ENVIRONMENT
enum {
  COMM_NONE,
  COMM_LOCAL
#if CPFG_VERSION >= 3200
  ,
  COMM_DISTRIBUTED
#endif
};
#endif

/*
        StringModule is a structure to hold a string symbol and its
        parameters.
*/

struct StringModule {
  int parameters; /* The number of associated parameters */
  int length;     /* The length of the token, symbol and parameters */
  union {
    float value;
    char bytes[4];
  } actual[MAXPARMS];
  char symbol; /* The symbol */
};

typedef struct StringModule StringModule;

#define SUBSTR_INITIALSIZE 2000
struct SUBSTRING {
  char *first, *last;
  float turtle_index;  /* float for compatibility with module parameters */
  float cut_parameter; /* parameter of the cut symbol */
};
typedef struct SUBSTRING SUBSTRING;

/********** work modes ***************/
#define NO_GRAPHICS_MODE 1 /* off-screen rendering */
#define SLAVE_MODE                                                             \
  2 /* as a slave of draw_server - for distributed                             \
       system */
#define PIPE_MODE                                                              \
  3                   /* off-screen rendering -                                \
                         string (binary) coming from stdin                     \
                         piping out the results */
#define SERVER_MODE 4 /* expects command from a client */

/* local prototypes */
void SaveFile(char *name, int format);
void InitializeDispatcher(void);
void OpenOutputFile(char *filename, int format);

/* functions for handling strings as parameters - JH1 */
double GlobalStringIndex(char *string); /* JH1 */
char *GlobalString(double index);       /* JH1 */
void Warning(char *message, int level);
void WarningParsing(char *message, int level);

void FirstRun(void);

enum RedrawFlags {
  rfScale = 1 << 0,
  rfViewVol = 1 << 1,
  rfClear = 1 << 2,
  rfCenter = 1 << 3,
  rfHCenter = 1 << 4,
  rfShadows = 1 << 5
};

void Redraw(unsigned int flags);
void StartRedraw(unsigned int flags);

int ContinueRedraw(void);
int IdleFunction(void);
int IdleFunctionContinue();
void SelectInMenu(int pupval);
int InputString(char *filename, char type);
void FreeStrings(void);
int QueryWorkMode(void);
void DisplayFrame(float);
void OutputObj(void);

void CalculateViewVolume(char *string, const DRAWPARAM *drawparamPtr,
                         VIEWPARAM *viewparamPtr);
void CalculateWindow(
    const VIEWPARAM *viewparamPtr, RECTANGLE *windowPtr,
    float aspect_ratio); // MC - added function prototype to header file
void SetView(VIEWPARAM *viewPtr);
void RegenerateString(void);
void Rerun(void);

void SaveRGB();
void SavePNG();
void SaveTGA();
void SaveBMP();
void SaveRayshade();
void SaveString();
void SaveGLS();
void SaveVV();
void SavePostscript();
void SaveOBJ();

void InsertX(int x, int y);

unsigned int getClearBetweenFrameFlag();

extern VIEWPARAM viewparam;

#ifdef __cplusplus
}
#endif

#endif /* _CPFG_CONTROL_ */
