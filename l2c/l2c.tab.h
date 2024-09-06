/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_L2C_L2C_TAB_H_INCLUDED
# define YY_L2C_L2C_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int l2cdebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    tAXIOM = 258,                  /* tAXIOM  */
    tMODULE = 259,                 /* tMODULE  */
    tPRODUCE = 260,                /* tPRODUCE  */
    tNPRODUCE = 261,               /* tNPRODUCE  */
    tSTART = 262,                  /* tSTART  */
    tSTARTEACH = 263,              /* tSTARTEACH  */
    tENDEACH = 264,                /* tENDEACH  */
    tEND = 265,                    /* tEND  */
    tMAXDEPTH = 266,               /* tMAXDEPTH  */
    tPROPENSITY = 267,             /* tPROPENSITY  */
    tGGROUP = 268,                 /* tGGROUP  */
    tCONSIDER = 269,               /* tCONSIDER  */
    tIGNORE = 270,                 /* tIGNORE  */
    tDERIVLENGTH = 271,            /* tDERIVLENGTH  */
    tRINGLSYSTEM = 272,            /* tRINGLSYSTEM  */
    tINTERPRETATION = 273,         /* tINTERPRETATION  */
    tDECOMPOSITION = 274,          /* tDECOMPOSITION  */
    tPRODUCTION = 275,             /* tPRODUCTION  */
    tGROUP = 276,                  /* tGROUP  */
    tENDGROUP = 277,               /* tENDGROUP  */
    tVGROUP = 278,                 /* tVGROUP  */
    tENDVGROUP = 279,              /* tENDVGROUP  */
    tCOLON = 280,                  /* tCOLON  */
    tLPAREN = 281,                 /* tLPAREN  */
    tRPAREN = 282,                 /* tRPAREN  */
    tSEMICOLON = 283,              /* tSEMICOLON  */
    tCOMMA = 284,                  /* tCOMMA  */
    tLESSTHAN = 285,               /* tLESSTHAN  */
    tLEFTSHIFT = 286,              /* tLEFTSHIFT  */
    tGREATERTHAN = 287,            /* tGREATERTHAN  */
    tRIGHTSHIFT = 288,             /* tRIGHTSHIFT  */
    tENDPRODPROTO = 289,           /* tENDPRODPROTO  */
    tEQUALS = 290,                 /* tEQUALS  */
    tINTEGER = 291,                /* tINTEGER  */
    tIDENT = 292,                  /* tIDENT  */
    tMODULEIDENT = 293,            /* tMODULEIDENT  */
    tERROR = 294,                  /* tERROR  */
    tVERIFYSTRING = 295,           /* tVERIFYSTRING  */
    tINRIGHTCONTEXT = 296,         /* tINRIGHTCONTEXT  */
    tINLEFTCONTEXT = 297,          /* tINLEFTCONTEXT  */
    tINNEWRIGHTCONTEXT = 298,      /* tINNEWRIGHTCONTEXT  */
    tINNEWLEFTCONTEXT = 299        /* tINNEWLEFTCONTEXT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define tAXIOM 258
#define tMODULE 259
#define tPRODUCE 260
#define tNPRODUCE 261
#define tSTART 262
#define tSTARTEACH 263
#define tENDEACH 264
#define tEND 265
#define tMAXDEPTH 266
#define tPROPENSITY 267
#define tGGROUP 268
#define tCONSIDER 269
#define tIGNORE 270
#define tDERIVLENGTH 271
#define tRINGLSYSTEM 272
#define tINTERPRETATION 273
#define tDECOMPOSITION 274
#define tPRODUCTION 275
#define tGROUP 276
#define tENDGROUP 277
#define tVGROUP 278
#define tENDVGROUP 279
#define tCOLON 280
#define tLPAREN 281
#define tRPAREN 282
#define tSEMICOLON 283
#define tCOMMA 284
#define tLESSTHAN 285
#define tLEFTSHIFT 286
#define tGREATERTHAN 287
#define tRIGHTSHIFT 288
#define tENDPRODPROTO 289
#define tEQUALS 290
#define tINTEGER 291
#define tIDENT 292
#define tMODULEIDENT 293
#define tERROR 294
#define tVERIFYSTRING 295
#define tINRIGHTCONTEXT 296
#define tINLEFTCONTEXT 297
#define tINNEWRIGHTCONTEXT 298
#define tINNEWLEFTCONTEXT 299

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 77 "l2c.y"

  char Ident[__lc_eMaxIdentifierLength+1];
  int ModuleId;
  int Integer;
  ParametersList ParamsList;
  FormalModuleDt* pFormalModuleData;
  FormalModuleDtList* pFormalModuleDataList;
  ContextData context;

#line 165 "l2c.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE l2clval;


int l2cparse (void);


#endif /* !YY_L2C_L2C_TAB_H_INCLUDED  */
