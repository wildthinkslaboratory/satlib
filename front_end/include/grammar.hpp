/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     ILLEGAL = 258,
     COMMA = 259,
     SEMICOLON = 260,
     LPAREN = 261,
     RPAREN = 262,
     LBRACK = 263,
     RBRACK = 264,
     LCURLY = 265,
     RCURLY = 266,
     ASSIGNMENT = 267,
     NEGATION = 268,
     LANGLE = 269,
     RANGLE = 270,
     ASSIGNMENT_OP = 271,
     TIMES = 272,
     ZERO = 273,
     NOTEQ = 274,
     SORT = 275,
     PREDICATE = 276,
     GROUP = 277,
     OPERATOR = 278,
     UNIVERSAL = 279,
     EXISTENTIAL = 280,
     SYMBOL = 281,
     INTEGER = 282
   };
#endif
/* Tokens.  */
#define ILLEGAL 258
#define COMMA 259
#define SEMICOLON 260
#define LPAREN 261
#define RPAREN 262
#define LBRACK 263
#define RBRACK 264
#define LCURLY 265
#define RCURLY 266
#define ASSIGNMENT 267
#define NEGATION 268
#define LANGLE 269
#define RANGLE 270
#define ASSIGNMENT_OP 271
#define TIMES 272
#define ZERO 273
#define NOTEQ 274
#define SORT 275
#define PREDICATE 276
#define GROUP 277
#define OPERATOR 278
#define UNIVERSAL 279
#define EXISTENTIAL 280
#define SYMBOL 281
#define INTEGER 282




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 17 "grammar.y"
{
  int			    i;
  char			  * s;
  zap::InputAtom		  * ia;
  zap::InputLiteral		  * il;
  zap::InputClause		  * ic;
  vector<zap::InputLiteral>    * ilv; 
  pair<string,string>     * pss;
  vector<pair<string,string> > *pssv; 
  vector<string>	  * sl;
  pair<string,int>	  * ds1;
  zap::DomainSpec		  * ds;
  pair<string,vector<string> > * ps1;
  zap::PredicateSpec		  * ps;
  pair<string,zap::igroup>	  * gs1;
  zap::GroupSpec		  * gs;
  zap::icycle		  * icyc;
  zap::iperm			  * ip;
  zap::igroup		  * ig;
  vector<zap::InputClause>	  * icv;
}
/* Line 1529 of yacc.c.  */
#line 125 "grammar.hpp"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

