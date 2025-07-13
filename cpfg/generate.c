/*
MODULE:		generate.c for the continuous plant and fractal generator

AUTHORS: Jim Hanan and Przemyslaw Prusinkiewicz

TO DO:
                Problem with freeing space for multiple L-systems; possibly
                introduced by MonaLisa port???

HISTORY:
                Work on parameter handling for continuous application started
June 27, 1989. Work on ring L-systems started Aug 27, 1989. Added cut symbol (%)
                September 1990:
                14: Change string symbol parameters to floats
                20: Convert production strings to link lists of modules
                24: Convert expressions to link lists of tokens
                27: Add lookup table for symbols without productions
                        Convert Lsystems and productions from arrays to lists
                October 1990:
                04: Set up context checking to maintain pointers to matched
symbols January 1991: 25: added fudged arctan function using binary operator #
to expressions March 1991: 13: added fudged functions using binary operator ~
                        first operand 1 is cos of second in degrees
                        first operand 2 is sin of second in degrees
                21: first operand 3 is uniform random number [0.0,1.0) scaled by
second May 1991: 31: added fudged functions using binary operator ~ first
operand 0 is tan of second in degrees first operand 4 is arctan in degrees of
second first operand 5 is arccos in degrees of second first operand 6 is arcsin
in degrees of second first operand 7 is exp of second first operand 8 is natural
log of second first operand 9 is floor of second first operand 10 is ceil of
second first operand 11 is trunc of second first operand 12 is abs of second
                November-December 1991:
                        unary minus and multi-character operators <= >= <> != &&
|| added replaced fudged functions with "real" functions: cos(a) in degrees
                        sin(a) in degrees
                        ran(a) uniform random number [0.0,1.0) scaled by a
                        tan(a) in degrees
                        atan(a) arctan in degrees
                        acos(a) arccos in degrees
                        asin(a) arcsin in degrees
                        exp(a)
                        log(a)
                        floor(a)
                        ceil(a)
                        trunc(a)
                        fabs(a)
                May 1994:
                        array stuff
                March-June 1994:
                        ansi standard + prepared for C++
                        speeding up:
                          skipright - modifications just inside
                          skipleft  - modifications just inside
                          NextStringModule -       -||-
                          ApplyProd        -       -||-
                          EToken instead of Token
                          effective evaluation (Eval)

-------------------------------------------------------------------------------
PURPOSE:	use an L-system to generate strings.

DESCRIPTION:	(This description does not reflect stochastic mechanisms,
                autoincrementing/autodecrementing nor the handling of bracketed
                context)

                Do until hit end of string1:
                    Replace predecessors by their successor
                         (Function call: FINDPROD)
                         i.  Check each predecessor for a match
                                (Function call: PREFIX)
                                a. Compare the predecessor to the symbol in
string1 b. If same, return a 1 c. If not the same, return a 0 ii. If match,
return a pointer to the production iii.Else, return a null value (Function call:
APPLYPROD) i.  If a production exists, append the predecessor into string2 and
                             go to the next symbol in string1
                         ii. Else, append the symbol in string1
                             to string2 and get the next symbol in string1
                    Note: if the string is too long, stop
                Return the derived string
*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <assert.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <mmsystem.h>
#include <rand.h>
#endif

#ifndef WIN32
#include <unistd.h>
#endif

#include <stdio.h>
#include <math.h>
#include <string.h>
#ifndef WIN32
#ifndef LINUX
#include <stdarg.h>
#include <bstring.h>
#endif /* LINUX */
#endif /* WIN32 */

#ifdef LINUX
#include <limits.h>
#endif

#include "platform.h"
#include "interpret.h"
#include "control.h"
#include "generate.h"
#include "lsys_input_yacc.h"
#include "environment.h"
#include "textures.h"
#include "utility.h"
#include "general.h"
#include "tsurfaces.h"
#include "irisGL.h"
#ifdef CPFG_ENVIRONMENT
#include "comm_lib.h"
#endif
#include "background.h"
#include "userfiles.h"

#if CPFG_VERSION >= 4000
#include "splinefunC.h"
#endif

#if CPFG_VERSION >= 6400
#include "curveXYZc.h"
#endif

#include "test_malloc.h"

extern int animateFlag;
extern RECTANGLE viewWindow;
extern VIEWPARAM viewparam;

unsigned short xsubi[3]; /* required by the random number generator */

/* constant token value - global so lsys_inputparse can get at it */
char constantToken[30];

/* Global symbol table */
static Symbol *globalSymbols = NULL;
#ifdef JIM
static SymbolInstance *globalSymbolInstance = NULL;
#endif

/* Variables for stacking of sub-L-systems. */
static LSYSDATA *lsystemStack[MAXNESTING]; /* L-system stack */
static int lstackIndex;                    /* current position on the stack */

/* used in ApplyProd() */
static char *one_successor = NULL; /* for storing successor before applying
                                      the decomposition */
static int one_successor_len;

extern int num_cut_substrings;
extern SUBSTRING *cut_substrings;

#ifdef WIN32
extern HANDLE hLSemaphore;
#endif

#ifdef VIEWER
void CleanLabTable();
#endif

extern LSYSDATA *LsystemList;

void FreeRest(void);
// extern int lsys_inputparse(void);
int lsys_inputparse(void);

/***************** local prototypes ***********************/
static LSYSDATA *FreeLsystemSpace(LSYSDATA *LsystemListPtr);
static void FreeStringSpace(Module *modulePtr);
static void FreeStatementSpace(Statement *statementPtr);
static void FreeSymbolSpace(Symbol *symbolPtr);
static void FreeSymbolTableSpace(SymbolTable *symbolTablePtr);
void FreeArraySpace(Array *arrayPtr);
static Production *FindProd(char *curPtr, LSYSDATA *LsysPtr);
#ifndef JIM
static void CheckPred(char *strtPtr, char *curPtr, LSYSDATA *LsystemListPtr,
                      LSYSDATA **currentLsystem);
#endif
#ifdef JIM
static void ProcessSubLsystems(char *strtPredPtr, char *curPtr,
                               char *strtSuccPtr, char *nextPtr,
                               LSYSDATA *LsystemListPtr,
                               LSYSDATA **currentLsystem, int endStep);
void InitSymbolInstances(LSYSDATA *lsysListPtr);
#endif
static void AppendSuccessor(char **strPtr, Production *prodPtr);
static void PrintParameterList(Parameter *start);
static char NextStringModuleForMatch(char **str, StringModule *module);
static void CreateVarTable(Production *prodPtr, char *str, LSYSDATA *LsysPtr);
static int GetPredVariables(Module *predPtr, char *strPtr);
#ifdef JIM
Symbol *CurrentSymbolFind(char *label, Symbol *currentSymbol);
#endif
static int ProcessSymbolVariables(Module *ptr, char **str);
static void GetRconVars(Module *rconPtr);
static char *moveright(char *s);
static char *skipright(char *s);
static void GetLconVars(Module *lconPtr);
static char *movestringleft(char *s);
static char *skipleft(char *s);
static void FindApplProductions(char *curPtr, LSYSDATA *LsysPtr,
                                Production *applSetPtr[], float *totalprobPtr);
static Production *SelectProd(Production *applSetPtr[], float totalprob,
                              char *curPtr, LSYSDATA *LsysPtr);
static FILE *PreprocessLsystem(const char *);

/***************** externs ******************************/
extern SymbolTable *currentSymbolTable; /* symbol table stack */

/* pointer to the global symbol table */
extern SymbolTable *globalSymbolTable;

#ifdef JIM

#define GetSymbolValuePtr(symbol) ((symbol)->values + (symbol)->offset)

#ifdef DEBUG2
/*********************************************************************/
double *GetSymbolValuePtr(Symbol *symbol) {

  if (symbol->offset < 0 || symbol->offset > symbol->valueSpace) {
    Message("Symbol offset %d out of bounds.\n", symbol->offset);
    return symbol->values;
  }

  return symbol->values + symbol->offset;
}
#endif
#endif

/*********************************************************************/
/* Function: ReadLsystem                                             */
/* This function reads file filename containing information about    */
/* the L-system, and sets up a structure with: the axiom, the set    */
/* of productions, the symbols to be ignored and                     */
/* the number of steps in the derivation to be applied.              */
/* If this is a reread, free previously allocated space              */
/*********************************************************************/

int ReadLsystem(char *filename) {
  extern LSYSDATA *LsystemList; /* Lsystem data */

  extern SymbolTable *globalSymbolTable; /* global symbol table pointer */
  extern int inputError;                 /* error flag  */
  extern FILE *lsys_inputin;             /* yacc/lex file pointer */
  extern int nextGlobalString; /* index to global strings array */ /* JH1 */
  int ret;
#ifndef WIN32
  int counter = 0;
  int current_size = -1;
  int size = 0;
  FILE *fp, *source, *target;
  char ch;
#endif

  char parseOK = 0;
  /* free previously allocated space */
  LsystemList = FreeLsystemSpace(LsystemList);
#ifdef MDEBUG
  {
    struct mallinfo info;
    info = mallinfo();
    Message("MALLOCINFO:\n");
    Message(" total space in arena  %d\n", info.arena);
    Message(" number of ordinary blocks  %d\n", info.ordblks);
    Message(" number of small blocks  %d\n", info.smblks);
    Message(" number of holding blocks  %d\n", info.hblks);
    Message(" space in holding block headers  %d\n", info.hblkhd);
    Message(" space in small blocks in use  %d\n", info.usmblks);
    Message(" space in free small blocks  %d\n", info.fsmblks);
    Message(" space in ordinary blocks in use  %d\n", info.uordblks);
    Message(" space in free ordinary blocks  %d\n", info.fordblks);
    Message(" cost of enabling keep option  %d\n", info.keepcost);
  }
#endif

  Initialize_substr_turtle_array();

  /* reset global string index to 0 */
  nextGlobalString = 0;

  FreeFps();

  /* setup symbol table stack */
#ifdef JIM
  /* start with 4 values this level */
  globalSymbolInstance = NewSymbolInstance(4);
  currentSymbolTable = globalSymbolTable = PushSymbolTable(
      currentSymbolTable, &globalSymbols, &globalSymbolInstance);
#else
  currentSymbolTable = globalSymbolTable =
      PushSymbolTable(currentSymbolTable, &globalSymbols);
#endif

  /* set error flag */
  inputError = FALSE;

  /* Run input file through C preprocessor */
  VERBOSE("Preprocessing production file %s ", filename);
  VERBOSE("using the %s preprocessor.\n", clp.preprocessor);
  ////////////////////////////////////////////////////////////////
  // Wait until filename has been fully written
  // [PASCAL]
  // open the file twice to check if the size is consistent

#ifndef WIN32
  // get size of file
  fp = fopen(filename, "r");
  // MIK - THis is a big hack. Need to add a counter to avoid infinite loop if
  // view file is missing
  while ((fp == NULL) && (counter < 1000)) {
    fp = fopen(filename, "r");
    counter++;
  }
  if (counter == 1000)
    fprintf(stderr, "WARNING (generate.c): Can't open the view file %s - using defaults.\n",
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
  ////////////////////////////////////////////////////////////////////

  /* open preprocessed file */
  if ((lsys_inputin = PreprocessLsystem(filename)) == NULL) {
#ifdef WIN32
    if (NULL != hLSemaphore)
      ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
    Message("Can't open preprocessed L-system file %s\n", filename);
    MyExit(1);
  }
#ifdef WIN32
  if (NULL != hLSemaphore)
    ReleaseSemaphore(hLSemaphore, 1, NULL);
#endif
  LsystemList = NULL;

  /* Call yacc- and lex-generated code to handle input */
  ret = lsys_inputparse();

  // [PASCAL] the following to understand why yacc doesn't return a LsystemList
  /*
  if (LsystemList == NULL) {
    source = fopen(filename, "r");
    target = fopen("tmp.tmp", "w");
    while ((ch = fgetc(source)) != EOF)
      fputc(ch, target);
    fclose(source);
    fclose(target);
  }
  */
  parseOK = (ret != 1);
  fclose(lsys_inputin);
  lsys_inputin = NULL;
#ifdef VIEWER
  CleanLabTable();
#endif
  if (!parseOK || inputError > WARNING_LVL) {
    Message("Syntax error in %s.\n", filename);
    my_ringbell();
    /* try to free previously allocated space and recover*/
    LsystemList = FreeLsystemSpace(LsystemList);
    return 0;
  } else if (LsystemList == NULL) {
    my_ringbell();
    //    Message("Parser is confused due to the previous errors.\n"
    //        "You may have to restart cpfg.\n");
    return 0;
  }

  VERBOSE("FINISHED READING PRODUCTION FILE.\nRUNNING\n");

  if (clp.warnings) {
    if (clp.checkEnvironment)
      Message("A query symbol or %%() encountered - interpretation pass on.\n");
  }
  return 1;
}

void FreeRest() {
  int i;

  for (i = 0; i < SAVE_COUNT; i++) {
    if (clp.savefilename[i] != NULL) {
      Free(clp.savefilename[i]);
      clp.savefilename[i] = NULL;
    }
  }
}

void FreeViewFileData(void) {
  FreeEnvironmentSpace();
  InitializeEnvironmentParam();

  FreeTextureSpace();
  FreePrimitives();
  FreeTsurfaces();
  FreeSplineFunctions();
  FreeCurveXYZ();

  // remove the preprocessed file: unlink the temp file 
  // UNIX will keep the file around until it is closed
  // at which point it is removed 
  unlink(viewparam.backgroundFilename);
}

#ifdef WIN32
HWND hLProject;
HWND hMain;
#define ID_MENUITEM40055 40055
extern FILE *fpLog;
#endif
extern int GenTexturesCount;

/***************************************************************************/
void MyExit(int status) {
  extern LSYSDATA *LsystemList;
  FreeMaterialSpace();
  FreeColormapSpace();
  FreeViewFileData();
  FreeLsystemSpace(LsystemList);
#if CPFG_VERSION >= 3200
  if (clp.communication != COMM_NONE)
    CTerminate();
#else
#ifdef CPFG_ENVIRONMENT
  CMFreeStructures();
#endif
#endif
  FreeGraphics();
  FreeSpline(); /* not in FreeViewFileData because I wanted to keep the
  stack allocated (the same as turtle stack). */
  FreeStrings();
  FreeStacks();
  FreeRest();

#ifdef TEST_MALLOC
  test_malloc_evaluate();
  /* cannot be called twice - it releases all its
  data structures */
#endif

  if (GenTexturesCount != 0) {
    Message("TexturesGenerated left: %d", GenTexturesCount);
  }
#ifdef WIN32
  if (NULL != hLProject)
    PostMessage(hLProject, WM_COMMAND,
                MAKEWPARAM((UINT)ID_MENUITEM40055, (UINT)0), (LPARAM)NULL);

  if (NULL != fpLog) {
    fclose(fpLog);
    fpLog = NULL;
  }

  if (NULL != hMain) {
    HKEY hKey;
    DWORD disp;
    WINDOWPLACEMENT wp;
    LONG res =
        RegCreateKeyEx(HKEY_CURRENT_USER, "Software\\RadekSoftware\\cpfg", 0,
                       NULL, 0, KEY_SET_VALUE, NULL, &hKey, &disp);

    if (ERROR_SUCCESS != res)
      return;
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hMain, &wp);
    res = RegSetValueEx(hKey, "WindowPlacement", 0, REG_BINARY,
                        (const BYTE *)&wp, sizeof(WINDOWPLACEMENT));
    RegCloseKey(hKey);
  }
  if (NULL != hLSemaphore) {
    CloseHandle(hLSemaphore);
    hLSemaphore = NULL;
  }
#endif
  exit(status);
}

/***************************************************************************/
/* used to free both homomorphism and decomposition space */
static void FreeProductionSpace(PRODDATA *production_set) {
  int i;
  Production *currentProduction;
  Production *nextProduction;

  if (!production_set->specified)
    return;

  production_set->specified = 0;

  if (production_set->stack != NULL) {
    for (i = 0; i < production_set->stack_len; i++) {
      if (production_set->stack[i] != NULL) {
        Free(production_set->stack[i]);
        production_set->stack[i] = NULL;
      }
    }
    Free(production_set->stack);
    production_set->stack = NULL;

    production_set->stack_len = 0;
  }

  for (i = 0; i < 128; i++) {
    nextProduction = production_set->firstProd[i];
    while (nextProduction != NULL) {
      currentProduction = nextProduction;
      FreeStringSpace(currentProduction->lCon);
      FreeStringSpace(currentProduction->pred);
      FreeStringSpace(currentProduction->rCon);
      FreeStringSpace(currentProduction->succ);
      FreeStatementSpace(currentProduction->preCondList);
      FreeExpressionSpace(currentProduction->condition);
      FreeStatementSpace(currentProduction->postCondList);
      FreeSymbolSpace(currentProduction->symbolTable);
#ifdef JIM
      FreeInstanceList(currentProduction->instance);
#endif
      nextProduction = currentProduction->nextProduction;
      Free(currentProduction);
      currentProduction = NULL;
    }
  }
}

#ifdef JIM
/***************************************************************************/
void CheckValues(__attribute__((unused)) LSYSDATA *LsysPtr,
                 __attribute__((unused)) SymbolTable *curTable) {
  VERBOSE("Checking \n");
}

#endif
/***************************************************************************/
static LSYSDATA *FreeLsystemSpace(LSYSDATA *LsystemListPtr) {
  int i;
  LSYSDATA *currentLsystem;
  LSYSDATA *nextLsystem;
  Production *currentProduction;
  Production *nextProduction;
  extern int num_substr_turtle;

  num_substr_turtle = 0;

  FreeSymbolSpace(globalSymbols);
  globalSymbols = NULL;

#ifdef JIM
  FreeInstanceList(globalSymbolInstance);
  globalSymbolInstance = NULL;
#endif

  FreeSymbolTableSpace(currentSymbolTable);
  currentSymbolTable = NULL;

  FreeFileDefSpace();

  Free_substr_turtle_array();

  if (one_successor != NULL) {
    Free(one_successor);
    one_successor = NULL;
  }

  nextLsystem = LsystemListPtr;
  /* loop over Lsystems */
  while (nextLsystem != NULL) {
    currentLsystem = nextLsystem;
    FreeSymbolSpace(currentLsystem->symbolTable);
#ifdef JIM
    FreeInstanceList(currentLsystem->instanceList);
#endif
    FreeStatementSpace(currentLsystem->startBlock);
    FreeStatementSpace(currentLsystem->endBlock);
    FreeStatementSpace(currentLsystem->startEach);
    FreeStatementSpace(currentLsystem->endEach);
    FreeStatementSpace(currentLsystem->defineBlock);

    FreeProductionSpace(&currentLsystem->Homomorphism);
    FreeProductionSpace(&currentLsystem->Decomposition);

    FreeStringSpace(currentLsystem->axiom);
    /* loop over productions */
    for (i = 0; i < 128; i++) {
      nextProduction = currentLsystem->firstProd[i];
      while (nextProduction != NULL) {
        currentProduction = nextProduction;
        FreeStringSpace(currentProduction->lCon);
        FreeStringSpace(currentProduction->pred);
        FreeStringSpace(currentProduction->rCon);
        FreeStringSpace(currentProduction->succ);
        FreeStatementSpace(currentProduction->preCondList);
        FreeExpressionSpace(currentProduction->condition);
        FreeStatementSpace(currentProduction->postCondList);
        FreeSymbolSpace(currentProduction->symbolTable);
#ifdef JIM
        FreeInstanceList(currentProduction->instance);
#endif
        nextProduction = currentProduction->nextProduction;
        Free(currentProduction);
        currentProduction = NULL;
      }
    }
    Free(currentLsystem->name);
    currentLsystem->name = NULL;

    nextLsystem = currentLsystem->nextLsystem;

    Free(currentLsystem);
    currentLsystem = NULL;
  }
  return NULL;
}

static void FreeStringSpace(Module *modulePtr) {
  Module *currentModule;
  Module *nextModule;
  Parameter *currentParameter;
  Parameter *nextParameter;

  nextModule = modulePtr;
  /* loop over modules */
  while (nextModule != NULL) {
    currentModule = nextModule;
    nextParameter = currentModule->parmList;
    /* loop over parameters */
    while (nextParameter != NULL) {
      currentParameter = nextParameter;
      if (currentParameter->type == 'e') {
        FreeExpressionSpace(currentParameter->ptrTo.expression);
      }

      nextParameter = currentParameter->nextParameter;
      Free(currentParameter);
      currentParameter = NULL;
    }
    nextModule = currentModule->nextModule;
    Free(currentModule);
    currentModule = NULL;
  }
}

void FreeExpressionSpace(EToken *tokenPtr) {
  if (tokenPtr == NULL)
    return;

  if (tokenPtr->nextParam != NULL)
    FreeExpressionSpace(tokenPtr->nextParam);
  else
    FreeExpressionSpace(tokenPtr->up);

  Free(tokenPtr->tokenString);
  tokenPtr->tokenString = NULL;
  Free(tokenPtr);
  tokenPtr = NULL;
}

static void FreeStatementSpace(Statement *statementPtr) {
  Statement *currentStatement;
  Statement *nextStatement;

  nextStatement = statementPtr;
  /* loop over statements */
  while (nextStatement != NULL) {
    currentStatement = nextStatement;
    nextStatement = currentStatement->nextStatement;
    FreeExpressionSpace(currentStatement->leftHandSide);
    FreeExpressionSpace(currentStatement->expression);
    FreeExpressionSpace(currentStatement->condition);
    FreeStatementSpace(currentStatement->block);
    FreeStatementSpace(currentStatement->elseblock);
    Free(currentStatement);
    currentStatement = NULL;
  }
}

static void FreeSymbolSpace(Symbol *symbolPtr) {
  Symbol *currentSymbol;
  Symbol *nextSymbol;

  nextSymbol = symbolPtr;
  /* loop over symbols */
  while (nextSymbol != NULL) {
    currentSymbol = nextSymbol;
    nextSymbol = currentSymbol->nextSymbol;
    if (currentSymbol->type == DOUBLEARRAY
#ifdef JIM
        || currentSymbol->type == EXTERNALARRAY
#endif
    ) {
      FreeArraySpace(currentSymbol->arrayData);
    }
    Free(currentSymbol->label);
    currentSymbol->label = NULL;
    Free(currentSymbol);
    currentSymbol = NULL;
  }
}

static void FreeSymbolTableSpace(SymbolTable *symbolTablePtr) {
  SymbolTable *currentSymbolTable;
  SymbolTable *nextSymbolTable;

  nextSymbolTable = symbolTablePtr;
  /* loop over symbols Table*/
  while (nextSymbolTable != NULL) {
    currentSymbolTable = nextSymbolTable;
    nextSymbolTable = currentSymbolTable->nextSymbolTable;
    Free(currentSymbolTable);
    currentSymbolTable = NULL;
  }
}

/* This routine clears the global array of file definitions and resets the
 * pointer to 0 */
/*
void FreeFileDefSpace(void)
{
        int i;
        for (i=0; i<MAXFILES; i++)
        {
                if (fps[i] != NULL)
                {
                        Message("File %s was never closed\n", fnames[i]);
                        fclose(fps[i]);
                        fps[i] = NULL;
                        strcpy(fnames[i], "");
                }
        }
}
*/

void FreeParameterList(Parameter *paramPtr) /* proto */
{
  Parameter *currentParameter;
  Parameter *nextParameter;

  nextParameter = paramPtr;
  /* loop over symbols */
  while (nextParameter != NULL) {
    currentParameter = nextParameter;
    nextParameter = currentParameter->nextParameter;
    switch (currentParameter->type) {
    case 'f': /* formal parameter */
      /* handled by freeing of symbol tables */
      break;
    case 'e': /* expression parameter */
      FreeExpressionSpace(currentParameter->ptrTo.expression);
      break;
    default:
      Message("ERROR: Invalid parameter type during free.\n");
      MyExit(1);
    }
    Free(currentParameter);
    currentParameter = NULL;
  }
}

void FreeArraySpace(Array *arrayPtr) {
  if (arrayPtr == NULL)
    return;

  FreeParameterList(arrayPtr->sizeExpressions);
  FreeParameterList(arrayPtr->initExpressions);
  Free(arrayPtr->size);
  arrayPtr->size = NULL;
#ifndef JIM
  Free(arrayPtr->values);
  arrayPtr->values = NULL;
#endif
  Free(arrayPtr);
  arrayPtr = NULL;
}

/*********************************************************************/
/* Function: CreateParameterString                                   */
/* Converts axiom from module list to packed string                  */
/* also creates any required instances of symbol table values        */
/*********************************************************************/

int CreateParameterString(char **targetStr, Module *sourceModules,
                          LSYSDATA *lsysPtr) {
  char *targetPtr;    /* pointer for target string */
  Module *sourcePtr;  /* pointer for source module list */
  Parameter *parmPtr; /* pointer for source module parameter list */
#ifdef JIM
  LSYSDATA *sublsysPtr;     /* pointer for sub-Lsystem definition */
  SymbolInstance *dummyPtr; /* dummy pointer for instance creation */
#endif
  targetPtr = *targetStr;
  sourcePtr = sourceModules;
  /* 	append modules to the target string after
  evaluating the parameter expressions */
  while (sourcePtr != NULL) {
    *targetPtr = sourcePtr->symbol;
    targetPtr++;

#ifdef JIM
    /* check for sub-Lsystem reference and initialise */
    if (sourcePtr->symbol == '?' && sourcePtr->parameters > 0) {
      int e = Eval(sourcePtr->parmList->ptrTo.expression);
      sublsysPtr = MatchingLsystem(e, lsysPtr);
      dummyPtr = AppendInstance(sublsysPtr);
    }
#endif
    /* check for parameters and process */
    if (sourcePtr->parameters > 0) {
      /* append all parameters */
      parmPtr = sourcePtr->parmList;
      *targetPtr = '(';
      while (parmPtr != NULL) {
        targetPtr++;
        AppendParameter(&targetPtr, Eval(parmPtr->ptrTo.expression));
        *targetPtr = ',';
        parmPtr = parmPtr->nextParameter;
      }
      *targetPtr = ')';
      targetPtr++;
    }
    sourcePtr = sourcePtr->nextModule;
  }
  *targetPtr = '\0';
  /* return the length of the new target string */
  return (int)(targetPtr - *targetStr);
}

int charCount;
FILE *fp;
unsigned long len;
extern StringModule module; /* from interpret.c */

/*********************************************************************/
/* the third parameter is ignored, it must be present, though, because the
   function is called from InterpreHomomorphism() */
void SaveTextModule(__attribute__((unused)) char c,
                    __attribute__((unused)) char **str,
                    __attribute__((unused)) char is_homo) {
  int i;

  if (charCount >= 70) {
    charCount = 0;
    fprintf(fp, "\n");
  }

  fprintf(fp, "%c", module.symbol);
  charCount++;

  if (module.parameters > 0) {
    charCount += fprintf(fp, "(");
    for (i = 0; i < module.parameters; i++) {
      charCount += fprintf(fp, "%g", module.actual[i].value);
      if (i != module.parameters - 1)
        fprintf(fp, ",");
    }
    charCount += fprintf(fp, ")");
  }
}

/*********************************************************************/
/* the third parameter is ignored, it must be present, though, because the
   function is called from InterpreHomomorphism() */
void SaveModuleBinary(__attribute__((unused)) char c, char **str,
                      __attribute__((unused)) char is_homo) {
  len += fwrite(*str - module.length, 1, module.length, fp);
}

/*********************************************************************/
/* Function: PrintParameterString                                    */
/* Prints string, converting parameters from packed float            */
/*********************************************************************/

void PrintParameterString(FILE *fp2, char *str, int format, char use_homo) {
  extern char unique_strb_header[];
  extern LSYSDATA *LsystemList;
  extern LSYSDATA *currLsysPtr;
  char c, *curPtr;

  fp = fp2; /* initializing the global variable */

  /* initialization */
  if (format == SAVE_STRINGDUMP) {
    if (fwrite(unique_strb_header, strlen(unique_strb_header), 1, fp) < 1) {
      Message("Saving unique header failed!\n");
      return;
    }

    /* what to do about length? - leave enough spaces */
    if (fprintf(fp, "                   %d\n", LsystemList->current) < 1) {
      Message("Saving of string length and current step failed!\n");
      return;
    }
    len = 0;
  } else
    charCount = 0;

  currLsysPtr = LsystemList;

  /* module by module */
  for (;;) {
    if (*str == '\0')
      break;

    /* extract next symbol and parameters if available */
    /* and process as appropriate                      */
    curPtr = str; /* for homomorphism: to remember the symbol to be replaced*/

    c = NextStringModule(&str, &module);

    if (format == SAVE_STRING) {
      if (!use_homo || currLsysPtr->Homomorphism.specified == 0)
        SaveTextModule(c, &str, 0);
      else
#ifdef CONTEXT_SENSITIVE_HOMO
        InterpretHomomorphism(c, curPtr, &str, curPtr - 1, str, 0,
                              SaveTextModule);
#else
        InterpretHomomorphism(c, curPtr, &str, 0, SaveTextModule);
#endif
    } else if (!use_homo || currLsysPtr->Homomorphism.specified == 0)
      SaveModuleBinary(c, &str, 0);
    else
#ifdef CONTEXT_SENSITIVE_HOMO
      InterpretHomomorphism(c, curPtr, &str, curPtr - 1, str, 0,
                            SaveModuleBinary);
#else
      InterpretHomomorphism(c, curPtr, &str, 0, SaveModuleBinary);
#endif
  }

  if (format == SAVE_STRING)
    fprintf(fp, "\n");
  else {
    /* now when we know the length, let's update it */
    fseek(fp, (long)strlen(unique_strb_header) + 1, SEEK_SET);
    fprintf(fp, "%lu", len);
  }
}

/*************************************************************************/
/* reallocates the string and updates the cirrent pointer */
void ReallocateString(char **string, char **stringend, char **currPtr,
                      int length_over) {
  int Length, nextPtrOffset;
  if (length_over < 0)
    return;

  if ((Length = (int)(*stringend - *string)) < length_over - 1)
    Length += length_over + 2;
  else
    Length *= 2;

  nextPtrOffset = (int)(*currPtr - *string);

  Message("String is too long; reallocating.\n");
  /* reallocate should start one character to the left */
  if ((*string = (char *)Realloc(*string - 1, Length)) == NULL) {
    Message("Error: can't reallocate L-system string.\n");
    MyExit(1);
  }

  *stringend = *string + Length - 1;
  (*string)++;
  *currPtr = *string + nextPtrOffset;
}

/*********************************************************************/
/* Function: Derive                                                  */
/* Given a string and a set of productions, derive the appropriate   */
/* successor string. This is a parallel derivation (inherent in      */
/* L-systems), not a sequential one (inherent in Chomsky grammars).  */
/* String1 is scanned from left to right. Its consecutive substrings */
/* are matched with the predecessors of productions. The appropriate */
/* successors are appended to the end of string2.                    */
/* endStep is a flag that's true if this is the last step and        */
/* therefore end statements should be run                            */
/*********************************************************************/

void Derive(LSYSDATA *LsystemListPtr, char **string1,
            __attribute__((unused)) char **string1end, char **string2,
            char **string2end, DRAWPARAM *drawparamPtr, VIEWPARAM *viewparamPtr
#ifdef JIM
            ,
            int endStep
#endif
) {
  char *curPtr, *nextPtr;
#ifdef JIM
  char *strtPredPtr, *strtSuccPtr; /* used to be strtPtr for CheckPred */
  unsigned long strtSuccPtrOff;
#else
  char *strtPtr;
#endif
  float totalprob;
  LSYSDATA *currentLsystem;
  Production *prodPtr;
  Production *applSetPtr[MAXPROD];
  int substrings_added = 0;
  extern unsigned long currentStringLength;

  if (!drawparamPtr->environment_display)
    /* if 0, process environment after
    interpretation, i.e. changes in
    the enviroment are displayed
    in the following step, AFTER
    the structure is visualized */
    ProcessEnvironment(*string1, drawparamPtr, viewparamPtr);

  curPtr = *string1;
  nextPtr = *string2;

  /* Start with the mainline L-system */
  currentLsystem = LsystemListPtr;
  lstackIndex = 0; /* current position on the stack */
  lsystemStack[lstackIndex] = currentLsystem; /* L-system stack */

#ifdef JIM
  /* main L-system is OK but others should be set to point to the default
   * instance */
  InitSymbolInstances(LsystemListPtr);
#endif

  /* Update current derivation step counter */
  currentLsystem->current++;
  /* ... and global symbol table */
  /* strdup is necessary for consistency with parsing routines */
  SymbolTableReplace(Strdup("Q"), (double)currentLsystem->current,
                     globalSymbolTable);

#ifdef JIM
  /* ... and evaluate start each global statements for the main L-system
   */
  ProcessStatements(currentLsystem->startEach);
  /* Formerly this evaluated all sub-Lsystem statements as well */
  /* using EvaluateStartEach(LsystemListPtr); */
#else
  /* ... and evaluate start each global statements */
  /* This evaluates all sub-Lsystem statements as well */
  /* Should this be handled some other way?? */
  EvaluateStartEach(LsystemListPtr);
#endif

  /*********************************************************************/
  /* Proceed along string1 from left to right replacing predecessors   */
  /* by their successors until the end of string1 is reached.          */
  /* If the current L-system is stochastic, perform a random derivation*/
  /* reallocate if the string being derived gets too long.             */
  /*********************************************************************/

  for (;;) {
    if (*curPtr == '\0') {
      /* if there is no other cut substring to be moved to the end, */
      /* exit the loop */
      if (substrings_added == num_cut_substrings)
        break;

      /* copy the cut symbol and add index to a turtle table */
      curPtr = cut_substrings[substrings_added].first;
      *curPtr = '%'; /* just in case it was replaced by \0 */

      *(nextPtr++) = '%';
      *(nextPtr++) = '(';

      /* place the first parameter */
#ifdef WIN32
      memcpy(nextPtr, &cut_substrings[substrings_added].cut_parameter,
             PARAMSIZE);
#else
      bcopy(&cut_substrings[substrings_added].cut_parameter, nextPtr,
            PARAMSIZE);
#endif

      nextPtr += PARAMSIZE;
      *(nextPtr++) = ',';

#ifdef WIN32
      memcpy(nextPtr, &cut_substrings[substrings_added].turtle_index,
             PARAMSIZE);
#else
      bcopy(&cut_substrings[substrings_added].turtle_index, nextPtr,
            PARAMSIZE); /* turtle index */
#endif

      nextPtr += PARAMSIZE;
      *(nextPtr++) = ')';

      curPtr += SKIPSIZE + 2;

      *(cut_substrings[substrings_added++].last) = '\0';

      /* JH Oct97 need to append any saved instances to the appropriate
      sub L-systems here */
    }

#ifdef JIM
    strtPredPtr = curPtr; /* record start of predecessor for later processing */
    strtSuccPtrOff = nextPtr - *string2;
    /* record start of successor (offset) for later processing */
#else
    strtPtr = curPtr;   /* record start of predecessor for later */
    /* processing */
#endif

    if (currentLsystem->stochastic) {
      /* Stochastic L-system */
      FindApplProductions(curPtr, currentLsystem, applSetPtr, &totalprob);
      prodPtr = SelectProd(applSetPtr, totalprob, curPtr, currentLsystem);
    } else {
      /* Deterministic */
      prodPtr = FindProd(curPtr, currentLsystem);
    }

    ApplyProd(&curPtr, &nextPtr, string2, string2end, prodPtr, currentLsystem,
              1);

    /* Mark end of string */
    *nextPtr = '\0';

#ifdef JIM
    /* JH Sept 97 handle sub-Lsystems in predecessor and or successor */
    /* To only do this when necessary, a flag could be set in ApplyProd to
     * indicate if a subLsystem appears in the predecessor or  successor, and in
     * SkipCutPart to handle cuts
     */
    strtSuccPtr = *string2 + strtSuccPtrOff;

    ProcessSubLsystems(strtPredPtr, curPtr, strtSuccPtr, nextPtr,
                       LsystemListPtr, &currentLsystem, endStep);

    /* JH Sept97 this is being included in the above addition for sub-Lsystem
     * instances */
#else
    /* Check matched predecessor for sub-Lsystem operators and
    switch production set if necessary */
    if (((*strtPtr == '?') && (*(strtPtr + 1) == '(')) || *strtPtr == '$' ||
        (prodPtr != NULL && prodPtr->lsystemChange)) {
      CheckPred(strtPtr, curPtr, LsystemListPtr, &currentLsystem);
    }
#endif
  }

  currentStringLength = nextPtr - *string2;

  /* Evaluate end each global statements */
#ifdef JIM
  ProcessStatements(LsystemListPtr->endEach);
  /* EvaluateEndEach(LsystemListPtr); */

  /* if last step, evaluate end statements for main Lsystem */
  if (endStep) {
    ProcessStatements(LsystemListPtr->endBlock);
  }
#else
  EvaluateEndEach(LsystemListPtr);
#endif

  /* remove substring array */
  if (clp.checkEnvironment) {
    Free(cut_substrings);
    cut_substrings = NULL;
  }

  if (drawparamPtr->environment_display)
    /* if 1, process environment before
    interpretation, i.e. changes in
    the enviroment are displayed
    in the current step */
    ProcessEnvironment(*string2, drawparamPtr, viewparamPtr);
}

/*********************************************************************/
/* Function: FindProd                                                */
/* Given a pointer to a string and a set of productions, return the  */
/* pointer to the first applicable production. The set of productions*/
/* is considered as ordered.                                         */
/*********************************************************************/

static Production *FindProd(char *curPtr, LSYSDATA *LsysPtr) {
  Production *prodPtr;
  int preflength; /* length of prefix string matched */

  /* start at first possible match */
  prodPtr = LsysPtr->firstProd[(int)(*curPtr)];

  while (prodPtr != NULL) {

    /* Check whether the strict predecessor, the right context and the left
       context match the string at the current position.  Check that the
       condition for this production applies (this updates the symbol table
       as a byproduct).  If there is no match, consider next production.
            If there is a match, return the pointer to the production.  */

    if (PredDiff(prodPtr->pred, curPtr, &preflength) ||
        RconDiff(prodPtr->rCon, curPtr + preflength, LsysPtr) ||
        LconDiff(prodPtr->lCon, curPtr - 1, LsysPtr) ||
        CondDiff(prodPtr, curPtr, LsysPtr))
      prodPtr = prodPtr->nextProduction;
    else
      return (prodPtr);
  }

  /* If no match has been found return a null value.  */
  return (NULL);
}

/*********************************************************************/
/* Function: SkipCutPart                                             */
/* Skips the cut substring . *str points to %.                       */
/*********************************************************************/

void SkipCutPart(char **str) {
  char *curPtr = *str;
  int level, slevel;

  level = 1;
  slevel = 1;

  for (;;) {
    switch (*(curPtr)) {
    case '(':
      curPtr += SKIPSIZE;
      while (*curPtr != ')') {
        /* skip parameter */
        curPtr += SKIPSIZE;
      }
      break;
    case '[':
      level++;
      break;
    case ']':
      if (--level == 0)
        goto out;
      break;
    case '?':
      if (*(curPtr + 1) == '(')
        slevel++;
      /* handling of sub l-system instances could happen here... */
      /* but then should not be in ProcessSubLsystems */
      /* grab first parameter, match L-system and delete instance if cut string
      not being saved */
      /* if sub lsystem being saved keep instance for later appending. */
      break;
    case '$':
      if (--slevel == 0)
        goto out;
      break;
    case '%':
      /* doesn't stop on % with no parameter */
      if (*(curPtr + 1) == '(')
        goto out;
      break;
    case '\0':
      goto out;
    }
    curPtr++;
  }
out:
  *str = curPtr;
}

/*********************************************************************/
void CopySymbol(char **curStrPtr, char **nextStrPtr, char **nextStrBeginPtr,
                char **nextStrEndPtr) {
  int modulelen, over;
  char *cur2Ptr, *curPtr = *curStrPtr;

  if (nextStrEndPtr != NULL)
    if ((over = (int)(*nextStrPtr + MAXPARMS * 5 + 2 - *nextStrEndPtr)) > 0)
      ReallocateString(nextStrBeginPtr, nextStrEndPtr, nextStrPtr, over);

  *((*nextStrPtr)++) = *curPtr;

  /* if present, move associated parameters */
  if (*(++curPtr) == '(') {
    cur2Ptr = curPtr;
    curPtr += SKIPSIZE;

    while (*curPtr != ')') {
      curPtr += SKIPSIZE;
    }

    modulelen = (int)(++curPtr - cur2Ptr);

#ifdef WIN32
    memcpy(*nextStrPtr, cur2Ptr, modulelen);
#else
    bcopy(cur2Ptr, *nextStrPtr, modulelen);
#endif

    *nextStrPtr += modulelen;
  }
  *curStrPtr = curPtr;
}

/*********************************************************************/
/* Recursively applies productions to the module, pointed to by *str.
   The result is stored in a string pointed to by *next.
   Productions are context-free and are applyied in depth-first fashion
   untill there is no matching production or a maximum depth is reached.
   Pointers leftcontext and rightcontext point before and after the decomposed
   module (in the current L-system string).
   */
/* cut symbol is ingnored for now */

#define PRODSTACKINCREMENT 100

static int ApplyRecursiveProductions(PRODDATA *productions, LSYSDATA *LsysPtr,
                                     char **str, char **next, char **nextBegin,
                                     char **nextEnd, char *leftcontext,
                                     char *rightcontext, int depth) {
  char *result, *endresult, *ptr;
  char *curPtr;
  Production *prodPtr;
  int preflength; /* length of prefix string matched */
  int i;
  extern LSYSDATA *LsystemList; /* Lsystem data - for the symbol table */

  /* Find a production according to *curPtr */
  curPtr = *str;

  /* start at first possible match */
  prodPtr = productions->firstProd[(int)(*curPtr)];

  while (prodPtr != NULL) {
    /* Check whether the strict predecessor matches the string at the */
    /* current position.  Check that the condition for this production */
    /* applies (this updates the symbol table as a byproduct).  If there */
    /* is no match, consider next production.  */

    if (PredDiff(prodPtr->pred, curPtr, &preflength) ||
        RconDiff(prodPtr->rCon, rightcontext, LsysPtr) ||
        LconDiff(prodPtr->lCon, leftcontext, LsysPtr) ||
        CondDiff(prodPtr, curPtr, LsystemList))
      /* context is taken from the original string */
      prodPtr = prodPtr->nextProduction;
    else
      break;
  }

  if (depth >= productions->depth) {
    if (productions->depth > 0) {
      CopySymbol(str, next, nextBegin, nextEnd);
      if (prodPtr != NULL && LsysPtr->decomp_warning) {
        if (0 != **str)
          Message("Warning: maximum depth in a decomposition reached"
                  " for module %c !\n",
                  **str);
        else {
          Message("Warning: maximum depth in a decomposition reached!\n");
          // [Pascal] we return false to avoid flickering"
          return 0;
        }
      }
      return 1;
    }

    return 0; /* depth 0 - act as if there is no decomposition */
  }

  if (prodPtr == NULL) {
    /* no matching production */
    if (depth == 0)
      return 0; /* no decomposition production found */

    /* higher depths */
    CopySymbol(str, next, nextBegin, nextEnd);
    return 1;
  }

  /* allocate the memory for the new string (as needed) */

  if (depth >= productions->stack_len) {
    if (productions->stack_len == 0) {
      assert(NULL == productions->stack);
      productions->stack = Malloc(PRODSTACKINCREMENT * sizeof(char *));
    } else
      productions->stack = Realloc(
          productions->stack,
          (productions->stack_len + PRODSTACKINCREMENT) * sizeof(char *));
    if (productions->stack == NULL) {
      Message("Error: not enough memory for production stack!\n");
      MyExit(-1);
    }

    for (i = 0; i < PRODSTACKINCREMENT; i++)
      productions->stack[productions->stack_len + i] = NULL;

    productions->stack_len += PRODSTACKINCREMENT;
  }

  if (productions->stack[depth] == NULL) {
    productions->stack[depth] = Malloc(productions->longest_succ + 1);
    if (productions->stack[depth] == NULL) {
      Message("Error: not enough memory for production stack!\n");
      MyExit(-1);
    }
  }

  result = productions->stack[depth];

  endresult = result + prodPtr->succLen;
  ptr = result; /* ApplyProd would otherwise change result pointer */

  /* Apply the selected production (NULL - doesn't need pointers to the string,
it will not need to reallocate */
  ApplyProd(str, &ptr, NULL, NULL, prodPtr, LsysPtr, 0);

  /* interpret the result */
  curPtr = result;
  *endresult = '\0';

  while (*curPtr != '\0') {
    ApplyRecursiveProductions(productions, LsysPtr, &curPtr, next, nextBegin,
                              nextEnd, leftcontext, rightcontext, depth + 1);
  }
  return 1;
}

/*********************************************************************/
/* Function: ApplyProd                                               */
/* Copy the successor of production *prodPtr starting at location    */
/* *nextStrPtr. Update curStrPtr and nextStrPtr.		     */
/*********************************************************************/
void ApplyProd(char **curStrPtr, char **nextStrPtr, char **nextStrBeginPtr,
               char **nextStrEndPtr, Production *prodPtr, LSYSDATA *LsysPtr,
               char use_decomposition) {
  int parameters;
  int index, over;
  char *next;
  char *curPtr = *curStrPtr;

  /* If a production exists then append the appropriate successor.     */
  if (prodPtr != NULL) {
    next = NULL;

    if (use_decomposition && LsysPtr->Decomposition.specified) {
      /* successor goes to one_successor */
      if (one_successor == NULL) {
        if ((one_successor = (char *)Malloc(prodPtr->succLen + 1)) == NULL) {
          Warning("Cannot allocate memory for single successor\n"
                  "Decomposition productions not applied!\n",
                  WARNING_LVL);
          LsysPtr->Decomposition.specified = 0;
        } else {
          one_successor_len = prodPtr->succLen + 1;
          next = one_successor;
        }
      } else if (one_successor_len < prodPtr->succLen + 1) {
        if ((one_successor = (char *)Realloc(one_successor,
                                             prodPtr->succLen + 1)) == NULL) {
          Warning("Cannot reallocate memory for single successor\n"
                  "Decomposition productions not applied!\n",
                  WARNING_LVL);
          LsysPtr->Decomposition.specified = 0;
        } else {
          one_successor_len = prodPtr->succLen + 1;
          next = one_successor;
        }
      } else
        next = one_successor;
    }

    if (next == NULL) {
      /* if no decomposition or malloc on one_successor failed */
      /* check the room */
      if (nextStrEndPtr != NULL)
        if ((over = (int)(*nextStrPtr + prodPtr->succLen + 2 -
                          *nextStrEndPtr)) > 0)
          ReallocateString(nextStrBeginPtr, nextStrEndPtr, nextStrPtr, over);

      next = *nextStrPtr;
    }

    /* evaluate postcondition statements */
    ProcessStatements(prodPtr->postCondList);
    /* evaluate and append successor modules */
    AppendSuccessor(&next, prodPtr);
    /* Move to the end of the matched string */
    (*curStrPtr) += prodPtr->predLen;

    if (use_decomposition && LsysPtr->Decomposition.specified) {
      /* apply decomposition to all successor */
      *next = 0;
      next = one_successor;

      while (*next != 0)
        if (!ApplyRecursiveProductions(
                &LsysPtr->Decomposition, LsysPtr, &next, nextStrPtr,
                nextStrBeginPtr, nextStrEndPtr,
                *curStrPtr - 1 - prodPtr->predLen, *curStrPtr, 0))
          /* deal with % here */
          CopySymbol(&next, nextStrPtr, nextStrBeginPtr, nextStrEndPtr);
    } else
      *nextStrPtr = next;

    return; /* done */
  }

  /* prodPtr is NULL */

  if (use_decomposition && LsysPtr->Decomposition.specified) {
    /* try to apply decomposition productions */
    char *ptr = *curStrPtr;
    StringModule module;

    NextStringModule(&ptr, &module);

    if (ApplyRecursiveProductions(&LsysPtr->Decomposition, LsysPtr, curStrPtr,
                                  nextStrPtr, nextStrBeginPtr, nextStrEndPtr,
                                  *curStrPtr - 1, ptr, 0))
      return;
  }

  /* otherwise, check for % or simply copy the module */
  if (*curPtr == '%') {
    parameters = 0;

    if (*(curPtr + 1) == '(') {

      if (*(curPtr + 1 + SKIPSIZE) == ')') {
        parameters = 1;

        /* get the proper index of the substring */
#ifdef WIN32
        memcpy(&index, curPtr + 2, PARAMSIZE);
#else
        bcopy(curPtr + 2, &index, PARAMSIZE);
#endif

        /* in case of one parameter, remember the position of a substring */
        /* to be added at the end of the string */
        cut_substrings[index].first = curPtr;
      } else
        parameters = 2;
    }

    if (parameters <= 1) {
      /* If the cut symbol appears and was not matched by a production */
      /* skip symbols to the end of the current branch or string or  */
      /* sub-L-system (marked by a $) */
      /* The substring is either removed (parameters==0) or processed at */
      /* the end  (parameters==1) */
      curPtr++;

      SkipCutPart(&curPtr);
      *curStrPtr = curPtr; /* update the value (nextStrPtr not changed) */

      if (parameters == 1)
        cut_substrings[index].last = curPtr;

      return;
    }

    /* otherwise, leave % with two or more parameters as it is */
  }

  /* If a production does not exist, append the current symbol and     */
  /* associated parameters as if a production symbol --> symbol        */
  /* was found (thus there is no need for such productions).           */

  CopySymbol(curStrPtr, nextStrPtr, nextStrBeginPtr, nextStrEndPtr);
}

/*********************************************************************/
/* Check matched predecessor for sub-Lsystem operators and process   */
/*********************************************************************/
#ifndef JIM
static void CheckPred(char *strtPtr, char *curPtr, LSYSDATA *LsystemListPtr,
                      LSYSDATA **currentLsystem) {
  StringModule ts; /* Structure for symbol and parameters in the string */
#ifdef JIM
  SymbolInstance *newInstance;
#endif

  while (strtPtr < curPtr) {
    switch (*strtPtr) {
    case '?': /* new sub-L-system; push stack */
      if (*(strtPtr + 1) == '(') {
        /* otherwise it's question */
        if (lstackIndex++ >= MAXNESTING) {
          Message("ERROR: Sub-L-system stack overflow.\n");
          MyExit(1);
        }
        /* determine new sub L-system; id in first parameter */
        (void)NextStringModule(&strtPtr, &ts);
        *currentLsystem =
            MatchingLsystem((int)ts.actual[0].value, LsystemListPtr);
        lsystemStack[lstackIndex] = *currentLsystem;
#ifdef JIM
        /* move to next instance and assign to symbols */
        newInstance = (*currentLsystem)->currentInstance->nextSymbolInstance;
        if (newInstance == NULL) {
          Message("INTERNAL ERROR: Instance not available for sub-Lsystem %d\n",
                  (int)ts.actual[0].value);
          MyExit(1);
        }
        (*currentLsystem)->currentInstance = newInstance;
        /* point symbol table variables at appropriate instance */
        AssignSymbolsToInstances((*currentLsystem)->symbolTable, newInstance);
#endif
      }
      /* should go to default  */
      goto deft;
    case '$': /* end of sub-L-system; pop stack */
      if (lstackIndex-- < 0) {
        Message("WARNING: Mismatched ? and $.\n");
        lstackIndex = 0;
      }
      *currentLsystem = lsystemStack[lstackIndex];
      /* no break here - strtPtr should be moved as in default case */
    default:;
    deft:
      strtPtr++;
      /*** This used to read: if (*strtPtr == '(') moveright(strtPtr); til I
       * linted*/
      if (*strtPtr == '(' && strtPtr < curPtr)
        strtPtr = moveright(strtPtr);
    }
  }
}
#endif

#ifdef JIM
/*********************************************************************/
/* Check matched predecessor and resulting successor for */
/* sub-Lsystem operators and process   */
/* handles L-system stack as well as instances  */
/* only the first sub-L-system  of predecessor and successor */
/* are checked to see if they match  */
/* if endStep is true, end statements should be run */
/*********************************************************************/
static void ProcessSubLsystems(char *strtPredPtr, char *curPtr,
                               char *strtSuccPtr, char *nextPtr,
                               LSYSDATA *LsystemListPtr,
                               LSYSDATA **currentLsystem, int endStep) {
  StringModule predModule,
      succModule; /* Structure for symbol and parameters in the string */
  SymbolInstance *newInstance, *oldInstance;
  LSYSDATA *newLsystem;
  char *firstSuccSub;
  int saveStackIndex, checkIndex;

  /* is this handled in Skipcutpart called from Derive??? */
  if (*strtPredPtr == '%') {
    /* handle sub-L-systems that are being cut */
    if (*(strtPredPtr + 1) == '(') {
      /* note that % with 2 parameters is one of the ones thats been saved */
      Message("WARNING: Sub-L-system handling not implemented for retained cut "
              "strings.\n");
    }
    /* save stack index to give warning of unmatched subLsystems being cut */
    saveStackIndex = lstackIndex;

    while (strtPredPtr < curPtr) {
      switch (*strtPredPtr) {
      case '?':
        /* only process if its not a query  */
        if (*(strtPredPtr + 1) == '(') {
          /* push stack */
          if (lstackIndex++ >= MAXNESTING) {
            Message("ERROR: Sub-L-system stack overflow.\n");
            MyExit(1);
          }
          /* determine sub L-system; id in first parameter */
          (void)NextStringModule(&strtPredPtr, &predModule);
          *currentLsystem =
              MatchingLsystem((int)predModule.actual[0].value, LsystemListPtr);
          lsystemStack[lstackIndex] = *currentLsystem;

          /* move to next instance and assign to symbols */
          newInstance = (*currentLsystem)->currentInstance->nextSymbolInstance;
          if (newInstance == NULL) {
            Message(
                "INTERNAL ERROR: Instance not available for sub-Lsystem %d\n",
                (int)predModule.actual[0].value);
            MyExit(1);
          }
          (*currentLsystem)->currentInstance = newInstance;
          /* point symbol table variables at appropriate instance */
          AssignSymbolsToInstances((*currentLsystem)->symbolTable, newInstance);
        }
        break;
      case '$': /* end of sub-L-system; pop stack */
        if (lstackIndex == saveStackIndex) {
          Message("WARNING: Cut of unmatched $.\n");
        } else {
          /* run end each and end statements */
          ProcessStatements((*currentLsystem)->endEach);
          ProcessStatements((*currentLsystem)->endBlock);
          /* remove from instance list and delete */
          oldInstance = (*currentLsystem)->currentInstance;
          (*currentLsystem)->currentInstance = oldInstance->prevSymbolInstance;
          RemoveInstance(oldInstance);
          FreeSymbolInstance(oldInstance);
          /* NB should be handled added to end of list if cut symbols are
           * retained  */
          if (lstackIndex-- < 0) {
            Message("WARNING: Cut of unmatched $.\n");
            lstackIndex = 0;
          }
          *currentLsystem = lsystemStack[lstackIndex];
        }
        break;
      default:;
      }

      strtPredPtr++;
      if (*strtPredPtr == '(' && strtPredPtr < curPtr)
        strtPredPtr = moveright(strtPredPtr);
    }
    /* handle any extra stacked L-systems */
    while (lstackIndex > saveStackIndex) {
      Message("WARNING: Unmatched ? in cut segment.\n");
      /* run end each and end statements for currentLsystem */
      ProcessStatements((*currentLsystem)->endEach);
      ProcessStatements((*currentLsystem)->endBlock);
      /* remove from instance list and delete */
      oldInstance = (*currentLsystem)->currentInstance;
      (*currentLsystem)->currentInstance = oldInstance->prevSymbolInstance;
      RemoveInstance(oldInstance);
      FreeSymbolInstance(oldInstance);
      /* NB should be handled added to end of list if cut symbols are retained
       */
      if (lstackIndex-- < 0) {
        Message("WARNING: Cut of unmatched $.\n");
        lstackIndex = 0;
      }
      *currentLsystem = lsystemStack[lstackIndex];
    }
  }
  /*********************************************/
  else {
    /* process non-cut strings */
    /* find the first sub-L-system start symbol in the successor */
    firstSuccSub = NULL;
    while (strtSuccPtr < nextPtr &&
           !(*strtSuccPtr == '?' && *(strtSuccPtr + 1) == '(')) {
      strtSuccPtr++;
      if (*strtSuccPtr == '(' && strtSuccPtr < nextPtr)
        strtSuccPtr = moveright(strtSuccPtr);
    }
    /* store for later use if found */
    if (strtSuccPtr != nextPtr)
      firstSuccSub = strtSuccPtr;

    /* process predecessor */
    while (strtPredPtr < curPtr) {
      switch (*strtPredPtr) {
      case '?':
        /* only process if it's not a query */
        if (*(strtPredPtr + 1) == '(') {
          /* push stack */
          if (lstackIndex++ >= MAXNESTING) {
            Message("ERROR: Sub-L-system stack overflow.\n");
            MyExit(1);
          }
          /* determine new sub L-system; id in first parameter */
          (void)NextStringModule(&strtPredPtr, &predModule);
          *currentLsystem =
              MatchingLsystem((int)predModule.actual[0].value, LsystemListPtr);
          lsystemStack[lstackIndex] = *currentLsystem;

          /* mark recursive calls as an error for the time being */
          checkIndex = lstackIndex - 1;
          while (checkIndex >= 0) {
            if (lsystemStack[checkIndex]->id == (*currentLsystem)->id) {
              Message("ERROR: Recursive call to Sub-L-system %d.\n",
                      (*currentLsystem)->id);
              MyExit(1);
            }
            checkIndex--;
          }

          /* move to next instance and assign to symbols */
          newInstance = (*currentLsystem)->currentInstance->nextSymbolInstance;
          if (newInstance == NULL) {
            Message(
                "INTERNAL ERROR: Instance not available for sub-Lsystem %d\n",
                (int)predModule.actual[0].value);
            MyExit(1);
          }
          (*currentLsystem)->currentInstance = newInstance;
          /* point symbol table variables at appropriate instance */
          AssignSymbolsToInstances((*currentLsystem)->symbolTable, newInstance);

          /* run start statements and array initializers if the first time
          through and mark done */
          if (newInstance->initFlag == FALSE) {
            ProcessStatements((*currentLsystem)->defineBlock);
            ProcessStatements((*currentLsystem)->startBlock);
            newInstance->initFlag = TRUE;
          }
          /* run startEach */
          ProcessStatements((*currentLsystem)->startEach);

          /* check against first successor sub-L-system for match */
          if (firstSuccSub != NULL) {
            /* determine successor sub L-system; id in first parameter */
            (void)NextStringModule(&firstSuccSub, &succModule);
            if (predModule.actual[0].value == succModule.actual[0].value) {
              /* if matching leave instances as is; do move right on succ */
              if (strtSuccPtr < nextPtr) {
                /* if you can */
                strtSuccPtr++;
                if (*strtSuccPtr == '(' && strtSuccPtr < nextPtr)
                  strtSuccPtr = moveright(strtSuccPtr);
              }
            } else {
              firstSuccSub = NULL; /* mark non-match */
            }
          }
          if (firstSuccSub == NULL) {
            /* if no match, mark for delete */
            (*currentLsystem)->currentInstance->deleteFlag = TRUE;
          }
        }
        break;
      case '$': /* end of sub-L-system; pop stack */
        /* run end each statements for currentLsystem */
        if (lstackIndex-- <= 0) {
          Message("WARNING: Mismatched ? and $.\n");
          lstackIndex = 0;
          break;
        }
        ProcessStatements((*currentLsystem)->endEach);
        /* handle deletion */
        if ((*currentLsystem)->currentInstance->deleteFlag == TRUE) {
          /* run end statements for currentLsystem */
          ProcessStatements((*currentLsystem)->endBlock);
          /* remove from instance list and delete */
          oldInstance = (*currentLsystem)->currentInstance;
          (*currentLsystem)->currentInstance = oldInstance->prevSymbolInstance;
          RemoveInstance(oldInstance);
          FreeSymbolInstance(oldInstance);
        }
        /* else if end step, run end statements for currentLsystem */
        else if (endStep) {
          ProcessStatements((*currentLsystem)->endBlock);
        }
        *currentLsystem = lsystemStack[lstackIndex];
        break;
      case '%': /* cut symbols other than 1st aren't handled yet */
        Message("WARNING: Sub-L-system handling not implemented for cut.\n");
        break;
      default:;
      }
      strtPredPtr++;
      if (*strtPredPtr == '(' && strtPredPtr < curPtr)
        strtPredPtr = moveright(strtPredPtr);
    }
  }

  /* handle successor subLsystems */
  /* go through successor string and insert any new instances */
  /* note that first may already have been handled if it matched
   * that in predecessor */
  /* also note that no check for proper nesting is done here */

  while (strtSuccPtr < nextPtr) {
    if (*strtSuccPtr == '?' && *(strtSuccPtr + 1) == '(') {
      /* only process if it's not a query */
      /* determine new sub L-system; id in first parameter */
      (void)NextStringModule(&strtSuccPtr, &succModule);
      newLsystem =
          MatchingLsystem((int)succModule.actual[0].value, LsystemListPtr);

      /* insert new instance */
      newInstance = CopySymbolInstance(newLsystem->instanceList);
      if (newInstance == NULL) {
        Message("INTERNAL ERROR: Instance not created for sub-Lsystem %d\n",
                (int)succModule.actual[0].value);
        MyExit(1);
      }
      InsertInstance(newInstance, newLsystem->currentInstance);
      /* mark as currentInstance, to ensure proper handling in order */
      /* ie the instance after this is the one to be used for the
       * next reference in this generation */
      newLsystem->currentInstance = newInstance;
      /* mark initialisation not done */
      newInstance->initFlag = FALSE;
    }
    strtSuccPtr++;
    if (*strtSuccPtr == '(' && strtSuccPtr < nextPtr)
      strtSuccPtr = moveright(strtSuccPtr);
  }
}
#endif

/*********************************************************************/
/* Match Lsystem id from ? with input L-system ids and return index  */
/*********************************************************************/
LSYSDATA *MatchingLsystem(int Lsysid, LSYSDATA *LsystemListPtr) {
  LSYSDATA *lsystemPtr;

  /* Compare all L-system names for possible match */
  lsystemPtr = LsystemListPtr;
  while (lsystemPtr != NULL) {
    if (lsystemPtr->id == Lsysid) {
      return lsystemPtr;
    }
    lsystemPtr = lsystemPtr->nextLsystem;
  }

  Message("ERROR: L-system %d not in .l file.\n", Lsysid);
  MyExit(1);
  return NULL;
}

/*********************************************************************/
/* Function: AppendSuccessor                                         */
/* Evaluate the successor of the production pointed to by prodPtr    */
/* and append the modules after strPtr.				     */
/*********************************************************************/

static void AppendSuccessor(char **strPtr, Production *prodPtr) {
  Module *succPtr;        /* pointer for scanning successor list */
  Parameter *currentParm; /* pointer for successor parameters */

  /* Extract symbols and parameters from the successor of the production
and, after evaluation, append to the successor string */
  succPtr = prodPtr->succ;
  while (succPtr != NULL) {
    /* append symbol */
    **strPtr = succPtr->symbol;
    (*strPtr)++;
    currentParm = succPtr->parmList;

    /* Evaluate and append the parameter string if present */
    if (currentParm != NULL) {
      **strPtr = '(';
      while (currentParm != NULL) {
        (*strPtr)++;
        AppendParameter(strPtr, Eval(currentParm->ptrTo.expression));
        **strPtr = ',';
        currentParm = currentParm->nextParameter;
      }
      **strPtr = ')';
      (*strPtr)++;
    }

    /* Get next successor symbol */
    succPtr = succPtr->nextModule;
  }
}

/*********************************************************************/
/* Function: PredDiff                                                */
/* Check whether module list predPtr `matches'  a prefix of the      */
/* string strPtr                                                     */
/* At each position until the "end" of predStr check that the symbol */
/* and the number of parameters match.                               */
/* If they match, return a 0, else return a 1.                       */
/* Pass back the length of the sub-string matched.                   */
/*********************************************************************/

int PredDiff(Module *predPtr, char *strPtr, int *length) {
  StringModule ts; /* Structure to describe the modules in the string */
  *length = 0;
  while (predPtr != NULL) {
    if (predPtr->symbol != NextStringModuleForMatch(&strPtr, &ts) ||
        predPtr->parameters != ts.parameters)
      return (1);
    *length += ts.length;
    predPtr = predPtr->nextModule;
  }
  return (0);
}

/*********************************************************************/
/* Function: CondDiff                                           */
/* Given the production and matching string, check that the          */
/* condition is satisfied. This involves extracting variable values  */
/* and evaluation of the condition expression.                       */
/* A byproduct of this process is the creation of the symbol table.	 */
/* If it is satisfied return a 0, else return a 1.                   */
/*********************************************************************/

int CondDiff(Production *prodPtr, char *str, LSYSDATA *LsysPtr) {
  double temp;

  /* Set up variable table */
  /* prodPtr structure will reflect lengths of matching strings */
  CreateVarTable(prodPtr, str, LsysPtr);

  /* Evaluate pre-condition statements */
  ProcessStatements(prodPtr->preCondList);

  temp = Eval(prodPtr->condition);
  return (!temp);
}

/***********************************************************************/
/* Function: PrintExpression                                           */
/* Print a linked list of tokens                                       */
/***********************************************************************/

void PrintExpression(EToken *start) {
  EToken *ptr;
  char comma[2] = ",";
  char *str;

  ptr = start;
  if (ptr->up == NULL) {
    Message("%s", start->tokenString);
    return;
  }

  for (;;) {
  firstparam:
    if (strcmp(ptr->up->tokenString, "function") == 0) {
      Message("%s", ptr->up->tokenString);
      str = comma;
    } else
      str = ptr->up->tokenString;

    Message("(");
    if (ptr->nextParam == NULL) {
      /* unary */
      if (str[0] != ',')
        Message("%s", str);
      Message("%s", ptr->tokenString);
    } else {
      while (ptr->nextParam != NULL) {
        Message("%s", ptr->tokenString);
        Message("%s", str); /* operator */
        if (ptr->up != ptr->nextParam->up) {
          ptr = ptr->nextParam;
          if (strcmp(ptr->up->tokenString, "function") == 0) {
            Message("%s", ptr->up->tokenString);
            str = comma;
          } else
            str = ptr->up->tokenString;
          /* beginning of a new operator */
          goto firstparam;
        } else
          ptr = ptr->nextParam;
      }
      /* last parameter */
      Message("%s", ptr->tokenString);
    }

    while (ptr->nextParam == NULL) {
      if ((ptr = ptr->up) == NULL)
        return;
      Message(")");
    }
    Message("%s", ptr->up->tokenString);
    ptr = ptr->nextParam;
  }
}

/***********************************************************************/
/* Function: PrintParameterList                                        */
/* Print a linked list of parameters                                   */
/***********************************************************************/

static void PrintParameterList(Parameter *start) {
  Parameter *ptr;
  ptr = start;
  Message("(");
  while (ptr != NULL) {
    switch (ptr->type) {
    case 'f': /* formal parameter */
      Message("%s", ptr->ptrTo.symbol->label);
      break;
    case 'e': /* expression parameter */
      PrintExpression(ptr->ptrTo.expression);
      break;
    default:
      Message("ERROR: Invalid parameter type.\n");
      MyExit(1);
    }
    ptr = ptr->nextParameter;
    if (ptr != NULL)
      Message(",");
  }
  Message(")");
}

/***********************************************************************/
/* Function: PrintModuleList                                           */
/* Print a linked list of modules                                      */
/***********************************************************************/

void PrintModuleList(Module *start) {
  Module *ptr;
  ptr = start;
  while (ptr != NULL) {
    Message("%c", ptr->symbol);
    if (ptr->parmList != NULL)
      PrintParameterList(ptr->parmList);
    ptr = ptr->nextModule;
  }
  Message("\n");
}

/***********************************************************************/
/* Function: PrintStatementList                                        */
/* Print a linked list of statements                                   */
/***********************************************************************/

void PrintStatementList(Statement *start) {
  Statement *ptr;
  ptr = start;
  Message("{ ");
  while (ptr != NULL) {
    /* will require a switch when multiple type statements are allowed */
    switch (ptr->type) {
    case stmntASSIGN:
      Message("%s = ", ptr->leftHandSide->symbol->label);
      PrintExpression(ptr->expression);
      Message(";\n");
      break;
    case stmntPROC:
      PrintExpression(ptr->expression);
      Message(";\n");
      break;
    case stmntIF:
      Message("if(");
      PrintExpression(ptr->condition);
      Message("){\n");
      PrintStatementList(ptr->block);
      Message("}\n");
      if (ptr->elseblock) {
        Message("else {\n");
        PrintStatementList(ptr->elseblock);
        Message("}\n");
      }
      break;
    case stmntWHILE:
      Message("while(");
      PrintExpression(ptr->condition);
      Message("){\n");
      PrintStatementList(ptr->block);
      Message("}\n");
      break;
    case stmntDO:
      Message("do {\n");
      PrintStatementList(ptr->block);
      Message("} while(");
      PrintExpression(ptr->condition);
      Message(");\n");
      break;
    case stmntARRAYDEF:
      Message("array ");
      PrintExpression(ptr->expression);
      Message(";\n");
      break;
    case stmntARRAYASSIGN:
      break;
    }
    ptr = ptr->nextStatement;
  }
  Message("}\n");
}

/************************************************************************/
/* Function: PackedModuleListLength                                     */
/* Returns the length a module list will be after conversion to packed  */
/************************************************************************/

int PackedModuleListLength(Module *start) {
  Module *current;
  int packedLength;

  packedLength = 0;
  current = start;
  while (current != NULL) {
    packedLength += current->length;
    current = current->nextModule;
  }
  return (packedLength);
}

/************************************************************************/
/* Function: AssignParameters                                           */
/* Assign the parameter list to the module updating length and          */
/* parameter count.                                                     */
/************************************************************************/

Module *AssignParameters(Module *newModule, Parameter *parameterList) {
  Parameter *parameterPtr;
  parameterPtr = newModule->parmList = parameterList;
  while (parameterPtr != NULL) {
    newModule->parameters++;
    newModule->length += SKIPSIZE;
    parameterPtr = parameterPtr->nextParameter;
  }
  newModule->length++;

  return (newModule);
}

/************************************************************************/
/* Function: CountParameters                                            */
/* Return the number of parameters.					*/
/************************************************************************/

int CountParameters(Parameter *parameterList) {
  Parameter *parameterPtr;
  int count = 0;

  parameterPtr = parameterList;
  while (parameterPtr != NULL) {
    count++;
    parameterPtr = parameterPtr->nextParameter;
  }
  return (count);
}

/************************************************************************/
/* Function: MatchParameters                                            */
/* Check that the number and type of parameters is appropriate for	*/
/* The function being called						*/
/* When result=1 a number of parameters is inserted as a last parameter */
/************************************************************************/

int MatchParameters(Parameter *parameterList, int functionid) {
  Parameter *paramPtr;
  EToken *tokenPtr;
  int count;
  char ok = 1;
  int result = 0;

  if (NULL == parameterList)
    count = 0;
  else
    count = CountParameters(parameterList);

  switch (functionid) {
  case tINBLOB:
    ok = (count == 4);
    break;
  case tBRAN:
  case tBIRAN:
  case tNRAN:
    ok = (count == 2);
    break;
  case tPRINT:
    ok = (count >= 1);
    if (ok) {
      result = 1;
      /* check that parameters are valid */
      paramPtr = parameterList;
      if (paramPtr->ptrTo.expression->token != tSTRING) {
        Warning("printf requires format string as second parameter", ERROR_LVL);
      }
      paramPtr = paramPtr->nextParameter;
      while (paramPtr != NULL) {
        if (paramPtr->ptrTo.expression->token == tSTRING) {
          Warning("printf cannot print string parameters", ERROR_LVL);
          result = 0;
        }
        paramPtr = paramPtr->nextParameter;
      }
    }
    break;

  case tFOPEN:
    if (count != 2) {
      Warning("Fopen requires filename and type string", ERROR_LVL);
    }
    return 1;
#if CPFG_VERSION >= 4000
  case tFUNC:
    ok = (2 == count);
    break;
  case tPLAY:
    ok = (1 == count);
    break;
#endif
#if CPFG_VERSION >= 6400
  case tCURVEX:
    ok = (2 == count);
    break;
  case tCURVEY:
    ok = (2 == count);
    break;
  case tCURVEZ:
    ok = (2 == count);
    break;
  case tCURVEGAL:
    ok = (1 == count);
    break;
#endif
#if CPFG_VERSION >= 6600
  case tVVXMIN:
  case tVVYMIN:
  case tVVZMIN:
  case tVVXMAX:
  case tVVYMAX:
  case tVVZMAX:
  case tVVSCALE:
    ok = (1 == count);
    break;
  case tDISPLAY:
    ok = (1 == count);
    break;
#endif
  case tFSCANF:
  case tFPRINTF:
    if (count < 2) {
      Warning("Fscanf/fprintf requires file ref and format", ERROR_LVL);
    } else {
      /* check that parameters are valid */
      paramPtr = parameterList;
      if (paramPtr->ptrTo.expression->token != tNAME) {
        Warning(
            "Fscanf/fprintf requires file pointer variable as first parameter",
            ERROR_LVL);
      }
      paramPtr = paramPtr->nextParameter;
      if (paramPtr->ptrTo.expression->token != tSTRING) {
        Warning("Fscanf/fprintf requires format string as second parameter",
                ERROR_LVL);
      }
      paramPtr = paramPtr->nextParameter;
      while (paramPtr != NULL) {
        tokenPtr = paramPtr->ptrTo.expression;
        while (tokenPtr->up != NULL) {
          tokenPtr = tokenPtr->up;
        }
        if (functionid == tFSCANF)
          if (tokenPtr->token != tNAMELVAL && tokenPtr->token != tARRAYLVAL) {
            Warning("Fscanf requires addresses of variables", ERROR_LVL);
          }
        paramPtr = paramPtr->nextParameter;
      }
    }
    return 1;

  default:
    if (count != 1) {
      Warning("Too many Parameters", WARNING_LVL);
    }
  }

  if (!ok)
    Warning("Incorrect Number of Parameters", WARNING_LVL);
  return result;
}

/********************************************************************/
/* Function: NextStringModule                                       */
/* Extract the next symbol and parameters from an Lsystem           */
/* string at position str, and return the symbol                    */
/********************************************************************/

char NextStringModule(char **str, StringModule *module) {
  int parameter;
  char *ptr = *str; /* local for *str */
  /* get the current symbol and set up the basic token structure */
  module->symbol = *(ptr++);
  module->length = 1;
  module->parameters = 0;

  /* extract the parameters if present */
  if (*ptr == '(') {
    parameter = module->parameters++; /* use last value for index*/
    module->length += SKIPSIZE;       /* 1 for '(' and length of float */
#ifdef WIN32
    memcpy(module->actual[parameter].bytes, ++ptr, PARAMSIZE);
#else
    bcopy(++ptr, module->actual[parameter].bytes, PARAMSIZE);
#endif
    ptr += PARAMSIZE;

    while (*ptr == ',') {
      parameter = module->parameters++; /* use last value for index */
      if (parameter >= MAXPARMS) {
        Message("ERROR: Too many parameters in module.\n");
        MyExit(1);
      }
      module->length += SKIPSIZE;

#ifdef WIN32
      memcpy(module->actual[parameter].bytes, ++ptr, PARAMSIZE);
#else
      bcopy(++ptr, module->actual[parameter].bytes, PARAMSIZE);
#endif
      ptr += PARAMSIZE;
    }
    if (*ptr != ')') {
      Message("ERROR: Unmatched parenthesis in NextStringModule.\n");
      MyExit(1);
    }
    ptr++;
    module->length++;
  }

  *str = ptr; /* update *str */
  return (module->symbol);
}

/********************************************************************/
/* Function: NextStringModuleForMatch                               */
/* Extract the next symbol and parameter count from an Lsystem       */
/* string at position str, and return the symbol                    */
/********************************************************************/

static char NextStringModuleForMatch(char **str, StringModule *module) {
  char *ptr = *str;

  /* get the current symbol and set up the basic token structure */
  module->symbol = *(ptr++);
  module->length = 1;
  module->parameters = 0;

  if (module->symbol == 0)
    return 0;

  /* count the parameters if present */
  if (*ptr == '(') {
    module->parameters++;
    module->length += SKIPSIZE;
    ptr += SKIPSIZE;
    while (*ptr == ',') {
      module->parameters++;
      module->length += SKIPSIZE;
      ptr += SKIPSIZE;
    }
    if (*(ptr++) != ')') {
      Message("ERROR: Unmatched parenthesis in"
              "NextStringModuleForMatch (Module %c).\n",
              module->symbol);
      MyExit(1);
    }
    module->length++;
  }

  *str = ptr;
  return (module->symbol);
}

/*********************************************************************/
/* Set up the variable table */
/* The actual length of the matched predecessor is assigned to the	*/
/* length in the production structure for later movement of str */
/*********************************************************************/
static void CreateVarTable(Production *prodPtr, char *str,
                           __attribute__((unused)) LSYSDATA *LsysPtr) {

  /* Get predecessor variables */
  /* assign actual length value for predecessor length */
  prodPtr->predLen = GetPredVariables(prodPtr->pred, str);

  /* Get right context variables */
  GetRconVars(prodPtr->rCon);

  /* Get left context variables */
  GetLconVars(prodPtr->lCon);
}

/***********************************************************************/
/* Get positional parameters from string and match with variable names */
/* from production, then enter in the variable table.                  */
/**********************************************************************/
static int GetPredVariables(Module *predPtr, char *strPtr) {
  int length; /* length of string actually matched */

  length = 0;
  while (predPtr != NULL) {
    /* Get parameter strings from the production and string */
    length += ProcessSymbolVariables(predPtr, &strPtr);
    predPtr = predPtr->nextModule;
  }
  /* return actual length value of matched string */
  return (length);
}

/************************************************************************/
/* Get positional parameters from string and match with variable names	*/
/* then enter in the variable table. Return the length of the string	*/
/* processed.
 */
/************************************************************************/
static int ProcessSymbolVariables(Module *ptr, char **str) {
  StringModule ts;     /* Structure for symbol and parameters in the string */
  int parameter;       /* index to parameter being processed */
  Parameter *prodParm; /* pointer for production parameters */

  /* Get parameter strings from the production and string */
  prodParm = ptr->parmList;
  (void)NextStringModule(str, &ts);
  parameter = 0;

  /* Add variable names and values to the variable table */
  while (prodParm != NULL) {
#ifdef JIM
    *(GetSymbolValuePtr(prodParm->ptrTo.symbol)) =
        (double)ts.actual[parameter].value;
#else
    (prodParm->ptrTo.symbol)->value = (double)ts.actual[parameter].value;
#endif
    prodParm = prodParm->nextParameter;
    parameter++;
  }

  if (parameter != ts.parameters) {
    Message("ERROR: Mismatched parameter string %s.\n", *str);
    MyExit(1);
  }

  /* return actual length value of matched string */
  return (ts.length);
}

/***********************************************************************/
/* Add a formal parameter or constant to a symbol table                */
/***********************************************************************/
Symbol *SymbolTableAdd(char *label, double value, SymbolTable *symbolTable) {
  Symbol **currentSymbol;
#ifdef JIM
  SymbolInstance *currentInstance;
#endif
  char buffer[80];

  currentSymbol = symbolTable->firstEntry;
  while (*currentSymbol != NULL) {
    if (strcmp((*currentSymbol)->label, label) == 0) {
      sprintf(buffer, "Multiple definition of formal parameter %s", label);
      Warning(buffer, ERROR_LVL);
    }
    currentSymbol = &((*currentSymbol)->nextSymbol);
  }
  assert(NULL == *currentSymbol);
  if ((*currentSymbol = (Symbol *)Malloc(sizeof(Symbol))) == NULL) {
    Warning("Symbol memory allocation failed", FATAL_LVL);
  }
  (*currentSymbol)->type = DOUBLE; /* assign default type */ /* JH2 */
  (*currentSymbol)->arrayData = NULL;
  (*currentSymbol)->label = label;
  (*currentSymbol)->nextSymbol = NULL;
#ifdef JIM
  /* set value in default instance */
  currentInstance = *(symbolTable->instance);
  (*currentSymbol)->defaultOffset = (*currentSymbol)->offset =
      AllocateValueOffset(symbolTable, currentInstance, 1);
  (*currentSymbol)->values = currentInstance->values;
#ifdef DEBUG2
  (*currentSymbol)->valueSpace = currentInstance->valueSpace;
#endif
  *(GetSymbolValuePtr(*currentSymbol)) = value;
#else
  (*currentSymbol)->value = value;
#endif
  return (*currentSymbol);
}

/***********************************************************************/
/* Replace a value in a symbol table                                   */
/***********************************************************************/
Symbol *SymbolTableReplace(char *label, double value,
                           SymbolTable *symbolTable) {
  Symbol **currentSymbol;

  currentSymbol = symbolTable->firstEntry;
  while (*currentSymbol != NULL) {
    if (strcmp((*currentSymbol)->label, label) == 0) {
#ifdef JIM
      *(GetSymbolValuePtr(*currentSymbol)) = value;
#else
      (*currentSymbol)->value = value;
#endif
      /* free previously allocated label space since its not used */
      Free(label);
      label = NULL;
      return *currentSymbol;
    }
    currentSymbol = &((*currentSymbol)->nextSymbol);
  }
  return (Symbol *)NULL;
}

/************************************************************************/
/* Find a formal parameter or constant in the symbol tables             */
/************************************************************************/
Symbol *SymbolTableFind(char *label, SymbolTable *symbolTableStack) {
  SymbolTable *currentSymbolTable;
  Symbol *currentSymbol;

  currentSymbolTable = symbolTableStack;

  /* search all tables in the stack */
  while (currentSymbolTable != NULL) {
    currentSymbol = *(currentSymbolTable->firstEntry);
    while (currentSymbol != NULL) {
      if (strcmp(currentSymbol->label, label) == 0) {
        return (currentSymbol);
      }
      currentSymbol = currentSymbol->nextSymbol;
    }
    currentSymbolTable = currentSymbolTable->nextSymbolTable;
  }

  /* if not found, return a null pointer */
  return (Symbol *)NULL;
}

#ifdef JIM
/************************************************************************/
/* Find a formal parameter or constant in the current symbol table      */
/************************************************************************/
Symbol *CurrentSymbolTableFind(char *label, SymbolTable *currentSymbolTable) {
  Symbol *currentSymbol;

  currentSymbol = *(currentSymbolTable->firstEntry);
  while (currentSymbol != NULL) {
    if (strcmp(currentSymbol->label, label) == 0) {
      return (currentSymbol);
    }
    currentSymbol = currentSymbol->nextSymbol;
  }

  /* if not found, return a null pointer */
  return (Symbol *)NULL;
}

/************************************************************************/
/* Find a formal parameter or constant in the current symbol list     */
/************************************************************************/
Symbol *CurrentSymbolFind(char *label, Symbol *currentSymbol) {
  while (currentSymbol != NULL) {
    if (strcmp(currentSymbol->label, label) == 0) {
      return (currentSymbol);
    }
    currentSymbol = currentSymbol->nextSymbol;
  }

  /* if not found, return a null pointer */
  return (Symbol *)NULL;
}
#endif

/*********************************************************************/
/* Push new symbol table on current symbol table stack               */
/*********************************************************************/
SymbolTable *PushSymbolTable(SymbolTable *current, Symbol **firstEntry
#ifdef JIM
                             ,
                             SymbolInstance **instance
#endif
) {
  SymbolTable *newTable = NULL;

  if ((newTable = (SymbolTable *)Malloc(sizeof(SymbolTable))) == NULL) {
    Warning("Symbol table memory allocation failed", FATAL_LVL);
  }
  newTable->firstEntry = firstEntry;
#ifdef JIM
  newTable->instance = instance;
#endif
  newTable->nextSymbolTable = current;
  return newTable;
}

/*********************************************************************/
/* Pop symbol table from current symbol table stack                  */
/*********************************************************************/
SymbolTable *PopSymbolTable(SymbolTable *current) {
  SymbolTable *newTable;

  if (current == NULL)
    return (SymbolTable *)NULL;
  newTable = current->nextSymbolTable;
  Free(current);
  current = NULL;
  return newTable;
}

#ifdef JIM
/*********************************************************************/
/* insert new symbol table instance in list after position indicated */
/*********************************************************************/
SymbolInstance *InsertInstance(SymbolInstance *instance,
                               SymbolInstance *position) {
  if (position->nextSymbolInstance != NULL) {
    position->nextSymbolInstance->prevSymbolInstance = instance;
  }
  instance->nextSymbolInstance = position->nextSymbolInstance;
  position->nextSymbolInstance = instance;
  instance->prevSymbolInstance = position;
  return instance;
}

/*********************************************************************/
/* remove symbol table instance from  list */
/*********************************************************************/
void RemoveInstance(SymbolInstance *instance) {
  if (instance->nextSymbolInstance != NULL) {
    instance->nextSymbolInstance->prevSymbolInstance =
        instance->prevSymbolInstance;
  }
  if (instance->prevSymbolInstance != NULL) {
    instance->prevSymbolInstance->nextSymbolInstance =
        instance->nextSymbolInstance;
  }
}

/*********************************************************************/
/* create new symbol table value space allocation                  */
/*********************************************************************/
SymbolInstance *NewSymbolInstance(int size) {
  SymbolInstance *instancePtr = NULL;

  if (size == 0) {
    Message("New instance of size 0 requested - not good!\n");
    size = 4; /* Radek */
  }

  /* Allocate production symbol-table value storage */
  if ((instancePtr = (SymbolInstance *)Malloc(sizeof(SymbolInstance))) ==
      NULL) {
    Warning("Instance memory allocation failed", INTERNAL_LVL);
  }
  /* allocate space for values */
  if ((instancePtr->values = (double *)Malloc(sizeof(double) * size)) == NULL) {
    Warning("Instance values memory allocation failed", INTERNAL_LVL);
  }
  instancePtr->initFlag = FALSE;
  instancePtr->deleteFlag = FALSE;
  instancePtr->valueCount = 0;
  instancePtr->valueSpace = size;
  instancePtr->nextSymbolInstance = NULL;
  instancePtr->prevSymbolInstance = NULL;
  return instancePtr;
}

/*********************************************************************/
/* free symbol table instance                  */
/* can be merged into FreeInstanceList?? */
/*********************************************************************/
void FreeSymbolInstance(SymbolInstance *instance) {
  Free(instance->values);
  instance->values = NULL;
  Free(instance);
  instance = NULL;
}

/*********************************************************************/
/* free symbol table instance list                 */
/*********************************************************************/
void FreeInstanceList(SymbolInstance *instancePtr) {
  SymbolInstance *currentInstance;
  SymbolInstance *nextInstance;

  nextInstance = instancePtr;
  /* loop over statements */
  while (nextInstance != NULL) {
    currentInstance = nextInstance;
    nextInstance = currentInstance->nextSymbolInstance;
    FreeSymbolInstance(currentInstance);
  }
}

/*********************************************************************/
/* Make new instance of symbol table; copy values   */
/*********************************************************************/
SymbolInstance *CopySymbolInstance(SymbolInstance *instance) {
  SymbolInstance *instancePtr;

  if (instance == NULL)
    Warning("Instance values cannot be copied", INTERNAL_LVL);

  instancePtr = NewSymbolInstance(instance->valueCount);

  if (instancePtr == NULL)
    Warning("Instance values cannot be copied", INTERNAL_LVL);
  else {
    memcpy(instancePtr->values, instance->values,
           instance->valueCount * sizeof(double));
  }
  instancePtr->valueCount = instance->valueCount;

  VERBOSE("Copying instance values memory \n");

  return instancePtr;
}

/*********************************************************************/
/* Set symbols to point to appropriate instance, current or external */
/*********************************************************************/
void AssignSymbolsToInstances(Symbol *symbolPtr, SymbolInstance *instancePtr) {
  double *valuesPtr;
  int index, dim;
  Symbol *foundSymbol;

  /* point all symbols to the current instance */
  valuesPtr = instancePtr->values;
  while (symbolPtr != NULL) {
    /* assign to current instance; default for externals, correct for others */
    symbolPtr->values = valuesPtr;
    symbolPtr->offset = symbolPtr->defaultOffset;
    /* for external variables, check for a definition in the Lsystem stack
     * if its available */
    if ((symbolPtr->type == EXTERNAL || symbolPtr->type == EXTERNALARRAY)) {
      index = lstackIndex - 1; /* search below current position on the stack */
      foundSymbol = NULL;
      while (index >= 0 && lsystemStack[index] != NULL && foundSymbol == NULL) {
        foundSymbol = CurrentSymbolFind(symbolPtr->label,
                                        (lsystemStack[index])->symbolTable);
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
              Message("INTERNAL ERROR: Incorrectly-typed variable.\n");
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
              Message("INTERNAL ERROR: Incorrectly-typed variable.\n");
              MyExit(1);
            }
            break;
          default:
            Message("INTERNAL ERROR: Non-external variable.\n");
            MyExit(1);
          }
        }
        if (foundSymbol == NULL) {
          /* not found; keep looking */
          index = index - 1;
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

/*********************************************************************/
/* Append new copy of default instance values to instance list   */
/*********************************************************************/
SymbolInstance *AppendInstance(LSYSDATA *lsysPtr) {
  SymbolInstance *instancePtr, *newPtr;

  /* copy default table */
  newPtr = CopySymbolInstance(lsysPtr->instanceList);

  instancePtr = lsysPtr->instanceList;
  while (instancePtr->nextSymbolInstance != NULL) {
    instancePtr = instancePtr->nextSymbolInstance;
  }
  /* add copy to the instance list */
  InsertInstance(newPtr, instancePtr);

  return newPtr;
}

/*********************************************************************/
/*set L-system symbol table instance pointers to default, except for main   */
/*********************************************************************/
void InitSymbolInstances(LSYSDATA *lsysListPtr) {
  LSYSDATA *curLsystem;
  /* skip main L-system */
  curLsystem = lsysListPtr->nextLsystem;
  /* loop over Lsystems */
  while (curLsystem != NULL) {
    curLsystem->currentInstance = curLsystem->instanceList;
    curLsystem = curLsystem->nextLsystem;
  }
}

/*********************************************************************/
/* Allocates new value space in instance; returns offset  */
/* handles reallocation if required */
/*********************************************************************/
int AllocateValueOffset(SymbolTable *symbolTable, SymbolInstance *instance,
                        int space) {
  int newSize;
  Symbol *symbolPtr;

  if (instance->valueCount + space >= instance->valueSpace) {
    /* allocate new space for values */
    newSize = floor(instance->valueSpace * 1.5) + space - 1;
    if ((instance->values = (double *)Realloc(
             instance->values, sizeof(double) * newSize)) == NULL) {
      Warning("Instance values memory allocation failed", INTERNAL_LVL);
    }
    if (clp.warnings)
      Message("Resetting instance values memory from %d to %d\n",
              instance->valueSpace, newSize);
    instance->valueSpace = newSize;
    /* fix up previously allocated instances */
    symbolPtr = *(symbolTable->firstEntry);
    while (symbolPtr != NULL) {
      symbolPtr->values = instance->values;
      symbolPtr = symbolPtr->nextSymbol;
    }
  }
  instance->valueCount = instance->valueCount + space;
  return instance->valueCount - 1;
}
#endif

/*********************************************************************/
/* Append the bytes of the float value to str */
/*********************************************************************/
void AppendParameter(char **str, double value) {
  union {
    float value;
    char bytes[4];
  } actual;
  int i;
  actual.value = (float)value;
  for (i = 0; i < 4; i++) {
    **str = actual.bytes[i];
    (*str) += 1;
  }
}

/*********************************************************************/
/* Evaluate the postfix expression
   Each token (EToken) has two pointers - 'nextParam' pointing to a
   next parameter of the operation and 'up' pointing to the token with
   the operation specification:

                        (5+k)*(-m)
                                        * -->NULL
                                       ^ ^
                                      /   \
                                     /     \
                                    /       \
                                   /         - -->NULL
                                  /          ^
                                 /           |
                                /            |
                               + ----------> m -->NULL
                              ^ ^
                             /   \
                            /     \
                      ---> 5 ----> k -->NULL
*/
/*********************************************************************/
double Eval(EToken *expression) {
  double stack[MAXPARMLEN]; /* Stack for evaluating postfix expression */
  int stitem;               /* index for stack */

  char *strstack[MAXPARMLEN]; /* Stack for strings */
  int strstitem;              /* index for stack */

  double *adrstack[MAXPARMLEN]; /* Stack for symbol "addresses" */
  int adrstitem;                /* index for stack */

  double temp;               /* temporary value */
  double temp2;              /* temporary value */
  double args[MAXPARMS];     /* for functions with multiple args */
  double *argAdrs[MAXPARMS]; /*for functions with multiple args */
  int argcount;              /* for printf */
  char *format, *actpos;     /* for printf */
  int fileIndex;             /* for functions like fscanf */
  char *fileType, *fileName; /* for functions like fopen */
  int inttemp;               /* temporary value */
  int x;
  Array *arrayPtr; /* pointer to array information */
#ifdef JIM
  Symbol *symbolPtr;   /* temporary symbol pointer */
  double *arrayValues; /* pointer to array values */
#endif
  extern int swapinterval;

  /* Null expression evaluates as 0 */
  if (expression == NULL)
    return 0.0;

  /* single entry expression evaluates as symbol table value */
  if (expression->up == NULL) {
    if (expression->symbol) {
#ifdef JIM
      return *GetSymbolValuePtr(expression->symbol);
#else
      return (expression->symbol)->value;
#endif
    } else { /* this should never occur since strings are now constants
                returning their global index - JH1 */
      Message("Warning: String \"%s\" used as a number\n",
              expression->tokenString);
      return 0.0;
    }
  }

  stitem = 0;    /* Initialize stack */
  strstitem = 0; /* Initialize string stack */
  adrstitem = 0; /* Initialize address stack */

  for (;;) {
    for (;;) {
#ifdef JIM
      symbolPtr = expression->symbol;
#endif
      switch (expression->token) {
      case tSTRING:
        strstack[++strstitem] = expression->tokenString;
        break;
      case tNAMELVAL:
        /* stack "address" and its index */
        adrstitem++;
#ifdef JIM
        adrstack[adrstitem] = GetSymbolValuePtr(symbolPtr);
        ;
#else
        adrstack[adrstitem] = &(expression->symbol->value);
#endif
        stitem++;
        stack[stitem] = adrstitem;
        break;
      default:
#ifdef JIM
        stack[++stitem] = *(GetSymbolValuePtr(symbolPtr));
#else
        stack[++stitem] = (expression->symbol)->value;
#endif
      }
      if (expression->nextParam == NULL)
        break;
      expression = expression->nextParam;
    }

    do {
    goup:
      if ((expression = expression->up) == NULL)
        return stack[stitem];
      switch (expression->token) {
      case tOR:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] || temp);
        break;
      case tAND:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] && temp);
        break;
      case tGT:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] > temp);
        break;
      case tGE:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] >= temp);
        break;
      case tLT:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] < temp);
        break;
      case tLE:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] <= temp);
        break;
      case tEQUAL:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] == temp);
        break;
      case tNOTEQUAL:
        temp = stack[stitem--];
        stack[stitem] = (stack[stitem] != temp);
        break;
      case tPLUS:
        temp = stack[stitem--];
        stack[stitem] += temp;
        break;
      case tMINUS:
        temp = stack[stitem--];
        stack[stitem] -= temp;
        break;
      case tUMINUS:
        stack[stitem] *= (-1.0);
        break;
      case tTIMES:
        temp = stack[stitem--];
        stack[stitem] *= temp;
        break;
      case tDIVIDE:
        temp = stack[stitem--];
        if (temp == 0.0) {
          Message("Warning: division by 0.\n");
          temp = 1.0;
        }
        stack[stitem] /= temp;
        break;
      case tREM:
        /* % is only defined for integers */
        temp = stack[stitem--];
        inttemp = (int)stack[stitem];
        stack[stitem] = (double)(inttemp % (int)temp);
        break;
      case tPOW:
        temp = stack[stitem--];
        stack[stitem] = pow(stack[stitem], temp);
        break;
      case tNOT:
        stack[stitem] = (!stack[stitem]);
        break;
      case tQUESTION:
        temp = stack[stitem--];
        temp2 = stack[stitem--];
        stack[stitem] = (stack[stitem]) ? temp2 : temp;
        break;
      case tSIGN:
        stack[stitem] =
            ((stack[stitem] == 0.0) ? 0.0
                                    : ((stack[stitem] > 0.0) ? 1.0 : -1.0));
        break;
      case tSQRT: /* Radek */
        stack[stitem] = sqrt(stack[stitem]);
        break;
      case tATAN2: /* right now only one argument is stacked */
        temp = stack[stitem--];
        stack[stitem] = R_TO_D(atan2(temp, stack[stitem]));
        break;
      case tTAN:
        stack[stitem] = tan(D_TO_R(stack[stitem]));
        break;
      case tCOS:
        stack[stitem] = cos(D_TO_R(stack[stitem]));
        break;
      case tSIN:
        stack[stitem] = sin(D_TO_R(stack[stitem]));
        break;
      case tSRAND: {
        long int arg = (long int)stack[stitem];
        Do_srand(arg);
        stack[stitem] = 0.0f;
      } break;
      case tRAN:
        stack[stitem] = Do_ran(stack[stitem]);
        break;
      case tNRAN:
        temp = stack[stitem--];
        stack[stitem] = Do_nrand(stack[stitem], temp);
        break;
#if CPFG_VERSION >= 4000
      case tFUNC: {
        int id;
        double val;
        val = stack[stitem--];
        id = (int)stack[stitem];
        stack[stitem] = SplineFuncValue(id, val);
      } break;
      case tPLAY: {
        fileName = strstack[strstitem--];
#ifdef WIN32
        PlaySound(fileName, NULL, SND_FILENAME | SND_ASYNC);
#endif
      } break;
#endif /* CPFG_VERSION>=4000 */
#if CPFG_VERSION >= 6400
      case tCURVEX: {
        int id;
        double t;
        t = stack[stitem--];
        id = (int)stack[stitem];
        stack[stitem] = CurveCXS(id, 0, t, 0, 0);
      } break;
      case tCURVEY: {
        int id;
        double t;
        t = stack[stitem--];
        id = (int)stack[stitem];
        stack[stitem] = CurveCYS(id, 0, t, 0, 0);
      } break;
      case tCURVEZ: {
        int id;
        double t;
        t = stack[stitem--];
        id = (int)stack[stitem];
        stack[stitem] = CurveCZS(id, 0, t, 0, 0);
      } break;
      case tCURVEGAL: {
        int id;
        id = (int)stack[stitem];
        stack[stitem] = CurveGAL(id);
      } break;
#endif
#if CPFG_VERSION >= 6500
      case tGETDERIVLENGTH:
        assert(NULL != LsystemList);
        stack[stitem] = LsystemList->n;
        break;
      case tSETDERIVLENGTH:
        assert(NULL != LsystemList);
        LsystemList->n = (int)stack[stitem];
        stack[stitem] = 0;
        break;
#endif /* CPFG_VERSION>=6500 */
#if CPFG_VERSION >= 6600
      case tVVXMIN:
        stack[stitem] = viewWindow.left + viewparam.xPan;
        break;
      case tVVXMAX:
        stack[stitem] = viewWindow.right + viewparam.xPan;
        break;
      case tVVYMIN:
        stack[stitem] = viewWindow.bottom + viewparam.yPan;
        break;
      case tVVYMAX:
        stack[stitem] = viewWindow.top + viewparam.yPan;
        break;
      case tVVZMIN:
        stack[stitem] = viewparam.front_dist;
        break;
      case tVVZMAX:
        stack[stitem] = viewparam.back_dist;
        break;
      case tVVSCALE:
        stack[stitem] = viewparam.scale;
        break;
      case tDISPLAY:
        DisplayFrame(stack[stitem]);
        break;
#endif
      case tBRAN:
        temp = stack[stitem--];
        stack[stitem] = Do_bran(stack[stitem], temp);
        break;
      case tBIRAN:
        temp = stack[stitem--];
        stack[stitem] = Do_biran((int)(stack[stitem]), temp);
        break;
      case tATAN:
        stack[stitem] = R_TO_D(atan(stack[stitem]));
        break;
      case tACOS:
        stack[stitem] = R_TO_D(acos(stack[stitem]));
        break;
      case tASIN:
        stack[stitem] = R_TO_D(asin(stack[stitem]));
        break;
      case tEXP:
        stack[stitem] = exp(stack[stitem]);
        break;
      case tLOG:
        stack[stitem] = log(stack[stitem]);
        break;
      case tFLOOR:
        stack[stitem] = floor(stack[stitem]);
        break;
      case tCEIL:
        stack[stitem] = ceil(stack[stitem]);
        break;
      case tTRUNC:
        stack[stitem] = floor(stack[stitem] + 0.5);
        break;
      case tFABS:
        stack[stitem] = fabs(stack[stitem]);
        break;

      case tFPRINTF:
        argcount = (int)stack[stitem];

        /* retrieve file index */
        fileIndex = (int)stack[stitem - argcount + 1];

        if (!IsValidFpIndex(fileIndex)) {
          Message("Cannot use fprintf. File pointer not valid");
          break;
        }
        fp = Getfp(fileIndex);
        if (NULL == fp) {
          Message("Cannot use fprintf. The file is not opened!\n");
          break;
        }

        /* no break */

      case tPRINT: {
        int index;

        argcount = (int)stack[stitem--] - 1;
        format = strstack[strstitem--];

        if (expression->token == tPRINT)
          fp = stderr;
        else
          argcount--;

        /* the order of parameters is reversed */
        stitem -= argcount;
        index = 1;
        actpos = format;

        for (;;) {
          if ((actpos = strchr(actpos, '%')) == NULL)
            break;
          if (*(actpos + 1) != '%')
            break;

          actpos += 2; /* double % - not for parameter */
        }

        while (actpos != NULL) {
          if (argcount-- == 0) {
            Message("Warning: printf - not enough parameters!\n");
            args[0] = 0;
          } else
            args[0] = stack[stitem + index++];

          for (;;) {
            if ((actpos = strchr(actpos + 1, '%')) == NULL)
              break;
            if (*(actpos + 1) != '%')
              break;

            actpos++; /* double % - not for parameter */
          }
          if (actpos == NULL)
            break;

          *actpos = '\0';
          if (stderr == fp)
            Message(format, args[0]);
          else
            fprintf(fp, format, args[0]);
          format = actpos;
          *format = '%';
        }

        if (stderr == fp)
          Message(format, args[0]);
        else
          fprintf(fp, format, args[0]);

        if (argcount > 0)
          Message("Warning: ^printf - too many parameters!\n");

        stack[stitem] = 0.0;
        break;
      }
      case tINBLOB:
        for (x = 3; x >= 0; x--) {
          args[x] = stack[stitem--];
        }
        stack[++stitem] = field((short)args[0], args[1], args[2], args[3]);
        break;
      case tSTRING:
        strstack[++strstitem] = expression->tokenString;
        break;
      case tSTOP:
        if (stack[stitem] == 1) {
          animateFlag = RUN; /* stop and continue */
        } else {
          animateFlag = STOP;
          swapinterval = 0;
          my_ringbell();
        }
        break;
      case tFOPEN:
        argcount = (int)stack[stitem];
        fileType = strstack[strstitem--];
        fileName = strstack[strstitem--];
        if (argcount != 2) {
          Message("Warning: Wrong number of arguments for fopen.\n");
        }

        stack[stitem] = Getnewfp(fileName, fileType);
        if (-1 == stack[stitem])
          Message("Cannot fopen %s\n", fileName);

        break;
      case tFCLOSE:
        fileIndex = (int)stack[stitem];
        if (!IsValidFpIndex(fileIndex)) {
          Message("fclose error: file pointer is not valid\n");
          break;
        }

        if (!Fclosefp(fileIndex))
          Message("Cannot fclose. The file %s already closed\n",
                  GetFname(fileIndex));
        stack[stitem] = 0;
        break;

      case tFFLUSH:
        fileIndex = (int)stack[stitem];
        if (!IsValidFpIndex(fileIndex)) {
          Message("fflush error: file pointer is not valid\n");
          break;
        }
        fp = Getfp(fileIndex);
        if (NULL != fp)
          fflush(fp);
        else
          Message("Cannot fflush. The file %s is not opened\n",
                  GetFname(fileIndex));

        stack[stitem] = fileIndex;
        break;
      case tFSCANF:
        argcount = (int)stack[stitem--];
        format = strstack[strstitem--];
        /* put address arguments into an array */
        for (x = argcount - 3; x >= 0; x--) {
          args[x] = stack[stitem--];
          argAdrs[x] = adrstack[adrstitem--];
        }
        /* retrieve file index */
        fileIndex = (int)stack[stitem--];
        if (!IsValidFpIndex(fileIndex)) {
          Message("Cannot fscanf: file pointer is not valid\n");
          break;
        }
        fp = Getfp(fileIndex);
        if (NULL == fp) {
          Message("Cannot fscanf. The file %s is not opened\n",
                  GetFname(fileIndex));
          break;
        }
        stack[stitem++] = fscanf(fp, format, argAdrs[0], argAdrs[1], argAdrs[2],
                                 argAdrs[3], argAdrs[4], argAdrs[5], argAdrs[6],
                                 argAdrs[7], argAdrs[8], argAdrs[9]);
        /* There's got to be a better way - copy from print*/

        if (argcount > 12) {
          Message("Warning: Too many arguments for fscanf.\n");
        }
        break;
      case tARRAYLVAL:
      case tARRAYLHS:
      case tARRAYREF:
        /* access array dimension information */
        arrayPtr = expression->symbol->arrayData;
#ifdef JIM
        arrayValues = GetSymbolValuePtr(expression->symbol);
#endif

        /* initialize array reference */
        temp = 0;
        /* get number of dimensions and extract subscripts */
        argcount = (int)stack[stitem--];
        for (x = argcount - 1; x >= 0; x--) {
          args[x] = stack[stitem--];
          if (args[x] >= arrayPtr->size[x] || args[x] < 0) {
            Message("Warning: Subscript # %d = %.0f out of range for %s; using "
                    "%d\n",
                    x + 1, args[x], expression->symbol->label,
                    (args[x] < 0) ? 0 : arrayPtr->size[x] - 1);
            args[x] = (args[x] < 0) ? 0 : arrayPtr->size[x] - 1;
          }
        }
        /* calculate array reference offset */
        for (x = 1; x < argcount; x++) {
          temp = (temp + args[x - 1]) * arrayPtr->size[x];
        }
        switch (expression->token) {
        case tARRAYLVAL:
          /* stack "address" and its index */
          adrstitem++;
#ifdef JIM
          adrstack[adrstitem] = arrayValues + (int)(temp + args[argcount - 1]);
#else
          adrstack[adrstitem] = expression->symbol->arrayData->values +
                                (int)(temp + args[argcount - 1]);
#endif
          stitem++;
          stack[stitem] = adrstitem;
          break;
        case tARRAYLHS:
          /* stack offset of reference value */
          stitem++;
          stack[stitem] = temp + args[argcount - 1];
          break;
        case tARRAYREF:
          /* stack referenced value */
          stitem++;
#ifdef JIM
          stack[stitem] = *(arrayValues + (int)(temp + args[argcount - 1]));
#else
          stack[stitem] =
              *(arrayPtr->values + (int)(temp + args[argcount - 1]));
#endif
          break;
        default:
          Message("INTERNAL ERROR: Illegal array token.\n");
          MyExit(1);
          break;
        }
        break;
      }

      /* repeat the loop if there is no other parameter following the
      operator -> happens for unary operators */
    } while (expression->nextParam == NULL);

    for (;;) {
      char skip = 0;

      switch (expression->up->token) {
      case tOR:
        /* if the first operand of OR is true, you can skip the second one */
        skip = stack[stitem];
        break;

      case tAND:
        /* if the first operand of AND is true, you can skip the second one */
        skip = !stack[stitem];
        break;

      case tQUESTION:
        if (expression->nextParam != NULL) {
          EToken *ptr;

          ptr = expression->nextParam;
          while (ptr->up != expression->up)
            ptr = ptr->up;

          if (!stack[stitem]) {
            /* if the first operand is false, you can go
                    directly to the second one */
            expression = ptr;
            stack[++stitem] = 0; /* a dummy value */
          }

          /* are we on the second operand? */
          if (ptr->nextParam == NULL)
            if (stack[stitem - 1]) /* was the condition true? */
              skip = 1;            /* if so, skip the third operand */
        }

        break;
      }

      if (skip) {
        if ((expression = expression->up)->up == NULL)
          return stack[stitem];
      } else
        break;
    }

    if (expression->nextParam == NULL)
      goto goup;
    expression = expression->nextParam;
  }
}

/****************************************************************************/
/* Process statements in a list                                             */
/****************************************************************************/
void ProcessStatements(Statement *statementPtr) {
  while (statementPtr != NULL) {
    switch (statementPtr->type) {
    case stmntASSIGN:
#ifdef JIM
      *(GetSymbolValuePtr(statementPtr->leftHandSide->symbol)) =
          Eval(statementPtr->expression);
#else
      statementPtr->leftHandSide->symbol->value =
          Eval(statementPtr->expression);
#endif
      break;
    case stmntPROC:
      Eval(statementPtr->expression);
      break;
    case stmntIF:
      if (Eval(statementPtr->condition) != 0.0) {
        ProcessStatements(statementPtr->block);
      } else {
        if (statementPtr->elseblock) {
          ProcessStatements(statementPtr->elseblock);
        }
      }
      break;
    case stmntWHILE:
      while (Eval(statementPtr->condition) != 0.0) {
        ProcessStatements(statementPtr->block);
      }
      break;
    case stmntDO:
      do {
        ProcessStatements(statementPtr->block);
      } while (Eval(statementPtr->condition) != 0.0);
      break;
    case stmntARRAYASSIGN:
      AssignArrayElement(statementPtr->leftHandSide,
                         Eval(statementPtr->expression));
      break;
    case stmntARRAYDEF:
      ProcessArrayDef(statementPtr->expression);
      break;
    }
    statementPtr = statementPtr->nextStatement;
  }
}

/****************************************************************************/
/* Evaluate global start statements                                         */
/****************************************************************************/
void EvaluateStartStatements(LSYSDATA *lsysPtr) {
  while (lsysPtr != NULL) {
    ProcessStatements(lsysPtr->startBlock);
    lsysPtr = lsysPtr->nextLsystem;
  }
}

/****************************************************************************/
/* Evaluate global end statements                                         */
/****************************************************************************/
void EvaluateEndStatements(const LSYSDATA *lsysPtr) {
  while (lsysPtr != NULL) {
    ProcessStatements(lsysPtr->endBlock);
    lsysPtr = lsysPtr->nextLsystem;
  }
}

/****************************************************************************/
/* Evaluate global starteach statements */
/****************************************************************************/
void EvaluateStartEach(LSYSDATA *lsysPtr) {
  while (lsysPtr != NULL) {
    ProcessStatements(lsysPtr->startEach);
    lsysPtr = lsysPtr->nextLsystem;
  }
}

/****************************************************************************/
/* Evaluate global endeach statements                                         */
/****************************************************************************/
void EvaluateEndEach(LSYSDATA *lsysPtr) {
  while (lsysPtr != NULL) {
    ProcessStatements(lsysPtr->endEach);
    lsysPtr = lsysPtr->nextLsystem;
  }
}

/****************************************************************************/
/* Build a statement list from component statements                         */
/****************************************************************************/
Statement *BuildStatementList(Statement *list1, Statement *list2) {
  Statement *statementPtr, *startStatement;

  if (list1 != NULL) {
    startStatement = statementPtr = list1;
    /* find end of the first list */
    while (statementPtr->nextStatement != NULL) {
      statementPtr = statementPtr->nextStatement;
    }
    /* append the second list */
    statementPtr->nextStatement = list2;
  } else {
    startStatement = list2;
  }

  return startStatement;
}

/****************************************************************************/
/* Build a module list from component lists                                 */
/****************************************************************************/
Module *BuildModuleList(Module *list1, Module *list2) {
  Module *modulePtr, *startModule;

  if (list1 != NULL) {
    startModule = modulePtr = list1;
    /* find end of the first list */
    while (modulePtr->nextModule != NULL) {
      modulePtr = modulePtr->nextModule;
    }
    /* append the second list */
    modulePtr->nextModule = list2;
  } else {
    startModule = list2;
  }

  return startModule;
}

/****************************************************************************/
/* Append an expression to a parameter list                                 */
/****************************************************************************/

Parameter *BuildParameterList(Parameter *parameterList, EToken *expression,
                              char type) {
  Parameter *parameterPtr, *startParameter, *parameter;

  /* allocate parameter space */
  if ((parameter = (Parameter *)Malloc(sizeof(Parameter))) == NULL) {
    Warning("Parameter memory allocation failed", INTERNAL_LVL);
  }

  parameter->type = type;
  parameter->nextParameter = (Parameter *)NULL;
  switch (type) {
  case 'f': /* formal parameter */
    parameter->ptrTo.symbol = expression->symbol;
    Free(expression->tokenString);
    expression->tokenString = NULL;
    Free(expression);
    expression = NULL;

    break;
  case 'e': /* expression parameter */
    parameter->ptrTo.expression = expression;
    break;
  default:
    Message("ERROR: Invalid parameter type.\n");
    MyExit(1);
  }

  if (parameterList != NULL) {
    startParameter = parameterPtr = parameterList;
    /* find end of the first postfix expression */
    while (parameterPtr->nextParameter != NULL) {
      parameterPtr = parameterPtr->nextParameter;
    }
    /* append the new parameter */
    parameterPtr->nextParameter = parameter;
  } else {
    startParameter = parameter;
  }
  return startParameter;
}

/****************************************************************************/
/* Build a postfix order expression given operator and 3 operands           */
/****************************************************************************/
EToken *BuildTrinary(EToken *operator_, EToken *operand1, EToken *operand2,
                     EToken *operand3) {
  EToken *ptr;

  if (operand1 != NULL) {
    ptr = operand1;
    while (ptr->up != NULL)
      ptr = ptr->up;
    ptr->up = operator_;

    ptr->nextParam = operand2;

    if (operand2 != NULL) {
      ptr = operand2;
      while (ptr->up != NULL)
        ptr = ptr->up;
      ptr->up = operator_;

      ptr->nextParam = operand3;

      if (operand3 != NULL) {
        ptr = operand3;
        while (ptr->up != NULL)
          ptr = ptr->up;
        ptr->up = operator_;
      }
    }
  }

  return operand1;
}

/****************************************************************************/
/* Build a postfix order expression given operator and 2 operands           */
/****************************************************************************/
EToken *BuildBinary(EToken *operand1, EToken *operator_, EToken *operand2) {
  EToken *ptr;

  if (operand1 != NULL) {
    ptr = operand1;
    while (ptr->up != NULL)
      ptr = ptr->up;
    ptr->up = operator_;

    ptr->nextParam = operand2;

    if (operand2 != NULL) {
      ptr = operand2;
      while (ptr->up != NULL)
        ptr = ptr->up;
      ptr->up = operator_;
    }
  }

  return operand1;
}

/****************************************************************************/
/* Build a postfix order expression given operator and 1 operand	    */
/****************************************************************************/
EToken *BuildUnary(EToken *operator_, EToken *operand) {
  EToken *ptr;

  if (operand != NULL) {
    ptr = operand;
    while (ptr->up != NULL)
      ptr = ptr->up;
    ptr->up = operator_;
  }

  return operand;
}

/****************************************************************************/
/* Build several postfix order expressions given parameterlist		    */
/****************************************************************************/
EToken *BuildExprList(EToken *fcToken, Parameter *params, int argcount) {
  EToken *tokenPtr, *startToken;
  int count = CountParameters(params);
  Parameter *ptr;

  startToken = tokenPtr = params->ptrTo.expression;

  ptr = params;
  params = params->nextParameter;

  Free(ptr);
  ptr = NULL;

  while (params) {
    /* find end of the postfix expression */
    while (tokenPtr->up != NULL) {
      tokenPtr = tokenPtr->up;
    }
    /* connect param with the function */
    tokenPtr->up = fcToken;
    /* append next param to the postfix string */
    tokenPtr->nextParam = params->ptrTo.expression;
    tokenPtr = params->ptrTo.expression;

    ptr = params;
    params = params->nextParameter;
    Free(ptr);
    ptr = NULL;
  }
  /* find end of the postfix expression */
  while (tokenPtr->up != NULL)
    tokenPtr = tokenPtr->up;
  tokenPtr->up = fcToken;

  if (argcount) {
    /* append next param to the postfix string */
    if ((tokenPtr->nextParam = (EToken *)Malloc(sizeof(EToken))) == NULL) {
      Warning("EToken memory allocation failed", FATAL_LVL);
    }
    tokenPtr = tokenPtr->nextParam;
    tokenPtr->nextParam = NULL;
    tokenPtr->up = fcToken;
    tokenPtr->symbol =
        SymbolTableAdd(Strdup(constantToken), count, currentSymbolTable);
    tokenPtr->token = tVALUE;
    tokenPtr->tokenString = Strdup("arg count");

    if (constantToken[strlen(constantToken) - 1] == '\377') {
      constantToken[strlen(constantToken) + 1] = '\0';
      constantToken[strlen(constantToken)] = '\200';
    } else
      constantToken[strlen(constantToken) - 1]++;
  }
  return startToken;
}

/****************************************************************************/
/* NextToken is a function which returns the next token from its string	    */
/* argument. Constants are added to the variable table. Constants and	    */
/* variables point to there respective symbol table entries		    */
/****************************************************************************/

/****************************************************************************/
/*	Function: RconDiff
 */
/*	Check whether the right context of a production (rconPtr) matches */
/*  string strPtr. The context is considered in the bracketed sense.
 */
/*	Return 0 if there is no discrepancy, 1 otherwise.
 */
/*  If a ring L-system is being processed, the right end is connected
 */
/*  to the left end for context checking purposes.
 */
/****************************************************************************/

int RconDiff(Module *rconPtr, char *strPtr, LSYSDATA *LsysPtr) {
  StringModule ts; /* Structure to describe the tokens in the string */

  if (rconPtr == NULL)
    return (0); /* if there is no context to match, there is no
                   discrepancy */

  for (;;) {
    /* for a ring L-system, check if string has reached the right end */
    /* MSH - fixed the following ring L-system code
    Note: this is NOT the best or most efficient way to do this
    the best way is to have pointers to the two null positions in
    the string (start and end), but these are not currently
    available */
    if (LsysPtr->ring &&
        ((*strPtr == '\0') || ((*strPtr == '%') && (*(strPtr + 1) == '(')))) {
      strPtr--;
      while ((*strPtr != '\0') &&
             ((*strPtr != '%') || (*(strPtr + 1) != '('))) {
        if (*strPtr == ')')
          strPtr = movestringleft(strPtr);
        strPtr--;
      }
      strPtr++;
    }
    switch (rconPtr->symbol) {
    case '=':
      strPtr = skipright(strPtr);
      rconPtr = rconPtr->nextModule;
      break;
    default:

      if ((*strPtr == '\0') || /* end of string reached */
          ((*strPtr == '%') && (*(strPtr + 1) == '(')))
        /* or the end of a substring */
        return 1; /* not all context symbols found */

      /*	Skip symbol to be ignored as the context member. */

      if (LsysPtr->ignore[(int)(*strPtr)]) {
        strPtr++;
        if (*strPtr == '(') {
          strPtr = moveright(strPtr);
        }
      }

      /*	Skip a branch if the context does not explicitly refer to it */

      else if (rconPtr->symbol != '[' && *strPtr == '[')
        strPtr = skipright(strPtr + 1) + 1;

      else {
        /* save pointer for matched symbol */
        rconPtr->matchedSymbol = strPtr;
        /*	If discrepancy found, return 1. */
        if (rconPtr->symbol != NextStringModuleForMatch(&strPtr, &ts) ||
            rconPtr->parameters != ts.parameters)
          return (1);
        rconPtr = rconPtr->nextModule;

        /*	If the leftmost character of the left context has been reached,
        return 0 - no discrepancy has been found. */

        if (rconPtr == NULL)
          return (0);
      }
      break;
    }
  }
}

/****************************************************************************/
/*	Function: GetRconVars
 */
/*	Extract variables from the string and add to the symbol table */
/*  with appropriate variable names from right context of the production.
 */
/****************************************************************************/

static void GetRconVars(Module *rconPtr) {
  while (rconPtr != NULL) {
    /* if the current rcon symbols an = it should be skipped */
    if (rconPtr->symbol != '=') {
      /* Get parameter strings from the production and string */
      (void)ProcessSymbolVariables(rconPtr, &(rconPtr->matchedSymbol));
    }
    rconPtr = rconPtr->nextModule;
  }
}

/*************************************************************************
Function: moveright
Move to the end of the current symbol in the actual string.
*************************************************************************/

static char *moveright(char *s) {
  s += SKIPSIZE;
  while (*s == ',') {
    s += SKIPSIZE;
  }
  if (*s != ')') {
    Message("ERROR: Unmatched parenthesis while skipping a symbol.\n");
    MyExit(1);
  } else
    s++;
  return (s);
}

/****************************************************************************
Function: skipright
Skip over all characters in a string up until the end of the current branch.
Slightly modified to improve the efficiency.
****************************************************************************/

static char *skipright(char *s) {
  int level = 0;

  --s;

  for (;;) {
    switch (*(++s)) {
    case '(':
      s += SKIPSIZE;
      while (*s == ',') {
        s += SKIPSIZE;
      }
      break;

    case '[':
      ++level;
      break;

    case ']':
      if (level == 0)
        return (s);
      else
        --level;
      break;

    case '\0':
      return (s);
    default:
      break;
    }
  }
}

/****************************************************************************
Function: LconDiff
Check whether the left context of a production (lconPtr) matches string strPtr.
The context is considered in the bracketed sense.  Return 0
if there is no discrepancy, 1 otherwise.  Note that the matching proceeds
"backwards", i.e., from the right to the left.  Branches (sub-strings
in square brackets) are skipped.
If a ring L-system is being processed, the left end is connected
to the right end for context checking purposes.
****************************************************************************/

int LconDiff(Module *lconPtr, char *strPtr, LSYSDATA *LsysPtr) {
  StringModule ts; /* storage for token from string */
  char *tempPtr;   /* temporary pointer for comparisons */

  while (lconPtr != NULL) {
    /* for a ring L-system, check if string has reached the left end */
    /* MSH - fixed the following ring L-system code
    Note: this is NOT the best or most efficient way to do this
    the best way is to have pointers to the two null positions in
    the string (start and end), but these are not currently
    available */
    if (LsysPtr->ring &&
        ((*strPtr == '\0') || ((*strPtr == '%') && (*(strPtr + 1) == '(')))) {
      strPtr++;
      while ((*strPtr != '\0') &&
             ((*strPtr != '%') || (*(strPtr + 1) != '('))) {
        strPtr++;
        if (*strPtr == '(')
          strPtr = moveright(strPtr);
      }
      strPtr--;
    }

    /* check if string has parameters */
    /* if so move to start of symbol and parameters */
    if (*strPtr == ')') {
      strPtr = movestringleft(strPtr);
    }

    switch (*strPtr) {
    case ']':
      strPtr = skipleft(strPtr); /* skip branch */
      break;
    case '[':
      strPtr--; /* skip left bracket */
      break;
    default:
      if ((*strPtr == '\0') || /* beginning of string reached */
          ((*strPtr == '%') && (*(strPtr + 1) == '(')))
        /* or beginning of a substring */
        return 1; /* not all context symbols found */

      if (LsysPtr->ignore[(int)(*strPtr)])
        strPtr--; /* ignore the character to be ignored */
      else {

        /*	If discrepancy found, return 1.  */
        tempPtr = strPtr;
        if (lconPtr->symbol != NextStringModuleForMatch(&tempPtr, &ts) ||
            lconPtr->parameters != ts.parameters)
          return (1);

        /* save pointer to matched symbol */
        lconPtr->matchedSymbol = strPtr;

        /* move pointers */
        strPtr--;
        lconPtr = lconPtr->nextModule;
      }
      break;
    }
  }
  /*	If the leftmost character of the left context has been reached,
  return 0: no discrepancy has been found. */
  return (0);
}

/****************************************************************************
Function: GetLconVars
Extract variables from the left context in the string
and add to the symbol table with variable names from
the left context of the production.
****************************************************************************/

static void GetLconVars(Module *lconPtr) {
  while (lconPtr != NULL) {
    /* Get parameter strings from the production and string */
    /* and add to the symbol table.
     */
    (void)ProcessSymbolVariables(lconPtr, &(lconPtr->matchedSymbol));
    lconPtr = lconPtr->nextModule;
  }
}

/*************************************************************************
Function: movestringleft
Move to the start of the current symbol in the string.
*************************************************************************/

static char *movestringleft(char *s) {
  s -= SKIPSIZE;
  while (*s == ',') {
    s -= SKIPSIZE;
  }
  if (*s != '(') {
    Message("ERROR: Unmatched parenthesis while checking left context.\n");
    MyExit(1);
  }
  s--;
  return (s);
}

/*************************************************************************
Function: skipleft
Skip over all characters up until the end of the current branch.
Slightly modified to improve efficiency.
*************************************************************************/

static char *skipleft(char *s) {
  int level = 0;

  for (;;) {
    switch (*(--s)) {
    case ')':
      s -= SKIPSIZE;
      while (*s == ',') {
        s -= SKIPSIZE;
      }
      s--; /*can skip one more - none of [,],\0 can follow*/
      break;
    case ']':
      ++level;
      break;
    case '[':
      if (level == 0)
        return (--s);
      else
        --level;
      break;
    case '\0':
      return s;
    default:
      break;
    }
  }
}

/*************************************************************************
        THE FUNCTIONS BELOW APPLY TO THE STOCHASTIC CASE
*************************************************************************/

/*************************************************************************
        Function: FindApplProductions()  (used in the stochastic case).
        Given a pointer to a string and a set of productions, place pointers
        to all applicable productions in the array applSet.  Place NULL after
        the pointer to the last applicable production.  Calculate the sum of
        the probabilities of all applicable productions.  Return the result
        via totalprobPtr.
**************************************************************************/

static void FindApplProductions(char *curPtr, LSYSDATA *LsysPtr,
                                Production *applSetPtr[], float *totalprobPtr) {
  Production *prodPtr;
  int preflength; /* length of prefix string matched */

  prodPtr = LsysPtr->firstProd[(int)(*curPtr)];

  /*
  Check each production for the match.  If there is a match, place
  the pointer to the production under consideration in the array
  applSet.
  */

  if (clp.debug)
    Message("In find\n");
  *totalprobPtr = 0.0;

  while (prodPtr != NULL) {
    if (!PredDiff(prodPtr->pred, curPtr, &preflength) &&
        !RconDiff(prodPtr->rCon, curPtr + preflength, LsysPtr) &&
        !LconDiff(prodPtr->lCon, curPtr - 1, LsysPtr) &&
        !CondDiff(prodPtr, curPtr, LsysPtr)) {
      *totalprobPtr += prodPtr->prob = Eval(prodPtr->probExpression);
      *applSetPtr = prodPtr;
      ++applSetPtr;
    }
    prodPtr = prodPtr->nextProduction;
  }
  /*
  Terminate the sequence of applicable productions by NULL.
  */

  *applSetPtr = NULL;
}

/* Allocate space for array with name given by symbol and                   */
/* dimensions given by expressions in parameter list                        */
/****************************************************************************/
void AllocateArray(Symbol *arraySymbol, Parameter *parameterList
#ifdef JIM
                   ,
                   SymbolTable *currentSymbolTable
#endif
) {
  Array *arrayPtr;
  Parameter *params;
#ifdef JIM
  double *arrayValues;
#endif
  int i, count, spaceReqd;

  if ((count = CountParameters(parameterList)) <= 0) {
    Warning("Attempt to allocate dimensionless array", ERROR_LVL);
    return;
  }
#ifndef JIM
  arraySymbol->type = DOUBLEARRAY;
#endif

  /* set up storage for array information */
  if ((arraySymbol->arrayData = arrayPtr = (Array *)Malloc(sizeof(Array))) ==
      NULL) {
    Warning("Array data memory allocation failed", INTERNAL_LVL);
  }

  /* initialize array data */
  arrayPtr->dimensions = count;
  if ((arrayPtr->size = (int *)Malloc(count * sizeof(int))) == NULL) {
    Warning("Array size data memory allocation failed", INTERNAL_LVL);
  }
  arrayPtr->sizeExpressions = params = parameterList;
  arrayPtr->initExpressions = NULL;

  /* determine space required for array storage */
  spaceReqd = 1;
  for (i = 0; i < count; i++) {
    if (params == NULL) {
      Warning("Count of parameters larger than actual number", INTERNAL_LVL);
    }
    arrayPtr->size[i] = (int)Eval(params->ptrTo.expression);
    spaceReqd = spaceReqd * arrayPtr->size[i];
    params = params->nextParameter;
  }
  if (params != NULL) {
    Warning("Count of parameters less than actual number", INTERNAL_LVL);
  }

#ifndef JIM
  /* this has been replaced by the code following that allocates
  space in the instance values */
  /* allocate storage */
  /* allocate storage */
  if ((arrayPtr->values = (double *)Malloc(spaceReqd * sizeof(double))) ==
      NULL) {
    Warning("Array memory allocation failed", INTERNAL_LVL);
  }
#endif
  arrayPtr->spaceRequired = spaceReqd;

#ifdef JIM
  /* check for and reserve sufficient space in instance values */
  /* note that space for one value has already been
   * allocated by SymbolTableAdd
   */
  (void)AllocateValueOffset(currentSymbolTable, (*currentSymbolTable->instance),
                            spaceReqd - 1);

  /* initialize to zero */
  /* JH Oct6 arrayPtr->values = arraySymbol->values+arraySymbol->offset; */
  arrayValues = GetSymbolValuePtr(arraySymbol);
  for (i = 0; i < spaceReqd; i++) {
    arrayValues[i] = 0.0;
  }
#else
  /* initialize to zero */
  for (i = 0; i < spaceReqd; i++) {
    arrayPtr->values[i] = 0.0;
  }
#endif

  return;
}

/****************************************************************************/
/* Initialize array storage with values given in parameter list             */
/* Give warning of mismatched numbers of initializers                       */
/****************************************************************************/
void InitializeArray(Symbol *arraySymbol, Parameter *initialiserList) {
  double *valuePtr;
  Parameter *inits;
  int count;

  /* initialize pointer to first array location */
#ifdef JIM
  valuePtr = GetSymbolValuePtr(arraySymbol);
  /*valuePtr = arraySymbol->values + arraySymbol->offset;*/
#else
  valuePtr = arraySymbol->arrayData->values;
#endif
  count = 0;

  /* save initialiserList for later use/freeing */
  arraySymbol->arrayData->initExpressions = initialiserList;

  /* initialize pointer to first initialiser */
  inits = initialiserList;

  while (count < arraySymbol->arrayData->spaceRequired && inits != NULL) {
    *(valuePtr) = Eval(inits->ptrTo.expression);
    valuePtr++;
    inits = inits->nextParameter;
    count++;
  }
  if (count >= arraySymbol->arrayData->spaceRequired && inits != NULL) {
    Warning("Too many initialisers", WARNING_LVL);
  }
  if (count < arraySymbol->arrayData->spaceRequired && inits == NULL) {
    Warning("Too few initialisers", WARNING_LVL);
  }
  return;
}

/****************************************************************************/
/* ProcessArrayDef reinitializes all arrays in a definition statement       */
/****************************************************************************/
void ProcessArrayDef(EToken *arrayDefs) {
  EToken *tokenPtr;
  Array *arrayData;
  double *valuePtr;
  Parameter *inits;
  int count, errorFlag;

  /* loop over array definitions, initializing as appropriate */
  tokenPtr = arrayDefs;
  while (tokenPtr != NULL) {
    if (tokenPtr->symbol != NULL && tokenPtr->symbol->arrayData != NULL) {
      arrayData = tokenPtr->symbol->arrayData;
      /* initialize pointer to first array location */
#ifdef JIM
      valuePtr = GetSymbolValuePtr(tokenPtr->symbol);
#else
      valuePtr = arrayData->values;
#endif
      count = 0;

      /* initialize pointer to first initialiser */
      inits = arrayData->initExpressions;
      errorFlag = FALSE;
      while (count < arrayData->spaceRequired) {
        if (inits == NULL) {
          errorFlag = TRUE;
          *(valuePtr) = 0.0;
        } else {
          *(valuePtr) = Eval(inits->ptrTo.expression);
          inits = inits->nextParameter;
        }
        valuePtr++;
        count++;
      }
      if (count >= arrayData->spaceRequired && inits != NULL) {
        Warning("Too many initialisers", WARNING_LVL);
      }
      if (errorFlag && arrayData->initExpressions != NULL) {
        Warning("Too few initialisers", WARNING_LVL);
      }
    }
    tokenPtr = tokenPtr->nextParam;
  }

  return;
}

/****************************************************************************/
/* Check for illegal type in array parameter lists - debugging tool */
/****************************************************************************/
void CheckArrayData(Symbol *arraySymbol) {
  double temp;
  Parameter *parmPtr;
  int count;

  if (arraySymbol->arrayData == NULL)
    return;

  /* initialize pointer to first size location */
  parmPtr = arraySymbol->arrayData->sizeExpressions;
  count = 0;

  while (parmPtr != NULL) {
    switch (parmPtr->type) {
    case 'e': /* expression parameter */
      temp = Eval(parmPtr->ptrTo.expression);
      if (temp != arraySymbol->arrayData->size[count]) {
        Warning("Mismatch on size", ERROR_LVL);
      }
      count++;
      break;
    case 'f': /* formal parameter */
    default:
      Warning("Invalid parameter type in sizes.", ERROR_LVL);
    }
    parmPtr = parmPtr->nextParameter;
  }
  if (count != arraySymbol->arrayData->dimensions) {
    Warning("Mismatch on dimensions", ERROR_LVL);
  }

  /* initialize pointer to first initialiser location */
  parmPtr = arraySymbol->arrayData->initExpressions;
  if (parmPtr == NULL)
    return;
  count = 0;

  while (parmPtr != NULL) {
    switch (parmPtr->type) {
    case 'e': /* expression parameter */
      temp = Eval(parmPtr->ptrTo.expression);
      ;
      count++;
      break;
    case 'f': /* formal parameter */
    default:
      Warning("Invalid parameter type in initialisers.", ERROR_LVL);
    }
    parmPtr = parmPtr->nextParameter;
  }
  if (count != arraySymbol->arrayData->spaceRequired) {
    Warning("Mismatch on initialisers", ERROR_LVL);
  }
  return;
}

/****************************************************************************/
/* Store value into memory location appropriate for array reference         */
/****************************************************************************/
void AssignArrayElement(EToken *LHS, double assignedValue) {
  int offset;
  EToken *tokenPtr;

  /* find end of array reference expression */
  tokenPtr = LHS;
  while (tokenPtr->up != NULL) {
    tokenPtr = tokenPtr->up;
  }
  if (tokenPtr->token != tARRAYLHS) {
    Warning("Invalid array reference", INTERNAL_LVL);
  }

#ifdef JIM
  /* calculate offset into array storage;
   * array offset in instance+calculated index  */
  offset = tokenPtr->symbol->offset + (int)Eval(LHS);

  /* store value */
  *(tokenPtr->symbol->values + offset) = assignedValue;
#else
  /* calculate offset into array storage */
  offset = (int)Eval(LHS);

  /* store value */
  *(tokenPtr->symbol->arrayData->values + offset) = assignedValue;
#endif
}

/****************************************************************************/
/* Evaluate array initializer statements                                    */
/****************************************************************************/
void EvaluateArrayInitializers(LSYSDATA *lsysPtr) {
  while (lsysPtr != NULL) {
    ProcessStatements(lsysPtr->defineBlock);
    lsysPtr = lsysPtr->nextLsystem;
  }
}

/**************************************************************************
        Function: SelectProd()
        Given the set of applicable productions, select one according
        to given probabilities.  Return NULL if there is no applicable
        production.
**************************************************************************/

static Production *SelectProd(Production *applSetPtr[], float totalprob,
                              __attribute__((unused)) char *curPtr,
                              __attribute__((unused)) LSYSDATA *LsysPtr) {
  extern unsigned short xsubi[];
  float scaled_random;

  if (clp.debug)
    Message("In select\n");
  //scaled_random = totalprob * erand48(xsubi);
  scaled_random = totalprob * drand48();

  while (*applSetPtr != NULL) {
    if ((scaled_random -= (*applSetPtr)->prob) <= 0) {
      /* Recreate variable table for the chosen production */
      /* CreateVarTable(*applSetPtr, curPtr, LsysPtr); */
      return (*applSetPtr);
    }
    ++applSetPtr;
  }
  return (NULL);
}

FILE *PreprocessLsystem(const char *fname) {
  static char defs[1024] = "";
  static char funcnm[64];
  char *prnt = defs;
  int i;
  if (FunctionsFromGallery()) {
    for (i = 0; i < SplineFuncCount(); i++) {
      char *dot = NULL;
      strcpy(funcnm, SplineFuncName(i));
      /* remove .whatever suffix (probably .func) */
      dot = strrchr(funcnm, '.');
      if (NULL != dot)
        *dot = 0;
      /* capitalize the name */
      /*
      dot = funcnm;
      while (0 != *dot)
      {
              *dot = toupper(*dot);
              dot++;
      }
      */
      prnt += sprintf(prnt, "-D%s=%d ", funcnm, i + 1);
    }
  }
  return PreprocessFile(fname, defs);
}
