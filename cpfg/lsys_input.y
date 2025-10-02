%{
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


/*
 * $RCSfile: lsys_input.y,v $
 * $Revision: 1.2 $
 * $Date: 2005/08/30 20:32:02 $
 * $Author: radekk $
 * $Locker:  $
 *
 * $Log: lsys_input.y,v $
 * Revision 1.2  2005/08/30 20:32:02  radekk
 * Merge with Windows version
 *
 * Revision 1.1.1.1  2005/07/04 21:15:24  federl
 * Imported vlab-for-mac port so that Colin and Radek can continue working on bug-fixes
 *
 * Revision 1.4  2004/12/14 21:10:55  radekk
 * Fixed preprocessing of L-system
 *
 * Revision 1.3  2004/12/03 22:54:44  radekk
 * Sync with the windows version
 *
 * Revision 1.2  2001/08/31 23:21:20  federl
 * More changes (bugfixes) performed by Samantha Filkas.
 *
 * Revision 1.3  2001/07/09 19:59:13  filkas
 * Hoping it's right now.
 *
 * Revision 1.1.1.1  2001/06/12 20:14:03  filkas
 * Imported using TkCVS
 *
 * Revision 1.1.1.1.2.1.2.2  1993/12/16  18:47:25  jamesm
 * Added "image" function.
 *
 * Revision 1.1.1.1.2.1.2.1  1993/11/15  23:37:45  mech
 * Radek's Blob version
 *
 * Revision 1.1.1.1.2.1  1993/11/13  00:02:09  jamesm
 * Experimental Environmental interaction version
 *
 * Revision 1.3  1993/10/27  20:40:02  jamesm
 * fixed problem of calling free more than once on the same token
 * removed static "trueToken" and made it allocated with malloc so that
 * it can be free'd safely.
 * Fixed bug where using an unknown variable in an expression causes a crash
 *
 * Revision 1.2  1993/10/20  21:39:29  jamesm
 * Removed MonaLisa and xpoly
 * Cleaned up compilation of lex and yacc files.
 *
 * Revision 1.1  1993/10/11  00:15:56  jamesm
 * Initial revision
 *
 *
 */

/* lsys_input.y								    */
/* Jim Hanan 9/89							    */
/* Modified:									*/
/*		September 1990 - replace production strings with link lists */
/*		October 1990   - allow definition of global constants */
/*		November 1991  - modified to include expression parsing */
/*	MODIFIED: March-June 1994 BY: Radek
	        ansi standard + prepared for C++ 
		EToken instead of Token 
		test for query symbol '?' 
*/
/* Problems:													*/
/*   - values are converted to from strings then back again     */
/*     maybe conversion should be done here rather than in lex 	*/

#ifdef WIN32
#include "warningset.h"
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "control.h"
#include "generate.h"
#include "utility.h"
#include "comlineparam.h"

#include "test_malloc.h"

#include <string.h>

#define YYDEBUG 1

extern int yylex(void);
/*extern FILE *yyin;*/

#ifdef __cplusplus
extern "C" int lsys_inputparse(void);
#endif

extern int inputError;			/* flag indicating error level */
extern char *inputFile;			/* file name of current input file */
extern int lineNumber;			/* current line number */
extern COMLINEPARAM clp;		/* command line parameters */
extern LSYSDATA *LsystemList;	        /* Lsystem data */
extern SymbolTable *currentSymbolTable;	/* symbol table stack */
extern SymbolTable *globalSymbolTable;	/* symbol table stack */
extern unsigned char constantToken[30];	/* value to be assigned as constant token */
static int lsystemCount = 0;	/* L-system counter */
static LSYSDATA *LsysPtr;		/* pointer to current L-system */

static Production *prodPtr;		/* pointer to current productions */
static Production *nextPtr;		/* pointer to productions in lists */
static Production **first;		/* pointer to ptr to the first item 
					   in an appropriate list */
static Statement *statementPtr;	/* statement pointer for definition */
static EToken *tokenPtr;			/* token pointer for definition */

#ifdef JIM
static Symbol trueSymbol;		/* storage for default condition symbol */
static double trueSymbolValue;		/* storage for default condition value */
static Symbol *externalSymbol;		/* pointer to external symbol */
#else
static Symbol trueSymbol;		/* storage for default condition value
					 */
#endif
static Module *newModule;		/* module pointer for new module */
static Module *modulePtr;		/* module pointer for ? search */
static Module *previousModule;	/* module pointer for reversing left context */
static Module *nextModule;		/* module pointer for reversing left*/
static char buffer[256];		/* string buffer */
static int bufferIndex;			/* buffer index */
static int production;			/* production counter */
static int i;					/* index for ignore tables */
static double stringIndex;		/* index for global string array */

static char first_warning;              /* to limit the number of warnings */

static char type_of_productions;
enum {NONE, L_SYSTEM, HOMOMORPHISM, DECOMPOSITION};

extern int c_expression;
extern char isObjectProduction;

/* local prototypes */
static EToken *NewToken(void);
Statement *NewStatement(void);
LSYSDATA *NewLsysData(void);
Module *NewModule(void);
static void yyerror(char *s);
EToken *OperatorToken(int tokenID, char *tokenString);
void ResetParserVars(void);

/* prototypes */
Production *AllocateProduction(void);

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

%}
%union {
	Module* module;
	int integer;
	int function;
	double value;
	EToken* expression;
	EToken* identifier;
	Parameter* parameter;
	Statement* statement;
	char *name;
	char *operator_;
	char symbol;
}
%token tLSTART tRING tSEED tDLENGTH tMDEPTH tCONSIDER tIGNORE tAXIOM tEND
%token  tDEFINE tSTARTBLOCK tENDBLOCK tSTARTEACH tENDEACH 
%token tSTARTHOMO tSTARTDECOMP tNOWARNING tWARNING
%token tLSEP tRSEP tYIELDS tOYIELDS tNULL tEOL tIF tDO tWHILE tELSE tARRAY 
%token tEXTERNAL 
%token <operator_> tLPAREN tRPAREN tLBRACE tRBRACE tCOMMA
%token <operator_> tLBRACKET tRBRACKET
%token <operator_> tSEMI
%token <function> tATAN2 tTAN tSIN tCOS tATAN tASIN tACOS
%token <function> tSRAND tRAN tNRAN tBRAN tBIRAN tEXP tLOG tFLOOR tCEIL tSIGN tSQRT tTRUNC tFABS
%token <function> tINBLOB tPRINT tFPRINTF tSTOP  tFOPEN  tFCLOSE tFFLUSH tFSCANF
%token <function> tFUNC tPLAY tSETDERIVLENGTH tGETDERIVLENGTH
%token <function> tDISPLAY
%token <function> tVVXMIN tVVXMAX tVVYMIN tVVYMAX tVVZMIN tVVZMAX tVVSCALE 
%token <function> tCURVEX tCURVEY tCURVEZ tCURVEGAL
%token <expression>  tARRAYREF tARRAYLHS tARRAYDEF tARRAYLVAL
%token <symbol> tSYMBOL
%token <name> tNAME tNAMELVAL tNAMELHS
%token <name> tSTRING
%token <value> tVALUE
%token <integer> tINTEGER

%type <module> Modules Module FormalModules FormalModule Symbol;
%type <expression> Expression Function ArrayDef ArrayDim ArrayRef LHS LValue;
%type <parameter> Parameters FormalParameters;
%type <parameter> ArrayInitBlock ArrayDefs ArrayDims ArrayRefs;
%type <identifier> FormalParameter Name String Value;
%type <function> FunctionName
%type <statement> Assignment Statement Statements Block Procedure IfStatement WhileStatement DoStatement
%type <statement> DefineBlock DefStatement DefStatements ArrayDefStatement 

%right <operator_> tASSIGN 
%right <operator_> tQUESTION tCOLON
%left <operator_> tOR
%left <operator_> tAND
%left <operator_> tEQUAL tNOTEQUAL
%left <operator_> tLT tLE tGE tGT
%left <operator_> tPLUS tMINUS
%left <operator_> tTIMES tDIVIDE tREM
%left <operator_> tPOW
%left <operator_> tUMINUS tADDRESS
%left <operator_> tNOT
%%
Lfile : {
          /* reset for reread */
          lsystemCount = 0;
          c_expression = 0;
          clp.checkEnvironment = 0;
          isObjectProduction = 0;
	  first_warning = 1;
	}
	Lsystems BlankLines
	{
	  /* return(0); */
	} 
	;

Lsystems : Lsystems Lsystem
         | /* empty */
	;

Lsystem : Header Productions Decomposition Homomorphism tEND
	{
	        type_of_productions = NONE;  /* all productions are in */

#ifdef JIM
		/* remove production instance */
		FreeSymbolInstance(prodPtr->instance); 
#endif

	        Free(prodPtr);
			prodPtr = NULL;

		/* prepare for next L-system */
#ifdef JIM
		/* remove production symbol table */
		currentSymbolTable = PopSymbolTable(currentSymbolTable); 
#endif
		/* remove L-system symbol table */
		currentSymbolTable = PopSymbolTable(currentSymbolTable); 
	} 
        ;

Homomorphism : tSTARTHOMO HomoWarning tEOL
                 /* homomorphism productions */
        {
		LsysPtr->Homomorphism.specified = 1;
		LsysPtr->Homomorphism.longest_succ = 1;
		LsysPtr->Homomorphism.stack = NULL;
		LsysPtr->Homomorphism.stack_len = 0;
		LsysPtr->Homomorphism.depth = 1;
		LsysPtr->Homomorphism.seed = -1;   /* no separate seed */
		type_of_productions = HOMOMORPHISM;
        }
         HomoItems Productions 
        |  
	;

HomoItems : HomoItems HomoItem
	  |  /* empty */
	  ;

HomoItem  : HomoSeed
	  | ProdDepth
	  ;

HomoWarning: tWARNING
        {
	  LsysPtr->homo_warning = 1;
	}
        | tNOWARNING
        {
	  LsysPtr->homo_warning = 0;
	}
        |
        ;


HomoSeed : tSEED Expression tEOL
	{
	  LsysPtr->Homomorphism.seed = (int) Eval($2);
	  printf("Homomorphism seed %d\n", 
		    LsysPtr->Homomorphism.seed);

	  VERBOSE( "Homomorphism seed %d\n", 
		    LsysPtr->Homomorphism.seed);
	}
	;

Decomposition : tSTARTDECOMP DecompWarning tEOL
                 /* homomorphism productions */
        {
		LsysPtr->Decomposition.specified = 1;
		LsysPtr->Decomposition.longest_succ = 1;
		LsysPtr->Decomposition.stack = NULL;
		LsysPtr->Decomposition.stack_len = 0;
		LsysPtr->Decomposition.depth = 1;
		type_of_productions = DECOMPOSITION;
        }
         DecompItems Productions
        |
	;

DecompWarning: tWARNING
        {
	  LsysPtr->decomp_warning = 1;
	}
        | tNOWARNING
        {
	  LsysPtr->decomp_warning = 0;
	}
        |
        ;


DecompItems : ProdDepth
        |  /* empty */
        ;

ProdDepth : tMDEPTH Expression tEOL
        {
	  switch(type_of_productions) {
	  case HOMOMORPHISM:
	    LsysPtr->Homomorphism.depth = (int) Eval($2);
	    break;
	  case DECOMPOSITION:
	    LsysPtr->Decomposition.depth = (int) Eval($2);
	    break;
	  }
	  FreeExpressionSpace($2);
        }
	;

Header : BlankLines Label Items Axiom
	;

BlankLines : BlankLines BlankLine
		 | /* empty */
	;

BlankLine : tEOL
	;

Label : tLSTART {bufferIndex = 0;} Characters tEOL
	{
		lsystemCount++;
		/* initialize if first L-system */
		if (lsystemCount == 1) {
			/* allocate space for L-system */
			LsysPtr = NewLsysData();
			LsystemList = LsysPtr;
			/* set default condition expression */
#ifndef JIM
			trueSymbol.value = 1;
#else
			trueSymbolValue=1;
			trueSymbol.values = &trueSymbolValue;
			trueSymbol.offset = 0;
			trueSymbol.defaultOffset = 0;
#endif
			trueSymbol.nextSymbol = NULL;
			trueSymbol.label = (char*) "TRUE";
		}
		else {
			LsysPtr->nextLsystem = NewLsysData();
			LsysPtr = LsysPtr->nextLsystem;
		}

		for (i=0; i<128; i++){
		  LsysPtr->Homomorphism.firstProd[i] = NULL;
		}
		LsysPtr->Homomorphism.specified = 0;
		LsysPtr->homo_warning = 1;
		
		for (i=0; i<128; i++){
		  LsysPtr->Decomposition.firstProd[i] = NULL;
		}
		LsysPtr->Decomposition.specified = 0;
		LsysPtr->decomp_warning = 1;
		
		/* Assign name and ID to L-system */
		/* must be integer for sub-Lsystems */
		LsysPtr->name = Strdup(buffer);
		LsysPtr->id = (int) atof(buffer);

		/* Set default values */
		LsysPtr->n = 0;			        /* no derivation step*/
		LsysPtr->axiom = (Module *) NULL;	/* null axiom */
		LsysPtr->stochastic = FALSE;		/* deterministic by default */
		LsysPtr->seed = 0;			/* random number seed */
		LsysPtr->ring = FALSE;			/* not a ring L-system */
		LsysPtr->symbolTable = NULL;		/* Pointer to symbol table */
#ifdef JIM
		LsysPtr->instanceList = NewSymbolInstance(64); /* Pointer to symbol table instance list*/
		LsysPtr->currentInstance=LsysPtr->instanceList; /* Pointer to symbol table current instance */
#endif
		LsysPtr->startBlock = NULL;		/* Pointer to starting statements */
		LsysPtr->endBlock = NULL; 		/* Pointer to ending statements */
		LsysPtr->startEach = NULL;		/* Pointer to step starting statements */
		LsysPtr->endEach = NULL;		/* Pointer to step ending statements */
		LsysPtr->defineBlock = NULL;		/* Pointer to definitions */
		LsysPtr->longest_succ = 1;
		/* push on symbol table stack */
#ifdef JIM
		currentSymbolTable = PushSymbolTable(currentSymbolTable, 
							&(LsysPtr->symbolTable), &(LsysPtr->instanceList));
#else
		currentSymbolTable = PushSymbolTable(currentSymbolTable, 
							&(LsysPtr->symbolTable));
#endif
		LsysPtr->nextLsystem = NULL;		/* initialize L-system link */
		for (i=0; i<128; i++){
			/* Set the ignore table */
			LsysPtr->ignore[i] = 0;
			/* Set production pointer table */
			LsysPtr->firstProd[i] = NULL;
		}

		/* initialize token for constants */
		constantToken[0]='K';
		constantToken[1]=0x80;
		constantToken[2]='\0';

		type_of_productions = L_SYSTEM;
		
		if (clp.verbose) {
			if (lsystemCount != 1) Message( "Sub-");
			Message( "L-system: %s\n", LsysPtr->name);
		}

#ifdef JIM
		CheckValues(LsysPtr, currentSymbolTable);
#endif
	} 
	;

Items : Items Item
	  |  /* empty */
	  ;

Item  : Ring
	  | Seed
	  | Dlength
	  | Ignore
	  | Consider
	  | BlankLine
	  | Defines
	  | Startblock
	  | Endblock
	  | Starteach
	  | Endeach
	  | error tEOL
	{
	  if(first_warning) {
	    WarningParsing((char*) "Invalid item", ERROR_LVL);
		first_warning = 0;
	  }
	} 
	  ;

Ring  : tRING tEOL
	{
		LsysPtr->ring = TRUE;
		VERBOSE( "Ring \n");
	} 
	;

Seed : tSEED Expression tEOL
	{
		LsysPtr->stochastic = TRUE;
		LsysPtr->seed = (int) Eval($2);
		VERBOSE( "Stochastic with seed %d\n", LsysPtr->seed);
	}
	;

Dlength : tDLENGTH Expression tEOL
	{
	  /* derivation length is important only for main Lsystem */
	  if (lsystemCount == 1) {
	    /* evaluate */
	    LsysPtr->n = (int) Eval($2);
	    /* allocate storage for globals and store */
	    SymbolTableAdd(Strdup("Q"), (double) 0.0,
                           globalSymbolTable);
            SymbolTableAdd(Strdup("Z"), (double) LsysPtr->n, globalSymbolTable);

	    VERBOSE("Derivation length is %d\n", LsysPtr->n);
	  }
	  FreeExpressionSpace($2);
	} 
	  | tDLENGTH error 
		{
		  WarningParsing((char*) "Invalid derivation length", ERROR_LVL);
		} 
	  	tEOL
	;
Consider : tCONSIDER {bufferIndex = 0;} Characters tEOL
	{
		/* Set the ignore table defaults to ignore everything */
		/* except the end of string marker (\0).			  */
		for (i=1; i<128; i++)	
			LsysPtr->ignore[i] = 1;

		/* Set symbols to be considered */
		i=0;
		while (i < 256 && buffer[i] != '\0') {
			LsysPtr->ignore[(int)(buffer[i])] = 0;
			i++;
		}

		VERBOSE( "Symbols to consider for context: %s\n", buffer);
	}
	;
Ignore : tIGNORE {bufferIndex = 0;} Characters tEOL
	{
		/* Set the ignore table defaults to consider everything */
		for (i=0; i<128; i++)		
			LsysPtr->ignore[i] = 0;

		/* Set symbols to be ignored */
		i=0;
		while (i < 256 && buffer[i] != '\0') {
			LsysPtr->ignore[(int)(buffer[i])] = 1;
			i++;
		}

		VERBOSE( "Symbols to ignore for context: %s\n", buffer);
	}
	;
Characters : tSYMBOL 
	{
		buffer[bufferIndex] = $1;
		bufferIndex++;
		buffer[bufferIndex] = '\0';
	}
		 | Characters tSYMBOL
	{
		buffer[bufferIndex] = $2;
		bufferIndex++;
		buffer[bufferIndex] = '\0';
	}
	;

Statements : Statements Statement 
	{
		$$ = BuildStatementList($1, $2);
	}
		 | 	Statement
	{
		$$ = BuildStatementList((Statement *)NULL, $1);
	}
	;

Statement : Assignment 
	{
		$$ = $1;
	}
          | Procedure
        {
		$$ = $1;
	}
          | IfStatement
        {
		$$ = $1;
	}
          | WhileStatement
        {
		$$ = $1;
	}
          | DoStatement
        {
		$$ = $1;
	}
          | error tEOL
        {
	  WarningParsing((char*) "Invalid statement", ERROR_LVL);
		$$ = NULL;
	}
	;

ArrayDim : tLBRACKET Expression tRBRACKET
	{
		$$ = $2;
	}
	;

ArrayDims : ArrayDims ArrayDim
	{
		$$ = BuildParameterList($1, $2, 'e');
	}
		 | 	ArrayDim
	{
		$$ = BuildParameterList((Parameter *)NULL, $1, 'e');
	}
	;

ArrayDef : tNAME ArrayDims
	{
		tokenPtr = NewToken();
#ifdef JIM
		tokenPtr->symbol = CurrentSymbolTableFind($1,
							  currentSymbolTable);
#else
		tokenPtr->symbol = SymbolTableFind($1, currentSymbolTable);
#endif
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd($1, 0.0, currentSymbolTable);
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", $1);
		  WarningParsing(buffer, ERROR_LVL);
		  Free($1);
		  $1 = NULL;

		}
		tokenPtr->tokenString = Strdup(yylval.name);
#ifdef JIM
		tokenPtr->symbol->type=DOUBLEARRAY;
#endif
		tokenPtr->token = tARRAYDEF;
		/* process ArrayDim parameter list and allocate storage */
#ifdef JIM
		AllocateArray(tokenPtr->symbol, $2, currentSymbolTable);
#else
		AllocateArray(tokenPtr->symbol, $2);
#endif
		$$ = tokenPtr;
	}
		 | 	tNAME ArrayDims tASSIGN ArrayInitBlock
	{
		tokenPtr = NewToken();
#ifdef JIM
		tokenPtr->symbol = CurrentSymbolTableFind($1,
							  currentSymbolTable);
#else
		tokenPtr->symbol = SymbolTableFind($1, currentSymbolTable);
#endif
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd($1, 0.0, currentSymbolTable);
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", $1);
		  WarningParsing(buffer, ERROR_LVL);
		  Free($1);
		  $1 = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);

#ifdef JIM
		tokenPtr->symbol->type=DOUBLEARRAY;
#endif
		tokenPtr->token = tARRAYDEF;
		/* process ArrayDim parameter list and allocate storage */
#ifdef JIM
		AllocateArray(tokenPtr->symbol, $2, currentSymbolTable);
#else
		AllocateArray(tokenPtr->symbol, $2);
#endif
		InitializeArray(tokenPtr->symbol, $4);
		/* CheckArrayData(tokenPtr->symbol);*/
		$$ = tokenPtr;
	}
	;

ArrayDefs : ArrayDefs tCOMMA ArrayDef
	{
		$$ = BuildParameterList($1, $3, 'e');
	}
		 | 	ArrayDef
	{
		$$ = BuildParameterList((Parameter *)NULL, $1, 'e');
	}
	;

ArrayDefStatement :	tARRAY ArrayDefs tSEMI BlankLines
	{ 
		statementPtr = NewStatement();
		statementPtr->expression = BuildExprList((EToken *)NULL, $2, TRUE) ;
		statementPtr->type = stmntARRAYDEF;
		$$ = statementPtr;
	}
	;

/* external definitions JH97 */
ExternalDef : tNAME
	{
#ifdef JIM
		externalSymbol = CurrentSymbolTableFind($1, currentSymbolTable);
		if (externalSymbol == NULL) {
		  externalSymbol = SymbolTableAdd($1, 0.0, currentSymbolTable);
		  externalSymbol->type=EXTERNAL;
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", $1);
		  WarningParsing(buffer, ERROR_LVL);
		  Free($1);
		  $1 = NULL;
		}
#endif
	}
		| tNAME ArrayDims
	{
#ifdef JIM
		externalSymbol = CurrentSymbolTableFind($1, currentSymbolTable);
		if (externalSymbol == NULL) {
		  externalSymbol = SymbolTableAdd($1, 0.0, currentSymbolTable);
		  externalSymbol->type=EXTERNALARRAY;
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", $1);
		  WarningParsing(buffer, ERROR_LVL);
		  Free($1);
		  $1 = NULL;
		}
		/* process ArrayDim parameter list and allocate storage */
		AllocateArray(externalSymbol, $2, currentSymbolTable);
#endif		
	}
		 | 	tNAME ArrayDims tASSIGN ArrayInitBlock
	{
#ifdef JIM
		externalSymbol = CurrentSymbolTableFind($1, currentSymbolTable);
		if (externalSymbol == NULL) {
		  externalSymbol = SymbolTableAdd($1, 0.0, currentSymbolTable);
		  externalSymbol->type=EXTERNALARRAY;
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", $1);
		  WarningParsing(buffer, ERROR_LVL);
		  Free($1);
		  $1 = NULL;
		}

		/* process ArrayDim parameter list and allocate storage */
		AllocateArray(externalSymbol, $2, currentSymbolTable);
		
		/* initialisation not allowed */
		sprintf(buffer, "Array initialisation ignored for external variable %s", $1);
		WarningParsing(buffer, WARNING_LVL);
		FreeParameterList($4);
#endif
	}
	;


ExternalDefs : ExternalDefs tCOMMA ExternalDef
	{
	}
		 | 	ExternalDef
	{
	}
	;

ExternalDefStatement :	tEXTERNAL ExternalDefs tSEMI BlankLines
	{
	}
	;

DefStatement : ArrayDefStatement 
	{
		$$ = $1;
	}
          | ExternalDefStatement
    {
			$$ = NULL;
	}
          | error tEOL
    {
      WarningParsing((char*) "Invalid statement in Define:", ERROR_LVL);
		$$ = NULL;
	}
	;

DefStatements : DefStatements DefStatement 
	{
		$$ = BuildStatementList($1, $2);
	}
		 | 	DefStatement
	{
		$$ = BuildStatementList((Statement *)NULL, $1);
	}
	;

Assignment : LHS tASSIGN Expression tSEMI BlankLines
	{
		statementPtr = NewStatement();
		statementPtr->leftHandSide = $1;
		statementPtr->expression = $3;

		/* LHS is either a variable or an array reference */
		if (statementPtr->leftHandSide->nextParam == NULL) {
			statementPtr->type = stmntASSIGN;
		}
		else {
			statementPtr->type = stmntARRAYASSIGN;
		}
		$$ = statementPtr;
	}
	;

ArrayRef : tLBRACKET Expression tRBRACKET
	{
		$$ = $2;
	}
	;

ArrayRefs : ArrayRefs ArrayRef
	{
		$$ = BuildParameterList($1, $2, 'e');
	}
		 | 	ArrayRef
	{
		$$ = BuildParameterList((Parameter *)NULL, $1, 'e');
	}
	;

LValue :	tADDRESS tNAME
	{ 
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind($2, currentSymbolTable);
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd($2, 0.0, currentSymbolTable);
		}
		else { /* here's the new stuff */
		  if (tokenPtr->symbol->type==DOUBLEARRAY 
#ifdef JIM
		      || tokenPtr->symbol->type==EXTERNALARRAY
#endif
		      ) {
		  	sprintf(buffer, "Identifier %s previously defined as an array", $1);
		  	WarningParsing(buffer, ERROR_LVL);
		   } /* new bit ends here */
		  Free($2);
		  $2 = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tNAMELVAL;
		$$ = tokenPtr;
	}
	 |	tADDRESS tNAME ArrayRefs
	{
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind($2, currentSymbolTable);
		if (tokenPtr->symbol == NULL||tokenPtr->symbol->arrayData == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined as an array", $2);
		  WarningParsing(buffer, ERROR_LVL);
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tARRAYLVAL;
		$$ = BuildExprList(tokenPtr, $3, TRUE); 
	}
	
	;

LHS :	tNAME
	{ 
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind($1, currentSymbolTable);
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd($1, 0.0, currentSymbolTable);
		}
		else { /* here's the new stuff */
		  if (tokenPtr->symbol->type==DOUBLEARRAY 
#ifdef JIM
		      || tokenPtr->symbol->type==EXTERNALARRAY
#endif
		      ) {
		  	sprintf(buffer, "Identifier %s previously defined as an array", $1);
		  	WarningParsing(buffer, ERROR_LVL);
		   } /* new bit ends here */
		  Free($1);
		  $1 = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tNAME;
		$$ = tokenPtr;
	}
	 |	tNAME ArrayRefs
	{
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind($1, currentSymbolTable);
		if (tokenPtr->symbol == NULL||tokenPtr->symbol->arrayData == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined as an array", $1);
		  WarningParsing(buffer, ERROR_LVL);
		}
		tokenPtr->tokenString = Strdup(yylval.name);
        Free($1);
		$1 = NULL;
		tokenPtr->token = tARRAYLHS;
		$$ = BuildExprList(tokenPtr, $2, TRUE); 
	}
	
	;

Procedure	: Expression tSEMI BlankLines
	{ 
		statementPtr = NewStatement();
		statementPtr->expression =  $1;
		statementPtr->type = stmntPROC;
		$$ = statementPtr;
	}
	    ;

IfStatement	: tIF tLPAREN Expression tRPAREN BlankLines Block BlankLines
	{ 
		statementPtr = NewStatement();
		statementPtr->condition =  $3;
		statementPtr->type = stmntIF;
		statementPtr->block = $6;
		statementPtr->elseblock = NULL;
		$$ = statementPtr;
	}
          | tIF tLPAREN Expression tRPAREN BlankLines Block tELSE BlankLines Block BlankLines
	{ 
		statementPtr = NewStatement();
		statementPtr->condition =  $3;
		statementPtr->type = stmntIF;
		statementPtr->block = $6;
		statementPtr->elseblock = $9;
		$$ = statementPtr;
	}
	    ;

WhileStatement	: tWHILE tLPAREN Expression tRPAREN BlankLines Block BlankLines
	{ 
		statementPtr = NewStatement();
		statementPtr->condition =  $3;
		statementPtr->type = stmntWHILE;
		statementPtr->block = $6;
		$$ = statementPtr;
	}
	    ;

DoStatement	: tDO BlankLines Block tWHILE tLPAREN Expression tRPAREN tSEMI BlankLines
	{ 
		statementPtr = NewStatement();
		statementPtr->condition =  $6;
		statementPtr->type = stmntDO;
		statementPtr->block = $3;
		$$ = statementPtr;
	}
	    ;


DefineBlock : LBRACE BlankLines DefStatements RBRACE  /* BlankLines ? */
	{
		$$ = $3;
	}
	;

Defines : tDEFINE DefineBlock tEOL
	{
		LsysPtr->defineBlock = $2;
		if (clp.verbose) {
			Message( "Define: ");
			PrintStatementList($2);
		}
	}
	;

ArrayInitBlock : LBRACE BlankLines Parameters RBRACE  /* BlankLines ? */
	{
		$$ = $3;
	}
	;

Block : LBRACE BlankLines Statements RBRACE  /* BlankLines ? */
	{
		$$ = $3;
	}
	;

LBRACE : tLBRACE
        {
		c_expression++; /* lines doesn't have to start with tabs */
        }
	;

RBRACE : tRBRACE
        {
		if(c_expression>0)
		  c_expression--; /* the end of c_like expression */
		else
		  Message("Mismatched parenthesis!");
        }
	; 

Startblock : tSTARTBLOCK Block tEOL
	{
		LsysPtr->startBlock = $2;
		if (clp.verbose) {
			Message( "Start: ");
			PrintStatementList($2);
		}
	}
	;

Endblock : tENDBLOCK Block tEOL
	{
		LsysPtr->endBlock = $2;
		if (clp.verbose) {
			Message( "End: ");
			PrintStatementList($2);
		}
	}
	;

Starteach : tSTARTEACH Block tEOL
	{
		LsysPtr->startEach = $2;
		if (clp.verbose) {
			Message( "StartEach: ");
			PrintStatementList($2);
		}
	}
	;

Endeach : tENDEACH Block tEOL
	{
		LsysPtr->endEach = $2;
		if (clp.verbose) {
			Message( "EndEach: ");
			PrintStatementList($2);
		}
	}
	;

Axiom : tAXIOM Modules tEOL
	{
        LsysPtr->axiom = $2;
        LsysPtr->axiomLength = PackedModuleListLength($2);
	prodPtr = AllocateProduction(); /* space for first production */
#ifdef JIM
	currentSymbolTable = PushSymbolTable(currentSymbolTable, 
					     &(prodPtr->symbolTable),
					     &(prodPtr->instance));
#else
	currentSymbolTable = PushSymbolTable(currentSymbolTable, 
					     &(prodPtr->symbolTable));
#endif
	production=0;		/* initialize production counter */

        if (clp.verbose) {
	  Message( "Axiom: ");
	  PrintModuleList(LsysPtr->axiom);
	}

	}
    | tAXIOM error tEOL
	{
	  WarningParsing((char*) "Error in axiom modules", ERROR_LVL);
	  /* set up to continue error checking */
	  prodPtr = AllocateProduction(); /* space for first production */
	  currentSymbolTable = PushSymbolTable(currentSymbolTable, 
					       &(prodPtr->symbolTable)
#ifdef JIM
					       ,&(prodPtr->instance)
#endif
					       );
	  production=0;	 /* initialize production counter */
	}
	;

Productions : Productions Production
			| Production
			;

Production :  BlankLine 
			| Predecessor Conditional tYIELDS Successor tEOL
	{
		/* allocate space for the next production */
		prodPtr->object_flag = 0;
		prodPtr = AllocateProduction();	
		currentSymbolTable = PopSymbolTable(currentSymbolTable); 
		currentSymbolTable = PushSymbolTable(currentSymbolTable, 
						     &(prodPtr->symbolTable)
#ifdef JIM
						     ,&(prodPtr->instance)
#endif
						     );
	}
			| Predecessor Conditional tOYIELDS Successor tEOL
	{
		/* allocate space for the next production */
		prodPtr->object_flag = 1;
		prodPtr = AllocateProduction();
		isObjectProduction = 1;
		currentSymbolTable = PopSymbolTable(currentSymbolTable); 
		currentSymbolTable = PushSymbolTable(currentSymbolTable, 
						     &(prodPtr->symbolTable)
#ifdef JIM
						     ,&(prodPtr->instance)
#endif
						     );
	}
			| error
	{
	  WarningParsing((char*) "Ill-formed production", ERROR_LVL);
		/* allocate space for the next production */
		prodPtr = AllocateProduction();
		currentSymbolTable = PopSymbolTable(currentSymbolTable); 
		currentSymbolTable = PushSymbolTable(currentSymbolTable, 
						     &(prodPtr->symbolTable)
#ifdef JIM
						     ,&(prodPtr->instance)
#endif
						     );
	}
			  tEOL
		   ;

Predecessor : Strictpred 
			| Lcontext tLSEP Strictpred 
			| Strictpred tRSEP Rcontext
			| Lcontext tLSEP Strictpred tRSEP Rcontext
	;


Lcontext : tNULL
	{
		prodPtr->lCon = (Module *) NULL;
  		VERBOSE( "Left context: NULL\n");
	}
		| FormalModules
	{ 
		modulePtr = $1;

   		if (clp.verbose) {
			Message( "Left context: ");
			PrintModuleList(modulePtr);
		}

		/* Set the left context to the reverse of the module list */
		/* in order to simplify left context search later         */
		previousModule = (Module *) NULL;
		while (modulePtr != NULL) {
			nextModule = modulePtr->nextModule;
			modulePtr->nextModule = previousModule;
			previousModule = modulePtr;
			modulePtr = nextModule;
		}

	        switch(type_of_productions) {
		case L_SYSTEM:
		case DECOMPOSITION:
#ifdef CONTEXT_SENSITIVE_HOMO
		case HOMOMORPHISM:
#endif
		  prodPtr->lCon = previousModule;
		  break;

#ifndef CONTEXT_SENSITIVE_HOMO
		case HOMOMORPHISM:
		  Message("Context for a homomorphism production is ignored!\n");
		  prodPtr->lCon = (Module *) NULL;
		  break;
#endif

		case NONE:
		  Message("Production not expected!\n");
		}
	}
	;

Strictpred : FormalModules
	{ /* Set the strict predecessor */
       	modulePtr = prodPtr->pred = $1;

	/* Check for presence of sub-Lsystem operators */
	prodPtr->lsystemChange = 0;
#ifdef JIM
	prodPtr->lsystemInPred = 0;
#endif
	while (modulePtr != NULL) {
#ifndef JIM
	  if ((modulePtr->symbol == '?' && modulePtr->parameters > 0) ||
	      modulePtr->symbol == '$') prodPtr->lsystemChange = 1;
#else
	  if (modulePtr->symbol == '?' && modulePtr->parameters > 0) {
	    prodPtr->lsystemInPred = 1;
	    prodPtr->lsystemChange = 1;
	  }
	  if (modulePtr->symbol == '$') prodPtr->lsystemChange = 1;
#endif
	  modulePtr = modulePtr->nextModule;
	}

	/* add production pointer to the end of the appropriate list */
	switch(type_of_productions) {
	case L_SYSTEM:
	  first = &(LsysPtr->firstProd[(int)(prodPtr->pred->symbol)]);
	  break;
	  
	case HOMOMORPHISM:
	  first = &(LsysPtr->Homomorphism.firstProd[(int)(prodPtr->pred->symbol)]);
	  break;
	  
	case DECOMPOSITION:
	  first = &(LsysPtr->Decomposition.firstProd[(int)(prodPtr->pred->symbol)]);
	  break;
	  
	case NONE:
	  Message("Production not expected!\n");
	}	
	
	if (*first == NULL) *first = prodPtr;
	else {
	  nextPtr = *first;
	  while (nextPtr->nextProduction != NULL) {
	    nextPtr = nextPtr->nextProduction;
	  }
	  nextPtr->nextProduction = prodPtr;
	}
	
        if (clp.verbose) {
	  Message( "Strict Predecessor: ");
	  PrintModuleList(prodPtr->pred);
	}
	}
	;

Rcontext : tNULL
	{
		prodPtr->rCon = (Module *) NULL;
      	VERBOSE( "Right context: NULL\n");
	}
			| FormalModules
	{ /* Set the right context of the predecessor.              */
	  switch(type_of_productions) {
	  case L_SYSTEM:
	  case DECOMPOSITION:
#ifdef CONTEXT_SENSITIVE_HOMO
	  case HOMOMORPHISM:
#endif
	    prodPtr->rCon = $1;
	    break;

#ifndef CONTEXT_SENSITIVE_HOMO
	  case HOMOMORPHISM:
	    Message(
		    "Context for a homomorphism production is ignored!\n");
	    prodPtr->rCon = (Module *) NULL;
	    break;
#endif
	    
	  case NONE:
	    Message("Production not expected!\n");
	  }	
	  
	  if (clp.verbose) {
	    Message( "Right context: ");
	    PrintModuleList(prodPtr->rCon);
	  }
	}
	;

Conditional : tCOLON Condition
			{ $1; }
			| tCOLON Precondition Condition
			{ $1; }
			| tCOLON Condition Postcondition 
			{ $1; }
			| tCOLON Precondition Condition Postcondition
			{ $1; }
			| /* null */ 
	{
		/* Condition statement does not exist. Assume true */
		EToken *trueToken = NewToken();
		trueToken->token = tVALUE;
		trueToken->symbol = &trueSymbol;
		trueToken->tokenString = Strdup("TRUE");

		prodPtr->condition = trueToken;

        VERBOSE( "Condition: TRUE\n");
	}
	;

Precondition : Block
	{
		prodPtr->preCondList = $1;

        if (clp.verbose) {
			Message( "Precondition Statements: ");
			PrintStatementList($1);
		}
	}
	;

Postcondition : Block
	{
		prodPtr->postCondList = $1;

        if (clp.verbose) {
			Message( "Postcondition Statements: ");
			PrintStatementList($1);
		}
	}
	;

Condition : tTIMES
	{
		/* Condition statement does not exist. Assume true */
		EToken *trueToken = NewToken();
		trueToken->token = tVALUE;
		trueToken->symbol = &trueSymbol;
		trueToken->tokenString = Strdup("TRUE");
		
		prodPtr->condition = trueToken;

        VERBOSE( "Condition: TRUE\n");
	}
			| Expression
	{
		prodPtr->condition = $1;

    	if (clp.verbose) {
			Message( "Condition: ");
			PrintExpression(prodPtr->condition);
			Message( "\n");
		}
	}
	;

Successor : StrictSucc 
	{
		if ( LsysPtr->stochastic ) {
			/* Warning("Probability not given for stochastic production", 
					WARNING_LVL); */
			/* Condition statement does not exist. Assume true */
			EToken *oneToken = NewToken();
			oneToken->token = tVALUE;
			oneToken->symbol = &trueSymbol;
			oneToken->tokenString = Strdup("ONE");
		
			prodPtr->probExpression =  oneToken;
		}
	} 
			| StrictSucc Probability 
	
	{ 
		if ( ! LsysPtr->stochastic ) {
			WarningParsing((char*) "Probability given for deterministic production", 
					WARNING_LVL);
		}
	}
	;

StrictSucc : tNULL
	{
		/* The successor is empty */
		prodPtr->succ = (Module *)NULL;
		prodPtr->succLen = 0;
		VERBOSE( "Successor: NULL\n");
	}
			| Modules
	{ /* The successor is not empty */
#ifdef JIM
		modulePtr = prodPtr->succ = $1;
#else
		prodPtr->succ = $1;
#endif
		prodPtr->succLen = PackedModuleListLength($1);

	        switch(type_of_productions) {
		case L_SYSTEM:
		  if(prodPtr->succLen > LsysPtr->longest_succ)
		    LsysPtr->longest_succ = prodPtr->succLen;
		  break;

		case HOMOMORPHISM:
		  if(prodPtr->succLen > LsysPtr->Homomorphism.longest_succ)
		    LsysPtr->Homomorphism.longest_succ = prodPtr->succLen;
		  break;

		case DECOMPOSITION:
		  if(prodPtr->succLen > LsysPtr->Decomposition.longest_succ)
		    LsysPtr->Decomposition.longest_succ = prodPtr->succLen;
		  break;
		
		}	

#ifdef JIM
		prodPtr->lsystemInSucc = 0;
		while (modulePtr != NULL) {
			if (modulePtr->symbol == '?') prodPtr->lsystemInSucc = 1;
			modulePtr = modulePtr->nextModule;
		}
#endif

		if (clp.verbose) {
			Message( "Successor: ");
			PrintModuleList(prodPtr->succ);
		}
	}
	;

Probability : tCOLON Expression
	{ /* convert probability */
		prodPtr->probExpression = $2;

		if (clp.verbose) {
			Message( "Probability: ");
			PrintExpression($2);
		}
	}
	;

FormalModules :	FormalModule
	{
		$$ = $1;
	}
			| FormalModules FormalModule
	{ 
		$$ = BuildModuleList($1, $2);
	}
	;
FormalModule :	Symbol
	{
		$$ = $1;
		if($1->symbol == '?') clp.checkEnvironment = 1;
	}
			| Symbol tLPAREN FormalParameters tRPAREN
	{ 
		/* assign parameter list to module */
		if($1->symbol == '%') clp.checkEnvironment = 1;

		$$ = AssignParameters($1, $3);
	}
	;
Modules :	Module
	{
		$$ = $1;
	}
			| Modules Module
	{ 
		$$ = BuildModuleList($1, $2);
	}
	;
Module :	Symbol
	{
		$$ = $1;
		if($1->symbol == '?') clp.checkEnvironment = 1;
	}
			| Symbol tLPAREN Parameters tRPAREN
	{ 
		/* assign parameter list to module $1 */
		if($1->symbol == '%') clp.checkEnvironment = 1;

		$$ = AssignParameters($1, $3);
	}
	;
Symbol :	tSYMBOL
	{
		/* allocate module */
		newModule = NewModule();
		/* get the current symbol and initialize the structure */
    	newModule->symbol = $1;
    	newModule->length = 1;
    	newModule->parameters = 0;
    	newModule->parmList = NULL;

		$$ = newModule;
	}
	;
FormalParameters :	FormalParameter
	{
		$$ = BuildParameterList((Parameter *)NULL, $1, 'f');
	}
			| FormalParameters tCOMMA FormalParameter
	{ 
		$$ = BuildParameterList($1, $3, 'f');
	}
	;
FormalParameter :	tNAME
	{
		tokenPtr = NewToken();
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tNAME;
		/* add to symbol table; warning if duplicated */
		tokenPtr->symbol = SymbolTableAdd($1, 0.0,currentSymbolTable);
		$$ = tokenPtr;
	}
	;
Parameters :	Expression
	{
		$$ = BuildParameterList((Parameter *)NULL, $1, 'e');
	}
			| Parameters tCOMMA Expression
	{ 
		$$ = BuildParameterList($1, $3, 'e');
	}
	;

Expression :	Expression tOR Expression
	{ $$ = BuildBinary($1, OperatorToken(tOR, $2), $3); }
	    	|	Expression tAND Expression
	{ $$ = BuildBinary($1, OperatorToken(tAND, $2), $3); }
	    	|	Expression tEQUAL Expression
	{ $$ = BuildBinary($1, OperatorToken(tEQUAL, $2), $3); }
	    	|	Expression tNOTEQUAL Expression
	{ $$ = BuildBinary($1, OperatorToken(tNOTEQUAL, $2), $3); }
	    	|	Expression tLT Expression
	{ $$ = BuildBinary($1, OperatorToken(tLT, $2), $3); }
	    	|	Expression tLE Expression
	{ $$ = BuildBinary($1, OperatorToken(tLE, $2), $3); }
	    	|	Expression tGT Expression
	{ $$ = BuildBinary($1, OperatorToken(tGT, $2), $3); }
	    	|	Expression tGE Expression
	{ $$ = BuildBinary($1, OperatorToken(tGE, $2), $3); }
	    	|	Expression tPLUS Expression
	{ $$ = BuildBinary($1, OperatorToken(tPLUS, $2), $3); }
	    	|	Expression tMINUS Expression
	{ $$ = BuildBinary($1, OperatorToken(tMINUS, $2), $3); }
	    	|	Expression tTIMES Expression
	{ $$ = BuildBinary($1, OperatorToken(tTIMES, $2), $3); }
	    	|	Expression tDIVIDE Expression
	{ $$ = BuildBinary($1, OperatorToken(tDIVIDE, $2), $3); }
	    	|	Expression tREM Expression
	{ $$ = BuildBinary($1, OperatorToken(tREM, $2), $3); }
	    	|	Expression tPOW Expression
	{ $$ = BuildBinary($1, OperatorToken(tPOW, $2), $3); }
		    |	tMINUS Expression		    %prec tUMINUS
	{ $$ = BuildUnary(OperatorToken(tUMINUS, $1), $2); }
		    |	tNOT Expression
	{ $$ = BuildUnary(OperatorToken(tNOT, $1), $2); }
		    |	Expression tQUESTION Expression tCOLON Expression
	{ $$ = BuildTrinary(OperatorToken(tQUESTION, $2), $1, $3, $5); }
		    |	tLPAREN Expression tRPAREN
	{ $$ = $2; }
		    |	LValue		    
	{ $$ = $1; }
		    |	Function
	{ $$ = $1; }
		    |	Name
	{ $$ = $1; }
		    |	Value
	{ $$ = $1; }
		    |	String
	{ $$ = $1; }
	    ;
Function	:	FunctionName tLPAREN Parameters tRPAREN
	{ 
		int argcount = MatchParameters($3,$1);		/* argcount must be passed to printf */
		$$ = BuildExprList(OperatorToken($1,(char*) "function"),$3,argcount); 
	}
	    ;
FunctionName	:	tTAN | tSIN | tCOS | tATAN | tASIN | tACOS
  | tRAN | tNRAN | tBRAN | tBIRAN | tSRAND | tEXP | tLOG | tFLOOR | tCEIL | tTRUNC | tFABS 
  | tSIGN | tINBLOB | tSTOP
  | tSQRT | tPRINT | tFPRINTF | tFOPEN | tFCLOSE | tFFLUSH | tFSCANF
  | tFUNC | tPLAY | tSETDERIVLENGTH | tGETDERIVLENGTH
  | tDISPLAY
  | tVVXMIN | tVVXMAX | tVVYMIN | tVVYMAX | tVVZMIN | tVVZMAX | tVVSCALE
  | tCURVEX | tCURVEY | tCURVEZ | tCURVEGAL
	    ;

Value	    :	tVALUE
	{ 
		tokenPtr = NewToken();
		tokenPtr->symbol = 
                    SymbolTableAdd(Strdup((char *) constantToken),
				 $1, currentSymbolTable);
		tokenPtr->token = tVALUE;
		sprintf(buffer,"%g",$1);
		tokenPtr->tokenString = Strdup(buffer);
		if(constantToken[strlen((char *) constantToken)-1] == 255 ) {
                    constantToken[strlen((char *) constantToken)+1] = '\0';
                    constantToken[strlen((char *) constantToken)] = '\200';
		}
		else
                    constantToken[strlen((char *) constantToken)-1]++;
		$$ = tokenPtr;
	}
	;

String	    :	tSTRING
	{ 
		tokenPtr = NewToken();
		/* tokenPtr->symbol = NULL; JH1 */
		tokenPtr->token = tSTRING;
		tokenPtr->tokenString = Strdup($1);
		/* store string pointer into global array for later access - JH1 */
		stringIndex = GlobalStringIndex(tokenPtr->tokenString);
		if (stringIndex>MAXSTRINGS-.5) /* float-integer compare */ {
		    WarningParsing((char*) "Not enough space for global strings", ERROR_LVL);
		    stringIndex = MAXSTRINGS-1;    
		}
		tokenPtr->symbol = 
                    SymbolTableAdd(Strdup((char *) constantToken),
				 stringIndex, currentSymbolTable);
		if(constantToken[strlen((char *) constantToken)-1] == 255){
                    constantToken[strlen((char *) constantToken)+1] = '\0';
                    constantToken[strlen((char *) constantToken)] = '\200';
		}
		else
                    constantToken[strlen((char *) constantToken)-1]++;
		/* end JH1 */				
		$$ = tokenPtr;
	}
	;

Name	    :	tNAME
	{ 
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind($1, currentSymbolTable);
		if (tokenPtr->symbol == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined", $1);
		  WarningParsing(buffer, WARNING_LVL);
		  tokenPtr->symbol = SymbolTableAdd($1, 0.0, currentSymbolTable);
		}
		else { /* here's the new stuff */
		  if (tokenPtr->symbol->type==DOUBLEARRAY 
#ifdef JIM
		      || tokenPtr->symbol->type==EXTERNALARRAY
#endif
		      ) {
		  	sprintf(buffer, "Identifier %s previously defined as an array", $1);
		  	WarningParsing(buffer, ERROR_LVL);
		   } /* new bit ends here */

		  Free($1);
		  $1 = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);

		tokenPtr->token = tNAME;


		$$ = tokenPtr;
	}
			| tNAME ArrayRefs
	{
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind($1, currentSymbolTable);
		if (tokenPtr->symbol == NULL||tokenPtr->symbol->arrayData == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined as an array", $1);
		  WarningParsing(buffer, ERROR_LVL);
		  Free(tokenPtr);
		  tokenPtr = NULL;
		  Free($1);
		  $1 = NULL;
		  return 0;
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tARRAYREF;
		i = CountParameters($2);
		if (tokenPtr->symbol->arrayData->dimensions != i) {
			sprintf(buffer, "Array identifier %s requires %d dimensions, not %d", 
				tokenPtr->symbol->label, tokenPtr->symbol->arrayData->dimensions, i);
			WarningParsing(buffer, ERROR_LVL);
			
		}

		Free($1);
		$1 = NULL;

		$$ = BuildExprList(tokenPtr, $2, TRUE); 
	}
	;
%%

static void yyerror( char * s)
{
	WarningParsing((char*) "Syntax error", ERROR_LVL);
	
}

/***************************************************************/
/*   OperatorToken - convert the given token identifier into   */
/*                   the appropriate Token structure           */
/***************************************************************/
EToken *OperatorToken(int tokenID, char *tokenString)
{
	EToken* tokenPtr;

	tokenPtr = NewToken();
	tokenPtr->token = tokenID;
	tokenPtr->tokenString = Strdup(tokenString);
	return tokenPtr;
}

/***************************************************************/
/*   AllocateProduction and initialize                         */ 
/***************************************************************/
Production *AllocateProduction(void)
{
	Production * prodPtr;

	/* Allocate production storage */
	if (( prodPtr = (Production *) Malloc(sizeof(Production))) == NULL) 
	{
		WarningParsing((char*) "Production memory allocation failed", INTERNAL_LVL);
	}

	/* Initialize production structure */
	prodPtr->lCon = NULL;
	prodPtr->pred = NULL;
	prodPtr->rCon = NULL;
	prodPtr->succ = NULL;
	prodPtr->preCondList = NULL;
	prodPtr->condition = NULL;
	prodPtr->postCondList = NULL;
	prodPtr->probExpression = NULL;
	prodPtr->prob = 0;
   	prodPtr->predLen = 0;		/* Set default predecessor length */
   	prodPtr->succLen = 0;		/* Set default successor length */
#ifdef JIM
	/* Allocate production symbol table value storage */
	prodPtr->instance = NewSymbolInstance(16);	
#endif
	prodPtr->symbolTable = NULL;
	prodPtr->nextProduction = NULL;
	production++;
	return prodPtr;
}

static EToken *NewToken(void)
{
	EToken *tokenPtr;
	if (( tokenPtr = (EToken *) Malloc(sizeof(EToken))) == NULL) 
	{
		WarningParsing((char*) "Token memory allocation failed", INTERNAL_LVL);
	}
	tokenPtr->symbol = NULL;
	tokenPtr->tokenString = NULL;
	tokenPtr->nextParam = NULL;
	tokenPtr->up = NULL;
	return tokenPtr;
}

Statement *NewStatement(void)
{
	Statement *statementPtr;
	if (( statementPtr = (Statement *) Malloc(sizeof(Statement))) == NULL) 
	{
		WarningParsing((char*) "Statement memory allocation failed", INTERNAL_LVL);
	}
	statementPtr->leftHandSide = NULL;
	statementPtr->expression = NULL;
	statementPtr->condition = NULL;
	statementPtr->block = NULL;
	statementPtr->elseblock = NULL;
	statementPtr->nextStatement = NULL;
	return statementPtr;
}


LSYSDATA *NewLsysData(void)
{
	LSYSDATA *LsysDataPtr;
	if (( LsysDataPtr = (LSYSDATA *) Malloc(sizeof(LSYSDATA))) == NULL) 
	{
		WarningParsing((char*) "LSYSDATA memory allocation failed", INTERNAL_LVL);
	}
	return LsysDataPtr;
}

Module *NewModule(void)
{
	Module *ModulePtr;
	if (( ModulePtr = (Module *) Malloc(sizeof(Module))) == NULL) 
	{
		WarningParsing((char*) "Module memory allocation failed", INTERNAL_LVL);
	}
	ModulePtr->parmList = NULL;
	ModulePtr->nextModule = NULL;
	ModulePtr->matchedSymbol = NULL;	
	return ModulePtr;
}

void YaccDebug(int callno)
{
  Message( "Yacc debug %d\n",  callno);
}
