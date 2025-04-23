/*
PROGRAM:	cpfg
MODULE:		control.c
COPYRIGHT:  (c) 1989,1990 Jim Hanan and P. Prusinkiewicz
 **************************************************************************
 *      This software is copyright, with all rights reserved.             *
 *      Under the copyright laws, this software cannot be copied, in      *
 *      whole or in part, without the written consent of the authors,     *
 *      except for normal use or to make a backup copy of the software.   *
 *      Under the law, copying includes translating into another language *
 *      or format. You can make modifications and extensions to the       *
 *      software for your own use, but you may not distribute the         *
 *      resulting software in any form.                                   *
 **************************************************************************
AUTHOR:		J. Hanan and P. Prusinkiewicz
STARTED ON:	April 17, 1987
PURPOSE:	This main-line module provides communications between the
                string-generating module, the string-interpreting
                module and the user.  It allows interaction with the user
                using menus, and provides control required for animation.
MODIFIED:	Fall 1989		BY: Jim
                Added parameters
MODIFIED:	February 1990	BY: Jim
                Added parsing code for rayshade and postscript output command
line parameters and new menu entry to handle output.

                Added hourglass cursor for long waits.

                Converted patch wireframes to use hardware.
MODIFIED:	March 1990	BY: Jim
                Fixed double buffering in animate mode.
                Added ps and rayshade output to animate menu.
MODIFIED:	October 1990	BY: Jim
                Added view volume output command line parameter
                Restructured mainline for one menu.
                Added interactive rotations
MODIFIED:	November 1990	BY: Jim
                Fixed swapinterval for rotations in animate mode.
MODIFIED:	December 1990	BY: Jim
                took out rescaling on new Lsystem menu choice.
                replaced obsolete setdepth call with lsetdepth
                        - still some question about glcompat(GLC_ZRANGEMAP)
                replaced clear and zclear with czclear
MODIFIED:	Summer 1991	BY: Jim at Apple
                added MonaLisa calls selectable by using ML compile-time switch.
MODIFIED:	November 1991	BY: Jim
                added output in Ned Greene's xpoly format

MODIFIED:	March-June 1994	BY: Radek
                ansi standard + prepared for C++
                changed call for query processing (not TurtleDraw)
                slightly better filename input
                command line options
                  -V verbose mode
                  -v warnings on

MODIFIED:       Oct 94   BY: Radek
                OpenGL

                by May 95   BY: Radek
                better calculation of view volume (for all view points)
                per partes interpretation

 ***********************************************************************
 Known bugs:

 A line drawing symbol (FfGg) with parameters does not take into account
 the step_size whereas the line surface and surfaces in general do.
 An F without parameters draws a line of length STEP_SIZE.
 An F with parameters should draw a line of length STEP_SIZE*parameter1.
 The temporary solution is to scale the line: surface in the .v file by .01.
*/

#define GL_SILENCE_DEPRECATION

#ifdef WIN32
#include "warningset.h"
#endif

#ifdef WIN32
#include <image.h>
#else
#include "image.h"
#endif

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef WIN32
#include <unistd.h>
#include <ctype.h>
#else
#include <io.h>

#endif

#include <limits.h>

#ifndef WIN32
#include <sys/time.h>
#endif

#include "platform.h"
#include "platformmenu.h"
#include "interpret.h"
#include "control.h"
#include "generate.h"
#include "blackbox.h"
#include "irisGL.h"
#include "rayshade.h"
#include "postscript.h"
#include "outputgls.h"
#include "targa.h"
#include "rle.h"
#include "utility.h"
#include "viewVol.h"
#include "animparam.h"
#include "comlineparam.h"
#include "userfiles.h"
#include "object.h"
#include "writergb.h"
#include "shaders.h" // MC - July 2016 - added GLSL shaders for shadow mapping

#ifdef CPFG_ENVIRONMENT
#include <comm_lib.h>
#endif

#ifdef RESEARCH_VER
#include "server.h"
#endif
#include "test_malloc.h"

int relativeSize;
int relativePos;

/*
        The external data structure clp holds names of files and other
        parameters specified in the command line.
*/

COMLINEPARAM clp;

/*
        Lsystem data is external so yyparse can get at it.
*/
LSYSDATA *LsystemList;

#ifdef WIN32
extern void SaveAsBmp(const char *);
#endif

int process_contours(void);
void InitializeSurfaces(void);
int checkSurfaceFiles(int);
void Change_Resolution(void);
int is_menu_up(void);
void RereadColors();
void RereadSurfaces();
void RereadContours();
int RereadFunctions();
void RereadFunctionsRerun();
void RereadCurveXYZRerun();
int RereadCurveXYZ();
int RereadAllDrawParams();
int RereadDrawParams();
void DrawExpired();

SymbolTable *currentSymbolTable; /* symbol table stack */
SymbolTable *globalSymbolTable;  /* global symbol table */
int lineNumber;
char *inputFile;
int inputError;
int expired = 0;
int oflist = 0;
int initializeAlpha=0;

/*
        DisplayThisFrame determines whether the next step should be displayed in
   the animation: -1 = don't display 0 = default (follow specifications in the
   anim file) 1 = display
*/

int DisplayThisFrame = 0;

int validLsystem = 0; /* Set to 1 when an L-system has been read successfully */

/* global variables for handling strings passed as indexes in parameters - JH1
 */
/* accessed by both generate and interpret steps - JH1 */

#define MAXSTRLEN 80
char globalStrings[MAXSTRINGS][MAXSTRLEN];
int nextGlobalString; /* JH1 */

/* flags controlling animation processing */
int animateFlag = 0;
int animatesave = -1;
int swapinterval = 0;

/* will the program start with the animate menu? */
int start_with_animate = 0;

/* is necessary to update rotation or scaling? */
int updateFlag;

RECTANGLE viewWindow;

char *type_strings[SAVE_COUNT] = {
				  "rgb", "png", "bmp", "gif", "jpg", "pbm", "tiff", "ras",    "tga",         "rle",         "rayshade",
    "postscript", "string", "view volume", "string dump", "gls format",
#ifdef WIN32
    "BMP",
#endif
    "OBJ"};

char *extension_strings[SAVE_COUNT] = {
    ".rgb", ".png", ".bmp", ".gif", ".jpg", ".pbm", ".tiff", ".ras", ".tga",
    ".rle", ".ray", ".ps",  ".str", ".vv",  ".strb", ".gls", ".obj"};

extern int enabled_menus;
extern int double_buffering;
extern int pixmaps_exist;
extern int interpret_new_model;

char *currentString = NULL, *currentStringEnd;
unsigned long currentStringLength;
char *nextString = NULL, *nextStringEnd;

char pipe_mode;
char isObjectProduction;

#ifdef WIN32
/* Semaphore to synchronize with L-studio */
HANDLE hLSemaphore = NULL;
#endif

/************ local prototypes **********************/
static void ParseCommand(int argc, char **argv);
static void Usage(void);
static void StringInit(char **stringPtr, char **stringEnd);
static void PrepareRedraw(unsigned int rflags);
static void Rewind(char **current, char **currentEnd, LSYSDATA *LsystemListPtr);
static void GenerateNSteps(int derivations, char **current, char **currentEnd,
                           char **next, char **nextEnd,
                           LSYSDATA *LsystemListPtr, DRAWPARAM *drawparamPtr,
                           VIEWPARAM *viewparamPtr);
static int GetAnimStep(int *current_step, ANIMPARAM *aptr);
void SetView(VIEWPARAM *viewPtr);
static void savewindow(char *name, int rgb);
static char *makeSaveName(char *base, char *suffix);
static turtleDrawDispatcher *SetScreenDispatcher(DRAWPARAM *dr, VIEWPARAM *vw);
void OutputString(int frame_num, char add_number_to_name);

#ifdef JIM
void InitialiseForStart(LSYSDATA *lsysPtr);
#endif

/********** MAIN EVENT HANDLING *****************************************/

VIEWPARAM viewparam;
DRAWPARAM drawparam;
ANIMPARAM animparam;
static turtleDrawDispatcher *screenDrawRoutines;

/************************************************************************/
/*
  Sets screen dispatcher routines
*/

void InitializeDispatcher(void) {
  screenDrawRoutines = SetScreenDispatcher(&drawparam, &viewparam);
}

/************************************************************************/
/*
   "FirstRun" performs the first generation of L-system string after
   cpfg execution
*/
void FirstRun(void) {

  /* Read the view file */
  ReadViewData(clp.vfilename, &drawparam, &viewparam);


  ReadAnimData(clp.afilename, &animparam);
  my_SetBufferType((char)DoubleBuffer(&animparam));
  if (!DoubleBuffer(&animparam))
    double_buffering = 0;
  validLsystem = ReadLsystem(clp.lfilename);

#ifdef CPFG_ENVIRONMENT
  if (clp.communication == COMM_LOCAL) {
    CMInitialize();
    CMAddProcess(clp.efilename);
  }
#endif

  if (validLsystem) {
    /* create a desired font */
#ifndef LINUX
    makeRasterFont();
#endif
    //[PASCAL] initialize random generator
    if (LsystemList->seed != 0){
      unsigned short seed[3] = {LsystemList->seed,0,155};
      seed48(&seed[0]);
    }

    /* Generate the string to the desired depth */
    Rewind(&currentString, &currentStringEnd, LsystemList);
    GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                   &nextString, &nextStringEnd, LsystemList, &drawparam,
                   &viewparam);

    /* set initial rotation */
    viewparam.xRotation = 0;
    viewparam.yRotation = 0;
    viewparam.zRotation = 0;
    viewparam.xPan = 0;
    viewparam.yPan = 0;

    init_blackbox(); /* initialize blackbox functions	*/

  } else {
    enabled_menus = 0;
  }

  /* set interaction flags */
  animateFlag = 0;
  updateFlag = NO_UPDATE;
}

/************************************************************************/
/*
   "Redraw" redraws the screen.
*/

extern int pending_interpretation;
static int clear_rendering = 1; // MC - July 2016 - flag is set to 1 to clear
                                // render buffer The flag is necessary to avoid
                                // introducing parameter to ContinueRedraw()
int ReadBackgroundSceneFile(const char *);
void Redraw(unsigned int rflags) {
  PrepareRedraw(rflags);

  if (drawparam.render_mode == RM_SHADOWS)
    beginMainShader();
  else
    endMainShader();

  // MC July 2016 - moved buffer clearing to here from PrepareRedraw()
  
  if (rflags & rfClear) {
    if (!DoubleBuffer(&animparam)) {
      glDrawBuffer(GL_BACK);
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      glDrawBuffer(GL_FRONT);
    }
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  }
  
  if (validLsystem)
    TurtleDraw(currentString, &drawparam, &viewparam);
}

void StartRedraw(unsigned int rflags) {
  //Pascal - March 2021, if we don't reread the view arameters the environment is not fully reset
#ifdef CPFG_ENVIRONMENT
  if (clp.communication == COMM_LOCAL) {
    ReadBackgroundSceneFile(viewparam.backgroundFilename );
  }
#endif

  PrepareRedraw(rflags);

  if (validLsystem)
    TurtleDrawInitialize(currentString, &drawparam, &viewparam);
}

void PrepareRedraw(unsigned int rflags) {
  if (pending_interpretation)
    TurtleDrawFinishUp();

  // MC - July 2016 - moved the buffer clearing to ContinueRedraw() and Redraw()
  // If buffer clearing is performed here, flickering may result in continuous
  // mode: e.g., if Panels are used to change the model (new L-system is called
  // rapidly), the following buffer clearing will be performed multiple times
  // before anything is drawn The multiple buffer clears can happen in both
  // shaded and shadows render modes.
  if (rflags & rfClear){
    clear_rendering = 1;
  }
  else{
    clear_rendering = 0;
  }
  if (validLsystem) {
    if (rflags & (rfScale | rfViewVol)) {
      if (rflags & rfViewVol) {
        CalculateViewVolume(currentString, &drawparam, &viewparam);
      }
      SetView(&viewparam);
    }
    if ((rflags & rfHCenter) && !(rflags & rfCenter)) {
      VIEWPARAM vp = viewparam;
      float oldViewPointX, newViewPointX, difPosX;

      CalculateViewVolume(currentString, &drawparam, &vp);
      oldViewPointX = 0.5f * (viewparam.min[0] + viewparam.max[0]);

      newViewPointX = 0.5f * (vp.min[0] + vp.max[0]);
      difPosX = oldViewPointX - newViewPointX;
      viewparam.vrp[0] = viewparam.viewpoint[0] = newViewPointX;
      viewparam.xPan = -difPosX;
      SetView(&viewparam);
    }
    if (rflags & rfCenter) {
      VIEWPARAM vp = viewparam;
      float oldViewPointX, newViewPointX, difPosX;
      float oldViewPointY, newViewPointY, difPosY;

      CalculateViewVolume(currentString, &drawparam, &vp);
      oldViewPointX = 0.5f * (viewparam.min[0] + viewparam.max[0]);
      newViewPointX = 0.5f * (vp.min[0] + vp.max[0]);
      difPosX = oldViewPointX - newViewPointX;
      viewparam.vrp[0] = viewparam.viewpoint[0] = newViewPointX;
      viewparam.xPan = -difPosX;

      oldViewPointY = 0.5f * (viewparam.min[1] + viewparam.max[1]);

      newViewPointY = 0.5f * (vp.min[1] + vp.max[1]);
      difPosY = oldViewPointY - newViewPointY;
      viewparam.vrp[1] = viewparam.viewpoint[1] = newViewPointY;
      viewparam.xPan = difPosY;

      SetView(&viewparam);
    }
    // MC - July 2016 - added flag for rendering the shadow map
    if (rflags & rfShadows) 
    {
      if (drawparam.render_mode == RM_SHADOWS) {
        // save double_buffering global state variable (do not use
        // my_SetBufferType it is much slower)
        int save_double_buffering = double_buffering;

        // set opengl rendering
        DRAWPARAM dp = drawparam;
        dp.tdd = screenDrawRoutines;
        dp.output_type = TYPE_OPENGL;

        // resize shadow map if necessary
        resizeShadowMap();

        // disable double buffering before rendering shadow map
        double_buffering = 0;

        // initialize turtle before drawing with shadow map
        TurtleDrawInitialize(currentString, &dp, &viewparam);

        // render to shadow map
        beginShadowMap();
        TurtleDrawNsteps(drawparam.interpretation_step);
        endShadowMap();

        // put back the double buffering state
        double_buffering = save_double_buffering;
      }
    }

    /* rendering specification */
    drawparam.tdd = screenDrawRoutines;
    drawparam.output_type = TYPE_OPENGL;
  }
}
/************************************************************************/
/* returns 1 when Redraw is finished */
int ContinueRedraw(void) {
  if (pending_interpretation) {
    if (drawparam.render_mode == RM_SHADOWS)
      beginMainShader();
    else
      endMainShader();
    // MC July 2016 - moved buffer clearing to here from PrepareRedraw()
    if (clear_rendering) {
      if (!DoubleBuffer(&animparam)) {
        glDrawBuffer(GL_BACK);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        glDrawBuffer(GL_FRONT);
      }

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    }

    if (TurtleDrawNsteps(drawparam.interpretation_step)) {
      return 1;
    } else {
      return 0;
    }
  }

  return 1;
}

/************************************************************************/
void NewView(void) {
  if (clp.iscolormapfile)
    load_in_colormaps();

  if (clp.ismaterialfile)
    load_in_materials();

  ReadViewData(clp.vfilename, &drawparam, &viewparam);

  /* create a desired font */
#ifndef LINUX
  makeRasterFont();
#endif

  /* set screen draw dispatcher routines */
  screenDrawRoutines = SetScreenDispatcher(&drawparam, &viewparam);

  /* reset initial rotation */
  viewparam.xRotation = 0;
  viewparam.yRotation = 0;
  viewparam.zRotation = 0;
  viewparam.xPan = 0;
  viewparam.yPan = 0;

  init_blackbox(); /* initialize blackbox functions */
#ifdef WIN32
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

/************************************************************************/
/*
   "Select in Menu" is called by motif callback after a menu item is
   selected. Each menu button has assosiated unique numerical code which
   is passed to this function in 'pupval'
*/

void SelectInMenu(int pupval) {
  int store_step = 0;
  int rereadOk = 0;
  float scale = 1.f;
  float min[3];
  float max[3];
  unsigned int redrawf;
  clp.filesFullyLoaded = 1;

  switch (pupval) {
  case 11:
    interpret_new_model = 1;

    /* New model. Reread all input files.
             Recalculate the viewing volume, and regenerate
    the image */
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    ReadViewData(clp.vfilename, &drawparam, &viewparam);
    validLsystem = ReadLsystem(clp.lfilename);
    if (validLsystem) {
      enabled_menus = 1;

      if (clp.iscolormapfile)
        load_in_colormaps();

      if (clp.ismaterialfile)
        load_in_materials();

      ReadAnimData(clp.afilename, &animparam);
      my_SetBufferType((char)DoubleBuffer(&animparam));

      if ((animateFlag == RUN) || (animateFlag == FOREVER))
        swapinterval = (int)animparam.swap_interval;
      else
        swapinterval = 0;

#ifdef CPFG_ENVIRONMENT
      if (clp.communication == COMM_LOCAL) {
        CTerminate();
        CMInitialize();
        CMAddProcess(clp.efilename);
      }
#endif

            /* create a desired font */
#ifndef LINUX
      makeRasterFont();
#endif
      //[PASCAL] initialize random generator
      if (LsystemList->seed != 0){
        unsigned short seed[3] = {LsystemList->seed,0,155};
	    seed48(&seed[0]);
      }

      Rewind(&currentString, &currentStringEnd, LsystemList);
      GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                     &nextString, &nextStringEnd, LsystemList, &drawparam,
                     &viewparam);

      /* set screen draw dispatcher routines */
      screenDrawRoutines = SetScreenDispatcher(&drawparam, &viewparam);

      /* reset initial rotation */
      viewparam.xRotation = 0;
      viewparam.yRotation = 0;
      viewparam.zRotation = 0;
      viewparam.xPan = 0;
      viewparam.yPan = 0;

      init_blackbox(); /* initialize blackbox functions	*/
      // MC - July 2016 - regenerate the shadow map in case the shadow
      // parameters have changed

      StartRedraw(rfScale | rfViewVol | rfClear | rfShadows);
    }

    interpret_new_model = 0;

    break;

  case 12:
    /* New L-system.  Read the L-system parameters
    and regenerate the image accordingly */
    interpret_new_model = 1;

    rereadOk = RereadAllDrawParams();
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    if (animateFlag) {
      swapinterval = 0;
    }
    if (clp.communication == COMM_LOCAL) {
      rereadOk = ReadBackgroundSceneFile(viewparam.backgroundFilename );
    }

    // why was there a while loop here to test a valid L-system?
    // if there is a syntax error, this will cause an infinite loop
    // because the user can't fix it and request a "New L-system".
    //validLsystem = 0;
    //while (!validLsystem)
      validLsystem = ReadLsystem(clp.lfilename);

    if (clp.communication == COMM_LOCAL) {
      CTerminate();
      CMInitialize();
      CMAddProcess(clp.efilename);
      if (validLsystem) {
        LsystemList->current = animparam.last;
	//	printf("LSYSTEM NEW LSYSTEM: %d - %d\n",LsystemList->n,LsystemList->current);
      }

    }

    if ((validLsystem) && (rereadOk)) {
      enabled_menus = 1;

      Rewind(&currentString, &currentStringEnd, LsystemList);
      GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                     &nextString, &nextStringEnd, LsystemList, &drawparam,
                     &viewparam);

 
      init_blackbox(); /* initialize blackbox functions */
      redrawf = rfClear;

      //[PASCAL] This is used to rescale if the number of derivation in the
      // Lsystem has been changed
      if (ScaleBetweenFrames(&animparam))
        redrawf |= rfScale | rfViewVol;

      StartRedraw(redrawf | rfShadows);

    }
    if (animateFlag) {
      swapinterval = animparam.swap_interval;
    }
    interpret_new_model = 0;

    break;

  case 16: /* New homomorphism.  Read the L-system parameters
          and regenerate the image accordingly */

    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    if (animateFlag) {
      swapinterval = 0;
    }

    if (LsystemList != NULL)
      store_step = LsystemList->current;

    validLsystem = ReadLsystem(clp.lfilename);
    if (validLsystem) {
      enabled_menus = 1;

#ifdef JIM
      InitialiseForStart(LsystemList);
#else
      /* Evaluate Array Initializers */
      EvaluateArrayInitializers(LsystemList);

      /* Make sure global file definitions have been cleared */
      FreeFileDefSpace();

      /* Evaluate global start statements */
      EvaluateStartStatements(LsystemList);

#endif
      init_blackbox(); /* initialize blackbox functions */

      // MC - July 2016 - set flag to regenerate the shadow map if in 'shadows'
      // mode
      StartRedraw(rfClear | rfShadows);

      LsystemList->current = store_step;
    }
    if (animateFlag) {
      swapinterval = animparam.swap_interval;
    }
    break;

  case 17: /* New resolution.  Just redraw */

    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    if (validLsystem) {
      if (animateFlag) {
        swapinterval = 0;
      }
      Change_Resolution();

      StartRedraw(rfClear);
      if (animateFlag) {
        swapinterval = animparam.swap_interval;
      }
      swapinterval = SINGLE;
    }
    break;

  case 13:
    /* New view.  Read the view parameters
            and regenerate the image accordingly */
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    if (validLsystem) {
      if (animateFlag) {
        swapinterval = 0;
      }
      NewView();
      redrawf = rfScale;
      redrawf |= rfViewVol | rfClear | rfShadows;
      // MC - July 2016 - set flag to regenerate the shadow map if in 'shadows'
      // mode
      StartRedraw(redrawf);
      // continueredraw is not needed. It is called later

      if (animateFlag) {
        swapinterval = animparam.swap_interval;
      }
    }
    break;

  case 14: /* New animate.  Read the animate parameters */
    animateFlag = STOP;
    swapinterval = 0;
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    ReadAnimData(clp.afilename, &animparam);
    my_SetBufferType((char)DoubleBuffer(&animparam));
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;
  case 141: // new rendering
    animateFlag = STOP;
    swapinterval = 0;
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    scale = viewparam.scale;   // save the scale before RereadDrawParams()
    min[0] = viewparam.min[0]; // also save the view volume
    min[1] = viewparam.min[1];
    min[2] = viewparam.min[2];
    max[0] = viewparam.max[0];
    max[1] = viewparam.max[1];
    max[2] = viewparam.max[2];
    RereadDrawParams();
    viewparam.scale =
        scale; // set scale back, so new rendering does not scale the model
    viewparam.min[0] = min[0]; // also set the view volume back
    viewparam.min[1] = min[1];
    viewparam.min[2] = min[2];
    viewparam.max[0] = max[0];
    viewparam.max[1] = max[1];
    viewparam.max[2] = max[2];

    SetGraphics();
    redrawf = rfClear;
    redrawf |= rfShadows;

    // MC - July 2016 - set flag to regenerate the shadow map if in 'shadows'
    // mode
    StartRedraw(redrawf); // in this case, we just want to clear and not scale
                          // or change view volume
    // continueredraw is not needed is all ready done later

    if (animateFlag) {
      swapinterval = animparam.swap_interval;
    }
    my_SetBufferType((char)DoubleBuffer(&animparam));
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;

#ifdef CPFG_ENVIRONMENT
  case 15: /* New environment.  Read environment file */
    if (clp.communication == COMM_LOCAL) {
      CTerminate();
      CMInitialize();
      CMAddProcess(clp.efilename);
    }
#if CPFG_VERSION >= 3200
    if (clp.communication == COMM_DISTRIBUTED) {
      /* send proper signal -> reread files or just step 0? */
    }
#endif
    break;
#endif

  case 20: /* select animate mode */
    SetAnimateMenu();
#ifndef VIEWER
    ReadAnimData(clp.afilename, &animparam);
    my_SetBufferType((char)DoubleBuffer(&animparam));
#endif
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    animateFlag = SINGLE;
    swapinterval = 0;
    if (validLsystem) {
      LsystemList->current =
          (animparam.last < 0) ? LsystemList->n : animparam.last;
    }
    break;

  case 21:                             /* single step */
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    animateFlag = SINGLE;
    swapinterval = 0;
    break;

  case 22:                             /* run */
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    animateFlag = RUN;
    swapinterval = (int)animparam.swap_interval;
    break;

  case 23:                             /* forever */
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    animateFlag = FOREVER;
    swapinterval = (int)animparam.swap_interval;
    break;

  case 24:                         /* stop */
    setcursor(CURSOR_ARROW, 0, 0); /* back to arrow cursor */
    animateFlag = STOP;
    swapinterval = 0;
    break;

  case 25:                             /* rewind */
    if (clp.communication == COMM_LOCAL) {
      CTerminate();
      CMInitialize();
      CMAddProcess(clp.efilename);
    }

    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    animateFlag = SINGLE;
    swapinterval = 0;
    if (validLsystem) {
      LsystemList->current =
          (animparam.last < 0) ? LsystemList->n : animparam.last;
    }
    break;

  case 26: /* clear */
    glDrawBuffer(GL_FRONT);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    if (double_buffering && clp.doublebuffer) {
      glDrawBuffer(GL_BACK);
    }

    if (expired)
      DrawExpired();
    glFlush();
    break;

  case 27:                             /* return to main menu */
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    swapinterval = 0;
    animateFlag = STOP;
    animatesave = -1;
    SetMainMenu();
    // [PASCAL] Why do we rerun the entire LSystem ?
    // answer: we want to see the model on the same way it was at the beginning
    if (validLsystem) {
      my_SetBufferType(TRUE);
      if (LsystemList->current != LsystemList->n) {
        if (LsystemList->current > LsystemList->n)
          Rewind(&currentString, &currentStringEnd, LsystemList);

        GenerateNSteps(LsystemList->n - LsystemList->current, &currentString,
                       &currentStringEnd, &nextString, &nextStringEnd,
                       LsystemList, &drawparam, &viewparam);

        init_blackbox(); /* initialize blackbox functions	*/
        StartRedraw(rfScale | rfViewVol | rfClear | rfShadows);
      }
    }
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;

  case SAVE_OFFSET + SAVE_RGB:
  case SAVE_OFFSET + SAVE_PNG:
  case SAVE_OFFSET + SAVE_BMP:
  case SAVE_OFFSET + SAVE_GIF:
  case SAVE_OFFSET + SAVE_JPG:
  case SAVE_OFFSET + SAVE_PBM:
  case SAVE_OFFSET + SAVE_TIFF:
  case SAVE_OFFSET + SAVE_RAS:
  case SAVE_OFFSET + SAVE_TGA:
  case SAVE_OFFSET + SAVE_RLE:
  case SAVE_OFFSET + SAVE_RAYSHADE:
  case SAVE_OFFSET + SAVE_POSTSCRIPT:
  case SAVE_OFFSET + SAVE_VIEWVOLUME:
  case SAVE_OFFSET + SAVE_STRING:
  case SAVE_OFFSET + SAVE_STRINGDUMP:
  case SAVE_OFFSET + SAVE_GLS:
    if (validLsystem) {
      char buf[256];
      char *name;

      sprintf(buf, clp.savefilename[pupval - SAVE_OFFSET],
              LsystemList->current);

      name = Strdup(buf);
      setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
      OpenOutputFile(name, pupval - SAVE_OFFSET);
      setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    }
    break;

  case NEW_FILE_OFFSET + SAVE_RGB:
  case NEW_FILE_OFFSET + SAVE_PNG:
  case NEW_FILE_OFFSET + SAVE_BMP:
  case NEW_FILE_OFFSET + SAVE_GIF:
  case NEW_FILE_OFFSET + SAVE_JPG:
  case NEW_FILE_OFFSET + SAVE_PBM:
  case NEW_FILE_OFFSET + SAVE_TIFF:
  case NEW_FILE_OFFSET + SAVE_RAS:
  case NEW_FILE_OFFSET + SAVE_TGA:
  case NEW_FILE_OFFSET + SAVE_RLE:
  case NEW_FILE_OFFSET + SAVE_RAYSHADE:
  case NEW_FILE_OFFSET + SAVE_POSTSCRIPT:
  case NEW_FILE_OFFSET + SAVE_VIEWVOLUME:
  case NEW_FILE_OFFSET + SAVE_STRING:
  case NEW_FILE_OFFSET + SAVE_STRINGDUMP:
  case NEW_FILE_OFFSET + SAVE_GLS: {
    Change_Filename(pupval - NEW_FILE_OFFSET);
  } break;
  case OUTPUT_ANIMATE_OFFSET + SAVE_RGB:
  case OUTPUT_ANIMATE_OFFSET + SAVE_PNG:
  case OUTPUT_ANIMATE_OFFSET + SAVE_BMP:
  case OUTPUT_ANIMATE_OFFSET + SAVE_GIF:
  case OUTPUT_ANIMATE_OFFSET + SAVE_JPG:
  case OUTPUT_ANIMATE_OFFSET + SAVE_PBM:
  case OUTPUT_ANIMATE_OFFSET + SAVE_TIFF:
  case OUTPUT_ANIMATE_OFFSET + SAVE_RAS:
  case OUTPUT_ANIMATE_OFFSET + SAVE_TGA:
  case OUTPUT_ANIMATE_OFFSET + SAVE_RLE:
  case OUTPUT_ANIMATE_OFFSET + SAVE_RAYSHADE:
  case OUTPUT_ANIMATE_OFFSET + SAVE_POSTSCRIPT:
  case OUTPUT_ANIMATE_OFFSET + SAVE_VIEWVOLUME:
  case OUTPUT_ANIMATE_OFFSET + SAVE_STRING:
  case OUTPUT_ANIMATE_OFFSET + SAVE_STRINGDUMP:
  case OUTPUT_ANIMATE_OFFSET + SAVE_GLS:
    animatesave = pupval - OUTPUT_ANIMATE_OFFSET;
    break;
  case 98: /* STOP recording */
    animatesave = -1;
    break;
  case 99:
    VERBOSE("BYE!\n");
    MyExit(0);
    /* break; not necessary */
  case 100:
    /* New colors.  Read the color parameters */
#ifdef WIN32
    animateFlag = STOP;
#endif
    swapinterval = 0;
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    RereadColors();
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;
  case 101:
    /* New Contours.  Read the contour parameters */
#ifdef WIN32
    animateFlag = STOP;
#endif
    swapinterval = 0;
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    RereadContours();
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;

  case 102:
    /* New surfaces.  Read the surface parameters */
#ifdef WIN32
    animateFlag = STOP;
#endif
    swapinterval = 0;
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    RereadSurfaces();
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;

  case 103:
    /* New functions.  Read the functions parameters */
#ifdef WIN32
    animateFlag = STOP;
#endif
    swapinterval = 0;
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    RereadFunctionsRerun();
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;

  case 104:
    /* New curve.  Read the curve parameters */
#ifdef WIN32
    animateFlag = STOP;
#endif
    swapinterval = 0;
    setcursor(CURSOR_HOURGLASS, 0, 0); /* change to hourglass cursor */
    RereadCurveXYZRerun();
    setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
    break;

  default:
    break;
  }
}

#ifdef WIN32
void BeginRecording() { animatesave = SAVE_BMP; }
#endif

int RereadDrawParams() {
  drawparam.openGL_2 = 1;
  // reread colors
  int readOk = 0;
  if (clp.iscolormapfile)
    readOk = load_in_colormaps();
  if (clp.ismaterialfile)
    readOk = load_in_materials();

  ReadAnimData(clp.afilename, &animparam);
  /* Read the view file */
  if (!ReadViewData(clp.vfilename, &drawparam, &viewparam))
    readOk = 0;
  return readOk;
}

int RereadAllDrawParams() {
  int rereadOk;
  // reread colors

  if (clp.iscolormapfile)
    rereadOk = load_in_colormaps();
  if (clp.ismaterialfile)
    rereadOk = load_in_materials();
  if (!rereadOk)
    return 0;

  // reread contours
  rereadOk = process_contours();
  if (!rereadOk)
    return 0;

  rereadOk = RereadCurveXYZ();
  if (!rereadOk)
    return 0;

  // reread functions

  rereadOk = RereadFunctions();
  if (!rereadOk)
    return 0;

  ReadAnimData(clp.afilename, &animparam);
  return 1;
}

void RereadColors() {
  if (clp.iscolormapfile)
    if (!load_in_colormaps()) {
      Message("Can't reload colormaps\n");
      clp.filesFullyLoaded = 0;
#ifndef WIN32
      return;
#endif
    }
  if (clp.ismaterialfile)
    if (!load_in_materials()) {
      Message("Can't reload material\n");
      clp.filesFullyLoaded = 0;
#ifndef WIN32
      return;
#endif
    }

  clp.filesFullyLoaded = 1;

  SetGraphics();

  StartRedraw(rfClear);

#ifdef WIN32
  ContinueRedraw();

  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

void RereadContours() {
  int rereadOk = process_contours();
  if (!rereadOk) {
    clp.filesFullyLoaded = 0;
#ifndef WIN32
    return;
#endif
  }
  clp.filesFullyLoaded = 1;
#ifndef WIN32
  if (animateFlag != STOP)
    return;
#endif

  StartRedraw(rfClear | rfShadows);

#ifdef WIN32
  ContinueRedraw();
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

void RereadFunctionsRerun() {
  int rereadsuccess = RereadFunctions();
  if (!rereadsuccess) {
    clp.filesFullyLoaded = 0;
#ifndef WIN32
    return;
#endif
  }
  clp.filesFullyLoaded = 1;

  if ((validLsystem) && (rereadsuccess)) {
#ifndef WIN32
    if (animateFlag != STOP)
      return;
#endif

    enabled_menus = 1;
    Rewind(&currentString, &currentStringEnd, LsystemList);
    GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                   &nextString, &nextStringEnd, LsystemList, &drawparam,
                   &viewparam);

    init_blackbox(); /* initialize blackbox functions */
    StartRedraw(rfClear | rfShadows);
#ifdef WIN32
    ContinueRedraw();
#endif
  }
#ifdef WIN32
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

void RereadFunctionsRerunNoSync() {
  int rereadOk = RereadFunctions();
  if (!rereadOk) {
    clp.filesFullyLoaded = 0;
#ifndef WIN32
    return;
#endif
  }
  clp.filesFullyLoaded = 1;

  if ((validLsystem) && (rereadOk)) {
#ifndef WIN32
    if (animateFlag != STOP)
      return;
#endif

    enabled_menus = 1;
    Rewind(&currentString, &currentStringEnd, LsystemList);
    GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                   &nextString, &nextStringEnd, LsystemList, &drawparam,
                   &viewparam);

    init_blackbox(); /* initialize blackbox functions */
    StartRedraw(rfClear | rfShadows);
#ifdef WIN32
    ContinueRedraw();
#endif
  }
}

void RereadCurveXYZRerun() {
  int rereadOk = RereadCurveXYZ();
  if (!rereadOk) {
    clp.filesFullyLoaded = 0;
#ifndef WIN32
    return;
#endif
  }
  clp.filesFullyLoaded = 1;

  if ((validLsystem) && (rereadOk)) {
#ifndef WIN32
    if (animateFlag != STOP)
      return;
#endif

    enabled_menus = 1;

    Rewind(&currentString, &currentStringEnd, LsystemList);
    GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                   &nextString, &nextStringEnd, LsystemList, &drawparam,
                   &viewparam);

    init_blackbox(); /* initialize blackbox functions */
    StartRedraw(rfClear | rfShadows);
#ifdef WIN32
    ContinueRedraw();
#endif
  }
#ifdef WIN32
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

extern int surface_number;
extern int rereadsurfacefile(int);

void RereadSurfaces() {
  int numofsurfaces = surface_number;
  int i;
  InitializeSurfaces();
  for (i = 1; i < numofsurfaces; i++) {
    if (rereadsurfacefile(i) == 0) {
#ifndef WIN32
      return;
#endif
    }
  }
  StartRedraw(rfClear | rfShadows);
#ifdef WIN32
  ContinueRedraw();
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

void RegenerateString(void) {
  if (validLsystem) {
    int s = LsystemList->current;
    enabled_menus = 1;

    Rewind(&currentString, &currentStringEnd, LsystemList);
    GenerateNSteps(s, &currentString, &currentStringEnd, &nextString,
                   &nextStringEnd, LsystemList, &drawparam, &viewparam);

    init_blackbox(); /* initialize blackbox functions */
  }
}

void Rerun(void) {
  if (validLsystem) {
    enabled_menus = 1;

    Rewind(&currentString, &currentStringEnd, LsystemList);
    GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                   &nextString, &nextStringEnd, LsystemList, &drawparam,
                   &viewparam);

    init_blackbox(); /* initialize blackbox functions */
    StartRedraw(rfClear | rfShadows);
#ifdef WIN32
    ContinueRedraw();
#endif
  }
#ifdef WIN32
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

void NewRender() {
  if (clp.iscolormapfile)
    load_in_colormaps();

  if (clp.ismaterialfile)
    load_in_materials();
  ReadViewData(clp.vfilename, &drawparam, NULL);

  /* create a desired font */
  makeRasterFont();

  /* set screen draw dispatcher routines */
  screenDrawRoutines = SetScreenDispatcher(&drawparam, &viewparam);

  init_blackbox(); /* initialize blackbox functions */

  StartRedraw(rfClear | rfShadows);
#ifdef WIN32
  ContinueRedraw();
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

void NewProjection() {
  if (clp.iscolormapfile)
    load_in_colormaps();

  if (clp.ismaterialfile)
    load_in_materials();

  ReadViewData(clp.vfilename, NULL, &viewparam);

  /* create a desired font */
  makeRasterFont();

  /* set screen draw dispatcher routines */
  screenDrawRoutines = SetScreenDispatcher(&drawparam, &viewparam);

  /* reset initial rotation */
  viewparam.xRotation = 0;
  viewparam.yRotation = 0;
  viewparam.zRotation = 0;
  viewparam.xPan = 0;
  viewparam.yPan = 0;

  init_blackbox(); /* initialize blackbox functions */

  StartRedraw(rfScale | rfViewVol | rfClear | rfShadows);
#ifdef WIN32
  ContinueRedraw();
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
}

void RestoreView() {
  viewparam.xRotation = 0;
  viewparam.yRotation = 0;
  viewparam.zRotation = 0;
  viewparam.xPan = 0;
  viewparam.yPan = 0;
  viewparam.scale = viewparam.initscale;

  StartRedraw(rfViewVol | rfClear);
#ifdef WIN32
  ContinueRedraw();
#endif
}

/************************************************************************/
/* Checks if the filename contains a % (is a format string). If it doesn't
   then it appends just before the extenson %d. Name must be reallocated!
*/
int CheckIfFormat(char **name, char add_number_to_name) {
  char *newname = NULL;
  char *last_dot = NULL;

  if (strchr(*name, '%') != NULL)
    return 1;

  if (!add_number_to_name)
    return 1;

  if ((newname = (char *)Malloc(strlen(*name) + 2)) == NULL) {
    Message("Cannot allocate a string for savename.\n"
            "Warning: savename is not a format string!");
    return 0;
  }

  strcpy(newname, *name);

  if ((last_dot = strrchr(*name, '.')) == NULL)
    /* append the %d to the end */
    strcat(newname, "%03d");
  else {
    newname[last_dot - *name] = '\0';
    strcat(newname, "%03d");
    strcat(newname, last_dot);
  }

  Free(*name);
  *name = NULL;
  *name = newname;

  return 1;
}

/************************************************************************/
/*
  "IdleFunction" performs drawing while the object is rotated or animated.
  It is defined as working procedure for X environment so it returns 1 if no
  other drawing is necessary and the working procedure can be uninstalled.
*/

#define FREEZEANIM

int IdleFunction(void) {
  int newframe = 0;
  unsigned int redrawf = 0;
#ifndef WIN32
  struct timeval timew; /* structure to get time */
  long start = 0, end;  /* for simulation of swapinterval */
#ifdef LINUX
  struct timezone timez;
#endif
#endif /* WIN32 */

#ifdef WIN32
  /* perform rotation and animation as required */
  switch (updateFlag) {
  case UPDATE_ROTATION:
    if (Update_Rotation())
      StartRedraw(rfClear);

    ContinueRedraw();
#ifndef FREEZEANIM
    if (!animateFlag)
#endif
      return 0;
    break;

  case UPDATE_SCALE:
    if (Update_Scale())
      StartRedraw(rfScale | rfClear);

    ContinueRedraw();
#ifndef FREEZEANIM
    if (!animateFlag)
#endif
      return 0;
    break;
  case UPDATE_PAN:
    if (Update_Pan())
      StartRedraw(rfClear);
    ContinueRedraw();
#ifndef FREEZEANIM
    if (!animateFlag)
#endif
      return 0;
    break;
  }
  if (NULL == LsystemList)
    return 1;
#endif

  if (animateFlag) {
    /* if at the end, restart */
    if (LsystemList->current >=
        ((animparam.last < 0) ? LsystemList->n : animparam.last)) {
      Rewind(&currentString, &currentStringEnd, LsystemList);
      GenerateNSteps(animparam.first, &currentString, &currentStringEnd,
                     &nextString, &nextStringEnd, LsystemList, &drawparam,
                     &viewparam);
    } else {
      /* derive next string (after animparam.steps steps) and make current */
      GenerateNSteps(GetAnimStep(&LsystemList->current, &animparam),
                     &currentString, &currentStringEnd, &nextString,
                     &nextStringEnd, LsystemList, &drawparam, &viewparam);
    }

    switch (animateFlag) {
    case SINGLE:
      animateFlag = STOP;
      break;
    case RUN:
      if (LsystemList->current >=
          ((animparam.last < 0) ? LsystemList->n : animparam.last)) {
        animateFlag = STOP;
      }
      break;
    case FOREVER:
      break;
    default:
      break;
    }
    newframe = 1;

    /* get the time before Redraw */
    if (swapinterval > 0) {
#ifndef WIN32
#ifdef LINUX
      gettimeofday(&timew, &timez);
#endif
      start = timew.tv_sec * 100 + timew.tv_usec / 10000;
#endif /* WIN32 */
    }


    // [PASCAL] Someone set the redrawf flag to clear why it so ?
    // should be set to 0
    // unsigned int redrawf = rfScale;
    redrawf = 0;

    if (ClearBetweenFrames(&animparam)) {
      redrawf |= rfClear;
    }
   if (NewViewBetweenFrames(&animparam)){
     redrawf = rfScale | rfViewVol | rfClear | rfCenter | rfHCenter;
     NewView();
   }

   else if (ScaleBetweenFrames(&animparam)) {
      redrawf |= rfScale | rfViewVol;
    }
   else if (CenterBetweenFrames(&animparam)) {
      redrawf |= rfCenter;
    }
   else if (HCenterBetweenFrames(&animparam)) {
      redrawf |= rfHCenter;
    }
    // MC - July 2016 - regenerate the shadow map for every animate step
    if (drawparam.render_mode == RM_SHADOWS) {
      redrawf |= rfShadows;
    }

    Redraw(redrawf);

    /* save if necessary */
    if (newframe && animatesave >= 0) {
      char buf[256];
      char *format;
      char *name;
#ifdef WIN32
      if (SAVE_BMP == animatesave)
        format = Strdup("frame%06d.bmp");
      else
#endif
        format = Strdup(clp.savefilename[animatesave]);
      if (CheckIfFormat(&format, 1)) {
        /* save only when name is a format string (contains %) */
        sprintf(buf, format, LsystemList->current);

        name = Strdup(buf);
        SaveFile(name, animatesave); /* doesn't ask whether to  overwrite */
        Free(name);
        name = NULL;
      }
      Free(format);
      format = NULL;
    }
    /* wait according to the swapinterval */
    if (swapinterval > 0) {
#ifdef WIN32
      Sleep(10 * swapinterval);
#else
      // usleep(swapinterval *10000);
      /*      
      do {
#ifdef LINUX
        gettimeofday(&timew, &timez);
#endif
        end = timew.tv_sec * 100 + timew.tv_usec / 10000;
      } while ((end - start) % (2400 * 3600) < swapinterval);
      */
#endif /* WIN32 */
    }
    if (animateFlag == STOP) {
      swapinterval = 0;
      setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
      return 1;
    }
    return 0;
  } else {

    if (ContinueRedraw()) {
      setcursor(CURSOR_ARROW, 0, 0); /* change to arrow cursor */
      return 1;
    } else
      return 0;
  }
}

#ifdef RESEARCH_VER
/******************************************************************/
/* Sends current color buffer with depth values to a master - using
   communication routines from library 'comm'
*/
int SendColorAndDepthBuffer(void) {
  char *cbuff = NULL;
  float *dbuff = NULL;
  int y;
  char line[256];

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-0.5, clp.xsize + 0.5, -0.5, clp.ysize + 0.5, 0, 1);

  glReadBuffer(GL_FRONT);

  if ((cbuff = (char *)Malloc(clp.xsize * 4)) == NULL) {
    Message("Cannot allocate memory for color buffer to sent.\n");
    return 0;
  }

  if ((dbuff = (float *)Malloc(clp.xsize * sizeof(float))) == NULL) {
    Message("Cannot allocate memory for depth buffer to sent.\n");
    Free(cbuff);
    cbuff = NULL;
    return 0;
  }

  sprintf(line, "image %d %d\n", clp.xsize, clp.ysize);

  CSSendString(0, line);

  for (y = 0; y < clp.ysize; y++) {
    /* get the rgba values for one row */
    glReadPixels(0, y, clp.xsize, 1, GL_RGBA, GL_UNSIGNED_BYTE, cbuff);

    /* send away */
    if (CSSendBinaryData(0, cbuff, 4, clp.xsize))
      break;

    /* get the depth buffer values for one row */
    glReadPixels(0, y, clp.xsize, 1, GL_DEPTH_COMPONENT, GL_FLOAT, dbuff);

    /* send away */
    if (CSSendBinaryData(0, (char *)dbuff, sizeof(float), clp.xsize))
      break;
  }

  Free(cbuff);
  cbuff = NULL;
  Free(dbuff);
  dbuff = NULL;

  /* restores the previous projection */
  glPopMatrix();
  /* modelview matrix is kept as default */
  glMatrixMode(GL_MODELVIEW);

  return 1;
}
#endif

/******************************************************************/

static int work_mode;

/******************************************************************/
int QueryWorkMode(void) { return work_mode; }

/******************************************************************/
void DoMode(int mode) {
  int draw = 0;
  int output = 1;
  int master;
  char str[2048];
  int step;
  char sendBuffer = 0;
  char in;
  int store_step = 0;
  FILE *fp = NULL;
  char add_number_to_name = 0; /* add frame number to the file name even
  if the fomat doesn't contain %d. */

  work_mode = mode;

  /* off-screen computations */
  if (clp.iscolormapfile)
    load_in_colormaps();

  if (clp.ismaterialfile)
    load_in_materials();

  if (mode == SLAVE_MODE || (clp.savefilename[SAVE_RGB] != NULL) ||
      (clp.savefilename[SAVE_PNG] != NULL) ||
      (clp.savefilename[SAVE_BMP] != NULL) ||
      (clp.savefilename[SAVE_GIF] != NULL) ||
      (clp.savefilename[SAVE_JPG] != NULL) ||
      (clp.savefilename[SAVE_PBM] != NULL) ||
      (clp.savefilename[SAVE_TIFF] != NULL) ||
      (clp.savefilename[SAVE_RAS] != NULL) ||
      (clp.savefilename[SAVE_RLE] != NULL) ||
      (clp.savefilename[SAVE_TGA] != NULL)){
    draw = 1;
  }
  if (mode == SLAVE_MODE) {
    sendBuffer = !clp.graphics_output;
    clp.graphics_output = 0;
  }

  /* Read the view file */
  ReadViewData(clp.vfilename, &drawparam, &viewparam);

  if (!animateFlag) {
    animparam.first = LsystemList->n;
    animparam.last = LsystemList->n;
  } else {
    /* Read the animate file */
#ifndef VIEWER
    ReadAnimData(clp.afilename, &animparam);
    my_SetBufferType((char)DoubleBuffer(&animparam));
#endif
    add_number_to_name = 1;
  }

  if (draw || clp.savefilename[SAVE_POSTSCRIPT] != NULL ||
      clp.savefilename[SAVE_GLS] != NULL) {
    /* off-screen drawing or just set up the view to be able to get
                the projection matrix and to be able to use opengl matrix
       opertions */
    if (clp.warnings)
      Message("Off-screen rendering. KEEP X-SERVER RUNNING!\n");

#ifndef LINUX
    InitializeGraphics();
#endif

    /* create a desired font */
    makeRasterFont();
  }

#ifdef CPFG_ENVIRONMENT
  if (clp.communication == COMM_LOCAL) {
    Message("Do mode => communication\n");
    CMInitialize();
    CMAddProcess(clp.efilename);
  }
#endif

  if (mode == PIPE_MODE) {
    InputString("stdin", 'b');

    if (LsystemList->current < animparam.first)
      GenerateNSteps(animparam.first - LsystemList->current, &currentString,
                     &currentStringEnd, &nextString, &nextStringEnd,
                     LsystemList, &drawparam, &viewparam);
  } else {

    Rewind(&currentString, &currentStringEnd, LsystemList);

    if (mode == NO_GRAPHICS_MODE) {
      GenerateNSteps(animparam.first, &currentString, &currentStringEnd,
                     &nextString, &nextStringEnd, LsystemList, &drawparam,
                     &viewparam);
    }
  }
  if (draw) {
    InitializeDispatcher();
    CalculateViewVolume(currentString, &drawparam, &viewparam);
    SetView(&viewparam);
    init_blackbox();
  }

#ifdef RESEARCH_VER
  if (clp.server_socket > 0) {
    if (clp.warnings)
      Message("Setting up the server socket.\n");

    if (!SetServer(clp.server_socket))
      Message("Cannot get server socket. "
              "Normal no graphics mode preformed.\n");
    else
      mode = SERVER_MODE;
  }
#endif

  /* infinite loop - until command 'exit' comes */
  for (;;) {

    in = 1;
    step = 0;

#ifdef RESEARCH_VER
    if (mode == SLAVE_MODE || mode == SERVER_MODE) {
      switch (mode) {
      case SLAVE_MODE:
        CSBeginTransmission();

        in = 0;
        output = 0;
        break;

      case SERVER_MODE:
        in = 0;
        output = 0;

        do {
          while (!GetNewClient(1))
            Message("Problem: client not connected. Trying again.\n");
          if ((fp = GetClientsData()) == NULL)
            Message("Problem: cannot get client's data. "
                    "Trying another client.\n");
        } while (fp == NULL);
        break;
      }

      do {
        switch (mode) {
        case SLAVE_MODE:
          in = CSGetString(&master, str, sizeof(str));

          if (in)
            output = 1;
          else {
            break;
          }

          if (master != 0)
            Message("Warning! Master index 0 assumed (is %d)!\n", master);

          if (clp.warnings)
            Message("From master: %s\n", str);
          break;

        case SERVER_MODE:
          in = fgets(str, sizeof(str), fp) != NULL;
          break;
        }

        if (!in)
          break;

        if (!strncmp(str, "new L-system", 8)) {

          validLsystem = ReadLsystem(clp.lfilename);
          if (validLsystem) {
            enabled_menus = 1;

            Rewind(&currentString, &currentStringEnd, LsystemList);
            GenerateNSteps(LsystemList->n, &currentString, &currentStringEnd,
                           &nextString, &nextStringEnd, LsystemList, &drawparam,
                           &viewparam);
            step = 0;
          } else
            strcat(str, "exit");
        }

        if (!strncmp(str, "new homomorphism", 14)) {
          if (LsystemList != NULL)
            store_step = LsystemList->current;

          validLsystem = ReadLsystem(clp.lfilename);
          if (validLsystem) {
            enabled_menus = 1;

#ifdef JIM
            InitialiseForStart(LsystemList);
#else
            /* Evaluate Array Initializers */
            EvaluateArrayInitializers(LsystemList);

            /* Make sure global file definitions have been cleared */
            FreeFileDefSpace();

            /* Evaluate global start statements */
            EvaluateStartStatements(LsystemList);
#endif
            LsystemList->current = store_step;
          }
        }

        if (!strncmp(str, "new view", 8)) {
          /* Read the view file */
          ReadViewData(clp.vfilename, &drawparam, &viewparam);
          CalculateViewVolume(currentString, &drawparam, &viewparam);
          SetView(&viewparam);
        }

        if (!strncmp(str, "new animate", 11)) {
          if (!animateFlag) {
            animparam.first = LsystemList->n;
            animparam.last = LsystemList->n;
          } else {
            /* Read the animate file */
            ReadAnimData(clp.afilename, &animparam);
            my_SetBufferType((char)DoubleBuffer(&animparam));
          }
        }

        if (!strncmp(str, "generate", 8)) {
          step = 0;
          sscanf(str, "generate %d", &step);

          if (clp.warnings)
            Message("Generating till step %d.\n", step);
          if (step < LsystemList->current)
            Rewind(&currentString, &currentStringEnd, LsystemList);

          step = step - LsystemList->current;
        }

        if (!strncmp(str, "step", 4))
          step = GetAnimStep(&LsystemList->current, &animparam);

        if (!strncmp(str, "output", 6))
          output = 1;
      } while (in == 1);
    }
#endif

    switch (mode) {
#ifdef RESEARCH_VER
    case SERVER_MODE:
      if (!strncmp(str, "exit", 4))
        return;
      break;
#endif

    case NO_GRAPHICS_MODE:
      if (NewViewBetweenFrames(&animparam))
        NewView();
      else
        init_blackbox();

      // MC - Dec. 2015 - is the following 'if' necessary?
      // sometimes we want to know the view volume even if "scale between
      // frames" is off if(ScaleBetweenFrames(&animparam))
      {
        CalculateViewVolume(currentString, &drawparam, &viewparam);
// MC - Dec. 2015 - SetView cannot be called if there is no OpenGL context
// there seems to be one on Windows but not on Mac...
#ifdef WIN32
        SetView(&viewparam);
#endif
      }
      break;
    }
    if (step > 0)
      /* Generate the string to the desired depth */
      GenerateNSteps(step, &currentString, &currentStringEnd, &nextString,
                     &nextStringEnd, LsystemList, &drawparam, &viewparam);

    if (output) {
#ifdef RESEARCH_VER      
      if (mode == SLAVE_MODE) {
        if (sendBuffer) {

          glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

          /* rendering specification */
          drawparam.tdd = screenDrawRoutines;
          drawparam.output_type = TYPE_OPENGL;

          TurtleDraw(currentString, &drawparam, &viewparam);

          if (clp.warnings)
            Message("Sending window buffers.\n");

          SendColorAndDepthBuffer();

          if (clp.warnings)
            Message("Buffers sent.\n");
        } else {
          /* sending gls */
          if (clp.warnings)
            Message("Sending the structure in gls format.\n");

          drawparam.output_type = TYPE_SLAVE;
          drawparam.tdd = glsSetDispatcher(&drawparam, &viewparam);
          TurtleDraw(currentString, &drawparam, &viewparam);
        }

      } else
#endif
      {

        if (draw) {
          glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

          /* rendering specification */
          drawparam.tdd = screenDrawRoutines;
          drawparam.output_type = TYPE_OPENGL;

          TurtleDraw(currentString, &drawparam, &viewparam);
        }
      }

      OutputString(LsystemList->current, add_number_to_name);
      
      if (mode == NO_GRAPHICS_MODE || mode == PIPE_MODE) {
        if (LsystemList->current >= animparam.last)
          break;
        GenerateNSteps(GetAnimStep(&LsystemList->current, &animparam),
                       &currentString, &currentStringEnd, &nextString,
                       &nextStringEnd, LsystemList, &drawparam, &viewparam);
      }
    }

#ifdef RESEARCH_VER
    if (mode == SLAVE_MODE)
      /* End transmission returns 1 when the process is requested to exit */
      if (CSEndTransmission())
        return;
#endif
  }
}

/******************************************************************/
void OutputString(int frame_num, char add_number_to_name) {
  int i;

  for (i = SAVE_COUNT - 1; i >= SAVE_RGB; i--)
    if (clp.savefilename[i] != NULL) {
      char buf[256];
      char *name, *format;

      format = Strdup(clp.savefilename[i]);

      if (CheckIfFormat(&format, add_number_to_name)) {
        /* save only when name is a format string (contains %) */
        sprintf(buf, format, frame_num);

        name = Strdup(buf);

        //Message("Saving %s file %s.\n", type_strings[i], name);

        SaveFile(name, i);

        Free(name);
        name = NULL;
      }
      Free(format);
      format = NULL;
    }
}

/******************************************************************/
/*
        "main" organizes computation.
*/
int main(int argc, char **argv) {
  /* malloc debugging - but better use malloc_cv library */
  /* mallopt(M_DEBUG, 1); */

  /* Parse the command line.  */
  relativeSize = 0;
  relativePos = 0;
  ParseCommand(argc, argv);

  /* Allocate memory for the strings and initialize them to '\0's. */

  StringInit(&currentString, &currentStringEnd);
  StringInit(&nextString, &nextStringEnd);
  InitializeStacks(); /* for TurtleDraw in generate.c */

  /* Input the L-system */
  LsystemList = NULL;
  currentSymbolTable = NULL;
  globalSymbolTable = NULL;

  if (!clp.graphics_output
#if CPFG_VERSION >= 3200
      || (CSGetNumberOfMasters() > 0)
#endif
  ) {
    validLsystem = ReadLsystem(clp.lfilename);

    if (validLsystem != 0) {
#if CPFG_VERSION >= 3200
      if (CSGetNumberOfMasters() > 0)
        DoMode(SLAVE_MODE);
      else
#endif
	if (pipe_mode)
	  DoMode(PIPE_MODE);
	else{
	    if (clp.savefilename[SAVE_POSTSCRIPT] != NULL ||
		clp.savefilename[SAVE_GLS] != NULL) {
	      InitializeGraphics(
#ifdef LINUX
				 argc, argv
#endif
				 );
	    }
	    else{	      
	      DoMode(NO_GRAPHICS_MODE);
	    }
	}
      MyExit(0);
    }
  } else {
    InitializeGraphics(
#ifdef LINUX
        argc, argv
#endif
    );
  }
  return 0;
} /* end of main() */

/************************************************************************/
/*
        Parse the command line.
*/
static char defaultPreprocessor[8] =
#if defined LINUX
    "vlabcpp"
#elif defined WIN32
    "vlabcpp"
#else
#error Default preprocessor not defined for the platform
#endif
    ;

static void ParseCommand(int argc, char **argv) {
  int temp, i, filenumber = 0;
  int cxscr, cyscr;

  /* Set defaults for the command line parameters */
  clp.savingMode = OFF;
  clp.programname = NULL;
  clp.lfilename = NULL;
  clp.vfilename = NULL;
  clp.afilename = NULL;
#ifdef CPFG_ENVIRONMENT
  clp.efilename = NULL;
  clp.communication = COMM_NONE;
#endif
#if CPFG_VERSION >= 3200
  clp.server_socket = -1;
#endif
  clp.titlename = NULL;
  clp.preprocessor = defaultPreprocessor;
  clp.initTexture = 0;
  clp.initSurface = 0;
  for (i = 0; i < NUMTEXTURES; i++) {
    // clp.textureFiles[i] = "";
  }
  for (i = 0; i < SAVE_COUNT; i++) {
    clp.savefilename[i] = NULL;
    clp.savefp[i] = NULL;
  }
#ifdef WIN32
  strcpy(clp.savedir, ".\\");
#endif
  clp.maxstringsize = MAXSTR;
  clp.colormap = 256 * COLORMAP;
  clp.verbose = FALSE;
  clp.warnings = FALSE;
  clp.debug = FALSE;
  clp.xsize = clp.ysize = 0;
  clp.checkEnvironment = 0;

  clp.iscolormapfile = 0;
  for (temp = 0; temp < MAXCOLORMAPS; temp++)
    clp.colormapname[temp] = NULL; /* name of the colormap file */
  clp.ismaterialfile = 0;
  for (temp = 0; temp < MAXCOLORMAPS; temp++)
    clp.materialname[temp] = NULL; /* name of the material file */

  clp.doublebuffer = 1;
  clp.pixmap = 0;
  clp.overlay_menus = 1;
#ifdef WIN32
  clp.xsize = -1;
  clp.ysize = -1;
  /*
  If not overwritten by the command line switch the last stored
  will be used
  */
#else
  clp.xsize = 320;
  clp.ysize = 240;
#endif
  clp.xpos = -1; /* do not set the position */
  clp.ypos = -1;
#ifndef WIN32
  clp.xpos = 300; /* do not set the position */
  clp.ypos = 300;
#endif
#ifdef NOGRAPHICS
  clp.graphics_output = 0;
  Message("Forced no_graphic mode\n");
#else
  clp.graphics_output = 1;
#endif
  clp.menu_bar = 0;
  pipe_mode = 0;
  clp.stringIO_homo = 0;
  clp.noborder = 0;
#ifdef WIN32
  clp.nomsglog = 0;
  clp.compensateforframe = 0;
#endif

  clp.programname = argv[0];

#ifdef WIN32
  cxscr = GetSystemMetrics(SM_CXSCREEN);
  cyscr = GetSystemMetrics(SM_CYSCREEN);
#else

  cxscr = 1;
  cyscr = 1;
#endif
  while (--argc > 0) {
    if ((*++argv)[0] == '-') {
      /* check for file formats */
      for (i = 0; i < SAVE_COUNT; i++){
	
        if (!strcmp(argv[0] + 1, extension_strings[i] + 1)) {
          clp.savefilename[i] = Strdup(*++argv);
          VERBOSE("%s output file: %s\n", type_strings[i], *argv);
          // MC - August, 2020 - code above simply saves the type of output file from command line.
          // I could not find where it uses this information to save the file in animate mode.
          // My feeling is that cpfg never did output the file in animate mode (unless it was specified in "recording" mode).
#ifdef LINUX        // I added the "ifdef" so nothing breaks on Windows
                    animatesave = i; // affects "save if necessary" code in IdleFunction
#endif
          --argc;
          break;
        }
      }

      if (i < SAVE_COUNT)
        continue;

      switch (*(argv[0] + 1)) {
      case 'a':
        start_with_animate = 1;
        animateFlag = 1;
        VERBOSE("animate mode on: %s\n", *argv);
        break;
      case 'b':
        clp.batch_save = 1;
	clp.savefilename[0] = *++argv;
	Message("Saving name: %s\n",clp.savefilename);
        VERBOSE("Saving in batch mode on\n");
        break;

      case 'c':
        sscanf(argv[0] + 2, "%d", &temp);
        clp.colormap = 256 * temp;
        VERBOSE("color map: %d\n", clp.colormap);
        break;
#if CPFG_VERSION >= 3200
      case 'C': /* distributed communication */
        if (clp.communication == COMM_NONE) {
          /* cannot have together distributed and local communication */
          CInitialize(clp.programname, *++argv);
          clp.communication = COMM_DISTRIBUTED;
        }
        --argc;
        break;

      case 'S': /* server mode */
        clp.server_socket = atoi(*++argv);
        argc--;
        break;

#endif
      case 'd':
        clp.debug = TRUE;
        Message("debug mode\n");
        break;
#ifdef CPFG_ENVIRONMENT
      case 'e':
        if (clp.communication == COMM_NONE) {
          clp.communication = COMM_LOCAL;
          clp.efilename = *++argv;
          VERBOSE("environment file: %s\n", *argv);
        } else
          ++argv;

        --argc;
        break;
#endif
      case 'g':
        clp.graphics_output = 0;
        VERBOSE("no graphics output\n");
        break;

      case 'h':
        clp.stringIO_homo = 1;
        VERBOSE("applying homomorphism for string output\n");
        break;
      case 'm':
        if (!strcmp(argv[0], "-mb")) { // not use in MacOsX/Linux versions
          clp.menu_bar = 1;
          start_with_animate = 1;
        } else {
          if (isdigit(*(argv[0] + 2)))
            sscanf(argv[0] + 2, "%d", &temp);
          else
            temp = 1;

          if ((temp < 0) || (temp >= MAXCOLORMAPS)) {
            Message("Colormap %d out of range, ignored!\n", temp);
            --argc;
            ++argv;
            break;
          }

          clp.iscolormapfile = 1;
          clp.ismaterialfile = 0;
          //[PASCAL] I don't see the need of "r" in the following statement:
          //clp.colormapname[temp] = *++argv, "r";
          // "r" has then been removed
          clp.colormapname[temp] = *++argv;
          VERBOSE("colormap %d file: %s\n", temp, *argv);
          --argc;
        }
        break;
      case 'M':
        if (isdigit(*(argv[0] + 2)))
          sscanf(argv[0] + 2, "%d", &temp);
        else
          temp = 1;

        if ((temp < 0) || (temp >= MAXCOLORMAPS)) {
          Message("Material set %d out of range, ignored!\n", temp);
          --argc;
          ++argv;
          break;
        }

        clp.ismaterialfile = 1;
        clp.iscolormapfile = 0; /* later wins */
        clp.materialname[temp] = *++argv;
        VERBOSE("material set %d file: %s\n", temp, *argv);
        --argc;
        break;
	// note overlay menus for SGI have been removed
      case 'p':
        if (!strcmp(argv[0], "-pipestrb")) {
          pipe_mode = 1;
          clp.graphics_output = 0;
          VERBOSE("Pipe mode on.\n");
        }
        break;
      case 'P':
        clp.preprocessor = *++argv;
        VERBOSE("Preprocessor: %s\n", *argv);
        --argc;
        break;
      case 's':
          sscanf(argv[0] + 2, "%u", &(clp.maxstringsize));
          VERBOSE("max string size: %u\n", clp.maxstringsize);
        break;
      case 'r':
        if (strcmp(argv[0], "-rmode") == 0) {
          const char *opt = *(++argv);
          --argc;
          if ((strcmp(opt, "expl") == 0) || (strcmp(opt, "explicit") == 0))
            clp.savingMode = OFF;
          if ((strcmp(opt, "cont") == 0) || (strcmp(opt, "continuous") == 0))
            clp.savingMode = CONTINUOUS;
          if ((strcmp(opt, "trig") == 0) || (strcmp(opt, "triggered") == 0))
            clp.savingMode = TRIGGERED;
        }
        break;

      case 'v':
        clp.warnings = TRUE;
        VERBOSE("warnings on\n");
        break;
      case 'V':
        clp.verbose = TRUE;
        clp.warnings = TRUE;
        Message("verbose mode\n");
        break;
      case 'w':
        if (!strcmp(argv[0], "-wt")) {
          /* window title */
          clp.titlename = *(++argv);
          VERBOSE("window title: %s\n", clp.titlename);
          --argc;
        }
	else if (!strcmp(argv[0], "-wnb")) {
          clp.noborder = 1;
	}

#ifdef WIN32
        else if (!strcmp(argv[0], "-wnb")) {
          clp.noborder = 1;
          clp.nomsglog = 1;
        } else if (!strcmp(argv[0], "-wnm")) {
          clp.nomsglog = 1;
        }
#endif
        else if (!strcmp(argv[0], "-wr")) {
          float xr, yr;
          xr = atof(*++argv);
          yr = atof(*++argv);
          --argc;
          --argc;
          clp.xsize = xr * cxscr;
          clp.ysize = yr * cyscr;
#ifdef LINUX // we set up the position in platformQt
          clp.xsize = xr ;
          clp.ysize = yr ;
          relativeSize = 1;
#endif
        } else if (!strcmp(argv[0], "-wpr")) {
          float xr, yr;
          xr = atof(*++argv);
          yr = atof(*++argv);
          --argc;
          --argc;
          clp.xpos = xr * cxscr;
          clp.ypos = yr * cyscr;
#ifdef LINUX // we set up the position in platformQt
          clp.xpos = xr ;
          clp.ypos = yr ;
          relativePos = 1;
#endif
        } else if (!strcmp(argv[0], "-wp")) {
          /* window position */
          clp.xpos = atoi(*++argv);
          clp.ypos = atoi(*++argv);
          VERBOSE("window position: %d %d\n", clp.xpos, clp.ypos);
          --argc;
          --argc;
#ifdef WIN32
          clp.compensateforframe = 1;
#endif
        } else {
          clp.xsize = atoi(*++argv);
          clp.ysize = atoi(*++argv);
          if (clp.xsize > 2048) {
            Message("Window xsize cannot exceed 2048 pixels\n");
            clp.xsize = 2048;
          }
          if (clp.ysize > 2048) {
            Message("Window ysize cannot exceed 2048 pixels\n");
            clp.ysize = 2048;
          }

          VERBOSE("window size: %d %d\n", clp.xsize, clp.ysize);
          --argc;
          --argc;
#ifdef WIN32
          clp.compensateforframe = 1;
#endif
        }
        break;
      default:
        Usage();
        MyExit(1);
      }
    } else {
      switch (++filenumber) {
      case 1:
        clp.lfilename = *argv;
        if (access(clp.lfilename, 4) < 0) {
          Message("Cannot access %s.\n", clp.lfilename);
          MyExit(1);
        }
        break;
      case 2:
        clp.vfilename = *argv;
        if (access(clp.vfilename, 4) < 0) {
          Message("Cannot access %s.\n", clp.vfilename);
          MyExit(1);
        }
        break;
      case 3:
        clp.afilename = *argv;
        if (access(clp.afilename, 4) < 0) {
          Message("Cannot access %s.\n", clp.afilename);
          MyExit(1);
        }
        break;
      default:
        Usage();
        MyExit(1);
      } /* switch */
    }   /* else */
  }     /* while */

  if (clp.titlename == NULL)
    clp.titlename = clp.lfilename;

  if (filenumber < 2) {
    Usage();
    MyExit(1);
  }
  if (!clp.graphics_output) {
    if ((!clp.iscolormapfile) && (!clp.ismaterialfile)) {
      Message("Error: for no_graphics mode a colormap file (-m "
              "file)\nor material file (-M) is necessary!\n");
      MyExit(1);
    }

    return;
  }

  if (clp.graphics_output
#if CPFG_VERSION >= 3200
      && (CSGetNumberOfMasters() == 0)
#endif
  )
    for (i = 0; i < SAVE_COUNT; i++)
      if (clp.savefilename[i] == NULL)
        clp.savefilename[i] =
            Strdup(makeSaveName(clp.lfilename, extension_strings[i]));
}

/************************************************************************/
/* makeSaveName is best illustrated by example:
   makeSaveName("foobar.l",".ps") returns "foobar.ps"
   makeSaveName("foobar",".ray") returns "foobar.ray"
   etc...

   note: makeSaveName returns a pointer to a static array,
         so if you say:
         a = makeSaveName(foo,bar);
         b = makeSaveName(foo2,bar);
         a and b will point to the same place, and the
         old results will be gone
*/

static char *makeSaveName(char *base, char *suffix) {
  static char newname[255];
  char *sufloc;

  strcpy(newname, base);
  sufloc = strrchr(newname, '.');
  if (!sufloc)
    sufloc = strrchr(newname, 0);
  strcpy(sufloc, suffix);
  return newname;
}

/************************************************************************/

static void Usage(void) {
  Message("Usage (version %g):\n%s ", (float)CPFG_VERSION / 1000.0,
          clp.programname);
  Message("[-s'stringsize'] [-v] [-V] [-d] [-P preprocessor] [-a] "
#ifdef CPFG_ENVIRONMENT
          "[-e environmentfile] "
#endif
#if CPFG_VERSION >= 3200
          "[-g] [-pipestrb] [-C "
          "communication_setup_string"
          "] "
          "[-S socket_num] "
#endif
          "[-c mapnr] [-w xsize ysize] [-wp xpos ypos] [-wt window_title] "
          "[-m[n] colormapfile] "
          "[-M[n] materialfile] [-mb] [-homo] "
          "[-ray rayfile] [-ps psfile] "
          "[-str stringfile] [-strb stringfile(binary)] [-gls glsfile] [-vv "
          "vvfile] "
          "file.l file.v [file.a]\n");
  Message(
      "\nGeneral:\n"
      " -s'stringsize' : specifies the inital length of the string.\n"
      "                  Note, there is no space between s and the number.\n"
      " -v : warning mode on.\n"
      " -V : verbose mode on.\n"
      " -d : debug mode on.\n"
      " -P preprocessor : changes default C preprocessor.\n"
      " -a : animate mode on (start with animate menu).\n"
#ifdef CPFG_ENVIRONMENT
      " -e environmentfile: specifies parameters of plant-field "
      "communication.\n"
#endif
#if CPFG_VERSION >= 3200
      "\nSpecial working modes:\n"
      " -g : no_graphics mode on. Preforms off-screen rendering. A colormap "
      "or\n"
      "      material file must be specified).\n"
      "      If -a is present, it uses animate file to create an animation.\n"
      " -pipestrb: similar to -g, but a binary string file is expected on the\n"
      "      standard input and no further generation is performed.\n"
      " -C "
      "communication_setup_string"
      ": specifies connections to masters\n"
      "      (-m comm_spec_file socket_num) servants (-s comm_spec_file"
      "socket_num\n"
      "      host) and where to confirm execution (-c socket_num host).\n"
      "      Used in distributed environment.\n"
      " -S socket_num: the program is able to process text commands coming "
      "through\n"
      "      the specified socket (using program command_client).\n"
#endif
      "\nGraphics and windowing:\n"
      " -c mapnr : specifies the reference colormap number (for colors "
      "0-255).\n"
      " -w xsize ysize: specifies the initial size of the window.\n"
      " -wp xpos ypos: specifies the initial position of the window (top "
      "left).\n"
      " -wt window_title: sets the title of the window (not of the icon).\n"
      " -m[n] mapfile : specifies colormap file for colormap n (just -m is "
      "-m1).\n"
      "                 Switches on RGBA mode.\n"
      " -M[n] matfile : specifies material file for material set n (just -M is "
      "-M1).\n"
      "                 Switches on lighting mode.\n"
      " -mb: menu bar on.\n"
      "\nOutput:\n"
      " -rgb rgbfile : specifies filename for image output (rgb).\n"
      " -ras rasfile : specifies filename for image output (ras).\n"
      " -tga tgafile : specifies filename for image output (targa).\n"
      " -ray rayfile : specifies filename for rayshade outpu.t\n"
      " -ps  psfile  : specifies filename for postscript output.\n"
      " -str stringfile: specifies filename for output of the string (text).\n"
      " -strb stringfile : specifies filename for output of the string "
      "(binary).\n"
      " -homo : strings are output after applying homomorphism. \n"
      " -vv vvfile      : specifies filename for output of the view volume.\n"
      " -gls datfile   : specifies filename for output in the gls format.\n"
      "       A filename may be specified as a format (e.g. plant%%03d.rgb)\n"
      "       and the number of the generation step is automatically "
      "inserted.\n"
      "       This can be used for saving animations.\n");
}

/************************************************************************/
/*
        Allocate memory for and initialize a string
*/

static void StringInit(char **stringPtr, char **stringEnd) {
  assert(NULL == *stringPtr);
  if ((*stringPtr = (char *)Malloc(clp.maxstringsize)) == NULL) {
    Message("%s: can't allocate a string\n", clp.programname);
    MyExit(1);
  }

  *stringEnd = *stringPtr + clp.maxstringsize - 1;
  *(*stringPtr) = '\0';

  /* Advance the pointer so that the '\0' character is always preserved
  at the beginning of the string for proper handling of the left
  context search */
  (*stringPtr)++;
  *(*stringPtr) = '\0';
}

/************************************************************************/
void FreeStrings(void) {
  if (NULL != currentString) {
    currentString--;
    Free(currentString);
    currentString = NULL;
  }

  if (NULL != nextString) {
    nextString--;
    Free(nextString);
    nextString = NULL;
  }
}

/********************************************************************/
/* rewinds current L-system to the beginning (axiom is in *current) */

static void Rewind(char **current, char **currentEnd,
                   LSYSDATA *LsystemListPtr) {
  int newLength;

  /* make sure there is room available for the axiom */
  if (*current + LsystemListPtr->axiomLength + 1 > *currentEnd) {
    newLength = LsystemListPtr->axiomLength * 2;
    Message("Axiom is too long; reallocating.\n");
    /* reallocate should start one character to the left */
    if ((*current = (char *)Realloc(*current - 1, newLength)) == NULL) {
      Message("Error: can't reallocate L-system string.\n");
      MyExit(1);
    }
    *currentEnd = *current + newLength - 1;
    (*current)++;
  }

  currentStringLength = LsystemListPtr->axiomLength;

#ifdef JIM
  InitialiseForStart(LsystemListPtr);
#else
  /* Evaluate Array Initializers */
  EvaluateArrayInitializers(LsystemListPtr);

  /* Make sure global file definitions have been cleared */
  FreeFileDefSpace();

  /* Evaluate global start statements */
  EvaluateStartStatements(LsystemListPtr);
#endif

  /* Evaluate the axiom expressions and put the result in string1. */
  CreateParameterString(current, LsystemListPtr->axiom, LsystemListPtr);
  if (clp.verbose) {
    Message("Converted axiom:\n");
    PrintParameterString(stderr, *current, SAVE_STRING, 0);
  }

  /* set the derivation step counter */
  LsystemListPtr->current = 0;
}

#ifdef JIM
/****************************************************************************/
/* Initialise for start of generation                                   */
/****************************************************************************/
void InitialiseForStart(LSYSDATA *lsysPtr) {
  /*SymbolInstance *instancePtr;*/

  /* set initial instance for main L-system */
  lsysPtr->currentInstance = AppendInstance(lsysPtr);

  /* point symbol table variables at appropriate instance */
  AssignSymbolsToInstances(lsysPtr->symbolTable, lsysPtr->currentInstance);

  /* Evaluate main L-system Array Initializers */
  /* EvaluateArrayInitializers(lsysPtr); */
  ProcessStatements(lsysPtr->defineBlock);

  /* Make sure global file definitions have been cleared */
  FreeFileDefSpace();

  /* Evaluate main L-system start statements */
  /* EvaluateStartStatements(lsysPtr); */
  ProcessStatements(lsysPtr->startBlock);
}
#endif

/************************************************************************/
/* generates N steps from the current string */

static void GenerateNSteps(int derivations, char **current, char **currentEnd,
                           char **next, char **nextEnd,
                           LSYSDATA *LsystemListPtr, DRAWPARAM *drawparamPtr,
                           VIEWPARAM *viewparamPtr) {
  int i, storeflag;
#ifdef JIM
  int endFlag;
#endif
  char **string1, **string2, **tempPtr;
  char **string1end, **string2end;
  char *ptr;

  string1 = current;
  string1end = currentEnd;
  string2 = next;
  string2end = nextEnd;

  storeflag = animateFlag;
  animateFlag = FOREVER;

  if (drawparamPtr->environment_display && LsystemListPtr->current == 0)
  /* process the environment for the axiom */
  {
    ProcessEnvironment(*string1, drawparamPtr, viewparamPtr);

    if (clp.checkEnvironment && NewViewBetweenFrames(&animparam))
      NewView();
  }

  for (i = 1; i <= derivations; i++) {
    VERBOSE("Computing derivation step %u\n", 1 + LsystemListPtr->current);

#ifdef JIM
    /* determine whether end statements should be run */
    /* not right for animation where end step > n of L-system */
    endFlag = FALSE;
    if (i == derivations && LsystemListPtr->current + 1 >= LsystemList->n) {
      endFlag = TRUE;
    }
    /* JH Oct 6 added endStep flag */
    Derive(LsystemListPtr, string1, string1end, string2, string2end,
           drawparamPtr, viewparamPtr, endFlag);
#else
    Derive(LsystemListPtr, string1, string1end, string2, string2end,
           drawparamPtr, viewparamPtr);
#endif
    tempPtr = string1;
    string1 = string2;
    string2 = tempPtr;
    tempPtr = string1end;
    string1end = string2end;
    string2end = tempPtr;
    if (clp.verbose) {
      Message("Resulting string:\n");
      PrintParameterString(stderr, *string1, SAVE_STRING, 0);
    }
    if ((animateFlag == STOP) || (animateFlag == RUN))
      break; /* added for stop function */
  }

  switch (animateFlag) {
  case STOP: /* leave STOP in the flag */
    break;
  default:
    animateFlag = storeflag;
  }

  ptr = *string1;
  *next = *string2;
  *current = ptr;

  ptr = *string1end;
  *nextEnd = *string2end;
  *currentEnd = ptr;

#ifndef JIM
  /* JH Oct6  this has been replaced by a flag in the derive; does it cause
  problems with the animate control??? */
  if ((LsystemListPtr->current >=
       ((animparam.last < 0) ? LsystemList->n : animparam.last)) ||
      animateFlag == STOP)
    /* Evaluate global statements */
    EvaluateEndStatements(LsystemListPtr);
#endif
}

/************************************************************************/

void CalculateViewVolume(char *string, const DRAWPARAM *drawparamPtr,
                         VIEWPARAM *viewparamPtr) {
  DRAWPARAM dr = *drawparamPtr;
  /* if view volume has not been input calculate it */

  if (viewparamPtr->auto_window) {
    /* rendering specification */
    dr.tdd = vvSetDispatcher(&dr, viewparamPtr);
    dr.gllighting = 0;
    dr.ourlighting = 0;
    dr.vertexbound = 0;
    dr.output_type = TYPE_VIEWVOLUME;

    TurtleDraw(string, &dr, viewparamPtr);
  }
}

/************************************************************************/
/*
  if necessary this function goes through the L-system string and sets
  the environmental query modules
*/

void ProcessEnvironment(char *string, DRAWPARAM *drawparamPtr,
                        VIEWPARAM *viewparamPtr) {
  /* gls dispatcher */
  drawparamPtr->tdd = glsSetDispatcher(drawparamPtr, viewparamPtr);
  drawparamPtr->output_type = TYPE_ENVIRONMENT;

  VERBOSE("Processing Environment:\n");
  if (clp.checkEnvironment) {
    TurtleDraw(string, drawparamPtr, viewparamPtr);
  }

  if (clp.verbose) {
    Message("Resulting string:\n");
    PrintParameterString(stderr, string, SAVE_STRING, 0);
  }
}

/************************************************************************/
/*
   according to the current derivation step and given set of intervals
   determines the number of derivation to the next desired frame.
   */

static int GetAnimStep(int *current_step, ANIMPARAM *aptr) {
  int ind, n, step, i;

  if (aptr->intervals[0].from < 0)
    return aptr->step;

  ind = 0;

  for (;;) {
    if (aptr->intervals[ind].from < 0)
      return aptr->step;

    if (*current_step < aptr->intervals[ind].to) {
      n = aptr->intervals[ind].from;

      step = aptr->intervals[ind].step;
      if (step == 0)
        step = 1;
      if (step < 0)
        step = aptr->step;

      while (n <= *current_step)
        n += step;

      if (n <= aptr->intervals[ind].to) {
        if (aptr->intervals[ind].function == 0)
          /* no function */
          return n - *current_step;

        if (aptr->intervals[ind].function == 1) {
          /* rotate the object */
          for (i = 0; i < 3; i++)
            viewparam.objectRotation[i] += aptr->intervals[ind].data[i];
        }
        if (aptr->intervals[ind].function == 2) {
          /* scale the object */
          for (i = 0; i < 3; i++)
            viewparam.objectScale[i] *= aptr->intervals[ind].data[i];
        }
        if (aptr->intervals[ind].step == 0) {
          (*current_step)++;
          return 0;
        }
        return n - *current_step;
      }
    }

    if (++ind >= MAXANIMINTERVALS)
      return aptr->step;
  }
}

/************************************************************************/

/************************************************************************/

void SetView(VIEWPARAM *viewPtr) {
#ifndef NOGRAPHICS
  SetGraphics();
  double aspect, zshift;

  glViewport(0, 0, clp.xsize, clp.ysize);

  /* Set viewing parameters as a function of the view volume */

  /* set desired projection */
  glMatrixMode(GL_PROJECTION);

  viewPtr->real_to_pixel = 1.0;

  if (viewPtr->parallel_projection_on) {

    CalculateWindow(viewPtr, &viewWindow,
                    ((float)(clp.ysize)) / ((float)(clp.xsize)));

    glLoadIdentity();

    glOrtho(viewWindow.left, viewWindow.right, viewWindow.bottom,
            viewWindow.top, viewPtr->front_dist, viewPtr->back_dist);

    viewPtr->real_to_pixel = clp.ysize / (viewWindow.top - viewWindow.bottom);
  } else {
    glLoadIdentity();

    aspect = (double)(clp.xsize) / (double)(clp.ysize);
    gluPerspective(viewPtr->viewing_angle, aspect, viewPtr->front_dist,
                   viewPtr->back_dist);

    // MC - Dec. 2015 - set real_to_pixel so that panning in perspective mode is
    // fov angle dependent
    zshift = viewPtr->vrp[2] +
             (viewPtr->viewpoint[2] - viewPtr->vrp[2]) / viewPtr->scale;
    viewPtr->real_to_pixel =
        ((double)(clp.ysize)) /
        (2.0 * zshift * tan(0.5 * viewPtr->viewing_angle * M_PI / 180.0));
  }

  /* distance depends on the scaling */
  gluLookAt(viewPtr->vrp[0] +
                (viewPtr->viewpoint[0] - viewPtr->vrp[0]) / viewPtr->scale,
            viewPtr->vrp[1] +
                (viewPtr->viewpoint[1] - viewPtr->vrp[1]) / viewPtr->scale,
            viewPtr->vrp[2] +
                (viewPtr->viewpoint[2] - viewPtr->vrp[2]) / viewPtr->scale,
            viewPtr->vrp[0], viewPtr->vrp[1], viewPtr->vrp[2],
            viewPtr->view_up[0], viewPtr->view_up[1], viewPtr->view_up[2]);

  /* XXXXXXXXXXXXXXX not supported by OpenGL XXXXXXXXXXXXXXXXXXXXXXXX */
  /* simulating by fog */

  if (viewPtr->depth_cue_on) {
    glEnable(GL_FOG);
    VERBOSE("depth cueing not on\n");
  } else {
    glDisable(GL_FOG);
    VERBOSE("depth cueing off\n");
  }

  if (viewPtr->z_buffer_on) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  /* For transparent textures */
  if (!initializeAlpha){
    // for some reason this should be done only once otherwise the background becomes
    // fully transparent and the color is not displayed anymore
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glAlphaFunc(GL_GREATER, 0.1f);
    glEnable(GL_ALPHA_TEST);
    initializeAlpha = 1;
  }
  
  if (viewPtr->antialiasing_on)
    glEnable(GL_LINE_SMOOTH);
  else
    glDisable(GL_LINE_SMOOTH);

  glMatrixMode(GL_MODELVIEW);
  
#endif
}

/********************************************************************/
/*
        Function: CalculateWindow
        Determine the window given the view volume.  The viewport is also
        considered, supposedly to establish the correct aspect ratio.
        Only centers correctly with a view down the Z-axis.
        A rethinking is necessary here.

        For MonaLisa must allocate and Set buffers.
*/
/********************************************************************/

void CalculateWindow(const VIEWPARAM *viewPtr, RECTANGLE *windowPtr,
                     float aspect_ratio) {
#ifndef NOGRAPHICS
  float center_x, center_y, span_x, span_y, span;
  float minx, maxx, miny, maxy;
  Matrix rotation, store;
  int i;

  // float aspect_ratio = ((float)(clp.ysize)) / ((float)(clp.xsize)); // MC -
  // changed to function parameter

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();

  /* set the matrix representing orthographic projection */
  glLoadIdentity();
  glOrtho(-1, 1, -1, 1, 0, 1);

  /* distance depends on the scaling */
  gluLookAt(viewPtr->vrp[0] +
                (viewPtr->viewpoint[0] - viewPtr->vrp[0]) / viewPtr->scale,
            viewPtr->vrp[1] +
                (viewPtr->viewpoint[1] - viewPtr->vrp[1]) / viewPtr->scale,
            viewPtr->vrp[2] +
                (viewPtr->viewpoint[2] - viewPtr->vrp[2]) / viewPtr->scale,
            viewPtr->vrp[0], viewPtr->vrp[1], viewPtr->vrp[2],
            viewPtr->view_up[0], viewPtr->view_up[1], viewPtr->view_up[2]);
  /* set the matrix representing orthographic projection */

  /* first four points (z=min)*/
  for (i = 0; i < 4; i++) {
    rotation[i][3] = 1.0;
    rotation[i][2] = viewPtr->min[2];
  }
  rotation[0][0] = viewPtr->min[0];
  rotation[0][1] = viewPtr->min[1];

  rotation[1][0] = viewPtr->max[0];
  rotation[1][1] = viewPtr->min[1];

  rotation[2][0] = viewPtr->max[0];
  rotation[2][1] = viewPtr->max[1];

  rotation[3][0] = viewPtr->min[0];
  rotation[3][1] = viewPtr->max[1];

  glGetFloatv(GL_PROJECTION_MATRIX, &store[0][0]);
  glMultMatrixf(&rotation[0][0]);

  /* find min and max coordinates of the four points in the view plane */
  glGetFloatv(GL_PROJECTION_MATRIX, &rotation[0][0]);
  glLoadMatrixf(&store[0][0]);

  minx = maxx = rotation[0][0];
  miny = maxy = rotation[0][1];

  for (i = 1; i < 4; i++) {
    if (minx > rotation[i][0])
      minx = rotation[i][0];
    if (miny > rotation[i][1])
      miny = rotation[i][1];
    if (maxx < rotation[i][0])
      maxx = rotation[i][0];
    if (maxy < rotation[i][1])
      maxy = rotation[i][1];
  }

  /* second four points (z=min)*/
  for (i = 0; i < 4; i++) {
    rotation[i][3] = 1.0;
    rotation[i][2] = viewPtr->max[2];
  }
  rotation[0][0] = viewPtr->min[0];
  rotation[0][1] = viewPtr->min[1];

  rotation[1][0] = viewPtr->max[0];
  rotation[1][1] = viewPtr->min[1];

  rotation[2][0] = viewPtr->max[0];
  rotation[2][1] = viewPtr->max[1];

  rotation[3][0] = viewPtr->min[0];
  rotation[3][1] = viewPtr->max[1];

  glMultMatrixf(&rotation[0][0]);

  /* find min and max coordinates of the four points in the view plane */
  glGetFloatv(GL_PROJECTION_MATRIX, &rotation[0][0]);

  for (i = 0; i < 4; i++) {
    if (minx > rotation[i][0])
      minx = rotation[i][0];
    if (miny > rotation[i][1])
      miny = rotation[i][1];
    if (maxx < rotation[i][0])
      maxx = rotation[i][0];
    if (maxy < rotation[i][1])
      maxy = rotation[i][1];
  }

  glPopMatrix();

  center_x = (minx + maxx) / 2;
  center_y = (miny + maxy) / 2;
  span_x = (maxx - minx) / 2;
  span_y = (maxy - miny) / 2;
  span = (span_x > span_y / aspect_ratio) ? span_x : span_y / aspect_ratio;
  span = span / viewPtr->scale;

  if (span <= 0) {
    span = 1;
    Message("Warning: view volume is zero\n");
  }

  windowPtr->left = center_x - span;
  windowPtr->right = center_x + span;
  windowPtr->bottom = center_y - span * aspect_ratio;
  windowPtr->top = center_y + span * aspect_ratio;
#endif
}

/************************************************************************/
/* Calls set up routine for screen dispatcher */
/* a level of indirection to allow easy MonaLisa addition */
static turtleDrawDispatcher *SetScreenDispatcher(DRAWPARAM *dr, VIEWPARAM *vw) {
#ifdef NOGRAPHICS
  return NULL;
#else
  return (iGlSetDispatcher(dr, vw));
#endif
}

/************************************************************************/
/* open the file for writing and write the content */
/* if not possible or desired return NULL     */
void OpenOutputFile(char *filename, int format) {
  FILE *fp;
  char buffer[255];
  int len;
  VERBOSE("%s\n", filename);

  if ((fp = fopen(filename, "r")) != NULL) {
    fclose(fp);
    strcpy(buffer, "Overwrite ");

    if ((len = strlen(filename)) > MAXWINLEN) {
      strcat(buffer, "...");
      strcat(buffer, &filename[len - MAXWINLEN]);
    } else
      strcat(buffer, filename);

    strcat(buffer, " ?");

    Dialog_Box(buffer, filename, format);
  } else {
    SaveFile(filename, format);
    Free(filename);
    filename = NULL;
  }
}

/* ---------------- saveTGA --------------------------- */

void saveTGA_RLE(char *fname) {
  FILE *file;
  GLint xsize, ysize;
  int y, x;
  GLboolean index_mode;
  unsigned char *row = NULL;
  unsigned char *row2 = NULL;
  targa_params_type TGAspec = {0};
  RLE_params_type RLEspec = {0};
  char is_rle;
  GLint tmp[4];

  glGetIntegerv(GL_VIEWPORT, tmp);
  xsize = tmp[2];
  ysize = tmp[3];

  is_rle = 0;
  {
    const char *pDot = strrchr(fname, '.');
    if (NULL != pDot) {
      if (!strcmp(pDot + 1, "rle"))
        is_rle = 1;
    }
  }

  if ((row = (unsigned char *)Malloc((unsigned int)(xsize * sizeof(char)))) ==
      NULL)
    return;

  if ((row2 = (unsigned char *)Malloc(
           (unsigned int)(3 * xsize * sizeof(char)))) == NULL)
    return;

  if ((file = fopen(fname, "wb")) == NULL)
    Message("Cannot open output image (targa/rle) %s.\n", fname);

  /* Sets a temporary othographics projection corresponding with the size
of the saved image */

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glOrtho(-0.5, xsize + 0.5, -0.5, ysize + 0.5, 0, 1);

  /* actual read buffer is the front one */
  glReadBuffer(GL_FRONT);

  /* query the actual mode */
  glGetBooleanv(GL_INDEX_MODE, &index_mode);

  /* all fields must be initialized to 0 */
  if (is_rle) {
    RLEspec.fp = file;
    RLEspec.Xres = xsize;
    RLEspec.Yres = ysize;
  } else {
    TGAspec.fp = file;
    TGAspec.Xres = xsize;
    TGAspec.Yres = ysize;
  }

  if (!index_mode) {
    if (is_rle) {
      saveRLEhead(&RLEspec);

      for (y = 0; y < ysize; y++) {
        glReadPixels(0, y, (int)xsize, 1, GL_RED, GL_UNSIGNED_BYTE, row2);
        glReadPixels(0, y, (int)xsize, 1, GL_GREEN, GL_UNSIGNED_BYTE,
                     row2 + xsize);
        glReadPixels(0, y, (int)xsize, 1, GL_BLUE, GL_UNSIGNED_BYTE,
                     row2 + 2 * xsize);

        saveRLErow(&RLEspec, y, row2);
      }
    } else {
      TGAspec.type = TGA_TRUECOLOR_RLE;
      saveTGAhead(&TGAspec);

      for (y = ysize - 1; y >= 0; y--) {
        glReadPixels(0, y, (int)xsize, 1, GL_BLUE, GL_UNSIGNED_BYTE, row);
        for (x = 0; x < xsize; x++)
          row2[3 * x + 0] = row[x];

        glReadPixels(0, y, (int)xsize, 1, GL_GREEN, GL_UNSIGNED_BYTE, row);
        for (x = 0; x < xsize; x++)
          row2[3 * x + 1] = row[x];

        glReadPixels(0, y, (int)xsize, 1, GL_RED, GL_UNSIGNED_BYTE, row);
        for (x = 0; x < xsize; x++)
          row2[3 * x + 2] = row[x];

        saveTGArow(&TGAspec, y, row2);
      }
    }
  } else {
    /* index mode */

    if (is_rle) {
      Message("Cannot save rle image in index mode\n");
      Free(row);
      row = NULL;
      Free(row2);
      row2 = NULL;
      return;
    }

    TGAspec.type = TGA_GRAY;

    /* set the colormap - not done yet */

    saveTGAhead(&TGAspec);

    for (y = ysize - 1; y >= 0; y--) {
      glReadPixels(0, y, (int)xsize, 1, GL_COLOR_INDEX, GL_UNSIGNED_BYTE, row);
      saveTGArow(&TGAspec, y, row);
    }
  }
  Free(row);
  row = NULL;
  Free(row2);
  row = NULL;

  if (is_rle)
    saveRLEfinish(&RLEspec);
  else
    saveTGAfinish(&TGAspec);

  /* restores the previous projection */
  glPopMatrix();
  /* modelview matrix is kept as default */
  glMatrixMode(GL_MODELVIEW);
}

/************************************************************************/
void savewindow(char *name, int rgb) {
  GLint xsize, ysize;
  int y, x;
  GLboolean index_mode;
  unsigned short *rows = NULL;
  unsigned char *rowb = NULL;
  IMAGE *oimage = NULL;
  GLint tmp[4];

  if (clp.graphics_output) {
    glGetIntegerv(GL_VIEWPORT, tmp);
    xsize = tmp[2];
    ysize = tmp[3];
  } else {
    xsize = clp.xsize;
    ysize = clp.ysize;
    rgb = 0;
  }

  if ((rows = (unsigned short *)Malloc(
           (unsigned int)(xsize * sizeof(short)))) == NULL)
    return;
  if ((rowb = (unsigned char *)Malloc((unsigned int)(xsize * sizeof(char)))) ==
      NULL)
    return;

  /* Sets a temporary orthographics projection corresponding with the size
  of the saved image */

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glOrtho(-0.5, xsize - 0.5, -0.5, ysize - 0.5, 0, 1);

  /* actual read buffer is the front one */
  glReadBuffer(GL_FRONT);

  index_mode = 0;

  if (!index_mode) {
    /* RGBA mode */
    FILE *fp;
    int bfsz;
    unsigned char *bf = NULL;
    VERBOSE("Outputing truecolor image.\n");
    fp = fopen(name, "wb");
    if (NULL == fp)
      goto ret;
    bfsz = xsize * ysize * 3;

    bf = malloc(bfsz);
    if (NULL == bf) {
      fclose(fp);
      goto ret;
    }

    for (y = 0; y < ysize; y++) {
      int pos;
      pos = 2 + xsize * 3 * (ysize - y - 1);
      glReadPixels(0, y, (int)xsize, 1, GL_RED, GL_UNSIGNED_BYTE, rowb);
      for (x = 0; x < xsize; x++) {
        bf[pos] = rowb[x];
        pos += 3;
      }

      pos = 1 + xsize * 3 * (ysize - y - 1);
      glReadPixels(0, y, (int)xsize, 1, GL_GREEN, GL_UNSIGNED_BYTE, rowb);
      for (x = 0; x < xsize; x++) {
        bf[pos] = rowb[x];
        pos += 3;
      }

      pos = 0 + xsize * 3 * (ysize - y - 1);
      glReadPixels(0, y, (int)xsize, 1, GL_BLUE, GL_UNSIGNED_BYTE, rowb);
      for (x = 0; x < xsize; x++) {
        bf[pos] = rowb[x];
        pos += 3;
      }
    }
    WriteIRIS(fp, bf, 2, xsize, ysize, 0, 0, 0, 102);
    fclose(fp);
    free(bf);
  } else {
    /* index mode */
    VERBOSE("Outputing colormap image.\n");

    oimage = iopen(name, "w", RLE(1), 2, xsize, ysize, 1);
    if (oimage == NULL) {
      perror(name);
      goto ret;
    }
    oimage->colormap = CM_SCREEN; /* see image.h */
    isetname(oimage, name);

    for (y = 0; y < ysize; y++) {
      glReadPixels(0, y, (int)xsize, 1, GL_COLOR_INDEX, GL_UNSIGNED_SHORT,
                   rows);
      putrow(oimage, rows, y, 0);
    }
  }
ret:
  if (NULL != rows) {
    Free(rows);
    rows = NULL;
  }
  if (NULL != rowb) {
    Free(rowb);
    rowb = NULL;
  }
  if (NULL != oimage) {
    iclose(oimage);
    oimage = NULL;
  }
  /* restores the previous projection */
  glPopMatrix();
  /* modelview matrix is kept as default */
  glMatrixMode(GL_MODELVIEW);
}

/************************************************************************/
void MakeEnoughRoom(unsigned long len) {
  unsigned long step = (unsigned long)floor(log((double)len) / log(2.0)) / 2;
  if (step <= 100000)
    step = 100000;

  /* check whether enough room */
  if (currentString + len + 1 > currentStringEnd) {
    currentStringLength = (1 + (len + 1000) / step) * step;
    Message("Inserted string is too long; reallocating.\n");
    /* reallocate should start one character to the left */
    if ((currentString =
             (char *)Realloc(currentString - 1, currentStringLength)) == NULL) {
      Message("Error: can't reallocate L-system string.\n");
      MyExit(1);
    }
    currentStringEnd = currentString + currentStringLength - 1;
    currentString++;
  }
}

/************************************************************************/
char unique_strb_header[] = "L-system string:";

int InputString(char *filename, char type) {
  FILE *fp;
  char str[30];
  unsigned long len;

  if (strcmp(filename, "stdin") == 0)
    fp = stdin;
  else if ((fp = fopen(filename, "r")) == NULL) {
    Message("Cannot open string file %s!\n", filename);
    return 0;
  }

  if (type == 'b') {
    /* binary */
    if (fread(str, strlen(unique_strb_header), 1, fp) < 1) {
      Message("Cannot read header of file %s!\n", filename);
      return 0;
    }
    str[strlen(unique_strb_header)] = 0;

    /* safety check */
    if (strcmp(unique_strb_header, str) != 0) {
      Message("File doesn't start with correct header '%s'! '%s' found.\n",
              unique_strb_header, str);
      return 0;
    }

    /* get the length */
    if (fscanf(fp, "%lu %d", &len, &LsystemList->current) < 1) {
      Message("Cannot read string length and/or current step!\n");
      return 0;
    }

    fgetc(fp);

    MakeEnoughRoom(len);

    if (fread(currentString, 1, len, fp) < len)
      Message("Only part of the string read in !!!\n");
  } else {
    char mod;
    float val;
    /* text string */

    /* no header */
    len = 0;

    while (!feof(fp)) {
      MakeEnoughRoom(len + MAXPARMS * 5 + 2);

      mod = fgetc(fp);

      /* skip white spaces */
      if (isspace(mod))
        continue;

      currentString[len++] = mod;

      if (mod == '(') {
        mod = ',';

        while (mod == ',') {
          fscanf(fp, "%f", &val);
          memcpy(currentString + len, &val, sizeof(val));
          len += sizeof(val);

          mod = fgetc(fp);
          currentString[len++] = mod;
        }

        if (mod != ')')
          Message("Error! ')' expected!\n");
      }
    }
  }

  if (fp != stdin)
    fclose(fp);

  *(currentString + len) = 0;

  /* reset initial rotation */
  viewparam.xRotation = 0;
  viewparam.yRotation = 0;
  viewparam.zRotation = 0;
  viewparam.xPan = 0;
  viewparam.yPan = 0;

  init_blackbox(); /* initialize blackbox functions	*/
  Redraw(rfScale | rfViewVol | rfClear);

  return 1;
}

/************************************************************************/
int StringDump(FILE *fp) {
  unsigned long len;

  len = currentStringLength;
  if (fwrite(unique_strb_header, strlen(unique_strb_header), 1, fp) < 1) {
    Message("Saving unique header failed!\n");
    return 0;
  }

  if (fprintf(fp, "%lu %d\n", len, LsystemList->current) < 2) {
    Message("Saving of string length and current step failed!\n");
    return 0;
  }

  if (fwrite(currentString, 1, len, fp) < len)
    Message("String dump failed! Do you have %lu bytes available"
            "on the disk?\n",
            len);

  return 1;
}

/************************************************************************/
void SaveFile(char *name, int format) {

  char *tmp;
  DRAWPARAM dr = drawparam;
  VERBOSE("Save File : %s\n", name);
#ifdef WIN32
  if (SAVE_BMP == format) {
    SaveAsBmp(name);
    return;
  }
#endif

  if (strncmp(name, "stdout", 6) == 0) {
    clp.savefp[format] = stdout;
    Message("Standard output used.\n");
  } else if ((clp.savefp[format] = fopen(name, "w")) == NULL) {
    Message("(control.c) SaveFile - Can't open %s\n", name);
    return;
  }

  tmp = clp.savefilename[format];
  clp.savefilename[format] = name;

  switch (format) {
  case SAVE_RGB:
    if (clp.savefp[SAVE_RGB] != NULL)
      if (clp.savefp[SAVE_RGB] != stdout)
        fclose(clp.savefp[SAVE_RGB]);
    /* Save window as RGB file */
    savewindow(name, 1);
    break;
  case SAVE_PNG:
    Warning("[E] To be Implemented *******\n",0);
    break;
  case SAVE_BMP:
    Warning("[E] To be Implemented *******\n",0);
    break;
  case SAVE_GIF:
    Warning("[E] To be Implemented *******\n",0);
    break;
  case SAVE_JPG:
    Warning("[E] To be Implemented *******\n",0);
    break;
  case SAVE_PBM:
    Warning("[E] To be Implemented *******\n",0);
    break;
  case SAVE_TIFF:
    Warning("[E] To be Implemented *******\n",0);
    break;
  case SAVE_RAS:
    if (clp.savefp[SAVE_RAS] != NULL)
      if (clp.savefp[SAVE_RAS] != stdout)
        fclose(clp.savefp[SAVE_RAS]);
    /* Save window as RAS file */
    savewindow(name, 0);
    break;

  case SAVE_TGA:
    if (clp.savefp[SAVE_TGA] != NULL)
      if (clp.savefp[SAVE_TGA] != stdout)
        fclose(clp.savefp[SAVE_TGA]);
    /* Save window as TGA file */
    saveTGA_RLE(name);
    break;

  case SAVE_RAYSHADE:
    /* interpret using rayshade routines */
    dr.output_type = TYPE_RAYSHADE;
    dr.gllighting = 1;
    dr.ourlighting = 0;
    dr.tdd = rsSetDispatcher(&dr, &viewparam);
    if (isObjectProduction) {
      /* two passes */
      rsSetPass(1);
      VERBOSE("rayshade: first pass\n");

      TurtleDraw(currentString, &dr, &viewparam);

      VERBOSE("rayshade: second pass\n");
    } else
      VERBOSE("rayshade: single pass\n");

    rsSetPass(2);
    TurtleDraw(currentString, &dr, &viewparam);

    break;

  case SAVE_POSTSCRIPT:

    /* interpret using postscript routines */
    dr.output_type = TYPE_POSTSCRIPT;
    dr.tdd = psSetDispatcher(&dr, &viewparam);
    TurtleDraw(currentString, &dr, &viewparam);
    break;

  case SAVE_VIEWVOLUME:
    /* set up for view volume output */
    if (clp.savefp[SAVE_VIEWVOLUME] == NULL) {
      break;
    }
    fprintf(clp.savefp[SAVE_VIEWVOLUME], "box: x: %g,%g y: %g,%g z: %g,%g\n",
            viewparam.min[0], viewparam.max[0], viewparam.min[1],
            viewparam.max[1], viewparam.min[2], viewparam.max[2]);
    /* close output file */
    if (clp.savefp[SAVE_VIEWVOLUME] != stdout)
      fclose(clp.savefp[SAVE_VIEWVOLUME]);
    clp.savefp[SAVE_VIEWVOLUME] = NULL;
    break;

  case SAVE_STRING:
    if (clp.savefp[SAVE_STRING] == NULL) {
      break;
    }
    PrintParameterString(clp.savefp[SAVE_STRING], currentString, SAVE_STRING,
                         clp.stringIO_homo);
    /* close output file */
    if (clp.savefp[SAVE_STRING] != stdout)
      fclose(clp.savefp[SAVE_STRING]);
    clp.savefp[SAVE_STRING] = NULL;
    break;

  case SAVE_STRINGDUMP:
    if (clp.savefp[SAVE_STRINGDUMP] == NULL) {
      break;
    }

    clp.savefilename[format] = tmp;
    if (!clp.stringIO_homo)
      StringDump(clp.savefp[SAVE_STRINGDUMP]);
    else
      PrintParameterString(clp.savefp[SAVE_STRINGDUMP], currentString,
                           SAVE_STRINGDUMP, 1);

    /* close output file */
    if (clp.savefp[SAVE_STRINGDUMP] != stdout)
      fclose(clp.savefp[SAVE_STRINGDUMP]);
    clp.savefp[SAVE_STRINGDUMP] = NULL;
    break;

  case SAVE_GLS:
    dr.output_type = TYPE_GLS;
    if (clp.savefp[SAVE_GLS] == NULL) {
      break;
    }

    clp.savefilename[format] = tmp;
    dr.tdd = glsSetDispatcher(&dr, &viewparam);

    TurtleDraw(currentString, &dr, &viewparam);
    break;
  }
  clp.savefilename[format] = tmp;
}

void SaveViewVolume(void) {
  FILE *fp;
  fp = fopen("output.vv", "wt");
  if (NULL == fp) {
    Message("Cannot create file output.vv\n");
    return;
  }
  fprintf(fp, "box: x: %g,%g y: %g,%g z: %g,%g\n", viewparam.min[0],
          viewparam.max[0], viewparam.min[1], viewparam.max[1],
          viewparam.min[2], viewparam.max[2]);
  fclose(fp);
}

void OutputObj(void) {
  DRAWPARAM dr = drawparam;
  dr.output_type = TYPE_OBJECT;
  dr.gllighting = 1;
  dr.ourlighting = 0;
  dr.tdd = objSetDispatcher(&dr, &viewparam);

  TurtleDraw(currentString, &dr, &viewparam);
  if (clp.savefp[SAVE_OBJ] != stdout) {
    fclose(clp.savefp[SAVE_OBJ]);
  }
}

/********************************************************************/
/*								JH1
        Function: GlobalStringIndex
        Stores the pointer to a string into a global array during
        L-system parsing for later use during the interpret step
*/
/********************************************************************/
double GlobalStringIndex(char *string) /* JH1 */
{
  if (nextGlobalString < MAXSTRINGS) {
    strncpy(globalStrings[nextGlobalString], string, MAXSTRLEN);
    globalStrings[nextGlobalString][MAXSTRLEN - 1] = 0;
    nextGlobalString++;
  } else {
    Message("Warning: Too many global strings in input L-system.\n");
  }
  return (double)nextGlobalString - 1;
}

/********************************************************************/
/*								JH1
        Function: GlobalString
        Returns the pointer to a global string given its index      */
/********************************************************************/
char *GlobalString(double index) /* JH1 */
{
  int integerIndex;

  /* round to nearest integer in case of storage accuracy problems */
  integerIndex = (int)(index + 0.5);

  if (integerIndex >= nextGlobalString) {
    Message("Warning: Global string index out of range.\n");
    integerIndex = nextGlobalString - 1;
  }

  return globalStrings[integerIndex];
}

/*****************************************************************/
/*   Warning - inform the user of errors and keep track of level */
/*   Exit on internal error                                      */
/*****************************************************************/
void Warning(char *message, int level) {
  extern char *inputFile; /* file name of current input file */
  extern int lineNumber;  /* current line number */
  extern int inputError;  /* flag indicating error level */
  static const char *levelLabel[] = {"Warning", "Error", "Fatal Error",
                                     "INTERNAL ERROR"};
  if (clp.warnings){
    Message("%s: %s in line %d of %s\n",
	    (level < 4) ? levelLabel[level] : "INTERNAL ERROR", message,
	    lineNumber, inputFile);
    if (level > WARNING_LVL)
      inputError = TRUE;
    if (level >= FATAL_LVL)
      MyExit(2);
  }
}

/*****************************************************************/
/*   Warning - inform the user of errors and keep track of level */
/*   Exit on internal error                                      */
/*****************************************************************/
void WarningParsing(char *message, int level) {
  extern char *inputFile; /* file name of current input file */
  extern int lineNumber;  /* current line number */
  extern int inputError;  /* flag indicating error level */
  static const char *levelLabel[] = {"Warning", "Error", "Fatal Error",
                                     "INTERNAL ERROR"};
  Message("%s: %s in line %d of %s\n",
	  (level < 4) ? levelLabel[level] : "INTERNAL ERROR", message,
	  lineNumber, inputFile);
  if (level > WARNING_LVL)
    inputError = TRUE;
  if (level >= FATAL_LVL)
    MyExit(2);
}


extern int selection_mode;

void processHits (GLint hits, GLuint buffer[])
{
   unsigned int i, j;
   GLuint names, *ptr;

   printf ("hits = %d\n", hits);
   ptr = (GLuint *) buffer;
   for (i = 0; i < hits; i++) { /*  for each hit  */
      names = *ptr;
      printf (" number of names for hit = %d\n", names); ptr++;
      printf("  z1 is %g;", (float) *ptr/0x7fffffff); ptr++;
      printf(" z2 is %g\n", (float) *ptr/0x7fffffff); ptr++;
      printf ("   the name is ");
      for (j = 0; j < names; j++) {     /*  for each name */
         printf ("%d ", *ptr); ptr++;
      }
      printf ("\n");
   }
}



void InsertX(int x, int y) {
  GLuint selection[4 * 64]; /* maximum 64 hits * 4 entries per hit */
  GLint viewport[4];
  int hits;
  GLdouble projection_mtrx[16], pick_mtrx[16];

  viewport[0] = 0;
  viewport[1] = 0;
  viewport[2] = clp.xsize;
  viewport[3] = clp.ysize;

  glSelectBuffer(4 * 64, selection);

  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(0);
  glMatrixMode(GL_PROJECTION);

  glPushMatrix();
  glGetDoublev(GL_PROJECTION_MATRIX, projection_mtrx);

  glLoadIdentity();

  gluPickMatrix(x, viewport[3] - y, 5.0, 5.0, viewport);
  glGetDoublev(GL_PROJECTION_MATRIX, pick_mtrx);

  glMultMatrixd(projection_mtrx);
  glGetDoublev(GL_PROJECTION_MATRIX, projection_mtrx);

  glMatrixMode(GL_MODELVIEW);

  selection_mode = 1;
  Redraw(0);

  selection_mode = 0;

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glFlush();
  hits = glRenderMode(GL_RENDER);
  glMatrixMode(GL_MODELVIEW);
  // processHits(hits,selection);
  if (hits > 0) {
    /* First: find the closest object that caused the hit */
    int insert_point, i;
    GLuint minz;
    insert_point = 3;
    minz = selection[1];
    for (i = 0; i < hits; i++) {
      if (selection[i * 4 + 1] < minz) {
        minz = selection[i * 4 + 1];
        insert_point = i * 3 + 1;
      }
    }
    //    printf("selection Point = %d \n",selection[insert_point]);
    /* Now insert 'X' in the proper location */
    memmove(currentString + selection[insert_point] + 1,
            currentString + selection[insert_point],
            currentStringLength - selection[insert_point] + 1);
    currentString[selection[insert_point]] = 'X';
    currentStringLength++;
    
    VERBOSE("Hits processed\n");
  } else if (-1 == hits) {
    Message("Couldn't determine selection\n");
  } else if (0 == hits) {
    Message("Nothing selected\n");
  }
}

void DisplayFrame(float d) {
  if (d > 0)
    DisplayThisFrame = 1;
  else if (d < 0)
    DisplayThisFrame = -1;
  else
    DisplayThisFrame = 0;
}

extern float bgclr[];

void SaveRGB() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  savewindow(clp.savefilename[SAVE_RGB], 0);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SavePNG() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  Warning("[E] SavePNG to be implemented ******\n",0);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SaveGIF() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  Warning("[E] SaveGIF to be implemented ******\n",0);
  setcursor(CURSOR_ARROW, 0, 0);
}
void SaveJPG() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  Warning("[E] SaveJPG to be implemented ******\n",0);
  setcursor(CURSOR_ARROW, 0, 0);
}
void SavePBM() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  Warning("[E] SavePBM to be implemented ******\n",0);
  setcursor(CURSOR_ARROW, 0, 0);
}
void SaveBMP() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
#ifdef WIN32
  SaveAsBmp(clp.savefilename[SAVE_BMP]);
#else
  Warning("[E] SaveBMP to be implemented ******\n",0);
#endif
  setcursor(CURSOR_ARROW, 0, 0);
}
void SaveTIFF() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  Warning("[E] SaveTIFF to be implemented ******\n",0);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SaveVV() {
  FILE *fp;
  setcursor(CURSOR_HOURGLASS, 0, 0);
  fp = fopen(clp.savefilename[SAVE_VIEWVOLUME], "wt");
  fprintf(fp, "box: x: %g,%g y: %g,%g z: %g,%g\n", viewparam.min[0],
          viewparam.max[0], viewparam.min[1], viewparam.max[1],
          viewparam.min[2], viewparam.max[2]);
  fclose(fp);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SaveTGA() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  SaveFile(clp.savefilename[SAVE_TGA], SAVE_TGA);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SaveRayshade() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  SaveFile(clp.savefilename[SAVE_RAYSHADE], SAVE_RAYSHADE);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SaveOBJ() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  OutputObj();
  setcursor(CURSOR_ARROW, 0, 0);
}

void SavePostscript() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  SaveFile(clp.savefilename[SAVE_POSTSCRIPT], SAVE_POSTSCRIPT);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SaveString() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  SaveFile(clp.savefilename[SAVE_STRING], SAVE_STRING);
  setcursor(CURSOR_ARROW, 0, 0);
}

void SaveGLS() {
  setcursor(CURSOR_HOURGLASS, 0, 0);
  SaveFile(clp.savefilename[SAVE_GLS], SAVE_GLS);
  setcursor(CURSOR_ARROW, 0, 0);
}

void DrawExpired() {
#ifdef WIN32
  float mview[16];
  float proj[16];
  GLboolean depth;
  GLboolean light;
  GLboolean blend;
  const float scale = 2.5f;

  glGetFloatv(GL_MODELVIEW_MATRIX, mview);
  glGetFloatv(GL_PROJECTION_MATRIX, proj);

  depth = glIsEnabled(GL_DEPTH_TEST);

  glDisable(GL_DEPTH_TEST);

  light = glIsEnabled(GL_LIGHTING);

  glDisable(GL_LIGHTING);

  blend = glIsEnabled(GL_BLEND);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glLineWidth(3.0f);

  if (fabs(bgclr[0] - 0.5f) < 0.1f && fabs(bgclr[1] - 0.5f) < 0.1f &&
      fabs(bgclr[2] - 0.5f) < 0.1f)
    glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
  else
    glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
  glTranslatef(-0.9f, 0.8f, 0.0f);
  glScalef(scale, scale, scale);
  glRotatef(-45.0f, 0.0f, 0.0f, 1.0f);
  glListBase(oflist);
  glCallLists(20, GL_UNSIGNED_BYTE, "Expired demo version");

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(proj);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(mview);

  if (blend)
    glEnable(GL_BLEND);
  else
    glDisable(GL_BLEND);

  if (light)
    glEnable(GL_LIGHTING);

  if (depth)
    glEnable(GL_DEPTH_TEST);
  else
    glDisable(GL_DEPTH_TEST);
#endif
}

unsigned int getClearBetweenFrameFlag() {
  return (ClearBetweenFrames(&animparam));
}
