/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         lsys_inputparse
#define yylex           lsys_inputlex
#define yyerror         lsys_inputerror
#define yydebug         lsys_inputdebug
#define yynerrs         lsys_inputnerrs
#define yylval          lsys_inputlval
#define yychar          lsys_inputchar

/* First part of user prologue.  */
#line 1 "lsys_input.y"

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


#line 258 "lsys_input.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "lsys_input.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_tLSTART = 3,                    /* tLSTART  */
  YYSYMBOL_tRING = 4,                      /* tRING  */
  YYSYMBOL_tSEED = 5,                      /* tSEED  */
  YYSYMBOL_tDLENGTH = 6,                   /* tDLENGTH  */
  YYSYMBOL_tMDEPTH = 7,                    /* tMDEPTH  */
  YYSYMBOL_tCONSIDER = 8,                  /* tCONSIDER  */
  YYSYMBOL_tIGNORE = 9,                    /* tIGNORE  */
  YYSYMBOL_tAXIOM = 10,                    /* tAXIOM  */
  YYSYMBOL_tEND = 11,                      /* tEND  */
  YYSYMBOL_tDEFINE = 12,                   /* tDEFINE  */
  YYSYMBOL_tSTARTBLOCK = 13,               /* tSTARTBLOCK  */
  YYSYMBOL_tENDBLOCK = 14,                 /* tENDBLOCK  */
  YYSYMBOL_tSTARTEACH = 15,                /* tSTARTEACH  */
  YYSYMBOL_tENDEACH = 16,                  /* tENDEACH  */
  YYSYMBOL_tSTARTHOMO = 17,                /* tSTARTHOMO  */
  YYSYMBOL_tSTARTDECOMP = 18,              /* tSTARTDECOMP  */
  YYSYMBOL_tNOWARNING = 19,                /* tNOWARNING  */
  YYSYMBOL_tWARNING = 20,                  /* tWARNING  */
  YYSYMBOL_tLSEP = 21,                     /* tLSEP  */
  YYSYMBOL_tRSEP = 22,                     /* tRSEP  */
  YYSYMBOL_tYIELDS = 23,                   /* tYIELDS  */
  YYSYMBOL_tOYIELDS = 24,                  /* tOYIELDS  */
  YYSYMBOL_tNULL = 25,                     /* tNULL  */
  YYSYMBOL_tEOL = 26,                      /* tEOL  */
  YYSYMBOL_tIF = 27,                       /* tIF  */
  YYSYMBOL_tDO = 28,                       /* tDO  */
  YYSYMBOL_tWHILE = 29,                    /* tWHILE  */
  YYSYMBOL_tELSE = 30,                     /* tELSE  */
  YYSYMBOL_tARRAY = 31,                    /* tARRAY  */
  YYSYMBOL_tEXTERNAL = 32,                 /* tEXTERNAL  */
  YYSYMBOL_tLPAREN = 33,                   /* tLPAREN  */
  YYSYMBOL_tRPAREN = 34,                   /* tRPAREN  */
  YYSYMBOL_tLBRACE = 35,                   /* tLBRACE  */
  YYSYMBOL_tRBRACE = 36,                   /* tRBRACE  */
  YYSYMBOL_tCOMMA = 37,                    /* tCOMMA  */
  YYSYMBOL_tLBRACKET = 38,                 /* tLBRACKET  */
  YYSYMBOL_tRBRACKET = 39,                 /* tRBRACKET  */
  YYSYMBOL_tSEMI = 40,                     /* tSEMI  */
  YYSYMBOL_tATAN2 = 41,                    /* tATAN2  */
  YYSYMBOL_tTAN = 42,                      /* tTAN  */
  YYSYMBOL_tSIN = 43,                      /* tSIN  */
  YYSYMBOL_tCOS = 44,                      /* tCOS  */
  YYSYMBOL_tATAN = 45,                     /* tATAN  */
  YYSYMBOL_tASIN = 46,                     /* tASIN  */
  YYSYMBOL_tACOS = 47,                     /* tACOS  */
  YYSYMBOL_tSRAND = 48,                    /* tSRAND  */
  YYSYMBOL_tRAN = 49,                      /* tRAN  */
  YYSYMBOL_tNRAN = 50,                     /* tNRAN  */
  YYSYMBOL_tBRAN = 51,                     /* tBRAN  */
  YYSYMBOL_tBIRAN = 52,                    /* tBIRAN  */
  YYSYMBOL_tEXP = 53,                      /* tEXP  */
  YYSYMBOL_tLOG = 54,                      /* tLOG  */
  YYSYMBOL_tFLOOR = 55,                    /* tFLOOR  */
  YYSYMBOL_tCEIL = 56,                     /* tCEIL  */
  YYSYMBOL_tSIGN = 57,                     /* tSIGN  */
  YYSYMBOL_tSQRT = 58,                     /* tSQRT  */
  YYSYMBOL_tTRUNC = 59,                    /* tTRUNC  */
  YYSYMBOL_tFABS = 60,                     /* tFABS  */
  YYSYMBOL_tINBLOB = 61,                   /* tINBLOB  */
  YYSYMBOL_tPRINT = 62,                    /* tPRINT  */
  YYSYMBOL_tFPRINTF = 63,                  /* tFPRINTF  */
  YYSYMBOL_tSTOP = 64,                     /* tSTOP  */
  YYSYMBOL_tFOPEN = 65,                    /* tFOPEN  */
  YYSYMBOL_tFCLOSE = 66,                   /* tFCLOSE  */
  YYSYMBOL_tFFLUSH = 67,                   /* tFFLUSH  */
  YYSYMBOL_tFSCANF = 68,                   /* tFSCANF  */
  YYSYMBOL_tFUNC = 69,                     /* tFUNC  */
  YYSYMBOL_tPLAY = 70,                     /* tPLAY  */
  YYSYMBOL_tSETDERIVLENGTH = 71,           /* tSETDERIVLENGTH  */
  YYSYMBOL_tGETDERIVLENGTH = 72,           /* tGETDERIVLENGTH  */
  YYSYMBOL_tDISPLAY = 73,                  /* tDISPLAY  */
  YYSYMBOL_tVVXMIN = 74,                   /* tVVXMIN  */
  YYSYMBOL_tVVXMAX = 75,                   /* tVVXMAX  */
  YYSYMBOL_tVVYMIN = 76,                   /* tVVYMIN  */
  YYSYMBOL_tVVYMAX = 77,                   /* tVVYMAX  */
  YYSYMBOL_tVVZMIN = 78,                   /* tVVZMIN  */
  YYSYMBOL_tVVZMAX = 79,                   /* tVVZMAX  */
  YYSYMBOL_tVVSCALE = 80,                  /* tVVSCALE  */
  YYSYMBOL_tCURVEX = 81,                   /* tCURVEX  */
  YYSYMBOL_tCURVEY = 82,                   /* tCURVEY  */
  YYSYMBOL_tCURVEZ = 83,                   /* tCURVEZ  */
  YYSYMBOL_tCURVEGAL = 84,                 /* tCURVEGAL  */
  YYSYMBOL_tARRAYREF = 85,                 /* tARRAYREF  */
  YYSYMBOL_tARRAYLHS = 86,                 /* tARRAYLHS  */
  YYSYMBOL_tARRAYDEF = 87,                 /* tARRAYDEF  */
  YYSYMBOL_tARRAYLVAL = 88,                /* tARRAYLVAL  */
  YYSYMBOL_tSYMBOL = 89,                   /* tSYMBOL  */
  YYSYMBOL_tNAME = 90,                     /* tNAME  */
  YYSYMBOL_tNAMELVAL = 91,                 /* tNAMELVAL  */
  YYSYMBOL_tNAMELHS = 92,                  /* tNAMELHS  */
  YYSYMBOL_tSTRING = 93,                   /* tSTRING  */
  YYSYMBOL_tVALUE = 94,                    /* tVALUE  */
  YYSYMBOL_tINTEGER = 95,                  /* tINTEGER  */
  YYSYMBOL_tASSIGN = 96,                   /* tASSIGN  */
  YYSYMBOL_tQUESTION = 97,                 /* tQUESTION  */
  YYSYMBOL_tCOLON = 98,                    /* tCOLON  */
  YYSYMBOL_tOR = 99,                       /* tOR  */
  YYSYMBOL_tAND = 100,                     /* tAND  */
  YYSYMBOL_tEQUAL = 101,                   /* tEQUAL  */
  YYSYMBOL_tNOTEQUAL = 102,                /* tNOTEQUAL  */
  YYSYMBOL_tLT = 103,                      /* tLT  */
  YYSYMBOL_tLE = 104,                      /* tLE  */
  YYSYMBOL_tGE = 105,                      /* tGE  */
  YYSYMBOL_tGT = 106,                      /* tGT  */
  YYSYMBOL_tPLUS = 107,                    /* tPLUS  */
  YYSYMBOL_tMINUS = 108,                   /* tMINUS  */
  YYSYMBOL_tTIMES = 109,                   /* tTIMES  */
  YYSYMBOL_tDIVIDE = 110,                  /* tDIVIDE  */
  YYSYMBOL_tREM = 111,                     /* tREM  */
  YYSYMBOL_tPOW = 112,                     /* tPOW  */
  YYSYMBOL_tUMINUS = 113,                  /* tUMINUS  */
  YYSYMBOL_tADDRESS = 114,                 /* tADDRESS  */
  YYSYMBOL_tNOT = 115,                     /* tNOT  */
  YYSYMBOL_YYACCEPT = 116,                 /* $accept  */
  YYSYMBOL_Lfile = 117,                    /* Lfile  */
  YYSYMBOL_118_1 = 118,                    /* $@1  */
  YYSYMBOL_Lsystems = 119,                 /* Lsystems  */
  YYSYMBOL_Lsystem = 120,                  /* Lsystem  */
  YYSYMBOL_Homomorphism = 121,             /* Homomorphism  */
  YYSYMBOL_122_2 = 122,                    /* $@2  */
  YYSYMBOL_HomoItems = 123,                /* HomoItems  */
  YYSYMBOL_HomoItem = 124,                 /* HomoItem  */
  YYSYMBOL_HomoWarning = 125,              /* HomoWarning  */
  YYSYMBOL_HomoSeed = 126,                 /* HomoSeed  */
  YYSYMBOL_Decomposition = 127,            /* Decomposition  */
  YYSYMBOL_128_3 = 128,                    /* $@3  */
  YYSYMBOL_DecompWarning = 129,            /* DecompWarning  */
  YYSYMBOL_DecompItems = 130,              /* DecompItems  */
  YYSYMBOL_ProdDepth = 131,                /* ProdDepth  */
  YYSYMBOL_Header = 132,                   /* Header  */
  YYSYMBOL_BlankLines = 133,               /* BlankLines  */
  YYSYMBOL_BlankLine = 134,                /* BlankLine  */
  YYSYMBOL_Label = 135,                    /* Label  */
  YYSYMBOL_136_4 = 136,                    /* $@4  */
  YYSYMBOL_Items = 137,                    /* Items  */
  YYSYMBOL_Item = 138,                     /* Item  */
  YYSYMBOL_Ring = 139,                     /* Ring  */
  YYSYMBOL_Seed = 140,                     /* Seed  */
  YYSYMBOL_Dlength = 141,                  /* Dlength  */
  YYSYMBOL_142_5 = 142,                    /* $@5  */
  YYSYMBOL_Consider = 143,                 /* Consider  */
  YYSYMBOL_144_6 = 144,                    /* $@6  */
  YYSYMBOL_Ignore = 145,                   /* Ignore  */
  YYSYMBOL_146_7 = 146,                    /* $@7  */
  YYSYMBOL_Characters = 147,               /* Characters  */
  YYSYMBOL_Statements = 148,               /* Statements  */
  YYSYMBOL_Statement = 149,                /* Statement  */
  YYSYMBOL_ArrayDim = 150,                 /* ArrayDim  */
  YYSYMBOL_ArrayDims = 151,                /* ArrayDims  */
  YYSYMBOL_ArrayDef = 152,                 /* ArrayDef  */
  YYSYMBOL_ArrayDefs = 153,                /* ArrayDefs  */
  YYSYMBOL_ArrayDefStatement = 154,        /* ArrayDefStatement  */
  YYSYMBOL_ExternalDef = 155,              /* ExternalDef  */
  YYSYMBOL_ExternalDefs = 156,             /* ExternalDefs  */
  YYSYMBOL_ExternalDefStatement = 157,     /* ExternalDefStatement  */
  YYSYMBOL_DefStatement = 158,             /* DefStatement  */
  YYSYMBOL_DefStatements = 159,            /* DefStatements  */
  YYSYMBOL_Assignment = 160,               /* Assignment  */
  YYSYMBOL_ArrayRef = 161,                 /* ArrayRef  */
  YYSYMBOL_ArrayRefs = 162,                /* ArrayRefs  */
  YYSYMBOL_LValue = 163,                   /* LValue  */
  YYSYMBOL_LHS = 164,                      /* LHS  */
  YYSYMBOL_Procedure = 165,                /* Procedure  */
  YYSYMBOL_IfStatement = 166,              /* IfStatement  */
  YYSYMBOL_WhileStatement = 167,           /* WhileStatement  */
  YYSYMBOL_DoStatement = 168,              /* DoStatement  */
  YYSYMBOL_DefineBlock = 169,              /* DefineBlock  */
  YYSYMBOL_Defines = 170,                  /* Defines  */
  YYSYMBOL_ArrayInitBlock = 171,           /* ArrayInitBlock  */
  YYSYMBOL_Block = 172,                    /* Block  */
  YYSYMBOL_LBRACE = 173,                   /* LBRACE  */
  YYSYMBOL_RBRACE = 174,                   /* RBRACE  */
  YYSYMBOL_Startblock = 175,               /* Startblock  */
  YYSYMBOL_Endblock = 176,                 /* Endblock  */
  YYSYMBOL_Starteach = 177,                /* Starteach  */
  YYSYMBOL_Endeach = 178,                  /* Endeach  */
  YYSYMBOL_Axiom = 179,                    /* Axiom  */
  YYSYMBOL_Productions = 180,              /* Productions  */
  YYSYMBOL_Production = 181,               /* Production  */
  YYSYMBOL_182_8 = 182,                    /* $@8  */
  YYSYMBOL_Predecessor = 183,              /* Predecessor  */
  YYSYMBOL_Lcontext = 184,                 /* Lcontext  */
  YYSYMBOL_Strictpred = 185,               /* Strictpred  */
  YYSYMBOL_Rcontext = 186,                 /* Rcontext  */
  YYSYMBOL_Conditional = 187,              /* Conditional  */
  YYSYMBOL_Precondition = 188,             /* Precondition  */
  YYSYMBOL_Postcondition = 189,            /* Postcondition  */
  YYSYMBOL_Condition = 190,                /* Condition  */
  YYSYMBOL_Successor = 191,                /* Successor  */
  YYSYMBOL_StrictSucc = 192,               /* StrictSucc  */
  YYSYMBOL_Probability = 193,              /* Probability  */
  YYSYMBOL_FormalModules = 194,            /* FormalModules  */
  YYSYMBOL_FormalModule = 195,             /* FormalModule  */
  YYSYMBOL_Modules = 196,                  /* Modules  */
  YYSYMBOL_Module = 197,                   /* Module  */
  YYSYMBOL_Symbol = 198,                   /* Symbol  */
  YYSYMBOL_FormalParameters = 199,         /* FormalParameters  */
  YYSYMBOL_FormalParameter = 200,          /* FormalParameter  */
  YYSYMBOL_Parameters = 201,               /* Parameters  */
  YYSYMBOL_Expression = 202,               /* Expression  */
  YYSYMBOL_Function = 203,                 /* Function  */
  YYSYMBOL_FunctionName = 204,             /* FunctionName  */
  YYSYMBOL_Value = 205,                    /* Value  */
  YYSYMBOL_String = 206,                   /* String  */
  YYSYMBOL_Name = 207                      /* Name  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1117

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  116
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  92
/* YYNRULES -- Number of rules.  */
#define YYNRULES  224
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  361

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   370


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   236,   236,   236,   250,   251,   254,   278,   276,   288,
     291,   292,   295,   296,   299,   303,   307,   311,   324,   322,
     333,   336,   340,   344,   348,   349,   352,   366,   369,   370,
     373,   376,   376,   469,   470,   473,   474,   475,   476,   477,
     478,   479,   480,   481,   482,   483,   484,   493,   500,   508,
     524,   523,   529,   529,   546,   546,   562,   568,   576,   580,
     586,   590,   594,   598,   602,   606,   613,   619,   623,   629,
     661,   697,   701,   707,   717,   733,   751,   778,   781,   786,
     791,   795,   799,   806,   810,   816,   833,   839,   843,   849,
     872,   887,   910,   927,   936,   945,   956,   966,   977,   983,
     993,   999,  1005,  1011,  1020,  1030,  1040,  1050,  1060,  1081,
    1096,  1097,  1100,  1101,  1114,  1129,  1128,  1144,  1145,  1146,
    1147,  1151,  1156,  1197,  1254,  1259,  1289,  1291,  1293,  1295,
    1298,  1311,  1322,  1333,  1345,  1357,  1371,  1381,  1388,  1430,
    1441,  1445,  1450,  1455,  1463,  1467,  1472,  1477,  1485,  1498,
    1502,  1507,  1517,  1521,  1527,  1529,  1531,  1533,  1535,  1537,
    1539,  1541,  1543,  1545,  1547,  1549,  1551,  1553,  1555,  1557,
    1559,  1561,  1563,  1565,  1567,  1569,  1571,  1574,  1580,  1580,
    1580,  1580,  1580,  1580,  1581,  1581,  1581,  1581,  1581,  1581,
    1581,  1581,  1581,  1581,  1581,  1582,  1582,  1582,  1583,  1583,
    1583,  1583,  1583,  1583,  1583,  1584,  1584,  1584,  1584,  1585,
    1586,  1586,  1586,  1586,  1586,  1586,  1586,  1587,  1587,  1587,
    1587,  1590,  1609,  1635,  1664
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "tLSTART", "tRING",
  "tSEED", "tDLENGTH", "tMDEPTH", "tCONSIDER", "tIGNORE", "tAXIOM", "tEND",
  "tDEFINE", "tSTARTBLOCK", "tENDBLOCK", "tSTARTEACH", "tENDEACH",
  "tSTARTHOMO", "tSTARTDECOMP", "tNOWARNING", "tWARNING", "tLSEP", "tRSEP",
  "tYIELDS", "tOYIELDS", "tNULL", "tEOL", "tIF", "tDO", "tWHILE", "tELSE",
  "tARRAY", "tEXTERNAL", "tLPAREN", "tRPAREN", "tLBRACE", "tRBRACE",
  "tCOMMA", "tLBRACKET", "tRBRACKET", "tSEMI", "tATAN2", "tTAN", "tSIN",
  "tCOS", "tATAN", "tASIN", "tACOS", "tSRAND", "tRAN", "tNRAN", "tBRAN",
  "tBIRAN", "tEXP", "tLOG", "tFLOOR", "tCEIL", "tSIGN", "tSQRT", "tTRUNC",
  "tFABS", "tINBLOB", "tPRINT", "tFPRINTF", "tSTOP", "tFOPEN", "tFCLOSE",
  "tFFLUSH", "tFSCANF", "tFUNC", "tPLAY", "tSETDERIVLENGTH",
  "tGETDERIVLENGTH", "tDISPLAY", "tVVXMIN", "tVVXMAX", "tVVYMIN",
  "tVVYMAX", "tVVZMIN", "tVVZMAX", "tVVSCALE", "tCURVEX", "tCURVEY",
  "tCURVEZ", "tCURVEGAL", "tARRAYREF", "tARRAYLHS", "tARRAYDEF",
  "tARRAYLVAL", "tSYMBOL", "tNAME", "tNAMELVAL", "tNAMELHS", "tSTRING",
  "tVALUE", "tINTEGER", "tASSIGN", "tQUESTION", "tCOLON", "tOR", "tAND",
  "tEQUAL", "tNOTEQUAL", "tLT", "tLE", "tGE", "tGT", "tPLUS", "tMINUS",
  "tTIMES", "tDIVIDE", "tREM", "tPOW", "tUMINUS", "tADDRESS", "tNOT",
  "$accept", "Lfile", "$@1", "Lsystems", "Lsystem", "Homomorphism", "$@2",
  "HomoItems", "HomoItem", "HomoWarning", "HomoSeed", "Decomposition",
  "$@3", "DecompWarning", "DecompItems", "ProdDepth", "Header",
  "BlankLines", "BlankLine", "Label", "$@4", "Items", "Item", "Ring",
  "Seed", "Dlength", "$@5", "Consider", "$@6", "Ignore", "$@7",
  "Characters", "Statements", "Statement", "ArrayDim", "ArrayDims",
  "ArrayDef", "ArrayDefs", "ArrayDefStatement", "ExternalDef",
  "ExternalDefs", "ExternalDefStatement", "DefStatement", "DefStatements",
  "Assignment", "ArrayRef", "ArrayRefs", "LValue", "LHS", "Procedure",
  "IfStatement", "WhileStatement", "DoStatement", "DefineBlock", "Defines",
  "ArrayInitBlock", "Block", "LBRACE", "RBRACE", "Startblock", "Endblock",
  "Starteach", "Endeach", "Axiom", "Productions", "Production", "$@8",
  "Predecessor", "Lcontext", "Strictpred", "Rcontext", "Conditional",
  "Precondition", "Postcondition", "Condition", "Successor", "StrictSucc",
  "Probability", "FormalModules", "FormalModule", "Modules", "Module",
  "Symbol", "FormalParameters", "FormalParameter", "Parameters",
  "Expression", "Function", "FunctionName", "Value", "String", "Name", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-279)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-123)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -279,    73,  -279,  -279,  -279,  -279,    24,    37,  -279,  -279,
    -279,  -279,  -279,    17,  -279,   -23,   110,    55,     0,  -279,
      46,  -279,  -279,  -279,   129,    36,   166,  -279,   684,    60,
      95,    -8,  -279,   128,   140,   468,  -279,  -279,  -279,   204,
     106,   220,   832,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,   195,  -279,  -279,
     832,  -279,   146,   832,  -279,  -279,  -279,   758,   202,   564,
    -279,   205,  -279,  -279,  -279,     2,     2,   219,    95,  -279,
    -279,    95,  -279,    31,  -279,  -279,     7,   217,   218,   832,
     455,  -279,  -279,    18,   202,   202,   202,   202,   202,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,   222,  -279,   167,   832,  -279,
     195,  -279,   195,  -279,   287,   202,  -279,  -279,   832,   832,
     832,   832,   832,   832,   832,   832,   832,   832,   832,   832,
     832,   832,   832,   832,  -279,   227,   147,    95,  -279,   213,
     228,    -8,  -279,   128,  -279,  -279,  -279,  -279,    59,  -279,
      88,   140,   140,   232,    21,   233,  -279,   234,   235,   236,
     254,   275,  -279,  -279,   931,  -279,   195,   257,   251,  -279,
     256,   -16,   371,  -279,  -279,   194,  -279,  -279,  -279,  -279,
     989,  -279,   548,   282,   101,    69,    69,    40,    40,    40,
      40,   111,   111,   179,   179,   179,  -279,    58,   564,  -279,
     832,  -279,  -279,   832,  -279,  -279,  -279,  -279,   266,  -279,
      22,    26,  -279,  -279,  -279,    56,  -279,  -279,  -279,  -279,
     832,    24,  -279,  -279,  -279,  -279,   832,    27,   832,    13,
    -279,  -279,  -279,   832,  -279,   832,  -279,   832,   564,    84,
    -279,  -279,  -279,   267,   206,   207,  -279,  -279,  -279,    38,
     588,    12,    19,   883,   269,   899,  1005,   273,   564,   564,
    -279,  -279,   262,  -279,    83,   262,  -279,    90,  -279,  -279,
    -279,   832,  -279,  -279,  -279,    35,  -279,   274,  -279,  -279,
     832,  -279,    16,   206,  -279,    20,   207,  -279,   604,    27,
     832,    27,   273,   958,   202,  -279,  -279,   273,   202,  -279,
     273,  -279,   265,   915,  -279,  -279,  -279,  -279,  -279,  -279,
     273,   268,   273,   529,    27,  -279,   117,  -279,   273,  -279,
     273
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     5,     1,    29,     4,     0,     3,   115,   121,
      30,   148,   112,     0,   111,   130,     0,   117,   123,   140,
     142,    31,    28,    34,     0,    23,     9,   110,     0,     0,
       0,     0,   141,     0,     0,     0,   116,    22,    21,     0,
      16,     0,     0,   102,   178,   179,   180,   181,   182,   183,
     188,   184,   185,   186,   187,   189,   190,   191,   192,   195,
     198,   193,   194,   196,   199,   200,   197,   201,   202,   203,
     204,   205,   206,   207,   208,   209,   210,   211,   212,   213,
     214,   215,   216,   217,   218,   219,   220,   223,   222,   221,
       0,   133,     0,     0,   172,   131,    29,     0,   126,   134,
     173,     0,   175,   176,   174,     0,     0,   118,   123,   124,
     119,   125,   151,     0,   149,    56,     0,     0,     0,     0,
       0,    52,    54,     0,     0,     0,     0,     0,     0,    40,
      33,    35,    36,    37,    39,    38,    41,    42,    43,    44,
      45,    27,    18,    15,    14,     0,     6,     0,     0,    88,
     224,   168,    89,   169,     0,   127,   132,   128,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   137,     0,   135,   138,   144,   146,
       0,     0,   143,     0,    32,    57,    46,    47,     0,    50,
       0,     0,     0,     0,     0,     0,    29,     0,     0,     0,
       0,    25,     7,   171,     0,    87,    90,     0,     0,    29,
       0,   223,     0,    59,    60,     0,    61,    62,    63,    64,
       0,   129,     0,   154,   155,   156,   157,   158,   159,   161,
     160,   162,   163,   164,   165,   166,   167,     0,   152,   113,
       0,   136,   145,     0,   114,   120,   150,    48,     0,    49,
       0,     0,   109,   108,    99,     0,   104,   105,   106,   107,
       0,     0,    24,    11,    86,    65,     0,     0,     0,   224,
     103,    58,   101,     0,    29,     0,   177,     0,   139,     0,
      51,    53,    55,     0,     0,     0,    80,    81,    84,     0,
       0,     0,     0,     0,     0,     0,     0,    93,   170,   153,
     147,    82,     0,    72,     0,    74,    78,     0,    83,    98,
      26,     0,    10,    12,    13,     0,    29,     0,    29,    29,
       0,    68,    69,     0,    29,    75,     0,    29,     0,     0,
       0,     0,    85,     0,     0,    67,    71,    73,     0,    77,
      79,    17,    29,     0,    29,    66,    70,    29,    76,    29,
      94,     0,    96,     0,     0,    29,     0,    29,    97,   100,
      95
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,    10,  -279,   -92,    -5,  -279,
    -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,  -279,
    -279,    25,  -279,    94,   -97,     4,   -12,  -279,  -279,   -14,
    -279,  -279,    28,  -279,  -279,  -140,  -121,  -279,  -279,  -279,
    -279,  -279,  -279,  -279,  -279,   -20,   -28,  -119,  -278,  -279,
    -279,  -279,  -279,  -279,  -220,   -10,  -279,  -279,  -279,   289,
     141,  -279,  -279,   168,   224,   221,  -279,  -279,   -24,    -6,
     201,  -118,   -91,  -279,   142,  -234,   -26,  -279,  -279,  -279,
    -279,  -279
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,     4,     5,    41,   263,   292,   312,   145,
     313,    26,   201,    39,   261,   262,     6,     7,    22,    23,
      34,    35,   130,   131,   132,   133,   248,   134,   191,   135,
     192,   116,   212,   213,   321,   322,   303,   304,   286,   306,
     307,   287,   288,   289,   214,   149,   150,    94,   215,   216,
     217,   218,   219,   195,   136,   346,   156,    96,   272,   137,
     138,   139,   140,   141,    13,    14,    24,    15,    16,    17,
     110,    29,    97,   157,    98,   175,   176,   241,    18,    19,
     177,   178,    20,   113,   114,   237,   238,   100,   101,   102,
     103,   104
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      95,    12,    99,    27,   154,   196,   108,   111,    12,   279,
     205,   309,    32,     8,   179,   179,   147,   109,     8,   193,
       8,  -122,   148,   -19,   311,     8,   260,   174,   -20,   -19,
     129,   206,   179,   184,   -20,    25,     8,     9,    10,   283,
      21,   291,     9,    10,     9,    10,    -8,   253,   281,     9,
      10,   148,   282,    10,   320,    37,    38,   283,   320,   242,
       9,    10,    43,    10,   151,   182,   205,   153,   183,   284,
     285,    99,   315,     3,   270,    28,   242,    31,   359,    33,
     -91,    11,    10,   105,   106,   247,   179,   284,   285,    11,
     269,    11,   276,   188,   190,   277,   185,   197,   198,   199,
     200,    11,    32,   179,   255,    32,    11,    11,    11,   -92,
      11,   185,   334,    11,   249,   185,   338,   267,   300,   356,
     323,   277,   204,   324,    11,   143,   144,   326,   220,   205,
     327,    30,   222,   223,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,   236,   167,   168,   169,
     170,   171,   172,   270,   277,    36,   158,   111,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   297,    40,    11,   158,   220,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   203,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,   278,   347,   250,   251,   112,   347,
     169,   170,   171,   172,   329,   335,   331,   332,   335,   115,
     142,   146,   337,   148,   290,   340,   152,    43,   173,   294,
     293,   181,   295,   186,   187,   240,   243,   296,   202,   298,
     350,   299,   352,   239,   244,   353,    12,   354,   252,   254,
     256,   257,   258,   358,   158,   360,   159,   160,   161,   162,
     163,   164,   165,   166,   167,   168,   169,   170,   171,   172,
     259,    27,   260,   265,   266,   328,    12,    12,   207,   268,
     273,   172,   280,   301,   333,   349,   302,   305,   317,    10,
     320,   342,   314,   344,   343,    27,   271,   330,   355,   325,
      12,   336,   339,    10,   208,   209,   210,   308,   348,   107,
      42,   155,   245,   221,   194,   246,   357,   180,     0,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,   207,     0,     0,     0,     0,   211,     0,     0,
      88,    89,   160,   161,   162,   163,   164,   165,   166,   167,
     168,   169,   170,   171,   172,    90,     0,     0,   208,   209,
     210,    92,    93,     0,    42,     0,     0,   270,     0,     0,
       0,     0,     0,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    85,    86,   189,     0,     0,     0,
       0,   211,     0,     0,    88,    89,     0,     0,     0,   117,
       0,     0,   118,   119,   120,     0,   121,   122,   123,    90,
     124,   125,   126,   127,   128,    92,    93,     0,    42,     0,
       0,     0,     0,     0,    10,     0,     0,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    66,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    84,    85,    86,
       0,     0,     0,     0,     0,    87,     0,     0,    88,    89,
       0,     0,     0,     0,     0,    10,     0,     0,     0,     0,
       0,     0,    42,    90,     0,     0,     0,     0,     0,    92,
      93,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,    85,    86,   310,     0,     0,     0,     0,    87,
       0,     0,    88,    89,     0,     0,     0,     0,     0,     0,
     341,     0,     0,     0,     0,     0,     0,    90,     0,     0,
       0,     0,     0,    92,    93,   158,   275,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   158,     0,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,     0,     0,     0,
       0,     0,     0,     0,     0,   158,     0,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,   158,     0,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,    42,     0,    43,
       0,     0,     0,     0,     0,     0,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    85,    86,     0,
       0,     0,     0,     0,    87,     0,     0,    88,    89,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    42,    90,    91,     0,     0,     0,     0,    92,    93,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      84,    85,    86,     0,     0,     0,     0,     0,    87,     0,
       0,    88,    89,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    42,    90,    91,     0,     0,
       0,     0,    92,    93,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    84,    85,    86,   316,     0,     0,
       0,     0,    87,     0,     0,    88,    89,     0,     0,     0,
       0,     0,     0,   318,     0,     0,     0,     0,     0,     0,
      90,     0,     0,     0,     0,     0,    92,    93,     0,   351,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     264,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     158,     0,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   158,   345,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   158,     0,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   158,   274,
     159,   160,   161,   162,   163,   164,   165,   166,   167,   168,
     169,   170,   171,   172,     0,   319,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   158,     0,   159,   160,   161,
     162,   163,   164,   165,   166,   167,   168,   169,   170,   171,
     172,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   158,     0,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   168,   169,   170,
     171,   172,   158,     0,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172
};

static const yytype_int16 yycheck[] =
{
      28,     6,    28,    13,    96,   124,    30,    31,    13,   243,
     150,   289,    18,     1,   105,   106,    42,    25,     1,     1,
       1,    21,    38,    11,     5,     1,     7,    25,    11,    17,
      35,   152,   123,    26,    17,    18,     1,    25,    26,     1,
       3,   261,    25,    26,    25,    26,    11,    26,    26,    25,
      26,    38,    26,    26,    38,    19,    20,     1,    38,   177,
      25,    26,    35,    26,    90,    34,   206,    93,    37,    31,
      32,    97,   292,     0,    36,    98,   194,    22,   356,    33,
      96,    89,    26,    23,    24,    26,   177,    31,    32,    89,
     211,    89,    34,   119,   120,    37,    89,   125,   126,   127,
     128,    89,   108,   194,   196,   111,    89,    89,    89,    96,
      89,    89,    96,    89,    26,    89,    96,   209,    34,   353,
      37,    37,   148,    40,    89,    19,    20,    37,   154,   269,
      40,    21,   158,   159,   160,   161,   162,   163,   164,   165,
     166,   167,   168,   169,   170,   171,   172,   107,   108,   109,
     110,   111,   112,    36,    37,    26,    97,   181,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,   274,    17,    89,    97,   212,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    34,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   240,   334,   191,   192,    90,   338,
     109,   110,   111,   112,   316,   322,   318,   319,   325,    89,
      26,    11,   324,    38,   260,   327,    90,    35,    33,   267,
     266,    22,   268,    26,    26,    98,    33,   273,    26,   275,
     342,   277,   344,    26,    26,   347,   261,   349,    26,    26,
      26,    26,    26,   355,    97,   357,    99,   100,   101,   102,
     103,   104,   105,   106,   107,   108,   109,   110,   111,   112,
      26,   291,     7,    26,    33,   311,   291,   292,     1,    33,
      96,   112,    26,    26,   320,    30,    90,    90,    29,    26,
      38,   329,   292,   331,   330,   315,   212,    33,    40,   305,
     315,   323,   326,    26,    27,    28,    29,   289,   338,    30,
      33,    97,   181,   155,   123,   183,   354,   106,    -1,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,     1,    -1,    -1,    -1,    -1,    90,    -1,    -1,
      93,    94,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   108,    -1,    -1,    27,    28,
      29,   114,   115,    -1,    33,    -1,    -1,    36,    -1,    -1,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,     1,    -1,    -1,    -1,
      -1,    90,    -1,    -1,    93,    94,    -1,    -1,    -1,     1,
      -1,    -1,     4,     5,     6,    -1,     8,     9,    10,   108,
      12,    13,    14,    15,    16,   114,   115,    -1,    33,    -1,
      -1,    -1,    -1,    -1,    26,    -1,    -1,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      -1,    -1,    -1,    -1,    -1,    90,    -1,    -1,    93,    94,
      -1,    -1,    -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,
      -1,    -1,    33,   108,    -1,    -1,    -1,    -1,    -1,   114,
     115,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    60,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,    26,    -1,    -1,    -1,    -1,    90,
      -1,    -1,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,
      26,    -1,    -1,    -1,    -1,    -1,    -1,   108,    -1,    -1,
      -1,    -1,    -1,   114,   115,    97,    98,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    97,    -1,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    97,    -1,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    97,    -1,    99,   100,   101,   102,   103,   104,   105,
     106,   107,   108,   109,   110,   111,   112,    33,    -1,    35,
      -1,    -1,    -1,    -1,    -1,    -1,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    -1,
      -1,    -1,    -1,    -1,    90,    -1,    -1,    93,    94,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    33,   108,   109,    -1,    -1,    -1,    -1,   114,   115,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,    -1,    -1,    -1,    -1,    -1,    90,    -1,
      -1,    93,    94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    33,   108,   109,    -1,    -1,
      -1,    -1,   114,   115,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    34,    -1,    -1,
      -1,    -1,    90,    -1,    -1,    93,    94,    -1,    -1,    -1,
      -1,    -1,    -1,    34,    -1,    -1,    -1,    -1,    -1,    -1,
     108,    -1,    -1,    -1,    -1,    -1,   114,   115,    -1,    34,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      97,    -1,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,    97,    39,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    97,    -1,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,    97,    40,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,    -1,    40,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    97,    -1,    99,   100,   101,
     102,   103,   104,   105,   106,   107,   108,   109,   110,   111,
     112,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    97,    -1,    99,   100,
     101,   102,   103,   104,   105,   106,   107,   108,   109,   110,
     111,   112,    97,    -1,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   117,   118,     0,   119,   120,   132,   133,     1,    25,
      26,    89,   134,   180,   181,   183,   184,   185,   194,   195,
     198,     3,   134,   135,   182,    18,   127,   181,    98,   187,
      21,    22,   195,    33,   136,   137,    26,    19,    20,   129,
      17,   121,    33,    35,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,    90,    93,    94,
     108,   109,   114,   115,   163,   172,   173,   188,   190,   202,
     203,   204,   205,   206,   207,    23,    24,   185,   194,    25,
     186,   194,    90,   199,   200,    89,   147,     1,     4,     5,
       6,     8,     9,    10,    12,    13,    14,    15,    16,   134,
     138,   139,   140,   141,   143,   145,   170,   175,   176,   177,
     178,   179,    26,    19,    20,   125,    11,   202,    38,   161,
     162,   202,    90,   202,   133,   190,   172,   189,    97,    99,
     100,   101,   102,   103,   104,   105,   106,   107,   108,   109,
     110,   111,   112,    33,    25,   191,   192,   196,   197,   198,
     191,    22,    34,    37,    26,    89,    26,    26,   202,     1,
     202,   144,   146,     1,   196,   169,   173,   172,   172,   172,
     172,   128,    26,    34,   202,   161,   162,     1,    27,    28,
      29,    90,   148,   149,   160,   164,   165,   166,   167,   168,
     202,   189,   202,   202,   202,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   202,   201,   202,    26,
      98,   193,   197,    33,    26,   186,   200,    26,   142,    26,
     147,   147,    26,    26,    26,   133,    26,    26,    26,    26,
       7,   130,   131,   122,    39,    26,    33,   133,    33,   162,
      36,   149,   174,    96,    40,    98,    34,    37,   202,   201,
      26,    26,    26,     1,    31,    32,   154,   157,   158,   159,
     202,   180,   123,   202,   172,   202,   202,   133,   202,   202,
      34,    26,    90,   152,   153,    90,   155,   156,   158,   174,
      26,     5,   124,   126,   131,   180,    34,    29,    34,    40,
      38,   150,   151,    37,    40,   151,    37,    40,   202,   133,
      33,   133,   133,   202,    96,   150,   152,   133,    96,   155,
     133,    26,   172,   202,   172,    39,   171,   173,   171,    30,
     133,    34,   133,   133,   133,    40,   201,   172,   133,   174,
     133
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,   116,   118,   117,   119,   119,   120,   122,   121,   121,
     123,   123,   124,   124,   125,   125,   125,   126,   128,   127,
     127,   129,   129,   129,   130,   130,   131,   132,   133,   133,
     134,   136,   135,   137,   137,   138,   138,   138,   138,   138,
     138,   138,   138,   138,   138,   138,   138,   139,   140,   141,
     142,   141,   144,   143,   146,   145,   147,   147,   148,   148,
     149,   149,   149,   149,   149,   149,   150,   151,   151,   152,
     152,   153,   153,   154,   155,   155,   155,   156,   156,   157,
     158,   158,   158,   159,   159,   160,   161,   162,   162,   163,
     163,   164,   164,   165,   166,   166,   167,   168,   169,   170,
     171,   172,   173,   174,   175,   176,   177,   178,   179,   179,
     180,   180,   181,   181,   181,   182,   181,   183,   183,   183,
     183,   184,   184,   185,   186,   186,   187,   187,   187,   187,
     187,   188,   189,   190,   190,   191,   191,   192,   192,   193,
     194,   194,   195,   195,   196,   196,   197,   197,   198,   199,
     199,   200,   201,   201,   202,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   202,   202,   202,   202,
     202,   202,   202,   202,   202,   202,   202,   203,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   204,   204,   204,   204,   204,   204,   204,   204,   204,
     204,   205,   206,   207,   207
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     3,     2,     0,     5,     0,     6,     0,
       2,     0,     1,     1,     1,     1,     0,     3,     0,     6,
       0,     1,     1,     0,     1,     0,     3,     4,     2,     0,
       1,     0,     4,     2,     0,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     3,     3,
       0,     4,     0,     4,     0,     4,     1,     2,     2,     1,
       1,     1,     1,     1,     1,     2,     3,     2,     1,     2,
       4,     3,     1,     4,     1,     2,     4,     3,     1,     4,
       1,     1,     2,     2,     1,     5,     3,     2,     1,     2,
       3,     1,     2,     3,     7,    10,     7,     9,     4,     3,
       4,     4,     1,     1,     3,     3,     3,     3,     3,     3,
       2,     1,     1,     5,     5,     0,     3,     1,     3,     3,
       5,     1,     1,     1,     1,     1,     2,     3,     3,     4,
       0,     1,     1,     1,     1,     1,     2,     1,     1,     2,
       1,     2,     1,     4,     1,     2,     1,     4,     1,     1,
       3,     1,     1,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     2,     2,
       5,     3,     1,     1,     1,     1,     1,     4,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* $@1: %empty  */
#line 236 "lsys_input.y"
        {
          /* reset for reread */
          lsystemCount = 0;
          c_expression = 0;
          clp.checkEnvironment = 0;
          isObjectProduction = 0;
	  first_warning = 1;
	}
#line 1904 "lsys_input.tab.c"
    break;

  case 3: /* Lfile: $@1 Lsystems BlankLines  */
#line 245 "lsys_input.y"
        {
	  /* return(0); */
	}
#line 1912 "lsys_input.tab.c"
    break;

  case 6: /* Lsystem: Header Productions Decomposition Homomorphism tEND  */
#line 255 "lsys_input.y"
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
#line 1936 "lsys_input.tab.c"
    break;

  case 7: /* $@2: %empty  */
#line 278 "lsys_input.y"
        {
		LsysPtr->Homomorphism.specified = 1;
		LsysPtr->Homomorphism.longest_succ = 1;
		LsysPtr->Homomorphism.stack = NULL;
		LsysPtr->Homomorphism.stack_len = 0;
		LsysPtr->Homomorphism.depth = 1;
		LsysPtr->Homomorphism.seed = -1;   /* no separate seed */
		type_of_productions = HOMOMORPHISM;
        }
#line 1950 "lsys_input.tab.c"
    break;

  case 14: /* HomoWarning: tWARNING  */
#line 300 "lsys_input.y"
        {
	  LsysPtr->homo_warning = 1;
	}
#line 1958 "lsys_input.tab.c"
    break;

  case 15: /* HomoWarning: tNOWARNING  */
#line 304 "lsys_input.y"
        {
	  LsysPtr->homo_warning = 0;
	}
#line 1966 "lsys_input.tab.c"
    break;

  case 17: /* HomoSeed: tSEED Expression tEOL  */
#line 312 "lsys_input.y"
        {
	  LsysPtr->Homomorphism.seed = (int) Eval((yyvsp[-1].expression));
	  printf("Homomorphism seed %d\n", 
		    LsysPtr->Homomorphism.seed);

	  VERBOSE( "Homomorphism seed %d\n", 
		    LsysPtr->Homomorphism.seed);
	}
#line 1979 "lsys_input.tab.c"
    break;

  case 18: /* $@3: %empty  */
#line 324 "lsys_input.y"
        {
		LsysPtr->Decomposition.specified = 1;
		LsysPtr->Decomposition.longest_succ = 1;
		LsysPtr->Decomposition.stack = NULL;
		LsysPtr->Decomposition.stack_len = 0;
		LsysPtr->Decomposition.depth = 1;
		type_of_productions = DECOMPOSITION;
        }
#line 1992 "lsys_input.tab.c"
    break;

  case 21: /* DecompWarning: tWARNING  */
#line 337 "lsys_input.y"
        {
	  LsysPtr->decomp_warning = 1;
	}
#line 2000 "lsys_input.tab.c"
    break;

  case 22: /* DecompWarning: tNOWARNING  */
#line 341 "lsys_input.y"
        {
	  LsysPtr->decomp_warning = 0;
	}
#line 2008 "lsys_input.tab.c"
    break;

  case 26: /* ProdDepth: tMDEPTH Expression tEOL  */
#line 353 "lsys_input.y"
        {
	  switch(type_of_productions) {
	  case HOMOMORPHISM:
	    LsysPtr->Homomorphism.depth = (int) Eval((yyvsp[-1].expression));
	    break;
	  case DECOMPOSITION:
	    LsysPtr->Decomposition.depth = (int) Eval((yyvsp[-1].expression));
	    break;
	  }
	  FreeExpressionSpace((yyvsp[-1].expression));
        }
#line 2024 "lsys_input.tab.c"
    break;

  case 31: /* $@4: %empty  */
#line 376 "lsys_input.y"
                {bufferIndex = 0;}
#line 2030 "lsys_input.tab.c"
    break;

  case 32: /* Label: tLSTART $@4 Characters tEOL  */
#line 377 "lsys_input.y"
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
#line 2125 "lsys_input.tab.c"
    break;

  case 46: /* Item: error tEOL  */
#line 485 "lsys_input.y"
        {
	  if(first_warning) {
	    WarningParsing((char*) "Invalid item", ERROR_LVL);
		first_warning = 0;
	  }
	}
#line 2136 "lsys_input.tab.c"
    break;

  case 47: /* Ring: tRING tEOL  */
#line 494 "lsys_input.y"
        {
		LsysPtr->ring = TRUE;
		VERBOSE( "Ring \n");
	}
#line 2145 "lsys_input.tab.c"
    break;

  case 48: /* Seed: tSEED Expression tEOL  */
#line 501 "lsys_input.y"
        {
		LsysPtr->stochastic = TRUE;
		LsysPtr->seed = (int) Eval((yyvsp[-1].expression));
		VERBOSE( "Stochastic with seed %d\n", LsysPtr->seed);
	}
#line 2155 "lsys_input.tab.c"
    break;

  case 49: /* Dlength: tDLENGTH Expression tEOL  */
#line 509 "lsys_input.y"
        {
	  /* derivation length is important only for main Lsystem */
	  if (lsystemCount == 1) {
	    /* evaluate */
	    LsysPtr->n = (int) Eval((yyvsp[-1].expression));
	    /* allocate storage for globals and store */
	    SymbolTableAdd(Strdup("Q"), (double) 0.0,
                           globalSymbolTable);
            SymbolTableAdd(Strdup("Z"), (double) LsysPtr->n, globalSymbolTable);

	    VERBOSE("Derivation length is %d\n", LsysPtr->n);
	  }
	  FreeExpressionSpace((yyvsp[-1].expression));
	}
#line 2174 "lsys_input.tab.c"
    break;

  case 50: /* $@5: %empty  */
#line 524 "lsys_input.y"
                {
		  WarningParsing((char*) "Invalid derivation length", ERROR_LVL);
		}
#line 2182 "lsys_input.tab.c"
    break;

  case 52: /* $@6: %empty  */
#line 529 "lsys_input.y"
                     {bufferIndex = 0;}
#line 2188 "lsys_input.tab.c"
    break;

  case 53: /* Consider: tCONSIDER $@6 Characters tEOL  */
#line 530 "lsys_input.y"
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
#line 2208 "lsys_input.tab.c"
    break;

  case 54: /* $@7: %empty  */
#line 546 "lsys_input.y"
                 {bufferIndex = 0;}
#line 2214 "lsys_input.tab.c"
    break;

  case 55: /* Ignore: tIGNORE $@7 Characters tEOL  */
#line 547 "lsys_input.y"
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
#line 2233 "lsys_input.tab.c"
    break;

  case 56: /* Characters: tSYMBOL  */
#line 563 "lsys_input.y"
        {
		buffer[bufferIndex] = (yyvsp[0].symbol);
		bufferIndex++;
		buffer[bufferIndex] = '\0';
	}
#line 2243 "lsys_input.tab.c"
    break;

  case 57: /* Characters: Characters tSYMBOL  */
#line 569 "lsys_input.y"
        {
		buffer[bufferIndex] = (yyvsp[0].symbol);
		bufferIndex++;
		buffer[bufferIndex] = '\0';
	}
#line 2253 "lsys_input.tab.c"
    break;

  case 58: /* Statements: Statements Statement  */
#line 577 "lsys_input.y"
        {
		(yyval.statement) = BuildStatementList((yyvsp[-1].statement), (yyvsp[0].statement));
	}
#line 2261 "lsys_input.tab.c"
    break;

  case 59: /* Statements: Statement  */
#line 581 "lsys_input.y"
        {
		(yyval.statement) = BuildStatementList((Statement *)NULL, (yyvsp[0].statement));
	}
#line 2269 "lsys_input.tab.c"
    break;

  case 60: /* Statement: Assignment  */
#line 587 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[0].statement);
	}
#line 2277 "lsys_input.tab.c"
    break;

  case 61: /* Statement: Procedure  */
#line 591 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[0].statement);
	}
#line 2285 "lsys_input.tab.c"
    break;

  case 62: /* Statement: IfStatement  */
#line 595 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[0].statement);
	}
#line 2293 "lsys_input.tab.c"
    break;

  case 63: /* Statement: WhileStatement  */
#line 599 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[0].statement);
	}
#line 2301 "lsys_input.tab.c"
    break;

  case 64: /* Statement: DoStatement  */
#line 603 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[0].statement);
	}
#line 2309 "lsys_input.tab.c"
    break;

  case 65: /* Statement: error tEOL  */
#line 607 "lsys_input.y"
        {
	  WarningParsing((char*) "Invalid statement", ERROR_LVL);
		(yyval.statement) = NULL;
	}
#line 2318 "lsys_input.tab.c"
    break;

  case 66: /* ArrayDim: tLBRACKET Expression tRBRACKET  */
#line 614 "lsys_input.y"
        {
		(yyval.expression) = (yyvsp[-1].expression);
	}
#line 2326 "lsys_input.tab.c"
    break;

  case 67: /* ArrayDims: ArrayDims ArrayDim  */
#line 620 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((yyvsp[-1].parameter), (yyvsp[0].expression), 'e');
	}
#line 2334 "lsys_input.tab.c"
    break;

  case 68: /* ArrayDims: ArrayDim  */
#line 624 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((Parameter *)NULL, (yyvsp[0].expression), 'e');
	}
#line 2342 "lsys_input.tab.c"
    break;

  case 69: /* ArrayDef: tNAME ArrayDims  */
#line 630 "lsys_input.y"
        {
		tokenPtr = NewToken();
#ifdef JIM
		tokenPtr->symbol = CurrentSymbolTableFind((yyvsp[-1].name),
							  currentSymbolTable);
#else
		tokenPtr->symbol = SymbolTableFind((yyvsp[-1].name), currentSymbolTable);
#endif
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd((yyvsp[-1].name), 0.0, currentSymbolTable);
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", (yyvsp[-1].name));
		  WarningParsing(buffer, ERROR_LVL);
		  Free((yyvsp[-1].name));
		  (yyvsp[-1].name) = NULL;

		}
		tokenPtr->tokenString = Strdup(yylval.name);
#ifdef JIM
		tokenPtr->symbol->type=DOUBLEARRAY;
#endif
		tokenPtr->token = tARRAYDEF;
		/* process ArrayDim parameter list and allocate storage */
#ifdef JIM
		AllocateArray(tokenPtr->symbol, (yyvsp[0].parameter), currentSymbolTable);
#else
		AllocateArray(tokenPtr->symbol, (yyvsp[0].parameter));
#endif
		(yyval.expression) = tokenPtr;
	}
#line 2378 "lsys_input.tab.c"
    break;

  case 70: /* ArrayDef: tNAME ArrayDims tASSIGN ArrayInitBlock  */
#line 662 "lsys_input.y"
        {
		tokenPtr = NewToken();
#ifdef JIM
		tokenPtr->symbol = CurrentSymbolTableFind((yyvsp[-3].name),
							  currentSymbolTable);
#else
		tokenPtr->symbol = SymbolTableFind((yyvsp[-3].name), currentSymbolTable);
#endif
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd((yyvsp[-3].name), 0.0, currentSymbolTable);
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", (yyvsp[-3].name));
		  WarningParsing(buffer, ERROR_LVL);
		  Free((yyvsp[-3].name));
		  (yyvsp[-3].name) = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);

#ifdef JIM
		tokenPtr->symbol->type=DOUBLEARRAY;
#endif
		tokenPtr->token = tARRAYDEF;
		/* process ArrayDim parameter list and allocate storage */
#ifdef JIM
		AllocateArray(tokenPtr->symbol, (yyvsp[-2].parameter), currentSymbolTable);
#else
		AllocateArray(tokenPtr->symbol, (yyvsp[-2].parameter));
#endif
		InitializeArray(tokenPtr->symbol, (yyvsp[0].parameter));
		/* CheckArrayData(tokenPtr->symbol);*/
		(yyval.expression) = tokenPtr;
	}
#line 2416 "lsys_input.tab.c"
    break;

  case 71: /* ArrayDefs: ArrayDefs tCOMMA ArrayDef  */
#line 698 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((yyvsp[-2].parameter), (yyvsp[0].expression), 'e');
	}
#line 2424 "lsys_input.tab.c"
    break;

  case 72: /* ArrayDefs: ArrayDef  */
#line 702 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((Parameter *)NULL, (yyvsp[0].expression), 'e');
	}
#line 2432 "lsys_input.tab.c"
    break;

  case 73: /* ArrayDefStatement: tARRAY ArrayDefs tSEMI BlankLines  */
#line 708 "lsys_input.y"
        { 
		statementPtr = NewStatement();
		statementPtr->expression = BuildExprList((EToken *)NULL, (yyvsp[-2].parameter), TRUE) ;
		statementPtr->type = stmntARRAYDEF;
		(yyval.statement) = statementPtr;
	}
#line 2443 "lsys_input.tab.c"
    break;

  case 74: /* ExternalDef: tNAME  */
#line 718 "lsys_input.y"
        {
#ifdef JIM
		externalSymbol = CurrentSymbolTableFind((yyvsp[0].name), currentSymbolTable);
		if (externalSymbol == NULL) {
		  externalSymbol = SymbolTableAdd((yyvsp[0].name), 0.0, currentSymbolTable);
		  externalSymbol->type=EXTERNAL;
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", (yyvsp[0].name));
		  WarningParsing(buffer, ERROR_LVL);
		  Free((yyvsp[0].name));
		  (yyvsp[0].name) = NULL;
		}
#endif
	}
#line 2463 "lsys_input.tab.c"
    break;

  case 75: /* ExternalDef: tNAME ArrayDims  */
#line 734 "lsys_input.y"
        {
#ifdef JIM
		externalSymbol = CurrentSymbolTableFind((yyvsp[-1].name), currentSymbolTable);
		if (externalSymbol == NULL) {
		  externalSymbol = SymbolTableAdd((yyvsp[-1].name), 0.0, currentSymbolTable);
		  externalSymbol->type=EXTERNALARRAY;
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", (yyvsp[-1].name));
		  WarningParsing(buffer, ERROR_LVL);
		  Free((yyvsp[-1].name));
		  (yyvsp[-1].name) = NULL;
		}
		/* process ArrayDim parameter list and allocate storage */
		AllocateArray(externalSymbol, (yyvsp[0].parameter), currentSymbolTable);
#endif		
	}
#line 2485 "lsys_input.tab.c"
    break;

  case 76: /* ExternalDef: tNAME ArrayDims tASSIGN ArrayInitBlock  */
#line 752 "lsys_input.y"
        {
#ifdef JIM
		externalSymbol = CurrentSymbolTableFind((yyvsp[-3].name), currentSymbolTable);
		if (externalSymbol == NULL) {
		  externalSymbol = SymbolTableAdd((yyvsp[-3].name), 0.0, currentSymbolTable);
		  externalSymbol->type=EXTERNALARRAY;
		}
		else {
		  sprintf(buffer, "Identifier %s previously defined", (yyvsp[-3].name));
		  WarningParsing(buffer, ERROR_LVL);
		  Free((yyvsp[-3].name));
		  (yyvsp[-3].name) = NULL;
		}

		/* process ArrayDim parameter list and allocate storage */
		AllocateArray(externalSymbol, (yyvsp[-2].parameter), currentSymbolTable);
		
		/* initialisation not allowed */
		sprintf(buffer, "Array initialisation ignored for external variable %s", (yyvsp[-3].name));
		WarningParsing(buffer, WARNING_LVL);
		FreeParameterList((yyvsp[0].parameter));
#endif
	}
#line 2513 "lsys_input.tab.c"
    break;

  case 77: /* ExternalDefs: ExternalDefs tCOMMA ExternalDef  */
#line 779 "lsys_input.y"
        {
	}
#line 2520 "lsys_input.tab.c"
    break;

  case 78: /* ExternalDefs: ExternalDef  */
#line 782 "lsys_input.y"
        {
	}
#line 2527 "lsys_input.tab.c"
    break;

  case 79: /* ExternalDefStatement: tEXTERNAL ExternalDefs tSEMI BlankLines  */
#line 787 "lsys_input.y"
        {
	}
#line 2534 "lsys_input.tab.c"
    break;

  case 80: /* DefStatement: ArrayDefStatement  */
#line 792 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[0].statement);
	}
#line 2542 "lsys_input.tab.c"
    break;

  case 81: /* DefStatement: ExternalDefStatement  */
#line 796 "lsys_input.y"
    {
			(yyval.statement) = NULL;
	}
#line 2550 "lsys_input.tab.c"
    break;

  case 82: /* DefStatement: error tEOL  */
#line 800 "lsys_input.y"
    {
      WarningParsing((char*) "Invalid statement in Define:", ERROR_LVL);
		(yyval.statement) = NULL;
	}
#line 2559 "lsys_input.tab.c"
    break;

  case 83: /* DefStatements: DefStatements DefStatement  */
#line 807 "lsys_input.y"
        {
		(yyval.statement) = BuildStatementList((yyvsp[-1].statement), (yyvsp[0].statement));
	}
#line 2567 "lsys_input.tab.c"
    break;

  case 84: /* DefStatements: DefStatement  */
#line 811 "lsys_input.y"
        {
		(yyval.statement) = BuildStatementList((Statement *)NULL, (yyvsp[0].statement));
	}
#line 2575 "lsys_input.tab.c"
    break;

  case 85: /* Assignment: LHS tASSIGN Expression tSEMI BlankLines  */
#line 817 "lsys_input.y"
        {
		statementPtr = NewStatement();
		statementPtr->leftHandSide = (yyvsp[-4].expression);
		statementPtr->expression = (yyvsp[-2].expression);

		/* LHS is either a variable or an array reference */
		if (statementPtr->leftHandSide->nextParam == NULL) {
			statementPtr->type = stmntASSIGN;
		}
		else {
			statementPtr->type = stmntARRAYASSIGN;
		}
		(yyval.statement) = statementPtr;
	}
#line 2594 "lsys_input.tab.c"
    break;

  case 86: /* ArrayRef: tLBRACKET Expression tRBRACKET  */
#line 834 "lsys_input.y"
        {
		(yyval.expression) = (yyvsp[-1].expression);
	}
#line 2602 "lsys_input.tab.c"
    break;

  case 87: /* ArrayRefs: ArrayRefs ArrayRef  */
#line 840 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((yyvsp[-1].parameter), (yyvsp[0].expression), 'e');
	}
#line 2610 "lsys_input.tab.c"
    break;

  case 88: /* ArrayRefs: ArrayRef  */
#line 844 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((Parameter *)NULL, (yyvsp[0].expression), 'e');
	}
#line 2618 "lsys_input.tab.c"
    break;

  case 89: /* LValue: tADDRESS tNAME  */
#line 850 "lsys_input.y"
        { 
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind((yyvsp[0].name), currentSymbolTable);
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd((yyvsp[0].name), 0.0, currentSymbolTable);
		}
		else { /* here's the new stuff */
		  if (tokenPtr->symbol->type==DOUBLEARRAY 
#ifdef JIM
		      || tokenPtr->symbol->type==EXTERNALARRAY
#endif
		      ) {
		  	sprintf(buffer, "Identifier %s previously defined as an array", (yyvsp[-1].operator_));
		  	WarningParsing(buffer, ERROR_LVL);
		   } /* new bit ends here */
		  Free((yyvsp[0].name));
		  (yyvsp[0].name) = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tNAMELVAL;
		(yyval.expression) = tokenPtr;
	}
#line 2645 "lsys_input.tab.c"
    break;

  case 90: /* LValue: tADDRESS tNAME ArrayRefs  */
#line 873 "lsys_input.y"
        {
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind((yyvsp[-1].name), currentSymbolTable);
		if (tokenPtr->symbol == NULL||tokenPtr->symbol->arrayData == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined as an array", (yyvsp[-1].name));
		  WarningParsing(buffer, ERROR_LVL);
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tARRAYLVAL;
		(yyval.expression) = BuildExprList(tokenPtr, (yyvsp[0].parameter), TRUE); 
	}
#line 2661 "lsys_input.tab.c"
    break;

  case 91: /* LHS: tNAME  */
#line 888 "lsys_input.y"
        { 
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind((yyvsp[0].name), currentSymbolTable);
		if (tokenPtr->symbol == NULL) {
		  tokenPtr->symbol = SymbolTableAdd((yyvsp[0].name), 0.0, currentSymbolTable);
		}
		else { /* here's the new stuff */
		  if (tokenPtr->symbol->type==DOUBLEARRAY 
#ifdef JIM
		      || tokenPtr->symbol->type==EXTERNALARRAY
#endif
		      ) {
		  	sprintf(buffer, "Identifier %s previously defined as an array", (yyvsp[0].name));
		  	WarningParsing(buffer, ERROR_LVL);
		   } /* new bit ends here */
		  Free((yyvsp[0].name));
		  (yyvsp[0].name) = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tNAME;
		(yyval.expression) = tokenPtr;
	}
#line 2688 "lsys_input.tab.c"
    break;

  case 92: /* LHS: tNAME ArrayRefs  */
#line 911 "lsys_input.y"
        {
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind((yyvsp[-1].name), currentSymbolTable);
		if (tokenPtr->symbol == NULL||tokenPtr->symbol->arrayData == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined as an array", (yyvsp[-1].name));
		  WarningParsing(buffer, ERROR_LVL);
		}
		tokenPtr->tokenString = Strdup(yylval.name);
        Free((yyvsp[-1].name));
		(yyvsp[-1].name) = NULL;
		tokenPtr->token = tARRAYLHS;
		(yyval.expression) = BuildExprList(tokenPtr, (yyvsp[0].parameter), TRUE); 
	}
#line 2706 "lsys_input.tab.c"
    break;

  case 93: /* Procedure: Expression tSEMI BlankLines  */
#line 928 "lsys_input.y"
        { 
		statementPtr = NewStatement();
		statementPtr->expression =  (yyvsp[-2].expression);
		statementPtr->type = stmntPROC;
		(yyval.statement) = statementPtr;
	}
#line 2717 "lsys_input.tab.c"
    break;

  case 94: /* IfStatement: tIF tLPAREN Expression tRPAREN BlankLines Block BlankLines  */
#line 937 "lsys_input.y"
        { 
		statementPtr = NewStatement();
		statementPtr->condition =  (yyvsp[-4].expression);
		statementPtr->type = stmntIF;
		statementPtr->block = (yyvsp[-1].statement);
		statementPtr->elseblock = NULL;
		(yyval.statement) = statementPtr;
	}
#line 2730 "lsys_input.tab.c"
    break;

  case 95: /* IfStatement: tIF tLPAREN Expression tRPAREN BlankLines Block tELSE BlankLines Block BlankLines  */
#line 946 "lsys_input.y"
        { 
		statementPtr = NewStatement();
		statementPtr->condition =  (yyvsp[-7].expression);
		statementPtr->type = stmntIF;
		statementPtr->block = (yyvsp[-4].statement);
		statementPtr->elseblock = (yyvsp[-1].statement);
		(yyval.statement) = statementPtr;
	}
#line 2743 "lsys_input.tab.c"
    break;

  case 96: /* WhileStatement: tWHILE tLPAREN Expression tRPAREN BlankLines Block BlankLines  */
#line 957 "lsys_input.y"
        { 
		statementPtr = NewStatement();
		statementPtr->condition =  (yyvsp[-4].expression);
		statementPtr->type = stmntWHILE;
		statementPtr->block = (yyvsp[-1].statement);
		(yyval.statement) = statementPtr;
	}
#line 2755 "lsys_input.tab.c"
    break;

  case 97: /* DoStatement: tDO BlankLines Block tWHILE tLPAREN Expression tRPAREN tSEMI BlankLines  */
#line 967 "lsys_input.y"
        { 
		statementPtr = NewStatement();
		statementPtr->condition =  (yyvsp[-3].expression);
		statementPtr->type = stmntDO;
		statementPtr->block = (yyvsp[-6].statement);
		(yyval.statement) = statementPtr;
	}
#line 2767 "lsys_input.tab.c"
    break;

  case 98: /* DefineBlock: LBRACE BlankLines DefStatements RBRACE  */
#line 978 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[-1].statement);
	}
#line 2775 "lsys_input.tab.c"
    break;

  case 99: /* Defines: tDEFINE DefineBlock tEOL  */
#line 984 "lsys_input.y"
        {
		LsysPtr->defineBlock = (yyvsp[-1].statement);
		if (clp.verbose) {
			Message( "Define: ");
			PrintStatementList((yyvsp[-1].statement));
		}
	}
#line 2787 "lsys_input.tab.c"
    break;

  case 100: /* ArrayInitBlock: LBRACE BlankLines Parameters RBRACE  */
#line 994 "lsys_input.y"
        {
		(yyval.parameter) = (yyvsp[-1].parameter);
	}
#line 2795 "lsys_input.tab.c"
    break;

  case 101: /* Block: LBRACE BlankLines Statements RBRACE  */
#line 1000 "lsys_input.y"
        {
		(yyval.statement) = (yyvsp[-1].statement);
	}
#line 2803 "lsys_input.tab.c"
    break;

  case 102: /* LBRACE: tLBRACE  */
#line 1006 "lsys_input.y"
        {
		c_expression++; /* lines doesn't have to start with tabs */
        }
#line 2811 "lsys_input.tab.c"
    break;

  case 103: /* RBRACE: tRBRACE  */
#line 1012 "lsys_input.y"
        {
		if(c_expression>0)
		  c_expression--; /* the end of c_like expression */
		else
		  Message("Mismatched parenthesis!");
        }
#line 2822 "lsys_input.tab.c"
    break;

  case 104: /* Startblock: tSTARTBLOCK Block tEOL  */
#line 1021 "lsys_input.y"
        {
		LsysPtr->startBlock = (yyvsp[-1].statement);
		if (clp.verbose) {
			Message( "Start: ");
			PrintStatementList((yyvsp[-1].statement));
		}
	}
#line 2834 "lsys_input.tab.c"
    break;

  case 105: /* Endblock: tENDBLOCK Block tEOL  */
#line 1031 "lsys_input.y"
        {
		LsysPtr->endBlock = (yyvsp[-1].statement);
		if (clp.verbose) {
			Message( "End: ");
			PrintStatementList((yyvsp[-1].statement));
		}
	}
#line 2846 "lsys_input.tab.c"
    break;

  case 106: /* Starteach: tSTARTEACH Block tEOL  */
#line 1041 "lsys_input.y"
        {
		LsysPtr->startEach = (yyvsp[-1].statement);
		if (clp.verbose) {
			Message( "StartEach: ");
			PrintStatementList((yyvsp[-1].statement));
		}
	}
#line 2858 "lsys_input.tab.c"
    break;

  case 107: /* Endeach: tENDEACH Block tEOL  */
#line 1051 "lsys_input.y"
        {
		LsysPtr->endEach = (yyvsp[-1].statement);
		if (clp.verbose) {
			Message( "EndEach: ");
			PrintStatementList((yyvsp[-1].statement));
		}
	}
#line 2870 "lsys_input.tab.c"
    break;

  case 108: /* Axiom: tAXIOM Modules tEOL  */
#line 1061 "lsys_input.y"
        {
        LsysPtr->axiom = (yyvsp[-1].module);
        LsysPtr->axiomLength = PackedModuleListLength((yyvsp[-1].module));
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
#line 2895 "lsys_input.tab.c"
    break;

  case 109: /* Axiom: tAXIOM error tEOL  */
#line 1082 "lsys_input.y"
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
#line 2912 "lsys_input.tab.c"
    break;

  case 113: /* Production: Predecessor Conditional tYIELDS Successor tEOL  */
#line 1102 "lsys_input.y"
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
#line 2929 "lsys_input.tab.c"
    break;

  case 114: /* Production: Predecessor Conditional tOYIELDS Successor tEOL  */
#line 1115 "lsys_input.y"
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
#line 2947 "lsys_input.tab.c"
    break;

  case 115: /* $@8: %empty  */
#line 1129 "lsys_input.y"
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
#line 2964 "lsys_input.tab.c"
    break;

  case 121: /* Lcontext: tNULL  */
#line 1152 "lsys_input.y"
        {
		prodPtr->lCon = (Module *) NULL;
  		VERBOSE( "Left context: NULL\n");
	}
#line 2973 "lsys_input.tab.c"
    break;

  case 122: /* Lcontext: FormalModules  */
#line 1157 "lsys_input.y"
        { 
		modulePtr = (yyvsp[0].module);

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
#line 3016 "lsys_input.tab.c"
    break;

  case 123: /* Strictpred: FormalModules  */
#line 1198 "lsys_input.y"
        { /* Set the strict predecessor */
       	modulePtr = prodPtr->pred = (yyvsp[0].module);

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
#line 3075 "lsys_input.tab.c"
    break;

  case 124: /* Rcontext: tNULL  */
#line 1255 "lsys_input.y"
        {
		prodPtr->rCon = (Module *) NULL;
      	VERBOSE( "Right context: NULL\n");
	}
#line 3084 "lsys_input.tab.c"
    break;

  case 125: /* Rcontext: FormalModules  */
#line 1260 "lsys_input.y"
        { /* Set the right context of the predecessor.              */
	  switch(type_of_productions) {
	  case L_SYSTEM:
	  case DECOMPOSITION:
#ifdef CONTEXT_SENSITIVE_HOMO
	  case HOMOMORPHISM:
#endif
	    prodPtr->rCon = (yyvsp[0].module);
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
#line 3116 "lsys_input.tab.c"
    break;

  case 126: /* Conditional: tCOLON Condition  */
#line 1290 "lsys_input.y"
                        { (yyvsp[-1].operator_); }
#line 3122 "lsys_input.tab.c"
    break;

  case 127: /* Conditional: tCOLON Precondition Condition  */
#line 1292 "lsys_input.y"
                        { (yyvsp[-2].operator_); }
#line 3128 "lsys_input.tab.c"
    break;

  case 128: /* Conditional: tCOLON Condition Postcondition  */
#line 1294 "lsys_input.y"
                        { (yyvsp[-2].operator_); }
#line 3134 "lsys_input.tab.c"
    break;

  case 129: /* Conditional: tCOLON Precondition Condition Postcondition  */
#line 1296 "lsys_input.y"
                        { (yyvsp[-3].operator_); }
#line 3140 "lsys_input.tab.c"
    break;

  case 130: /* Conditional: %empty  */
#line 1298 "lsys_input.y"
        {
		/* Condition statement does not exist. Assume true */
		EToken *trueToken = NewToken();
		trueToken->token = tVALUE;
		trueToken->symbol = &trueSymbol;
		trueToken->tokenString = Strdup("TRUE");

		prodPtr->condition = trueToken;

        VERBOSE( "Condition: TRUE\n");
	}
#line 3156 "lsys_input.tab.c"
    break;

  case 131: /* Precondition: Block  */
#line 1312 "lsys_input.y"
        {
		prodPtr->preCondList = (yyvsp[0].statement);

        if (clp.verbose) {
			Message( "Precondition Statements: ");
			PrintStatementList((yyvsp[0].statement));
		}
	}
#line 3169 "lsys_input.tab.c"
    break;

  case 132: /* Postcondition: Block  */
#line 1323 "lsys_input.y"
        {
		prodPtr->postCondList = (yyvsp[0].statement);

        if (clp.verbose) {
			Message( "Postcondition Statements: ");
			PrintStatementList((yyvsp[0].statement));
		}
	}
#line 3182 "lsys_input.tab.c"
    break;

  case 133: /* Condition: tTIMES  */
#line 1334 "lsys_input.y"
        {
		/* Condition statement does not exist. Assume true */
		EToken *trueToken = NewToken();
		trueToken->token = tVALUE;
		trueToken->symbol = &trueSymbol;
		trueToken->tokenString = Strdup("TRUE");
		
		prodPtr->condition = trueToken;

        VERBOSE( "Condition: TRUE\n");
	}
#line 3198 "lsys_input.tab.c"
    break;

  case 134: /* Condition: Expression  */
#line 1346 "lsys_input.y"
        {
		prodPtr->condition = (yyvsp[0].expression);

    	if (clp.verbose) {
			Message( "Condition: ");
			PrintExpression(prodPtr->condition);
			Message( "\n");
		}
	}
#line 3212 "lsys_input.tab.c"
    break;

  case 135: /* Successor: StrictSucc  */
#line 1358 "lsys_input.y"
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
#line 3230 "lsys_input.tab.c"
    break;

  case 136: /* Successor: StrictSucc Probability  */
#line 1373 "lsys_input.y"
        { 
		if ( ! LsysPtr->stochastic ) {
			WarningParsing((char*) "Probability given for deterministic production", 
					WARNING_LVL);
		}
	}
#line 3241 "lsys_input.tab.c"
    break;

  case 137: /* StrictSucc: tNULL  */
#line 1382 "lsys_input.y"
        {
		/* The successor is empty */
		prodPtr->succ = (Module *)NULL;
		prodPtr->succLen = 0;
		VERBOSE( "Successor: NULL\n");
	}
#line 3252 "lsys_input.tab.c"
    break;

  case 138: /* StrictSucc: Modules  */
#line 1389 "lsys_input.y"
        { /* The successor is not empty */
#ifdef JIM
		modulePtr = prodPtr->succ = (yyvsp[0].module);
#else
		prodPtr->succ = (yyvsp[0].module);
#endif
		prodPtr->succLen = PackedModuleListLength((yyvsp[0].module));

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
#line 3296 "lsys_input.tab.c"
    break;

  case 139: /* Probability: tCOLON Expression  */
#line 1431 "lsys_input.y"
        { /* convert probability */
		prodPtr->probExpression = (yyvsp[0].expression);

		if (clp.verbose) {
			Message( "Probability: ");
			PrintExpression((yyvsp[0].expression));
		}
	}
#line 3309 "lsys_input.tab.c"
    break;

  case 140: /* FormalModules: FormalModule  */
#line 1442 "lsys_input.y"
        {
		(yyval.module) = (yyvsp[0].module);
	}
#line 3317 "lsys_input.tab.c"
    break;

  case 141: /* FormalModules: FormalModules FormalModule  */
#line 1446 "lsys_input.y"
        { 
		(yyval.module) = BuildModuleList((yyvsp[-1].module), (yyvsp[0].module));
	}
#line 3325 "lsys_input.tab.c"
    break;

  case 142: /* FormalModule: Symbol  */
#line 1451 "lsys_input.y"
        {
		(yyval.module) = (yyvsp[0].module);
		if((yyvsp[0].module)->symbol == '?') clp.checkEnvironment = 1;
	}
#line 3334 "lsys_input.tab.c"
    break;

  case 143: /* FormalModule: Symbol tLPAREN FormalParameters tRPAREN  */
#line 1456 "lsys_input.y"
        { 
		/* assign parameter list to module */
		if((yyvsp[-3].module)->symbol == '%') clp.checkEnvironment = 1;

		(yyval.module) = AssignParameters((yyvsp[-3].module), (yyvsp[-1].parameter));
	}
#line 3345 "lsys_input.tab.c"
    break;

  case 144: /* Modules: Module  */
#line 1464 "lsys_input.y"
        {
		(yyval.module) = (yyvsp[0].module);
	}
#line 3353 "lsys_input.tab.c"
    break;

  case 145: /* Modules: Modules Module  */
#line 1468 "lsys_input.y"
        { 
		(yyval.module) = BuildModuleList((yyvsp[-1].module), (yyvsp[0].module));
	}
#line 3361 "lsys_input.tab.c"
    break;

  case 146: /* Module: Symbol  */
#line 1473 "lsys_input.y"
        {
		(yyval.module) = (yyvsp[0].module);
		if((yyvsp[0].module)->symbol == '?') clp.checkEnvironment = 1;
	}
#line 3370 "lsys_input.tab.c"
    break;

  case 147: /* Module: Symbol tLPAREN Parameters tRPAREN  */
#line 1478 "lsys_input.y"
        { 
		/* assign parameter list to module $1 */
		if((yyvsp[-3].module)->symbol == '%') clp.checkEnvironment = 1;

		(yyval.module) = AssignParameters((yyvsp[-3].module), (yyvsp[-1].parameter));
	}
#line 3381 "lsys_input.tab.c"
    break;

  case 148: /* Symbol: tSYMBOL  */
#line 1486 "lsys_input.y"
        {
		/* allocate module */
		newModule = NewModule();
		/* get the current symbol and initialize the structure */
    	newModule->symbol = (yyvsp[0].symbol);
    	newModule->length = 1;
    	newModule->parameters = 0;
    	newModule->parmList = NULL;

		(yyval.module) = newModule;
	}
#line 3397 "lsys_input.tab.c"
    break;

  case 149: /* FormalParameters: FormalParameter  */
#line 1499 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((Parameter *)NULL, (yyvsp[0].identifier), 'f');
	}
#line 3405 "lsys_input.tab.c"
    break;

  case 150: /* FormalParameters: FormalParameters tCOMMA FormalParameter  */
#line 1503 "lsys_input.y"
        { 
		(yyval.parameter) = BuildParameterList((yyvsp[-2].parameter), (yyvsp[0].identifier), 'f');
	}
#line 3413 "lsys_input.tab.c"
    break;

  case 151: /* FormalParameter: tNAME  */
#line 1508 "lsys_input.y"
        {
		tokenPtr = NewToken();
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tNAME;
		/* add to symbol table; warning if duplicated */
		tokenPtr->symbol = SymbolTableAdd((yyvsp[0].name), 0.0,currentSymbolTable);
		(yyval.identifier) = tokenPtr;
	}
#line 3426 "lsys_input.tab.c"
    break;

  case 152: /* Parameters: Expression  */
#line 1518 "lsys_input.y"
        {
		(yyval.parameter) = BuildParameterList((Parameter *)NULL, (yyvsp[0].expression), 'e');
	}
#line 3434 "lsys_input.tab.c"
    break;

  case 153: /* Parameters: Parameters tCOMMA Expression  */
#line 1522 "lsys_input.y"
        { 
		(yyval.parameter) = BuildParameterList((yyvsp[-2].parameter), (yyvsp[0].expression), 'e');
	}
#line 3442 "lsys_input.tab.c"
    break;

  case 154: /* Expression: Expression tOR Expression  */
#line 1528 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tOR, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3448 "lsys_input.tab.c"
    break;

  case 155: /* Expression: Expression tAND Expression  */
#line 1530 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tAND, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3454 "lsys_input.tab.c"
    break;

  case 156: /* Expression: Expression tEQUAL Expression  */
#line 1532 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tEQUAL, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3460 "lsys_input.tab.c"
    break;

  case 157: /* Expression: Expression tNOTEQUAL Expression  */
#line 1534 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tNOTEQUAL, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3466 "lsys_input.tab.c"
    break;

  case 158: /* Expression: Expression tLT Expression  */
#line 1536 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tLT, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3472 "lsys_input.tab.c"
    break;

  case 159: /* Expression: Expression tLE Expression  */
#line 1538 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tLE, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3478 "lsys_input.tab.c"
    break;

  case 160: /* Expression: Expression tGT Expression  */
#line 1540 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tGT, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3484 "lsys_input.tab.c"
    break;

  case 161: /* Expression: Expression tGE Expression  */
#line 1542 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tGE, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3490 "lsys_input.tab.c"
    break;

  case 162: /* Expression: Expression tPLUS Expression  */
#line 1544 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tPLUS, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3496 "lsys_input.tab.c"
    break;

  case 163: /* Expression: Expression tMINUS Expression  */
#line 1546 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tMINUS, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3502 "lsys_input.tab.c"
    break;

  case 164: /* Expression: Expression tTIMES Expression  */
#line 1548 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tTIMES, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3508 "lsys_input.tab.c"
    break;

  case 165: /* Expression: Expression tDIVIDE Expression  */
#line 1550 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tDIVIDE, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3514 "lsys_input.tab.c"
    break;

  case 166: /* Expression: Expression tREM Expression  */
#line 1552 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tREM, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3520 "lsys_input.tab.c"
    break;

  case 167: /* Expression: Expression tPOW Expression  */
#line 1554 "lsys_input.y"
        { (yyval.expression) = BuildBinary((yyvsp[-2].expression), OperatorToken(tPOW, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3526 "lsys_input.tab.c"
    break;

  case 168: /* Expression: tMINUS Expression  */
#line 1556 "lsys_input.y"
        { (yyval.expression) = BuildUnary(OperatorToken(tUMINUS, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3532 "lsys_input.tab.c"
    break;

  case 169: /* Expression: tNOT Expression  */
#line 1558 "lsys_input.y"
        { (yyval.expression) = BuildUnary(OperatorToken(tNOT, (yyvsp[-1].operator_)), (yyvsp[0].expression)); }
#line 3538 "lsys_input.tab.c"
    break;

  case 170: /* Expression: Expression tQUESTION Expression tCOLON Expression  */
#line 1560 "lsys_input.y"
        { (yyval.expression) = BuildTrinary(OperatorToken(tQUESTION, (yyvsp[-3].operator_)), (yyvsp[-4].expression), (yyvsp[-2].expression), (yyvsp[0].expression)); }
#line 3544 "lsys_input.tab.c"
    break;

  case 171: /* Expression: tLPAREN Expression tRPAREN  */
#line 1562 "lsys_input.y"
        { (yyval.expression) = (yyvsp[-1].expression); }
#line 3550 "lsys_input.tab.c"
    break;

  case 172: /* Expression: LValue  */
#line 1564 "lsys_input.y"
        { (yyval.expression) = (yyvsp[0].expression); }
#line 3556 "lsys_input.tab.c"
    break;

  case 173: /* Expression: Function  */
#line 1566 "lsys_input.y"
        { (yyval.expression) = (yyvsp[0].expression); }
#line 3562 "lsys_input.tab.c"
    break;

  case 174: /* Expression: Name  */
#line 1568 "lsys_input.y"
        { (yyval.expression) = (yyvsp[0].identifier); }
#line 3568 "lsys_input.tab.c"
    break;

  case 175: /* Expression: Value  */
#line 1570 "lsys_input.y"
        { (yyval.expression) = (yyvsp[0].identifier); }
#line 3574 "lsys_input.tab.c"
    break;

  case 176: /* Expression: String  */
#line 1572 "lsys_input.y"
        { (yyval.expression) = (yyvsp[0].identifier); }
#line 3580 "lsys_input.tab.c"
    break;

  case 177: /* Function: FunctionName tLPAREN Parameters tRPAREN  */
#line 1575 "lsys_input.y"
        { 
		int argcount = MatchParameters((yyvsp[-1].parameter),(yyvsp[-3].function));		/* argcount must be passed to printf */
		(yyval.expression) = BuildExprList(OperatorToken((yyvsp[-3].function),(char*) "function"),(yyvsp[-1].parameter),argcount); 
	}
#line 3589 "lsys_input.tab.c"
    break;

  case 221: /* Value: tVALUE  */
#line 1591 "lsys_input.y"
        { 
		tokenPtr = NewToken();
		tokenPtr->symbol = 
                    SymbolTableAdd(Strdup((char *) constantToken),
				 (yyvsp[0].value), currentSymbolTable);
		tokenPtr->token = tVALUE;
		sprintf(buffer,"%g",(yyvsp[0].value));
		tokenPtr->tokenString = Strdup(buffer);
		if(constantToken[strlen((char *) constantToken)-1] == 255 ) {
                    constantToken[strlen((char *) constantToken)+1] = '\0';
                    constantToken[strlen((char *) constantToken)] = '\200';
		}
		else
                    constantToken[strlen((char *) constantToken)-1]++;
		(yyval.identifier) = tokenPtr;
	}
#line 3610 "lsys_input.tab.c"
    break;

  case 222: /* String: tSTRING  */
#line 1610 "lsys_input.y"
        { 
		tokenPtr = NewToken();
		/* tokenPtr->symbol = NULL; JH1 */
		tokenPtr->token = tSTRING;
		tokenPtr->tokenString = Strdup((yyvsp[0].name));
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
		(yyval.identifier) = tokenPtr;
	}
#line 3638 "lsys_input.tab.c"
    break;

  case 223: /* Name: tNAME  */
#line 1636 "lsys_input.y"
        { 
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind((yyvsp[0].name), currentSymbolTable);
		if (tokenPtr->symbol == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined", (yyvsp[0].name));
		  WarningParsing(buffer, WARNING_LVL);
		  tokenPtr->symbol = SymbolTableAdd((yyvsp[0].name), 0.0, currentSymbolTable);
		}
		else { /* here's the new stuff */
		  if (tokenPtr->symbol->type==DOUBLEARRAY 
#ifdef JIM
		      || tokenPtr->symbol->type==EXTERNALARRAY
#endif
		      ) {
		  	sprintf(buffer, "Identifier %s previously defined as an array", (yyvsp[0].name));
		  	WarningParsing(buffer, ERROR_LVL);
		   } /* new bit ends here */

		  Free((yyvsp[0].name));
		  (yyvsp[0].name) = NULL;
		}
		tokenPtr->tokenString = Strdup(yylval.name);

		tokenPtr->token = tNAME;


		(yyval.identifier) = tokenPtr;
	}
#line 3671 "lsys_input.tab.c"
    break;

  case 224: /* Name: tNAME ArrayRefs  */
#line 1665 "lsys_input.y"
        {
		tokenPtr = NewToken();
		tokenPtr->symbol = SymbolTableFind((yyvsp[-1].name), currentSymbolTable);
		if (tokenPtr->symbol == NULL||tokenPtr->symbol->arrayData == NULL) {
		  sprintf(buffer, "Identifier %s not previously defined as an array", (yyvsp[-1].name));
		  WarningParsing(buffer, ERROR_LVL);
		  Free(tokenPtr);
		  tokenPtr = NULL;
		  Free((yyvsp[-1].name));
		  (yyvsp[-1].name) = NULL;
		  return 0;
		}
		tokenPtr->tokenString = Strdup(yylval.name);
		tokenPtr->token = tARRAYREF;
		i = CountParameters((yyvsp[0].parameter));
		if (tokenPtr->symbol->arrayData->dimensions != i) {
			sprintf(buffer, "Array identifier %s requires %d dimensions, not %d", 
				tokenPtr->symbol->label, tokenPtr->symbol->arrayData->dimensions, i);
			WarningParsing(buffer, ERROR_LVL);
			
		}

		Free((yyvsp[-1].name));
		(yyvsp[-1].name) = NULL;

		(yyval.identifier) = BuildExprList(tokenPtr, (yyvsp[0].parameter), TRUE); 
	}
#line 3703 "lsys_input.tab.c"
    break;


#line 3707 "lsys_input.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 1693 "lsys_input.y"


static void yyerror( __attribute__((unused)) char * s)
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
