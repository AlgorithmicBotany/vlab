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



#ifndef _CPFG_GENERATE_
#define _CPFG_GENERATE_
/*
MODULE:		generate.h
AUTHOR:		Jim Hanan and P. Prusinkiewicz
HISTORY:
June, 1989. Added structure for symbols and parameters called Token.
Sept, 1989. Added #defines for sub-L-systems.
Sept, 1990. Added Parameter, Token and Symbol structures.
            Production components are now kept in lists.
PURPOSE: This is the header file for the continuous L-system string generator.
*/

#include "interpret.h"

#ifdef __cplusplus
extern "C" {
#endif

/* define error levels */
#define WARNING_LVL 0
#define ERROR_LVL 1
#define FATAL_LVL 2
#define INTERNAL_LVL 3

#define MAXPROD 100 /* maximum number of productions */

/*#define JIM */ /* jim's extensions from Oct 1997 */

/*
 *  Array is a structure containing info for a variable
 *  defined as an array,  including maximum dimensions
 *  and a pointer to allocated storage.
 */
struct Array_s {
  int dimensions; /* number of dimensions */
  int spaceRequired;
  struct Parameter_s *sizeExpressions; /* expressions declaring dimensions */
  struct Parameter_s *initExpressions; /* expressions initializing array */
  int *size; /* pointer to array of "dimensions" integers giving
                sizes for each */
#ifndef JIM
  double *values; /* pointer to space allocated for array */
#endif
};

typedef struct Array_s Array;

/* type of symbol in symbol table */
#ifdef JIM
typedef enum { DOUBLE, DOUBLEARRAY, EXTERNAL, EXTERNALARRAY } symbolType;
#else
typedef enum { DOUBLE, DOUBLEARRAY } symbolType;
#endif

/*
        Symbol is a structure to hold a variable or constant symbol and its
   value used as components of a symbol table list.
*/

struct Symbol_s {
#ifdef JIM
  double *values;    /* pointer to currently assigned instance values */
  int offset;        /* offset in current instance value space */
  int defaultOffset; /* offset in default instance value
                        space */
#else
  double value; /* The currently assigned value */
#endif
  struct Symbol_s *nextSymbol;
  char *label;      /* The symbol */
  Array *arrayData; /* pointer to array information */
  symbolType type;
};

typedef struct Symbol_s Symbol;

#ifdef JIM
/*
        SymbolInstance is a structure used as a component of a symbol table
   instance list
*/
struct SymbolInstance_s {
  double *values;
  int initFlag;   /* flag indicating whether initialisation has been run */
  int deleteFlag; /* flag indicating the instance is to be deleted */
  int valueCount; /* current number of values stored */
  int valueSpace; /* maximum number of values: current size of values space */
  struct SymbolInstance_s *prevSymbolInstance;
  struct SymbolInstance_s *nextSymbolInstance;
};

typedef struct SymbolInstance_s SymbolInstance;
#endif

/*
        SymbolTable is a structure used as a component of a symbol table stack
*/

struct SymbolTable_s {
  Symbol **firstEntry;
#ifdef JIM
  SymbolInstance **instance;
#endif
  struct SymbolTable_s *nextSymbolTable;
};

typedef struct SymbolTable_s SymbolTable;

/*
        The Token structure is a component of expression lists
        may be an operator or a symbol
*/

/* expression token - a tree structure */
struct EToken_s {
  int token;
  Symbol *symbol;
  char *tokenString;          /* The token string for debuggery */
  struct EToken_s *nextParam; /* next parameter on the same level */
  struct EToken_s *up;        /* up one level (function symbol) */
};

typedef struct EToken_s EToken;

/* Parameter list */
struct Parameter_s {
  union {
    EToken *expression;
    Symbol *symbol;
  } ptrTo;
  struct Parameter_s *nextParameter;
  char type;
};

typedef struct Parameter_s Parameter;

/*
        Statement is a structure to build a Statement list
*/

typedef enum {
  stmntARRAYDEF,
  stmntARRAYASSIGN,
  stmntASSIGN,
  stmntPROC,
  stmntIF,
  stmntWHILE,
  stmntDO
} StmntType;

struct Statement_s {
  StmntType type;
  EToken *leftHandSide;
  EToken *expression;
  EToken *condition;
  struct Statement_s *block;
  struct Statement_s *elseblock;
  struct Statement_s *nextStatement;
};

typedef struct Statement_s Statement;

/*
        Module is a structure to hold a production symbol and its
        parameters.
*/

struct Module_s {
  int parameters; /* The number of associated parameters */
  int length;     /* The length of the token, symbol and parameters */
  Parameter *parmList;
  struct Module_s *nextModule;
  char *matchedSymbol; /* The string symbol matched */
  char symbol;         /* The symbol */
};

typedef struct Module_s Module;

/*
Production is a structure which contains:
        - Pointers to the first module of the linked lists representing:
                - the left context,
                - the strict predecessor,
                - the right context,
                - the successor.
        - Pointer to the pre-condition statement list
        - Pointer to the condition expression list
        - Pointer to the post-condition statement list
        - Pointer to the probability expression list (stochastic L-systems).
        - Calculated probability of the production (stochastic L-systems).
        - Length of matched predecessor: to be filled in
        - Length of successor when packed into string
        - flag indicating Lsystem change in strict predecessor
        - Pointer to symbol table list
        - Pointer to next production
*/

struct Production_s {
  Module *lCon;
  Module *pred;
  Module *rCon;
  Module *succ;
  Statement *preCondList;
  EToken *condition;
  Statement *postCondList;
  EToken *probExpression;
  float prob;
  int predLen;
  int succLen;
  int lsystemChange;
  char object_flag;
#ifdef JIM
  int lsystemInPred;
  int lsystemInSucc;
  SymbolInstance *instance; /* instance for symbol table values */
#endif
  Symbol *symbolTable; /* list of symbols */
  struct Production_s *nextProduction;
};

typedef struct Production_s Production;

/* following data structure is used both by homomorphism and decomposition */
struct PRODDATA {
  Production *firstProd[128]; /* a lookup table to find the first possibly
                                 applicable production in the set */
  char specified;             /* is the production set specified? */
  int depth;                  /* maximal depth of the recursion */
  int longest_succ;           /* length of the longest successor (in bytes)*/
  char **stack;               /* stack of strings of length 'longest_succ'
                                 for recursive interpretation of the
                                 set of productions */
  int seed;                   /* separate seed for the homomorphism */
  unsigned short xsubi[3];    /* number x used in the random generator */
  int stack_len;
};

typedef struct PRODDATA PRODDATA;

/*
        LSYSDATA is a structure which collects all data related to
        string generation.
*/

struct LSYSDATA_s {
  int id;      /* identifying number (used in sub-L-system processing */
  int n;       /* derivation length */
  int current; /* current derivation step */
  int axiomLength;
  int longest_succ;    /* length of the longest successor (in bytes)*/
  Symbol *symbolTable; /* symbol table for this L-system */
#ifdef JIM
  SymbolInstance *instanceList;    /* symbol table instance list */
  SymbolInstance *currentInstance; /* symbol table instance currently in
                                      use */
#endif
  Statement *startBlock; /* statements executed when starting the L-system */
  Statement *endBlock;   /* statements executed when ending the L-system */
  Statement *startEach;  /* statements executed when starting each step */
  Statement *endEach;    /* statements executed when ending each step */
  Statement *defineBlock;
  int stochastic;             /* is the L-system a stochastic one? */
  int seed;                   /* for the random number generator */
  int ring;                   /* Is the L-system a ring or string? */
  Production *firstProd[128]; /* a lookup table to find the first
                      possibly applicable production in the set */
  struct LSYSDATA_s *nextLsystem;
  char *name;       /* identifying name of the L-system */
  char ignore[128]; /* a lookup table of the characters to be
                          ignored while context matching */
  Module *axiom;
  PRODDATA Homomorphism; /* homomorphism data */
  char homo_warning;
  PRODDATA Decomposition; /* decomposition data */
  char decomp_warning;
};

typedef struct LSYSDATA_s LSYSDATA;

/* prototypes */
int ReadLsystem(char *filename);
void FreeViewFileData(void);
void FreeExpressionSpace(EToken *tokenPtr);
void MyExit(int status);

int CreateParameterString(char **targetStr, Module *sourceModules,
                          LSYSDATA *lsysPtr);
void PrintParameterString(FILE *fp, char *sourceStr, int format, char use_homo);
void PrintExpression(EToken *start);
void PrintStatementList(Statement *start);
int MatchParameters(Parameter *parameterList, int functionid);
char NextStringModule(char **str, StringModule *module);
void Derive(LSYSDATA *LsystemListPtr, char **string1, char **string1end,
            char **string2, char **string2end, DRAWPARAM *drawparamPtr,
            VIEWPARAM *viewparamPtr
#ifdef JIM
            ,
            int endStep
#endif
);

void CheckValues(LSYSDATA *LsysPtr, SymbolTable *curTable);
void FreeParameterList(Parameter *paramPtr);
int PredDiff(Module *predPtr, char *strPtr, int *length);
int CondDiff(Production *prodPtr, char *str, LSYSDATA *LsysPtr);
void SkipCutPart(char **str);
void ApplyProd(char **curStrPtr, char **nextStrPtr, char **nextStrBeginPtr,
               char **nextStrEndPtr, Production *prodPtr, LSYSDATA *LsysPtr,
               char ignore_decomposition);

void AppendParameter(char **str, double value);
Module *AssignParameters(Module *, Parameter *);
EToken *BuildBinary(EToken *, EToken *, EToken *);
EToken *BuildUnary(EToken *, EToken *);
EToken *BuildTrinary(EToken *, EToken *, EToken *, EToken *);
Module *BuildModuleList(Module *list1, Module *list2);
Parameter *BuildParameterList(Parameter *parameterList, EToken *expression,
                              char type);
EToken *BuildExprList(EToken *fcToken, Parameter *param, int argcount);
Statement *BuildStatementList(Statement *list1, Statement *list2);
double Eval(EToken *expression);
void EvaluateEndEach(LSYSDATA *lsysPtr);
void EvaluateEndStatements(const LSYSDATA *lsysPtr);
void EvaluateStartEach(LSYSDATA *lsysPtr);
void EvaluateStartStatements(LSYSDATA *lsysPtr);
int PackedModuleListLength(Module *start);
int CountParameters(Parameter *parameterList);
void PrintModuleList(Module *ptr);
void ProcessStatements(Statement *statementPtr);
Symbol *SymbolTableAdd(char *label, double value, SymbolTable *symbolTable);
Symbol *SymbolTableReplace(char *label, double value, SymbolTable *symbolTable);
Symbol *SymbolTableFind(char *label, SymbolTable *symbolTableStack);
SymbolTable *PopSymbolTable(SymbolTable *current);

#ifndef JIM
SymbolTable *PushSymbolTable(SymbolTable *current, Symbol **firstEntry);

void AllocateArray(Symbol *arraySymbol, Parameter *params);
#else

SymbolTable *PushSymbolTable(SymbolTable *current, Symbol **firstEntry,
                             SymbolInstance **instance);
Symbol *CurrentSymbolTableFind(char *label, SymbolTable *currentSymbolTable);
void AssignSymbolsToInstances(Symbol *symbolPtr, SymbolInstance *instancePtr);

SymbolInstance *NewSymbolInstance(int size);
void FreeSymbolInstance(SymbolInstance *instance);
void FreeInstanceList(SymbolInstance *instance);
void ResetInstanceSpace(SymbolInstance *instance, int newSize);
int AllocateValueOffset(SymbolTable *symbolTable, SymbolInstance *instance,
                        int amount);
SymbolInstance *CopySymbolInstance(SymbolInstance *instance);
SymbolInstance *InsertInstance(SymbolInstance *instance,
                               SymbolInstance *position);
void RemoveInstance(SymbolInstance *instance);
void InitialiseInstance(LSYSDATA *lsysPtr);
SymbolInstance *AppendInstance(LSYSDATA *lsysPtr);

void AllocateArray(Symbol *arraySymbol, Parameter *params,
                   SymbolTable *currentSymbolTable);
#endif

void EvaluateArrayInitializers(LSYSDATA *lsysPtr);
void CheckArrayData(Symbol *arraySymbol);
void InitializeArray(Symbol *arraySymbol, Parameter *initializerList);
void ProcessArrayDef(EToken *arrayDefs);
void AssignArrayElement(EToken *LHS, double assignedValue);
int RconDiff(Module *rconPtr, char *strPtr, LSYSDATA *LsysPtr);
int LconDiff(Module *lconPtr, char *strPtr, LSYSDATA *LsysPtr);
LSYSDATA *MatchingLsystem(int Lsysid, LSYSDATA *LsystemListPtr);

#ifdef __cplusplus
}
#endif

#endif /* _CPFG_GENERATE_ */
