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

#ifndef YY_LSYS_INPUT_LSYS_INPUT_TAB_H_INCLUDED
# define YY_LSYS_INPUT_LSYS_INPUT_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int lsys_inputdebug;
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
    tLSTART = 258,                 /* tLSTART  */
    tRING = 259,                   /* tRING  */
    tSEED = 260,                   /* tSEED  */
    tDLENGTH = 261,                /* tDLENGTH  */
    tMDEPTH = 262,                 /* tMDEPTH  */
    tCONSIDER = 263,               /* tCONSIDER  */
    tIGNORE = 264,                 /* tIGNORE  */
    tAXIOM = 265,                  /* tAXIOM  */
    tEND = 266,                    /* tEND  */
    tDEFINE = 267,                 /* tDEFINE  */
    tSTARTBLOCK = 268,             /* tSTARTBLOCK  */
    tENDBLOCK = 269,               /* tENDBLOCK  */
    tSTARTEACH = 270,              /* tSTARTEACH  */
    tENDEACH = 271,                /* tENDEACH  */
    tSTARTHOMO = 272,              /* tSTARTHOMO  */
    tSTARTDECOMP = 273,            /* tSTARTDECOMP  */
    tNOWARNING = 274,              /* tNOWARNING  */
    tWARNING = 275,                /* tWARNING  */
    tLSEP = 276,                   /* tLSEP  */
    tRSEP = 277,                   /* tRSEP  */
    tYIELDS = 278,                 /* tYIELDS  */
    tOYIELDS = 279,                /* tOYIELDS  */
    tNULL = 280,                   /* tNULL  */
    tEOL = 281,                    /* tEOL  */
    tIF = 282,                     /* tIF  */
    tDO = 283,                     /* tDO  */
    tWHILE = 284,                  /* tWHILE  */
    tELSE = 285,                   /* tELSE  */
    tARRAY = 286,                  /* tARRAY  */
    tEXTERNAL = 287,               /* tEXTERNAL  */
    tLPAREN = 288,                 /* tLPAREN  */
    tRPAREN = 289,                 /* tRPAREN  */
    tLBRACE = 290,                 /* tLBRACE  */
    tRBRACE = 291,                 /* tRBRACE  */
    tCOMMA = 292,                  /* tCOMMA  */
    tLBRACKET = 293,               /* tLBRACKET  */
    tRBRACKET = 294,               /* tRBRACKET  */
    tSEMI = 295,                   /* tSEMI  */
    tATAN2 = 296,                  /* tATAN2  */
    tTAN = 297,                    /* tTAN  */
    tSIN = 298,                    /* tSIN  */
    tCOS = 299,                    /* tCOS  */
    tATAN = 300,                   /* tATAN  */
    tASIN = 301,                   /* tASIN  */
    tACOS = 302,                   /* tACOS  */
    tSRAND = 303,                  /* tSRAND  */
    tRAN = 304,                    /* tRAN  */
    tNRAN = 305,                   /* tNRAN  */
    tBRAN = 306,                   /* tBRAN  */
    tBIRAN = 307,                  /* tBIRAN  */
    tEXP = 308,                    /* tEXP  */
    tLOG = 309,                    /* tLOG  */
    tFLOOR = 310,                  /* tFLOOR  */
    tCEIL = 311,                   /* tCEIL  */
    tSIGN = 312,                   /* tSIGN  */
    tSQRT = 313,                   /* tSQRT  */
    tTRUNC = 314,                  /* tTRUNC  */
    tFABS = 315,                   /* tFABS  */
    tINBLOB = 316,                 /* tINBLOB  */
    tPRINT = 317,                  /* tPRINT  */
    tFPRINTF = 318,                /* tFPRINTF  */
    tSTOP = 319,                   /* tSTOP  */
    tFOPEN = 320,                  /* tFOPEN  */
    tFCLOSE = 321,                 /* tFCLOSE  */
    tFFLUSH = 322,                 /* tFFLUSH  */
    tFSCANF = 323,                 /* tFSCANF  */
    tFUNC = 324,                   /* tFUNC  */
    tPLAY = 325,                   /* tPLAY  */
    tSETDERIVLENGTH = 326,         /* tSETDERIVLENGTH  */
    tGETDERIVLENGTH = 327,         /* tGETDERIVLENGTH  */
    tDISPLAY = 328,                /* tDISPLAY  */
    tVVXMIN = 329,                 /* tVVXMIN  */
    tVVXMAX = 330,                 /* tVVXMAX  */
    tVVYMIN = 331,                 /* tVVYMIN  */
    tVVYMAX = 332,                 /* tVVYMAX  */
    tVVZMIN = 333,                 /* tVVZMIN  */
    tVVZMAX = 334,                 /* tVVZMAX  */
    tVVSCALE = 335,                /* tVVSCALE  */
    tCURVEX = 336,                 /* tCURVEX  */
    tCURVEY = 337,                 /* tCURVEY  */
    tCURVEZ = 338,                 /* tCURVEZ  */
    tCURVEGAL = 339,               /* tCURVEGAL  */
    tARRAYREF = 340,               /* tARRAYREF  */
    tARRAYLHS = 341,               /* tARRAYLHS  */
    tARRAYDEF = 342,               /* tARRAYDEF  */
    tARRAYLVAL = 343,              /* tARRAYLVAL  */
    tSYMBOL = 344,                 /* tSYMBOL  */
    tNAME = 345,                   /* tNAME  */
    tNAMELVAL = 346,               /* tNAMELVAL  */
    tNAMELHS = 347,                /* tNAMELHS  */
    tSTRING = 348,                 /* tSTRING  */
    tVALUE = 349,                  /* tVALUE  */
    tINTEGER = 350,                /* tINTEGER  */
    tASSIGN = 351,                 /* tASSIGN  */
    tQUESTION = 352,               /* tQUESTION  */
    tCOLON = 353,                  /* tCOLON  */
    tOR = 354,                     /* tOR  */
    tAND = 355,                    /* tAND  */
    tEQUAL = 356,                  /* tEQUAL  */
    tNOTEQUAL = 357,               /* tNOTEQUAL  */
    tLT = 358,                     /* tLT  */
    tLE = 359,                     /* tLE  */
    tGE = 360,                     /* tGE  */
    tGT = 361,                     /* tGT  */
    tPLUS = 362,                   /* tPLUS  */
    tMINUS = 363,                  /* tMINUS  */
    tTIMES = 364,                  /* tTIMES  */
    tDIVIDE = 365,                 /* tDIVIDE  */
    tREM = 366,                    /* tREM  */
    tPOW = 367,                    /* tPOW  */
    tUMINUS = 368,                 /* tUMINUS  */
    tADDRESS = 369,                /* tADDRESS  */
    tNOT = 370                     /* tNOT  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define tLSTART 258
#define tRING 259
#define tSEED 260
#define tDLENGTH 261
#define tMDEPTH 262
#define tCONSIDER 263
#define tIGNORE 264
#define tAXIOM 265
#define tEND 266
#define tDEFINE 267
#define tSTARTBLOCK 268
#define tENDBLOCK 269
#define tSTARTEACH 270
#define tENDEACH 271
#define tSTARTHOMO 272
#define tSTARTDECOMP 273
#define tNOWARNING 274
#define tWARNING 275
#define tLSEP 276
#define tRSEP 277
#define tYIELDS 278
#define tOYIELDS 279
#define tNULL 280
#define tEOL 281
#define tIF 282
#define tDO 283
#define tWHILE 284
#define tELSE 285
#define tARRAY 286
#define tEXTERNAL 287
#define tLPAREN 288
#define tRPAREN 289
#define tLBRACE 290
#define tRBRACE 291
#define tCOMMA 292
#define tLBRACKET 293
#define tRBRACKET 294
#define tSEMI 295
#define tATAN2 296
#define tTAN 297
#define tSIN 298
#define tCOS 299
#define tATAN 300
#define tASIN 301
#define tACOS 302
#define tSRAND 303
#define tRAN 304
#define tNRAN 305
#define tBRAN 306
#define tBIRAN 307
#define tEXP 308
#define tLOG 309
#define tFLOOR 310
#define tCEIL 311
#define tSIGN 312
#define tSQRT 313
#define tTRUNC 314
#define tFABS 315
#define tINBLOB 316
#define tPRINT 317
#define tFPRINTF 318
#define tSTOP 319
#define tFOPEN 320
#define tFCLOSE 321
#define tFFLUSH 322
#define tFSCANF 323
#define tFUNC 324
#define tPLAY 325
#define tSETDERIVLENGTH 326
#define tGETDERIVLENGTH 327
#define tDISPLAY 328
#define tVVXMIN 329
#define tVVXMAX 330
#define tVVYMIN 331
#define tVVYMAX 332
#define tVVZMIN 333
#define tVVZMAX 334
#define tVVSCALE 335
#define tCURVEX 336
#define tCURVEY 337
#define tCURVEZ 338
#define tCURVEGAL 339
#define tARRAYREF 340
#define tARRAYLHS 341
#define tARRAYDEF 342
#define tARRAYLVAL 343
#define tSYMBOL 344
#define tNAME 345
#define tNAMELVAL 346
#define tNAMELHS 347
#define tSTRING 348
#define tVALUE 349
#define tINTEGER 350
#define tASSIGN 351
#define tQUESTION 352
#define tCOLON 353
#define tOR 354
#define tAND 355
#define tEQUAL 356
#define tNOTEQUAL 357
#define tLT 358
#define tLE 359
#define tGE 360
#define tGT 361
#define tPLUS 362
#define tMINUS 363
#define tTIMES 364
#define tDIVIDE 365
#define tREM 366
#define tPOW 367
#define tUMINUS 368
#define tADDRESS 369
#define tNOT 370

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 180 "lsys_input.y"

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

#line 311 "lsys_input.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE lsys_inputlval;


int lsys_inputparse (void);


#endif /* !YY_LSYS_INPUT_LSYS_INPUT_TAB_H_INCLUDED  */
