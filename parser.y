%{
#include <cmath>
#include <cstdio>
#include <cstring>
#include "JojobaScript.h"
#include "scanner.h"

#pragma warning(push)
#pragma warning(disable: 4127 4244 4702)
%}

%union {
	int value;
	char* id;
}

%token AS ASYNC BREAK CASE CONTINUE DEC DEFAULT DO ELSE FOR FROM FUNCTION IF IMPORT INC RETURN SEP STRING SWITCH VAR WHILE YIELD
%token <value> ASSIGNMENT NUMBER
%token <id> ID
%nonassoc '?' ':'
%left AND OR
%left EQ NE LE GE '<' '>'
%left '@' ')' /* TODO:  perhaps nonassoc? */
%left '+' '-'
%left '&' '|' '^'
%left ASR LSR SHL
%left '*' '/' '%'
%precedence NEG /* negation:  unary minus, bit-wise complement, logical complement */
%right SS
%nonassoc AWAIT
%left '.' '['
%type <value> dexpr dfpexpr expr expr_list fexpr id_list kv_list oexpr_list oid_list pexpr

%%

block:
statement { emit(); }
| block SEP statement { emit(); }
;

statement:
%empty { emit(); }
| ID ':' { add_symbol($1); }
| function ID '(' oid_list ')' { undo_bracket(0); } '{' block '}' { add_symbol($2); }
| VAR initializers { emit(); }
| BREAK { emit(); }
| CONTINUE { emit(); }
| DO { undo_bracket(0); } '{' block '}' WHILE expr { emit(); }
| FOR simple_statement SEP expr SEP simple_statement { undo_bracket(1); } '{' block '}' { emit(); }
| FOR SEP expr SEP simple_statement { undo_bracket(1); } '{' block '}' { emit(); }
| FOR simple_statement SEP SEP simple_statement { undo_bracket(1); } '{' block '}' { emit(); }
| FOR id_list ':' simple_statement { undo_bracket(1); } '{' block '}' { emit(); }
| if_statement { emit(); }
| if_statement ELSE { undo_bracket(0); } '{' block '}' { emit(); }
| RETURN expr { emit(); }
| SWITCH expr { undo_bracket(1); } '{' cases '}' { emit(); }
| WHILE expr { undo_bracket(1); } '{' block '}' { emit(); }
| YIELD expr { emit(); }
| FROM STRING IMPORT imports { emit(); }
| IMPORT STRING { emit(); }
| IMPORT STRING AS ID { emit(); }
| simple_statement { emit(); }
;

function:
ASYNC FUNCTION { emit(); }
| FUNCTION { emit(); }
;

initializers:
initializer { emit(); }
| initializers ',' initializer { emit(); }
;

initializer:
ID { emit(); }
| ID '=' expr { emit(); }
;

if_statement:
IF expr { undo_bracket(1); } '{' block '}' { emit(); }
;

cases:
separators { emit(); }
| cases CASE expr ':' block { emit(); }
| cases DEFAULT ':' block { emit(); }
;

separators:
%empty { emit(); }
| separators SEP { emit(); }
;

imports:
import { emit(); }
| imports ',' import { emit(); }
;

import:
ID { emit(); }
| ID AS ID { emit(); }
;

simple_statement:
dexpr ASSIGNMENT expr { emit($3); }
| di dexpr { emit($2); }
| dexpr di { emit($1); }
| fexpr { emit($1); }
;

di:
DEC { emit(); }
| INC { emit(); }
;

expr:
dfpexpr
| expr '?' expr ':' expr { $$ = $1 ? $3 : $5; }
| expr AND expr          { $$ = $1 && $3;     }
| expr OR expr           { $$ = $1 || $3;     }
| expr EQ expr           { $$ = $1 == $3;     }
| expr NE expr           { $$ = $1 != $3;     }
| expr LE expr           { $$ = $1 <= $3;     }
| expr GE expr           { $$ = $1 >= $3;     }
| expr '<' expr          { $$ = $1 < $3;      }
| expr '>' expr          { $$ = $1 > $3;      }
| expr '+' expr          { $$ = $1 + $3;      }
| expr '-' expr          { $$ = $1 - $3;      }
| expr '&' expr          { $$ = $1 & $3;      }
| expr '|' expr          { $$ = $1 | $3;      }
| expr '^' expr          { $$ = $1 ^ $3;      }
| expr SHL expr          { $$ = $1 << $3;     }
| expr LSR expr          { $$ = $1 >> $3;     }
| expr ASR expr          { $$ = $1 < 0 ? -(-$1 >> $3) : $1 >> $3; }
| expr '*' expr          { $$ = $1 * $3;      }
| expr '/' expr          { $$ = $1 / $3;      }
| expr '%' expr          { $$ = $1 % $3;      }
| expr SS expr           { $$ = pow($1, $3);  }
| '-' expr %prec NEG     { $$ = -$2;          }
| '~' expr %prec NEG     { $$ = ~$2;          }
| '!' expr %prec NEG     { $$ = !$2;          }
| '{' oexpr_list '}'     { $$ = $2;           } /* If this is empty, interpret as an object, not a set. */
| '{' kv_list '}'        { $$ = $2;           }
| AWAIT expr             { $$ = $2;           }
| expr '@' '(' id_list ')' expr { $$ = $1; }
;

dfpexpr:
dexpr
| fexpr
| pexpr
;

dexpr:
dfpexpr '.' ID         { emit();             }
| dfpexpr '[' expr ']' { emit();             }
| '[' oexpr_list ']'   { $$ = $2;            }
| ID                   { $$ = get_value($1); }
| NUMBER               { $$ = $1;            }
;

fexpr:
dfpexpr '(' oexpr_list ')' { $$ = $1; }
;

pexpr:
'(' expr ')' { $$ = $2; }
;

oexpr_list:
%empty { emit(); }
| expr_list { $$ = $1; }
;

expr_list:
expr { $$ = $1; }
| expr_list ',' expr { $$ = $3; }
;

kv_list:
expr ':' expr { $$ = $3; }
| kv_list ',' ID ':' expr { $$ = $5; }
;

id_list:
ID { $$ = get_value($1); }
| id_list ',' ID { $$ = $1; }
;

oid_list:
%empty { emit(); }
| id_list { $$ = $1; }
;

%%

#pragma warning(pop)

extern int yylineno;

void yyerror(char const* message) {
	fprintf(stderr, "problem in line %d: %s\n", yylineno, message);
}
