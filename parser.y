%{
#include <cstdio>
#include <stdexcept>
#include "JojobaScript.h"
#include "scanner.h"

#pragma warning(push)
#pragma warning(disable: 4127 4244 4702)
%}

%union {
	int value;
	char* id;
}

%token AS BREAK CASE CONTINUE DEC DEFAULT DO ELSE FOR FROM FUNCTION IF IMPORT IN INC RETURN STRING SWITCH UNTIL VAR WHILE YIELD
%token <value> ASSIGNMENT NUMBER
%token <id> ID
%nonassoc '?' ':'
%left AND OR
%left EQ NE LE GE '<' '>'
%left '+' '-'
%left '&' '|' '^'
%left ASR LSR SHL
%left '*' '/' '%'
%precedence NEG /* negation:  unary minus, bit-wise complement, logical complement */
%right SS
%nonassoc AWAIT
%left '.' '['

%%

block:
%empty
| block statement
;

statement:
FUNCTION ID '(' oid_list ')' otype '{' block '}'
| VAR initializers
| BREAK
| CONTINUE
| DO '{' block '}' uw expr
| FOR ofor_clauses ';' oexpr_list ';' ofor_clauses '{' block '}'
| FOR id_list IN for_clause '{' block '}'
| if_statement
| if_statement oelseif_statements ELSE '{' block '}'
| RETURN expr
| SWITCH expr '{' cases '}'
| uw expr '{' block '}'
| YIELD expr
| FROM STRING IMPORT imports
| IMPORT STRING
| IMPORT STRING AS ID
| '@' dexpr ASSIGNMENT expr
| '@' fexpr
| di dexpr
;

otype:
%empty
| ':' type
;

type:
ID
| type '[' otype_list ']'
;

otype_list:
%empty
| type_list
;

type_list:
type
| type_list ',' type
;

initializers:
initializer
| initializers ',' initializer
;

initializer:
ID otype
| ID otype ASSIGNMENT expr { if ($3 != 0) throw std::logic_error("invalid initializing assignment"); }
;

uw:
UNTIL
| WHILE
;

ofor_clauses:
%empty
| for_clauses
;

for_clauses:
for_clause
| for_clauses ',' for_clause
;

for_clause:
dexpr ASSIGNMENT expr
| di dexpr
| fexpr
;

if_statement:
IF expr '{' block '}'
;

oelseif_statements:
%empty
| oelseif_statements elseif_statement
;

elseif_statement:
ELSE IF expr '{' block '}'
;

cases:
%empty
| cases CASE expr ':' block
| cases DEFAULT ':' block
;

imports:
import
| imports ',' import
;

import:
ID
| ID AS ID
;

di:
DEC
| INC
;

expr:
dfpexpr
| expr '?' expr ':' expr
| expr AND expr
| expr OR expr
| expr EQ expr
| expr NE expr
| expr LE expr
| expr GE expr
| expr '<' expr
| expr '>' expr
| expr '+' expr
| expr '-' expr
| expr '&' expr
| expr '|' expr
| expr '^' expr
| expr SHL expr
| expr LSR expr
| expr ASR expr
| expr '*' expr
| expr '/' expr
| expr '%' expr
| expr SS expr
| '-' expr %prec NEG
| '~' expr %prec NEG
| '!' expr %prec NEG
| '[' oexpr_list ']'
| '[' expr FOR id_list IN expr ']'
| '{' expr ':' expr FOR id_list IN expr '}'
| '{' oexpr_list '}' /* If this is empty, interpret as an object, not a set. */
| '{' kv_list '}'
| AWAIT expr
| NUMBER
;

dfpexpr:
dexpr
| fexpr
| '(' expr ')'
;

dexpr:
dfpexpr '.' ID
| dfpexpr '[' expr ']'
| ID
;

fexpr:
dfpexpr '(' oexpr_list ')'
;

oexpr_list:
%empty
| expr_list
;

expr_list:
expr
| expr_list ',' expr
;

kv_list:
expr ':' expr
| kv_list ',' expr ':' expr
;

oid_list:
%empty
| id_list
;

id_list:
ID otype
| id_list ',' ID otype
;

%%

#pragma warning(pop)

extern int yylineno;

void yyerror(char const* message) {
	fprintf(stderr, "problem in line %d: %s\n", yylineno, message);
}
