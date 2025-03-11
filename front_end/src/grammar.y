%{

#define T_NULL 0
#define WARNINGS_TO_BE_KEPT
#include <stdio.h>
#include "InputTheory.h"
#define yacc_debug 0

int yylex(void);
int yyerror(char[] ,...);
  
zap::InputTheory mainTheory;

%}

%union
{
  int			    i;
  char			    *s;
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


%token ILLEGAL COMMA SEMICOLON LPAREN RPAREN LBRACK RBRACK LCURLY RCURLY ASSIGNMENT
%token NEGATION LANGLE RANGLE ASSIGNMENT_OP TIMES ZERO NOTEQ
%token SORT PREDICATE GROUP
%token <i> OPERATOR UNIVERSAL EXISTENTIAL 
%token <s> SYMBOL INTEGER

%type <i>   num
%type <ia>  atom
%type <il>  literal
%type <ilv> term_list
%type <pss> restriction
%type <pssv> restriction_list
%type <sl>  sym_list variable_list parameter_list existentials universals
%type <icyc> cycle;
%type <ip>  perm;
%type <ig>  group generator_list;
%type <ds1> domain_spec
%type <ds>  domain_specs
%type <ps1> predicate_spec
%type <ps>  predicate_specs
%type <gs1> group_def
%type <gs>  group_defs
%type <ic>  clause rhs
%type <icv> clauses

%%

theory	: domain_specs predicate_specs group_defs clauses
	  {
	    mainTheory.m_domainSpecs = *$1;
	    mainTheory.m_predicateSpecs = *$2;
	    mainTheory.m_groupSpecs = *$3;
	    reverse($4->begin(),$4->end());
	    mainTheory.m_clauses = *$4;
	    delete $1;
	    delete $2;
	    delete $3;
	    delete $4;
	  }
	;

domain_specs	: /* empty */
                  {
					$$ = new zap::DomainSpec;
		  }
		| domain_spec domain_specs
                  {
		    $$ = $2;
		    (*$$)[$1->first] = $1->second;
		    delete $1;
		  }
		;

domain_spec	: SORT SYMBOL INTEGER SEMICOLON
                  {
		    $$ = new pair<string,int>($2,atoi($3));
		    free($2);
		    free($3);
		  }
                ;

predicate_specs	: /* empty */
                  {
					$$ = new zap::PredicateSpec;
		  }
		| predicate_spec predicate_specs
                  {
		    $$ = $2;
		    (*$$)[$1->first] = $1->second;
		    delete $1;
		  }
		;

predicate_spec	: PREDICATE SYMBOL LPAREN sym_list RPAREN SEMICOLON
                  {
		    $$ = new pair<string,vector<string> >($2,*$4);
		    free($2);
		    delete $4;
		  }
		;

sym_list	: /* empty */
                  {
		    $$ = new vector<string>;
		  }
		| SYMBOL sym_list
                  {
		    $$ = $2;
		    $$->push_back(string($1));
		    free($1);
		  }
		;

restriction_list : restriction
                  {
		    $$ = new vector<pair<string,string> >;
		    $$->push_back(*$1);
		    delete $1;
		  }
                | restriction COMMA restriction_list
                  {
		    $$ = $3;
		    $$->push_back(*$1);
		    delete $1;
                  }
                ;

group_defs	: /* empty */ 
                  {
					$$ = new zap::GroupSpec;
		  }                
		| group_def group_defs
                  {
		    $$ = $2;
		    (*$$)[$1->first] = $1->second;
		    delete $1;
		  }
		;

group_def	: GROUP SYMBOL group SEMICOLON
              {
				$$ = new pair<string,zap::igroup>($2,*$3);
				free($2);
				delete $3;
			  }
| GROUP SYMBOL clause
              {
				zap::igroup i;
				i.m_ic = *$3;
				$$ = new pair<string,zap::igroup>($2,i);
				free($2);
				delete $3;
			  }
| GROUP SYMBOL sym_list SEMICOLON
              {
				zap::igroup i;
				i.domains = *$3;
				$$ = new pair<string,zap::igroup>($2,i);
				free($2);
				delete $3;
			  }
;

clauses		: clause
                  {
					$$ = new vector<zap::InputClause>;
		    $$->push_back(*$1);
		    delete $1;
		  }
		| clauses clause
                  {
		    $$ = $1;
		    $$->push_back(*$2);
		    delete $2;
		  }
		;

clause		: universals existentials term_list rhs
{
  $$ = new zap::InputClause;
  $$->restrictions() = $4->restrictions();
  $$->add_literals(*$3);
  delete $3;
  $$->value() = $4->value();
  $$->op() = $4->op();
  $$->mod() = $4->mod();
  $$->gid() = $4->gid();
  delete $4;
  $$->universal() = *$1;
  $$->existential() = *$2;
  delete $1;
  delete $2;
} 
;

universals	: /* empty */
		  {
		    $$ = new vector<string>;
		  }
		| UNIVERSAL LPAREN variable_list RPAREN universals
		  {
		    $$ = $5;
		    $$->insert($$->end(),$3->begin(),$3->end());
		    delete $3;
		  }
		;

existentials	: /* empty */
		  {
		    $$ = new vector<string>;
		  }
		| EXISTENTIAL LPAREN variable_list RPAREN existentials
		  {
		    $$ = $5;
		    $$->insert($$->end(),$3->begin(),$3->end());
		    delete $3;
		  }
		;

rhs		: ZERO
                  {
					$$ = new zap::InputClause;
		  }
		| SEMICOLON
                  {
					$$ = new zap::InputClause;
		  }
		| ZERO SEMICOLON
                  {
					$$ = new zap::InputClause;
		  }
		| GROUP sym_list SEMICOLON
                  {
					$$ = new zap::InputClause;
		    $$->gid() = *$2;
		    delete $2;
		  }
                | LCURLY restriction_list RCURLY SEMICOLON
                  {
					$$ = new zap::InputClause;
		    $$->restrictions() = *$2;
		    delete $2;
		  }
		| OPERATOR num SEMICOLON
                  {
					$$ = new zap::InputClause;
		    if ($1 < 0) $$->op() = $1;
		    else { $$->op() = zap::EQ; $$->mod() = $1; }
		    $$->value() = $2;
		  }
		;

num		: ZERO
		  {
		    $$ = 0;
		  }
		| INTEGER
		  {
		    $$ = atoi($1);
		    free($1);
		  }
		;

variable_list	: SYMBOL
                  {
		    $$ = new vector<string>;
		    $$->push_back(string($1));
		    free($1);
		  }
                | SYMBOL variable_list
                  {
		    $$ = $2;
		    $$->push_back(string($1));
		    free($1);
		  }
		;

term_list	: literal
                  {
				
					$$ = new vector<zap::InputLiteral>;
		    $$->push_back(*$1);
		    delete $1;
		  }
                | literal term_list   
                  {
				
		    $$ = $2;
		    $$->push_back(*$1);
		    delete $1;
		  }                 
/*		| INTEGER TIMES literal term_list
                  {
		    temp_lit.set_weight(atoi($1) * temp_lit.get_weight());
		    temp_clause.add_literal(temp_lit);
		    }*/
		;

restriction     : SYMBOL NOTEQ SYMBOL
                  {
		    $$ = new pair<string,string>($1,$3);
		    free($1);
		    free($3);
		  }
                ;
literal		: atom
                  {
					$$ = new zap::InputLiteral;
		    $$->set_literal(true,*$1);
		    delete $1;
		  }
		| INTEGER
                  {
					$$ = new zap::InputLiteral;
		    int n = atoi($1);
		    $$->set_literal(n > 0,$1 + (n < 0));
		    free($1);
		  }
		| NEGATION atom
                  {
					$$ = new zap::InputLiteral;
		    $$->set_literal(false,*$2);
		    delete $2;
		  }
		;

atom		: SYMBOL
                  {
					$$ = new zap::InputAtom($1);
		    free($1);
		  }
		|  SYMBOL LBRACK parameter_list RBRACK
                   {
					 $$ = new zap::InputAtom($1,*$3);
		     delete $3;
		     free($1);
		   }
		;

parameter_list	: /* empty */
                  {
		    $$ = new vector<string>;
		  }
		| SYMBOL parameter_list
                  {
		    $$ = $2;
		    $$->push_back(string($1));
		    free($1);
		  }
                | INTEGER parameter_list
                  {
		    $$ = $2;
		    string temp("#");
		    temp.append($1);
		    $$->push_back(temp);
		    free($1);
		  }
		;

group		: LANGLE generator_list RANGLE
		  {
		    $$ = $2;
		  }
		;

generator_list	: /* empty */
		  {
		    $$ = new zap::igroup;
		  }
		| LPAREN perm RPAREN generator_list
		  {
		    $$ = $4;
		    $$->push_back(*$2);
		    delete $2;
		  }
		;

perm		: /* empty */
		  {
		    $$ = new zap::iperm;
		  }
		| LPAREN cycle RPAREN perm
		  {
		    $$ = $4;
		    $$->push_back(*$2);
		    delete $2;
		  }
		;

cycle		: /* empty */
		  {
		    $$ = new zap::icycle;
		  }
		| literal cycle
		  {
		    $$ = $2;
		    $$->push_back(*$1);
		    delete $1;
		  }
		;

%%

int yyerror(char *fmt,...)
{
  //  va_list argptr = NULL;	/* avoid compiler warning; va_list is void * */
    
  cout << "YYERROR: ";

  vprintf(fmt, NULL);
  cout << endl;
  return 1;
}
