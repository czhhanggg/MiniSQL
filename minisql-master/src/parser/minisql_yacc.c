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




/* First part of user prologue.  */
#line 1 "minisql.y"

  #include <stdio.h>
  #include "parser/parser.h"

  extern char *yytext;
  extern int yylex(void);
  int yyerror(char* error);

#line 80 "./minisql_yacc.c"

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

#include "minisql_yacc.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_CREATE = 3,                     /* CREATE  */
  YYSYMBOL_DROP = 4,                       /* DROP  */
  YYSYMBOL_SELECT = 5,                     /* SELECT  */
  YYSYMBOL_INSERT = 6,                     /* INSERT  */
  YYSYMBOL_DELETE = 7,                     /* DELETE  */
  YYSYMBOL_UPDATE = 8,                     /* UPDATE  */
  YYSYMBOL_TRXBEGIN = 9,                   /* TRXBEGIN  */
  YYSYMBOL_TRXCOMMIT = 10,                 /* TRXCOMMIT  */
  YYSYMBOL_TRXROLLBACK = 11,               /* TRXROLLBACK  */
  YYSYMBOL_QUIT = 12,                      /* QUIT  */
  YYSYMBOL_EXECFILE = 13,                  /* EXECFILE  */
  YYSYMBOL_SHOW = 14,                      /* SHOW  */
  YYSYMBOL_USE = 15,                       /* USE  */
  YYSYMBOL_USING = 16,                     /* USING  */
  YYSYMBOL_DATABASE = 17,                  /* DATABASE  */
  YYSYMBOL_DATABASES = 18,                 /* DATABASES  */
  YYSYMBOL_TABLE = 19,                     /* TABLE  */
  YYSYMBOL_TABLES = 20,                    /* TABLES  */
  YYSYMBOL_INDEX = 21,                     /* INDEX  */
  YYSYMBOL_INDEXES = 22,                   /* INDEXES  */
  YYSYMBOL_ON = 23,                        /* ON  */
  YYSYMBOL_FROM = 24,                      /* FROM  */
  YYSYMBOL_WHERE = 25,                     /* WHERE  */
  YYSYMBOL_INTO = 26,                      /* INTO  */
  YYSYMBOL_SET = 27,                       /* SET  */
  YYSYMBOL_VALUES = 28,                    /* VALUES  */
  YYSYMBOL_PRIMARY = 29,                   /* PRIMARY  */
  YYSYMBOL_KEY = 30,                       /* KEY  */
  YYSYMBOL_UNIQUE = 31,                    /* UNIQUE  */
  YYSYMBOL_CHAR = 32,                      /* CHAR  */
  YYSYMBOL_INT = 33,                       /* INT  */
  YYSYMBOL_FLOAT = 34,                     /* FLOAT  */
  YYSYMBOL_AND = 35,                       /* AND  */
  YYSYMBOL_OR = 36,                        /* OR  */
  YYSYMBOL_NOT = 37,                       /* NOT  */
  YYSYMBOL_IS = 38,                        /* IS  */
  YYSYMBOL_FLAGNULL = 39,                  /* FLAGNULL  */
  YYSYMBOL_IDENTIFIER = 40,                /* IDENTIFIER  */
  YYSYMBOL_STRING = 41,                    /* STRING  */
  YYSYMBOL_NUMBER = 42,                    /* NUMBER  */
  YYSYMBOL_EQ = 43,                        /* EQ  */
  YYSYMBOL_NE = 44,                        /* NE  */
  YYSYMBOL_LE = 45,                        /* LE  */
  YYSYMBOL_GE = 46,                        /* GE  */
  YYSYMBOL_47_ = 47,                       /* ';'  */
  YYSYMBOL_48_ = 48,                       /* '('  */
  YYSYMBOL_49_ = 49,                       /* ')'  */
  YYSYMBOL_50_ = 50,                       /* ','  */
  YYSYMBOL_51_ = 51,                       /* '*'  */
  YYSYMBOL_52_ = 52,                       /* '<'  */
  YYSYMBOL_53_ = 53,                       /* '>'  */
  YYSYMBOL_YYACCEPT = 54,                  /* $accept  */
  YYSYMBOL_start = 55,                     /* start  */
  YYSYMBOL_sql = 56,                       /* sql  */
  YYSYMBOL_sql_create_database = 57,       /* sql_create_database  */
  YYSYMBOL_sql_drop_database = 58,         /* sql_drop_database  */
  YYSYMBOL_sql_show_databases = 59,        /* sql_show_databases  */
  YYSYMBOL_sql_use_database = 60,          /* sql_use_database  */
  YYSYMBOL_sql_show_tables = 61,           /* sql_show_tables  */
  YYSYMBOL_sql_create_table = 62,          /* sql_create_table  */
  YYSYMBOL_column_list = 63,               /* column_list  */
  YYSYMBOL_column_definition_list = 64,    /* column_definition_list  */
  YYSYMBOL_column_definition = 65,         /* column_definition  */
  YYSYMBOL_column_type = 66,               /* column_type  */
  YYSYMBOL_sql_drop_table = 67,            /* sql_drop_table  */
  YYSYMBOL_sql_create_index = 68,          /* sql_create_index  */
  YYSYMBOL_sql_drop_index = 69,            /* sql_drop_index  */
  YYSYMBOL_sql_show_indexes = 70,          /* sql_show_indexes  */
  YYSYMBOL_sql_select = 71,                /* sql_select  */
  YYSYMBOL_select_columns = 72,            /* select_columns  */
  YYSYMBOL_where_conditions = 73,          /* where_conditions  */
  YYSYMBOL_connector = 74,                 /* connector  */
  YYSYMBOL_where_condition = 75,           /* where_condition  */
  YYSYMBOL_column_value = 76,              /* column_value  */
  YYSYMBOL_operator = 77,                  /* operator  */
  YYSYMBOL_sql_insert = 78,                /* sql_insert  */
  YYSYMBOL_insert_value_lists = 79,        /* insert_value_lists  */
  YYSYMBOL_column_values = 80,             /* column_values  */
  YYSYMBOL_sql_delete = 81,                /* sql_delete  */
  YYSYMBOL_sql_update = 82,                /* sql_update  */
  YYSYMBOL_update_values = 83,             /* update_values  */
  YYSYMBOL_update_value = 84,              /* update_value  */
  YYSYMBOL_sql_trx_begin = 85,             /* sql_trx_begin  */
  YYSYMBOL_sql_trx_commit = 86,            /* sql_trx_commit  */
  YYSYMBOL_sql_trx_rollback = 87,          /* sql_trx_rollback  */
  YYSYMBOL_sql_quit = 88,                  /* sql_quit  */
  YYSYMBOL_sql_exec_file = 89              /* sql_exec_file  */
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
#define YYFINAL  53
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   108

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  36
/* YYNRULES -- Number of rules.  */
#define YYNRULES  79
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  137

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   301


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
      48,    49,    51,     2,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    47,
      52,     2,    53,     2,     2,     2,     2,     2,     2,     2,
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
      45,    46
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    36,    36,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    65,    72,    79,    85,    92,    98,   108,   112,
     118,   122,   125,   132,   137,   145,   148,   151,   158,   165,
     173,   187,   194,   200,   205,   216,   219,   226,   231,   237,
     240,   246,   254,   257,   260,   266,   269,   272,   275,   278,
     281,   284,   287,   293,   301,   307,   315,   319,   325,   329,
     339,   346,   361,   365,   371,   379,   385,   391,   397,   403
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
  "\"end of file\"", "error", "\"invalid token\"", "CREATE", "DROP",
  "SELECT", "INSERT", "DELETE", "UPDATE", "TRXBEGIN", "TRXCOMMIT",
  "TRXROLLBACK", "QUIT", "EXECFILE", "SHOW", "USE", "USING", "DATABASE",
  "DATABASES", "TABLE", "TABLES", "INDEX", "INDEXES", "ON", "FROM",
  "WHERE", "INTO", "SET", "VALUES", "PRIMARY", "KEY", "UNIQUE", "CHAR",
  "INT", "FLOAT", "AND", "OR", "NOT", "IS", "FLAGNULL", "IDENTIFIER",
  "STRING", "NUMBER", "EQ", "NE", "LE", "GE", "';'", "'('", "')'", "','",
  "'*'", "'<'", "'>'", "$accept", "start", "sql", "sql_create_database",
  "sql_drop_database", "sql_show_databases", "sql_use_database",
  "sql_show_tables", "sql_create_table", "column_list",
  "column_definition_list", "column_definition", "column_type",
  "sql_drop_table", "sql_create_index", "sql_drop_index",
  "sql_show_indexes", "sql_select", "select_columns", "where_conditions",
  "connector", "where_condition", "column_value", "operator", "sql_insert",
  "insert_value_lists", "column_values", "sql_delete", "sql_update",
  "update_values", "update_value", "sql_trx_begin", "sql_trx_commit",
  "sql_trx_rollback", "sql_quit", "sql_exec_file", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-86)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      33,     2,     3,   -36,   -19,   -12,   -26,   -86,   -86,   -86,
     -86,    10,     7,    12,    53,     8,   -86,   -86,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,    16,    18,    19,    20,    21,
      22,     4,   -86,   -86,    39,    24,    25,    40,   -86,   -86,
     -86,   -86,   -86,   -86,   -86,   -86,    23,    43,   -86,   -86,
     -86,    28,    29,    42,    47,    34,   -24,    35,   -86,    48,
      30,    36,    37,    52,    31,    49,     0,    38,    32,    41,
      36,   -11,   -86,   -35,    14,   -86,   -11,    36,    34,    44,
      45,   -86,   -86,    54,   -86,   -24,    28,    14,   -86,   -86,
     -86,    46,    50,   -86,   -86,   -86,   -86,   -86,   -86,   -86,
     -86,   -11,   -86,   -86,    36,   -86,    14,   -86,    28,    55,
     -86,   -86,    51,   -11,    56,   -86,   -86,    58,    59,    67,
     -86,    30,   -86,   -86,    61,   -86,   -86
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,    75,    76,    77,
      78,     0,     0,     0,     0,     0,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,     0,     0,     0,     0,     0,
       0,    29,    45,    46,     0,     0,     0,     0,    79,    24,
      26,    42,    25,     1,     2,    22,     0,     0,    23,    38,
      41,     0,     0,     0,    68,     0,     0,     0,    28,    43,
       0,     0,     0,    70,    73,     0,     0,     0,    31,     0,
       0,     0,    63,     0,    69,    48,     0,     0,     0,     0,
       0,    35,    36,    34,    27,     0,     0,    44,    54,    52,
      53,    67,     0,    62,    61,    55,    56,    57,    58,    59,
      60,     0,    49,    50,     0,    74,    71,    72,     0,     0,
      33,    30,     0,     0,    65,    51,    47,     0,     0,    39,
      66,     0,    32,    37,     0,    64,    40
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -61,
      -9,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -86,   -74,
     -86,   -30,   -85,   -86,   -86,   -43,   -33,   -86,   -86,     6,
     -86,   -86,   -86,   -86,   -86,   -86
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    14,    15,    16,    17,    18,    19,    20,    21,    43,
      77,    78,    93,    22,    23,    24,    25,    26,    44,    84,
     114,    85,   101,   111,    27,    82,   102,    28,    29,    73,
      74,    30,    31,    32,    33,    34
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      68,   115,   103,   104,    41,    75,    97,    45,   105,   106,
     107,   108,    46,   116,    47,    42,    76,   109,   110,    35,
      38,    36,    39,    37,    40,    49,   125,    50,    98,    51,
      99,   100,    90,    91,    92,   122,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,   112,
     113,    48,    52,    53,    61,    54,    55,   127,    56,    57,
      58,    59,    60,    62,    63,    64,    67,    65,    41,    69,
      70,    66,    71,    80,    72,    79,    83,    87,    81,    89,
      86,    88,    95,   134,   126,   120,   121,    94,   135,    96,
     130,     0,   118,   119,   117,     0,   123,   128,     0,   124,
     129,   136,     0,     0,     0,     0,   131,   132,   133
};

static const yytype_int16 yycheck[] =
{
      61,    86,    37,    38,    40,    29,    80,    26,    43,    44,
      45,    46,    24,    87,    40,    51,    40,    52,    53,    17,
      17,    19,    19,    21,    21,    18,   111,    20,    39,    22,
      41,    42,    32,    33,    34,    96,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    35,
      36,    41,    40,     0,    50,    47,    40,   118,    40,    40,
      40,    40,    40,    24,    40,    40,    23,    27,    40,    40,
      28,    48,    25,    25,    40,    40,    40,    25,    48,    30,
      43,    50,    50,    16,   114,    31,    95,    49,   131,    48,
     123,    -1,    48,    48,    88,    -1,    50,    42,    -1,    49,
      49,    40,    -1,    -1,    -1,    -1,    50,    49,    49
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    55,    56,    57,    58,    59,    60,
      61,    62,    67,    68,    69,    70,    71,    78,    81,    82,
      85,    86,    87,    88,    89,    17,    19,    21,    17,    19,
      21,    40,    51,    63,    72,    26,    24,    40,    41,    18,
      20,    22,    40,     0,    47,    40,    40,    40,    40,    40,
      40,    50,    24,    40,    40,    27,    48,    23,    63,    40,
      28,    25,    40,    83,    84,    29,    40,    64,    65,    40,
      25,    48,    79,    40,    73,    75,    43,    25,    50,    30,
      32,    33,    34,    66,    49,    50,    48,    73,    39,    41,
      42,    76,    80,    37,    38,    43,    44,    45,    46,    52,
      53,    77,    35,    36,    74,    76,    73,    83,    48,    48,
      31,    64,    63,    50,    49,    76,    75,    63,    42,    49,
      80,    50,    49,    49,    16,    79,    40
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    54,    55,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    56,    56,    56,    56,    56,    56,    56,    56,
      56,    56,    57,    58,    59,    60,    61,    62,    63,    63,
      64,    64,    64,    65,    65,    66,    66,    66,    67,    68,
      68,    69,    70,    71,    71,    72,    72,    73,    73,    74,
      74,    75,    76,    76,    76,    77,    77,    77,    77,    77,
      77,    77,    77,    78,    79,    79,    80,    80,    81,    81,
      82,    82,    83,    83,    84,    85,    86,    87,    88,    89
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     3,     2,     2,     2,     6,     3,     1,
       3,     1,     5,     3,     2,     1,     1,     4,     3,     8,
      10,     3,     2,     4,     6,     1,     1,     3,     1,     1,
       1,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     5,     5,     3,     3,     1,     3,     5,
       4,     6,     3,     1,     3,     1,     1,     1,     1,     2
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
  case 2: /* start: sql ';'  */
#line 36 "minisql.y"
          {
    (yyval.syntax_node) = (yyvsp[-1].syntax_node);
    MinisqlParserSetRoot((yyval.syntax_node));
  }
#line 1251 "./minisql_yacc.c"
    break;

  case 3: /* sql: sql_create_database  */
#line 43 "minisql.y"
                      { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1257 "./minisql_yacc.c"
    break;

  case 4: /* sql: sql_drop_database  */
#line 44 "minisql.y"
                      { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1263 "./minisql_yacc.c"
    break;

  case 5: /* sql: sql_show_databases  */
#line 45 "minisql.y"
                       { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1269 "./minisql_yacc.c"
    break;

  case 6: /* sql: sql_use_database  */
#line 46 "minisql.y"
                     { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1275 "./minisql_yacc.c"
    break;

  case 7: /* sql: sql_show_tables  */
#line 47 "minisql.y"
                    { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1281 "./minisql_yacc.c"
    break;

  case 8: /* sql: sql_create_table  */
#line 48 "minisql.y"
                     { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1287 "./minisql_yacc.c"
    break;

  case 9: /* sql: sql_drop_table  */
#line 49 "minisql.y"
                   { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1293 "./minisql_yacc.c"
    break;

  case 10: /* sql: sql_create_index  */
#line 50 "minisql.y"
                     { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1299 "./minisql_yacc.c"
    break;

  case 11: /* sql: sql_drop_index  */
#line 51 "minisql.y"
                   { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1305 "./minisql_yacc.c"
    break;

  case 12: /* sql: sql_show_indexes  */
#line 52 "minisql.y"
                     { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1311 "./minisql_yacc.c"
    break;

  case 13: /* sql: sql_select  */
#line 53 "minisql.y"
               { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1317 "./minisql_yacc.c"
    break;

  case 14: /* sql: sql_insert  */
#line 54 "minisql.y"
               { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1323 "./minisql_yacc.c"
    break;

  case 15: /* sql: sql_delete  */
#line 55 "minisql.y"
               { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1329 "./minisql_yacc.c"
    break;

  case 16: /* sql: sql_update  */
#line 56 "minisql.y"
               { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1335 "./minisql_yacc.c"
    break;

  case 17: /* sql: sql_trx_begin  */
#line 57 "minisql.y"
                  { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1341 "./minisql_yacc.c"
    break;

  case 18: /* sql: sql_trx_commit  */
#line 58 "minisql.y"
                   { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1347 "./minisql_yacc.c"
    break;

  case 19: /* sql: sql_trx_rollback  */
#line 59 "minisql.y"
                     { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1353 "./minisql_yacc.c"
    break;

  case 20: /* sql: sql_quit  */
#line 60 "minisql.y"
             { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1359 "./minisql_yacc.c"
    break;

  case 21: /* sql: sql_exec_file  */
#line 61 "minisql.y"
                  { (yyval.syntax_node) = (yyvsp[0].syntax_node); }
#line 1365 "./minisql_yacc.c"
    break;

  case 22: /* sql_create_database: CREATE DATABASE IDENTIFIER  */
#line 65 "minisql.y"
                             {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCreateDB, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1374 "./minisql_yacc.c"
    break;

  case 23: /* sql_drop_database: DROP DATABASE IDENTIFIER  */
#line 72 "minisql.y"
                           {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeDropDB, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1383 "./minisql_yacc.c"
    break;

  case 24: /* sql_show_databases: SHOW DATABASES  */
#line 79 "minisql.y"
                 {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeShowDB, NULL);
  }
#line 1391 "./minisql_yacc.c"
    break;

  case 25: /* sql_use_database: USE IDENTIFIER  */
#line 85 "minisql.y"
                 {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeUseDB, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1400 "./minisql_yacc.c"
    break;

  case 26: /* sql_show_tables: SHOW TABLES  */
#line 92 "minisql.y"
              {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeShowTables, NULL);
  }
#line 1408 "./minisql_yacc.c"
    break;

  case 27: /* sql_create_table: CREATE TABLE IDENTIFIER '(' column_definition_list ')'  */
#line 98 "minisql.y"
                                                         {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCreateTable, NULL);
    pSyntaxNode list_node = CreateSyntaxNode(kNodeColumnDefinitionList, NULL);
    SyntaxNodeAddChildren(list_node, (yyvsp[-1].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-3].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), list_node);
  }
#line 1420 "./minisql_yacc.c"
    break;

  case 28: /* column_list: IDENTIFIER ',' column_list  */
#line 108 "minisql.y"
                             {
    (yyval.syntax_node) = (yyvsp[-2].syntax_node);
    SyntaxNodeAddSibling((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1429 "./minisql_yacc.c"
    break;

  case 29: /* column_list: IDENTIFIER  */
#line 112 "minisql.y"
               {
    (yyval.syntax_node) = (yyvsp[0].syntax_node);
  }
#line 1437 "./minisql_yacc.c"
    break;

  case 30: /* column_definition_list: column_definition ',' column_definition_list  */
#line 118 "minisql.y"
                                               {
    (yyval.syntax_node) = (yyvsp[-2].syntax_node);
    SyntaxNodeAddSibling((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1446 "./minisql_yacc.c"
    break;

  case 31: /* column_definition_list: column_definition  */
#line 122 "minisql.y"
                      {
    (yyval.syntax_node) = (yyvsp[0].syntax_node);
  }
#line 1454 "./minisql_yacc.c"
    break;

  case 32: /* column_definition_list: PRIMARY KEY '(' column_list ')'  */
#line 125 "minisql.y"
                                    {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeColumnList, "primary keys");
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-1].syntax_node));
  }
#line 1463 "./minisql_yacc.c"
    break;

  case 33: /* column_definition: IDENTIFIER column_type UNIQUE  */
#line 132 "minisql.y"
                                {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeColumnDefinition, "unique");
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-1].syntax_node));
  }
#line 1473 "./minisql_yacc.c"
    break;

  case 34: /* column_definition: IDENTIFIER column_type  */
#line 137 "minisql.y"
                           {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeColumnDefinition, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-1].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1483 "./minisql_yacc.c"
    break;

  case 35: /* column_type: INT  */
#line 145 "minisql.y"
      {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeColumnType, "int");
  }
#line 1491 "./minisql_yacc.c"
    break;

  case 36: /* column_type: FLOAT  */
#line 148 "minisql.y"
          {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeColumnType, "float");
  }
#line 1499 "./minisql_yacc.c"
    break;

  case 37: /* column_type: CHAR '(' NUMBER ')'  */
#line 151 "minisql.y"
                        {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeColumnType, "char");
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-1].syntax_node));
  }
#line 1508 "./minisql_yacc.c"
    break;

  case 38: /* sql_drop_table: DROP TABLE IDENTIFIER  */
#line 158 "minisql.y"
                        {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeDropTable, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1517 "./minisql_yacc.c"
    break;

  case 39: /* sql_create_index: CREATE INDEX IDENTIFIER ON IDENTIFIER '(' column_list ')'  */
#line 165 "minisql.y"
                                                            {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCreateIndex, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-5].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-3].syntax_node));
    pSyntaxNode index_keys_node = CreateSyntaxNode(kNodeColumnList, "index keys");
    SyntaxNodeAddChildren(index_keys_node, (yyvsp[-1].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), index_keys_node);
  }
#line 1530 "./minisql_yacc.c"
    break;

  case 40: /* sql_create_index: CREATE INDEX IDENTIFIER ON IDENTIFIER '(' column_list ')' USING IDENTIFIER  */
#line 173 "minisql.y"
                                                                               {
      (yyval.syntax_node) = CreateSyntaxNode(kNodeCreateIndex, NULL);
      SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-7].syntax_node));
      SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-5].syntax_node));
      pSyntaxNode index_keys_node = CreateSyntaxNode(kNodeColumnList, "index keys");
      SyntaxNodeAddChildren(index_keys_node, (yyvsp[-3].syntax_node));
      SyntaxNodeAddChildren((yyval.syntax_node), index_keys_node);
      pSyntaxNode index_type_node = CreateSyntaxNode(kNodeIndexType, "index type");
      SyntaxNodeAddChildren(index_type_node, (yyvsp[0].syntax_node));
      SyntaxNodeAddChildren((yyval.syntax_node), index_type_node);
  }
#line 1546 "./minisql_yacc.c"
    break;

  case 41: /* sql_drop_index: DROP INDEX IDENTIFIER  */
#line 187 "minisql.y"
                        {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeDropIndex, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1555 "./minisql_yacc.c"
    break;

  case 42: /* sql_show_indexes: SHOW INDEXES  */
#line 194 "minisql.y"
               {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeShowIndexes, NULL);
  }
#line 1563 "./minisql_yacc.c"
    break;

  case 43: /* sql_select: SELECT select_columns FROM IDENTIFIER  */
#line 200 "minisql.y"
                                        {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeSelect, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1573 "./minisql_yacc.c"
    break;

  case 44: /* sql_select: SELECT select_columns FROM IDENTIFIER WHERE where_conditions  */
#line 205 "minisql.y"
                                                                 {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeSelect, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-4].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    pSyntaxNode condition_node = CreateSyntaxNode(kNodeConditions, NULL);
    SyntaxNodeAddChildren(condition_node, (yyvsp[0].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), condition_node);
  }
#line 1586 "./minisql_yacc.c"
    break;

  case 45: /* select_columns: '*'  */
#line 216 "minisql.y"
      {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeAllColumns, NULL);
  }
#line 1594 "./minisql_yacc.c"
    break;

  case 46: /* select_columns: column_list  */
#line 219 "minisql.y"
                {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeColumnList, "select columns");
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1603 "./minisql_yacc.c"
    break;

  case 47: /* where_conditions: where_conditions connector where_condition  */
#line 226 "minisql.y"
                                              {
    (yyval.syntax_node) = (yyvsp[-1].syntax_node);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1613 "./minisql_yacc.c"
    break;

  case 48: /* where_conditions: where_condition  */
#line 231 "minisql.y"
                    {
    (yyval.syntax_node) = (yyvsp[0].syntax_node);
  }
#line 1621 "./minisql_yacc.c"
    break;

  case 49: /* connector: AND  */
#line 237 "minisql.y"
      {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeConnector, "and");
  }
#line 1629 "./minisql_yacc.c"
    break;

  case 50: /* connector: OR  */
#line 240 "minisql.y"
       {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeConnector, "or");
  }
#line 1637 "./minisql_yacc.c"
    break;

  case 51: /* where_condition: IDENTIFIER operator column_value  */
#line 246 "minisql.y"
                                   {
    (yyval.syntax_node) = (yyvsp[-1].syntax_node);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1647 "./minisql_yacc.c"
    break;

  case 52: /* column_value: STRING  */
#line 254 "minisql.y"
         {
    (yyval.syntax_node) = (yyvsp[0].syntax_node);
  }
#line 1655 "./minisql_yacc.c"
    break;

  case 53: /* column_value: NUMBER  */
#line 257 "minisql.y"
           {
    (yyval.syntax_node) = (yyvsp[0].syntax_node);
  }
#line 1663 "./minisql_yacc.c"
    break;

  case 54: /* column_value: FLAGNULL  */
#line 260 "minisql.y"
             {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeNull, NULL);
  }
#line 1671 "./minisql_yacc.c"
    break;

  case 55: /* operator: EQ  */
#line 266 "minisql.y"
     {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, "=");
  }
#line 1679 "./minisql_yacc.c"
    break;

  case 56: /* operator: NE  */
#line 269 "minisql.y"
       {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, "<>");
  }
#line 1687 "./minisql_yacc.c"
    break;

  case 57: /* operator: LE  */
#line 272 "minisql.y"
       {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, "<=");
  }
#line 1695 "./minisql_yacc.c"
    break;

  case 58: /* operator: GE  */
#line 275 "minisql.y"
       {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, ">=");
  }
#line 1703 "./minisql_yacc.c"
    break;

  case 59: /* operator: '<'  */
#line 278 "minisql.y"
        {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, "<");
  }
#line 1711 "./minisql_yacc.c"
    break;

  case 60: /* operator: '>'  */
#line 281 "minisql.y"
        {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, ">");
  }
#line 1719 "./minisql_yacc.c"
    break;

  case 61: /* operator: IS  */
#line 284 "minisql.y"
       {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, "is");
  }
#line 1727 "./minisql_yacc.c"
    break;

  case 62: /* operator: NOT  */
#line 287 "minisql.y"
        {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeCompareOperator, "not");
  }
#line 1735 "./minisql_yacc.c"
    break;

  case 63: /* sql_insert: INSERT INTO IDENTIFIER VALUES insert_value_lists  */
#line 293 "minisql.y"
                                                   {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeInsert, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1745 "./minisql_yacc.c"
    break;

  case 64: /* insert_value_lists: '(' column_values ')' ',' insert_value_lists  */
#line 301 "minisql.y"
                                               {
    pSyntaxNode tuple_node = CreateSyntaxNode(kNodeColumnValues, NULL);
    SyntaxNodeAddChildren(tuple_node, (yyvsp[-3].syntax_node));
    SyntaxNodeAddSibling(tuple_node, (yyvsp[0].syntax_node));
    (yyval.syntax_node) = tuple_node;
  }
#line 1756 "./minisql_yacc.c"
    break;

  case 65: /* insert_value_lists: '(' column_values ')'  */
#line 307 "minisql.y"
                          {
    pSyntaxNode tuple_node = CreateSyntaxNode(kNodeColumnValues, NULL);
    SyntaxNodeAddChildren(tuple_node, (yyvsp[-1].syntax_node));
    (yyval.syntax_node) = tuple_node;
  }
#line 1766 "./minisql_yacc.c"
    break;

  case 66: /* column_values: column_value ',' column_values  */
#line 315 "minisql.y"
                                 {
    (yyval.syntax_node) = (yyvsp[-2].syntax_node);
    SyntaxNodeAddSibling((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1775 "./minisql_yacc.c"
    break;

  case 67: /* column_values: column_value  */
#line 319 "minisql.y"
                 {
    (yyval.syntax_node) = (yyvsp[0].syntax_node);
  }
#line 1783 "./minisql_yacc.c"
    break;

  case 68: /* sql_delete: DELETE FROM IDENTIFIER  */
#line 325 "minisql.y"
                         {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeDelete, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1792 "./minisql_yacc.c"
    break;

  case 69: /* sql_delete: DELETE FROM IDENTIFIER WHERE where_conditions  */
#line 329 "minisql.y"
                                                  {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeDelete, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    pSyntaxNode condition_node = CreateSyntaxNode(kNodeConditions, NULL);
    SyntaxNodeAddChildren(condition_node, (yyvsp[0].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), condition_node);
  }
#line 1804 "./minisql_yacc.c"
    break;

  case 70: /* sql_update: UPDATE IDENTIFIER SET update_values  */
#line 339 "minisql.y"
                                      {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeUpdate, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    pSyntaxNode upd_values_node = CreateSyntaxNode(kNodeUpdateValues, NULL);
    SyntaxNodeAddChildren(upd_values_node, (yyvsp[0].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), upd_values_node);
  }
#line 1816 "./minisql_yacc.c"
    break;

  case 71: /* sql_update: UPDATE IDENTIFIER SET update_values WHERE where_conditions  */
#line 346 "minisql.y"
                                                               {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeUpdate, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-4].syntax_node));
    // update values
    pSyntaxNode upd_values_node = CreateSyntaxNode(kNodeUpdateValues, NULL);
    SyntaxNodeAddChildren(upd_values_node, (yyvsp[-2].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), upd_values_node);
    // where conditions
    pSyntaxNode condition_node = CreateSyntaxNode(kNodeConditions, NULL);
    SyntaxNodeAddChildren(condition_node, (yyvsp[0].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), condition_node);
  }
#line 1833 "./minisql_yacc.c"
    break;

  case 72: /* update_values: update_value ',' update_values  */
#line 361 "minisql.y"
                                 {
    (yyval.syntax_node) = (yyvsp[-2].syntax_node);
    SyntaxNodeAddSibling((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1842 "./minisql_yacc.c"
    break;

  case 73: /* update_values: update_value  */
#line 365 "minisql.y"
                 {
    (yyval.syntax_node) = (yyvsp[0].syntax_node);
  }
#line 1850 "./minisql_yacc.c"
    break;

  case 74: /* update_value: IDENTIFIER EQ column_value  */
#line 371 "minisql.y"
                             {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeUpdateValue, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[-2].syntax_node));
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1860 "./minisql_yacc.c"
    break;

  case 75: /* sql_trx_begin: TRXBEGIN  */
#line 379 "minisql.y"
           {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeTrxBegin, NULL);
  }
#line 1868 "./minisql_yacc.c"
    break;

  case 76: /* sql_trx_commit: TRXCOMMIT  */
#line 385 "minisql.y"
            {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeTrxCommit, NULL);
  }
#line 1876 "./minisql_yacc.c"
    break;

  case 77: /* sql_trx_rollback: TRXROLLBACK  */
#line 391 "minisql.y"
              {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeTrxRollback, NULL);
  }
#line 1884 "./minisql_yacc.c"
    break;

  case 78: /* sql_quit: QUIT  */
#line 397 "minisql.y"
       {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeQuit, NULL);
  }
#line 1892 "./minisql_yacc.c"
    break;

  case 79: /* sql_exec_file: EXECFILE STRING  */
#line 403 "minisql.y"
                  {
    (yyval.syntax_node) = CreateSyntaxNode(kNodeExecFile, NULL);
    SyntaxNodeAddChildren((yyval.syntax_node), (yyvsp[0].syntax_node));
  }
#line 1901 "./minisql_yacc.c"
    break;


#line 1905 "./minisql_yacc.c"

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

#line 409 "minisql.y"

int yyerror(char* error) {
	MinisqlParserSetError(error);
	return 0;
}
