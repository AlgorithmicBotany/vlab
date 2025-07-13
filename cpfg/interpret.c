
/********************************************************************/
/*                           interpret.c              		        */
/*                                                                  */
/* interpret creates a picture by controlling a turtle according to */
/* a string produced by generate using viewing parameters specified */
/* in the viewfile. 	                                            */
/*                                                                  */
/* The interpreted symbols are: Ff +- [] | /\ &^ ;, _ {} !# @ ~ %     */
/*               i.   If symbol is '+' turn the turtle right    	*/
/*               ii.  If symbol is '-' turn the turtle left     	*/
/*               iii. If symbol is '|' spin the turtle around  		*/
/*                    (180 degree turn)                         	*/
/*               iv.  If symbol is '&' pitch the turtle down    	*/
/*               v.   If symbol is '^' pitch the turtle up      	*/
/*               vi.  If symbol is '/' roll the turtle right    	*/
/*               vii. If symbol is '\' roll the turtle left     	*/
/*               viii.If symbol is '[' save current status of   	*/
/*                    turtle on the stack                       	*/
/*               ix.  If symbol is ']' pop the status on the top of */
/*                    the stack and make it the turtle's current 	*/
/*                    status. Move the turtle to the new position. 	*/
/*               x.   If symbol is '{' save the vertices to     	*/
/*                    follow in the polygon stack so that they  	*/
/*                    may be filled when a '}' is encountered   	*/
/*               xi.  If symbol is '}' get the most current     	*/
/*                    polygon array from the top of the stack   	*/
/*                    and fill this polygon with the current color 	*/
/*               xii. If symbol is 'F' move the turtle forwards 	*/
/*                    one step, save the vertices in the polygon	*/
/*                    array if '{' was hit before, and draw a line 	*/
/*                    from the old position to this new position.   */
/*               xiii.If symbol is 'f' move the turtle forwards 	*/
/*                    one step and  save the vertices in the    	*/
/*                    polygon array if '{' was hit before       	*/
/*                    (do not draw a line)                      	*/
/*               xiv. If symbol is ';' decrease the color index 	*/
/*               xv.  If symbol is ',' increase the color index 	*/
/*               xv.  If symbol is '_' increase the elasticity	 	*/
/*               xvi. The symbol ! decreases line width         	*/
/*               xvii.The symbol # increases line width         	*/
/*               xviii. The symbol ? stacks sub-lsystem scale      	*/
/*               xix. The symbol $ unstacks sub-lsystem scale      	*/
/*               xx.  The symbol @ calls a blackbox function	 	*/
/*                 specified by following symbol(s) in the string. 	*/
/*               xxi. The symbol '~' draws the surface indicated 	*/
/*				   by the immediatly following symbol at the
 */
/*			 	   current location with the current
 * orientation.
 */
/*               xxii. The symbol '%' cuts the remainder of a branch*/
/*                                                                  */
/********************************************************************/

#ifdef WIN32
#include "warningset.h"
#endif

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef WIN32
#include <unistd.h>
#else
#include <stdlib.h>
#endif

#ifdef IRIX
#include <bstring.h>
#endif
#ifdef IRIX64
#include <strings.h>
#endif
#ifdef LINUX
#include <limits.h>
#endif

#include "platform.h"

#include "interpret.h"
#include "generate.h"
#include "control.h"
#include "quaternions.h"
#include "utility.h"

#include "blackbox.h"
#include "patch.h"
#include "tsurfaces.h"
#include "general.h"
#include "textures.h"
#ifdef CPFG_ENVIRONMENT
#include "comm_lib.h"
#endif
#include "background.h"
#include "rayshade.h"

#if CPFG_VERSION >= 4000
#include "splinefunC.h"
#endif
#if CPFG_VERSION >= 6400
#include "curveXYZc.h"
#endif

#include "test_malloc.h"

#include "maxpth.h"

/*********************************************************************/
/* Global environmental structure - defined in environment.c         */
/*********************************************************************/

extern struct ENVIRONMENTPARAM environmentparam;
void read_light(char *);
#define EPr environmentparam

void InitSymbolInstances(LSYSDATA *lsysListPtr);
Symbol *CurrentSymbolFind(char *label, Symbol *currentSymbol);

/* library prototype */
#ifdef __cplusplus
extern "C" {
void bcopy(const void *src, void *dst, int length);
}
#endif

/*********************************************************************
  local prototypes **/

static void change_dir(double *vect1, double *vect2, double *vect3,
                       double sinangle, double cosangle);
static void TurtleRotate(double *temp, const double *vector,
                         const double *perpvect, double sinangle,
                         double cosangle);
static void InitializeParameters(DRAWPARAM *drawPtr, VIEWPARAM *viewPtr);
static char *NextLabel(FILE *fp, int *lineNumber);
static void CheckInputError(FILE *fp, int inputFlag, int inputCount,
                            char *labelPtr, int lineNumber, char *fileName);
static void bend(DRAWPARAM *dr, TURTLE *tu);
void UpdateCurrentLsystem(char c, TURTLE *tu);
#ifdef JIM
static void AssignSymbolsToInstancesForInterpret(Symbol *symbolPtr,
                                                 SymbolInstance *instancePtr,
                                                 SUBLSYSTEM *subLstackPtr,
                                                 SUBLSYSTEM *subLbottom);
#endif

static void read_shadow_map(
    char *input_line,
    DRAWPARAM *drawPtr); // MC - Oct. 2015 - read the shadow map command

/*********************************************************************/
/*********************************************************************/
/* Stack used to save the current position of the turtle so that it  */
/* may return to that position after a branch (like a subroutine     */
/* return except that it is a turtle return).                        */
/*********************************************************************/

TURTLE *turtlestack, *turtlebottom, *turtletop;
int turtlestacksize;

/*********************************************************************/
/* stack used to store polygon vertices (vertices calculated inside  */
/* { }) which will then be filled by the current color.              */
/*********************************************************************/

POLYGON *polystack, *polybottom, *polytop;
int polystacksize;
POLYGON *polystackPtr;

/********************************************************************/
/* stack used to store sub-L-system parameters
 */
/* currently limited to id and scaling (which defaults to 1)		*/
/********************************************************************/

SUBLSYSTEM *subLstack, *subLbottom, *subLtop;
int subLstacksize;

/********************************************************************/
/* array used to store turtle parameters for cut substrings
 */
TURTLE *substr_turtle = NULL;
int substr_turtle_size;
int num_substr_turtle;

/* structure to remember cut substrings to be moved to the end */
static int substr_array_size;

int num_cut_substrings;

SUBSTRING *cut_substrings = NULL;

/* This variable is set to 1 if TurtleDrawNsteps is called for selection */
int selection_mode = 0;

// [PASCAL] add a global variable to keep concavePolygons in memory
int concavePolygons = 0;

int interpret_new_model;

/********************************************************************/

void InitializeStacks(void) {
  POLYGON *polystackIter;

  if ((turtlestack = (TURTLE *)Malloc(TURTLESTACK_SIZE * sizeof(TURTLE))) ==
      NULL)
    Warning("Not enough memory for turtle stack!\n", FATAL_LVL);
  turtlebottom = turtlestack;
  turtletop = turtlestack + TURTLESTACK_SIZE - 1;
  turtlestacksize = TURTLESTACK_SIZE;

  if ((polystack = (POLYGON *)Malloc(POLYSTACK_SIZE * sizeof(POLYGON))) == NULL)
    Warning("Not enough memory for polygon stack!\n", FATAL_LVL);
  polystack->max_edges = 200;
  polystack->vertex = (TURTLE *)Malloc(polystack->max_edges * sizeof(TURTLE));
  polybottom = polystack;
  polytop = polystack + POLYSTACK_SIZE - 1;
  polystacksize = POLYSTACK_SIZE;
  for (polystackIter = polytop; polystackIter > polybottom; polystackIter--)
    polystackIter->vertex = NULL;

  if ((subLstack = (SUBLSYSTEM *)Malloc(MAXNESTING * sizeof(SUBLSYSTEM))) ==
      NULL)
    Warning("Not enough memory for subL-system stack!\n", FATAL_LVL);
  subLbottom = subLstack;
  subLtop = subLstack + MAXNESTING - 1;
  subLstacksize = MAXNESTING;
}

/********************************************************************/

void FreeStacks(void) {
  if (turtlestack) {
    Free(turtlestack);
    turtlestack = NULL;
  }

  if (polystack) {
    for (polystackPtr = polytop; polystackPtr >= polybottom; polystackPtr--)
      Free(polystackPtr->vertex);
    Free(polystack);
    polystack = NULL;
  }

  if (subLstack) {
    Free(subLstack);
    subLstack = NULL;
  }
}

/********************************************************************/

void Free_substr_turtle_array(void) {
  if (substr_turtle != NULL) {
    Free(substr_turtle);
    substr_turtle = NULL;
  }
}

/********************************************************************/

void Initialize_substr_turtle_array(void) {
  Free_substr_turtle_array();

  if ((substr_turtle = (TURTLE *)Malloc(SUBSTR_INITIALSIZE * sizeof(TURTLE))) ==
      NULL)
    Warning("Not enough memory for turtle for substrings!\n", FATAL_LVL);
  substr_turtle_size = SUBSTR_INITIALSIZE;
  num_substr_turtle = 0;
}

static void reallocateTurtleStack(TURTLE **ptr) {
  int dist = (int)(*ptr - turtlestack);

  turtlestacksize += TURTLESTACK_SIZE;
  Message("Reallocating turtle stack %d\n", turtlestacksize);

  if ((turtlestack = (TURTLE *)Realloc(
           turtlestack, turtlestacksize * sizeof(TURTLE))) == NULL)
    Warning("Not enough memory for turtle stack!\n", FATAL_LVL);
  turtlebottom = turtlestack;
  turtletop = turtlestack + turtlestacksize - 1;
  *ptr = turtlestack + dist;
}

static void reallocatePolyStack(POLYGON **ptr) {
  int dist = (int)(*ptr - polystack);

  polystacksize += POLYSTACK_SIZE;
  Message("Reallocating polygon stack %d\n", polystacksize);

  if ((polystack = (POLYGON *)Realloc(polystack,
                                      polystacksize * sizeof(POLYGON))) == NULL)
    Warning("Not enough memory for poly stack!\n", FATAL_LVL);
  polybottom = polystack;
  polytop = polystack + polystacksize - 1;
  for ((*ptr) = polytop; (*ptr) >= polystack + dist; (*ptr)--)
    (*ptr)->vertex = NULL;
  *ptr = polystack + dist;
}

static void reallocateSubLStack(SUBLSYSTEM **ptr) {
  int dist = (int)(*ptr - subLstack);

  subLstacksize += MAXNESTING;
  Message("Reallocating subL-system stack %d\n", subLstacksize);

  if ((subLstack = (SUBLSYSTEM *)Realloc(
           subLstack, subLstacksize * sizeof(SUBLSYSTEM))) == NULL)
    Warning("Not enough memory for subL-system stack!\n", FATAL_LVL);
  subLbottom = subLstack;
  subLtop = subLstack + subLstacksize - 1;
  *ptr = subLstack + dist;
}

/*********************************************************************/

void InitializeTurtle(DRAWPARAM *drawPtr, TURTLE *tu) {
  short i;

  tu->heading[0] = 0.0;
  tu->heading[1] = 1.0;
  tu->heading[2] = 0.0;
  tu->left[0] = -1.0;
  tu->left[1] = 0.0;
  tu->left[2] = 0.0;
  tu->up[0] = 0.0;
  tu->up[1] = 0.0;
  tu->up[2] = -1.0;
  tu->position[0] = 0.0;
  tu->position[1] = 0.0;
  tu->position[2] = 0.0;
  tu->line_width = drawPtr->line_width * drawPtr->step_size;
  tu->elasticity = drawPtr->elasticity;

  for (i = 0; i < environmentparam.number_of_movements; i++) {
    tu->movements[i].elasticity =
        environmentparam.movements[i].initial_elasticity;
    update_movement(i, environmentparam.movements[i].initial_elasticity -
                           environmentparam.movements[i].elasticity);
  }
  tu->scale_factor = drawPtr->scale_factor;
  tu->color_index = drawPtr->color_index + clp.colormap;
  tu->color_index_back = drawPtr->color_index_back + clp.colormap;

  /* for previous turtle state */
  tu->prev_heading[0] = tu->heading[0];
  tu->prev_heading[1] = tu->heading[1];
  tu->prev_heading[2] = tu->heading[2];
  tu->prev_up[0] = tu->up[0];
  tu->prev_up[1] = tu->up[1];
  tu->prev_up[2] = tu->up[2];

  /* textures */
  tu->texture = NOTEXTURE;
  tu->tex_t = 0.0;

  tu->tangent_parameter[0] = 1.2f;
  tu->tangent_parameter[1] = 1.2f;

  tu->radius_tangent[0][0] = 0; /* use zero tangents to */
  tu->radius_tangent[0][1] = 0; /* linearly interpolate between radii */
  tu->radius_tangent[1][0] = 0;
  tu->radius_tangent[1][1] = 0;
  tu->scale_radius_tangents = 0;

  tu->contour = 0;
  tu->spline_flag = 0;
  tu->cylinder_sides = drawPtr->cylinder_sides;
}

/*********************************************************************/
/*********************************************************************/

/* Global parameters accesible in all TurtleDraw functions -
   I need C++ classes! */
extern LSYSDATA *LsystemList; /* pointer to the first L-system -homomorphism */
                              /* is using the same symbol table */

DRAWPARAM *dr;
VIEWPARAM *vw;
turtleDrawDispatcher ltdd; /* Local dispatcher-faster access */

TURTLE *tu, *turtlestackPtr;
SUBLSYSTEM *subLstackPtr;
LSYSDATA *currLsysPtr;

char polyflag;       /* inside a polygon ? */
StringModule module; /* Structure to hold symbol and parameters */
char c;              /* interpreted character */
char *str;           /* actual position in the interpreted string */
char *lastModule;    /* remember the previous symbol -
                        in InterpretSymbol, it is the current one */
char first_comm_symbol;
char *first_comm_symbol_ptr;
char env_finish_symbol; /* finish the symbol (call EndNode) */
char env_started;

int pending_interpretation = 0; /* 1 when an interpretation is in progress */

/*********************************************************************/
/* Initializes parameters for interpretation. If returns 0, interpretation
   is not possible.
   */
int TurtleDrawInitialize(char *string, DRAWPARAM *drp, VIEWPARAM *vwp) {
  LSYSDATA *LsysPtr;

  if (pending_interpretation)
    TurtleDrawFinishUp();

  dr = drp;
  vw = vwp;

  polyflag = 0;

  if (dr->tdd == NULL)
    return 0;
  ltdd = *dr->tdd;

  /* Initialize the stacks. */
  str = string;
  turtlestackPtr = turtlestack;
  polystackPtr = polystack - 1;

  subLstackPtr = subLstack;
  subLstackPtr->scale = dr->scale_factor;

  tu = turtlestackPtr;

  currLsysPtr = LsystemList; /* current L-system */
#ifdef JIM
  subLstackPtr->LsysPtr = currLsysPtr;
  subLstackPtr->id = currLsysPtr->id;
  /* set up initial sybol table instance pointers for sub-Lsystems */
  InitSymbolInstances(LsystemList);
#endif
  /* go through all homomorphism and if necessary, initialize its random number
  generator value x (xsubi) */
  LsysPtr = LsystemList;

  while (LsysPtr != NULL) {
    if (LsysPtr->Homomorphism.seed >= 0) {
      LsysPtr->Homomorphism.xsubi[0] =
          0x330E; /* like srand() - see the man page */
      LsysPtr->Homomorphism.xsubi[1] = 0xffff & LsysPtr->Homomorphism.seed;
      LsysPtr->Homomorphism.xsubi[2] =
          0xffff & (LsysPtr->Homomorphism.seed >> 16);
    }

    LsysPtr = LsysPtr->nextLsystem;
  }

  /* spline stack is kept even when no generalized cylinders used */
  reset_spline_stack();

  /* Initial blackbox and turtle */
  init_blackbox();
  InitializeTurtle(dr, tu);

  /* Initialize rendering parameters - now even for the environmental pass. */
  if (ltdd.Setup(tu, dr, vw))
    return 0;

  if (dr->output_type == TYPE_ENVIRONMENT) {
    /* for enviromental pass */

    /* initialize the structure for cut substrings */

    if (cut_substrings != NULL) {
      Free(cut_substrings);
      cut_substrings = NULL;
    }

    num_cut_substrings = 0;
    substr_array_size = SUBSTR_INITIALSIZE;

    cut_substrings = (SUBSTRING *)Malloc(sizeof(SUBSTRING) * substr_array_size);

    if (cut_substrings == NULL)
      Warning("Not enough memory to store cut substrings", INTERNAL_LVL);

    drp->env_output_symbol = 0;
    env_finish_symbol = 0;
    first_comm_symbol = 1;
    env_started = 0;
  }

  pending_interpretation = 1;

  return 1;
}

/*********************************************************************/
#ifdef CONTEXT_SENSITIVE_HOMO
void InterpretHomomorphism(char c, char *curPtr, char **str, char *leftcontext,
                           char *rightcontext, int depth,
                           void ProcessSymbol(char c, char **str, char is_homo))
#else
void InterpretHomomorphism(char c, char *curPtr, char **str, int depth,
                           void ProcessSymbol(char c, char **str, char is_homo))
#endif
{
  char *result, *endresult, *ptr, *storePtr = NULL;
  Production *prodPtr;
  int preflength; /* length of prefix string matched */
  int i;
  unsigned short store_xsubi[3];
  extern unsigned short main_xsubi[3]; /* random number generator seed */
  const int HomoStackIncrement = 100;

  /* Find a production according to c */
  /* start at first possible match */
  prodPtr = currLsysPtr->Homomorphism.firstProd[(int)c];

  while (prodPtr != NULL) {
    /* Check whether the strict predecessor matches the string at the */
    /* current position.  Check that the condition for this production */
    /* applies (this updates the symbol table as a byproduct).  If there */
    /* is no match, consider next production.  */

    if (PredDiff(prodPtr->pred, curPtr, &preflength) ||
#ifdef CONTEXT_SENSITIVE_HOMO
        RconDiff(prodPtr->rCon, rightcontext, LsystemList) ||
        LconDiff(prodPtr->lCon, leftcontext, LsystemList) ||
#endif
        CondDiff(prodPtr, curPtr, LsystemList))
      prodPtr = prodPtr->nextProduction;
    else
      break;
  }

  if (depth >= currLsysPtr->Homomorphism.depth) {
    ProcessSymbol(c, str, (char)(depth != 0));
    /* last parameter (is_homo) is set to 0 only for depth == 0 */

    if (ProcessSymbol != InterpretSymbol) {
      UpdateCurrentLsystem(c, tu);
    }

    if (prodPtr != NULL && currLsysPtr->homo_warning)
      Message("Warning: maximum depth in a homomorphism reached"
              " for module %c !\n",
              c);
    return;
  }
  if (prodPtr == NULL) {
    /* no matching production */
    ProcessSymbol(c, str, (char)(depth != 0));
    if (ProcessSymbol != InterpretSymbol) {
      UpdateCurrentLsystem(c, tu);
    }
  } else {
    if (prodPtr->object_flag == 1 && dr->output_type == TYPE_RAYSHADE) {
      storePtr = curPtr;

      if (rsStartObject(tu, depth, curPtr, prodPtr))
        return; /* return if pass two - the object is instantiated */
    }

    if (depth == 0 && currLsysPtr->Homomorphism.seed >= 0) {
      int i;

      /* store the current main_xsubi and set it to the homo_xsubi */
      /* thus the random numbers generators for the homomorphism are
      independent of the stream of random numbers used in normal
      productions*/
      for (i = 0; i < 3; i++) {
        store_xsubi[i] = main_xsubi[i];
        main_xsubi[i] = currLsysPtr->Homomorphism.xsubi[i];
      }
    }

    /* allocate the memory for the new string (as needed) */

    if (depth >= currLsysPtr->Homomorphism.stack_len) {
      currLsysPtr->Homomorphism.stack =
          currLsysPtr->Homomorphism.stack_len == 0
              ? Malloc(HomoStackIncrement * sizeof(char *))
              : Realloc(
                    currLsysPtr->Homomorphism.stack,
                    (currLsysPtr->Homomorphism.stack_len + HomoStackIncrement) *
                        sizeof(char *));
      if (currLsysPtr->Homomorphism.stack == NULL) {
        Message("Error: not enough memory for homomorphism stack!\n");
        MyExit(-1);
      }

      for (i = 0; i < HomoStackIncrement; i++)
        currLsysPtr->Homomorphism
            .stack[currLsysPtr->Homomorphism.stack_len + i] = NULL;

      currLsysPtr->Homomorphism.stack_len += HomoStackIncrement;
    }

    if (currLsysPtr->Homomorphism.stack[depth] == NULL) {
      currLsysPtr->Homomorphism.stack[depth] =
          Malloc(currLsysPtr->Homomorphism.longest_succ + 1);
      if (currLsysPtr->Homomorphism.stack[depth] == NULL) {
        Message("Error: not enough memory for homomorphism stack!\n");
        MyExit(-1);
      }
    }

    result = currLsysPtr->Homomorphism.stack[depth];

    endresult = result + prodPtr->succLen;
    ptr = result; /* ApplyProd would otherwise change result pointer */

    /* Apply the selected production (NULL - doesn't need pointers to the
string, it will not need to reallocate */
    ApplyProd(&curPtr, &ptr, NULL, NULL, prodPtr, NULL, 0);
    /* 0 - ignore possible decomposition */

    /* for productions with multiple-symbol predecessor */
    *str = curPtr;

    /* interpret the result */
    curPtr = result;
    *endresult = '\0';

    while (*curPtr != '\0') {
      /* extract next symbol and parameters if available */
      /* and process as appropriate                      */
      lastModule = curPtr;

      c = NextStringModule(&curPtr, &module);

#ifdef CONTEXT_SENSITIVE_HOMO
      InterpretHomomorphism(c, lastModule, &curPtr, leftcontext, rightcontext,
                            depth + 1, ProcessSymbol);
#else
      InterpretHomomorphism(c, lastModule, &curPtr, depth + 1, ProcessSymbol);
#endif
    }

    if (prodPtr->object_flag == 1 && dr->output_type == TYPE_RAYSHADE) {
      rsEndObject(tu, depth, storePtr, prodPtr);
    }

    if (depth == 0 && currLsysPtr->Homomorphism.seed >= 0) {
      int i;

      /* restore the current main_xsubi */
      /* thus the random numbers generators for the homomorphism are
      independent of the stream of random numbers used in normal
      productions*/
      for (i = 0; i < 3; i++) {
        currLsysPtr->Homomorphism.xsubi[i] = main_xsubi[i];
        main_xsubi[i] = store_xsubi[i];
      }
    }
  }
}

/*********************************************************************/
/* Performs 'counter' steps of interpretation. If counter is -1 at the
   beginning it does all interpretation.
   Returns 1, when interpretation is finished.
   */
int TurtleDrawNsteps(int counter) {
  int sub;
  char *curPtr;
  char aux;

  char *theString = NULL;
  if (selection_mode)
    theString = str;

  if (counter == -1) {
    counter = 1;
    sub = 0;
  } else
    sub = 1;

  /* Interpret the string module after module */
  /* If a non-zero parameter is available, use it rather than globals */

  for (;;) {
    /*
    curPtr = str; 
    if (selection_mode) {
      assert(NULL != theString);
      glLoadName((int)(curPtr - theString));
      printf("Name: %d\n",(int)(curPtr - theString));
    }
    */
    if (*str == '\0') {
      TurtleDrawFinishUp();
      return 1;
    }

    // stop interpreting if "interpretation step" reached (maybe there should be a message)
    if ((counter -= sub) < 0)
      return 0;

    /* extract next symbol and parameters if available */
    /* and process as appropriate                      */
    curPtr = str; /* for homomorphism: to remember the symbol to be replaced*/

    lastModule = str;
    
    c = NextStringModule(&str, &module);
    //   printf("Next string module: %c\n",c);

    aux = dr->env_output_symbol;
    
    if (selection_mode) {
      assert(NULL != theString);
      glLoadName((int)(curPtr - theString));
      //      printf("Name: %d\n",(int)(curPtr - theString));
    }
    
    if (currLsysPtr->Homomorphism.specified == 0)
      InterpretSymbol(c, &str, 0);
    else {
#ifdef CONTEXT_SENSITIVE_HOMO
      InterpretHomomorphism(c, curPtr, &str, curPtr - 1, str, 0,
                            InterpretSymbol);
#else
      InterpretHomomorphism(c, curPtr, &str, 0, InterpretSymbol);
#endif
    }

    /* set to 0 only if it was 1 before interpreting the module */
    if (aux)
      dr->env_output_symbol = 0;

  }
}

#ifdef CPFG_ENVIRONMENT
/*********************************************************************/
/* function provided to the communication routines, so the results coming form
   the environment can be set properly. */
int SetCommModulePars(unsigned long dist, Cmodule_type *comm_symbol,
                      __attribute__((unused)) int index) {
  char *dest;
  int i;

  if (comm_symbol->num_params > 0 && dist != 1) {
    /* if dist is one -> module was in homomorphism - 1 could not
                occur otherwise. The closest to 0 is 2 (?E?E) */
    /* pointer to the left brace of ?E */
    dest = first_comm_symbol_ptr + dist;

    for (i = 0; i < comm_symbol->num_params; i++) {
      if ((*dest == '(') || (*dest == ',')) {
        if (comm_symbol->params[i].set)
#ifdef WIN32
          memcpy(dest + 1, &comm_symbol->params[i].value, sizeof(float));
#else
          bcopy(&comm_symbol->params[i].value, dest + 1, sizeof(float));
#endif
      } else {
        Message("Warning: not enough room for %d parameters coming "
                "from the field.\n",
                comm_symbol->num_params);
        return 0;
      }
      dest += sizeof(float) + 1;
    }
  }
  return 1;
}
#endif

/*********************************************************************/
void TurtleDrawFinishUp(void) {
  int index;

  if (pending_interpretation) {
    if (dr->output_type != TYPE_ENVIRONMENT) {
      /* Mark end of last segment */
      ltdd.EndNode(tu, dr, vw, c);

      /* add code */ /* should close all branches, polygons, and sublsystems
      still open and give warning messages */

      if (dr->output_type != TYPE_VIEWVOLUME)
        DrawBackgroundScene(dr);

      ltdd.FinishUp(tu, dr, vw);
    } else {
#ifdef CPFG_ENVIRONMENT
      /* environmental pass */
      if (dr->env_output_symbol)
        ltdd.EndNode(tu, dr, vw, c);

      if (env_started) {
        Cmodule_type comm_module;
        unsigned long module_id;

        CMEndTransmission(LsystemList->current);
        env_started = 0;

        for (index = 0; index < CMGetNumberOfSlaves(); index++)
          while (CMGetCommunicationModule(index, &module_id, &comm_module))
            SetCommModulePars(module_id, &comm_module, index);
      }
#endif
    }

    pending_interpretation = 0;
  }
}

/*********************************************************************/
/*
        Function: TurtleDraw
        Draw the picture, using string to control the turtle, and
        rendering specific routines as specified by the turtleDrawDispatcher.
        The parameters have the following meaning:
        string - the string being interpreted
*/
void TurtleDraw(char *string, DRAWPARAM *drp, VIEWPARAM *vwp) {

  if (!TurtleDrawInitialize(string, drp, vwp))
    return;

  TurtleDrawNsteps(-1); /* till the end of the string */
}

/*********************************************************************/
void UpdateCurrentLsystem(char c, TURTLE *tu) {
  double tscale;
#ifdef JIM
  SymbolInstance *newInstance;
#endif

  switch (c) {
  case '?':
    /* ? Push current sub-L-system scale to stack and
    ** calculate new value. Sub-L-system id is the first
    ** and scale is the second parameter	*/
    /* check that the stack isn't full */
    if (subLstackPtr >= subLtop) {
      reallocateSubLStack(&subLstackPtr);
    }
    tscale = subLstackPtr->scale;
#ifndef JIM
    /* I've moved this 10 lines down so the current is always stored */
    /* store the pointer to the current L-system */
    subLstackPtr->LsysPtr = currLsysPtr;
#endif

    ++subLstackPtr;
    subLstackPtr->id = module.actual[0].value;

    /* get the pointer to the sub L-system */
    currLsysPtr = MatchingLsystem((int)module.actual[0].value, LsystemList);

#ifdef JIM
    /* I've moved this so the current is always stored right away JH 15 April*/
    /* store the pointer to the current L-system */
    subLstackPtr->LsysPtr = currLsysPtr;

    /* move to next instance and assign to symbols */
    newInstance = currLsysPtr->currentInstance->nextSymbolInstance;
    if (newInstance == NULL) {
      Message("INTERNAL ERROR: Instance not available for sub-Lsystem %d\n",
              (int)module.actual[0].value);
      MyExit(1);
    }
    currLsysPtr->currentInstance = newInstance;
    /* point symbol table variables at appropriate instance */
    AssignSymbolsToInstancesForInterpret(currLsysPtr->symbolTable, newInstance,
                                         subLstackPtr, subLbottom);
#endif
    /* If parameter for scale is present use it else default to 1 */
    if (module.parameters >= 2) {
      subLstackPtr->scale = tscale * module.actual[1].value;
    } else {
      subLstackPtr->scale = tscale;
    }
    /* modify appropriate turtle state variables */
    tu->scale_factor = subLstackPtr->scale;
    tu->line_width = tu->line_width * tu->scale_factor / tscale;
    break;

  case '$':
    tscale = subLstackPtr->scale;
    if (subLstackPtr <= subLbottom) {
      Message("Warning: unmatched $ ignored!\n");
      break;
    }
    --subLstackPtr;
    /* modify appropriate turtle state variables */
    tu->scale_factor = subLstackPtr->scale;
    tu->line_width = tu->line_width * tu->scale_factor / tscale;

    /* restore the pointer to the current L-system */
    currLsysPtr = subLstackPtr->LsysPtr;

    break;
  }
}

/*********************************************************************/
void InterpretSymbol(char c, char **str, char in_homo) {
  double temp[3];
  int i;
  double aux;
  float internodeLength;
  double angle; /* Value of first parameter in radians */
  char desired_surface;

  char *endptr;
  int x;
  float f;
  double *dptr;
  int count = 3;
  int new_realloc = 0;

  const float MinWidth = 0.0000001f;

  switch (c) {
    /* + Turn the turtle to the right by the angle (same plane). */

  case '+':
    if (module.parameters == 0)
      change_dir(temp, tu->heading, tu->left, dr->sinangle, dr->cosangle);
    else {
      angle = D_TO_R(module.actual[0].value);
      change_dir(temp, tu->heading, tu->left, sin(angle), cos(angle));
    }
    break;

    /* - Turn the turtle to the left by the angle (same plane).  */

  case '-':
    if (module.parameters == 0)
      change_dir(temp, tu->heading, tu->left, -dr->sinangle, dr->cosangle);
    else {
      angle = D_TO_R(module.actual[0].value);
      change_dir(temp, tu->heading, tu->left, -sin(angle), cos(angle));
    }
    break;

    /* | Turn the turtle around 180 degrees                      */

  case '|':
    change_dir(temp, tu->heading, tu->left, 0.0, -1.0);
    break;

    /* & Pitch the turtle down by the angle.                     */

  case '&':
    if (module.parameters == 0)
      change_dir(temp, tu->heading, tu->up, dr->sinangle, dr->cosangle);
    else {
      angle = D_TO_R(module.actual[0].value);
      change_dir(temp, tu->heading, tu->up, sin(angle), cos(angle));
    }
    break;

    /* ^ Pitch the turtle up by the angle.                       */

  case '^':
    if (module.parameters == 0)
      change_dir(temp, tu->heading, tu->up, -dr->sinangle, dr->cosangle);
    else {
      angle = D_TO_R(module.actual[0].value);
      change_dir(temp, tu->heading, tu->up, -sin(angle), cos(angle));
    }
    break;

    /* / Roll the turtle to the right by the angle (same plane).  */

  case '/':
    if (module.parameters == 0)
      change_dir(temp, tu->left, tu->up, dr->sinangle, dr->cosangle);
    else {
      angle = D_TO_R(module.actual[0].value);
      change_dir(temp, tu->left, tu->up, sin(angle), cos(angle));
    }
    break;

    /* \ Roll the turtle to the left by the angle (same plane).   */

  case '\\':
    if (module.parameters == 0)
      change_dir(temp, tu->left, tu->up, -dr->sinangle, dr->cosangle);
    else {
      angle = D_TO_R(module.actual[0].value);
      change_dir(temp, tu->left, tu->up, -sin(angle), cos(angle));
    }
    break;

    /* [ Save the current state of the turtle on a stack after		   */
    /*   passing the end of the internode to the rendering routines    */

  case '[':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      ltdd.EndNode(tu, dr, vw, c);
      if (!polyflag)
        env_finish_symbol = 0;
    }

    /* check that the stack isn't full */
    if (++turtlestackPtr >= turtletop) {
      reallocateTurtleStack(&turtlestackPtr);
      tu = turtlestackPtr - 1;
    }
    TurtleCopy(turtlestackPtr, tu);
    tu = turtlestackPtr;

    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol))
      ltdd.StartBranch(tu, dr, vw);

    PushSpline(tu);
    break;

    /* ] Return the turtle to the position and color index specified by   */
    /*   the top of the stack after                                       */
    /*   passing the end of the internode to the rendering routines    */

  case ']':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      ltdd.EndNode(tu, dr, vw, c);
      if (!polyflag)
        env_finish_symbol = 0;
    }

    /* check that the stack isn't empty */
    if (turtlestackPtr-- <= turtlebottom) {
      Message("Warning: unmatched ] ignored!\n");
      turtlestackPtr++;
      break;
    }

    /* update movements - if changed */
    for (i = 0; i < EPr.number_of_movements; i++)
      if ((aux = turtlestackPtr->movements[i].elasticity -
                 tu->movements[i].elasticity) != 0)
        update_movement((short)i, aux);

    tu = turtlestackPtr;

    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol))
      ltdd.EndBranch(tu, dr, vw);

    PopSpline(tu);
    break;

  case '{':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      ltdd.EndNode(tu, dr, vw, c);
      env_finish_symbol = 0;
    }

    if (module.parameters == 0 || module.actual[0].value == 0) {
      /* start a polygon */
      if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
        /*  Save the vertices to follow in a polygon stack so that they  */
        /*  may be filled with the color index when a '}' is encountered. */
        /*   Make sure the stack is not full first.                       */

        ++polystackPtr;

        /* check that the stack isn't full */
        if (polystackPtr >= polytop) {
          reallocatePolyStack(&polystackPtr);
          new_realloc = 1;
        }
        polystackPtr->edge_count = 0;
        polyflag = 1;
        /* initialize the new stack object */

        // [PASCAL] problem here if the interpretation is done during a rotation
        // or a zoom, the memory is reallocated ... First I think we should have
        // a reallocation only for concave polygons
        if ((interpret_new_model) || (new_realloc)) {
          polystackPtr->max_edges = 200;
          polystackPtr->vertex =
              (TURTLE *)Malloc(polystackPtr->max_edges * sizeof(TURTLE));
        }

        ltdd.StartPolygon(polystackPtr, tu, dr, vw);
        env_finish_symbol = 1;
      }
    }
#if CPFG_VERSION >= 3200
    else
        /* there is a parameter and it is not a zero */
        if (module.parameters >= 2)
      InitializeSpline(tu, (int)module.actual[0].value,
                       (int)module.actual[1].value);
    else
      InitializeSpline(tu, (int)module.actual[0].value, 0);
#endif

    break;

  case '}':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      if (module.parameters == 0 || module.actual[0].value == 0) {
        /* } Get the polygon array from the top of the stack and     */
        /*   fill the described polygon with the current color.      */

        /* check that the stack isn't empty */
        if (polystackPtr <= (polybottom - 1)) {
          Message("Warning: unmatched } ignored!\n");
          break;
        }

        ltdd.EndNode(tu, dr, vw, c);
        ltdd.EndPolygon(polystackPtr, tu, dr, vw);

        --polystackPtr;
        if (polystackPtr == (polybottom - 1))
          polyflag = 0;

        env_finish_symbol = 0;
      }
#if CPFG_VERSION >= 3200
      else
        /* there is a parameter and it is not a zero */
        /* but the parameter is ignored */
        FinishSpline(tu, dr /*, module.actual[0].value*/);
#endif
    }
    break;

    /* . Put current position of the turtle on the polystack. */

  case '.':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      if (polyflag) {
        TurtleCopy(&(polystackPtr->vertex[polystackPtr->edge_count]), tu);
        /* check that there aren't too many polygon edges */
        if (++(polystackPtr->edge_count) >= polystackPtr->max_edges) {
          TURTLE *newVertices = NULL;

          newVertices =
              (TURTLE *)Realloc(polystackPtr->vertex,
                                2 * polystackPtr->max_edges * sizeof(TURTLE));
          if (newVertices) {
            polystackPtr->max_edges *= 2;
            polystackPtr->vertex = newVertices;
          } else {
            Message("Out of memory: Could not allocate more space for polygon "
                    "points.\n");
            MyExit(1);
          }
        }
      }
#if CPFG_VERSION >= 3200
      else if (module.parameters == 0)
        SetControlPoint(tu, dr, 0);
      else
        SetControlPoint(tu, dr, module.actual[0].value);
#endif
    }
    break;
    /* ? is handled together with the environmental stuff bellow */

    /* $ Pop sub-L-system scale from stack */

  case '$':
    UpdateCurrentLsystem(c, tu);
    break;

    /* F and G  Create a new segment.                                */
    /*          Mark end of old and start of new, then move turtle;  */
    /*          add to current polygon if necessary, and adjust     */
    /*          for movements                                         */

  case 'F':
  case 'G':

    /* Adjust for movements */
    movement_adjust(tu);

    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      ltdd.EndNode(tu, dr, vw, c);
      if (!polyflag)
        env_finish_symbol = 0;
    }

    /* determine length of segment */
    if (module.parameters == 0) {
      internodeLength = dr->step_size * tu->scale_factor;
    } else {
      internodeLength = module.actual[0].value * tu->scale_factor;
    }

    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      ltdd.StartNode(tu, dr, vw, internodeLength, c);
      env_finish_symbol = 1;
    }

    /* adjust position */
    tu->position[0] += internodeLength * tu->heading[0];
    tu->position[1] += internodeLength * tu->heading[1];
    tu->position[2] += internodeLength * tu->heading[2];

    /* Adjust for old tropisms */
    bend(dr, tu);

    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      /* textures */
      tu->tex_t +=
          update_segment_texture(tu->texture, tu->line_width, internodeLength);

      /* If the position is part of a polygon, save it in the */
      /* array at the top of the polygon stack for F symbol.  */
      if (polyflag && c == 'F') {
        TurtleCopy(&(polystackPtr->vertex[polystackPtr->edge_count]), tu);
        /* check that there aren't too many polygon edges */
        if (++(polystackPtr->edge_count) >= polystackPtr->max_edges) {
          TURTLE *newVertices = NULL;
          newVertices =
              (TURTLE *)Realloc(polystackPtr->vertex,
                                2 * polystackPtr->max_edges * sizeof(TURTLE));

          if (newVertices) {
            polystackPtr->max_edges *= 2;
            polystackPtr->vertex = newVertices;
          } else {
            Message("Out of memory: Could not allocate more space for polygon "
                    "points.\n");
            MyExit(1);
          }
        }
      }

#if CPFG_VERSION >= 3200
      if (tu->spline_flag == 1 && c == 'F')
        SetControlPoint(tu, dr, 0);
#endif
    }

    tu->prev_heading[0] = tu->heading[0];
    tu->prev_heading[1] = tu->heading[1];
    tu->prev_heading[2] = tu->heading[2];
    tu->prev_up[0] = tu->up[0];
    tu->prev_up[1] = tu->up[1];
    tu->prev_up[2] = tu->up[2];
    break;

    /* f and g  move the length of a new segment without drawing.    */
    /*          Mark end of old segment, then move turtle;           */
    /*          add to current polygon if necessary, and adjust      */
    /*          for movements                                         */

  case 'f':
  case 'g':

    /* Adjust for movements */
    movement_adjust(tu);
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      ltdd.EndNode(tu, dr, vw, c);
      if (!polyflag)
        env_finish_symbol = 0;
    }

    /* determine length of segment */
    if (module.parameters == 0) {
      internodeLength = dr->step_size * tu->scale_factor;
    } else {
      internodeLength = module.actual[0].value * tu->scale_factor;
    }

    /* adjust position */
    tu->position[0] += internodeLength * tu->heading[0];
    tu->position[1] += internodeLength * tu->heading[1];
    tu->position[2] += internodeLength * tu->heading[2];

    /* Adjust for tropisms */
    bend(dr, tu);

    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      /* textures */
      tu->tex_t +=
          update_segment_texture(tu->texture, tu->line_width, internodeLength);

      /* add code */ /* if special surface is specified determine end ??*/
      /* or assume same length ??  same for F&G?? */
      /* if (internodeLength != 0.0) { */
      /*determine_end_point(tu, dr->line_type, internodeLength); }*/

      /* If the position is part of a polygon, save it in the */
      /* array at the top of the polygon stack for f symbol.  */
      if (polyflag && c == 'f') {
        TurtleCopy(&(polystackPtr->vertex[polystackPtr->edge_count]), tu);
        /* check that there aren't too many polygon edges */
        if (++(polystackPtr->edge_count) >= polystackPtr->max_edges) {
          TURTLE *newVertices = NULL;
          newVertices =
              (TURTLE *)Realloc(polystackPtr->vertex,
                                2 * polystackPtr->max_edges * sizeof(TURTLE));
          if (newVertices) {
            polystackPtr->max_edges *= 2;
            polystackPtr->vertex = newVertices;
          } else {
            Message("Out of memory: Could not allocate more space for polygon "
                    "points.\n");
            MyExit(1);
          }
        }
      }

#if CPFG_VERSION >= 3200
      if (tu->spline_flag == 1 && c == 'f')
        SetControlPoint(tu, dr, 0);
#endif
    }
    break;

    /* ,  Assign the parameter value(s) to color if present or */
    /*    Decrease the current color index by the color increment. */

  case ',':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      if (module.parameters == 0)
        tu->color_index -= dr->color_increment;
      else
        tu->color_index = clp.colormap + (int)module.actual[0].value;
      if (tu->color_index < clp.colormap)
        tu->color_index += 256;
      ltdd.SetColour(tu, dr, vw);
    }
    break;

    /* ;  Assign the parameter value(s) to color if present or */
    /*    Increase the current color index by the color increment. */

  case ';':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      if (module.parameters == 0) {
        tu->color_index += dr->color_increment;
      } else {
        tu->color_index = clp.colormap + (int)module.actual[0].value;
      }
      if (tu->color_index > clp.colormap + 255) {
        tu->color_index -= 256;
      }
      ltdd.SetColour(tu, dr, vw);
    }
    break;

  case '%':
    if (in_homo) /* ignored in homomorphism */
      break;

    if (module.parameters == 0) {
      /* normal cut symbol */
      if (!dr->interpretation_past_cut) {
        SkipCutPart(str);
        break;
      }
    }

    if (module.parameters == 2)
      *tu = substr_turtle[(int)module.actual[1].value];

    if (dr->output_type == TYPE_ENVIRONMENT) {
      if (module.parameters == 1) {
        /* remember the beginning and the end of the substring together with */
        /* the turtle */
        if (num_cut_substrings == substr_array_size) {
          substr_array_size += SUBSTR_INITIALSIZE;
          cut_substrings = (SUBSTRING *)Realloc(
              cut_substrings, substr_array_size * sizeof(SUBSTRING));
          Message("Cut substring array reallocated.\n");

          if (cut_substrings == NULL)
            Warning("Not enough memory to store cut substrings", INTERNAL_LVL);
        }

        if (num_substr_turtle == substr_turtle_size) {
          substr_turtle_size += SUBSTR_INITIALSIZE;
          substr_turtle = (TURTLE *)Realloc(substr_turtle, substr_turtle_size *
                                                               sizeof(TURTLE));
          Message("Cut substring turtle array reallocated.\n");

          if (substr_turtle == NULL)
            Warning("Not enough memory to store turtle for cut substrings",
                    INTERNAL_LVL);
        }

        substr_turtle[num_substr_turtle] = *tu;

        cut_substrings[num_cut_substrings].cut_parameter =
            module.actual[0].value;

        /* place num_cut_substrings (integer!) as the symbols parameter */
#ifdef WIN32
        memcpy(lastModule + 2, &num_cut_substrings, PARAMSIZE);
#else
        bcopy(&num_cut_substrings, lastModule + 2, PARAMSIZE);
#endif

        cut_substrings[num_cut_substrings++].turtle_index =
            (float)(num_substr_turtle++);
      }
    }

    break;

    /* !  Decrease the current line width by the line width increment       */
    /*    or assign the parameter value if present */

  case '!':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      if (module.parameters == 0)
        tu->line_width -= dr->width_increment * tu->scale_factor;
      else
        tu->line_width = (float)(module.actual[0].value * tu->scale_factor);

      if (tu->line_width < MinWidth)
        tu->line_width = MinWidth;

      ltdd.SetLineWidth(tu, dr, vw);
    }
    break;

    /* #  Increase the current line width by the line width increment       */
    /*    or assign the parameter value if present */

  case '#':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      if (module.parameters == 0)
        tu->line_width += dr->width_increment * tu->scale_factor;
      else
        tu->line_width = (float)(module.actual[0].value * tu->scale_factor);

      if (tu->line_width < MinWidth)
        tu->line_width = MinWidth;

      ltdd.SetLineWidth(tu, dr, vw);
    }
    break;

#ifdef OLDTROPISMS
    /* _  Increase the current elasticity by the elasticity increment       */
    /*    or assign the parameter value if present */

  case '_':
    if (module.parameters == 0)
      tu->elasticity += dr->elasticity_increment;
    else
      tu->elasticity = (float)module.actual[0].value;
    break;
#endif

    /* ~  Draw a surface at the current position after marking segment end */
  case '~':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      ltdd.EndNode(tu, dr, vw, c);
      env_finish_symbol = 0;
    }

    /* Determine desired surface for further processing */
    desired_surface = NextStringModule(str, &module);

    /* if end of string reset for normal loop termination */
    if (desired_surface == '\0') {
      (*str)--;
    } else {
      /* allow non-uniform scaling - lars 18 april 2000 */
      /* if no parameter, scale by 1 */
      if (module.parameters == 0) {
        module.actual[0].value = 1.0;
        module.actual[1].value = 1.0;
        module.actual[2].value = 1.0;
      }
      /* if less than 3 parameters, scale uniform */
      else if (module.parameters < 3) {
        module.actual[1].value = module.actual[0].value;
        module.actual[2].value = module.actual[0].value;
      }

      if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol))
        ltdd.PredefinedSurface(tu, dr, vw, desired_surface,
                               module.actual[0].value * tu->scale_factor,
                               module.actual[1].value * tu->scale_factor,
                               module.actual[2].value * tu->scale_factor);
      determine_end_point(tu, desired_surface,
                          module.actual[0].value * tu->scale_factor);
    }
    break;

    /* Call blackbox functions after marking segment end.               */
    /* Pass str to allow black box to 			                */
    /* determine function to be called. Str is returned shortened by 	*/
    /* symbols used to determine what function to apply. tu and dr are 	*/
    /* passed to give access to the current environment.	     	*/

  case '@':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (env_finish_symbol)) {
      ltdd.EndNode(tu, dr, vw, c);
      env_finish_symbol = 0;
    }

    call_blackbox(str, tu, dr, vw);
    break;

    /* Environmental interaction stuff */
  case 's': /* backward compatibility */
    if (dr->output_type == TYPE_ENVIRONMENT)
      if ((module.parameters == 2) || (module.parameters == 3)) {
        lastModule += 2;
        for (x = 0; x < module.parameters; x++) {
          f = tu->position[x];
#ifdef WIN32
          memcpy(lastModule, &f, PARAMSIZE);
#else
          bcopy(&f, lastModule, PARAMSIZE);
#endif
          lastModule += SKIPSIZE;
        }
      }
    break;
  case '?':
    if (module.parameters >= 1) {
      UpdateCurrentLsystem(c, tu);
    } else {
      count = 3;
      lastModule = *str;
      c = NextStringModule(str, &module);

#ifdef CPFG_ENVIRONMENT
      if (dr->output_type == TYPE_ENVIRONMENT) {
        if (c == 'E') {
          /* ?E is processed even in homomorphism, but
                  the distance is -1 -> no response can be accepted */
#if CPFG_VERSION < 3200
          if (clp.efilename != NULL) {
#else
          if (CMGetNumberOfSlaves() > 0) {
#endif
            /* field communication */
            Cmodule_type two_modules[2], comm_module;
            unsigned long module_id;
            CTURTLE cturtle;

            /* communication symbol */
            strcpy(two_modules[0].symbol, "E");

            two_modules[0].num_params = module.parameters;
            for (i = 0; i < module.parameters; i++) {
              two_modules[0].params[i].value = module.actual[i].value;
              two_modules[0].params[i].set = 1;
            }

            if (first_comm_symbol && !in_homo) {
              first_comm_symbol = 0;
              first_comm_symbol_ptr = lastModule + 1;
            }

            endptr = *str;

            /* get the next symbol */
            if (*endptr == 0) {
              /* end of the string reached */
              two_modules[1].symbol[0] = '\0';
              two_modules[1].num_params = 0;
            } else {
              c = NextStringModule(&endptr, &module);
              two_modules[1].symbol[0] = c;

              if ((c == '~') || (c == '@')) {
                /* double symbols */
                /* more work necessary for @Gs, @Gc, etc. */
                c = NextStringModule(&endptr, &module);
                two_modules[1].symbol[1] = c;
                two_modules[1].symbol[2] = '\0';
              } else
                two_modules[1].symbol[1] = '\0';

              two_modules[1].num_params = module.parameters;
              for (i = 0; i < module.parameters; i++) {
                two_modules[1].params[i].value = module.actual[i].value;
                two_modules[1].params[i].set = 1;
              }
            }

            /* copy the turtle */
            for (i = 0; i < 3; i++) {
              cturtle.position[i] = tu->position[i];
              cturtle.heading[i] = tu->heading[i];
              cturtle.up[i] = tu->up[i];
              cturtle.left[i] = tu->left[i];
            }
            cturtle.line_width = tu->line_width;
            cturtle.scale_factor = tu->scale_factor;

            if (!env_started) {
              env_started = 1;

              CMBeginTransmission();
            }

            /* returns one if the graphics of the following symbol should
            be output */
            dr->env_output_symbol = CMSendCommSymbol(
                0, in_homo ? 1 : lastModule + 1 - first_comm_symbol_ptr,
                two_modules, &cturtle);

            while (CMGetCommunicationModule(0, &module_id, &comm_module))
              SetCommModulePars(module_id, &comm_module, 0);

            return; /* so env_output_symbol is not set to 0 */
          }
          break;
        }
      }
#endif

      /* query symbols */
      switch (c) {
      case 'P':
        dptr = tu->position;
        break;
      case 'H':
        dptr = tu->heading;
        break;
      case 'L':
        dptr = tu->left;
        break;
      case 'U':
        dptr = tu->up;
        break;
      case 'Q': {
        double mat[4][4];
        double q[4];

        for (x = 0; x < 3; x++) {
          mat[0][x] = tu->heading[x];
          mat[1][x] = tu->left[x];
          mat[2][x] = tu->up[x];
          mat[3][x] = 0.0;
          mat[x][3] = 0.0;
        }
        mat[3][3] = 0.0;

        matrix_to_qt(q, mat);
        dptr = q;
        count = 4;
        break;
      }
      default:
        if (c != 'E')
          Message("Warning: unknown query '?%c'\n", c);
        goto out;
      }
      lastModule += 2;
      if (module.parameters < count)
        count = module.parameters;
      for (x = 0; x < count; x++) {
        f = *(dptr++);
#ifdef WIN32
        memcpy(lastModule, &f, PARAMSIZE);
#else
        bcopy(&f, lastModule, PARAMSIZE);
#endif
        lastModule += SKIPSIZE;
      }
    out:;
    }

    break;

    /* Default.                                                         */

  default:
    break;
  }
}

void TurtleCopy(TURTLE *to, const TURTLE *from) { *to = *from; }

/********************************************************************/
/* Function: change_dir                                             */
/* Change_dir is used to change the direction of the turtle by      */
/* rotating it either aroung the heading direction (roll right/left)*/
/* or around the left vector (pitch up/down), or around the up      */
/* vector (turn right/left).                                        */
/********************************************************************/

static void change_dir(double *vect1, double *vect2, double *vect3,
                       double sinangle, double cosangle) {
  TurtleRotate(vect1, vect2, vect3, sinangle, cosangle);
  TurtleRotate(vect3, vect3, vect2, -sinangle, cosangle);
  vect2[0] = vect1[0];
  vect2[1] = vect1[1];
  vect2[2] = vect1[2];
}

/********************************************************************/
/* Function: TurtleRotate                                           */
/* Rotate is used to rotate the given vectors around each other by  */
/* the angle (using sine and cosine of the angle) so that the       */
/* turtle can turn, pitch, and roll.                                */
/********************************************************************/

static void TurtleRotate(double *temp, const double *vector,
                         const double *perpvect, double sinangle,
                         double cosangle) {
  temp[0] = cosangle * vector[0] + sinangle * perpvect[0];
  temp[1] = cosangle * vector[1] + sinangle * perpvect[1];
  temp[2] = cosangle * vector[2] + sinangle * perpvect[2];
}

/*
        Initialize drawing and viewing structures.
*/

static void InitializeParameters(DRAWPARAM *drawPtr, VIEWPARAM *viewPtr) {
  drawPtr->angle_increment = 45.0;
  drawPtr->line_width = 1;
  drawPtr->width_increment = 0;
  viewPtr->viewpoint[0] = 0.0;
  viewPtr->viewpoint[1] = 0.0;
  viewPtr->viewpoint[2] = 1.0;
  viewPtr->vrp[0] = 0.0;
  viewPtr->vrp[1] = 0.0;
  viewPtr->vrp[2] = 0.0;
  viewPtr->viewing_angle = 45.0;
  viewPtr->twist = 0;
  viewPtr->parallel_projection_on = 1;
  viewPtr->front_dist = -100000.;
  viewPtr->back_dist = 100000.;
  viewPtr->initscale = viewPtr->scale = 1.0;
  viewPtr->z_buffer_on = 1;
  viewPtr->auto_window = 1;
  viewPtr->min[0] = 0.0;
  viewPtr->min[1] = 0.0;
  viewPtr->min[2] = 0.0;
  viewPtr->max[0] = 1.0;
  viewPtr->max[1] = 1.0;
  viewPtr->max[2] = 1.0;

  viewPtr->objectScale[0] = 1.0;
  viewPtr->objectScale[1] = 1.0;
  viewPtr->objectScale[2] = 1.0;
  viewPtr->objects_include_turtle = 0;
  strcpy(viewPtr->objects_format, "%g");
#if CPFG_VERSION >= 6500
  viewPtr->antialiasing_on = 0;
#endif
  viewPtr->zoom_min = 0.05f;
  viewPtr->zoom_max = 50.0f;
  viewPtr->GenerateOnViewChange = eOff;

  drawPtr->cue_range = 0;

  /* old */
  drawPtr->shade_mode = SIMPLE_FILL;
  drawPtr->line_type = '~';
  /* new */
  drawPtr->render_mode = RM_FILLED;
  drawPtr->line_style = LS_PIXEL;
  drawPtr->tapered_lines_on = 1;

  drawPtr->light_dir[0] = 1.0;
  drawPtr->light_dir[1] = 0.0;
  drawPtr->light_dir[2] = 0.0;
  drawPtr->diff_coef = 10;
#ifdef OLDTROPISMS
  drawPtr->tropism_dir[0] = 0.0;
  drawPtr->tropism_dir[1] = 1.0;
  drawPtr->tropism_dir[2] = 0.0;
  drawPtr->elasticity = 0.0;
  drawPtr->elasticity_increment = 0.0;
#endif
  drawPtr->ambient = 0.15f;
  drawPtr->diffuse = 0.75;
  drawPtr->step_size = STEP_SIZE;
  drawPtr->scale_factor = 1.0;
  drawPtr->scale_multiplier = 1.0;
  drawPtr->color_index = 128;
  drawPtr->color_index_back = 128;
  // [PASCAL] bug list from 2016-01-25 => set increment color to 1
  drawPtr->color_increment = 1;
  drawPtr->color_increment_back = 1;
  drawPtr->double_sided = 0;
  strcpy(drawPtr->fontname, "-*-courier-bold-r-*-*-12-*-*-*-*-*-*-*");
  strcpy(drawPtr->winfont.Name, "Arial");
  drawPtr->winfont.size = 12;
  drawPtr->winfont.bold = 0;
  drawPtr->winfont.italic = 0;
  drawPtr->cylinder_sides = 8;
  drawPtr->gen_cyl_twist = 0;
  drawPtr->interpretation_step = -1;
  drawPtr->interpretation_past_cut = 1;
  drawPtr->rayshade_scale = 1.0;
  drawPtr->environment_display = 0;

  // MC - Oct. 2015 - shadow map parameters
  drawPtr->shadow_map_size = 1024;
  drawPtr->shadow_color[0] = 0.2f;
  drawPtr->shadow_color[1] = 0.2f;
  drawPtr->shadow_color[2] = 0.4f;
  drawPtr->shadow_offset[0] = 5.f;
  drawPtr->shadow_offset[1] = 10.f;
  drawPtr->ps_linecap = 1;
}

/*
        Move to next label in input stream
*/

static char *NextLabel(FILE *fp, int *lineNumber) {
  static char inputBuffer[160];
  char *inputPtr;
  unsigned char inputChar;
  int comment;
  int endFile;
  int labelFound;
  int labelStarted;

  inputPtr = inputBuffer;
  endFile = 0;
  comment = 0;
  labelFound = 0;
  labelStarted = 0;

  while (!endFile && !labelFound) {

    inputChar = getc(fp);
    switch (inputChar) {
    case '/':
      inputChar = getc(fp);
      if (inputChar == '*') {
        comment = 1;
      } else {
        ungetc(inputChar, fp);
        inputChar = '/';
      }
      break;
    case '*':
      if (comment) {
        inputChar = getc(fp);
        if (inputChar == '/') {
          comment = 0;
          inputChar = ' ';
        } else {
          ungetc(inputChar, fp);
          inputChar = '*';
        }
      }
      break;
    case '#':
      if (!comment) {
        labelFound = 1;
      }
      break;
    case ':':
      if (!comment) {
        labelFound = 1;
      }
      break;
    case '\n':
      if (labelStarted) {
        labelFound = 1;
        ungetc(inputChar, fp);
      } else {
        (*lineNumber)++;
      }
      inputChar = ' ';
      break;
    case 0xff:
      if (labelStarted) {
        labelFound = 1;
        ungetc(inputChar, fp);
      } else {
        endFile = 1;
      }
      inputChar = ' ';
      break;
    default:
      break;
    }
    if (!comment) {
      if (labelStarted || inputChar != ' ') {
        labelStarted = 1;
        *inputPtr = inputChar;
        inputPtr++;
      }
    }
  }

  /* mark end of label */
  *inputPtr = '\0';

  /* check for label available */
  if (labelStarted)
    return inputBuffer;
  else
    return NULL;
}

/*
        Check for input errors
        if found reset stream pointer to end of line
*/

static void CheckInputError(FILE *fp, int inputFlag, int inputCount,
                            char *labelPtr, int lineNumber, char *fileName) {
  unsigned char inputChar;

  if (inputFlag == inputCount)
    return;

  /* check for unknown label */
  if (inputCount == 0) {
    Message("Unknown label %s on line %d of %s\n", labelPtr, lineNumber,
            fileName);
  }
  /* otherwise there are invalid parameters */
  else {
    Message("Invalid parameters for %s on line %d of %s\n", labelPtr,
            lineNumber, fileName);
  }

  if (inputFlag == EOF) {
    ungetc(0xff, fp);
    return;
  }

  /* skip to end of line or file, whichever comes first */
  do {
    inputChar = getc(fp);
  } while (inputChar != '\n' && inputChar != 0xff);
  ungetc(inputChar, fp);
}

/* keywords for render mode */
char *rm_words[RM_COUNT] = {"wireframe", "filled", "flat",   "interpolated",
                            "shaded",    "fast",   "shadows"};

/* keywords for line style */
char *ls_words[LS_COUNT] = {"pixel", "polygon", "cylinder"};

/*
        Read input file filename into viewing structures.
*/

#define READONENUMBER(var, format)                                             \
  inputFlag = fscanf(fp, format, &var);                                        \
  CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);        \
  VERBOSE("%s %g\n", *(labelPtr - 1), (float)var);

int ReadViewData(char *filename, DRAWPARAM *drawPtr, VIEWPARAM *viewPtr) {
  FILE *fp;
  static char *labels[] = {
    "angle factor:",          /*  0 */
    "angle increment:",       /*  1 */
    "initial color:",         /*  2 */
    "color increment:",       /*  3 */
    "initial line width:",    /*  4 */
    "line width increment:",  /*  5 */
    "viewpoint:",             /*  6 */
    "view reference point:",  /*  7 */
    "twist:",                 /*  8 */
    "projection:",            /*  9 */
    "front distance:",        /* 10 */
    "back distance:",         /* 11 */
    "scale factor:",          /* 12 */
    "z buffer:",              /* 13 */
    "cue range:",             /* 14 */
    "shade mode:",            /* 15 */
    "light direction:",       /* 16 */
    "diffuse reflection:",    /* 17 */
    "surface ambient:",       /* 18 */
    "surface diffuse:",       /* 19 */
    "line:",                  /* 20 */
    "surface:",               /* 21 */
    "box:",                   /* 22 */
    "#",                      /* 23 */
    "background color:",      /* 24 */
    "ambient light:",         /* 25 */
    "font:",                  /* 26 */
    "polygonization level:",  /* 27 */
    "tropism direction:",     /* 28 */
    "initial elasticity:",    /* 29 */
    "elasticity increment:",  /* 30 */
    "blob:",                  /* 31 */
    "tropism:",               /* 32 */
    "torque:",                /* 33 */
    "light:",                 /* 34 */
    "texture:",               /* 35 */
    "render mode:",           /* 36 */
    "line style:",            /* 37 */
    "tapered lines:",         /* 38 */
    "contour sides:",         /* 39 */
    "contour:",               /* 40 */
    "viewing angle:",         /* 41 */
    "background scene:",      /* 42 */
    "twist of cylinders:",    /* 43 */
    "interpretation step:",   /* 44 */
    "interpretation past %:", /* 45 */
    "tsurface:",              /* 46 */
    "rayshade objects:",      /* 47 */
    "rayshade scale:",        /* 48 */
    "initial scale:",         /* 49 */
    "scale multiplier:",      /* 50 */
    "environment display:",   /* 51 */
#if CPFG_VERSION >= 4000
    "function:", /* 52 */
#endif
#if CPFG_VERSION >= 6400
    "curve:", /* 53 */
#endif
#if CPFG_VERSION >= 6500
    "antialiasing:", /* 54 */
    "function set:", /* 55 */
#endif
    "min zoom:",                /* 56 */
    "max zoom:",                /* 57 */
    "generate on view change:", /* 58 */
    "winfont:",                 /* 59 */
    "concave polygons:",        /* 60 */
    "shadow map:",              /* 61 */
    "PS linecap:",              /* 62 */
    NULL                        /* the last one must be NULL */
  };
  char *inputPtr, **labelPtr;
  char input_string[255];
  float temp_x, temp_y, temp_z, temp_length;
  char surface_id;
  char surface_filename[255];
#define TMPFILELEN 2048
  char tmpfile[TMPFILELEN]; /* temporary file name */
  char includeFile[255];    /* include file name */
  float surface_size;
  int s_precision;
  int s_texture;
  int t_precision;
  int id;
#define MaxLineLength 255
  char input_line[MaxLineLength];
  char input_line_tmp[MaxLineLength];
  double v1[3], v2[3];
  int lineNumber;
  int labelIndex;
  int inputFlag;
  double angle;
  float distance;
  float red, green, blue;
  int i;
  int count = 0;
  extern int gl_numlights;   /* number of lights - from irisGL.c */
  extern int gl_numtextures; /* number of textures */
  static VIEWPARAM DummyVp;
  static DRAWPARAM DummyDp;

  fpos_t filepos;

  char buffer[1024]; /* buffer for creating system call */
  char scenefile[TMPFILELEN]; /* temporary file name */

  if (NULL == drawPtr)
    drawPtr = &DummyDp;
  if (NULL == viewPtr)
    viewPtr = &DummyVp;

  InitializeBackgroundScene();

  FreeViewFileData();

  InitializeSurfaces();
  InitializeTsurfaces();

  InitializeSplines();

  InitializeParameters(drawPtr, viewPtr);

  gl_numlights = 0;
  gl_numtextures = 0;

  for (i = 0; i < 3; i++)
    viewPtr->objectRotation[i] = 0;
// [PASCAL]
// open the file twice to check if the size is consistent
#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
  // get size of file
  fp = fopen(filename, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((fp == NULL) && (counter < 1000)) {
    fp = fopen(filename, "r");
    counter++;
  }
  if (counter == 1000)
    fprintf(stderr, "WARNING (interpret.c): Can't open the view file %s - using defaults.\n",
            filename);
  else {
    fseek(fp, 0, SEEK_END); // seek to end of file
    size = ftell(fp);       // get current file pointer
    fclose(fp);

    while (current_size != size) {
      current_size = size;
      fp = fopen(filename, "r");
      while (fp == NULL) {
        fp = fopen(filename, "r");
        counter++;
      }
      fseek(fp, 0, SEEK_END); // seek to end of file
      size = ftell(fp);       // get current file pointer
      fclose(fp);
    }
  }
#endif

  if ((fp = fopen(filename, "r")) == NULL) {
    Message("WARNING (interpret.c -2 : Can't open the view file %s - using defaults.\n",
           filename);
  } else {
    fclose(fp);

    /* Run input file through C preprocessor */
    VERBOSE("Preprocessing view file %s.\n", filename);

    /* open preprocessed file */
    if ((fp = PreprocessFile(filename, "")) == NULL) {
      Message("Can't preprocess view file %s\n", filename);
      MyExit(1);
    }

    VERBOSE("Reading File: %s.\n", filename);

    /* set up for error messages */
    strcpy(includeFile, filename);
    lineNumber = 1;

    /* loop over input lines */
    while ((inputPtr = NextLabel(fp, &lineNumber)) != NULL) {
      /*
      determine parameter type - index,
      when found, is used in switch
              */
      labelPtr = labels;
      for (labelIndex = 0; *labelPtr != NULL; labelIndex++) {
        if (strcmp(inputPtr, *(labelPtr++)) == 0)
          break;
      }

      /* process accordingly */
      switch (labelIndex) {
      case 0: /* angle factor */
        READONENUMBER(drawPtr->angle_increment, " %f");
        drawPtr->angle_increment = 360.0 / drawPtr->angle_increment;
        break;
      case 1: /* angle increment */
        READONENUMBER(drawPtr->angle_increment, " %f");
        break;
      case 2: /* initial colour */
        READONENUMBER(drawPtr->color_index, " %d");
        if (fscanf(fp, " %d", &drawPtr->color_index_back) == 1)
          drawPtr->double_sided = 1;
        break;
      case 3: /* color increment */
        READONENUMBER(drawPtr->color_increment, " %d");
        fscanf(fp, " %d", &drawPtr->color_increment_back);
        break;
      case 4: /* initial line width */
      {
        char input_char;

        inputFlag = fscanf(fp, " %f%c", &drawPtr->line_width, &input_char);
        CheckInputError(fp, inputFlag, 2, inputPtr, lineNumber, includeFile);
        drawPtr->line_style = LS_POLYGON;
        while (input_char == ' ') {
          input_char = getc(fp);
        }
        if (input_char != '\n') {
          ungetc(input_char, fp);
          /* Check for "pnts" here */
          inputFlag = fscanf(fp, "%s", input_string);
          if (inputFlag) {
            if (input_string[0] == 'p') {
              drawPtr->line_style = LS_PIXEL;
            } else if (input_string[0] == 's') {
              drawPtr->line_style = LS_CYLINDER;
            }
          }
        }
        if (clp.verbose) {
          Message("initial line width: %g", drawPtr->line_width);
          if (drawPtr->line_style == LS_PIXEL)
            Message(" pixels\n");
          else if (drawPtr->line_style == LS_CYLINDER)
            Message(" cylinder\n");
        }
        break;
      }
      case 5: /* line width increment */
        READONENUMBER(drawPtr->width_increment, " %f");
        break;
      case 6: /* viewpoint */
        inputFlag = fscanf(fp, " %f,%f,%f", &viewPtr->viewpoint[0],
                           &viewPtr->viewpoint[1], &viewPtr->viewpoint[2]);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);
        VERBOSE("viewpoint: %g,%g,%g\n", viewPtr->viewpoint[0],
                viewPtr->viewpoint[1], viewPtr->viewpoint[2]);
        break;
      case 7: /* view reference point */
        inputFlag = fscanf(fp, " %f,%f,%f", &viewPtr->vrp[0], &viewPtr->vrp[1],
                           &viewPtr->vrp[2]);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);
        VERBOSE("view reference point: %g,%g,%g\n", viewPtr->vrp[0],
                viewPtr->vrp[1], viewPtr->vrp[2]);
        break;
      case 8: /* twist */
        READONENUMBER(viewPtr->twist, " %d");
        break;
      case 9: {
        /* projection */
        char *parallel = "parallel";
        char *perspect = "perspective";
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        VERBOSE("projection: %s\n", input_string);
        if (strncmp(input_string, parallel, strlen(parallel)) == 0)
          viewPtr->parallel_projection_on = 1;
        else if (strncmp(input_string, perspect, strlen(perspect)) == 0)
          viewPtr->parallel_projection_on = 0;
        else {
          Message("WARNING: unknown projection - %s in line %d", input_string,
                  lineNumber);
          Message(" of %s; Parallel assumed.\n", includeFile);
          viewPtr->parallel_projection_on = 1;
        }
        break;
      }
      case 10: /* front distance */
        READONENUMBER(viewPtr->front_dist, " %f");
        break;
      case 11: /* back distance */
        READONENUMBER(viewPtr->back_dist, " %f");
        break;
      case 12: /* scale factor */
        READONENUMBER(viewPtr->scale, " %f");
        if (viewPtr->scale == 0.0)
          viewPtr->scale = 1.0;
        viewPtr->initscale = viewPtr->scale;
        break;
      case 13: /* z buffer */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (strncmp(input_string, "on", 2) == 0) {
          viewPtr->z_buffer_on = 1;
          VERBOSE("z buffer: on\n");
        } else {
          viewPtr->z_buffer_on = 0;
          VERBOSE("z buffer: off\n");
        }
        break;
      case 14: /* cue range */
        READONENUMBER(drawPtr->cue_range, " %d");
        break;
      case 15: /* shade mode */
      {
        READONENUMBER(drawPtr->shade_mode, " %d");

        /* conversion into a new render mode */
        switch (drawPtr->shade_mode) {
        case SIMPLE_FILL:
          drawPtr->render_mode = RM_FILLED;
          break;
        case INTERPOLATED_FILL:
          drawPtr->render_mode = RM_INTERPOLATED;
          break;
        case GOURAUD_SHADE:
          drawPtr->render_mode = RM_SHADED;
          break;
        case B_SPLINE:
        case CLOSED_B_SPLINE:
          drawPtr->render_mode = RM_WIREFRAME; /* ?? */
          break;
        case TWO_SIDED:
          drawPtr->render_mode = RM_SHADED;
          break;
        case WIREFRAME:
          drawPtr->render_mode = RM_WIREFRAME;
          break;
        default:
          drawPtr->render_mode = RM_FILLED;
        }

        /* nurbs parameter */
        if (fgetpos(fp, &filepos))
          if (!fscanf(fp, "%f", &drawPtr->nurbs_sampling)) {
            drawPtr->nurbs_sampling = 0.0;
            fsetpos(fp, &filepos);
          }

        if (drawPtr->nurbs_sampling < 0.0)
          drawPtr->nurbs_sampling = 0.0;
      } break;
      case 16: /* light direction */
        inputFlag = fscanf(fp, " %f,%f,%f", &temp_x, &temp_y, &temp_z);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);
        temp_length = sqrt(temp_x * temp_x + temp_y * temp_y + temp_z * temp_z);
        drawPtr->light_dir[0] = temp_x / temp_length;
        drawPtr->light_dir[1] = temp_y / temp_length;
        drawPtr->light_dir[2] = temp_z / temp_length;
        VERBOSE("light direction: %g,%g,%g\n", drawPtr->light_dir[0],
                drawPtr->light_dir[1], drawPtr->light_dir[2]);
        break;
      case 17: /* diffuse reflection */
        READONENUMBER(drawPtr->diff_coef, " %d");
        break;
      case 18: /* surface ambient */
        READONENUMBER(drawPtr->ambient, " %f");
        break;
      case 19: /* surface diffuse */
        READONENUMBER(drawPtr->diffuse, " %f");
        break;
      case 20: /* line */
        inputFlag = fscanf(fp, " %c %s %f", &surface_id, surface_filename,
                           &surface_size);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);
        if (fscanf(fp, " %d %d", &s_precision, &t_precision) != 2) {
          s_precision = 0;
          t_precision = 0;
        }
        if (fscanf(fp, " %d", &s_texture) != 1)
          s_texture = -1;

        if (surface_size <= 0.0)
          surface_size = 1.0;
        /* Read surface description and calculate surface polygons */
        /* Size relative to step size */
        if (access(surface_filename, F_OK) != -1) {
          //        if (surface_filename != NULL) {
          read_surface(surface_filename, surface_size /* *STEP_SIZE*/,
                       surface_id, s_precision, t_precision, s_texture);
          drawPtr->line_type = surface_id;
        }
        break;
      case 21: /* surface */
        inputFlag = fscanf(fp, " %c %s %f", &surface_id, surface_filename,
                           &surface_size);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);
        s_precision = 0;
        t_precision = 0;

        if ((i = fscanf(fp, " %d %d", &s_precision, &t_precision)) != 2) {
          /* just one number entered */
          s_texture = i == 1 ? s_precision : -1;
          s_precision = 0;
        } else if (fscanf(fp, " %d", &s_texture) != 1)
          s_texture = -1;

        if (surface_size <= 0.0)
          surface_size = 1.0;

        /* Read surface descriptions and calculate surface polygons */
        /* Size relative to step size */
        if (access(surface_filename, F_OK) != -1) {
          //        if (surface_filename != NULL) {
          read_surface(surface_filename, surface_size /* *STEP_SIZE*/,
                       surface_id, s_precision, t_precision, s_texture);
        }
        break;
      case 22: /* bounding box */
        inputFlag = fscanf(fp, " x: %f,%f y: %f,%f z: %f,%f", &viewPtr->min[0],
                           &viewPtr->max[0], &viewPtr->min[1], &viewPtr->max[1],
                           &viewPtr->min[2], &viewPtr->max[2]);
        CheckInputError(fp, inputFlag, 6, inputPtr, lineNumber, includeFile);
        /* turn auto window off */
        viewPtr->auto_window = 0;
        VERBOSE("box: x: %g,%g y: %g,%g z: %g,%g\n", viewPtr->min[0],
                viewPtr->max[0], viewPtr->min[1], viewPtr->max[1],
                viewPtr->min[2], viewPtr->max[2]);
        break;
      case 23: /* # or #line line directive from preprocessor JH March 98 */
        if ((inputFlag = fscanf(fp, "line %d %s", &lineNumber, includeFile)) !=
            2) {
          inputFlag = fscanf(fp, " %d %s", &lineNumber, includeFile);
        }
        CheckInputError(fp, inputFlag, 2, inputPtr, lineNumber, includeFile);
        /* adjust for \n on this line */
        lineNumber--;
        VERBOSE("# %d %s\n", lineNumber, includeFile);
        break;
      case 24: /* background color */
        inputFlag = fscanf(fp, " %f,%f,%f", &red, &green, &blue);
        Warning("Warning: Background color not used by GL\n",WARNING_LVL);
        break;
      case 25: /* ambient light */
        inputFlag = fscanf(fp, " %f,%f,%f", &red, &green, &blue);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);
        Warning("Warning: Ambient light not used by GL\n",WARNING_LVL);
        break;
      case 26: /* font - for labelling on-screen - JH1 */
        inputFlag = fscanf(fp, " %s", &drawPtr->fontname[0]);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        VERBOSE("font: %s \n", drawPtr->fontname);
        break;
      case 27: /* sphere and cylinder polygonization level */
        READONENUMBER(drawPtr->cylinder_sides, " %d");
        drawPtr->cylinder_sides = (int)
#ifdef WIN32
            floor
#else
            rint
#endif
            (pow(2.0, (double)(drawPtr->cylinder_sides + 1)));

        if (drawPtr->cylinder_sides <= 2)
          drawPtr->cylinder_sides = 3;
        /* for backward compatibility */
        break;
      case 28: /* tropism direction */
        inputFlag = fscanf(fp, " %f,%f,%f", &temp_x, &temp_y, &temp_z);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);
        temp_length = sqrt(temp_x * temp_x + temp_y * temp_y + temp_z * temp_z);
        drawPtr->tropism_dir[0] = temp_x / temp_length;
        drawPtr->tropism_dir[1] = temp_y / temp_length;
        drawPtr->tropism_dir[2] = temp_z / temp_length;
        VERBOSE("tropism direction: %g,%g,%g\n", drawPtr->tropism_dir[0],
                drawPtr->tropism_dir[1], drawPtr->tropism_dir[2]);
        break;
      case 29: /* initial elasticity */
        READONENUMBER(drawPtr->elasticity, " %f");
        break;
      case 30: /* elasticity increment */
        READONENUMBER(drawPtr->elasticity_increment, " %f");
        break;
      case 31: /* environmental bloby field */
#define NTr environmentparam.number_of_fields
        if (NTr == MAXFIELDS) {
          Message("Warning! No room for new bloby field, field IGNORED!!!\n");
          break;
        }
        inputFlag = fscanf(fp, " %s %lf %lf %lf %lf %lf %lf", tmpfile, &v1[0],
                           &v1[1], &v1[2], &v2[0], &v2[1], &v2[2]);

        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (fscanf(fp, "%lf %lf %lf", &v1[0], &v1[1], &v1[2]) != 3) {
          v1[0] = v1[1] = v1[2] = 1.0;
          v2[0] = v2[1] = v2[2] = 0.0;
        } else if (fscanf(fp, "%lf %lf %lf", &v2[0], &v2[1], &v2[2]) != 3) {
          v2[0] = v2[1] = v2[2] = 0.0;
        }

        VERBOSE("bloby field ratios: %lf %lf %lf\n", v1[0], v1[1], v1[2]);
        VERBOSE("bloby field position: %lf %lf %lf\n", v2[0], v2[1], v2[2]);

        EPr.fields[NTr].ratio[0] = 1.0 / v1[0];
        EPr.fields[NTr].ratio[1] = 1.0 / v1[1];
        EPr.fields[NTr].ratio[2] = 1.0 / v1[2];

        EPr.fields[NTr].mid[0] = v2[0];
        EPr.fields[NTr].mid[1] = v2[1];
        EPr.fields[NTr].mid[2] = v2[2];

        if (field_initialize(NTr, tmpfile))
          NTr++;

        break;
      case 32: /* New tropism stuff  Radek */

        fgets(input_line, MaxLineLength, fp);

        read_movement(input_line, mtTropism);
        break;
      case 33: /* twist */
        fgets(input_line, MaxLineLength, fp);

        read_movement(input_line, mtTwist);
        break;

      case 34: /* OpenGL lights */
        fgets(input_line, MaxLineLength, fp);

        read_light(input_line);
        break;

      case 35: /* textures */
        fgets(input_line, MaxLineLength, fp);
        strcpy(input_line_tmp, input_line);
        while (!read_texture(input_line_tmp) && (count < 1000)) {
          strcpy(input_line_tmp, input_line);
          count++;
        }
        if (count >= 1000) {
          Warning("\n WARNING: Failed to open texture file for reading\n",WARNING_LVL);
        }
        break;

      case 36: /* render mode */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);

        for (i = 0; i < RM_COUNT; i++) {
          if (strcmp(input_string, rm_words[i]) == 0) {
            drawPtr->render_mode = i;
            break;
          }
        }

        if (isdigit(input_string[0]))
          drawPtr->render_mode = input_string[0] - '1'; /* '1' goes to 0 */

        if (drawPtr->render_mode >= RM_COUNT) {
          Message("Invalid render mode: %s. Default used!\n", input_string);
        } else
          VERBOSE("Render mode: %s\n", rm_words[(int)drawPtr->render_mode]);
        break;

      case 37: /* line style */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);

        for (i = 0; i < LS_COUNT; i++)
          if (strcmp(input_string, ls_words[i]) == 0) {
            drawPtr->line_style = i;
            break;
          }

        if (isdigit(input_string[0]))
          drawPtr->line_style = input_string[0] - '1'; /* '1' goes to 0 */

        if (drawPtr->line_style >= LS_COUNT) {
          Message("Invalid line style: %s. Default used!\n", input_string);
        } else
          VERBOSE("Line style: %s\n", ls_words[(int)drawPtr->line_style]);
        break;

      case 38: /* tapered lines */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        drawPtr->tapered_lines_on = (strcmp(input_string, "on") == 0);

        if (clp.verbose) {
          Message("Tapered lines ");
          if (drawPtr->tapered_lines_on)
            Message("on\n");
          else
            Message("off\n");
        }

        break;

      case 39: /* contour sides */
        READONENUMBER(drawPtr->cylinder_sides, " %d");
        if (drawPtr->cylinder_sides <= 2) {
          Message("Cannot draw cylinders with less than 3 sides! 3 used.\n",WARNING_LVL);
          drawPtr->cylinder_sides = 3;
        }
        break;

      case 40: /* contour */
        inputFlag = fscanf(fp, "%d %s", &id, tmpfile);
        CheckInputError(fp, inputFlag, 2, inputPtr, lineNumber, includeFile);

        VERBOSE("Contour with id %d and input file %s.\n", id, tmpfile);

        store_contour(tmpfile, id);
        break;

      case 41: /* viewing angle */
        READONENUMBER(viewPtr->viewing_angle, " %f");
        break;

      case 42: /* background scene */
        //fgets(scenefile, TMPFILELEN, fp);
        fscanf(fp, "%s", scenefile);
        // Set up temporary file name and preprocess.  
        strcpy(tmpfile, "/tmp/scene.XXXXXX");
        mkstemp(tmpfile);
        sprintf(buffer, "preproc %s > %s", scenefile, tmpfile);
        system(buffer);
        // save name of preprocessed file for rereading later, and removing when cpfg is closed
        // the preprocessed tmpfile is unlinked in FreeViewFileData() in generate.c
        strcpy(viewPtr->backgroundFilename, tmpfile);
        if (ReadBackgroundSceneFile(viewPtr->backgroundFilename ) == 0) {
	      Message("Can't read background scene file: %s\n", scenefile);
	    }
        break;

      case 43: /* twist of generalized cylinders */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (strncmp(input_string, "on", 2) == 0) {
          drawPtr->gen_cyl_twist = 1;
          VERBOSE("twist of generalized cylinders: on\n");
        } else {
          drawPtr->gen_cyl_twist = 0;
          VERBOSE("twist of generalized cylinders: off\n");
        }
        break;

      case 44: /* interpretation step */
        READONENUMBER(drawPtr->interpretation_step, " %d");
        if ((drawPtr->interpretation_step < -1) ||
            (drawPtr->interpretation_step == 0))
          drawPtr->interpretation_step = -1;
        break;

      case 45: /* interpretation past % */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (strncmp(input_string, "off", 3) == 0) {
          drawPtr->interpretation_past_cut = 0;
          VERBOSE("interpretation past %%: off\n");
        } else {
          drawPtr->interpretation_past_cut = 1;
          VERBOSE("interpretation past %%: on\n");
        }
        break;

      case 46: /* tsurface (a set of triangles) */
        inputFlag = fscanf(fp, " %c %s %f", &surface_id, surface_filename,
                           &surface_size);
        CheckInputError(fp, inputFlag, 3, inputPtr, lineNumber, includeFile);

        if (surface_size <= 0.0)
          surface_size = 1.0;

        /* Read surface descriptions and calculate surface polygons */
        /* Size relative to step size */
        if (access(surface_filename, F_OK) != -1)
          read_tsurface(surface_filename, surface_size, surface_id);
        break;

      case 47: /* rayshade objects */
        fgets(input_line, MaxLineLength, fp);

        rsReadViewCommand(input_line, viewPtr);
        break;

      case 48: /* rayshade scale */
        READONENUMBER(drawPtr->rayshade_scale, " %f");
        break;

      case 49: /* initial scale */
        READONENUMBER(drawPtr->scale_factor, " %f");
        if (drawPtr->scale_factor == 0) {
          Message("Initial scale cannot be 0! Set to 1.\n");
          drawPtr->scale_factor = 1;
        }
        break;

      case 50: /* scale multiplier */
        READONENUMBER(drawPtr->scale_multiplier, " %f");
        if (drawPtr->scale_multiplier == 0) {
          Message("Scale multiplier cannot be 0! Set to 1.\n");
          drawPtr->scale_multiplier = 1;
        }
        break;
      case 51: /* environment display */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (strncmp(input_string, "before", 6) == 0) {
          drawPtr->environment_display = 1;
        }
        break;
#if CPFG_VERSION >= 4000
      case 52: /* function */
      {
        char FuncFile[MaxPath + 1];
        int samples;
        inputFlag = fscanf(fp, " %s %d", FuncFile, &samples);
        if (2 != inputFlag)
          samples = -1;
        if ((1 != inputFlag) && (2 != inputFlag))
          CheckInputError(fp, inputFlag, 2, inputPtr, lineNumber, includeFile);
        ReadSplineFunction(FuncFile, samples);
        SetFunctionsFromGallery(FALSE);
      } break;
#endif
#if CPFG_VERSION >= 6400
      case 53: /* curve */
      {
        char CurveFile[MaxPath + 1];
        int samples;
        inputFlag = fscanf(fp, " %s %d", CurveFile, &samples);
        if (2 != inputFlag)
          samples = 100;
        if ((1 != inputFlag) && (2 != inputFlag))
          CheckInputError(fp, inputFlag, 2, inputPtr, lineNumber, includeFile);
        ReadCurveXYZ(CurveFile, samples);
      } break;
#endif
#if CPFG_VERSION >= 6500
      case 54: /* antialiasing */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (strncmp(input_string, "on", 2) == 0) {
          viewPtr->antialiasing_on = 1;
          VERBOSE("antialiasing: on\n");
        } else {
          viewPtr->antialiasing_on = 0;
          VERBOSE("antialiasing: off\n");
        }
        break;
      case 55: /* function set */
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        ReadFunctionSet(input_string);
        SetFunctionsFromGallery(TRUE);
        break;
#endif
      case 56: /* zoom min */
      {
        float minzoom;
        inputFlag = fscanf(fp, "%f", &minzoom);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (1 == inputFlag)
          viewPtr->zoom_min = minzoom;
      } break;
      case 57: /* zoom max */
      {
        float maxzoom;
        inputFlag = fscanf(fp, "%f", &maxzoom);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (1 == inputFlag)
          viewPtr->zoom_max = maxzoom;
      } break;
      case 58: /* generate on view changes */
      {
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (!(strcmp("on", input_string)))
          viewPtr->GenerateOnViewChange = eOn;
        else if (!(strcmp("continuous", input_string)))
          viewPtr->GenerateOnViewChange = eOn;
        else if (!(strcmp("off", input_string)))
          viewPtr->GenerateOnViewChange = eOff;
        else if (!(strcmp("triggered", input_string)))
          viewPtr->GenerateOnViewChange = eTriggered;
        else
          Warning("Invalid value of \"generate on view changes\" parameter\n",WARNING_LVL);
      } break;
      case 59: /* win font */
      {
        char flags[5];
        char line[128];
        const char *input;
        fgets(line, 127, fp);
        input = ReadQuotedString(line, drawPtr->winfont.Name, 80);
        inputFlag = sscanf(input, " %d %3s", &(drawPtr->winfont.size), flags);
        if (inputFlag < 1)
          CheckInputError(fp, inputFlag, 2, inputPtr, lineNumber, includeFile);
        if (2 == inputFlag) {
          flags[0] = toupper(flags[0]);
          if (0 != flags[1])
            flags[1] = toupper(flags[1]);
          if ('I' == flags[0] || 'I' == flags[1])
            drawPtr->winfont.italic = 1;
          else
            drawPtr->winfont.italic = 0;
          if ('B' == flags[0] || 'B' == flags[1])
            drawPtr->winfont.bold = 1;
          else
            drawPtr->winfont.bold = 0;
        }
      } break;
      case 60: /* concave polygons */
      {
        inputFlag = fscanf(fp, " %s", input_string);
        CheckInputError(fp, inputFlag, 1, inputPtr, lineNumber, includeFile);
        if (strncmp(input_string, "on", 2) == 0) {
          viewPtr->concavePolygons = 1;
          concavePolygons = 1;
        } else if (strncmp(input_string, "off", 3) == 0) {
          viewPtr->concavePolygons = 0;
          concavePolygons = 0;
        } else {
          Warning("Invalid value for concave polygons parameter.\n",WARNING_LVL);
        }
        break;
      }
      case 61: /* shadow map */
      {
        fgets(input_line, MaxLineLength, fp);
        read_shadow_map(input_line, drawPtr);
      }

      break;
      case 62: /* set PS line cap */
        READONENUMBER(drawPtr->ps_linecap, " %d");
        Message("Read ps line cap %d \n", drawPtr->ps_linecap);
        if (drawPtr->ps_linecap > 2) {
          Warning("PS line cap must be less than 3, set to 2\n",WARNING_LVL);
          drawPtr->ps_linecap = 2;
        }

        break;

      default:
        CheckInputError(fp, 1, 0, inputPtr, lineNumber, includeFile);
      }
    }

#undef NTr
#undef EPr
    VERBOSE("%d lines read from %s\n", lineNumber, filename);
    fclose(fp);

    VERBOSE("View file processed\n");
  }

  process_contours();

  /* Set values of parameters which are not directly read, but are
  calculated as a function of other parameters */

  /***  Calculate viewing normal */
  viewPtr->view_normal[0] = viewPtr->viewpoint[0] - viewPtr->vrp[0];
  viewPtr->view_normal[1] = viewPtr->viewpoint[1] - viewPtr->vrp[1];
  viewPtr->view_normal[2] = viewPtr->viewpoint[2] - viewPtr->vrp[2];
  distance =
      (float)sqrt((double)(viewPtr->view_normal[0] * viewPtr->view_normal[0] +
                           viewPtr->view_normal[1] * viewPtr->view_normal[1] +
                           viewPtr->view_normal[2] * viewPtr->view_normal[2]));

  if (distance == 0) {
    Warning("WARNING: view point and view reference point are the "
            "same!\nSetting to default!",WARNING_LVL);

    /* default values */
    viewPtr->view_normal[0] = viewPtr->view_normal[1] = 0;
    viewPtr->view_normal[2] = -1;

    viewPtr->viewpoint[0] = viewPtr->viewpoint[1] = 0;
    viewPtr->viewpoint[2] = -1;

    viewPtr->vrp[0] = viewPtr->vrp[1] = viewPtr->vrp[2] = 0;
  } else {
    viewPtr->view_normal[0] /= distance;
    viewPtr->view_normal[1] /= distance;
    viewPtr->view_normal[2] /= distance;
  }
  // MC - Dec. 2015 - check for incorrect values of front and back distance in
  // perspective mode
  if (!viewPtr->parallel_projection_on) {
    if (viewPtr->front_dist < 0.0f) {
      Warning(
          "WARNING: front distance cannot be negative in perspective mode.\n"
          "Please specify front distance in view file. Setting front distance "
          "to 0.1\n",WARNING_LVL);
      viewPtr->front_dist = 0.1f;
    }
    if (viewPtr->front_dist > viewPtr->back_dist) {
      Warning("WARNING: front distance cannot be greater than back distance in "
              "perspective mode.\n"
              "Setting front distance to 1 and back distance to 100.\n",WARNING_LVL);
      viewPtr->front_dist = 1.0f;
      viewPtr->back_dist = 100.0f;
    }
  }

  if (drawPtr->cue_range != 0)
    viewPtr->depth_cue_on = 1;
  else
    viewPtr->depth_cue_on = 0;

  angle = D_TO_R(drawPtr->angle_increment);
  drawPtr->sinangle = sin(angle);
  drawPtr->cosangle = cos(angle);
#ifndef GL
  {
    double vec[3], up[3] = {0.0, 1.0, 0.0};
    double q[4], angle, bendsin;

    DCrossProduct(viewPtr->view_normal, up, vec);

    if (DDotProduct(vec, vec) == 0.0) {
      vec[0] = -1.0;
      vec[1] = 0.0;
      vec[2] = 0.0;
    } else
      DNormalize(vec);

    if (viewPtr->twist != 0) {

      angle = -D_TO_R(viewPtr->twist * 0.1) / 2.0;
      q[0] = cos(angle);
      q[1] = (bendsin = sin(angle)) * viewPtr->view_normal[0];
      q[2] = bendsin * viewPtr->view_normal[1];
      q[3] = bendsin * viewPtr->view_normal[2];

      rot_by_quat(vec, q);
    }

    DCrossProduct(vec, viewPtr->view_normal, viewPtr->view_up);
  }
#endif
  return 1;
#undef MaxLineLength
}

static void bend(DRAWPARAM *dr, TURTLE *tu) {
  double bendcos, bendsin;
  double temp[3];

  if (tu->elasticity == 0.0)
    return;

  bendcos = (tu->elasticity) * DDotProduct(dr->tropism_dir, tu->up);

  bendsin = sqrt(1 - bendcos * bendcos);

  change_dir(temp, tu->heading, tu->up, bendcos, bendsin);

  /* new stuff */
  bendcos = (tu->elasticity) * DDotProduct(dr->tropism_dir, tu->left);

  bendsin = sqrt(1 - bendcos * bendcos);

  change_dir(temp, tu->heading, tu->left, bendcos, bendsin);
}

static void read_shadow_map(char *input_line, DRAWPARAM *drawPtr) {
  char *token;
  int size;
  float x;

  // read the shadow map directive one token at a time...
  token = strtok(input_line, " \t:");
  while (token != NULL) {

    if (strcmp("size", token) == 0) {

      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        Warning("shadow map: nothing specified for size directive\n",WARNING_LVL);
        size = 1024;
      } else
        size = atoi(token);
      if (size % 2 != 0) {
        Warning("shadow map: size should not be an odd number.\n",WARNING_LVL);
        size += 1;
      }
      drawPtr->shadow_map_size = size;
    } else if (strcmp("color", token) == 0) {
      // red
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        Warning("shadow map: r,g,b not specified for color directive\n",WARNING_LVL);
        continue;
      }
      x = atof(token);
      if (x < 0.0)
        x = 0.0;
      if (x > 1.0)
        x = 1.0;
      drawPtr->shadow_color[0] = x;
      // green
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        Warning("shadow map: r,g,b not specified for color directive\n",WARNING_LVL);
        continue;
      }
      x = atof(token);
      if (x < 0.0)
        x = 0.0;
      if (x > 1.0)
        x = 1.0;
      drawPtr->shadow_color[1] = x;
      // blue
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        Warning("shadow map: r,g,b not specified for color directive\n",WARNING_LVL);
        continue;
      }
      x = atof(token);
      if (x < 0.0)
        x = 0.0;
      if (x > 1.0)
        x = 1.0;
      drawPtr->shadow_color[2] = x;
    } else if (strcmp("offset", token) == 0) {
      // offset factor
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        Warning("shadow map: factor not specified for offset directive\n",WARNING_LVL);
        continue;
      }
      x = atof(token);
      if (x < 0.0)
        x = 0.0;
      drawPtr->shadow_offset[0] = x;
      // units
      token = strtok(NULL, ",; \t:\n");
      if (token == NULL) {
        Warning("shadow map: units not specified for offset directive\n",WARNING_LVL);
        continue;
      }
      x = atof(token);
      if (x < 0.0)
        x = 0.0;
      drawPtr->shadow_offset[1] = x;
    } else {
      Message("Unrecognized shadow map command: %s\n", token);
    }

    token = strtok(NULL, " \t:\n");
  }

  return;
}

#ifdef JIM
/*********************************************************************/
/* Set symbols to point to appropriate instance, current or external */
/* Note that this should really be integrated with AssignSymbolsToInstances
 * in generate.c, and could be if the sublsystem stacks were handled
 * the same way  */
/*********************************************************************/
void AssignSymbolsToInstancesForInterpret(Symbol *symbolPtr,
                                          SymbolInstance *instancePtr,
                                          SUBLSYSTEM *subLstackPtr,
                                          SUBLSYSTEM *subLbottom) {
  double *valuesPtr;
  int dim;
  Symbol *foundSymbol;
  SUBLSYSTEM *subLptr;
  /* point all symbols to the current instance */
  valuesPtr = instancePtr->values;
  while (symbolPtr != NULL) {
    /* assign to current instance; default for externals, correct for others */
    symbolPtr->values = valuesPtr;
    symbolPtr->offset = symbolPtr->defaultOffset;
    /* for external variables, check for a definition in the Lsystem stack
     * if its available */
    if ((symbolPtr->type == EXTERNAL || symbolPtr->type == EXTERNALARRAY)) {
      subLptr = subLstackPtr;
      subLptr--; /* search below current position on the stack */
      foundSymbol = NULL;
      while (subLptr >= subLbottom && subLptr->LsysPtr != NULL &&
             foundSymbol == NULL) {
        foundSymbol =
            CurrentSymbolFind(symbolPtr->label, subLptr->LsysPtr->symbolTable);
        if (foundSymbol != NULL) {
          /* check that type is appropriate */
          switch (symbolPtr->type) {
          case EXTERNAL:
            switch (foundSymbol->type) {
            case DOUBLE:
            case EXTERNAL:
              /* found a match */
              break;
            case DOUBLEARRAY:
            case EXTERNALARRAY:
              /* not a match */
              foundSymbol = NULL;
              Message("WARNING: Matching array name ignored for external "
                      "variable %s.\n",
                      symbolPtr->label);
              break;
            default:
              Warning("INTERNAL ERROR: Incorrectly-typed variable.\n",WARNING_LVL);
              MyExit(1);
            }
            break;
          case EXTERNALARRAY:
            switch (foundSymbol->type) {
            case DOUBLE:
            case EXTERNAL:
              /* not a match */
              foundSymbol = NULL;
              Message("WARNING: Matching variable name ignored for external "
                      "array %s.\n",
                      symbolPtr->label);
              break;
            case DOUBLEARRAY:
            case EXTERNALARRAY:
              /* check array limits */
              if (symbolPtr->arrayData->dimensions ==
                      foundSymbol->arrayData->dimensions &&
                  symbolPtr->arrayData->spaceRequired ==
                      foundSymbol->arrayData->spaceRequired) {
                dim = symbolPtr->arrayData->dimensions - 1;
                while (dim >= 0) {
                  if (symbolPtr->arrayData->size[dim] !=
                      foundSymbol->arrayData->size[dim]) {
                    /* not a match */
                    foundSymbol = NULL;
                    Message("WARNING: Matching array name with mismatched "
                            "dimensions ignored for external array %s.\n",
                            symbolPtr->label);
                  }
                  dim--;
                }
              } else {
                /* not a match */
                foundSymbol = NULL;
                Message("WARNING: Matching array name with mismatched "
                        "dimensions ignored for external array %s.\n",
                        symbolPtr->label);
              }
              break;
            default:
              Warning("INTERNAL ERROR: Incorrectly-typed variable.\n",WARNING_LVL);
              MyExit(1);
            }
            break;
          default:
            Warning("INTERNAL ERROR: Non-external variable.\n",WARNING_LVL);
            MyExit(1);
          }
        }
        if (foundSymbol == NULL) {
          /* not found; keep looking */
          subLptr--;
          ;
        }
      }
      if (foundSymbol != NULL) {
        /* assign as current value overiding default */
        symbolPtr->values = foundSymbol->values;
        symbolPtr->offset = foundSymbol->offset;
      } else {
        Message("WARNING: No matching name found for external %s.\n",
                symbolPtr->label);
      }
    }
    symbolPtr = symbolPtr->nextSymbol;
  }
}
#endif
