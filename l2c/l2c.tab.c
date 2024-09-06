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
#define yyparse         l2cparse
#define yylex           l2clex
#define yyerror         l2cerror
#define yydebug         l2cdebug
#define yynerrs         l2cnerrs
#define yylval          l2clval
#define yychar          l2cchar

/* First part of user prologue.  */
#line 1 "l2c.y"


#define YYDEBUG 1

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <lparams.h>

#include "module.h"
#include "contextdata.h"
#include "production.h"

extern int l2clex(void);
extern void l2cerror(const char*, ...);
extern void l2cwarning(const char*, ...);
extern void StartGenerateProduce(const char*);
extern void ParameterCast();
extern void EndParameterCast();
extern void EndGenerateProduce();
extern void StartAxiom();
extern void EndAxiom();
extern void StartProduce();
extern void EndProduce();
extern void StartNProduce();
extern void EndNProduce();
extern void ExpandStart();
extern void ExpandStartEach();
extern void ExpandEndEach();
extern void ExpandEnd();
extern void GeneratePred(FormalModuleDtList*, FormalModuleDtList*, FormalModuleDtList*);
extern void StartConsider();
extern void StartIgnore();
extern void EndConsider();
extern void EndIgnore();
extern void StartVerify();
extern void EndVerify();
extern void AppendConIgnModule(const char*);
extern void StartDerivLength();
extern void EndDerivLength();
extern void StartMaxDepth();
extern void EndMaxDepth();
extern void SwitchToInterpretation();
extern void SwitchToDecomposition();
extern void SwitchToProduction();
extern ProductionType ProductionMode();
extern void StartRingLsystem();
extern void EndRingLsystem();
extern void StartGroup(int);
extern void StartGGroup(int);
extern void EndGroup();
extern void StartVGroup(int);
extern void EndVGroup();
extern void StartGProduce();
extern void EndGProduce();
extern void StartPropensity();
extern void GenerateInRightContext(const FormalModuleDtList* pContextModules);
extern void GenerateInLeftContext(FormalModuleDtList* pContextModules);
extern void GenerateInNewRightContext(const FormalModuleDtList* pContextModules);
extern void GenerateInNewLeftContext(FormalModuleDtList* pContextModules);

extern ModuleTable moduleTable;
extern ProductionTable productionTable;
extern HomomorphismTable interpretationTable;
extern HomomorphismTable decompositionTable;

static int counter = __lc_eFirstModuleId;

int ModuleCounter()
{ return counter; }


#line 153 "l2c.tab.c"

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

#include "l2c.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_tAXIOM = 3,                     /* tAXIOM  */
  YYSYMBOL_tMODULE = 4,                    /* tMODULE  */
  YYSYMBOL_tPRODUCE = 5,                   /* tPRODUCE  */
  YYSYMBOL_tNPRODUCE = 6,                  /* tNPRODUCE  */
  YYSYMBOL_tSTART = 7,                     /* tSTART  */
  YYSYMBOL_tSTARTEACH = 8,                 /* tSTARTEACH  */
  YYSYMBOL_tENDEACH = 9,                   /* tENDEACH  */
  YYSYMBOL_tEND = 10,                      /* tEND  */
  YYSYMBOL_tMAXDEPTH = 11,                 /* tMAXDEPTH  */
  YYSYMBOL_tPROPENSITY = 12,               /* tPROPENSITY  */
  YYSYMBOL_tGGROUP = 13,                   /* tGGROUP  */
  YYSYMBOL_tCONSIDER = 14,                 /* tCONSIDER  */
  YYSYMBOL_tIGNORE = 15,                   /* tIGNORE  */
  YYSYMBOL_tDERIVLENGTH = 16,              /* tDERIVLENGTH  */
  YYSYMBOL_tRINGLSYSTEM = 17,              /* tRINGLSYSTEM  */
  YYSYMBOL_tINTERPRETATION = 18,           /* tINTERPRETATION  */
  YYSYMBOL_tDECOMPOSITION = 19,            /* tDECOMPOSITION  */
  YYSYMBOL_tPRODUCTION = 20,               /* tPRODUCTION  */
  YYSYMBOL_tGROUP = 21,                    /* tGROUP  */
  YYSYMBOL_tENDGROUP = 22,                 /* tENDGROUP  */
  YYSYMBOL_tVGROUP = 23,                   /* tVGROUP  */
  YYSYMBOL_tENDVGROUP = 24,                /* tENDVGROUP  */
  YYSYMBOL_tCOLON = 25,                    /* tCOLON  */
  YYSYMBOL_tLPAREN = 26,                   /* tLPAREN  */
  YYSYMBOL_tRPAREN = 27,                   /* tRPAREN  */
  YYSYMBOL_tSEMICOLON = 28,                /* tSEMICOLON  */
  YYSYMBOL_tCOMMA = 29,                    /* tCOMMA  */
  YYSYMBOL_tLESSTHAN = 30,                 /* tLESSTHAN  */
  YYSYMBOL_tLEFTSHIFT = 31,                /* tLEFTSHIFT  */
  YYSYMBOL_tGREATERTHAN = 32,              /* tGREATERTHAN  */
  YYSYMBOL_tRIGHTSHIFT = 33,               /* tRIGHTSHIFT  */
  YYSYMBOL_tENDPRODPROTO = 34,             /* tENDPRODPROTO  */
  YYSYMBOL_tEQUALS = 35,                   /* tEQUALS  */
  YYSYMBOL_tINTEGER = 36,                  /* tINTEGER  */
  YYSYMBOL_tIDENT = 37,                    /* tIDENT  */
  YYSYMBOL_tMODULEIDENT = 38,              /* tMODULEIDENT  */
  YYSYMBOL_tERROR = 39,                    /* tERROR  */
  YYSYMBOL_tVERIFYSTRING = 40,             /* tVERIFYSTRING  */
  YYSYMBOL_tINRIGHTCONTEXT = 41,           /* tINRIGHTCONTEXT  */
  YYSYMBOL_tINLEFTCONTEXT = 42,            /* tINLEFTCONTEXT  */
  YYSYMBOL_tINNEWRIGHTCONTEXT = 43,        /* tINNEWRIGHTCONTEXT  */
  YYSYMBOL_tINNEWLEFTCONTEXT = 44,         /* tINNEWLEFTCONTEXT  */
  YYSYMBOL_YYACCEPT = 45,                  /* $accept  */
  YYSYMBOL_Translate = 46,                 /* Translate  */
  YYSYMBOL_TranslationUnit = 47,           /* TranslationUnit  */
  YYSYMBOL_ModuleDeclaration = 48,         /* ModuleDeclaration  */
  YYSYMBOL_InRightContext = 49,            /* InRightContext  */
  YYSYMBOL_InNewRightContext = 50,         /* InNewRightContext  */
  YYSYMBOL_InLeftContext = 51,             /* InLeftContext  */
  YYSYMBOL_InNewLeftContext = 52,          /* InNewLeftContext  */
  YYSYMBOL_ConsiderStatement = 53,         /* ConsiderStatement  */
  YYSYMBOL_54_1 = 54,                      /* $@1  */
  YYSYMBOL_IgnoreStatement = 55,           /* IgnoreStatement  */
  YYSYMBOL_56_2 = 56,                      /* $@2  */
  YYSYMBOL_VerifyStatement = 57,           /* VerifyStatement  */
  YYSYMBOL_58_3 = 58,                      /* $@3  */
  YYSYMBOL_ModuleList = 59,                /* ModuleList  */
  YYSYMBOL_DerivLength = 60,               /* DerivLength  */
  YYSYMBOL_61_4 = 61,                      /* $@4  */
  YYSYMBOL_RingLsystem = 62,               /* RingLsystem  */
  YYSYMBOL_63_5 = 63,                      /* $@5  */
  YYSYMBOL_MaxDepthStatement = 64,         /* MaxDepthStatement  */
  YYSYMBOL_65_6 = 65,                      /* $@6  */
  YYSYMBOL_ProductionPredecessor = 66,     /* ProductionPredecessor  */
  YYSYMBOL_LeftContext = 67,               /* LeftContext  */
  YYSYMBOL_RightContext = 68,              /* RightContext  */
  YYSYMBOL_StrictPred = 69,                /* StrictPred  */
  YYSYMBOL_FormalModules = 70,             /* FormalModules  */
  YYSYMBOL_FormalModule = 71,              /* FormalModule  */
  YYSYMBOL_AxiomStatement = 72,            /* AxiomStatement  */
  YYSYMBOL_73_7 = 73,                      /* $@7  */
  YYSYMBOL_GProduceStatement = 74,         /* GProduceStatement  */
  YYSYMBOL_75_8 = 75,                      /* $@8  */
  YYSYMBOL_76_9 = 76,                      /* $@9  */
  YYSYMBOL_ProduceStatement = 77,          /* ProduceStatement  */
  YYSYMBOL_78_10 = 78,                     /* $@10  */
  YYSYMBOL_NProduceStatement = 79,         /* NProduceStatement  */
  YYSYMBOL_80_11 = 80,                     /* $@11  */
  YYSYMBOL_ParametricWord = 81,            /* ParametricWord  */
  YYSYMBOL_ParametricLetter = 82,          /* ParametricLetter  */
  YYSYMBOL_83_12 = 83,                     /* $@12  */
  YYSYMBOL_84_13 = 84,                     /* $@13  */
  YYSYMBOL_85_14 = 85,                     /* $@14  */
  YYSYMBOL_CallParameters = 86,            /* CallParameters  */
  YYSYMBOL_87_15 = 87,                     /* $@15  */
  YYSYMBOL_Parameters = 88,                /* Parameters  */
  YYSYMBOL_GroupStart = 89,                /* GroupStart  */
  YYSYMBOL_VGroupStart = 90                /* VGroupStart  */
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
typedef yytype_uint8 yy_state_t;

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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   124

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  45
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  46
/* YYNRULES -- Number of rules.  */
#define YYNRULES  91
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  147

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   299


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
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   109,   109,   110,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   123,   124,   125,   127,   129,   131,   133,
     135,   137,   139,   140,   141,   143,   144,   146,   147,   148,
     149,   152,   158,   165,   171,   180,   186,   192,   198,   204,
     204,   208,   208,   210,   213,   213,   217,   219,   223,   223,
     227,   227,   231,   231,   235,   256,   273,   290,   309,   314,
     321,   326,   333,   339,   343,   349,   353,   358,   358,   362,
     362,   362,   365,   365,   369,   369,   374,   375,   379,   379,
     379,   379,   381,   385,   385,   386,   389,   397,   403,   408,
     410,   414
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
  "\"end of file\"", "error", "\"invalid token\"", "tAXIOM", "tMODULE",
  "tPRODUCE", "tNPRODUCE", "tSTART", "tSTARTEACH", "tENDEACH", "tEND",
  "tMAXDEPTH", "tPROPENSITY", "tGGROUP", "tCONSIDER", "tIGNORE",
  "tDERIVLENGTH", "tRINGLSYSTEM", "tINTERPRETATION", "tDECOMPOSITION",
  "tPRODUCTION", "tGROUP", "tENDGROUP", "tVGROUP", "tENDVGROUP", "tCOLON",
  "tLPAREN", "tRPAREN", "tSEMICOLON", "tCOMMA", "tLESSTHAN", "tLEFTSHIFT",
  "tGREATERTHAN", "tRIGHTSHIFT", "tENDPRODPROTO", "tEQUALS", "tINTEGER",
  "tIDENT", "tMODULEIDENT", "tERROR", "tVERIFYSTRING", "tINRIGHTCONTEXT",
  "tINLEFTCONTEXT", "tINNEWRIGHTCONTEXT", "tINNEWLEFTCONTEXT", "$accept",
  "Translate", "TranslationUnit", "ModuleDeclaration", "InRightContext",
  "InNewRightContext", "InLeftContext", "InNewLeftContext",
  "ConsiderStatement", "$@1", "IgnoreStatement", "$@2", "VerifyStatement",
  "$@3", "ModuleList", "DerivLength", "$@4", "RingLsystem", "$@5",
  "MaxDepthStatement", "$@6", "ProductionPredecessor", "LeftContext",
  "RightContext", "StrictPred", "FormalModules", "FormalModule",
  "AxiomStatement", "$@7", "GProduceStatement", "$@8", "$@9",
  "ProduceStatement", "$@10", "NProduceStatement", "$@11",
  "ParametricWord", "ParametricLetter", "$@12", "$@13", "$@14",
  "CallParameters", "$@15", "Parameters", "GroupStart", "VGroupStart", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-55)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-84)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -55,     1,   -55,   -55,    11,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,    -3,   -55,    21,   -55,   -55,   -55,   -55,
     -55,    56,   -55,    57,   -55,    69,   -55,    70,    71,    72,
      73,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,    50,    -2,    52,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   -55,    51,   -55,   -55,    74,    95,
      76,    65,   -55,    65,    77,    78,    79,    82,    75,    65,
      50,    50,    50,    50,    20,    50,    50,    50,   -55,    80,
     -55,   -55,   -55,    22,    75,   -55,    81,    23,    27,   -55,
     -55,   -55,   -55,    42,    43,   -55,   -55,   -55,   -55,   -55,
      49,    46,    -1,     0,     8,     9,   -55,    84,    50,    50,
     -55,   -55,    83,   -55,    62,    85,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,    86,   -55,   -55,   -55,   -55,   -55,   -55,
      89,    59,   -55,    47,   -55,   -55,   -55,    88,   -55,   -55,
      91,    87,   -55,    93,    92,   -55,   -55
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     1,    67,     0,    72,    74,    15,    16,    17,
      18,    52,    69,     0,    39,    41,    48,    50,    19,    20,
      21,     0,    24,     0,    26,    66,    44,     0,     0,     0,
       0,     2,     4,    27,    29,    28,    30,     5,     6,     7,
      13,    14,    22,     9,     0,     0,    62,    64,     8,    12,
      10,    11,    23,    25,    77,     0,    77,    77,     0,     0,
       0,     0,    43,     0,     0,     0,     0,     0,    88,     0,
       0,     0,     0,     0,     0,    62,     0,     0,    57,     0,
      59,    58,    63,     0,    88,    33,     0,     0,     0,    53,
      70,    90,    46,     0,     0,    49,    51,    89,    91,    87,
       0,     0,     0,     0,     0,     0,    56,     0,    60,    61,
      55,    68,    82,    76,     0,     0,    73,    75,    77,    40,
      47,    42,    65,     0,    45,    35,    37,    36,    38,    54,
       0,     0,    34,     0,    86,    79,    31,     0,    71,    85,
       0,    80,    32,     0,     0,    81,    84
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   -35,   -55,   -55,   -55,   -55,   -55,
     -55,   -55,   -55,    34,    66,    -4,   -46,   -55,   -55,   -55,
     -55,   -55,   -55,   -55,   -55,   -55,   -54,   -55,   -55,   -55,
     -55,   -55,   -55,    38,   -55,   -55
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,     1,    31,    32,    33,    34,    35,    36,    37,    61,
      38,    63,    39,    69,    93,    40,    64,    41,    65,    42,
      58,    43,    44,    79,    45,    46,    47,    48,    54,    49,
      59,   118,    50,    56,    51,    57,    83,   113,   130,   139,
     143,   141,   144,   100,    52,    53
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      82,     2,    87,    88,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,   125,   126,    94,    82,
      76,    77,    78,    60,   101,   127,   128,    25,    25,    25,
      75,    26,    27,    28,    29,    30,    25,    25,    55,    62,
     111,   116,    76,    77,   106,   117,    82,    82,    82,    82,
     112,   112,    82,    82,   133,   112,   102,   103,   104,   105,
     119,   121,   108,   109,   124,   138,   122,    84,   123,    85,
     120,   120,    80,    81,   120,   112,    86,   136,    25,   131,
      25,   123,    66,    67,   137,    68,    70,    71,    72,    73,
      90,    91,    89,    92,    97,    95,    96,    98,   107,   -78,
      74,     0,    99,   132,   110,   135,   -83,   115,   129,   142,
     145,   146,   114,   134,   140
};

static const yytype_int8 yycheck[] =
{
      46,     0,    56,    57,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    27,    27,    63,    75,
      32,    33,    34,    36,    69,    27,    27,    38,    38,    38,
      44,    40,    41,    42,    43,    44,    38,    38,    37,    28,
      28,    28,    32,    33,    34,    28,   102,   103,   104,   105,
      38,    38,   108,   109,   118,    38,    70,    71,    72,    73,
      28,    28,    76,    77,    28,    28,    27,    26,    29,    28,
      38,    38,    30,    31,    38,    38,    35,    28,    38,    27,
      38,    29,    36,    36,    35,    26,    26,    26,    26,    26,
       5,    25,    28,    38,    25,    28,    28,    25,    74,    26,
      44,    -1,    37,    28,    34,    26,    29,    36,    34,    28,
      27,    29,    84,    37,    36
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    46,     0,     3,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    38,    40,    41,    42,    43,
      44,    47,    48,    49,    50,    51,    52,    53,    55,    57,
      60,    62,    64,    66,    67,    69,    70,    71,    72,    74,
      77,    79,    89,    90,    73,    37,    78,    80,    65,    75,
      36,    54,    28,    56,    61,    63,    36,    36,    26,    58,
      26,    26,    26,    26,    69,    70,    32,    33,    34,    68,
      30,    31,    71,    81,    26,    28,    35,    81,    81,    28,
       5,    25,    38,    59,    59,    28,    28,    25,    25,    37,
      88,    59,    70,    70,    70,    70,    34,    68,    70,    70,
      34,    28,    38,    82,    88,    36,    28,    28,    76,    28,
      38,    28,    27,    29,    28,    27,    27,    27,    27,    34,
      83,    27,    28,    81,    37,    26,    28,    35,    28,    84,
      36,    86,    28,    85,    87,    27,    29
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    45,    46,    46,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    47,    47,    47,    47,    47,    47,    47,    47,    47,
      47,    48,    48,    48,    48,    49,    50,    51,    52,    54,
      53,    56,    55,    55,    58,    57,    59,    59,    61,    60,
      63,    62,    65,    64,    66,    66,    66,    66,    67,    67,
      68,    68,    69,    70,    70,    71,    71,    73,    72,    75,
      76,    74,    78,    77,    80,    79,    81,    81,    83,    84,
      85,    82,    82,    87,    86,    86,    88,    88,    88,    89,
      89,    90
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     0,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     6,     8,     3,     5,     4,     4,     4,     4,     0,
       4,     0,     4,     2,     0,     4,     1,     2,     0,     3,
       0,     3,     0,     3,     4,     3,     3,     2,     2,     2,
       2,     2,     1,     2,     1,     4,     1,     0,     4,     0,
       0,     6,     0,     4,     0,     4,     2,     0,     0,     0,
       0,     7,     1,     0,     3,     0,     3,     1,     0,     3,
       3,     3
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
  case 15: /* TranslationUnit: tSTART  */
#line 126 "l2c.y"
        { ExpandStart(); }
#line 1338 "l2c.tab.c"
    break;

  case 16: /* TranslationUnit: tSTARTEACH  */
#line 128 "l2c.y"
        { ExpandStartEach(); }
#line 1344 "l2c.tab.c"
    break;

  case 17: /* TranslationUnit: tENDEACH  */
#line 130 "l2c.y"
        { ExpandEndEach(); }
#line 1350 "l2c.tab.c"
    break;

  case 18: /* TranslationUnit: tEND  */
#line 132 "l2c.y"
        { ExpandEnd(); }
#line 1356 "l2c.tab.c"
    break;

  case 19: /* TranslationUnit: tINTERPRETATION  */
#line 134 "l2c.y"
        { SwitchToInterpretation(); }
#line 1362 "l2c.tab.c"
    break;

  case 20: /* TranslationUnit: tDECOMPOSITION  */
#line 136 "l2c.y"
        { SwitchToDecomposition(); }
#line 1368 "l2c.tab.c"
    break;

  case 21: /* TranslationUnit: tPRODUCTION  */
#line 138 "l2c.y"
        { SwitchToProduction(); }
#line 1374 "l2c.tab.c"
    break;

  case 24: /* TranslationUnit: tENDGROUP  */
#line 142 "l2c.y"
        { EndGroup(); }
#line 1380 "l2c.tab.c"
    break;

  case 26: /* TranslationUnit: tENDVGROUP  */
#line 145 "l2c.y"
        { EndVGroup(); }
#line 1386 "l2c.tab.c"
    break;

  case 31: /* ModuleDeclaration: tMODULE tIDENT tLPAREN Parameters tRPAREN tSEMICOLON  */
#line 153 "l2c.y"
        { 
		ModuleDeclaration mdecl((yyvsp[-4].Ident), &((yyvsp[-2].ParamsList)), counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
#line 1396 "l2c.tab.c"
    break;

  case 32: /* ModuleDeclaration: tMODULE tIDENT tLPAREN Parameters tRPAREN tEQUALS tINTEGER tSEMICOLON  */
#line 159 "l2c.y"
        { 
		counter = (yyvsp[-1].Integer);
		ModuleDeclaration mdecl((yyvsp[-6].Ident), &((yyvsp[-4].ParamsList)), counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
#line 1407 "l2c.tab.c"
    break;

  case 33: /* ModuleDeclaration: tMODULE tIDENT tSEMICOLON  */
#line 166 "l2c.y"
        { 
		ModuleDeclaration mdecl((yyvsp[-1].Ident), NULL, counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
#line 1417 "l2c.tab.c"
    break;

  case 34: /* ModuleDeclaration: tMODULE tIDENT tEQUALS tINTEGER tSEMICOLON  */
#line 172 "l2c.y"
        { 
		counter = (yyvsp[-1].Integer);
		ModuleDeclaration mdecl((yyvsp[-3].Ident), NULL, counter++);
		mdecl.GenerateModId();
		moduleTable.Add(mdecl);
	}
#line 1428 "l2c.tab.c"
    break;

  case 35: /* InRightContext: tINRIGHTCONTEXT tLPAREN FormalModules tRPAREN  */
#line 181 "l2c.y"
        {
		GenerateInRightContext((yyvsp[-1].pFormalModuleDataList));
	}
#line 1436 "l2c.tab.c"
    break;

  case 36: /* InNewRightContext: tINNEWRIGHTCONTEXT tLPAREN FormalModules tRPAREN  */
#line 187 "l2c.y"
        {
		GenerateInNewRightContext((yyvsp[-1].pFormalModuleDataList));
	}
#line 1444 "l2c.tab.c"
    break;

  case 37: /* InLeftContext: tINLEFTCONTEXT tLPAREN FormalModules tRPAREN  */
#line 193 "l2c.y"
        {
		GenerateInLeftContext((yyvsp[-1].pFormalModuleDataList));
	}
#line 1452 "l2c.tab.c"
    break;

  case 38: /* InNewLeftContext: tINNEWLEFTCONTEXT tLPAREN FormalModules tRPAREN  */
#line 199 "l2c.y"
        {
		GenerateInNewLeftContext((yyvsp[-1].pFormalModuleDataList));
	}
#line 1460 "l2c.tab.c"
    break;

  case 39: /* $@1: %empty  */
#line 204 "l2c.y"
                             { StartConsider(); }
#line 1466 "l2c.tab.c"
    break;

  case 40: /* ConsiderStatement: tCONSIDER $@1 ModuleList tSEMICOLON  */
#line 205 "l2c.y"
        { EndConsider(); }
#line 1472 "l2c.tab.c"
    break;

  case 41: /* $@2: %empty  */
#line 208 "l2c.y"
                         { StartIgnore(); }
#line 1478 "l2c.tab.c"
    break;

  case 42: /* IgnoreStatement: tIGNORE $@2 ModuleList tSEMICOLON  */
#line 209 "l2c.y"
        { EndIgnore(); }
#line 1484 "l2c.tab.c"
    break;

  case 43: /* IgnoreStatement: tIGNORE tSEMICOLON  */
#line 210 "l2c.y"
                             { StartIgnore(); EndIgnore(); }
#line 1490 "l2c.tab.c"
    break;

  case 44: /* $@3: %empty  */
#line 213 "l2c.y"
                               { StartVerify(); }
#line 1496 "l2c.tab.c"
    break;

  case 45: /* VerifyStatement: tVERIFYSTRING $@3 ModuleList tSEMICOLON  */
#line 214 "l2c.y"
        { EndVerify(); }
#line 1502 "l2c.tab.c"
    break;

  case 46: /* ModuleList: tMODULEIDENT  */
#line 218 "l2c.y"
        { AppendConIgnModule((yyvsp[0].Ident)); }
#line 1508 "l2c.tab.c"
    break;

  case 47: /* ModuleList: ModuleList tMODULEIDENT  */
#line 220 "l2c.y"
        { AppendConIgnModule((yyvsp[0].Ident)); }
#line 1514 "l2c.tab.c"
    break;

  case 48: /* $@4: %empty  */
#line 223 "l2c.y"
                          { StartDerivLength(); }
#line 1520 "l2c.tab.c"
    break;

  case 49: /* DerivLength: tDERIVLENGTH $@4 tSEMICOLON  */
#line 224 "l2c.y"
        { EndDerivLength(); }
#line 1526 "l2c.tab.c"
    break;

  case 50: /* $@5: %empty  */
#line 227 "l2c.y"
                          { StartRingLsystem(); }
#line 1532 "l2c.tab.c"
    break;

  case 51: /* RingLsystem: tRINGLSYSTEM $@5 tSEMICOLON  */
#line 228 "l2c.y"
        { EndRingLsystem(); }
#line 1538 "l2c.tab.c"
    break;

  case 52: /* $@6: %empty  */
#line 231 "l2c.y"
                             { StartMaxDepth(); }
#line 1544 "l2c.tab.c"
    break;

  case 53: /* MaxDepthStatement: tMAXDEPTH $@6 tSEMICOLON  */
#line 232 "l2c.y"
        { EndMaxDepth(); }
#line 1550 "l2c.tab.c"
    break;

  case 54: /* ProductionPredecessor: LeftContext StrictPred RightContext tENDPRODPROTO  */
#line 236 "l2c.y"
{ 
	if ((yyvsp[-3].context).HasNewContext() && (yyvsp[-1].context).HasNewContext())
		l2cerror("Production cannot have both left new context "
				"and right new context");

	ProductionProto* pNew = new ProductionProto(&((yyvsp[-3].context)), (yyvsp[-2].pFormalModuleDataList), &((yyvsp[-1].context)));
	pNew->Generate();
	switch (ProductionMode())
	{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
	}		
}
#line 1575 "l2c.tab.c"
    break;

  case 55: /* ProductionPredecessor: StrictPred RightContext tENDPRODPROTO  */
#line 257 "l2c.y"
{ 
	ProductionProto* pNew = new ProductionProto(NULL, (yyvsp[-2].pFormalModuleDataList), &((yyvsp[-1].context)));
	pNew->Generate();
	switch (ProductionMode())
	{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
	}
}
#line 1596 "l2c.tab.c"
    break;

  case 56: /* ProductionPredecessor: LeftContext StrictPred tENDPRODPROTO  */
#line 274 "l2c.y"
{ 
	ProductionProto* pNew = new ProductionProto(&((yyvsp[-2].context)), (yyvsp[-1].pFormalModuleDataList), NULL);
	pNew->Generate();
	switch (ProductionMode())
	{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
	}
}
#line 1617 "l2c.tab.c"
    break;

  case 57: /* ProductionPredecessor: StrictPred tENDPRODPROTO  */
#line 291 "l2c.y"
        { 
		ProductionProto* pNew = new ProductionProto(NULL, (yyvsp[-1].pFormalModuleDataList), NULL);
		pNew->Generate();
		switch (ProductionMode())
		{ 
		case eInterpretation :
			interpretationTable.Add(pNew);
			break;
		case eDecomposition :
			decompositionTable.Add(pNew);
			break;
		case eProduction :
			productionTable.Add(pNew);
			break;
		}
	}
#line 1638 "l2c.tab.c"
    break;

  case 58: /* LeftContext: FormalModules tLEFTSHIFT  */
#line 310 "l2c.y"
        { 
		(yyval.context).SetContext(NULL);
		(yyval.context).SetNewContext((yyvsp[-1].pFormalModuleDataList));
	}
#line 1647 "l2c.tab.c"
    break;

  case 59: /* LeftContext: FormalModules tLESSTHAN  */
#line 315 "l2c.y"
        { 
		(yyval.context).SetContext((yyvsp[-1].pFormalModuleDataList));
		(yyval.context).SetNewContext(NULL);
	}
#line 1656 "l2c.tab.c"
    break;

  case 60: /* RightContext: tGREATERTHAN FormalModules  */
#line 322 "l2c.y"
        { 
		(yyval.context).SetContext((yyvsp[0].pFormalModuleDataList));
		(yyval.context).SetNewContext(NULL);
	}
#line 1665 "l2c.tab.c"
    break;

  case 61: /* RightContext: tRIGHTSHIFT FormalModules  */
#line 327 "l2c.y"
        { 
		(yyval.context).SetContext(NULL);
		(yyval.context).SetNewContext((yyvsp[0].pFormalModuleDataList));
	}
#line 1674 "l2c.tab.c"
    break;

  case 62: /* StrictPred: FormalModules  */
#line 334 "l2c.y"
        { 
	(yyval.pFormalModuleDataList) = (yyvsp[0].pFormalModuleDataList);
	}
#line 1682 "l2c.tab.c"
    break;

  case 63: /* FormalModules: FormalModules FormalModule  */
#line 340 "l2c.y"
        { 
		(yyval.pFormalModuleDataList) = (yyvsp[-1].pFormalModuleDataList); (yyval.pFormalModuleDataList)->Add((yyvsp[0].pFormalModuleData)); 
	}
#line 1690 "l2c.tab.c"
    break;

  case 64: /* FormalModules: FormalModule  */
#line 344 "l2c.y"
        { 
		(yyval.pFormalModuleDataList) = new FormalModuleDtList((yyvsp[0].pFormalModuleData)); 
	}
#line 1698 "l2c.tab.c"
    break;

  case 65: /* FormalModule: tMODULEIDENT tLPAREN Parameters tRPAREN  */
#line 350 "l2c.y"
        { 
	(yyval.pFormalModuleData) = new FormalModuleDt((yyvsp[-3].Ident), &((yyvsp[-1].ParamsList))); 
	}
#line 1706 "l2c.tab.c"
    break;

  case 66: /* FormalModule: tMODULEIDENT  */
#line 354 "l2c.y"
        { 
	(yyval.pFormalModuleData) = new FormalModuleDt((yyvsp[0].Ident),NULL);
 	}
#line 1714 "l2c.tab.c"
    break;

  case 67: /* $@7: %empty  */
#line 358 "l2c.y"
                       { StartAxiom(); }
#line 1720 "l2c.tab.c"
    break;

  case 68: /* AxiomStatement: tAXIOM $@7 ParametricWord tSEMICOLON  */
#line 359 "l2c.y"
        { EndAxiom(); }
#line 1726 "l2c.tab.c"
    break;

  case 69: /* $@8: %empty  */
#line 362 "l2c.y"
                               { StartPropensity(); }
#line 1732 "l2c.tab.c"
    break;

  case 70: /* $@9: %empty  */
#line 362 "l2c.y"
                                                               { StartGProduce(); }
#line 1738 "l2c.tab.c"
    break;

  case 71: /* GProduceStatement: tPROPENSITY $@8 tPRODUCE $@9 ParametricWord tSEMICOLON  */
#line 362 "l2c.y"
                                                                                                              { EndGProduce(); }
#line 1744 "l2c.tab.c"
    break;

  case 72: /* $@10: %empty  */
#line 365 "l2c.y"
                           { StartProduce(); }
#line 1750 "l2c.tab.c"
    break;

  case 73: /* ProduceStatement: tPRODUCE $@10 ParametricWord tSEMICOLON  */
#line 366 "l2c.y"
        { EndProduce(); }
#line 1756 "l2c.tab.c"
    break;

  case 74: /* $@11: %empty  */
#line 369 "l2c.y"
                             { StartNProduce(); }
#line 1762 "l2c.tab.c"
    break;

  case 75: /* NProduceStatement: tNPRODUCE $@11 ParametricWord tSEMICOLON  */
#line 370 "l2c.y"
        { EndNProduce(); }
#line 1768 "l2c.tab.c"
    break;

  case 76: /* ParametricWord: ParametricWord ParametricLetter  */
#line 374 "l2c.y"
                                                { }
#line 1774 "l2c.tab.c"
    break;

  case 78: /* $@12: %empty  */
#line 379 "l2c.y"
                               { StartGenerateProduce((yyvsp[0].Ident)); }
#line 1780 "l2c.tab.c"
    break;

  case 79: /* $@13: %empty  */
#line 379 "l2c.y"
                                                                     { ParameterCast(); }
#line 1786 "l2c.tab.c"
    break;

  case 80: /* $@14: %empty  */
#line 379 "l2c.y"
                                                                                                         { EndParameterCast(); }
#line 1792 "l2c.tab.c"
    break;

  case 81: /* ParametricLetter: tMODULEIDENT $@12 tLPAREN $@13 CallParameters $@14 tRPAREN  */
#line 380 "l2c.y"
        { EndGenerateProduce(); }
#line 1798 "l2c.tab.c"
    break;

  case 82: /* ParametricLetter: tMODULEIDENT  */
#line 382 "l2c.y"
        { StartGenerateProduce((yyvsp[0].Ident)); EndGenerateProduce(); }
#line 1804 "l2c.tab.c"
    break;

  case 83: /* $@15: %empty  */
#line 385 "l2c.y"
                               { EndParameterCast(); }
#line 1810 "l2c.tab.c"
    break;

  case 84: /* CallParameters: CallParameters $@15 tCOMMA  */
#line 385 "l2c.y"
                                                              { ParameterCast(); }
#line 1816 "l2c.tab.c"
    break;

  case 86: /* Parameters: Parameters tCOMMA tIDENT  */
#line 390 "l2c.y"
        { 
	  if ((yyvsp[-2].ParamsList).count == __lc_eMaxParams)
	    l2cerror("Too many parameters");
	  strcpy((yyvsp[-2].ParamsList).Params[(yyvsp[-2].ParamsList).count], (yyvsp[0].Ident));
	  (yyvsp[-2].ParamsList).count++;
	  (yyval.ParamsList) = (yyvsp[-2].ParamsList);
	}
#line 1828 "l2c.tab.c"
    break;

  case 87: /* Parameters: tIDENT  */
#line 398 "l2c.y"
        { 
    strcpy((yyval.ParamsList).Params[0], (yyvsp[0].Ident));
	  (yyval.ParamsList).count = 1;
	}
#line 1837 "l2c.tab.c"
    break;

  case 88: /* Parameters: %empty  */
#line 403 "l2c.y"
        { 
	(yyval.ParamsList).count = 0; 
}
#line 1845 "l2c.tab.c"
    break;

  case 89: /* GroupStart: tGROUP tINTEGER tCOLON  */
#line 409 "l2c.y"
        { StartGroup((yyvsp[-1].Integer)); }
#line 1851 "l2c.tab.c"
    break;

  case 90: /* GroupStart: tGGROUP tINTEGER tCOLON  */
#line 411 "l2c.y"
        { StartGGroup((yyvsp[-1].Integer)); }
#line 1857 "l2c.tab.c"
    break;

  case 91: /* VGroupStart: tVGROUP tINTEGER tCOLON  */
#line 415 "l2c.y"
        { StartVGroup((yyvsp[-1].Integer)); }
#line 1863 "l2c.tab.c"
    break;


#line 1867 "l2c.tab.c"

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

#line 419 "l2c.y"

