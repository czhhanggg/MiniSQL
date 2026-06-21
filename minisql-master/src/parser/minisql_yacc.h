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

#ifndef YY_YY_MINISQL_YACC_H_INCLUDED
# define YY_YY_MINISQL_YACC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
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
    CREATE = 258,                  /* CREATE  */
    DROP = 259,                    /* DROP  */
    SELECT = 260,                  /* SELECT  */
    INSERT = 261,                  /* INSERT  */
    DELETE = 262,                  /* DELETE  */
    UPDATE = 263,                  /* UPDATE  */
    TRXBEGIN = 264,                /* TRXBEGIN  */
    TRXCOMMIT = 265,               /* TRXCOMMIT  */
    TRXROLLBACK = 266,             /* TRXROLLBACK  */
    QUIT = 267,                    /* QUIT  */
    EXECFILE = 268,                /* EXECFILE  */
    SHOW = 269,                    /* SHOW  */
    USE = 270,                     /* USE  */
    USING = 271,                   /* USING  */
    DATABASE = 272,                /* DATABASE  */
    DATABASES = 273,               /* DATABASES  */
    TABLE = 274,                   /* TABLE  */
    TABLES = 275,                  /* TABLES  */
    INDEX = 276,                   /* INDEX  */
    INDEXES = 277,                 /* INDEXES  */
    ON = 278,                      /* ON  */
    FROM = 279,                    /* FROM  */
    WHERE = 280,                   /* WHERE  */
    INTO = 281,                    /* INTO  */
    SET = 282,                     /* SET  */
    VALUES = 283,                  /* VALUES  */
    PRIMARY = 284,                 /* PRIMARY  */
    KEY = 285,                     /* KEY  */
    UNIQUE = 286,                  /* UNIQUE  */
    CHAR = 287,                    /* CHAR  */
    INT = 288,                     /* INT  */
    FLOAT = 289,                   /* FLOAT  */
    AND = 290,                     /* AND  */
    OR = 291,                      /* OR  */
    NOT = 292,                     /* NOT  */
    IS = 293,                      /* IS  */
    FLAGNULL = 294,                /* FLAGNULL  */
    IDENTIFIER = 295,              /* IDENTIFIER  */
    STRING = 296,                  /* STRING  */
    NUMBER = 297,                  /* NUMBER  */
    EQ = 298,                      /* EQ  */
    NE = 299,                      /* NE  */
    LE = 300,                      /* LE  */
    GE = 301                       /* GE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 10 "minisql.y"

	pSyntaxNode syntax_node;

#line 114 "./minisql_yacc.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_MINISQL_YACC_H_INCLUDED  */
