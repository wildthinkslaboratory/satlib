%{
/* Lexical analyzer specification for a regular expression analyzer

Possible output tokens and additional information are as follows:

COMMA
SEMICOLON
ZERO
TIMES
LPAREN
RPAREN
LBRACK
RBRACK
LCURLY
RCURLY
OPERATOR		i is type (> >= = <= < are -1 to -5; n>0 is = mod n)
SYMBOL			i is lookup in symbol table
INTEGER			i is value
NOTEQ
ASSIGNMENT
EXISTENTIAL
UNIVERSAL		
NEGATION
LANGLE
RANGLE
ILLEGAL			? in group specification
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "InputTheory.h"
#include "grammar.hpp"
#include <ctype.h>
#include <stdarg.h>

#define YY_NEVER_INTERACTIVE 1
extern YYSTYPE yylval;

%}
%option noyywrap
str [a-zA-Z][a-zA-Z0-9]*
num [+-]?[1-9][0-9]*
white [ \t\n\r+]+
/* note + included in white space! */

%x COMMENT
%s GRP 

%%

"/*"		BEGIN(COMMENT);

<COMMENT>[^*\n]*	/* eat anything not a * */
<COMMENT>"*"+[^*/]*	/* eat *'s followed by anything other than / */
<COMMENT>"*"+"/"	BEGIN(INITIAL);

"//".*$		/* // style comment */
^"c ".*$
^"c"$
^"p cnf ".*$

<GRP>"?"	return ILLEGAL;

","             return COMMA;
";"		return SEMICOLON;
"*"		return TIMES;
0		return ZERO;
"("		return LPAREN;
")"		return RPAREN;
"["		return LBRACK;
"]"		return RBRACK;
"{"             return LCURLY;
"}"             return RCURLY;
<GRP>"<"	return LANGLE;
<GRP>">"	{ BEGIN(INITIAL); return RANGLE; }
">"		{ yylval.i = -1; return OPERATOR; }
">="		{ yylval.i = -2; return OPERATOR; }
"="		{ yylval.i = -3; return OPERATOR; }
"<="		{ yylval.i = -4; return OPERATOR; }
"<"		{ yylval.i = -5; return OPERATOR; }
"%"{num}"="	{ yylval.i = atoi(yytext+1); return OPERATOR; }
"!="            return NOTEQ;
FORALL		{ return UNIVERSAL; }
EXISTS		{ return EXISTENTIAL; }
SORT		return SORT;
PREDICATE	return PREDICATE;
GROUP		{ BEGIN(GRP); return GROUP; }
{str}		{ yylval.s = strdup(yytext); return SYMBOL; }
{num}		{ yylval.s = strdup(yytext); return INTEGER; }
":="		return ASSIGNMENT;
[-~]		return NEGATION;

{white}		/* eat white space */

%%

